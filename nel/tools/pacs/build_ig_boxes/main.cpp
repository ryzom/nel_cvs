/** \file main.cpp
 *
 *
 * $Id: main.cpp,v 1.1 2001/11/28 15:54:46 corvazier Exp $
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

#include "nel/misc/aabbox.h"

#include "3d/register_3d.h"
#include "3d/scene_group.h"
#include "3d/transform_shape.h"
#include "3d/water_model.h"
#include "3d/water_shape.h"

#include <string>
#include <map>
#include <deque>

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
		int			i;
	
		cf.load("build_ig_boxes.cfg");
	
		Output = getString(cf, "Output");
		IGPath = getString(cf, "IGPath");

		CConfigFile::CVar &cvIGs = cf.getVar("IGs");
		for (i=0; i<cvIGs.size(); i++)
		{
			IGs.push_back(cvIGs.asString(i));
		}

		CPath::addSearchPath(IGPath);

		nlinfo("Output=%s", Output.c_str());
		nlinfo("IGPath=%s", IGPath.c_str());
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

			// search in group for water instance
			for (j=0; j<ig._InstancesInfos.size(); ++j)
			{
				// c'est degueulasse, mais c'est les coders a la 3D, y savent pas coder
				CIFile			f(CPath::lookup(ig._InstancesInfos[j].Name+".shape"));
				CShapeStream	shape;
				shape.serial(f);

				CWaterShape	*wshape = dynamic_cast<CWaterShape *>(shape.getShapePointer());
				if (wshape == NULL)
					continue;

				CPolygon			wpoly;
				wshape->getShapeInWorldSpace(wpoly);

				for (k=0; k<wpoly.Vertices.size(); ++k)
				{
					if (boxSet)
					{
						igBBox.extend(wpoly.Vertices[k]);
					}
					else
					{
						igBBox.setCenter(wpoly.Vertices[k]);
						boxSet = true;
					}
				}
			}

			Boxes.push_back(CIGBox(igName, igBBox));
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
