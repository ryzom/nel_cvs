/** \file main.cpp
 * Enumeration of all objects in an instance group
 *
 * $Id: main.cpp,v 1.1 2002/03/18 14:44:39 besson Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include <iostream>

#include "nel/misc/file.h"

#include "3d/scene_group.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// ***************************************************************************
CInstanceGroup* LoadInstanceGroup (const char* sFilename)
{
	CIFile file;
	CInstanceGroup *newIG = new CInstanceGroup;

	if( file.open( sFilename ) )
	{
		try
		{
			newIG->serial (file);
		}
		catch (Exception &)
		{
			// Cannot save the file
			delete newIG;
			return NULL;
		}
	}
	else
	{
		delete newIG;
		return NULL;
	}
	return newIG;
}

// ***************************************************************************
int main(int nNbArg, char**ppArgs)
{

	if (nNbArg != 2)
	{
		cerr << "Bad number of argument" << endl;
		return -1;
	}

	_strupr (ppArgs[1]);
	if ( (strcmp (ppArgs[1], "-H") == 0) ||
		 (strcmp (ppArgs[1], "--HELP") == 0) ||
		 (strcmp (ppArgs[1], "/H") == 0) )
	{
		// Display help information
		cout << "USAGE : ig_enum myig.ig" << endl;
		cout << "This gives the list of the shapes in the instance group" << endl;
		return 1;
	}

	CInstanceGroup *pIG = LoadInstanceGroup (ppArgs[1]);
	if (pIG == NULL)
	{
		cerr << "File not found" << endl;
		return -1;
	}

	for (uint32 i = 0; i < pIG->getNumInstance(); ++i)
	{
		cout << pIG->getShapeName (i).c_str() << endl;
	}

	return 1;
}