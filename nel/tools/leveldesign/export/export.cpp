/** \file export.cpp
 * Implementation of export from leveldesign data to client data
 *
 * $Id: export.cpp,v 1.1 2001/12/28 14:47:59 besson Exp $
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

#include "export.h"
#include "formVegetable.h"

#include <windows.h>

#include "../../3d/ligo/lib/zone_region.h"
#include "../../3d/ligo/lib/primitive.h"

#include "../georges_lib/loader.h"
#include "../georges_lib/item.h"

#include "../georges_lib/FormBodyElt.h"
#include "../georges_lib/FormBodyEltAtom.h"
#include "../georges_lib/FormBodyEltList.h"
#include "../georges_lib/FormBodyEltStruct.h"

#include "3d/zone.h"
#include "3d/landscape.h"

#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLLIGO;

// ---------------------------------------------------------------------------
// SExportOptions
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SExportOptions::SExportOptions ()
{
	GenerateLandscape = true;
	GenerateVegetable = true;
}

// ---------------------------------------------------------------------------
void SExportOptions::serial (NLMISC::IStream& f)
{
	sint version = f.serialVersion (1);

	f.serial (GenerateLandscape);
	f.serial (GenerateVegetable);
	f.serial (OutLandscapeDir);
	f.serial (OutVegetableDir);

	if (version > 0)
	{
		f.serial (LandBankFile);
		f.serial (LandFarBankFile);
	}
}

// ---------------------------------------------------------------------------
// CExport
// ---------------------------------------------------------------------------

bool CExport::_GeorgesLibInitialized = false;

CExport::CExport()
{
	if (!_GeorgesLibInitialized)
	{
		NLMISC_REGISTER_CLASS( CFormBodyElt );
		NLMISC_REGISTER_CLASS( CFormBodyEltAtom );
		NLMISC_REGISTER_CLASS( CFormBodyEltList );
		NLMISC_REGISTER_CLASS( CFormBodyEltStruct );
		_GeorgesLibInitialized = true;
	}
}

// ---------------------------------------------------------------------------
bool CExport::export (SExportOptions &opt, IExportCB *expCB)
{
	char sTmp[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sTmp);

	_Options = &opt;
	_ExportCB = expCB;
	_Landscape = NULL;
	
	if (_Options->GenerateLandscape)
	{
		if (_ExportCB)
			_ExportCB->dispPass ("Generate Landscape");
		// \todo trap : GenerateLandscape with hulud
		//generateLandscape ();
	}

	// If we want to generate flora then we have to load the landscape
	if (_Options->GenerateVegetable)
	{
		if (_ExportCB)
			_ExportCB->dispPass ("Load Landscape");
		_Landscape = new CLandscape;
		_Landscape->init ();
		try 
		{
			CIFile bankFile (_Options->LandBankFile);
			_Landscape->TileBank.serial (bankFile);
			CIFile farbankFile (_Options->LandFarBankFile);
			_Landscape->TileFarBank.serial (farbankFile);
			_Landscape->initTileBanks ();
			vector<string>	allLandFiles;
			SetCurrentDirectory (_Options->SourceDir.c_str());
			getAllFiles (".land", allLandFiles);
			for (uint32 i = 0; i < allLandFiles.size(); ++i)
			{
				loadLandscape (allLandFiles[i]);
			}
		}
		catch (Exception &/*e*/)
		{
			if (_ExportCB)
				_ExportCB->dispError ("Cannot load banks files");
		}
	}

	if (_Options->GenerateVegetable)
	{
		if (_ExportCB)
			_ExportCB->dispPass ("Generate Vegetable");
		vector<string>	allVegetableFiles;
		SetCurrentDirectory (_Options->SourceDir.c_str());
		getAllFiles (".vegetable", allVegetableFiles);
		for (uint32 i = 0; i < allVegetableFiles.size(); ++i)
		{
			generateVegetable (allVegetableFiles[i]);
		}
	}

	if (_ExportCB)
		_ExportCB->dispPass ("Cleaning");

	SetCurrentDirectory (sTmp);
	if (_Landscape != NULL)
		delete _Landscape;
	
	if (_ExportCB)
		_ExportCB->dispPass ("Finished");

	return true;
}

// ---------------------------------------------------------------------------
void CExport::getAllFiles (const string &ext, vector<string> &files)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string sNewDir = sCurDir;
				sNewDir += string("\\") + findData.cFileName;
				SetCurrentDirectory (sNewDir.c_str());
				getAllFiles (ext, files);
				SetCurrentDirectory (sCurDir);
			}
			else
			{
				if (strlen(findData.cFileName) > strlen(ext.c_str()))
					if (stricmp(&findData.cFileName[strlen(findData.cFileName)-strlen(ext.c_str())], ext.c_str()) == 0)
					{
						string fullName = sCurDir;
						fullName += string("\\") + findData.cFileName;
						files.push_back (fullName);
					}
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
bool CExport::generateVegetable (const std::string &SrcFile)
{
	uint32 i;
	// Load all .prim
	vector<CPrimRegion> allPrimRegion;
	vector<string> allPrimFiles;
	getAllFiles (".prim", allPrimFiles);
	for (i = 0; i < allPrimFiles.size(); ++i)
	{
		try 
		{
			CPrimRegion tmpPrimRegion;
			CIFile fileIn;
			fileIn.open (allPrimFiles[i]);
			CIXml input;
			input.init (fileIn);
			tmpPrimRegion.serial (input);
			allPrimRegion.push_back (tmpPrimRegion);
		}
		catch (Exception &/*e*/)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load ") + allPrimFiles[i]);
		}
	}

	// Load the SrcFile (georges file)
	CLoader loader;
	CItem	item;
	loader.SetRootDirectory (_Options->RootDir);
	loader.SetWorkDirectory (_Options->SourceDir);
	item.SetLoader (&loader);
	try
	{
		item.Load (SrcFile);
	}
	catch (Exception &e)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError (string("Cant load ") + SrcFile + "(" + e.what() + ")" );
	}

	// Read .vegetable
	SFormVegetable formVegetable;
	formVegetable.build (item);
	
	// temptemptemp
	for (i = 0; i < formVegetable.PlantInstances.size(); ++i)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispInfo (formVegetable.PlantInstances[i].Name + "(" + toString(formVegetable.PlantInstances[i].Density)
								+ ")(" + toString(formVegetable.PlantInstances[i].Falloff) + ")" );
	}
	// temptemptemp

	// "File name"
	// "Density"
	// "Falloff"

	//"Name"
	//"Shape"
	//"Shadow"
	//"Collision_radius"
	//"Bunding_radius"


	return true;
}

// ---------------------------------------------------------------------------
void CExport::loadLandscape (const string &LandFile)
{
	CZoneRegion zoneRegion;
	CIFile inFile;
	inFile.open (LandFile);
	zoneRegion.serial (inFile);
	inFile.close();
	// Load all zone

	sint32 nTotalFile = (zoneRegion.getMaxY() - zoneRegion.getMinY()) * (zoneRegion.getMaxX() - zoneRegion.getMinX());
	sint32 nCurrentFile = 0;
	for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		++nCurrentFile;
		if (_ExportCB != NULL)
			_ExportCB->dispPassProgress(((float)nCurrentFile)/((float)nTotalFile));

		if ((zoneRegion.getName(i,j) == STRING_OUT_OF_BOUND) ||
			(zoneRegion.getName(i,j) == STRING_UNUSED))
			continue;

		// Generate zone name
		string ZoneName = NLMISC::toString(-j) + "_";
		ZoneName += ('a' + (i/26));
		ZoneName += ('a' + (i%26));

		ZoneName = _Options->OutLandscapeDir + string("\\") + ZoneName + string(".zonew");

		//if (_ExportCB != NULL)
		//	_ExportCB->dispInfo (string("Loading ") + ZoneName);

		try 
		{
			CZone zone;
			inFile.open (ZoneName);
			zone.serial (inFile);
			inFile.close ();
			_Landscape->addZone (zone);
		}
		catch(Exception &/*e*/)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load ") + ZoneName);
		}
		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			return;
	}

}