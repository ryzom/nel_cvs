
#include "stdafx.h"
#include "builderZoneRegion.h"
#include "../lib/zone_bank.h"
#include "nel/misc/stream.h"
#include "builderZone.h"
#include <queue>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************
// CBuilderZoneRegion
// ***************************************************************************

std::string CBuilderZoneRegion::_StringOutOfBound;

// ---------------------------------------------------------------------------
CBuilderZoneRegion::SZoneUnit::SZoneUnit()
{
	ZoneName = STRING_UNUSED;
	PosX = PosY = 0;
	Rot = Flip = 0;
	SharingMatNames[0] = STRING_UNUSED;
	SharingMatNames[1] = STRING_UNUSED;
	SharingMatNames[2] = STRING_UNUSED;
	SharingMatNames[3] = STRING_UNUSED;
	SharingCutEdges[0] = 0;
	SharingCutEdges[1] = 0;
	SharingCutEdges[2] = 0;
	SharingCutEdges[3] = 0;
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::SZoneUnit::serial (NLMISC::IStream &f)
{
	f.serial (ZoneName);
	f.serial (PosX);
	f.serial (PosY);
	f.serial (Rot);
	f.serial (Flip);

	for (uint32 i = 0; i < 4; ++i)
	{
		f.serial (SharingMatNames[i]);
		f.serial (SharingCutEdges[i]);
	}
}

// ---------------------------------------------------------------------------
const CBuilderZoneRegion::SZoneUnit& CBuilderZoneRegion::SZoneUnit::operator=(const CBuilderZoneRegion::SZoneUnit&zu)
{
	this->ZoneName	= zu.ZoneName;
	this->PosX		= zu.PosX;
	this->PosY		= zu.PosY;
	this->Rot		= zu.Rot;
	this->Flip		= zu.Flip;
	for (uint32 i = 0; i < 4; ++i)
	{
		this->SharingMatNames[i] = zu.SharingMatNames[i];
		this->SharingCutEdges[i] = zu.SharingCutEdges[i];
	}
	return *this;
}

// ---------------------------------------------------------------------------
CBuilderZoneRegion::CBuilderZoneRegion ()
{
	_StringOutOfBound = STRING_OUT_OF_BOUND;
	_MinX = _MinY = 0;
	_MaxX = _MaxY = 0;
	SZoneUnit zuTmp;
	zuTmp.ZoneName = STRING_UNUSED;
	zuTmp.PosX = 0;
	zuTmp.PosY = 0;
	_Zones.push_back (zuTmp);
	_ZeBank = NULL;
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::init (NLLIGO::CZoneBank *pBank, CBuilderZone *pBuilder)
{
	if (_ZeBank != NULL)
		return;
	_ZeBank = pBank;
	_Builder = pBuilder;
	// Build the material tree
	uint32 i, j, k;
	SMatNode mn;
	vector<string> AllValues;
	_ZeBank->getCategoryValues ("material", AllValues);
	for (i = 0; i < AllValues.size(); ++i)
	{
		mn.Name = AllValues[i];
		_MatTree.push_back (mn);
	}
	// Link between materials
	AllValues.clear ();
	_ZeBank->getCategoryValues ("transname", AllValues);
	for (i = 0; i < AllValues.size(); ++i)
	{
		// Get the 2 materials linked together
		string matAstr, matBstr;
		for (j = 0; j < AllValues[i].size(); ++j)
		{
			if (AllValues[i][j] == '-')
				break;
			else
				matAstr += AllValues[i][j];
		}
		++j;
		for (; j < AllValues[i].size(); ++j)
			matBstr += AllValues[i][j];

		// Find matA
		for (j = 0; j < _MatTree.size(); ++j)
		if (_MatTree[j].Name == matAstr)
			break;

		nlassert(j < _MatTree.size());
		// MatA not found !!! Transition between materials that does not exists !!!

		// Find matB
		for (k = 0; k < _MatTree.size(); ++k)
		if (_MatTree[k].Name == matBstr)
			break;

		nlassert(k < _MatTree.size());
		// MatB not found !!! Transition between materials that does not exists !!!

		_MatTree[j].Arcs.push_back (k); // Add a ref to matB in node matA
		_MatTree[k].Arcs.push_back (j); // Add a ref to matA in node matB
	}

	// DEBUG
	// DEBUG
	// DEBUG
/*
	_MatTree.clear();
	_MatTree.resize (6);
	_MatTree[0].Name = "A";
	_MatTree[0].Arcs.resize (2);
	_MatTree[0].Arcs[0] = 1;
	_MatTree[0].Arcs[1] = 2;

	_MatTree[1].Name = "F";
	_MatTree[1].Arcs.resize (3);
	_MatTree[1].Arcs[0] = 0;
	_MatTree[1].Arcs[1] = 4;
	_MatTree[1].Arcs[2] = 5;

	_MatTree[2].Name = "B";
	_MatTree[2].Arcs.resize (3);
	_MatTree[2].Arcs[0] = 0;
	_MatTree[2].Arcs[1] = 3;
	_MatTree[2].Arcs[2] = 5;

	_MatTree[3].Name = "C";
	_MatTree[3].Arcs.resize (1);
	_MatTree[3].Arcs[0] = 2;

	_MatTree[4].Name = "Z";
	_MatTree[4].Arcs.resize (1);
	_MatTree[4].Arcs[0] = 1;

	_MatTree[5].Name = "E";
	_MatTree[5].Arcs.resize (2);
	_MatTree[5].Arcs[0] = 1;
	_MatTree[5].Arcs[1] = 2;

	vector<uint32> vRetPath;
	tryPath(0, 3, vRetPath);

	uint32 ij;
	++ij;
	ij = 0;
*/
	// DEBUG
	// DEBUG
	// DEBUG
}

// ---------------------------------------------------------------------------
class CToUpdate
{
	struct SElt
	{
		sint32				x, y;
		string				MatPut; // Material put into the cell to update
		CBuilderZoneRegion	*BZR;
	};

	vector<SElt> Elts;

public:

	void add (CBuilderZoneRegion* pBZR, sint32 x, sint32 y, const string &MatName)
	{
		bool bFound = false;
		for (uint32 m = 0; m < Elts.size(); ++m)
			if ((Elts[m].x == x) && (Elts[m].y == y))
			{
				bFound = true;
				break;
			}
		if (!bFound)
		{
			SElt tuTmp;
			tuTmp.x = x;
			tuTmp.y = y;
			tuTmp.MatPut = MatName;
			tuTmp.BZR = pBZR;
			Elts.push_back (tuTmp);
		}
	}

	void del (sint32 x, sint32 y)
	{
		bool bFound = false;
		for (uint32 m = 0; m < Elts.size(); ++m)
			if ((Elts[m].x == x) && (Elts[m].y == y))
			{
				bFound = true;
				break;
			}
		if (bFound)
		{
			for (; m < Elts.size()-1; ++m)
				Elts[m] = Elts[m+1];
			Elts.resize (Elts.size()-1);
		}
	}

	uint32 size()
	{
		return Elts.size();
	}

	sint32 getX (uint32 m)
	{
		return Elts[m].x;
	}

	sint32 getY (uint32 m)
	{
		return Elts[m].y;
	}

	CBuilderZoneRegion* getBZR (uint32 m)
	{
		return Elts[m].BZR;
	}

	const string& getMat (uint32 m)
	{
		return Elts[m].MatPut;
	}
};

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::add (sint32 x, sint32 y, uint8 nRot, uint8 nFlip, NLLIGO::CZoneBankElement *pElt)
{
	sint32 sizeX = pElt->getSizeX(), sizeY = pElt->getSizeY();
	sint32 i, j;
	SPiece sMask, sPosX, sPosY;
	CToUpdate tUpdate; // Transition to update

	if (!_Builder->getZoneMask (x,y))
		return;

	// Create the mask in the good rotation and flip
	sMask.Tab.resize (sizeX*sizeY);
	sPosX.Tab.resize (sizeX*sizeY);
	sPosY.Tab.resize (sizeX*sizeY);

	for (j = 0; j < sizeY; ++j)
	for (i = 0; i < sizeX; ++i)
	{
		sPosX.Tab[i+j*sizeX] = (uint8)i;
		sPosY.Tab[i+j*sizeX] = (uint8)j;
		sMask.Tab[i+j*sizeX] = pElt->getMask()[i+j*sizeX];
	}
	sPosX.w = sPosY.w = sMask.w = sizeX;
	sPosX.h = sPosY.h = sMask.h = sizeY;
	rotFlip (sMask, nRot, nFlip);
	rotFlip (sPosX, nRot, nFlip);
	rotFlip (sPosY, nRot, nFlip);

	// Test if the pieces can be put (due to mask)
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		if (_Builder->getZoneMask(x+i, y+j) == false)
			return;
	}

	// Delete all pieces that are under the mask
	for (j = 0; j < sMask.h; ++j)
	for (i = 0; i < sMask.w; ++i)
	if (sMask.Tab[i+j*sMask.w])
	{
		del (x+i, y+j, true, &tUpdate);
	}

	// Delete all around all material that are not from the same as us
	const string &CurMat = pElt->getCategory ("material");

	if (CurMat != STRING_NO_CAT_TYPE)
	{	// This element is a valid material
		// Place the piece
		const string &EltName = pElt->getName ();
		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+i, y+j, sPosX.Tab[i+j*sPosX.w], sPosY.Tab[i+j*sPosY.w], EltName);
			setRot (x+i, y+j, nRot);
			setFlip (x+i, y+j, nFlip);
		}

		// Put all transitions between different materials
		putTransitions (x, y, sMask, CurMat, &tUpdate);

		// Place the piece
		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+i, y+j, sPosX.Tab[i+j*sPosX.w], sPosY.Tab[i+j*sPosY.w], EltName);
			setRot (x+i, y+j, nRot);
			setFlip (x+i, y+j, nFlip);
		}
	}
	else // This element is a transition
	{
		const string &CurTrans = pElt->getCategory ("transname");
		if (sMask.Tab.size() > 1)
			return;
		// Check if this is the same type of transition
		sint32 stride = 1+_MaxX-_MinX;
		CZoneBankElement *pEltUnder = _ZeBank->getElementByZoneName(_Zones[(x-_MinX)+(y-_MinY)*stride].ZoneName);
		if (pEltUnder->getCategory("transname") != pElt->getCategory("transname"))
			return;
		if (pEltUnder->getCategory("transtype") != pElt->getCategory("transtype"))
			return;
		

		// Place the piece
		if (sMask.Tab[0])
		{
			set (x, y, sPosX.Tab[0], sPosY.Tab[0], pElt->getName(), true);
			setRot (x, y, nRot);
			setFlip (x, y, nFlip);
		}

		// Propagate
		if (x > _MinX)
		{	// [x-1][y].right = [x][y].left
			_Zones[(x-1-_MinX)+(y-_MinY)*stride].SharingCutEdges[3] = 
				_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[2];
		}
		if (y > _MinY)
		{	// [x][y-1].up = [x][y].down
			_Zones[(x-_MinX)+(y-1-_MinY)*stride].SharingCutEdges[0] = 
				_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[1]; 
		}
		if (x < _MaxX)
		{	// [x+1][y].left = [x][y].right
			_Zones[(x+1-_MinX)+(y-_MinY)*stride].SharingCutEdges[2] = 
				_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[3]; 
		}
		if (y < _MaxY)
		{	// [x][y+1].down = [x][y].up
			_Zones[(x-_MinX)+(y+1-_MinY)*stride].SharingCutEdges[1] = 
				_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[0]; 
		}

		// Update
		updateTrans (x, y);
		if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[0] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[2])
			updateTrans (x-1, y);
		if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[1] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[3])
			updateTrans (x+1, y);
		if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[0] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[1])
			updateTrans (x, y-1);
		if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[2] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[3])
			updateTrans (x, y+1);

	}
}

// ---------------------------------------------------------------------------
// Get a specific zone among all zones
CBuilderZoneRegion::SZoneUnit *CBuilderZoneRegion::getZoneAmongRegions (CBuilderZoneRegion*& pBZRfrom, sint32 x, sint32 y)
{
	uint32 nNbRegion = _Builder->_ZoneRegions.size();
	for (uint32 i = 0; i < nNbRegion; ++i)
	{
		CBuilderZoneRegion *pBZR = _Builder->_ZoneRegions[i];
		if ((x < pBZR->_MinX)||(x > pBZR->_MaxX)||(y < pBZR->_MinY)||(y > pBZR->_MaxY))
			continue;
		sint32 stride = (1 + pBZR->_MaxX - pBZR->_MinX);
		SZoneUnit *pZU = &pBZR->_Zones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride];
		if (pZU->ZoneName != STRING_UNUSED)
		{
			pBZRfrom = pBZR;
			return pZU;
		}
	}
	// The zone is not present in other region so it is an empty or oob zone of the current region
	if ((x < pBZRfrom->_MinX)||(x > pBZRfrom->_MaxX)||(y < pBZRfrom->_MinY)||(y > pBZRfrom->_MaxY))
		return NULL; // Out Of Bound
	sint32 stride = (1 + pBZRfrom->_MaxX - pBZRfrom->_MinX);
	SZoneUnit *pZU = &pBZRfrom->_Zones[(x-pBZRfrom->_MinX)+(y-pBZRfrom->_MinY)*stride];
	return pZU;
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::addToUpdateAndCreate (CBuilderZoneRegion *pBZRfrom, sint32 sharePos, sint32 x, sint32 y, const string &sNewMat, void *pInt1, void *pInt2)
{
	CToUpdate *ptCreate = (CToUpdate*)pInt1;
	CToUpdate *ptUpdate = (CToUpdate*)pInt2;
	sint32 stride = (1+_MaxX-_MinX);

	SZoneUnit *pZU = getZoneAmongRegions (pBZRfrom, x, y);
	if (pZU != NULL)
	{
		pZU->SharingMatNames[sharePos] = sNewMat;
		pBZRfrom->del (x, y, true, ptUpdate);
		ptCreate->add (pBZRfrom, x, y, sNewMat);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::putTransitions (sint32 inX, sint32 inY, const SPiece &rMask, const string &MatName,
										 void *pInternal)
{
	CToUpdate tCreate; // Transition to create
	CToUpdate *ptUpdate = (CToUpdate*)pInternal; // Transition to update

	sint32 i, j, k, l, m;
	sint32 x = inX, y = inY;
	for (j = 0; j < rMask.h; ++j)
	for (i = 0; i < rMask.w; ++i)
	if (rMask.Tab[i+j*rMask.w])
	{
		for (k = -1; k <= 1; ++k)
		for (l = -1; l <= 1; ++l)
		{
			CBuilderZoneRegion *pBZR = this;
			if (getZoneAmongRegions (pBZR, inX+i+l, inY+j+k) != NULL)
				tCreate.add (pBZR, inX+i+l, inY+j+k, MatName);
		}
	}
	
	// Check coherency of the transition to update
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		CBuilderZoneRegion *pBZR = tCreate.getBZR(m);
		vector<SZoneUnit> &rZones = pBZR->_Zones;
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		string putMat = tCreate.getMat(m);
		sint32 stride = (1+pBZR->_MaxX-pBZR->_MinX);

		//if ((x < _MinX)||(x > _MaxX)||(y < _MinY)||(y > _MaxY))
		//	continue;
		
		if (!((rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[0] == rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1])&&
			(rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1] == rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2])&&
			(rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2] == rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[3])))
			pBZR->del (x, y, true, ptUpdate);

		// Expand material for the 1st quarter
		string newMat = getNextMatInTree (putMat, rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[0]);
		if (newMat != rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[0])
		{	// Update the quarter
			rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[0] = newMat;

			addToUpdateAndCreate (pBZR, 1, x-1, y, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 3, x-1, y-1, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 2, x, y-1, newMat, &tCreate, ptUpdate);
		}

		// Expand material for the 2nd quarter
		newMat = getNextMatInTree (putMat, rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1]);
		if (newMat != rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1])
		{	// Update the quarter
			//if (_Builder->getZoneMask(x,y))
				rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1] = newMat;

			addToUpdateAndCreate (pBZR, 0, x+1, y, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 2, x+1, y-1, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 3, x, y-1, newMat, &tCreate, ptUpdate);
		}

		// Expand material for the 3rd quarter
		newMat = getNextMatInTree (putMat, rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2]);
		if (newMat != rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2])
		{	// Update the quarter
			//if (_Builder->getZoneMask(x,y))
				rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2] = newMat;
			
			addToUpdateAndCreate (pBZR, 3, x-1, y, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 1, x-1, y+1, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 0, x, y+1, newMat, &tCreate, ptUpdate);
		}

		// Expand material for the 4th quarter
		newMat = getNextMatInTree (putMat, rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[3]);
		if (newMat != rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[3])
		{	// Update the quarter
			//if (_Builder->getZoneMask(x,y))
				rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[3] = newMat;
			addToUpdateAndCreate (pBZR, 2, x+1, y, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 0, x+1, y+1, newMat, &tCreate, ptUpdate);
			addToUpdateAndCreate (pBZR, 1, x, y+1, newMat, &tCreate, ptUpdate);
		}
	}

	// Delete transitions that are inside the mask
	for (j = 0; j < rMask.h; ++j)
	for (i = 0; i < rMask.w; ++i)
	if (rMask.Tab[i+j*rMask.w])
	{
		tCreate.del (inX+i, inY+j);
	}

	// For all transition to update choose the cut edge
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		CBuilderZoneRegion *pBZR = tCreate.getBZR(m);
		vector<SZoneUnit> &rZones = pBZR->_Zones;
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		sint32 stride = (1 + pBZR->_MaxX - pBZR->_MinX);

		if ((x < pBZR->_MinX)||(x > pBZR->_MaxX)||(y < pBZR->_MinY)||(y > pBZR->_MaxY))
			continue;

		for (i = 0; i < 4; ++i)
		{
			uint8 nCut = (uint8)(1.0f+NLMISC::frand(2.0f));
			NLMISC::clamp (nCut, (uint8)1, (uint8)2);
			rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingCutEdges[i] = nCut;
		}
		// Propagate
		if (rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[0] != 
			rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2])
		{	// [x-1][y].right = [x][y].left
			CBuilderZoneRegion *pBZR2 = pBZR;
			SZoneUnit *pZU = getZoneAmongRegions (pBZR2, x-1, y);
			if (pZU != NULL)
			{
				pZU->SharingCutEdges[3] = rZones[(x-pBZR->_MinX)+(y-_MinY)*stride].SharingCutEdges[2]; 
				ptUpdate->add (pBZR2, x-1, y, "");
			}
		}

		if (rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[0] != 
			rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1])
		{	// [x][y-1].up = [x][y].down
			CBuilderZoneRegion *pBZR2 = pBZR;
			SZoneUnit *pZU = getZoneAmongRegions (pBZR2, x, y-1);
			if (pZU != NULL)
			{
				pZU->SharingCutEdges[0] = rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingCutEdges[1]; 
				ptUpdate->add (pBZR2, x, y-1, "");
			}
		}

		if (rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[3] != 
			rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[1])
		{	// [x+1][y].left = [x][y].right
			CBuilderZoneRegion *pBZR2 = pBZR;
			SZoneUnit *pZU = getZoneAmongRegions (pBZR2, x+1, y);
			if (pZU != NULL)
			{
				pZU->SharingCutEdges[2] = rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingCutEdges[3]; 
				ptUpdate->add (pBZR2, x+1, y, "");
			}
		}

		if (rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[2] != 
			rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingMatNames[3])
		{	// [x][y+1].down = [x][y].up
			CBuilderZoneRegion *pBZR2 = pBZR;
			SZoneUnit *pZU = getZoneAmongRegions (pBZR2, x, y+1);
			if (pZU != NULL)
			{
				pZU->SharingCutEdges[1] = rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride].SharingCutEdges[0]; 
				ptUpdate->add (pBZR2, x, y+1, "");
			}
		}
	}	

	// Delete in tUpdate each element in common with tCreate
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{		
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		ptUpdate->del (x,y);
	}

	// Finally update all transition
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		CBuilderZoneRegion *pBZR = tCreate.getBZR(m);
		x = tCreate.getX(m);
		y = tCreate.getY(m);

		if ((x >= pBZR->_MinX)&&(x <= pBZR->_MaxX)&&(y >= pBZR->_MinY)&&(y <= pBZR->_MaxY))
			pBZR->updateTrans (x, y);
	}
	for (m = 0; m < (sint32)ptUpdate->size(); ++m)
	{
		CBuilderZoneRegion *pBZR = ptUpdate->getBZR(m);
		x = ptUpdate->getX(m);
		y = ptUpdate->getY(m);
		if ((x >= pBZR->_MinX)&&(x <= pBZR->_MaxX)&&(y >= pBZR->_MinY)&&(y <= pBZR->_MaxY))
			pBZR->updateTrans (x, y);
	}

	// Cross material
	for (m = 0; m < (sint32)tCreate.size(); ++m)
	{
		CBuilderZoneRegion *pBZR = tCreate.getBZR(m);
		vector<SZoneUnit> &rZones = pBZR->_Zones;
		x = tCreate.getX(m);
		y = tCreate.getY(m);
		sint32 stride = (1 + pBZR->_MaxX - pBZR->_MinX);

		SZoneUnit &rZone = rZones[(x-pBZR->_MinX)+(y-pBZR->_MinY)*stride];
		std::set<string> matNameSet;
		for (i = 0; i < 4; ++i)
			matNameSet.insert (rZone.SharingMatNames[i]);

		if (((rZone.SharingMatNames[0] == rZone.SharingMatNames[3]) &&
			(rZone.SharingMatNames[1] == rZone.SharingMatNames[2]) &&
			(rZone.SharingMatNames[0] != rZone.SharingMatNames[1])) 
			|| (matNameSet.size()>2))
		{

			_ZeBank->resetSelection ();
			_ZeBank->addOrSwitch ("material", tCreate.getMat(m));
			_ZeBank->addAndSwitch ("size", "1x1");
			vector<CZoneBankElement*> vElts;
			_ZeBank->getSelection (vElts);
			if (vElts.size() == 0)
				return;
			sint32 nRan = (sint32)(NLMISC::frand((float)vElts.size()));
			NLMISC::clamp (nRan, (sint32)0, (sint32)(vElts.size()-1));
			CZoneBankElement *pZBE = vElts[nRan];
			nRan = (uint32)(NLMISC::frand (1.0) * 4);
			NLMISC::clamp (nRan, (sint32)0, (sint32)3);
			uint8 rot = (uint8)nRan;
			nRan = (uint32)(NLMISC::frand (1.0) * 2);
			NLMISC::clamp (nRan, (sint32)0, (sint32)1);
			uint8 flip = (uint8)nRan;

			pBZR->add (x, y, rot, flip, pZBE);
		}
	}

}


// ---------------------------------------------------------------------------
void CBuilderZoneRegion::putTransition (sint32 x, sint32 y, const string &MatName)
{
	const string &rSZone = getName (x, y);
	if (rSZone != STRING_UNUSED)
		return;

	sint32 stride = (1+_MaxX-_MinX);
	sint32 m;
	// Set Random edges
	for (m = 0; m < 4; ++m)
	{
		uint8 nCut = (uint8)(1.0f+NLMISC::frand(2.0f));
		NLMISC::clamp (nCut, (uint8)1, (uint8)2);
		_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[m] = nCut;
	}
	// Propagate
	if (x > _MinX)
	{	// [x-1][y].right = [x][y].left
		_Zones[(x-1-_MinX)+(y-_MinY)*stride].SharingCutEdges[3] = 
			_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[2]; 
	}
	if (y > _MinY)
	{	// [x][y-1].up = [x][y].down
		_Zones[(x-_MinX)+(y-1-_MinY)*stride].SharingCutEdges[0] = 
			_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[1]; 
	}
	if (x < _MaxX)
	{	// [x+1][y].left = [x][y].right
		_Zones[(x+1-_MinX)+(y-_MinY)*stride].SharingCutEdges[2] = 
			_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[3]; 
	}
	if (y < _MaxY)
	{	// [x][y+1].down = [x][y].up
		_Zones[(x-_MinX)+(y+1-_MinY)*stride].SharingCutEdges[1] = 
			_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[0]; 
	}

	// Update Transitions
	updateTrans (x, y);
	if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[0] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[2])
		updateTrans (x-1, y);
	if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[1] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[3])
		updateTrans (x+1, y);
	if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[0] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[1])
		updateTrans (x, y-1);
	if (_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[2] != _Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[3])
		updateTrans (x, y+1);
}

// ---------------------------------------------------------------------------
struct STrans
{
	uint8 Num;
	uint8 Rot;
	uint8 Flip;
};

// ---------------------------------------------------------------------------
STrans TranConvTable[128] =
{
	{ 0,0,0 }, // Quart = 0, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 0, CutEdge = 3, Np = 1 UNUSED

	{ 6,0,0 }, // Quart = 1, CutEdge = 0, Np = 0
	{ 6,3,1 }, // Quart = 1, CutEdge = 0, Np = 1
	{ 7,0,0 }, // Quart = 1, CutEdge = 1, Np = 0
	{ 7,0,0 }, // Quart = 1, CutEdge = 1, Np = 1
	{ 7,3,1 }, // Quart = 1, CutEdge = 2, Np = 0
	{ 7,3,1 }, // Quart = 1, CutEdge = 2, Np = 1
	{ 8,0,0 }, // Quart = 1, CutEdge = 3, Np = 0
	{ 8,3,1 }, // Quart = 1, CutEdge = 3, Np = 1

	{ 6,1,0 }, // Quart = 2, CutEdge = 0, Np = 0
	{ 6,0,1 }, // Quart = 2, CutEdge = 0, Np = 1
	{ 7,0,1 }, // Quart = 2, CutEdge = 1, Np = 0
	{ 7,0,1 }, // Quart = 2, CutEdge = 1, Np = 1
	{ 7,1,0 }, // Quart = 2, CutEdge = 2, Np = 0
	{ 7,1,0 }, // Quart = 2, CutEdge = 2, Np = 1
	{ 8,1,0 }, // Quart = 2, CutEdge = 3, Np = 0
	{ 8,0,1 }, // Quart = 2, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 3, CutEdge = 0, Np = 0
	{ 0,0,1 }, // Quart = 3, CutEdge = 0, Np = 1
	{ 1,0,1 }, // Quart = 3, CutEdge = 1, Np = 0
	{ 1,0,1 }, // Quart = 3, CutEdge = 1, Np = 1
	{ 1,0,0 }, // Quart = 3, CutEdge = 2, Np = 0
	{ 1,0,0 }, // Quart = 3, CutEdge = 2, Np = 1
	{ 2,0,0 }, // Quart = 3, CutEdge = 3, Np = 0
	{ 2,0,1 }, // Quart = 3, CutEdge = 3, Np = 1

	{ 6,3,0 }, // Quart = 4, CutEdge = 0, Np = 0
	{ 6,2,1 }, // Quart = 4, CutEdge = 0, Np = 1
	{ 7,2,1 }, // Quart = 4, CutEdge = 1, Np = 0
	{ 7,2,1 }, // Quart = 4, CutEdge = 1, Np = 1
	{ 7,3,0 }, // Quart = 4, CutEdge = 2, Np = 0
	{ 7,3,0 }, // Quart = 4, CutEdge = 2, Np = 1
	{ 8,3,0 }, // Quart = 4, CutEdge = 3, Np = 0
	{ 8,2,1 }, // Quart = 4, CutEdge = 3, Np = 1

	{ 0,3,0 }, // Quart = 5, CutEdge = 0, Np = 0
	{ 0,3,1 }, // Quart = 5, CutEdge = 0, Np = 1
	{ 1,3,1 }, // Quart = 5, CutEdge = 1, Np = 0
	{ 1,3,1 }, // Quart = 5, CutEdge = 1, Np = 1
	{ 1,3,0 }, // Quart = 5, CutEdge = 2, Np = 0
	{ 1,3,0 }, // Quart = 5, CutEdge = 2, Np = 1
	{ 2,3,0 }, // Quart = 5, CutEdge = 3, Np = 0
	{ 2,3,1 }, // Quart = 5, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 6, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 6, CutEdge = 3, Np = 1 UNUSED

	{ 5,2,0 }, // Quart = 7, CutEdge = 0, Np = 0
	{ 5,1,1 }, // Quart = 7, CutEdge = 0, Np = 1
	{ 4,1,1 }, // Quart = 7, CutEdge = 1, Np = 0
	{ 4,1,1 }, // Quart = 7, CutEdge = 1, Np = 1
	{ 4,2,0 }, // Quart = 7, CutEdge = 2, Np = 0
	{ 4,2,0 }, // Quart = 7, CutEdge = 2, Np = 1
	{ 3,2,0 }, // Quart = 7, CutEdge = 3, Np = 0
	{ 3,1,1 }, // Quart = 7, CutEdge = 3, Np = 1

	{ 6,2,0 }, // Quart = 8, CutEdge = 0, Np = 0
	{ 6,1,1 }, // Quart = 8, CutEdge = 0, Np = 1
	{ 7,2,0 }, // Quart = 8, CutEdge = 1, Np = 0
	{ 7,2,0 }, // Quart = 8, CutEdge = 1, Np = 1
	{ 7,1,1 }, // Quart = 8, CutEdge = 2, Np = 0
	{ 7,1,1 }, // Quart = 8, CutEdge = 2, Np = 1
	{ 8,2,0 }, // Quart = 8, CutEdge = 3, Np = 0
	{ 8,1,1 }, // Quart = 8, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 9, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 9, CutEdge = 3, Np = 1 UNUSED

	{ 0,1,0 }, // Quart = 10, CutEdge = 0, Np = 0
	{ 0,1,1 }, // Quart = 10, CutEdge = 0, Np = 1
	{ 1,1,0 }, // Quart = 10, CutEdge = 1, Np = 0
	{ 1,1,0 }, // Quart = 10, CutEdge = 1, Np = 1
	{ 1,1,1 }, // Quart = 10, CutEdge = 2, Np = 0
	{ 1,1,1 }, // Quart = 10, CutEdge = 2, Np = 1
	{ 2,1,0 }, // Quart = 10, CutEdge = 3, Np = 0
	{ 2,1,0 }, // Quart = 10, CutEdge = 3, Np = 1

	{ 5,3,0 }, // Quart = 11, CutEdge = 0, Np = 0
	{ 5,2,1 }, // Quart = 11, CutEdge = 0, Np = 1
	{ 4,3,0 }, // Quart = 11, CutEdge = 1, Np = 0
	{ 4,3,0 }, // Quart = 11, CutEdge = 1, Np = 1
	{ 4,2,1 }, // Quart = 11, CutEdge = 2, Np = 0
	{ 4,2,1 }, // Quart = 11, CutEdge = 2, Np = 1
	{ 3,3,0 }, // Quart = 11, CutEdge = 3, Np = 0
	{ 3,2,1 }, // Quart = 11, CutEdge = 3, Np = 1

	{ 0,2,0 }, // Quart = 12, CutEdge = 0, Np = 0
	{ 0,2,1 }, // Quart = 12, CutEdge = 0, Np = 1
	{ 1,2,0 }, // Quart = 12, CutEdge = 1, Np = 0
	{ 1,2,0 }, // Quart = 12, CutEdge = 1, Np = 1
	{ 1,2,1 }, // Quart = 12, CutEdge = 2, Np = 0
	{ 1,2,1 }, // Quart = 12, CutEdge = 2, Np = 1
	{ 2,2,0 }, // Quart = 12, CutEdge = 3, Np = 0
	{ 2,2,1 }, // Quart = 12, CutEdge = 3, Np = 1

	{ 5,1,0 }, // Quart = 13, CutEdge = 0, Np = 0
	{ 5,0,1 }, // Quart = 13, CutEdge = 0, Np = 1
	{ 4,1,0 }, // Quart = 13, CutEdge = 1, Np = 0
	{ 4,1,0 }, // Quart = 13, CutEdge = 1, Np = 1
	{ 4,0,1 }, // Quart = 13, CutEdge = 2, Np = 0
	{ 4,0,1 }, // Quart = 13, CutEdge = 2, Np = 1
	{ 3,1,0 }, // Quart = 13, CutEdge = 3, Np = 0
	{ 3,0,1 }, // Quart = 13, CutEdge = 3, Np = 1

	{ 5,0,0 }, // Quart = 14, CutEdge = 0, Np = 0
	{ 5,3,1 }, // Quart = 14, CutEdge = 0, Np = 1
	{ 4,3,1 }, // Quart = 14, CutEdge = 1, Np = 0
	{ 4,3,1 }, // Quart = 14, CutEdge = 1, Np = 1
	{ 4,0,0 }, // Quart = 14, CutEdge = 2, Np = 0
	{ 4,0,0 }, // Quart = 14, CutEdge = 2, Np = 1
	{ 3,0,0 }, // Quart = 14, CutEdge = 3, Np = 0
	{ 3,3,1 }, // Quart = 14, CutEdge = 3, Np = 1

	{ 0,0,0 }, // Quart = 15, CutEdge = 0, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 0, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 1, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 1, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 2, Np = 0 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 2, Np = 1 UNUSED
	{ 0,0,0 }, // Quart = 15, CutEdge = 3, Np = 0 UNUSED
	{ 0,0,0 }  // Quart = 15, CutEdge = 3, Np = 1 UNUSED
};

// ---------------------------------------------------------------------------
void CBuilderZoneRegion:: updateTrans (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) || (y < _MinY) || (y > _MaxY))
		return;

	//if (!_Builder->getZoneMask(x,y))
	//	return;

	// Interpret the transition info
	x -= _MinX;
	y -= _MinY;
	sint32 stride = (1+_MaxX-_MinX);
	sint32 m;
	// Calculate the number of material around with transition info
	std::set<string> matNameSet;
	for (m = 0; m < 4; ++m)
		matNameSet.insert (_Zones[x+y*stride].SharingMatNames[m]);

	if ((_Zones[x+y*stride].SharingMatNames[0] != STRING_UNUSED) &&
		(matNameSet.size() == 1))
	{
		CZoneBankElement *pZBE = _ZeBank->getElementByZoneName (_Zones[x+y*stride].ZoneName);
		if ((pZBE != NULL) && (pZBE->getCategory("material")==_Zones[x+y*stride].SharingMatNames[0]))
			return;
		_ZeBank->resetSelection ();
		_ZeBank->addOrSwitch ("material", _Zones[x+y*stride].SharingMatNames[0]);
		_ZeBank->addAndSwitch ("size", "1x1");
		vector<CZoneBankElement*> vElts;
		_ZeBank->getSelection (vElts);
		if (vElts.size() == 0)
			return;
		sint32 nRan = (sint32)(NLMISC::frand((float)vElts.size()));
		NLMISC::clamp (nRan, (sint32)0, (sint32)(vElts.size()-1));
		pZBE = vElts[nRan];
		nRan = (uint32)(NLMISC::frand (1.0) * 4);
		NLMISC::clamp (nRan, (sint32)0, (sint32)3);
		uint8 rot = (uint8)nRan;
		nRan = (uint32)(NLMISC::frand (1.0) * 2);
		NLMISC::clamp (nRan, (sint32)0, (sint32)1);
		uint8 flip = (uint8)nRan;

		set (x+_MinX, y+_MinY, 0, 0, pZBE->getName(), false);
		setRot (x+_MinX, y+_MinY, rot);
		setFlip (x+_MinX, y+_MinY, flip);
		return;
	}

	// No 2 materials so the transition system dont work
	if (matNameSet.size() != 2)
		return;

	std::set<string>::iterator it = matNameSet.begin();
	string sMatA = *it;
	++it;
	string sMatB = *it;

	_ZeBank->resetSelection ();
	_ZeBank->addOrSwitch ("transname", sMatA + "-" + sMatB);
	vector<CZoneBankElement*> selection;
	_ZeBank->getSelection (selection);
	if (selection.size() == 0)
	{
		string sTmp = sMatA;
		sMatA = sMatB;
		sMatB = sTmp;
		_ZeBank->resetSelection ();
		_ZeBank->addOrSwitch ("transname", sMatA + "-" + sMatB);
		_ZeBank->getSelection (selection);
	}

	if (selection.size() == 0)
		return;

	// Convert the sharingCutEdges and SharingNames to the num and type of transition
	uint8 nQuart = 0; // 0-MatA 1-MatB
	for (m = 0; m < 4; ++m)
		if (_Zones[x+y*stride].SharingMatNames[m] == sMatB)
			nQuart |= (1<<m);

	if ((nQuart == 0)||(nQuart == 6)||(nQuart == 9)||(nQuart == 15))
		return; // No transition for those types

	uint8 nCutEdge = 0;
	uint8 nPosCorner = 0;

	// If up edge is cut write the cut position in nCutEdge bitfield (1->0, 2->1)
	if ((nQuart == 4)||(nQuart == 5)||(nQuart == 7)||(nQuart == 8)||(nQuart == 10)||(nQuart == 11))
	{
		if (_Zones[x+y*stride].SharingCutEdges[0] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}

	// Same for down edge
	if ((nQuart == 1)||(nQuart == 2)||(nQuart == 5)||(nQuart == 10)||(nQuart == 13)||(nQuart == 14))
	{
		if (_Zones[x+y*stride].SharingCutEdges[1] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}

	// Same for left edge
	if ((nQuart == 1)||(nQuart == 3)||(nQuart == 4)||(nQuart == 11)||(nQuart == 12)||(nQuart == 14))
	{
		if (_Zones[x+y*stride].SharingCutEdges[2] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}

	// Same for right edge
	if ((nQuart == 2)||(nQuart == 3)||(nQuart == 7)||(nQuart == 8)||(nQuart == 12)||(nQuart == 13))
	{
		if (_Zones[x+y*stride].SharingCutEdges[3] == 2)
			nCutEdge |= 1 << nPosCorner;
		++nPosCorner;
	}

	nlassert (nPosCorner == 2); // If not this means that more than 2 edges are cut which is nont possible

	STrans Trans, TransTmp1, TransTmp2;

	TransTmp1 = TranConvTable[nQuart*8+2*nCutEdge+0];
	TransTmp2 = TranConvTable[nQuart*8+2*nCutEdge+1];

	// Choose one or the two
	sint32 nTrans = (sint32)(NLMISC::frand(2.0f));
	NLMISC::clamp (nTrans, (sint32)0, (sint32)1);
	if (nTrans == 0)
		Trans = TransTmp1;
	else
		Trans = TransTmp2;

	_ZeBank->addAndSwitch ("transnum", NLMISC::toString(Trans.Num));
	_ZeBank->getSelection (selection);

	if (selection.size() > 0)
	{
		nTrans = (uint32)(NLMISC::frand (1.0) * selection.size());
		NLMISC::clamp (nTrans, (sint32)0, (sint32)(selection.size()-1));

		_Zones[x+y*stride].ZoneName = selection[nTrans]->getName();
		_Zones[x+y*stride].PosX = _Zones[x+y*stride].PosY = 0;
		_Zones[x+y*stride].Rot = Trans.Rot;
		_Zones[x+y*stride].Flip = Trans.Flip;
	}
}

// ---------------------------------------------------------------------------
string CBuilderZoneRegion::getNextMatInTree (const string &sMatA, const string &sMatB)
{
	uint32 i, posA = 10000, posB = 10000;

	if (sMatA == sMatB)
		return sMatA;

	for (i = 0; i < _MatTree.size(); ++i)
	{
		if (_MatTree[i].Name == sMatA) 
			posA = i;
		if (_MatTree[i].Name == sMatB) 
			posB = i;
	}
	if ((posA == 10000) || (posB == 10000))
		return STRING_UNUSED;
	
	vector<uint32> vTemp;
	tryPath (posA, posB, vTemp);
	if (vTemp.size() <= 1)
		return STRING_UNUSED;
	else
		return _MatTree[vTemp[1]].Name;
}

// ---------------------------------------------------------------------------
struct SNode
{
	sint32 NodePos, Dist, PrevNodePos;

	SNode()
	{
		NodePos = Dist = PrevNodePos = -1;
	}
};

// ---------------------------------------------------------------------------
// Find the fastest way between posA and posB in the MatTree (Dijkstra)
void CBuilderZoneRegion::tryPath (uint32 posA, uint32 posB, vector<uint32> &vPath)
{
	// Build the adjascence matrix
	vector<sint32> matAdj;
	sint32 nNumNodes = _MatTree.size();
	sint32 i, j, cost;
	matAdj.resize (nNumNodes*nNumNodes, -1);
	for (i = 0; i < nNumNodes; ++i)
		for (j = 0; j < (sint32)_MatTree[i].Arcs.size(); ++j)
			matAdj[i+_MatTree[i].Arcs[j]*nNumNodes] = 1;

	vector<SNode> vNodes; // NodesPos == index
	vNodes.resize (nNumNodes);
	for (i = 0; i < nNumNodes; ++i)
		vNodes[i].NodePos = i;
	vNodes[posA].Dist = 0;

	queue<SNode> qNodes;
	qNodes.push (vNodes[posA]);

	while (qNodes.size() > 0)
	{
		SNode node = qNodes.front ();
		qNodes.pop ();

		for (i = 0; i < nNumNodes; ++i)
		{
			cost = matAdj[node.NodePos+i*nNumNodes];
			if (cost != -1)	
			{
				if ((vNodes[i].Dist == -1) || (vNodes[i].Dist > (cost+node.Dist)))
				{
					vNodes[i].Dist = cost+node.Dist;
					vNodes[i].PrevNodePos = node.NodePos;
					qNodes.push (vNodes[i]);
				}
			}
		}
	}

	// Get path length
	i = posB;
	j = 0;
	while (i != -1)
	{
		++j;
		i = vNodes[i].PrevNodePos;
	}

	// Write the path in the good order (from posA to posB)
	vPath.resize (j);
	i = posB;
	while (i != -1)
	{
		--j;
		vPath[j] = i;
		i = vNodes[i].PrevNodePos;
	}
}



// ---------------------------------------------------------------------------
void CBuilderZoneRegion::del (sint32 x, sint32 y, bool transition, void *pInternal)
{
	if (!_Builder->getZoneMask (x,y))
		return;
	
	const string &rSZone = getName (x, y);

	CToUpdate *pUpdate = (CToUpdate *)pInternal;

	CZoneBankElement *pZBE = _ZeBank->getElementByZoneName (rSZone);
	if (pZBE != NULL)
	{
		sint32 sizeX = pZBE->getSizeX(), sizeY = pZBE->getSizeY();
		sint32 posX = getPosX (x, y), posY = getPosY (x, y);
		uint8 rot = getRot (x, y);
		uint8 flip = getFlip (x, y);
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
		rotFlip (sMask, rot, flip);

		for (j = 0; j < sMask.h; ++j)
		for (i = 0; i < sMask.w; ++i)
		if (sMask.Tab[i+j*sMask.w])
		{
			set (x+deltaX+i, y+deltaY+j, 0, 0, STRING_UNUSED, true);
			setRot (x+deltaX+i, y+deltaY+j, 0);
			setFlip (x+deltaX+i, y+deltaY+j, 0);
			if (pUpdate != NULL)
			{
				pUpdate->add (this, x+deltaX+i, y+deltaY+j, "");
			}
		}
		if (!transition)
			reduceMin ();
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::serial (NLMISC::IStream &f)
{
	sint32 version = f.serialVersion (0);
	f.serialCheck ((uint32)'DNAL');
	f.serial (_MinX);
	f.serial (_MinY);
	f.serial (_MaxX);
	f.serial (_MaxY);
	f.serialCont(_Zones);
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::move (sint32 x, sint32 y)
{
	_MinX += x;
	_MinY += y;
	_MaxX += x;
	_MaxY += y;
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::set (sint32 x, sint32 y, sint32 PosX, sint32 PosY, 
						const std::string &ZoneName, bool transition)
{
	// Do we need to resize ?
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		sint32 newMinX = (x<_MinX?x:_MinX), newMinY = (y<_MinY?y:_MinY);
		sint32 newMaxX = (x>_MaxX?x:_MaxX), newMaxY = (y>_MaxY?y:_MaxY);

		resize (newMinX, newMaxX, newMinY, newMaxY);
	}
	sint32 stride = (1+_MaxX-_MinX); // Nb to go to next line

	_Zones[(x-_MinX)+(y-_MinY)*stride].ZoneName = ZoneName;
	_Zones[(x-_MinX)+(y-_MinY)*stride].PosX = (uint8)PosX;
	_Zones[(x-_MinX)+(y-_MinY)*stride].PosY = (uint8)PosY;
	if (!transition)
	{
		CZoneBankElement *pZBE = _ZeBank->getElementByZoneName (ZoneName);
		if (pZBE == NULL)
			return;
		const string &sMatName = pZBE->getCategory ("material");
		if (sMatName == STRING_NO_CAT_TYPE)
			return;
		for (uint32 i = 0; i < 4; ++i)
		{
			_Zones[(x-_MinX)+(y-_MinY)*stride].SharingMatNames[i] = sMatName;
			_Zones[(x-_MinX)+(y-_MinY)*stride].SharingCutEdges[i] = 0;
		}

		CBuilderZoneRegion *pBZR = this;
		SZoneUnit *pZU = getZoneAmongRegions (pBZR, x-1, y-1);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[3] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x, y-1);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[2] = sMatName;
			pZU->SharingMatNames[3] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x+1, y-1);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[2] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x-1, y);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[1] = sMatName;
			pZU->SharingMatNames[3] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x+1, y);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[0] = sMatName;
			pZU->SharingMatNames[2] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x-1, y+1);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[1] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x, y+1);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[0] = sMatName;
			pZU->SharingMatNames[1] = sMatName;
		}
		pBZR = this;
		pZU = getZoneAmongRegions (pBZR, x+1, y+1);
		if (pZU != NULL)
		{
			pZU->SharingMatNames[0] = sMatName;
		}

		/*
		if (x > _MinX)
		{
			_Zones[(x-1-_MinX)+(y-_MinY)*stride].SharingMatNames[1] = sMatName;
			_Zones[(x-1-_MinX)+(y-_MinY)*stride].SharingMatNames[3] = sMatName;
			if (y > _MinY)
				_Zones[(x-1-_MinX)+(y-1-_MinY)*stride].SharingMatNames[3] = sMatName;
			if (y < _MaxY)
				_Zones[(x-1-_MinX)+(y+1-_MinY)*stride].SharingMatNames[1] = sMatName;
		}
		if (y > _MinY)
		{
			_Zones[(x-_MinX)+(y-1-_MinY)*stride].SharingMatNames[2] = sMatName;
			_Zones[(x-_MinX)+(y-1-_MinY)*stride].SharingMatNames[3] = sMatName;
		}
		if (y < _MaxY)
		{
			_Zones[(x-_MinX)+(y+1-_MinY)*stride].SharingMatNames[0] = sMatName;
			_Zones[(x-_MinX)+(y+1-_MinY)*stride].SharingMatNames[1] = sMatName;
		}
		if (x < _MaxX)
		{
			_Zones[(x+1-_MinX)+(y-_MinY)*stride].SharingMatNames[0] = sMatName;
			_Zones[(x+1-_MinX)+(y-_MinY)*stride].SharingMatNames[2] = sMatName;
			if (y > _MinY)
				_Zones[(x+1-_MinX)+(y-1-_MinY)*stride].SharingMatNames[2] = sMatName;
			if (y < _MaxY)
				_Zones[(x+1-_MinX)+(y+1-_MinY)*stride].SharingMatNames[0] = sMatName;
		}
		*/
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::setRot (sint32 x, sint32 y, uint8 rot)
{
	_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Rot = rot;

}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::setFlip (sint32 x, sint32 y, uint8 flip)
{
	_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Flip = flip;
}


// ---------------------------------------------------------------------------
void CBuilderZoneRegion::reduceMin ()
{
	sint32 i, j;
	sint32 Stride = 1+_MaxX-_MinX;
	sint32 newMinX = _MinX, newMinY = _MinY;
	sint32 newMaxX = _MaxX, newMaxY = _MaxY;
	bool bCanSuppr;

	// Reduce the MinY
	while (true)
	{
		if (newMinY == newMaxY)
			break;
		j = newMinY;
		bCanSuppr = true;
		for (i = newMinX; i <= newMaxX; ++i)
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)].ZoneName != STRING_UNUSED)
			{
				bCanSuppr = false;
				break;
			}
		if (bCanSuppr)
			++newMinY;
		else
			break;
	}

	// Reduce the MaxY
	while (true)
	{
		if (newMinY == newMaxY)
			break;
		j = newMaxY;
		bCanSuppr = true;
		for (i = newMinX; i <= newMaxX; ++i)
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)].ZoneName != STRING_UNUSED)
			{
				bCanSuppr = false;
				break;
			}
		if (bCanSuppr)
			--newMaxY;
		else
			break;
	}

	// Reduce the MinX
	while (true)
	{
		if (newMinX == newMaxX)
			break;
		i = newMinX;
		bCanSuppr = true;
		for (j = newMinY; j <= newMaxY; ++j)
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)].ZoneName != STRING_UNUSED)
			{
				bCanSuppr = false;
				break;
			}
		if (bCanSuppr)
			++newMinX;
		else
			break;
	}

	// Reduce the MaxX
	while (true)
	{
		if (newMinX == newMaxX)
			break;
		i = newMaxX;
		bCanSuppr = true;
		for (j = newMinY; j <= newMaxY; ++j)
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)].ZoneName != STRING_UNUSED)
			{
				bCanSuppr = false;
				break;
			}
		if (bCanSuppr)
			--newMaxX;
		else
			break;
	}
	resize (newMinX, newMaxX, newMinY, newMaxY);
}

// ---------------------------------------------------------------------------
const string &CBuilderZoneRegion::getName (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return _StringOutOfBound;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].ZoneName;
	}
}

// ---------------------------------------------------------------------------
uint8 CBuilderZoneRegion::getPosX (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosX;
	}
}

// ---------------------------------------------------------------------------
uint8 CBuilderZoneRegion::getPosY (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].PosY;
	}
}

// ---------------------------------------------------------------------------
uint8 CBuilderZoneRegion::getRot (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Rot;
	}
}

// ---------------------------------------------------------------------------
uint8 CBuilderZoneRegion::getFlip (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return 0;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)].Flip;
	}
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::setStart (sint32 x, sint32 y)
{
	_MinX = _MaxX = x;
	_MinY = _MaxY = y;
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// PRIVATE
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	sint32 i, j;
	vector<SZoneUnit> newZones;
	SZoneUnit zuTmp;

	newZones.resize ((1+newMaxX-newMinX)*(1+newMaxY-newMinY));
	sint32 newStride = 1+newMaxX-newMinX;
	sint32 Stride = 1+_MaxX-_MinX;
	for (j = newMinY; j <= newMaxY; ++j)
	for (i = newMinX; i <= newMaxX; ++i)
	{
		if ((i >= _MinX)&&(i <= _MaxX)&&(j >= _MinY)&&(j <= _MaxY))
		{
			newZones[(i-newMinX)+(j-newMinY)*newStride] = _Zones[(i-_MinX)+(j-_MinY)*Stride];
		}
		else
		{
			zuTmp.ZoneName = STRING_UNUSED;
			zuTmp.PosX = 0;
			zuTmp.PosY = 0;
			newZones[(i-newMinX)+(j-newMinY)*newStride] = zuTmp;
		}
	}
	_MinX = newMinX; _MaxX = newMaxX;
	_MinY = newMinY; _MaxY = newMaxY;
	_Zones = newZones;
}

// ---------------------------------------------------------------------------
void CBuilderZoneRegion::rotFlip (SPiece &piece, uint8 rot, uint8 flip)
{
	uint8 nTmp;
	sint32 i, j;

	if (flip == 1)
	{
		for (j = 0; j < piece.h; ++j)
		for (i = 0; i < (piece.w/2); ++i)
		{
			nTmp = piece.Tab[i+j*piece.w];
			piece.Tab[i+j*piece.w] = piece.Tab[(piece.w-1-i)+j*piece.w];
			piece.Tab[(piece.w-1-i)+j*piece.w] = nTmp;
		}
	}

	if (rot == 1)
	{
		vector<uint8> TabDest;
		TabDest.resize (piece.Tab.size());
		for (j = 0; j < piece.h; ++j)
		for (i = 0; i < piece.w;  ++i)
			TabDest[j+i*piece.h] = piece.Tab[i+(piece.h-1-j)*piece.w];
		piece.Tab = TabDest;
		i = piece.w;
		piece.w = piece.h;
		piece.h = i;
	}

	if (rot == 2)
	{
		for (j = 0; j < (piece.h/2); ++j)
		for (i = 0; i < piece.w; ++i)
		{
			nTmp = piece.Tab[i+j*piece.w];
			piece.Tab[i+j*piece.w] = piece.Tab[(piece.w-1-i)+(piece.h-1-j)*piece.w];
			piece.Tab[(piece.w-1-i)+(piece.h-1-j)*piece.w] = nTmp;
		}
		if ((piece.h/2)*2 != piece.h)
		{
			j = (piece.h/2);
			for (i = 0; i < (piece.w/2); ++i)
			{
				nTmp = piece.Tab[i+j*piece.w];
				piece.Tab[i+j*piece.w] = piece.Tab[(piece.w-1-i)+j*piece.w];
				piece.Tab[(piece.w-1-i)+j*piece.w] = nTmp;
			}
		}
	}

	if (rot == 3)
	{
		vector<uint8> TabDest;
		TabDest.resize (piece.Tab.size());
		for (j = 0; j < piece.h; ++j)
		for (i = 0; i < piece.w;  ++i)
			TabDest[j+i*piece.h] = piece.Tab[piece.w-1-i+j*piece.w];
		piece.Tab = TabDest;
		i = piece.w;
		piece.w = piece.h;
		piece.h = i;
	}
}
