/** \file moulinette.cpp
 *
 *
 * $Id: build_rbank.cpp,v 1.8 2002/12/17 16:23:04 legros Exp $
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


#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/polygon.h"
#include "nel/misc/smart_ptr.h"

#include "3d/scene_group.h"
#include "3d/transform_shape.h"
#include "3d/water_model.h"
#include "3d/water_shape.h"
#include "3d/quad_grid.h"

#include "build_rbank.h"
#include "build_surf.h"

#include "surface_splitter.h"

#include "pacs/global_retriever.h"
#include "pacs/retriever_bank.h"
#include "pacs/surface_quad.h"
#include "pacs/local_retriever.h"
#include "pacs/retriever_instance.h"
#include "pacs/chain.h"

#include <string>
#include <deque>

using namespace std;
using namespace NLMISC;
using namespace NL3D;


class CIGBox
{
public:
	CIGBox() {}
	CIGBox(const string &name, const CAABBox &bbox) : Name(name), BBox(bbox) {}
	string			Name;
	CAABBox			BBox;
	void			serial(IStream &f) { f.serial(Name, BBox); }
};

/*
string getZoneNameById(uint16 id)
{
	uint	x = id%256;
	uint	y = id/256;

	char ych[32];
	sprintf(ych,"%d_%c%c", y+1, 'A'+x/26, 'A'+x%26);
	return string(ych);
}
*/
string getZoneNameByCoord(float x, float y)
{
	const float zoneDim = 160.0f;

	float xcount = x/zoneDim;
	float ycount = -y/zoneDim + 1;

	char ych[32];
	sprintf(ych,"%d_%c%c",(sint)ycount, 'A'+(sint)xcount/26, 'A'+(sint)xcount%26);
	return string(ych);
}

CVector	getZoneCenterById(uint16 id)
{
	CAABBox		bbox;
	uint		x, y;
	const float	zdim = 160.0f;

	x = id%256;
	y = id/256;

	return CVector(zdim*((float)x+0.5f), -zdim*((float)y+0.5f), 0.0f);
}

uint32	getIdByCoord(uint x, uint y)
{
	return y*256+x;
}

string	changeExt(string name, string &ext)
{
	string::iterator	it, last;
	last = name.end();

	for (it=name.begin(); it!=name.end(); ++it)
		if (*it == '.')
			last = it;

	name.erase(last, name.end());
	name.append(".");
	name.append(ext);
	return name;
}






void tessellateZone(string &zoneName)
{
	NLPACS::CZoneTessellation		tessellation;
	vector<NLPACS::COrderedChain3f>	fullChains;
	string							name;
	string							filename;

	try
	{
		uint16	zid = getZoneIdByName(zoneName);
		CAABBox	box = getZoneBBoxById(zid);

		CVector		translation = -box.getCenter();
		if (tessellation.setup(zid, 4, translation))
		{
			tessellation.build();

			COFile	tesselOutput;
			name = changeExt(zoneName, string("tessel"));
			filename = TessellationPath+name;
			tesselOutput.open(filename);
			tessellation.saveTessellation(tesselOutput);
			tesselOutput.close();
		}
	}
	catch(Exception &e)
	{
		printf(e.what ());
	}
}

void moulineZone(string &zoneName)
{
	uint	i, j;

	NLPACS::CZoneTessellation		tessellation;
	vector<NLPACS::COrderedChain3f>	fullChains;
	string							name;
	string							filename;

	try
	{
		uint16	zid = getZoneIdByName(zoneName);
		CAABBox	box = getZoneBBoxById(zid);

		CVector		translation = -box.getCenter();

		// if can't setup tessellation, don't mouline zone (no .lr created)
		if (tessellation.setup(zid, 4, translation))
		{
			CIFile	tesselInput;
			name = changeExt(zoneName, string("tessel"));
			filename = TessellationPath+name;
			tesselInput.open(filename);
			tessellation.loadTessellation(tesselInput);
			tesselInput.close();

			CAABBox	tbox = tessellation.computeBBox();

			vector<CIGBox>				boxes;
			try
			{
				if (CFile::fileExists (IGBoxes))
				{
					CIFile		binput(IGBoxes);
					binput.serialCont(boxes);
				}
				else
				{
					nlinfo("WARNING: IG list no found");
				}
			}
			catch (Exception &) { nlinfo("WARNING: IG list no found"); }

			for (i=0; i<boxes.size(); ++i)
			{
				if (tbox.intersect(boxes[i].BBox))
				{
					try
					{
						// load ig associated to the zone
						string	igname = boxes[i].Name;
						CIFile			monStream(CPath::lookup(igname));
						CInstanceGroup	ig;
						monStream.serial(ig);

						// search in group for water instance
						for (j=0; j<ig._InstancesInfos.size(); ++j)
						{
							// c'est degueulasse, mais c'est les coders a la 3D, y savent pas coder
							CIFile			monfile(CPath::lookup(ig._InstancesInfos[j].Name+".shape"));
							CShapeStream	shape;
							shape.serial(monfile);

							CWaterShape	*wshape = dynamic_cast<CWaterShape *>(shape.getShapePointer());
							if (wshape == NULL)
								continue;

							CPolygon			wpoly;
							wshape->getShapeInWorldSpace(wpoly);

							tessellation.addWaterShape(wpoly);
						}
					}
					catch (Exception &e)
					{
						nlwarning("%s", e.what());
					}
				}
			}

			tessellation.compile();
			tessellation.generateBorders(1.0);
			tessellation.generateStats();

			NLPACS::CLocalRetriever	retriever;

			CAABBox	rbbox = tessellation.BestFittingBBox;
			CVector hs = rbbox.getHalfSize();
			hs.z = 10000.0f;
			rbbox.setHalfSize(hs);
			retriever.setBBox(rbbox);
			retriever.setType(NLPACS::CLocalRetriever::Landscape);

			for (j=0; j<(sint)tessellation.Surfaces.size(); ++j)
			{
				retriever.addSurface(tessellation.Surfaces[j].NormalQuanta,
									 tessellation.Surfaces[j].OrientationQuanta,
									 tessellation.Surfaces[j].Material,
									 tessellation.Surfaces[j].Character,
									 tessellation.Surfaces[j].Level,
									 tessellation.Surfaces[j].IsUnderWater,
									 tessellation.Surfaces[j].WaterHeight,
									 tessellation.Surfaces[j].Center,
									 tessellation.Surfaces[j].HeightQuad);
			}

			for (j=0; j<(sint)tessellation.Borders.size(); ++j)
			{
				if (tessellation.Borders[j].Right < -1)
				{
					retriever.addChain(tessellation.Borders[j].Vertices,
									   tessellation.Borders[j].Left,
									   NLPACS::CChain::getDummyBorderChainId());

				}
				else
				{
					retriever.addChain(tessellation.Borders[j].Vertices,
									   tessellation.Borders[j].Left,
									   tessellation.Borders[j].Right);
				}
			}

			fullChains = retriever.getFullOrderedChains();

			// save raw retriever
			COFile	outputRetriever;
			name = changeExt(zoneName, string("lr"));
			filename = OutputPath+PreprocessDirectory+name;
			nldebug("save file %s", filename.c_str());
			outputRetriever.open(filename);
			retriever.serial(outputRetriever);

			// save raw chains
			COFile	outputChains;
			name = changeExt(zoneName, string("ochain"));
			filename = OutputPath+name;
			nldebug("save file %s", filename.c_str());
			outputChains.open(filename);
			outputChains.serialCont(fullChains);
		}
	}
	catch(Exception &e)
	{
		printf(e.what ());
	}
}


void processRetriever(string &zoneName)
{
	string							name;
	string							filename;

	try
	{
		uint16	zid = getZoneIdByName(zoneName);
		CAABBox	box = getZoneBBoxById(zid);

		NLPACS::CLocalRetriever	retriever;

		// load raw retriever
		CIFile	inputRetriever;
		name = changeExt(zoneName, string("lr"));
		filename = OutputPath+PreprocessDirectory+name;
		nlinfo("load file %s", filename.c_str());

		if (CFile::fileExists(filename))
		{
			inputRetriever.open(filename);
			retriever.serial(inputRetriever);

			// compute the retriever

			retriever.computeLoopsAndTips();

			retriever.findBorderChains();
			retriever.updateChainIds();
			retriever.computeTopologies();

			retriever.computeCollisionChainQuad();

			retriever.setType(NLPACS::CLocalRetriever::Landscape);

			//
			CSurfaceSplitter	splitter;
			splitter.build(retriever);

			// and save it...

			COFile	outputRetriever;
			name = changeExt(zoneName, string("lr"));
			filename = OutputPath+name;
			nlinfo("save file %s", filename.c_str());
			outputRetriever.open(filename);
			retriever.serial(outputRetriever);
		}
	}
	catch(Exception &e)
	{
		printf(e.what ());
	}
}


void	processGlobalRetriever()
{
	NLPACS::CRetrieverBank		retrieverBank;
	NLPACS::CGlobalRetriever	globalRetriever;

	uint						ULid = getZoneIdByName(GlobalUL),
								DRid = getZoneIdByName(GlobalDR);
	
	CAABBox						ULbbox = getZoneBBoxById(ULid);
	CAABBox						DRbbox = getZoneBBoxById(DRid);
	CAABBox						bbox;

	CVector						vmin, vmax;

	vmin.minof(ULbbox.getMin(), DRbbox.getMin());
	vmax.maxof(ULbbox.getMax(), DRbbox.getMax());
	bbox.setMinMax(vmin, vmax);

	uint						x0 = ULid%256, 
								y0 = ULid/256,
								x1 = DRid%256, 
								y1 = DRid/256;
	
	globalRetriever.setRetrieverBank(&retrieverBank);
	globalRetriever.init();

	uint						x, y;

	nlinfo("make all instances");

	for (y=y0; y<=y1; ++y)
	{
		for (x=x0; x<=x1; ++x)
		{
			try
			{
				string filename = OutputPath+getZoneNameById(x+y*256)+".lr";
				if (CFile::fileExists (filename))
				{
					uint	retrieverId = retrieverBank.addRetriever(filename);
					globalRetriever.makeInstance(retrieverId, 0, getZoneCenterById((uint16)getIdByCoord(x, y))); 
				}
			}
			catch (Exception &e)
			{
				printf(e.what ());
			}
		}
	}

	nlinfo("init the quad grid");
	globalRetriever.initQuadGrid();

	nlinfo("make all links");
	globalRetriever.makeAllLinks();

	nlinfo("clean retriever bank up");
//	retrieverBank.clean();

	const vector<NLPACS::CRetrieverInstance>	&instances = globalRetriever.getInstances();
	uint	i, j;
	uint	totalUnlinked = 0, totalLink = 0;
	for (i=0; i<instances.size(); ++i)
	{
		const vector<NLPACS::CRetrieverInstance::CLink>	&links = instances[i].getBorderChainLinks();
		CVector	pos = instances[i].getBBox().getCenter();
		string	unlinkstr = "instance "+toString(i)+":"+getZoneNameById(getZoneIdByPos(pos))+":";
		bool	unlinkerr = false;
		for (j=0; j<links.size(); ++j)
		{
			++totalLink;
			if (links[j].Instance == 0xffff)
			{
				unlinkstr += (string(" ")+toString(j));
				++totalUnlinked;
				unlinkerr = true;
			}
		}
		if (unlinkerr)
			nlinfo("unlink: %s", unlinkstr.c_str());
	}

	nlinfo("%d are still unlinked (%d links total)", totalUnlinked, totalLink);

	string	filename;

	COFile	outputRetriever;
	filename = OutputPath+GlobalRetriever;
	nlinfo("save file %s", filename.c_str());
	outputRetriever.open(filename);
	globalRetriever.serial(outputRetriever);

	COFile	outputBank;
	filename = OutputPath+RetrieverBank;
	nlinfo("save file %s", filename.c_str());
	outputBank.open(filename);
	retrieverBank.serial(outputBank);

	//retrieverBank.saveRetrievers(OutputPath, CFile::getFilenameWithoutExtension(RetrieverBank));
}

///

void	updateRetrieverBank()
{
	NLPACS::CRetrieverBank		retrieverBank;

	string	filename;
	filename = OutputPath+RetrieverBank;

	CIFile	inputBank;
	nlinfo("load file %s", filename.c_str());
	inputBank.open(filename);
	retrieverBank.serial(inputBank);
	inputBank.close();

	COFile	outputBank;
	nlinfo("save file %s", filename.c_str());
	outputBank.open(filename);
	retrieverBank.serial(outputBank);
	outputBank.close();
}

