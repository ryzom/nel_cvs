/** \file ligo/export.cpp
 * Implementation of export from leveldesign data to client data
 *
 * $Id: export.cpp,v 1.6 2002/03/13 12:41:03 besson Exp $
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
#include "zone_region.h"
#include "zone_bank.h"

#include "nel/misc/file.h"
#include "nel/misc/aabbox.h"

#include "3d/zone.h"
#include "3d/tile_bank.h"
#include "3d/zone_lighter.h"
#include "3d/landscape.h"

#include <windows.h>

using namespace NL3D;
using namespace NLMISC;
using namespace std;

namespace NLLIGO
{

// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SExportOptions::SExportOptions ()
{
	ZoneRegion = NULL;
	CellSize = 160.0f;
	ZFactor = 1.0f;
	ZFactor2 = 1.0f;
	Light = false;
}

// ---------------------------------------------------------------------------
void SExportOptions::serial (NLMISC::IStream& s)
{
	int version = s.serialVersion (6);

	s.serial (OutZoneDir);
	s.serial (RefZoneDir);
	
	if (version > 0)
		s.serial (LigoBankDir);

	if (version > 1)
		s.serial (TileBankFile);

	if (version > 2)
		s.serial (HeightMapFile);

	if (version > 3)
		s.serial (Light);

	if (version > 4)
	{
		s.serial (ZFactor);
		s.serial (HeightMapFile2);
		s.serial (ZFactor2);
	}

	if (version > 5)
	{
		s.serial (ZoneMin);
		s.serial (ZoneMax);
	}
}

// ---------------------------------------------------------------------------
// CExport
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CExport::CExport ()
{
	_ZeZoneBank = NULL;
}

// ---------------------------------------------------------------------------
CExport::~CExport ()
{
}
	
// ---------------------------------------------------------------------------
bool CExport::export (SExportOptions &options, IExportCB *expCB)
{
	char sTmp[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sTmp);

	_Options = &options;
	_ExportCB = expCB;

	if (_Options->ZoneRegion == NULL)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError ("No Zone to export");
		return false;
	}

	// LOADING
	// --- ligozone
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading ligozone bank");
	_ZeZoneBank = new CZoneBank;
	_ZeZoneBank->initFromPath (_Options->LigoBankDir);

	// --- tile
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading tile bank");
	_ZeTileBank = new CTileBank;
	try 
	{
		CIFile inFile (_Options->TileBankFile);
		_ZeTileBank->serial (inFile);
	}
	catch (Exception &)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning (string("Cant load tile bank : ") + _Options->TileBankFile);
	}

	// --- height map
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading height map");
	_HeightMap = NULL;
	if (_Options->HeightMapFile != "")
	{
		_HeightMap = new CBitmap;
		try 
		{
			CIFile inFile (_Options->HeightMapFile);
			_HeightMap->load (inFile);
		}
		catch (Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load height map : ") + _Options->HeightMapFile);
			delete _HeightMap;
			_HeightMap = NULL;
		}
	}

	// --- height map 2
	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Loading height map");
	_HeightMap2 = NULL;
	if (_Options->HeightMapFile2 != "")
	{
		_HeightMap2 = new CBitmap;
		try 
		{
			CIFile inFile (_Options->HeightMapFile2);
			_HeightMap2->load (inFile);
		}
		catch (Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant load height map : ") + _Options->HeightMapFile2);
			delete _HeightMap2;
			_HeightMap2 = NULL;
		}
	}

	// ****************
	// *EXPORTING CODE*
	// ****************

	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Exporting");

	// Clip the min max to the x[0,255],y[0,-255] values
	sint32 nMinX = _Options->ZoneRegion->getMinX() < 0 ? 0 : _Options->ZoneRegion->getMinX();
	sint32 nMaxX = _Options->ZoneRegion->getMaxX() > 255 ? 255 : _Options->ZoneRegion->getMaxX();
	sint32 nMinY = _Options->ZoneRegion->getMinY() > 0 ? 0 : _Options->ZoneRegion->getMinY();
	sint32 nMaxY = _Options->ZoneRegion->getMaxY() < -255 ? -255 : _Options->ZoneRegion->getMaxY();

	_ZoneMinX = nMinX;
	_ZoneMinY = nMinY;
	_ZoneMaxX = nMaxX;
	_ZoneMaxY = nMaxY;

	if ((_Options->ZoneMin != "") && (_Options->ZoneMax != ""))
	{
		_Options->ZoneMin = strupr (_Options->ZoneMin);
		_Options->ZoneMax = strupr (_Options->ZoneMax);
		sint32 nNewMinX = getXFromZoneName (_Options->ZoneMin);
		sint32 nNewMinY = getYFromZoneName (_Options->ZoneMin);
		sint32 nNewMaxX = getXFromZoneName (_Options->ZoneMax);
		sint32 nNewMaxY = getYFromZoneName (_Options->ZoneMax);

		if (nNewMinX > nNewMaxX) 
			swap (nNewMinX, nNewMaxX);
		if (nNewMinY > nNewMaxY) 
			swap (nNewMinY, nNewMaxY);

		if (nNewMinX > nMinX)
			nMinX = nNewMinX;
		if (nNewMinY > nMinY)
			nMinY = nNewMinY;

		if (nNewMaxX < nMaxX)
			nMaxX = nNewMaxX;
		if (nNewMaxY < nMaxY)
			nMaxY = nNewMaxY;
	}

	sint32 nTotalFile = (1 + nMaxY - nMinY) * (1 + nMaxX - nMinX);
	sint32 nCurrentFile = 0;

	vector<bool> ZoneTreated;
	ZoneTreated.resize(nTotalFile, false);

	for (sint32 j = nMinY; j <= nMaxY; ++j)
	for (sint32 i = nMinX; i <= nMaxX; ++i)
	if (!ZoneTreated[i-nMinX+(j-nMinY)*(1+nMaxX-nMinX)])
	{
		++nCurrentFile;
		if (_ExportCB != NULL)
			_ExportCB->dispPassProgress(((float)nCurrentFile)/((float)nTotalFile));

		const string &SrcZoneName = _Options->ZoneRegion->getName(i,j);

		if ((SrcZoneName == STRING_OUT_OF_BOUND) ||
			(SrcZoneName == STRING_UNUSED))
			continue;

		treatPattern (i, j, ZoneTreated, nMinX, nMinY, 1+nMaxX-nMinX);

		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			break;
	}

	if (_ExportCB != NULL)
		_ExportCB->dispPass ("Finished");
	delete _ZeZoneBank;
	delete _ZeTileBank;
	
	return true;
}

// ---------------------------------------------------------------------------
void CExport::treatPattern (sint32 x, sint32 y, 
							vector<bool> &ZoneTreated, sint32 nMinX, sint32 nMinY, sint32 nStride)
{

	CZoneRegion *pZR = _Options->ZoneRegion;
	const string &rSZone = pZR->getName (x, y);
	CZoneBankElement *pZBE = _ZeZoneBank->getElementByZoneName (rSZone);
	
	if (pZBE == NULL)
		return;

	sint32 sizeX = pZBE->getSizeX(), sizeY = pZBE->getSizeY();
	sint32 posX = pZR->getPosX (x, y), posY = pZR->getPosY (x, y);
	uint8 rot = pZR->getRot (x, y);
	uint8 flip = pZR->getFlip (x, y);
	sint32 i, j;
	sint32 deltaX, deltaY;

	if (flip == 0)
	{
		switch (rot)
		{
			case 0: deltaX = -posX; deltaY = -posY; break;
			case 1: deltaX = -(sizeY-1-posY); deltaY = -posX; break;
			case 2: deltaX = -(sizeX-1-posX); deltaY = -(sizeY-1-posY); break;
			case 3: deltaX = -posY; deltaY = -(sizeX-1-posX); break;
		}
	}
	else
	{
		switch (rot)
		{
			case 0: deltaX = -(sizeX-1-posX); deltaY = -posY; break;
			case 1: deltaX = -(sizeY-1-posY); deltaY = -(sizeX-1-posX); break;
			case 2: deltaX = -posX; deltaY = -(sizeY-1-posY); break;
			case 3: deltaX = -posY; deltaY = -posX; break;
		}
	}

	SPiece sMask;
	sMask.Tab.resize (sizeX*sizeY);
	for(i = 0; i < sizeX*sizeY; ++i)
		sMask.Tab[i] = pZBE->getMask()[i];
	sMask.w = sizeX;
	sMask.h = sizeY;
	sMask.rotFlip (rot, flip);

	// Put the big zone at the right position
	CZone BigZone;
	
	// 1 - Load the zone
	string BigZoneFileName;
	try
	{
		BigZoneFileName = _Options->RefZoneDir + string("\\") + rSZone + string(".zone");
		CIFile inFile (BigZoneFileName);
		BigZone.serial (inFile);
	}
	catch (Exception &)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispWarning (string("Cant load ") + BigZoneFileName);
	}

	// 2 - Transform the zone
	if (flip == 0)
	{
		switch (rot)
		{
			case 0: transformZone (BigZone, x+deltaX, y+deltaY, rot, flip); break;
			case 1: transformZone (BigZone, x+deltaX+sizeY, y+deltaY, rot, flip); break;
			case 2: transformZone (BigZone, x+deltaX+sizeX, y+deltaY+sizeY, rot, flip); break;
			case 3: transformZone (BigZone, x+deltaX, y+deltaY+sizeX, rot, flip); break;
		}
	}
	else
	{
		switch (rot)
		{
			case 0: transformZone (BigZone, x+deltaX+sizeX, y+deltaY, rot, flip); break;
			case 1: transformZone (BigZone, x+deltaX+sizeY, y+deltaY+sizeX, rot, flip); break;
			case 2: transformZone (BigZone, x+deltaX, y+deltaY+sizeY, rot, flip); break;
			case 3: transformZone (BigZone, x+deltaX, y+deltaY, rot, flip); break;
		}
	}
	
	// 3 - Cut the big zone into a set of unit zones
	vector<bool> PatchTransfered; // Is the patch n is transfered in a zoneUnit ?
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		CZone UnitZone;
		CZone UnitZoneLighted;

		// Put all the patches contained in the square ... in the unit zone
		cutZone (BigZone, UnitZone, x+deltaX+i, y+deltaY+j, PatchTransfered);

		if (_Options->Light > 0)
			light (UnitZoneLighted, UnitZone);
		else
			UnitZoneLighted = UnitZone;

		// Save the zone
		string DstZoneFileName;
		try
		{
			DstZoneFileName = getZoneNameFromXY(x+deltaX+i, y+deltaY+j);
			//////// ATTENTION
			//////// ATTENTION
			//////// ATTENTION				il faut remettre en .zone pour la release !!!
			//////// ATTENTION
			//////// ATTENTION
			DstZoneFileName = _Options->OutZoneDir + string("\\") + DstZoneFileName + string(".zonel");
			COFile outFile (DstZoneFileName);
			UnitZoneLighted.serial (outFile);
		}
		catch (Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant write ") + DstZoneFileName);
		}
		
		// Set the zone as unused to not treat it the next time
		ZoneTreated[(x+deltaX+i)-nMinX + ((y+deltaY+j)-nMinY) * nStride] = true;

		if ((_ExportCB != NULL) && (_ExportCB->isCanceled()))
			break;
	}
}

// ---------------------------------------------------------------------------
// Tile conversion 
int TransitionFlipLR[48] =
{
	0,	// 0
	4,	// 1
	5,	// 2
	27,	// 3
	1,	// 4
	2,	// 5
	6,	// 6
	34,	// 7
	11,	// 8
	33,	// 9
	31,	// 10
	8,	// 11
	13,	// 12
	12,	// 13
	47,	// 14
	40,	// 15
	39,	// 16
	20,	// 17
	46,	// 18
	45,	// 19
	17,	// 20
	43,	// 21
	42,	// 22
	41,	// 23
	24,	// 24
	28,	// 25
	29,	// 26
	3,	// 27
	25,	// 28
	26,	// 29
	30,	// 30
	10,	// 31
	35,	// 32
	9,	// 33
	7,	// 34
	32,	// 35
	37,	// 36
	36,	// 37
	44,	// 38
	16,	// 39
	15,	// 40
	23,	// 41
	22,	// 42
	21,	// 43
	38,	// 44
	19,	// 45
	18,	// 46
	14	// 47
};

int TransitionFlipUD[48] =
{
	24,	// 0
	28,	// 1
	29,	// 2
	3,	// 3
	25,	// 4
	26,	// 5
	30,	// 6
	10,	// 7
	35,	// 8
	9,	// 9
	7,	// 10
	32,	// 11
	37,	// 12
	36,	// 13
	23,	// 14
	16,	// 15
	15,	// 16
	44,	// 17
	22,	// 18
	21,	// 19
	38,	// 20
	19,	// 21
	18,	// 22
	14,	// 23
	0,	// 24
	4,	// 25
	5,	// 26
	27,	// 27
	1,	// 28
	2,	// 29
	6,	// 30
	34,	// 31
	11,	// 32
	33,	// 33
	31,	// 34
	8,	// 35
	13,	// 36
	12,	// 37
	20,	// 38
	40,	// 39
	39,	// 40
	47,	// 41
	46,	// 42
	45,	// 43
	17,	// 44
	43,	// 45
	42,	// 46
	41	// 47
};

int TransitionRotCCW[48] =
{
	27,	// 0
	28,	// 1
	29,	// 2
	0,	// 3
	1,	// 4
	2,	// 5
	33,	// 6
	34,	// 7
	35,	// 8
	6,	// 9
	7,	// 10
	8,	// 11
	39,	// 12
	40,	// 13
	17,	// 14
	12,	// 15
	13,	// 16
	41,	// 17
	45,	// 18
	46,	// 19
	47,	// 20
	18,	// 21
	19,	// 22
	20,	// 23
	3,	// 24
	4,	// 25
	5,	// 26
	24,	// 27
	25,	// 28
	26,	// 29
	9,	// 30
	10,	// 31
	11,	// 32
	30,	// 33
	31,	// 34
	32,	// 35
	15,	// 36
	16,	// 37
	14,	// 38
	36,	// 39
	37,	// 40
	38,	// 41
	21,	// 42
	22,	// 43
	23,	// 44
	42,	// 45
	43,	// 46
	44	// 47
};

// ---------------------------------------------------------------------------
void CExport::transformZone (CZone &zeZone, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip)
{
	// Conversion nPosX,nPosY to Zone Coordinate ZoneX, ZoneY
	uint32 i, j, k, pass;
	vector<CPatchInfo>		PatchInfos;
	vector<CBorderVertex>	BorderVertices;

	sint32 nZoneX = nPosX;
	sint32 nZoneY = -1 - nPosY;
	uint16 nZoneId = nZoneX+(nZoneY*256);

	zeZone.retrieve (PatchInfos, BorderVertices);

	nlassert (BorderVertices.size() == 0);

	CMatrix Transfo;
	Transfo.setRot (CQuat(CVector::K, (float)(nRot * Pi / 2.0f)));
	Transfo.setPos (CVector(nPosX*_Options->CellSize, (nPosY)*_Options->CellSize, 0.0f));

	if (nFlip != 0)
		nFlip = 1;

	if (nFlip == 1)
		Transfo.scale(CVector(-1.0f, 1.0f, 1.0f));

	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		rPI.Patch.applyMatrix (Transfo);

		for (j = 0; j < 4; ++j)
			rPI.Patch.Vertices[j].z += getHeight(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);

		for (j = 0; j < 4; ++j)
			rPI.Patch.Interiors[j].z += getHeight(rPI.Patch.Vertices[j].x, rPI.Patch.Vertices[j].y);

		// when j == 7 or 0 use vertex 0 for delta Z to ensure continuity of normals
		// when j == 1 or 2 use vertex 1
		// when j == 3 or 4 use vertex 2
		// when j == 5 or 6 use vertex 3
		for (j = 0; j < 8; ++j)
			rPI.Patch.Tangents[j].z += getHeight(rPI.Patch.Vertices[((j+1)/2)%4].x, rPI.Patch.Vertices[((j+1)/2)%4].y);

		for (j = 0; j < 4; ++j)
			rPI.BindEdges[j].ZoneId = nZoneId;
	}

	if (nFlip == 1)
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];

		// Flip the bezier patch (reorder)
		swap(rPI.Patch.Vertices[0], rPI.Patch.Vertices[3]); // A <-> D
		swap(rPI.Patch.Vertices[1], rPI.Patch.Vertices[2]); // B <-> C

		swap(rPI.Patch.Tangents[0], rPI.Patch.Tangents[5]); // ab <-> dc
		swap(rPI.Patch.Tangents[1], rPI.Patch.Tangents[4]); // ba <-> cd
		swap(rPI.Patch.Tangents[2], rPI.Patch.Tangents[3]); // bc <-> cb
		swap(rPI.Patch.Tangents[7], rPI.Patch.Tangents[6]); // ad <-> da

		swap(rPI.Patch.Interiors[0], rPI.Patch.Interiors[3]); // ia <-> id
		swap(rPI.Patch.Interiors[1], rPI.Patch.Interiors[2]); // ib <-> ic

		// Flip the base vertice
		swap(rPI.BaseVertices[0], rPI.BaseVertices[3]);
		swap(rPI.BaseVertices[1], rPI.BaseVertices[2]);

		// Flip bind edge only AB and CD
		swap(rPI.BindEdges[0], rPI.BindEdges[2]);

		// Flip bind edge content only if multiple bind
		for (j = 0; j < 4; j++)
		{
			if (rPI.BindEdges[j].NPatchs == 2)
			{
				swap (rPI.BindEdges[j].Next[0], rPI.BindEdges[j].Next[1]);
				swap (rPI.BindEdges[j].Edge[0], rPI.BindEdges[j].Edge[1]);
			}
			if (rPI.BindEdges[j].NPatchs == 4)
			{
				swap (rPI.BindEdges[j].Next[0], rPI.BindEdges[j].Next[3]);
				swap (rPI.BindEdges[j].Next[1], rPI.BindEdges[j].Next[2]);

				swap (rPI.BindEdges[j].Edge[0], rPI.BindEdges[j].Edge[3]);
				swap (rPI.BindEdges[j].Edge[1], rPI.BindEdges[j].Edge[2]);
			}
		}

		for (j = 0; j < 4; ++j)
		{
			uint32 nNbPatch = rPI.BindEdges[j].NPatchs == 5 ? 1 : rPI.BindEdges[j].NPatchs;
			for (k = 0; k < nNbPatch; ++k)
			{
				if (rPI.BindEdges[j].Edge[k] == 0)
					rPI.BindEdges[j].Edge[k] = 2;
				else if (rPI.BindEdges[j].Edge[k] == 2)
					rPI.BindEdges[j].Edge[k] = 0;
			}
		}

		// Tile switching
		for (j = 0; j < (uint32)(rPI.OrderS/2); ++j)
		{
			for (k = 0; k < rPI.OrderT; ++k)
			{
				swap(rPI.Tiles[j+k*rPI.OrderS], rPI.Tiles[(rPI.OrderS-1-j)+k*rPI.OrderS]);
			}
		}
		
		for (j = 0; j < (uint32)((rPI.OrderS+1)/2); ++j)
		{
			for (k = 0; k < (uint32)(rPI.OrderT+1); ++k)
			{
				swap(rPI.TileColors [j+k*(rPI.OrderS+1)], rPI.TileColors[(rPI.OrderS-j)+k*(rPI.OrderS+1)]);
			}
		}

		for (k = 0; k < rPI.OrderT; ++k)
		for (j = 0; j < rPI.OrderS; ++j)
		{
			for (pass = 0; pass < 3; ++pass)
			{
				if (rPI.Tiles[j+k*rPI.OrderS].Tile[pass] == NL_TILE_ELM_LAYER_EMPTY)
					break;
				int tileSet, number;
				CTileBank::TTileType type;
				_ZeTileBank->getTileXRef(rPI.Tiles[j+k*rPI.OrderS].Tile[pass], tileSet, number, type);
				if (type == CTileBank::transition)
				{
					nlassert((number >= 0) && (number <= 47));
					uint8 tileOrient = rPI.Tiles[j+k*rPI.OrderS].getTileOrient(pass);
					int numberAfterFlip;
					if ((tileOrient == 0) || (tileOrient == 2))
					{
						// Flip Left-Right
						numberAfterFlip = TransitionFlipLR[number];
					}
					else
					{
						// Flip Up-Down
						numberAfterFlip = TransitionFlipUD[number];
					}
					CTileSet *pTS = _ZeTileBank->getTileSet(tileSet);
					CTileSetTransition *pTST = pTS->getTransition (numberAfterFlip);
					int tileIdAfterFlip = pTST->getTile();
					rPI.Tiles[j+k*rPI.OrderS].Tile[pass] = tileIdAfterFlip;
				}
			}

			// 256x256
			bool is256x256;
			uint8 uvOff;

			rPI.Tiles[j+k*rPI.OrderS].getTile256Info (is256x256, uvOff);
			if (is256x256)
			{
				if (uvOff == 1)
					rPI.Tiles[j+k*rPI.OrderS].setTile256Info (is256x256,  2);
				else if (uvOff == 2)
					rPI.Tiles[j+k*rPI.OrderS].setTile256Info (is256x256,  1);
				else if (uvOff == 0)
					rPI.Tiles[j+k*rPI.OrderS].setTile256Info (is256x256,  3);
				else if (uvOff == 3)
					rPI.Tiles[j+k*rPI.OrderS].setTile256Info (is256x256,  0);
			}
		}
	}

	// Rotate all tile elements in CW (because zones are turned in CCW)
	// If zone flipped rotate tile elements by 180°
	set<string> allnames; // Debug
	for (i = 0; i < PatchInfos.size(); ++i)
	{
		CPatchInfo &rPI = PatchInfos[i];
		
		for (j = 0; j < rPI.Tiles.size(); ++j)
		{
			int tileSet, number;
			CTileBank::TTileType type;
			CTileSet *pTS;
			uint8 nbOfRot;

			// Is the tile is painted ?
			if (rPI.Tiles[j].Tile[0] == 65535)
				continue;
			
			_ZeTileBank->getTileXRef (rPI.Tiles[j].Tile[0], tileSet, number, type);
			pTS = _ZeTileBank->getTileSet (tileSet);

			// Debug beg
			/*
			if (allnames.find(pTS->getName()) == allnames.end())
			{
				allnames.insert(pTS->getName());
				if (_ExportCB != NULL)
				{
					if (!pTS->getOriented())
						_ExportCB->dispInfo (pTS->getName() + " NOT oriented");
					else
						_ExportCB->dispInfo (pTS->getName() + " oriented");
				}
			}
			*/
			// Debug end

			if (!pTS->getOriented())
			{
				nbOfRot = (4-nRot+2*nFlip)%4; // Not oriented so we can rot tile
			}
			else
			{
				nbOfRot = (2*nFlip)%4; // Oriented so we cant rot tile
			}

			// Rotate Tiles
			// Invert rotation effect on transition
			for (pass = 0; pass < 3; ++pass)
			{
				if (rPI.Tiles[j].Tile[pass] == NL_TILE_ELM_LAYER_EMPTY)
					break;
				uint8 ori = rPI.Tiles[j].getTileOrient (pass);

				// Invert rotation effect on transition
				_ZeTileBank->getTileXRef(rPI.Tiles[j].Tile[pass], tileSet, number, type);
				if (type == CTileBank::transition)
				{
					nlassert((number >= 0) && (number <= 47));
					int NumberAfterRot = number;
					for (k = 0; k < nbOfRot; ++k)
						NumberAfterRot = TransitionRotCCW[NumberAfterRot];

					pTS = _ZeTileBank->getTileSet (tileSet);
					CTileSetTransition *pTST = pTS->getTransition (NumberAfterRot);
					int tileIdAfterRot = pTST->getTile();
					rPI.Tiles[j].Tile[pass] = tileIdAfterRot;

				}
				// Rotate tile
				ori = (4+ori-nbOfRot)%4;
				rPI.Tiles[j].setTileOrient (pass, ori);
			}

			// Process the tile 256x256
			bool is256x256;
			uint8 uvOff;

			rPI.Tiles[j].getTile256Info (is256x256, uvOff);
			if (is256x256)
				rPI.Tiles[j].setTile256Info (is256x256,  (uvOff+nbOfRot)%4);

		}
	}
	
	zeZone.build (nZoneId, PatchInfos, BorderVertices);
}
/*
CLandscape gLand;
bool gLandInited = false;
*/
// ---------------------------------------------------------------------------
void CExport::cutZone (NL3D::CZone &bigZone, NL3D::CZone &unitZone, sint32 nPosX, sint32 nPosY,
						vector<bool> &PatchTransfered)
{
	string DstZoneFileName = getZoneNameFromXY (nPosX, nPosY);

	uint32 i, j, k, m;
	vector<CPatchInfo>		SrcPI;
	vector<CPatchInfo>		DstPI;
	vector<CBorderVertex>	BorderVertices;

	bigZone.retrieve (SrcPI, BorderVertices);
	sint32 nZoneX = nPosX;
	sint32 nZoneY = -1 - nPosY;
	uint16 nZoneId = nZoneX+(nZoneY*256);

	float rMinX = nPosX		* _Options->CellSize;
	float rMaxX = (1+nPosX) * _Options->CellSize;
	float rMinY = nPosY		* _Options->CellSize;
	float rMaxY = (1+nPosY) * _Options->CellSize;

	if (PatchTransfered.size() == 0)
		PatchTransfered.resize(SrcPI.size(), false);

	map<int,int> OldToNewPatchId; // Used to convert old patch id to new patch id

	for (i = 0; i < SrcPI.size(); ++i)
	if (!PatchTransfered[i]) // If patch not already transfered in a zone unit
	{
		CPatchInfo &rPI = SrcPI[i];

		// Is the Patch contained in the current mask ? Center of bbox tested
		CAABBox bb;

		bb.setCenter (rPI.Patch.Vertices[0]);
		for (j = 0; j < 4; ++j)
			bb.extend (rPI.Patch.Vertices[j]);

		for (j = 0; j < 4; ++j)
			bb.extend (rPI.Patch.Interiors[j]);

		for (j = 0; j < 8; ++j)
			bb.extend (rPI.Patch.Tangents[j]);

		if ((bb.getCenter().x >= rMinX)&&(bb.getCenter().x <= rMaxX)&&
			(bb.getCenter().y >= rMinY)&&(bb.getCenter().y <= rMaxY))
		{
			for (j = 0; j < 4; ++j)
				rPI.BindEdges[j].ZoneId = nZoneId;

			PatchTransfered[i] = true;
			DstPI.push_back (rPI);
			OldToNewPatchId.insert (pair<int,int>(i, DstPI.size()-1));
		}
	}

	// Add all patch that are binded to one of those of the DstPI list 
	uint32 nPreviousDstPISize = DstPI.size();
	for (;;)
	{
		for (i = 0; i < DstPI.size(); ++i)
		{
			CPatchInfo &rPI = DstPI[i];
			for (j = 0; j < 4; ++j)
			{
				if (rPI.BindEdges[j].NPatchs == 5)
				{
					uint next = rPI.BindEdges[j].Next[0];
					if (!PatchTransfered[next])
					{
						CPatchInfo &rPITmp = SrcPI[next];
						for (k = 0; k < 4; ++k)
							rPITmp.BindEdges[k].ZoneId = nZoneId;
						DstPI.push_back (rPITmp);
						OldToNewPatchId.insert (pair<int,int>(next, DstPI.size()-1));
						PatchTransfered[next] = true;
					}
				}

				if ((rPI.BindEdges[j].NPatchs == 2) || (rPI.BindEdges[j].NPatchs == 4))
				{
					for (k = 0; k < rPI.BindEdges[j].NPatchs; ++k)
					{
						uint next = rPI.BindEdges[j].Next[k];
						if (!PatchTransfered[next])
						{
							CPatchInfo &rPITmp = SrcPI[next];	
							for (m = 0; m < 4; ++m)
								rPITmp.BindEdges[m].ZoneId = nZoneId;
							DstPI.push_back (rPITmp);
							OldToNewPatchId.insert (pair<int,int>(next, DstPI.size()-1));
							PatchTransfered[next] = true;
						}
					}
				}
			}
		}

		// Do it until no more patch added
		if (nPreviousDstPISize == DstPI.size())
			break;
		nPreviousDstPISize = DstPI.size();
	}

	for (i = 0; i < DstPI.size(); ++i)
	{
		CPatchInfo &rPI = DstPI[i];
		for (j = 0; j < 4; ++j)
		{
			if ((rPI.BindEdges[j].NPatchs == 1) || (rPI.BindEdges[j].NPatchs == 5))
			{
				map<int,int>::iterator it = OldToNewPatchId.find (rPI.BindEdges[j].Next[0]);
				if (it == OldToNewPatchId.end())
				{
					if (rPI.BindEdges[j].NPatchs == 5)
					{
						if (_ExportCB != NULL)
							_ExportCB->dispError (string("Continuity problem in zone ") + DstZoneFileName);
					}
					else
						rPI.BindEdges[j].NPatchs = 0;
				}
				else
				{
					rPI.BindEdges[j].Next[0] = it->second;
				}
			}

			if ((rPI.BindEdges[j].NPatchs == 2) || (rPI.BindEdges[j].NPatchs == 4))
			{
				for (k = 0; k < rPI.BindEdges[j].NPatchs; ++k)
				{
					map<int,int>::iterator it = OldToNewPatchId.find (rPI.BindEdges[j].Next[k]);
					if (it == OldToNewPatchId.end())
					{
						if (_ExportCB != NULL)
							_ExportCB->dispError (string("Continuity problem in zone ") + DstZoneFileName);
					}
					else
					{
						rPI.BindEdges[j].Next[k] = it->second;
					}
				}
			}
		}
	}

	unitZone.build (nZoneId, DstPI, BorderVertices);
/*	{ // Debug
		if (!gLandInited)
		{
			gLand.init();
			gLandInited = true;
		}
		
		gLand.addZone(unitZone);
		gLand.checkBinds();
	}*/
}

// ---------------------------------------------------------------------------
float CExport::getHeight (float x, float y)
{
	float deltaZ = 0.0f, deltaZ2 = 0.0f;
	CRGBAF color;
	sint32 SizeX = _ZoneMaxX - _ZoneMinX + 1;
	sint32 SizeY = _ZoneMaxY - _ZoneMinY + 1;
	
	clamp (x, _Options->CellSize*_ZoneMinX, _Options->CellSize*(_ZoneMaxX+1));
	clamp (y, _Options->CellSize*_ZoneMinY, _Options->CellSize*(_ZoneMaxY+1));

	if (_HeightMap != NULL)
	{
		color = _HeightMap->getColor (	(x-_Options->CellSize*_ZoneMinX)/(_Options->CellSize*SizeX), 
										1.0f - ((y-_Options->CellSize*_ZoneMinY)/(_Options->CellSize*SizeY)));
		deltaZ = color.A;
		deltaZ = deltaZ - 127.0f; // Median intensity is 127
		deltaZ *= _Options->ZFactor;
	}

	if (_HeightMap2 != NULL)
	{
		color = _HeightMap2->getColor (	(x-_Options->CellSize*_ZoneMinX)/(_Options->CellSize*SizeX), 
										1.0f - ((y-_Options->CellSize*_ZoneMinY)/(_Options->CellSize*SizeY)));
		deltaZ2 = color.A;
		deltaZ2 = deltaZ2 - 127.0f; // Median intensity is 127
		deltaZ2 *= _Options->ZFactor2;
	}

	return (deltaZ + deltaZ2);
}

// ---------------------------------------------------------------------------
void CExport::light (NL3D::CZone &zoneOut, NL3D::CZone &zoneIn)
{
	// Same as zone_lighter stand-alone exe
	// ------------------------------------
	/*	CZoneLighter zl;
	CLandscape land;
	CZoneLighter::CLightDesc ld;
	vector<CZoneLighter::CTriangle> obstacle;
	vector<uint> listzone;

	ld.SkyContribution = false;
	ld.Oversampling = CZoneLighter::CLightDesc::NoOverSampling;
	ld.Shadow = false;
	ld.Softshadow = false;
	ld.NumCPU = 1;
	ld.GridSize = 2;

	try
	{
		zl.init ();
		land.init ();
		land.TileBank = *_ZeTileBank;
		land.initTileBanks();
		land.addZone (zoneIn);

		listzone.push_back(zoneIn.getZoneId());

		zl.light (land, zoneOut, zoneIn.getZoneId(), ld, obstacle, listzone);
	}
	catch (Exception &e)
	{
		if (_ExportCB != NULL)
			_ExportCB->dispError (e.what());
	}*/

	// Quickest version without noise
	// ------------------------------

	CLandscape land;

	land.init ();
	land.TileBank = *_ZeTileBank;
	land.initTileBanks();
	land.addZone (zoneIn);
	

	vector<CPatchInfo> vPI;
	vector<CBorderVertex> vBV;
	uint32 i, j, k, m;
	float s, t, val;
	CVector n, l = CVector (1.0f, 1.0f, -1.0f);
	vector<CVector> vertices;
	CVector v[4];

	l.normalize();

	CZone *dyn = land.getZone(zoneIn.getZoneId());
	uint32 numPatch = dyn->getNumPatchs();

	zoneIn.retrieve (vPI, vBV);

	if (_Options->Light == 2) // Noise ?
	for (i = 0; i < numPatch; ++i)
	{
		const CPatch *pCP = const_cast<const CZone *>(dyn)->getPatch (i);

		CPatchInfo &rPI = vPI[i];
		vertices.resize((rPI.OrderT*4+1)*(rPI.OrderS*4+1));

		for (k = 0; k < (uint32)(rPI.OrderT*4+1); ++k)
		for (j = 0; j < (uint32)(rPI.OrderS*4+1); ++j)
		{
			s = (((float)j) / (rPI.OrderS*4));
			t = (((float)k) / (rPI.OrderT*4));
			vertices[j+k*(rPI.OrderS*4+1)] = pCP->computeVertex(s, t);
		}

		for (k = 0; k < (uint32)(rPI.OrderT*4); ++k)
		for (j = 0; j < (uint32)(rPI.OrderS*4); ++j)
		{
			v[0] = vertices[(j+0)+(k+0)*(rPI.OrderS*4+1)];
			v[1] = vertices[(j+1)+(k+0)*(rPI.OrderS*4+1)];
			v[2] = vertices[(j+1)+(k+1)*(rPI.OrderS*4+1)];
			v[3] = vertices[(j+0)+(k+1)*(rPI.OrderS*4+1)];

			val = 0.0f;
			for (m = 0; m < 4; ++m)
			{
				n = (v[(m+0)%4]-v[(m+2)%4])^(v[(m+0)%4]-v[(m+1)%4]);
				n.normalize();
				val += 255.0f*(1.0f-n*l)/2.0f;
			}
			val = val / 4.0f;
			clamp (val, 0.0f, 255.0f);
			rPI.Lumels[j+k*rPI.OrderS*4] = (uint8)(val);
		}
	}
	else // No noise
	for (i = 0; i < numPatch; ++i)
	{
		const CPatch *pCP = const_cast<const CZone *>(dyn)->getPatch (i);
		CBezierPatch *pBP = pCP->unpackIntoCache();

		CPatchInfo &rPI = vPI[i];

		for (k = 0; k < (uint32)(rPI.OrderT*4); ++k)
		for (j = 0; j < (uint32)(rPI.OrderS*4); ++j)
		{
			s = ((0.5f+(float)j) / (rPI.OrderS*4));
			t = ((0.5f+(float)k) / (rPI.OrderT*4));
			n = pBP->evalNormal (s, t);
			val = 255.0f*(1.0f-n*l)/2.0f;
			clamp (val, 0.0f, 255.0f);
			rPI.Lumels[j+k*rPI.OrderS*4] = (uint8)(val);
		}
	}

	zoneOut.build (zoneIn.getZoneId(), vPI, vBV);
}

// ---------------------------------------------------------------------------
string CExport::getZoneNameFromXY (sint32 x, sint32 y)
{
	string tmp;

	if ((y>0) || (y<-255) || (x<0) || (x>255))
		return "NOT VALID";
	tmp = toString(-1-y) + "_";
	tmp += ('A' + (x/26));
	tmp += ('A' + (x%26));
	return tmp;
}

// ---------------------------------------------------------------------------
sint32 CExport::getXFromZoneName (const string &ZoneName)
{
	string xStr, yStr;
	uint32 i = 0;
	while (ZoneName[i] != '_')
	{
		yStr += ZoneName[i]; ++i;
	}
	++i;
	while (i < ZoneName.size())
	{
		xStr += ZoneName[i]; ++i;
	}
	return ((xStr[0] - 'A')*26 + (xStr[1] - 'A'));
}

// ---------------------------------------------------------------------------
sint32 CExport::getYFromZoneName (const string &ZoneName)
{
	string xStr, yStr;
	uint32 i = 0;
	while (ZoneName[i] != '_')
	{
		yStr += ZoneName[i]; ++i;
	}
	++i;
	while (i < ZoneName.size())
	{
		xStr += ZoneName[i]; ++i;
	}
	return -1-atoi(yStr.c_str());
}

} // namespace NLLIGO