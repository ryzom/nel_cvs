/** \file zone_welder.cpp
 * Tool for welding zones exported from 3dsMax
 *
 * $Id: zone_welder.cpp,v 1.4 2001/01/11 16:02:21 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


#include <iostream.h>
#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/3d/quad_tree.h"
#include "nel/3d/zone.h"
#include <vector>
#include <set>


using namespace NL3D;
using namespace NLMISC;
using namespace std;


#define WELD_LOG 1

FILE *fdbg;

std::string inputDir;
std::string inputExt;
std::string outputDir;
std::string outputExt;


/**
 * CWeldableVertexInfos
 */
struct CWeldableVertexInfos
{
	uint16 IndexInZone; // base vertex
	sint PatchIndex;	// patch
	uint8 PatchVertex;	// 0,1,2,3

	bool operator< (const CWeldableVertexInfos& wvinf) const
	{
		if(IndexInZone<wvinf.IndexInZone)
			return true;
		if(IndexInZone>wvinf.IndexInZone)
			return false;
		if(PatchIndex<wvinf.PatchIndex)
			return true;
		if(PatchIndex>wvinf.PatchIndex)
			return false;
		return PatchVertex<wvinf.PatchVertex;
	}
};

std::string getDir (const std::string& path)
{
	char tmpPath[512];
	strcpy (tmpPath, path.c_str());
	char* slash=strrchr (tmpPath, '/');
	if (!slash)
	{
		slash=strrchr (tmpPath, '\\');
	}

	if (!slash)
		return "";

	slash++;
	*slash=0;
	return tmpPath;
}

std::string getName (const std::string& path)
{
	std::string dir=getDir (path);

	char tmpPath[512];
	strcpy (tmpPath, path.c_str());

	char *name=tmpPath;
	nlassert (dir.length()<=strlen(tmpPath));
	name+=dir.length();

	char* point=strrchr (name, '.');
	if (point)
		*point=0;

	return name;
}

std::string getExt (const std::string& path)
{
	std::string dir=getDir (path);
	std::string name=getName (path);

	char tmpPath[512];
	strcpy (tmpPath, path.c_str());

	char *ext=tmpPath;
	nlassert (dir.length()+name.length()<=strlen(tmpPath));
	ext+=dir.length()+name.length();

	return ext;
}

/*******************************************************************\
						writeInstructions()
\*******************************************************************/
void writeInstructions()
{
	printf("zone_welder [input.zone][output.zone]\n");
	printf("\t/? for this help\n");
}


/*******************************************************************\
						findPatchIndex()
\*******************************************************************/
bool getPatchAndEdge(const std::vector<CPatchInfo>& patchs,
					  uint16 baseVertex1, uint16 baseVertex2, 
					  uint16& patchIndex,
					  uint8& edgeIndex)
{
	uint ptch;
		
	for(ptch=0; ptch<patchs.size(); ptch++)
	{
		uint i;
		for(i=0; i<4; i++)
		{
			if(patchs[ptch].BaseVertices[i] == baseVertex1)
			{
#if WELD_LOG
				fprintf(fdbg,"patch %d contient bv %d : %d %d %d %d\n",
					ptch,
					baseVertex1,
					patchs[ptch].BaseVertices[0],
					patchs[ptch].BaseVertices[1],
					patchs[ptch].BaseVertices[2],
					patchs[ptch].BaseVertices[3]);
#endif
				
				if(patchs[ptch].BaseVertices[(i+1)%4] == baseVertex2)
				{
					edgeIndex = i;
					patchIndex = ptch;
					return true;
				}
				if(patchs[ptch].BaseVertices[(i-1)%4] == baseVertex2)
				{
					edgeIndex = (i-1)%4;
					patchIndex = ptch;
					return true;
				}
			}
		}
	}
	return false;
}


/*******************************************************************\
						getZoneCoordByName()
\*******************************************************************/
void getZoneCoordByName(const char * name, uint16& x, uint16& y)
{
	uint i;
	
	std::string zoneName(name);

	// y
	uint ind1 = zoneName.find("_");
	if(ind1>=zoneName.length())
	{
		nlwarning("bad file name");
		return;
	}
	std::string ystr = zoneName.substr(0,ind1);
	for(i=0; i<ystr.length(); i++)
	{
		if(!isdigit(ystr[i]))
		{
			nlwarning("y code size is not a 2 characters code");
			return;
		}
	}
	y = atoi(ystr.c_str());

	// x
	x = 0;
	uint ind2 = zoneName.length();
	if((ind2-ind1-1)!=2)
	{
		nlwarning("x code size is not a 2 characters code");
		return;
	}
	std::string xstr = zoneName.substr(ind1+1,ind2-ind1-1);
	for(i=0; i<xstr.length(); i++)
	{
		if (isalpha(xstr[i]))
		{
			x *= 26;
			x += (tolower(xstr[i])-'a');
		}
		else
		{
			nlwarning("invalid");
			return;
		}
	}
}


/*******************************************************************\
						getLettersFromNum()
\*******************************************************************/
void getLettersFromNum(uint16 num, std::string& code)
{
	if(num>26*26) 
	{
		nlwarning("zone index too high");
		return;
	}
	code.resize(0);
	uint16 remainder = num%26;
	code += 'A' + num/26;
	code += 'A' + remainder;
}


/*******************************************************************\
						getZoneNameByCoord()
\*******************************************************************/
void getZoneNameByCoord(uint16 x, uint16 y, std::string& zoneName)
{
	// y str
	char stmp[10];
	sprintf(stmp,"%d",y);
	std::string ystrtmp = std::string(stmp);

	// x str
	std::string xstrtmp;
	getLettersFromNum(x, xstrtmp);

	// name
	zoneName = ystrtmp;
	zoneName +="_";
	zoneName +=xstrtmp;
}



/*******************************************************************\
						getAdjacentZonesName()
\*******************************************************************/
void getAdjacentZonesName(const std::string& zoneName, std::vector<std::string>& names)
{
	uint16 x,y;
	int xtmp,ytmp;
	std::string nametmp;
	std::string empty("empty");

	names.reserve(8);
	
	getZoneCoordByName(zoneName.c_str(), x, y);

	// top left
	xtmp = x-1;
	ytmp = y-1;
	if(xtmp<0||ytmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// top
	xtmp = x;
	ytmp = y-1;
	if(ytmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// top right
	xtmp = x+1;
	ytmp = y-1;
	if(ytmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// left
	xtmp = x-1;
	ytmp = y;
	if(xtmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// right
	xtmp = x+1;
	ytmp = y;
	getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// bottom left
	xtmp = x-1;
	ytmp = y+1;
	if(xtmp<0)
		nametmp = empty;
	else
		getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// bottom
	xtmp = x;
	ytmp = y+1;
	getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);

	// bottom right 
	xtmp = x+1;
	ytmp = y+1;
	getZoneNameByCoord(xtmp, ytmp, nametmp);
	names.push_back(nametmp);
}


/*******************************************************************\
						createZoneId()
\*******************************************************************/
uint16 createZoneId(std::string zoneName)
{
	uint16 x,y;
	getZoneCoordByName(zoneName.c_str(), x, y);
	return (x<<8) + y;
}


/*******************************************************************\
							weldZones()
\*******************************************************************/
void weldZones(const char *center)
{
	uint i,j;

	float weldRadius = 0.05f;//0.03f;

	// load zone in the center
	CIFile zoneFile(inputDir+center+inputExt);
	CZone zone;
	zone.serial(zoneFile);
	zoneFile.close();
	
	// retrieving infos from the center zone
	uint16 centerZoneId = zone.getZoneId();
	std::vector<CPatchInfo> centerZonePatchs;
	std::vector<CBorderVertex> centerZoneBorderVertices;
	zone.retrieve(centerZonePatchs, centerZoneBorderVertices);
	
	std::vector<CPatchInfo>::iterator itptch;
	std::vector<CBorderVertex>::iterator itbv;

	// if no id yet, we add a correct id
	if(centerZoneId==0) 
	{
		centerZoneId = createZoneId(center);
		
		// edge neighbour : current zone
		for(itptch = centerZonePatchs.begin(); itptch!=centerZonePatchs.end(); itptch++)
		{
			for(j=0; j<4; j++)
			{
				(*itptch).BindEdges[j].ZoneId = centerZoneId;
			}
		}

		// border vertices neighbour : current zone
		for(itbv = centerZoneBorderVertices.begin(); itbv<centerZoneBorderVertices.end(); itbv++)
		{
			(*itbv).NeighborZoneId = centerZoneId;
		}
		
	}
#if WELD_LOG
	fprintf(fdbg,"id(center) = %d\n",centerZoneId);
#endif



	// load 8 adjacent adjZones
	bool adjZoneFileFound[8];
	CZone adjZones[8];
	uint16 adjZonesId[8];
	std::vector<std::string> adjZonesName;
	getAdjacentZonesName(center, adjZonesName);
	for(i=0; i<8; i++)
	{
		if(adjZonesName[i]=="empty") continue;
		
		adjZoneFileFound[i] = true;
		CIFile f;
		try
		{
			std::string ss(outputDir+adjZonesName[i]+outputExt);
			if (f.open(ss))
			{
				printf("reading file %s\n", ss.c_str());
				adjZones[i].serial(f);
				adjZonesId[i] = adjZones[i].getZoneId();
				f.close();
			}
			else
			{
				printf("File not found: %s\n", ss.c_str());
				adjZonesName[i]="empty";
			}
		}
		catch(exception &e)
		{
			printf ("%s\n", e.what ());
			adjZoneFileFound[i] = false;
		}
	}
	
	// QuadTree for storing adjZones points
	CQuadTree<CWeldableVertexInfos> quadTrees[8];
	
	// new base, to change from XZ to XY (Nel speaking)
	CMatrix	base;
	CVector	I(1,0,0);
	CVector	J(0,0,-1);
	CVector	K(0,1,0);
	base.setRot(I,J,K, true);

	
	
	uint ptch;

	uint16 weldCount = 0;

	for(i=0; i<8; i++)
	{
		if(adjZonesName[i]=="empty") continue;
		if(!adjZoneFileFound[i]) continue;

		// setting quad tree
		uint qTreeDepth = 5;
		CAABBoxExt bb = adjZones[i].getZoneBB();
		quadTrees[i].create (5, bb.getCenter(), 2*bb.getRadius());
		quadTrees[i].changeBase(base);

		// retrieving infos from the current adjacent zone
		std::vector<CPatchInfo> adjZonePatchs;
		std::vector<CBorderVertex> adjZoneBorderVertices;
		adjZones[i].retrieve(adjZonePatchs, adjZoneBorderVertices);


		// if no id yet, we add a correct id
		nlassert(adjZonesId[i]!=0);
		if(adjZonesId[i]==0) 
		{
			adjZonesId[i] = createZoneId(getName (adjZonesName[i]));
			
			// edge neighbour : current zone
			for(itptch = adjZonePatchs.begin(); itptch!=adjZonePatchs.end(); itptch++)
			{
				for(j=0; j<4; j++)
				{
					(*itptch).BindEdges[j].ZoneId = adjZonesId[i];
				}
			}

			// border vertices neighbour : current zone
			for(itbv = adjZoneBorderVertices.begin(); itbv!=adjZoneBorderVertices.end(); itbv++)
			{
				(*itbv).NeighborZoneId = adjZonesId[i];
			}
			
		}
#if WELD_LOG
		fprintf(fdbg,"------------------------------------------\n");
		fprintf(fdbg,"id(%d) = %d\n",i,adjZonesId[i]);
#endif

		// an edge of current adjacent patch with neighbour zoneId==center zoneId is
		// set to no neighbour.
		for(ptch = 0; ptch<adjZonePatchs.size(); ptch++)
		{
			for(j=0; j<4; j++)
			{
				if(adjZonePatchs[ptch].BindEdges[j].ZoneId ==0)
				{
					adjZonePatchs[ptch].BindEdges[j].ZoneId	= adjZonesId[i];
				}
				if(adjZonePatchs[ptch].BindEdges[j].ZoneId == centerZoneId)
				{
					adjZonePatchs[ptch].BindEdges[j].NPatchs = 0;
				}
			}
		}

		fprintf(fdbg,"(before) zone %d vertices size : %d\n",i,adjZoneBorderVertices.size());

		// delete border vertices of the adjacent zone if their neighbour zoneId
		// is equal to current zone zoneId
		std::vector<CBorderVertex>::iterator itborder = adjZoneBorderVertices.begin();
		while(itborder != adjZoneBorderVertices.end())
		{
			if((*itborder).NeighborZoneId == centerZoneId)
			{
				itborder = adjZoneBorderVertices.erase(itborder);
			}
			else
				itborder++;
		}
		fprintf(fdbg,"(after) zone %d vertices size : %d\n",i,adjZoneBorderVertices.size());

		// A set for storing base vertex index already added in the quad tree
		std::set<uint16> adjBaseVertexIndexSet;

		// if point in adjacent zone is not in the set :
		// -> add it in the set
		// -> add it in the quad
		for(ptch = 0; ptch<adjZonePatchs.size(); ptch++)
		{
			for(j=0; j<4; j++)
			{
				CWeldableVertexInfos wvinf;
				wvinf.IndexInZone = adjZonePatchs[ptch].BaseVertices[j]; // useful ????
				wvinf.PatchIndex = ptch;
				wvinf.PatchVertex = j;
				if(adjBaseVertexIndexSet.find(wvinf.IndexInZone) == adjBaseVertexIndexSet.end())
				{
					adjBaseVertexIndexSet.insert(wvinf.IndexInZone);
					CVector bboxmin;
					CVector bboxmax;
					bboxmin.x = adjZonePatchs[ptch].Patch.Vertices[j].x;
					bboxmin.y = adjZonePatchs[ptch].Patch.Vertices[j].y;
					bboxmin.z = adjZonePatchs[ptch].Patch.Vertices[j].z;
					bboxmax = bboxmin;
					quadTrees[i].insert(bboxmin,bboxmax,wvinf);
				}
			}
		}

		quadTrees[i].clearSelection();


		float bboxRadius = 10; //TEMP !!
		
		std::set<uint16> centerBaseVertexIndexSet;
		std::set<uint16> currentAdjBaseVertexIndexSet;

		for(ptch=0; ptch<centerZonePatchs.size(); ptch++) // for all patchs in center zone
		{
			// stores infos for edge part
			CWeldableVertexInfos nearVertexInfos[4];
			
			bool toWeld[4];
			
			CVector bboxmin;
			CVector bboxmax;
			
			
			// for every points in center patch we look for close points in adjacent patch
			for(j=0; j<4; j++) // 4 patch vertices (in center zone)
			{
				toWeld[j] = false;

				
				// already 'checked for welding' vertices are stored in a set
				centerBaseVertexIndexSet.insert(centerZonePatchs[ptch].BaseVertices[j]);

				//fprintf(fdbg,"%d - %d) CZBV(%d)\n",i,baseVertexIndexSet.size(),centerZonePatchs[ptch].BaseVertices[j]);

				bboxmin.x = centerZonePatchs[ptch].Patch.Vertices[j].x - bboxRadius;
				bboxmin.y = centerZonePatchs[ptch].Patch.Vertices[j].y - bboxRadius;
				bboxmin.z = centerZonePatchs[ptch].Patch.Vertices[j].z - bboxRadius;

				bboxmax.x = centerZonePatchs[ptch].Patch.Vertices[j].x + bboxRadius;
				bboxmax.y = centerZonePatchs[ptch].Patch.Vertices[j].y + bboxRadius;
				bboxmax.z = centerZonePatchs[ptch].Patch.Vertices[j].z + bboxRadius;
			
				//quadTrees[i].select(bboxmin,bboxmax);
				quadTrees[i].selectAll();	// TEMP !!!

				// current vertex coordinates in center zone
				CVector vctr;
				vctr.x = centerZonePatchs[ptch].Patch.Vertices[j].x;
				vctr.y = centerZonePatchs[ptch].Patch.Vertices[j].y;
				vctr.z = centerZonePatchs[ptch].Patch.Vertices[j].z;
				
				CWeldableVertexInfos wvinf;
				float minDistance = weldRadius + 1;  // rq: we weld only if we found a distance
													 //     inferior to weldRadius
				
				CQuadTree<CWeldableVertexInfos>::CIterator itqdt = quadTrees[i].begin();
				// for all points near of current vertex in adjacent zone..
				while (itqdt != quadTrees[i].end()) 
				{
					CVector vadj;
					vadj.x = adjZonePatchs[(*itqdt).PatchIndex].Patch.Vertices[(*itqdt).PatchVertex].x;
					vadj.y = adjZonePatchs[(*itqdt).PatchIndex].Patch.Vertices[(*itqdt).PatchVertex].y;
					vadj.z = adjZonePatchs[(*itqdt).PatchIndex].Patch.Vertices[(*itqdt).PatchVertex].z;
					
					CVector adjToCenter;
					adjToCenter.x = vctr.x - vadj.x;
					adjToCenter.y = vctr.y - vadj.y;
					adjToCenter.z = vctr.z - vadj.z;
					float dist = adjToCenter.norm();
					
					// if dist min we keep infos on this vertex(adj zone)
					// we keep the closest.
					if(dist<weldRadius && dist<minDistance) 
					{
						minDistance = dist;
						wvinf = (*itqdt);
					}
					itqdt++;
				}

				quadTrees[i].clearSelection();

				if(minDistance<weldRadius) // i.e if we have found 2 vertices to weld
				{				
					// we save CBorderVertex info, and add it into the adjacent zone
					CBorderVertex adjBorderV;
					adjBorderV.CurrentVertex = wvinf.IndexInZone;
					adjBorderV.NeighborZoneId = centerZoneId;
					adjBorderV.NeighborVertex = centerZonePatchs[ptch].BaseVertices[j];
					nearVertexInfos[j] = wvinf;

					// we save CBorderVertex info, and add it into the center zone
					CBorderVertex centerBorderV;
					centerBorderV.CurrentVertex = centerZonePatchs[ptch].BaseVertices[j];
					centerBorderV.NeighborZoneId = adjZonesId[i];
					centerBorderV.NeighborVertex = wvinf.IndexInZone;

					toWeld[j] = true;

					if(centerBaseVertexIndexSet.find(centerZonePatchs[ptch].BaseVertices[j]) != centerBaseVertexIndexSet.end())
					{
						if(currentAdjBaseVertexIndexSet.find(wvinf.IndexInZone) == currentAdjBaseVertexIndexSet.end())
						{
							currentAdjBaseVertexIndexSet.insert(wvinf.IndexInZone);
							adjZoneBorderVertices.push_back(adjBorderV);
							centerZoneBorderVertices.push_back(centerBorderV);

							weldCount++;
#if WELD_LOG
							fprintf(fdbg,"%d) weld vertices : zone%d.(patch%d.vertex%d).baseVertex%d to centerZone.(patch%d.vertex%d).baseVertex%d\n",
								weldCount,i,wvinf.PatchIndex,wvinf.PatchVertex,wvinf.IndexInZone,ptch,j,centerZonePatchs[ptch].BaseVertices[j]);
#endif
						}
					}
				}
			}
			

			
			// then we bind edges (made of weldable vertices) and modify tangents
			
			for(j=0; j<4; j++) 
			{
				// if vertex has been welded...
				if(toWeld[j] == false) continue;
				// ...we look if next vertex(i.e if the edge) in center zone has to be welded
				if(toWeld[(j+1)%4] == false) continue;
								
				// We know the two adjacent base vertices
				// we look for the adjacent patch and the edge containing these vertices
				uint8 edgeIndex;
				uint16 patchIndex;
				if(! getPatchAndEdge(adjZonePatchs,
									 nearVertexInfos[j].IndexInZone,
									 nearVertexInfos[(j+1)%4].IndexInZone,
									 patchIndex,
									 edgeIndex))
				{
#if WELD_LOG
					fprintf(fdbg,"* Error * : Can't find patch containing the following edge : %d - %d\n",
						nearVertexInfos[j].IndexInZone,
						nearVertexInfos[(j+1)%4].IndexInZone);
#endif
					continue;
				}

#if WELD_LOG
				fprintf(fdbg,"weld edges : zone%d.patch%d.edge%d(%d-%d) to centerZone.patch%d.edge%d(%d-%d)\n",
					i,
					patchIndex,
					edgeIndex,
					nearVertexInfos[j].IndexInZone,
					nearVertexInfos[(j+1)%4].IndexInZone,
					ptch,
					j,
					centerZonePatchs[ptch].BaseVertices[j],
					centerZonePatchs[ptch].BaseVertices[(j+1)%4] );
				fprintf(fdbg,"center patch %d : %d %d %d %d\n\n",
					ptch,
					centerZonePatchs[ptch].BaseVertices[0],
					centerZonePatchs[ptch].BaseVertices[1],
					centerZonePatchs[ptch].BaseVertices[2],
					centerZonePatchs[ptch].BaseVertices[3]);
#endif
					
				centerZonePatchs[ptch].BindEdges[j].NPatchs = 1;
				centerZonePatchs[ptch].BindEdges[j].ZoneId = adjZonesId[i];
				centerZonePatchs[ptch].BindEdges[j].Next[0] = patchIndex;   
				centerZonePatchs[ptch].BindEdges[j].Edge[0] = edgeIndex;  

				// adjacent zone edge
				adjZonePatchs[patchIndex].BindEdges[edgeIndex].NPatchs = 1;
				adjZonePatchs[patchIndex].BindEdges[edgeIndex].ZoneId = centerZoneId;
				adjZonePatchs[patchIndex].BindEdges[edgeIndex].Next[0] = ptch;
				adjZonePatchs[patchIndex].BindEdges[edgeIndex].Edge[0] = j;


				// tangent become the mean or both tangents (adj and center)
				// Here we cross the mean because adjacent edges are counter-oriented
				// due to the patchs constant orientation.
				CVector		middle0= (centerZonePatchs[ptch].Patch.Tangents[2*j]+
					adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex+1])/2;
				CVector		middle1= (centerZonePatchs[ptch].Patch.Tangents[2*j+1]+
					adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex])/2;

				centerZonePatchs[ptch].Patch.Tangents[2*j] = 
				adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex+1] = middle0;
					
				centerZonePatchs[ptch].Patch.Tangents[2*j+1] = 
				adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex] = middle1;

			}
			
		}

		adjZones[i].build(adjZonesId[i], adjZonePatchs, adjZoneBorderVertices);
#if WELD_LOG
		fprintf(fdbg,"[%d] binds :\n");
		adjZones[i].debugBinds(fdbg);
#endif
		std::string strtmp;

		//strtmp = outputPath;
		strtmp = outputDir+adjZonesName[i]+outputExt;
		COFile adjSave(strtmp);
		printf("writing file %s\n",strtmp.c_str());
		adjZones[i].serial(adjSave);
	}

	zone.build(centerZoneId, centerZonePatchs, centerZoneBorderVertices);
	std::string strtmp;
	strtmp = outputDir+center+outputExt;

	COFile centerSave(strtmp);
	printf("writing file %s\n",strtmp.c_str());
	zone.serial(centerSave);

}

/*******************************************************************\
							main()
\*******************************************************************/
int main(sint argc, char **argv)
{
	// no zone file in argument
	if(argc<3)
	{
		writeInstructions();
		return 0;
	}
	
	// help
	if(strcmp(argv[1],"/?")==0)
	{
		writeInstructions();
		return 0;
	}

#if WELD_LOG
	fdbg = fopen("log.txt","wt");
	fprintf(fdbg,"Center zone : %s\n",argv[1]);
#endif

	inputDir = getDir (argv[1]);
	inputExt = getExt (argv[1]);
	outputDir = getDir (argv[2]);
	outputExt = getExt (argv[2]);

	std::string center=getName(argv[1]).c_str();
	weldZones(center.c_str());
	
#if WELD_LOG
	fclose(fdbg);
#endif

	return 0;
}


