/** \file ligo/export.h
 * Export from leveldesign data to client data
 * Ligo To NeL
 * Get the .land, the reference zones and the heightmap and generate
 * all zones that will be next welded and lighted.
 *
 * $Id: export.h,v 1.3 2002/02/28 08:15:56 besson Exp $
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

#ifndef WE_EXPORT_H
#define WE_EXPORT_H

// ---------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include <string>
#include <vector>

// ---------------------------------------------------------------------------

namespace NLMISC
{
	class IStream;
	class CBitmap;
}

namespace NL3D
{
	class CZone;
	class CTileBank;
}

// ---------------------------------------------------------------------------

namespace NLLIGO
{

class CZoneRegion;
class CZoneBank;


// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------
struct SExportOptions
{
	// Options saved
	std::string		OutZoneDir;
	std::string		RefZoneDir;
	std::string		LigoBankDir;
	std::string		TileBankFile;
	
	std::string		HeightMapFile;
	float			ZFactor;
	std::string		HeightMapFile2;
	float			ZFactor2;

	uint8			Light; // Roughly light the zone (0-none, 1-patch, 2-noise)

	std::string		ZoneMin;
	std::string		ZoneMax;
	
	// Options not saved
	NLLIGO::CZoneRegion		*ZoneRegion; // The region to make
	float					CellSize;

	// =======================================================================
	
	SExportOptions ();
	void serial (NLMISC::IStream& s);
};

// ---------------------------------------------------------------------------
// Export callback
// ---------------------------------------------------------------------------
// The user of CExport can be informed of what's happen in the export process with
// this class and can cancel the process by returning true in the isCanceled method.
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

class CExport
{

public:

	CExport ();
	~CExport ();
	
	// EXPORT :
	bool export (SExportOptions &options, IExportCB *expCB = NULL);

	static std::string getZoneNameFromXY (sint32 x, sint32 y);
	static sint32 getXFromZoneName (const std::string &ZoneName);
	static sint32 getYFromZoneName (const std::string &ZoneName);

private:

	SExportOptions		*_Options;
	IExportCB			*_ExportCB;
	NLLIGO::CZoneBank	*_ZeZoneBank;

	NL3D::CTileBank		*_ZeTileBank;
	NLMISC::CBitmap		*_HeightMap;
	NLMISC::CBitmap		*_HeightMap2;
	sint32				_ZoneMinX, _ZoneMinY, _ZoneMaxX, _ZoneMaxY;
	
private:

	void treatPattern (sint32 nPosX, sint32 nPosY, 
						std::vector<bool> &ZoneTreated, sint32 nMinX, sint32 nMinY, sint32 nStride);

	void transformZone (NL3D::CZone &zeZone, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip);
	
	void cutZone (NL3D::CZone &bigZone, NL3D::CZone &unitZone, sint32 nPosX, sint32 nPosY, 
				std::vector<bool> &PatchTransfered);

	float getHeight (float x, float y);

	void light (NL3D::CZone &zoneOut, NL3D::CZone &zoneIn);
};

} // namespace NLLIGO

#endif // WE_EXPORT_H