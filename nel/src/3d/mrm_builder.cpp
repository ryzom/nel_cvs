/** \file mrm_builder.cpp
 * <File description>
 *
 * $Id: mrm_builder.cpp,v 1.4 2001/01/02 14:23:14 lecroart Exp $
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

#include "nel/3d/mrm_builder.h"
using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// Tools Methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static bool	findElement(vector<sint>	&array, sint elt)
{
	return find(array.begin(), array.end(), elt) != array.end();
}
// ***************************************************************************
static bool	deleteElement(vector<sint>	&array, sint elt)
{
	bool	found=false;
	vector<sint>::iterator	it=array.begin();

	while(  (it=find(array.begin(), array.end(), elt)) != array.end() )
		found=true, array.erase(it);

	return found;
	// Must not use remove<> since it do not modify size ... (???)
	// This seems not to work.
	//return remove(array.begin(), array.end(), elt)!=array.end();
}


// ***************************************************************************
// ***************************************************************************
// Edge Cost methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMRMBuilder::vertexHasOneWedge(sint numvertex)
{
	CMRMVertex	&vert= TmpVertices[numvertex];
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		sint	numwedge=-1;
		for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
		{
			sint	w= TmpFaces[vert.SharedFaces[i]].getAssociatedWedge(attId, numvertex);
			if(numwedge>=0 && numwedge!=w)	return false;
			else	numwedge=w;
		}
	}
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::vertexHasOneMaterial(sint numvertex)
{
	sint	matId=-1;
	CMRMVertex	&vert= TmpVertices[numvertex];
	for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		sint	m= TmpFaces[vert.SharedFaces[i]].MaterialId;
		if(matId>=0 && matId!=m)	return false;
		else	matId=m;
	}
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::vertexContinue(sint numvertex)
{
	return vertexHasOneWedge(numvertex) && vertexHasOneMaterial(numvertex);
}
// ***************************************************************************
bool	CMRMBuilder::vertexClosed(sint numvertex)
{
	CMRMVertex	&vert= TmpVertices[numvertex];
	map<CMRMEdge, sint>		EdgeShare;
	// Init to 0.
	sint i;
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		EdgeShare[f.getEdge(0)]= 0;
		EdgeShare[f.getEdge(1)]= 0;
		EdgeShare[f.getEdge(2)]= 0;
	}
	// Inc count.
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		EdgeShare[f.getEdge(0)]++;
		EdgeShare[f.getEdge(1)]++;
		EdgeShare[f.getEdge(2)]++;
	}
	// Test open edges.
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		sint	v0= f.Corner[0].Vertex;
		sint	v1= f.Corner[1].Vertex;
		sint	v2= f.Corner[2].Vertex;
		if(EdgeShare[f.getEdge(0)]<2 && (v0==numvertex || v1==numvertex)) return false;
		if(EdgeShare[f.getEdge(1)]<2 && (v1==numvertex || v2==numvertex)) return false;
		if(EdgeShare[f.getEdge(2)]<2 && (v0==numvertex || v2==numvertex)) return false;
	}
	return true;
}
// ***************************************************************************
float	CMRMBuilder::getDeltaFaceNormals(sint numvertex)
{
	// return a positive value of Somme(|DeltaNormals|) / NNormals.
	CMRMVertex	&vert= TmpVertices[numvertex];
	float	delta=0;
	CVector	refNormal;
	sint	nfaces=vert.SharedFaces.size();
	for(sint i=0;i<nfaces;i++)
	{
		CVector	normal;
		CVector	&v0= TmpVertices[TmpFaces[i].Corner[0].Vertex].Current;
		CVector	&v1= TmpVertices[TmpFaces[i].Corner[1].Vertex].Current;
		CVector	&v2= TmpVertices[TmpFaces[i].Corner[2].Vertex].Current;
		normal= (v1-v0)^(v2-v0);
		normal.normalize();
		if(i==0)
			refNormal=normal;
		else
			delta+=(1-refNormal*normal);
	}
	if(nfaces<2)
		return 0;
	else
		return delta/(nfaces-1);
}
// ***************************************************************************
bool	CMRMBuilder::edgeContinue(const CMRMEdge &edge)
{
	sint v0= edge.v0;
	sint v1= edge.v1;
	CMRMVertex	&Vertex1=TmpVertices[v0], &Vertex2=TmpVertices[v1];

	// build list sharing edge.
	vector<sint>	deletedFaces;
	sint i;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].hasVertex(v1))
			deletedFaces.push_back(numFace);
	}

	sint	matId=-1;
	// test if faces have same material.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		sint	m;
		m= TmpFaces[deletedFaces[i]].MaterialId;
		if(matId>=0 && matId!=m)	return false;
		else	matId=m;
	}

	// test if faces have same wedge (for all att).
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		sint	numwedge1=-1,numwedge2=-1;
		for(i=0;i<(sint)deletedFaces.size();i++)
		{
			sint	w;
			w= TmpFaces[deletedFaces[i]].getAssociatedWedge(attId, v0);
			if(numwedge1>=0 && numwedge1!=w)	return false;
			else	numwedge1=w;
			w= TmpFaces[deletedFaces[i]].getAssociatedWedge(attId, v1);
			if(numwedge2>=0 && numwedge2!=w)	return false;
			else	numwedge2=w;
		}
	}

	return true;
}
// ***************************************************************************
bool	CMRMBuilder::edgeNearUniqueMatFace(const CMRMEdge &edge)
{
	sint v0= edge.v0;
	sint v1= edge.v1;
	CMRMVertex	&Vertex1=TmpVertices[v0], &Vertex2=TmpVertices[v1];

	// build list sharing edge.
	vector<sint>	deletedFaces;
	sint i;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].hasVertex(v1))
			deletedFaces.push_back(numFace);
	}

	// test if faces are not isolated OneMaterial faces.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		CMRMFaceBuild	&f=TmpFaces[deletedFaces[i]];
		if( !edgeContinue(f.getEdge(0)) && 
			!edgeContinue(f.getEdge(1)) && 
			!edgeContinue(f.getEdge(2)))
			return true;
	}

	return false;
}

// ***************************************************************************
float	CMRMBuilder::computeEdgeCost(const CMRMEdge &edge)
{
	sint	v1= edge.v0;
	sint	v2= edge.v1;
	// more expensive is the edge, later it will collapse.
	float	cost=(TmpVertices[v1].Current-TmpVertices[v2].Current).norm();

	float	faceCost= (getDeltaFaceNormals(v1)+getDeltaFaceNormals(v2));
	// Must minimize.
	faceCost= max(faceCost, 0.01f);
	cost*= faceCost;

	// Like hope, add a weight on discontinuities..
	if( !vertexContinue(v1) && !vertexContinue(v2) )
	{
		// Nb: don't do this on discontinuities edges, unless the unique material face will collapse (fiou!!).
		if( edgeContinue(edge) || edgeNearUniqueMatFace(edge) )
			cost*=4;
	}

	return cost;
}




// ***************************************************************************
// ***************************************************************************
// Collapse Methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMRMBuilder::faceShareWedges(CMRMFaceBuild *face, sint attribId, sint numVertex1, sint numVertex2)
{
	sint	numWedge1= face->getAssociatedWedge(attribId, numVertex1);
	sint	numWedge2= face->getAssociatedWedge(attribId, numVertex2);
	if(numWedge1<0) return false;
	if(numWedge2<0) return false;

	CMRMAttribute	&w1= TmpAttributes[attribId][numWedge1];
	CMRMAttribute	&w2= TmpAttributes[attribId][numWedge2];
	return w1.Shared && w2.Shared && w1.NbSharedFaces>0 && w2.NbSharedFaces>0;
}


// ***************************************************************************
void	CMRMBuilder::insertFaceIntoEdgeList(CMRMFaceBuild &f)
{
	float	len;
	if(f.ValidIt0)
	{
		len= computeEdgeCost(f.getEdge(0));
		f. It0= EdgeCollapses.insert( TEdgeMap::value_type( len, CMRMEdgeFace(f.getEdge(0),&f) ) );
	}
	if(f.ValidIt1)
	{
		len= computeEdgeCost(f.getEdge(1));
		f. It1= EdgeCollapses.insert( TEdgeMap::value_type( len, CMRMEdgeFace(f.getEdge(1),&f) ) );
	}
	if(f.ValidIt2)
	{
		len= computeEdgeCost(f.getEdge(2));
		f. It2= EdgeCollapses.insert( TEdgeMap::value_type( len, CMRMEdgeFace(f.getEdge(2),&f) ) );
	}
}
// ***************************************************************************
void	CMRMBuilder::removeFaceFromEdgeList(CMRMFaceBuild &f)
{
	if(f.ValidIt0)
		EdgeCollapses.erase(f.It0);
	if(f.ValidIt1)
		EdgeCollapses.erase(f.It1);
	if(f.ValidIt2)
		EdgeCollapses.erase(f.It2);
}
// ***************************************************************************
sint	CMRMBuilder::collapseEdge(const CMRMEdge &edge)
{
	sint	i,j;
	float	InterValue;
	sint	edgeV1=edge.v0;
	sint	edgeV2=edge.v1;

	// 0. collapse the vertices.
	//==========================

	// edge.Vertex1 kept, but morphed.
	// edge.Vertex2 deleted, and must know on which vertex it collapse.
	CMRMVertex	&Vertex1=TmpVertices[edgeV1], &Vertex2=TmpVertices[edgeV2];

	// Interpolation choice.
	InterValue=0.5;
	//InterValue=1;
	bool	vc1= vertexClosed(edgeV1);
	bool	vc2= vertexClosed(edgeV2);
	if(!vc1 && vc2) InterValue=0;
	else if(vc1 && !vc2) InterValue=1;
	else
	{
		bool	vc1= vertexContinue(edgeV1);
		bool	vc2= vertexContinue(edgeV2);
		if(!vc1 && vc2) InterValue=0;
		if(vc1 && !vc2) InterValue=1;
	}
	/*BENCH_TotalCollapses++;
	if(InterValue==0.5)
		BENCH_MiddleCollapses++;*/

	// let's interpolate.
	Vertex1.Current= Vertex1.Current*(1-InterValue) + Vertex2.Current*InterValue;
	Vertex2.CollapsedTo= edgeV1;


	// TODO_BUG: Don't know why, but vertices may point on deleted faces.
	// Temp: we destroy here thoses face from SharedFaces...
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].Deleted)
			deleteElement(Vertex1.SharedFaces, numFace), i--;
	}
	for(i=0;i<(sint)Vertex2.SharedFaces.size();i++)
	{
		sint	numFace= Vertex2.SharedFaces[i];
		if(TmpFaces[numFace].Deleted)
			deleteElement(Vertex2.SharedFaces, numFace), i--;
	}


	// Build Neighbor faces.
	vector<sint>	neighboorFaces;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(!findElement(neighboorFaces, numFace))
			neighboorFaces.push_back(numFace);
	}
	for(i=0;i<(sint)Vertex2.SharedFaces.size();i++)
	{
		sint	numFace= Vertex2.SharedFaces[i];
		if(!findElement(neighboorFaces, numFace))
			neighboorFaces.push_back(numFace);
	}

	// Build faces which will be destroyed (may 1 or 2, maybe more for non conventionnal meshes).
	vector<sint>	deletedFaces;
	for(i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		nlassert(!TmpFaces[numFace].Deleted);
		if(TmpFaces[numFace].hasVertex(edgeV2))
			deletedFaces.push_back(numFace);
	}


	// 1. Collapse the wedges.
	//========================

	// For ALL Attributes.
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		// a/ Stock the wedge interpolation each destroyed face.
		//------------------------------------------------------
		for(i=0;i<(sint)deletedFaces.size();i++)
		{
			CMRMFaceBuild		&face= TmpFaces[deletedFaces[i]];

			CVectorH	&w0= TmpAttributes[attId][ face.getAssociatedWedge(attId, edgeV1) ].Current;
			CVectorH	&w1= TmpAttributes[attId][ face.getAssociatedWedge(attId, edgeV2) ].Current;

			CVectorH	&itp= face.InterpolatedAttributes[attId];
			itp.x= w0.x*(1-InterValue) + w1.x*InterValue;
			itp.y= w0.y*(1-InterValue) + w1.y*InterValue;
			itp.z= w0.z*(1-InterValue) + w1.z*InterValue;
			itp.w= w0.w*(1-InterValue) + w1.w*InterValue;
		}


		// b/ Build wedge list to be modify.
		//----------------------------------
		vector<sint>	wedges;

		for(i=0;i<(sint)neighboorFaces.size();i++)
		{
			CMRMFaceBuild	&face= TmpFaces[neighboorFaces[i]];
			sint	numWedge;

			numWedge= face.getAssociatedWedge(attId, edgeV1);
			if(numWedge>=0 && !findElement(wedges, numWedge))
				wedges.push_back(numWedge);

			numWedge= face.getAssociatedWedge(attId, edgeV2);
			if(numWedge>=0 && !findElement(wedges, numWedge))
				wedges.push_back(numWedge);
		}


		// c/ Count numFaces which point on those wedges. (- deleted faces).
		//------------------------------------------------------------------

		for(i=0;i<(sint)wedges.size();i++)
		{
			sint			numWedge= wedges[i];
			CMRMAttribute	&wedge= TmpAttributes[attId][numWedge];

			wedge.NbSharedFaces=0;
			wedge.Shared=false;

			// Count total ref count.
			for(j=0;j<(sint)neighboorFaces.size();j++)
			{
				if(TmpFaces[neighboorFaces[j]].hasWedge(attId, numWedge))
					wedge.NbSharedFaces++;
			}

			// Minus deleted faces.
			for(j=0;j<(sint)deletedFaces.size();j++)
			{
				if(TmpFaces[deletedFaces[j]].hasWedge(attId, numWedge))
				{
					wedge.NbSharedFaces--;
					wedge.Shared=true;
					wedge.InterpolatedFace=deletedFaces[j];
				}
			}
		}


		// d/ Collapse wedge following 3 possibles cases.
		//-----------------------------------------------


		for(i=0;i<(sint)wedges.size();i++)
		{
			sint			numWedge= wedges[i];
			CMRMAttribute	&wedge= TmpAttributes[attId][numWedge];

			// si le wedge n'est pas partagé...
			if(!wedge.Shared)
			{
				// We've got an "exterior wedge" which lost no corner => do not merge it nor delete it. 
				// Leave it as the same value (extrapolate it may not be a good solution).
			}
			else
			{
				// if wedge dissapears, notify.
				if(wedge.NbSharedFaces==0)
				{
					wedge.CollapsedTo=-2;
					// Do not change his value. (as specified in Hope article).
				}
				else
				{
					CMRMFaceBuild	&face= TmpFaces[wedge.InterpolatedFace];

					// Must interpolate it.
					wedge.Current= face.InterpolatedAttributes[attId];

					// Must merge the wedge of the second vertex on first
					// ONLY IF 2 interpolated wedges are shared and NbSharedFaces!=0.
					if(	numWedge==face.getAssociatedWedge(attId, edgeV2) && 
						faceShareWedges(&face, attId, edgeV1, edgeV2) )
					{
						wedge.CollapsedTo= face.getAssociatedWedge(attId, edgeV1);
					}
				}
			}
		}

	}

	// 3. collapse faces.
	//===================
	
	// delete face shared by edge.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		sint	numFace= deletedFaces[i];
		TmpFaces[numFace].Deleted=true;

		// release edges from list.
		removeFaceFromEdgeList(TmpFaces[numFace]);
		// ivalid all it!!
		TmpFaces[numFace].invalidAllIts();


		// delete from vertex1 and 2 the deleted faces.
		deleteElement( Vertex1.SharedFaces, numFace);
		deleteElement( Vertex2.SharedFaces, numFace);
	}


	// must ref correctly the faces.
	for(i=0;i<(sint)neighboorFaces.size();i++)
	{
		CMRMFaceBuild		&face=TmpFaces[neighboorFaces[i]];

		// good vertices
		if(face.Corner[0].Vertex ==edgeV2)	face.Corner[0].Vertex=edgeV1;
		if(face.Corner[1].Vertex ==edgeV2)	face.Corner[1].Vertex=edgeV1;
		if(face.Corner[2].Vertex ==edgeV2)	face.Corner[2].Vertex=edgeV1;
		// nb: doesn't matter if deletedFaces are modified...

		// good wedges
		for(sint attId=0;attId<NumAttributes;attId++)
		{
			sint	newWedge;
			newWedge= TmpAttributes[attId][ face.Corner[0].Attributes[attId] ].CollapsedTo;
			if(newWedge>=0)	face.Corner[0].Attributes[attId]= newWedge;
			newWedge= TmpAttributes[attId][ face.Corner[1].Attributes[attId] ].CollapsedTo;
			if(newWedge>=1)	face.Corner[1].Attributes[attId]= newWedge;
			newWedge= TmpAttributes[attId][ face.Corner[2].Attributes[attId] ].CollapsedTo;
			if(newWedge>=2)	face.Corner[2].Attributes[attId]= newWedge;
		}
	}


	// The vertex1 has now the shared env of vertex2.
	Vertex1.SharedFaces.insert(Vertex1.SharedFaces.end(), Vertex2.SharedFaces.begin(), 
		Vertex2.SharedFaces.end());
	

	return deletedFaces.size();
}


// ***************************************************************************
sint	CMRMBuilder::followVertex(sint i)
{
	CMRMVertex	&vert=TmpVertices[i];
	if(vert.CollapsedTo>=0)
		return followVertex(vert.CollapsedTo);
	else
		return i;
}
// ***************************************************************************
sint	CMRMBuilder::followWedge(sint attribId, sint i)
{
	CMRMAttribute	&wedge= TmpAttributes[attribId][i];
	if(wedge.CollapsedTo>=0)
		return followWedge(attribId, wedge.CollapsedTo);
	else
		return i;
}


// ***************************************************************************
// ***************************************************************************
// Mesh Level method.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMRMBuilder::CMRMBuilder()
{
	NumAttributes= 0;
}

// ***************************************************************************
void	CMRMBuilder::init(const CMRMMesh &baseMesh)
{
	sint	i, attId;


	// First clear ALL.
	TmpVertices.clear();
	for(attId=0;attId<NL3D_MRM_MAX_ATTRIB;attId++)
	{
		TmpAttributes[attId].clear();
	}
	TmpFaces.clear();
	EdgeCollapses.clear();


	// resize.
	NumAttributes= baseMesh.NumAttributes;
	TmpVertices.resize(baseMesh.Vertices.size());
	for(attId=0;attId<NumAttributes;attId++)
	{
		TmpAttributes[attId].resize(baseMesh.Attributes[attId].size());
	}
	TmpFaces.resize(baseMesh.Faces.size());


	// Then copy.
	for(i=0;i<(sint)baseMesh.Vertices.size();i++)
		TmpVertices[i].Current= TmpVertices[i].Original= baseMesh.Vertices[i];
	for(attId=0;attId<NumAttributes;attId++)
	{
		for(i=0;i<(sint)baseMesh.Attributes[attId].size();i++)
			TmpAttributes[attId][i].Current= TmpAttributes[attId][i].Original= 
			baseMesh.Attributes[attId][i];
	}
	for(i=0;i<(sint)baseMesh.Faces.size();i++)
		TmpFaces[i]= baseMesh.Faces[i];


	// Create vertices sharedFaces.
	for(i=0;i<(sint)TmpFaces.size();i++)
	{
		CMRMFaceBuild		&face= TmpFaces[i];

		TmpVertices[face.Corner[0].Vertex].SharedFaces.push_back(i);
		TmpVertices[face.Corner[1].Vertex].SharedFaces.push_back(i);
		TmpVertices[face.Corner[2].Vertex].SharedFaces.push_back(i);
	}


	// Compute EdgeCost.
	for(i=0;i<(sint)TmpFaces.size();i++)
	{
		CMRMFaceBuild		&f= TmpFaces[i];
		// At start, valid all edges.
		f. ValidIt0= true;
		f. ValidIt1= true;
		f. ValidIt2= true;
		insertFaceIntoEdgeList(f);
	}
}
// ***************************************************************************
void	CMRMBuilder::collapseEdges(sint nWantedFaces)
{
	ItEdgeMap		EdgeIt;

	sint	nCurrentFaces=TmpFaces.size();
	sint	bug0=0,bug1=0,bug2=0,bug3=0;

	while(nCurrentFaces>nWantedFaces)
	{
		bug0++;
		EdgeIt= EdgeCollapses.begin();

		if(EdgeIt== EdgeCollapses.end())
			break;

		// 0. Look if edge already deleted
		//================================
		CMRMEdge	edge=(*EdgeIt).second;

		// Is it valid?? (ie his vertices exist yet??).
		if(TmpVertices[ edge.v0 ].CollapsedTo>=0
			|| TmpVertices[ edge.v1 ].CollapsedTo>=0)
		{
			// TODO_BUG: potential bug here...
			CMRMFaceBuild		&f= *(EdgeIt->second.Face);
			nlassert(f.validEdgeIt(EdgeIt->second));
			f.invalidEdgeIt(EdgeIt->second);
			EdgeCollapses.erase(EdgeIt);
			bug2++;
			continue;
		}
		// TODO_BUG: potential bug here...
		// If a mesh is "open" it will crash if a "hole collapse"...
		if(edge.v0==edge.v1)
		{
			CMRMFaceBuild		&f= *(EdgeIt->second.Face);
			nlassert(f.validEdgeIt(EdgeIt->second));
			f.invalidEdgeIt(EdgeIt->second);
			EdgeCollapses.erase(EdgeIt);
			bug3++;
			continue;
		}


		// 1. else, OK, collapse it!!
		//===========================
		sint	vertexCollapsed= edge.v0;
		nCurrentFaces-= collapseEdge(edge);


		// 2. Must reorder all his neighborhood.
		//======================================
		CMRMVertex	&vert=TmpVertices[vertexCollapsed];
		sint	i;
		// On efface de la liste les edges modifies, et on les rajoute avec leur nouvelle valeur.
		for(i=0;i<(sint)vert.SharedFaces.size();i++)
		{
			CMRMFaceBuild		&f= TmpFaces[vert.SharedFaces[i]];
			removeFaceFromEdgeList(f);
			insertFaceIntoEdgeList(f);
		}

	}
}
// ***************************************************************************
void	CMRMBuilder::saveCoarserMesh(CMRMMesh &coarserMesh)
{
	sint	i,attId,index;
	// First clear ALL.
	coarserMesh.Vertices.clear();
	for(attId=0;attId<NL3D_MRM_MAX_ATTRIB;attId++)
	{
		coarserMesh.Attributes[attId].clear();
	}
	coarserMesh.Faces.clear();
	coarserMesh.NumAttributes= NumAttributes;

	// Vertices.
	//==========
	index=0;
	// Here, CollpasedTo is used to store the new indexation.
	for(i=0;i<(sint)TmpVertices.size();i++)
	{
		CMRMVertex	&vert=TmpVertices[i];
		if(vert.CollapsedTo==-1)	// if exist yet.
		{
			vert.CollapsedTo=index;
			coarserMesh.Vertices.push_back(vert.Current);
			index++;
		}
		else
			vert.CollapsedTo=-1;	// just for bug check. vertex no more exist.
	}


	// Attributes.
	//============
	// Here, CollpasedTo is used to store the new indexation.
	for(attId=0;attId<NumAttributes;attId++)
	{
		for(i=0;i<(sint)TmpAttributes[attId].size();i++)
		{
			CMRMAttribute	&wedge= TmpAttributes[attId][i];
			if(wedge.CollapsedTo==-1)	// if exist yet.
			{
				wedge.CollapsedTo=index;
				coarserMesh.Attributes[attId].push_back(wedge.Current);
				index++;
			}
			else
				wedge.CollapsedTo=-1;	// just for bug check. wedge no more exist.
		}
	}

	// Faces.
	//=======
	for(i=0;i<(sint)TmpFaces.size();i++)
	{
		CMRMFaceBuild	&face=TmpFaces[i];
		if(!face.Deleted)
		{
			CMRMFace	newFace;
			// Material.
			newFace.MaterialId= face.MaterialId;
			for(sint j=0;j<3;j++)
			{
				// Vertex.
				newFace.Corner[j].Vertex= TmpVertices[face.Corner[i].Vertex].CollapsedTo;
				// Attributes.
				for(attId=0;attId<NumAttributes;attId++)
				{
					sint	oldidx= face.Corner[i].Attributes[attId];
					newFace.Corner[j].Attributes[attId]= TmpAttributes[attId][oldidx].CollapsedTo;
				}

			}

			coarserMesh.Faces.push_back(newFace);
		}
	}

}



} // NL3D
