/** \file export.h
 * Export from leveldesign data to client data
 *
 * $Id: export.h,v 1.4 2002/02/13 16:55:54 besson Exp $
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

#ifndef LD_EXPORT_H
#define LD_EXPORT_H

// ---------------------------------------------------------------------------

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "nel/misc/vector.h"
#include <string>
#include <vector>

// ---------------------------------------------------------------------------

namespace NL3D
{
	class CLandscape;
	class CVisualCollisionManager;
	class CVisualCollisionEntity;
}

struct SPlantInstance;
struct SFormPlant;

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------
struct SExportOptions
{
	// Options saved
	bool		GenerateLandscape;
	bool		GenerateVegetable;
	std::string OutLandscapeDir;
	std::string OutVegetableDir;
	std::string LandBankFile;
	std::string LandFarBankFile;
	float		CellSize;
	std::string	LandTileNoiseDir;

	// Options not saved
	std::string SourceDir; // Directory to parse (typically this is a region dir)
	std::string RootDir; // To access georges dfn

	// =======================================================================
	
	SExportOptions ();
	void serial (NLMISC::IStream& s);
};

// ---------------------------------------------------------------------------
// Export callback
// ---------------------------------------------------------------------------
class IExportCB
{
public:
	virtual bool isCanceled () = 0; // Tell the exporter if it must end as quick as possible
	// Display callbacks
	virtual void dispPass (const std::string &Text) = 0; // Pass (generate land, vegetable, etc...)
	virtual void dispPassProgress (float percentage) = 0; // [ 0.0 , 1.0 ]
	virtual void dispInfo (const std::string &Text) = 0; // Verbose
	virtual void dispWarning (const std::string &Text) = 0; // Error but not critical
	virtual void dispError (const std::string &Text) = 0; // Should block (misfunction)
};

// ---------------------------------------------------------------------------
// Vegetable export
// ---------------------------------------------------------------------------
struct SVegInst
{
	NLMISC::CVector Pos;
	float			Rot;
	float			Scale;

	float			Radius;
	std::string		ShapeName; // the .shape stored in the name field of the instance (that represents the shape name)
	std::string		PlantName; // the .plant associated which is stored in the instance name
};

// ---------------------------------------------------------------------------
// Export class
// ---------------------------------------------------------------------------
class CExport
{

public:

	CExport ();
	~CExport ();
	
	// EXPORT one region :
	// Parse the SourceDir find the .land and .prim
	bool export (SExportOptions &options, IExportCB *expCB = NULL);

	// HELPERS
	// Get All files with the extension ext in the current directory and subdirectory
	static void getAllFiles (const std::string &ext, std::vector<std::string> &files);
	// Search a file through all subdirectories of the current one (and in the current too)
	static bool searchFile (const std::string &plantName, std::string &dir);
	
private:
	
	static bool			_GeorgesLibInitialized;

	SExportOptions		*_Options;
	IExportCB			*_ExportCB;

	// Temp data to generate vegetable
	NL3D::CLandscape				*_Landscape;
	NL3D::CVisualCollisionManager	*_VCM;
	NL3D::CVisualCollisionEntity	*_VCE;

	std::vector<SVegInst>	_VegInsts;

private:

	// All the functions to generate the vegetables
	// ********************************************

	// Entry point
	bool generateVegetable (const std::string &SrcFile);

	// Get the altitude from the position in 2D
	float getZFromXY (float x, float y);

	// Does the plant is well placed
	bool isWellPlaced (NLMISC::CVector &pos, SPlantInstance &rPI, SFormPlant &rFP);

	// Write zone by zone the instance group corresponding to the vegetables generated in the specific land
	void writeVegetable (const std::string &LandFile);
	
	// Helpers
	// *******

	// Load all zones of a .land
	void loadLandscape (const std::string &name);
};

#endif // LD_EXPORT_H