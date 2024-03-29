/** \file main.cpp
 *
 *
 * $Id: main.cpp,v 1.7 2007/03/19 09:55:28 boucher Exp $
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
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/displayer.h"
#include "nel/misc/file.h"
#include "nel/misc/matrix.h"

#include "nel/misc/aabbox.h"

#include "nel/../../src/3d/register_3d.h"
#include "nel/../../src/3d/scene_group.h"
#include "nel/../../src/3d/transform_shape.h"
#include "nel/../../src/3d/water_model.h"
#include "nel/../../src/3d/water_shape.h"

#include <string>
#include <map>
#include <deque>
#include <set>

using namespace std;
using namespace NLMISC;
using namespace NL3D;



string												Output;
string												IGPath;
vector<string>										IGs;

class CIGBox
{
public:
	CIGBox() {}
	CIGBox(const string &name, const CAABBox &bbox) : Name(name), BBox(bbox) {}
	string			Name;
	CAABBox			BBox;
	void			serial(IStream &f) { f.serial(Name, BBox); }
};

vector<CIGBox>										Boxes;
set<string>											NonWaterShapes;


//

int		getInt(CConfigFile &cf, const string &varName)
{
	CConfigFile::CVar &var = cf.getVar(varName);
	return var.asInt();
}

string	getString(CConfigFile &cf, const string &varName)
{
	CConfigFile::CVar &var = cf.getVar(varName);
	return var.asString();
}

void	init()
{
	registerSerial3d();

	try
	{
		CConfigFile cf;
		uint			i;
	
		cf.load("build_ig_boxes.cfg");
	
		Output = getString(cf, "Output");
		// nlinfo("Output=%s", Output.c_str());

		CConfigFile::CVar &cvIGs = cf.getVar("IGs");
		for (i=0; i<cvIGs.size(); i++)
		{
			IGs.push_back(cvIGs.asString(i));
		}

		CConfigFile::CVar &cvPathes = cf.getVar("Pathes");
		for (i=0; i<cvPathes.size(); ++i)
		{
			nlinfo("Using search path %s", cvPathes.asString(i).c_str());
			CPath::addSearchPath(cvPathes.asString(i));
		}
	}
	catch (EConfigFile &e)
	{
		printf ("Problem in config file : %s\n", e.what ());
	}
}

//
CFileDisplayer fd("evallog.log", true);

int main(int argc, char **argv)
{
	// Filter addSearchPath
	NLMISC::createDebug();
	InfoLog->addNegativeFilter("adding the path");

	createDebug();

	try
	{
		// Init
		init();

		uint	i, j, k;

		for (i=0; i<IGs.size(); ++i)
		{
			// load ig associated to the zone
			string			igName = IGs[i]+".ig";
			CIFile			igStream(CPath::lookup(igName));
			CInstanceGroup	ig;
			igStream.serial(ig);

			CAABBox			igBBox;
			bool			boxSet = false;

			nlinfo("Generating BBOX for %s", igName.c_str());

			// search in group for water instance
			for (j=0; j<ig._InstancesInfos.size(); ++j)
			{
				/*
				   Ben: c'est degueulasse, mais c'est les coders a la 3D, y savent pas coder
				   Hld: ouai, mais ca marche pas ton truc, alors p'tet qu'on sait pas coder mais toi non plus :p Special Dedicace to SupaGreg!
				string	shapeName = ig._InstancesInfos[j].Name+".shape";
				*/
				string	shapeName = ig._InstancesInfos[j].Name;
				if (CFile::getExtension (shapeName) == "")
					shapeName += ".shape";

				if (NonWaterShapes.find(shapeName) != NonWaterShapes.end())
					continue;

				string	shapeNameLookup = CPath::lookup (shapeName, false, false);
				if (!shapeNameLookup.empty())
				{
					CIFile			f;
					if (f.open (shapeNameLookup))
					{
						CShapeStream	shape;
						shape.serial(f);

						CWaterShape	*wshape = dynamic_cast<CWaterShape *>(shape.getShapePointer());
						if (wshape == NULL)
						{
							NonWaterShapes.insert(shapeName);
							continue;
						}

						CMatrix	matrix;
						ig.getInstanceMatrix(j, matrix);

						CPolygon			wpoly;
						wshape->getShapeInWorldSpace(wpoly);

						for (k=0; k<wpoly.Vertices.size(); ++k)
						{
							if (boxSet)
							{
								igBBox.extend(matrix * wpoly.Vertices[k]);
							}
							else
							{
								igBBox.setCenter(matrix * wpoly.Vertices[k]);
								boxSet = true;
							}
						}
					}
					else
					{
						nlwarning ("Can't load shape %s", shapeNameLookup.c_str());
					}
				}
				else
				{
					NonWaterShapes.insert(shapeName);
				}
			}

			if (boxSet)
			{
				Boxes.push_back(CIGBox(igName, igBBox));
				nlinfo("Bbox: (%.1f,%.1f)-(%.1f,%.1f)", igBBox.getMin().x, igBBox.getMin().y, igBBox.getMax().x, igBBox.getMax().y);
			}
		}

		COFile	output(Output);
		output.serialCont(Boxes);
	}
	catch (Exception &e)
	{
		fprintf (stderr,"main trapped an exception: '%s'\n", e.what ());
	}
#ifndef NL_DEBUG
	catch (...)
	{
		fprintf(stderr,"main trapped an unknown exception\n");
	}
#endif // NL_DEBUG

	return 0;
}
