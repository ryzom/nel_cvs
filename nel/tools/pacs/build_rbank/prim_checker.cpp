/** \file prim_checker.cpp
 * <File description>
 *
 * $Id: prim_checker.cpp,v 1.2 2004/01/13 16:36:59 legros Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#include "prim_checker.h"

// NeL Misc includes
#include "nel/misc/vectord.h"
#include "nel/misc/path.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"

// NeL Ligo includes
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"

// STL includes
#include <vector>

using namespace NLMISC;
using namespace NLLIGO;
using namespace std;

NLLIGO::CLigoConfig LigoConfig;
extern bool						Verbose;

/*
 * Constructor
 */
CPrimChecker::CPrimChecker()
{
}



/*
 *		init()
 */
bool	CPrimChecker::init(const string &primitivesPath, const string &outputDirectory, bool forceRebuild)
{
	if (Verbose)
		nlinfo("Checking pacs.packed_prims consistency");

	NLLIGO::Register();

	// Init ligo
	if (!LigoConfig.readPrimitiveClass ("world_editor_classes.xml"))
	{
		// Should be in l:\leveldesign\world_edit_files
		nlwarning ("Can't load ligo primitive config file world_editor_classes.xml");
		return false;
	}

	vector<string>	files;
	CPath::getPathContent(primitivesPath, true, false, true, files);

	uint	i;
	string	outputfname = CPath::standardizePath(outputDirectory)+"pacs.packed_prims";

	forceRebuild |= (!CFile::fileExists(outputfname));
	uint	numPrims = 0;

	uint32	outputstamp = (forceRebuild ? 0 : CFile::getFileModificationDate(outputfname));
	for (i=0; i<files.size(); ++i)
		if (CFile::getExtension(files[i]) == "primitive" && CFile::getFileModificationDate(files[i]) > outputstamp)
			forceRebuild = true, ++numPrims;

	_Grid.clear();

	if (forceRebuild)
	{
		if (Verbose)
			nlinfo("Building file pacs.packed_prims (%d files changed)", numPrims);

		for (i=0; i<files.size(); ++i)
		{
			if (CFile::getExtension(files[i]) == "primitive")
			{
				readFile(files[i]);
			}
		}

		if (Verbose)
			nlinfo("pacs.packed_prims built!");

		COFile	f;
		if (f.open(outputfname))
		{
			f.serial(_Grid);
		}
		else
		{
			nlwarning("Couldn't save pacs.packed_prims file '%s'", outputfname.c_str());
		}
	}
	else
	{
		CIFile	f;
		if (f.open(outputfname))
		{
			f.serial(_Grid);
		}
		else
		{
			nlwarning("Couldn't load pacs.packed_prims file '%s'", outputfname.c_str());
			return false;
		}
	}

	return true;
}





/*
 *		readFile()
 */
void	CPrimChecker::readFile(const string &filename)
{
	string	fullpath = CPath::lookup(filename, false);

	if (fullpath.empty())
		return;

	// lookup for primitive file
	CIFile		f(fullpath);
	CIXml		xml;

	CPrimitives	prims;

	// load xml file
	xml.init(f);
	if (Verbose)
		nlinfo("Loaded prim file '%s'", filename.c_str());

	// read nodes
	if (!prims.read(xml.getRootNode(), filename.c_str(), LigoConfig))
	{
		nlwarning("Can't use primitive file '%s', xml parse error",  filename.c_str());
		return;
	}

	// get CPrimNode
	CPrimNode	*primRootNode = prims.RootNode;

	// read recursive node
	readPrimitive(primRootNode);
}

/*
 *		readPrimitive()
 */
void	CPrimChecker::readPrimitive(IPrimitive *primitive)
{
	string	className;

	// check good class and check primitive has a class name
	if (dynamic_cast<CPrimZone*>(primitive) != NULL && primitive->getPropertyByName("class", className))
	{
		if (className == "pacs_include")
			render(static_cast<CPrimZone*>(primitive), Include);
		else if (className == "pacs_exclude")
			render(static_cast<CPrimZone*>(primitive), Exclude);
		else if (className == "pacs_cluster_hint")
			render(static_cast<CPrimZone*>(primitive), ClusterHint);
	}

	// parse children
	uint	i;
	for (i=0; i<primitive->getNumChildren(); ++i)
	{
		IPrimitive	*child;

		if (!primitive->getChild(child, i))
			continue;

		readPrimitive(child);
	}
}


/*
 *		render()
 */
void	CPrimChecker::render(CPrimZone *zone, uint8 bits)
{
	if (zone->VPoints.size() < 3)
		return;

	string	name;
	if (zone->getPropertyByName("name", name) && Verbose)
		nlinfo("Rendering CPrimZone '%s'", name.c_str());

	// get the bouding box of the CPrimZone
	CAABBox	box;

	box.setCenter(zone->VPoints[0]);
	box.setHalfSize(CVector::Null);

	uint	i;
	for (i=1; i<zone->VPoints.size(); ++i)
		box.extend(zone->VPoints[i]);

	sint32	xmin, ymin, xmax, ymax;

	xmin = (sint32)(floor(box.getMin().x));
	ymin = (sint32)(floor(box.getMin().y));

	xmax = (sint32)(ceil(box.getMax().x));
	ymax = (sint32)(ceil(box.getMax().y));

	// Fill grid with points that belong to the CPrimZone
	sint32	x, y;
	for (y=ymin; y<=ymax; ++y)
		for (x=xmin; x<=xmax; ++x)
			if (zone->contains(CVector((float)x, (float)y, 0.0f)))
				_Grid.set(x, y, bits);
}
