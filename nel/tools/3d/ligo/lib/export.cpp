/** \file export.cpp
 * Implementation of export from leveldesign data to client data
 *
 * $Id: export.cpp,v 1.1 2002/01/16 15:22:33 besson Exp $
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

#include <windows.h>

using namespace NL3D;
using namespace NLMISC;
using namespace std;
using namespace NLLIGO;

// ---------------------------------------------------------------------------
// Export options
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SExportOptions::SExportOptions ()
{
	ZoneRegion = NULL;
	CellSize = 160.0f;
	ZFactor = 1.0f;
}

// ---------------------------------------------------------------------------
void SExportOptions::serial (NLMISC::IStream& s)
{
	int version = s.serialVersion(3);

	s.serial (OutZoneDir);
	s.serial (RefZoneDir);
	
	if (version > 0)
		s.serial (LigoBankDir);

	if (version > 1)
		s.serial (TileBankFile);

	if (version > 2)
		s.serial (HeightMapFile);
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
		//set (x+deltaX+i, y+deltaY+j, 0, 0, STRING_UNUSED, true);
		//setRot (x+deltaX+i, y+deltaY+j, 0);
		//setFlip (x+deltaX+i, y+deltaY+j, 0);

		CZone UnitZone;

		// Put all the patches contained in the square ... in the unit zone
		cutZone (BigZone, UnitZone, x+deltaX+i, y+deltaY+j, PatchTransfered);

		// Save the zone
		string DstZoneFileName;
		try
		{
			DstZoneFileName = toString(-(y+deltaY+j)) + "_";
			DstZoneFileName += ('A' + ((x+deltaX+i)/26));
			DstZoneFileName += ('A' + ((x+deltaX+i)%26));
			//////// ATTENTION
			//////// ATTENTION
			//////// ATTENTION				il faut remettre en .zone pour la release !!!
			//////// ATTENTION
			//////// ATTENTION
			DstZoneFileName = _Options->OutZoneDir + string("\\") + DstZoneFileName + string(".zonel");
			COFile outFile (DstZoneFileName);
			UnitZone.serial (outFile);
		}
		catch (Exception &)
		{
			if (_ExportCB != NULL)
				_ExportCB->dispWarning (string("Cant write ") + DstZoneFileName);
		}
		
		// Set the zone as unused to not treat it the next time
		//pZR->basicSet (x+deltaX+i, y+deltaY+j, 0, 0, STRING_UNUSED);
		ZoneTreated[(x+deltaX+i)-nMinX + ((y+deltaY+j)-nMinY) * nStride] = true;
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

// ---------------------------------------------------------------------------
void CExport::transformZone (CZone &zeZone, sint32 nPosX, sint32 nPosY, uint8 nRot, uint8 nFlip)
{
	// Conversion nPosX,nPosY to Zone Coordinate ZoneX, ZoneY
	uint32 i, j, k;
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

		// when j == 7 or 0 use vertex 0
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
		for (j = 1; j < 4; j+=2)
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
			for (int pass = 0; pass < 3; ++pass)
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

			rPI.Tiles[j+k*rPI.OrderS].getTile256Info(is256x256, uvOff);
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

	zeZone.build (nZoneId, PatchInfos, BorderVertices);
}

// ---------------------------------------------------------------------------
void CExport::cutZone (NL3D::CZone &bigZone, NL3D::CZone &unitZone, sint32 nPosX, sint32 nPosY,
						vector<bool> &PatchTransfered)
{
	string DstZoneFileName = toString(-(nPosY)) + "_";
	DstZoneFileName += ('A' + ((nPosX)/26));
	DstZoneFileName += ('A' + ((nPosX)%26));

	uint32 i, j, k;
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
}

// ---------------------------------------------------------------------------
float CExport::getHeight (float x, float y)
{
	if (_HeightMap == NULL)
		return 0.0f;

	y = -y;

	clamp (x, 0.0f, _Options->CellSize*256.0f);
	clamp (y, 0.0f, _Options->CellSize*256.0f);

	CRGBAF color = _HeightMap->getColor(x / (_Options->CellSize*256.0f), y / (_Options->CellSize*256.0f));

	float deltaZ = color.A;
	deltaZ = deltaZ - 127.0f; // Median intensity is 127
	deltaZ *= _Options->ZFactor;

	return deltaZ;
}
