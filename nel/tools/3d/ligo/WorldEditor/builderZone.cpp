
#include "stdafx.h"

#include "builderZone.h"
#include "display.h"
#include "toolsZone.h"
#include "../lib/primitive.h"

#include "nel/misc/vector.h"

#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"

#include "3d/nelu.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "3d/texture_file.h"

using namespace NLLIGO;
using namespace std;

// Because we cant use the namespace NLMISC or NL3D in global due to some collision (CBitmap,...)
using NLMISC::CIFile;
using NLMISC::Exception;
using NLMISC::CVector;
using NLMISC::CUV;
using NLMISC::CMatrix;
using NLMISC::CSmartPtr;

using NL3D::CTextureFile;
using NL3D::CVertexBuffer;
using NL3D::CPrimitiveBlock;
using NL3D::CMaterial;
using NL3D::CViewport;
using NL3D::CNELU;

// ***************************************************************************
// CDataBase
// ***************************************************************************

// ---------------------------------------------------------------------------
CDataBase::CDataBase ()
{
}

// ---------------------------------------------------------------------------
CDataBase::~CDataBase ()
{
}

// ---------------------------------------------------------------------------
bool CDataBase::initFromPath (const string &Path)
{
	char sDirBackup[512];
	char sDirNew[512];
	GetCurrentDirectory (512, sDirBackup);
	strcpy (sDirNew, sDirBackup);
	strcat (sDirNew, "\\");
	strcat (sDirNew, Path.c_str());
	SetCurrentDirectory (sDirNew);
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	hFind = FindFirstFile ("*.*", &findData);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		// If the name of the file is not . or .. then its a valid entry in the DataBase
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			SElement zdbTmp;

			// Read the texture file
			zdbTmp.Name = findData.cFileName;
			zdbTmp.Texture = loadTexture (zdbTmp.Name + ".TGA");
			zdbTmp.WinBitmap = convertToWin (zdbTmp.Texture);

			// Add the entry in the DataBase
			_ZoneDB.push_back (zdbTmp);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	SetCurrentDirectory (sDirBackup);
	return true;
}

// ---------------------------------------------------------------------------
bool CDataBase::init (const string &Path, CZoneBank &zb)
{
	char sDirBackup[512];
	char sDirNew[512];
	GetCurrentDirectory (512, sDirBackup);
	strcpy (sDirNew, sDirBackup);
	strcat (sDirNew, "\\");
	strcat (sDirNew, Path.c_str());
	SetCurrentDirectory (sDirNew);

	vector<string> ZoneNames;
	zb.getCategoryValues ("Zone", ZoneNames);
	for (uint32 i = 0; i < ZoneNames.size(); ++i)
	{
		SElement zdbTmp;

		// Read the texture file
		zdbTmp.Name = ZoneNames[i];
		zdbTmp.Texture = loadTexture (zdbTmp.Name + ".TGA");
		zdbTmp.WinBitmap = convertToWin (zdbTmp.Texture);

		// Add the entry in the DataBase
		_ZoneDB.push_back (zdbTmp);
	}
	UnusedTexture = loadTexture ("_UNUSED_.TGA");
	SetCurrentDirectory (sDirBackup);
	return true;
}

// ---------------------------------------------------------------------------
CBitmap *CDataBase::getBitmap (const string &ZoneName)
{
	for (uint32 i = 0; i < _ZoneDB.size(); ++i)
		if (ZoneName == _ZoneDB[i].Name)
			return _ZoneDB[i].WinBitmap;
	return NULL;
}

// ---------------------------------------------------------------------------
CTextureFile *CDataBase::getTexture (const string &ZoneName)
{
	if (ZoneName == STRING_UNUSED)
		return UnusedTexture;
	for (uint32 i = 0; i < _ZoneDB.size(); ++i)
		if (ZoneName == _ZoneDB[i].Name)
			return _ZoneDB[i].Texture;
	return NULL;
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CBitmap *CDataBase::convertToWin (CTextureFile *pTF)
{
	CBitmap *pWinBitmap = new CBitmap;
	vector<uint8> &rPixel = pTF->getPixels();
	uint8 *pNewPixel = new uint8[rPixel.size()];
	for (uint32 i = 0; i < (rPixel.size()/4); ++i)
	{
		pNewPixel[i*4+0] = rPixel[i*4+2];
		pNewPixel[i*4+1] = rPixel[i*4+1];
		pNewPixel[i*4+2] = rPixel[i*4+0];
		pNewPixel[i*4+3] = rPixel[i*4+3];
	}
	pWinBitmap->CreateBitmap (64, 64, 1, 32, pNewPixel);
	return pWinBitmap;
}

// ---------------------------------------------------------------------------
CTextureFile *CDataBase::loadTexture (const std::string &fileName)
{
	CTextureFile *pTexture = new CTextureFile;
	pTexture->setFileName (fileName);
	pTexture->setReleasable (false);
	pTexture->generate ();
	return pTexture;
}

// ***************************************************************************
// CZoneRegion
// ***************************************************************************

std::string CZoneRegion::_StringOutOfBound;

// ---------------------------------------------------------------------------
CZoneRegion::CZoneRegion ()
{
	_StringOutOfBound = STRING_OUT_OF_BOUND;
	_MinX = _MinY = 0;
	_MaxX = _MaxY = 0;
	_Zones.push_back (STRING_UNUSED);
}

// ---------------------------------------------------------------------------
void CZoneRegion::resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY)
{
	sint32 i, j;
	vector<string> newZones;

	newZones.resize ((1+newMaxX-newMinX)*(1+newMaxY-newMinY));
	sint32 newStride = 1+newMaxX-newMinX;
	sint32 Stride = 1+_MaxX-_MinX;
	for (j = newMinY; j <= newMaxY; ++j)
	for (i = newMinX; i <= newMaxX; ++i)
	{
		if ((i >= _MinX)&&(i <= _MaxX)&&(j >= _MinY)&&(j <= _MaxY))
			newZones[(i-newMinX)+(j-newMinY)*newStride] = _Zones[(i-_MinX)+(j-_MinY)*Stride];
		else
			newZones[(i-newMinX)+(j-newMinY)*newStride] = STRING_UNUSED;
	}
	_MinX = newMinX; _MaxX = newMaxX;
	_MinY = newMinY; _MaxY = newMaxY;
	_Zones = newZones;
}

// ---------------------------------------------------------------------------
void CZoneRegion::set (sint32 x, sint32 y, const std::string &ZoneName)
{
	// Do we need to resize ?
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		sint32 newMinX = (x<_MinX?x:_MinX), newMinY = (y<_MinY?y:_MinY);
		sint32 newMaxX = (x>_MaxX?x:_MaxX), newMaxY = (y>_MaxY?y:_MaxY);

		resize (newMinX, newMaxX, newMinY, newMaxY);
	}

	_Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)] = ZoneName;
}

// ---------------------------------------------------------------------------
void CZoneRegion::reduceMin ()
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
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)] != STRING_UNUSED)
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
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)] != STRING_UNUSED)
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
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)] != STRING_UNUSED)
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
			if (_Zones[(i-_MinX)+(j-_MinY)*(1+_MaxX-_MinX)] != STRING_UNUSED)
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
const string &CZoneRegion::get (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return _StringOutOfBound;
	}
	else
	{
		return _Zones[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)];
	}
}

// ***************************************************************************
// CBuilderZone
// ***************************************************************************

// ---------------------------------------------------------------------------
CBuilderZone::CBuilderZone ()
{
	_ImageList = NULL;

	// Init the ZoneBank
	_ZoneBank.debugInit ();

	// Construct the DataBase (Parse the ZoneBitmaps directory)
	//_DataBase.initFromPath ("ZoneBitmaps");
	
	// Construct the DataBase from the ZoneBank
	_DataBase.init ("ZoneBitmaps", _ZoneBank);
	
	// Set Current Filter
	_FilterType1 = STRING_UNUSED; _FilterValue1 = "";
	_FilterType2 = STRING_UNUSED; _FilterValue2 = "";
	_FilterType3 = STRING_UNUSED; _FilterValue3 = "";
	_FilterType4 = STRING_UNUSED; _FilterValue4 = "";
	_FilterOperator2 = 0;
	_FilterOperator3 = 0;
	_FilterOperator4 = 0;
	_RandomSelection = false;
	_CurSelectedZone = STRING_UNUSED;
}

// ---------------------------------------------------------------------------
void CBuilderZone::setDisplay (CDisplay *pDisp)
{
	_Display = pDisp;
}

// ---------------------------------------------------------------------------
void CBuilderZone::setToolsZone (CToolsZone *pTool)
{
	_ToolsZone = pTool;
}

// ---------------------------------------------------------------------------
void CBuilderZone::updateToolsZone ()
{
	uint32 i;
	
	if (_ToolsZone == NULL)
		return;

	// Execute the filter
	_ZoneBank.resetSelection ();
	if(_FilterType1 != STRING_UNUSED)
		_ZoneBank.addOrSwitch (_FilterType1, _FilterValue1);

	if(_FilterType2 != STRING_UNUSED)
	{
		if (_FilterOperator2 == 0) // AND switch wanted
			_ZoneBank.addAndSwitch (_FilterType2, _FilterValue2);
		else // OR switch wanted
			_ZoneBank.addOrSwitch (_FilterType2, _FilterValue2);
	}

	if(_FilterType3 != STRING_UNUSED)
	{
		if (_FilterOperator3 == 0) // AND switch wanted
			_ZoneBank.addAndSwitch (_FilterType3, _FilterValue3);
		else // OR switch wanted
			_ZoneBank.addOrSwitch (_FilterType3, _FilterValue3);
	}

	if(_FilterType4 != STRING_UNUSED)
	{
		if (_FilterOperator4 == 0) // AND switch wanted
			_ZoneBank.addAndSwitch (_FilterType4, _FilterValue4);
		else // OR switch wanted
			_ZoneBank.addOrSwitch (_FilterType4, _FilterValue4);
	}

	vector<CZoneBankElement*> vZBE;
	_ZoneBank.getSelection (vZBE);

	// Create the corresponding image list from selected item using DataBase
	if (_ImageList != NULL)
		delete _ImageList;

	_ImageList = new CImageList;
	_ImageList->Create (64, 64, ILC_COLOR32, 0, 5);

	for (i = 0; i < vZBE.size(); ++i)
	{
		CZoneBankElement *pElt = vZBE[i];
		// Get bitmap from DataBase
		_ImageList->Add (_DataBase.getBitmap(pElt->getName()) , RGB(0, 0, 0));
	}

	// Construct the tree
	_ToolsZone->getTreeCtrl()->DeleteAllItems ();
	_ToolsZone->getTreeCtrl()->SetImageList (_ImageList, TVSIL_NORMAL);
	for (i = 0; i < vZBE.size(); ++i)
	{
		CZoneBankElement *pElt = vZBE[i];
		_ToolsZone->getTreeCtrl()->InsertItem( pElt->getName().c_str(), i, i);
	}
}

// ---------------------------------------------------------------------------
bool CBuilderZone::load(const char *fileName)
{
	return false;
}

// ---------------------------------------------------------------------------
bool CBuilderZone::save(const char *fileName)
{
	return false;
}

// ---------------------------------------------------------------------------
void CBuilderZone::render (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax)
{
	CVertexBuffer VB;
	CPrimitiveBlock PB;
	CMaterial Mat;

	Mat.initUnlit ();
	Mat.setBlend (false);
	VB.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag);

	// Select all blocks visible
	float minx = floorf(viewMin.x/_Display->_CellSize)*_Display->_CellSize;
	float miny = floorf(viewMin.y/_Display->_CellSize)*_Display->_CellSize;
	float maxx = ceilf(viewMax.x/_Display->_CellSize)*_Display->_CellSize;
	float maxy = ceilf(viewMax.y/_Display->_CellSize)*_Display->_CellSize;

	while (minx < maxx)
	{
		miny = floorf(viewMin.y/_Display->_CellSize)*_Display->_CellSize;
		while (miny < maxy)
		{
			CVector pos1, pos2, pos3, pos4;

			pos1.x = (minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos1.y = 0.0f;
			pos1.z = (miny-viewMin.y)/(viewMax.y-viewMin.y);

			pos2.x = (_Display->_CellSize+minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos2.y = 0.0f;
			pos2.z = (miny-viewMin.y)/(viewMax.y-viewMin.y);

			pos3.x = (_Display->_CellSize+minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos3.y = 0.0f;
			pos3.z = (_Display->_CellSize+miny-viewMin.y)/(viewMax.y-viewMin.y);

			pos4.x = (minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos4.y = 0.0f;
			pos4.z = (_Display->_CellSize+miny-viewMin.y)/(viewMax.y-viewMin.y);

			VB.setNumVertices (4);
			VB.setVertexCoord (0, pos1);
			VB.setVertexCoord (1, pos2);
			VB.setVertexCoord (2, pos3);
			VB.setVertexCoord (3, pos4);
			VB.setTexCoord (0, 0, CUV(0,1));
			VB.setTexCoord (1, 0, CUV(1,1));
			VB.setTexCoord (2, 0, CUV(1,0));
			VB.setTexCoord (3, 0, CUV(0,0));

			PB.setNumTri (2);
			PB.setTri (0, 0, 1, 2);
			PB.setTri (1, 0, 2, 3);

			sint32 x = (sint32)floor(minx / _Display->_CellSize);
			sint32 y = (sint32)floor(miny / _Display->_CellSize);

			const string &rSZone = _ZoneRegion.get (x, y);
			Mat.setTexture (0, _DataBase.getTexture (rSZone));

			CMatrix mtx;
			mtx.identity();
			CNELU::Driver->setupViewport (CViewport());
			CNELU::Driver->setupViewMatrix (mtx);
			CNELU::Driver->setupModelMatrix (mtx);
			CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
			CNELU::Driver->activeVertexBuffer (VB);
			CNELU::Driver->render (PB, Mat);

			miny += _Display->_CellSize;
		}
		minx += _Display->_CellSize;
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::add (CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);

	if (_RandomSelection)
	{
		vector<CZoneBankElement*> vZBE;
		_ZoneBank.getSelection (vZBE);
		if (vZBE.size() > 0)
		{
			uint32 nSel = (uint32)(NLMISC::frand (1.0) * vZBE.size());
			NLMISC::clamp (nSel, (uint32)0, (uint32)(vZBE.size()-1));
			_CurSelectedZone = vZBE[nSel]->getName();
		}
	}

	_ZoneRegion.set (x, y, _CurSelectedZone);
}

// ---------------------------------------------------------------------------
void CBuilderZone::del (CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);
	_ZoneRegion.set (x, y, STRING_UNUSED);
	_ZoneRegion.reduceMin ();
}
