
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
	pWinBitmap->CreateBitmap (pTF->getWidth(), pTF->getHeight(), 1, 32, pNewPixel);
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
// CBuilderZone
// ***************************************************************************

// ---------------------------------------------------------------------------
CBuilderZone::CBuilderZone ()
{
	// Init the ZoneBank
	_ZoneBank.debugInit ();
	initZoneBank ("ZoneLigos");


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
	_CurSelectedZone = -1;
	_ApplyRot = 0;
	_ApplyRotRan = false;
	_ApplyFlip = 0;
	_ApplyFlipRan = false;
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

	_ZoneBank.getSelection (_CurrentSelection);

	// Create the corresponding image list from selected item using DataBase

	vector<CBitmap*> vIL;
	vIL.resize (_CurrentSelection.size());
	for (i = 0; i < _CurrentSelection.size(); ++i)
	{
		CZoneBankElement *pElt = _CurrentSelection[i];
		// Get bitmap from DataBase
		vIL[i] = _DataBase.getBitmap (pElt->getName());
	}

	// Construct the tree add first items then the images!
	_ToolsZone->getListCtrl()->reset ();
	for (i = 0; i < _CurrentSelection.size(); ++i)
	{
		CZoneBankElement *pElt = _CurrentSelection[i];
		_ToolsZone->getListCtrl()->addItem (pElt->getName());
	}
	_ToolsZone->getListCtrl()->setImages (vIL);
	_CurSelectedZone = -1;
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

			PB.setNumTri (2);
			PB.setTri (0, 0, 1, 2);
			PB.setTri (1, 0, 2, 3);

			sint32 x = (sint32)floor(minx / _Display->_CellSize);
			sint32 y = (sint32)floor(miny / _Display->_CellSize);

			const string &rSZone = _ZoneRegion.getName (x, y);
			CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (rSZone);
			float uMin, vMin, uMax, vMax;
			if (pZBE == NULL)
			{
				Mat.setTexture (0, _DataBase.getTexture (rSZone));
				uMin = 0.0f;
				vMin = 1.0f - 0.0f;
				uMax = 1.0f;
				vMax = 1.0f - 1.0f;
				VB.setTexCoord (0, 0, CUV(uMin, vMin));
				VB.setTexCoord (1, 0, CUV(uMax, vMin));
				VB.setTexCoord (2, 0, CUV(uMax, vMax));
				VB.setTexCoord (3, 0, CUV(uMin, vMax));
			}
			else
			{
				Mat.setTexture (0, _DataBase.getTexture (rSZone));
				uMin = ((float)_ZoneRegion.getPosX (x, y)) / pZBE->getSizeX();
				vMin = 1.0f - ((float)_ZoneRegion.getPosY (x, y)) / pZBE->getSizeY();
				uMax = ((float)_ZoneRegion.getPosX (x, y)+1.0f) / pZBE->getSizeX();
				vMax = 1.0f - ((float)_ZoneRegion.getPosY (x, y)+1.0f) / pZBE->getSizeY();

				if (_ZoneRegion.getFlip (x, y) == 1)
				{
					float rTmp = uMin;
					uMin = uMax;
					uMax = rTmp;
				}

				VB.setTexCoord ((_ZoneRegion.getRot (x, y)+0)%4, 0, CUV(uMin, vMin));
				VB.setTexCoord ((_ZoneRegion.getRot (x, y)+1)%4, 0, CUV(uMax, vMin));
				VB.setTexCoord ((_ZoneRegion.getRot (x, y)+2)%4, 0, CUV(uMax, vMax));
				VB.setTexCoord ((_ZoneRegion.getRot (x, y)+3)%4, 0, CUV(uMin, vMax));
			}
			
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
	uint8 rot, flip;

	if (_RandomSelection)
	{
		if (_CurrentSelection.size() > 0)
		{
			uint32 nSel = (uint32)(NLMISC::frand (1.0) * _CurrentSelection.size());
			NLMISC::clamp (nSel, (uint32)0, (uint32)(_CurrentSelection.size()-1));
			_CurSelectedZone = nSel;
		}
	}

	if (_ApplyRotRan)
	{
		uint32 nSel = (uint32)(NLMISC::frand (1.0) * 4);
		NLMISC::clamp (nSel, (uint32)0, (uint32)3);
		rot = (uint8)nSel;
	}
	else
	{
		rot = _ApplyRot;
	}

	if (_ApplyFlipRan)
	{
		uint32 nSel = (uint32)(NLMISC::frand (1.0) * 2);
		NLMISC::clamp (nSel, (uint32)0, (uint32)1);
		flip = (uint8)nSel;
	}
	else
	{
		flip = _ApplyFlip;
	}

	if ((_CurSelectedZone >= 0)&&(_CurSelectedZone <= ((sint32)_CurrentSelection.size()-1)))
	{
		CZoneBankElement *pZBE = _CurrentSelection[_CurSelectedZone];
		_ZoneRegion.init (&_ZoneBank);
		_ZoneRegion.add (x, y, rot, flip, pZBE);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::del (CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);
	_ZoneRegion.init (&_ZoneBank);
	_ZoneRegion.del (x, y);
}

// ---------------------------------------------------------------------------
bool CBuilderZone::initZoneBank (const string &pathName)
{
	char sDirBackup[512];
	char sDirNew[512];
	GetCurrentDirectory (512, sDirBackup);
	strcpy (sDirNew, sDirBackup);
	strcat (sDirNew, "\\");
	strcat (sDirNew, pathName.c_str());
	SetCurrentDirectory (sDirNew);
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	hFind = FindFirstFile ("*.ligozone", &findData);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		// If the name of the file is not . or .. then its a valid entry in the DataBase
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			_ZoneBank.addElement (findData.cFileName);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	SetCurrentDirectory (sDirBackup);
	return true;
}



/*
// ---------------------------------------------------------------------------
void CBuilderZone::setTrans (sint32 x, sint32 y, CZoneBankElement *pZBE)
{
}

// ---------------------------------------------------------------------------
void CBuilderZone::placeRandomTrans (sint32 x, sint32 y, string TransNameVal)
{	
	vector<CZoneBankElement*> Selection;
	_ZoneBank.getSelection (Selection);
	if (Selection.size() == 0)
		return;

	string MatA, MatB;
	sint32 m, n;

	for (m = 0; m < (sint32)TransNameVal.size(); ++m)
	{
		if (TransNameVal[m] == '_')
			break;
		MatA += TransNameVal[m];
	}
	++m;
	for (; m < (sint32)TransNameVal.size(); ++m)
		MatB += TransNameVal[m];

	CZoneBankElement* localMap[9];
	for (m = -1; m <= 1; ++m)
	for (n = -1; n <= 1; ++n)
	{
		const string &rSZone = _ZoneRegion.getName (x+n, y+m);
		CZoneBankElement *pElt = _ZoneBank.getElementByZoneName (rSZone);
		localMap[n+1+(m+1)*3] = pElt;
	}

	// If we have a material different up and down
	if ((((localMap[1] != NULL)&&(localMap[7] != NULL)) &&
		(((localMap[1]->getCategory("Material") == MatA) && (localMap[7]->getCategory("Material") == MatB)) ||
		((localMap[1]->getCategory("Material") == MatB) && (localMap[7]->getCategory("Material") == MatA)))) ||
		(((localMap[3] != NULL)&&(localMap[5] != NULL)) &&
		(((localMap[3]->getCategory("Material") == MatA) && (localMap[5]->getCategory("Material") == MatB)) ||
		((localMap[3]->getCategory("Material") == MatB) && (localMap[5]->getCategory("Material") == MatA)))))
	{
		_ZoneBank.addAndSwitch ("TransType", "Flat");
		_ZoneBank.getSelection (Selection);
		if (Selection.size() == 0)
			return;

		// Select the tile
		uint32 nSel = (uint32)(NLMISC::frand (1.0) * Selection.size());
		NLMISC::clamp (nSel, (uint32)0, (uint32)(Selection.size()-1));
		
		CZoneBankElement *pZBE = Selection[nSel];

		_ZoneRegion.set (x, y, 0, 0, pZBE->getName());
		nSel = (uint32)(NLMISC::frand (1.0) * 2);
		NLMISC::clamp (nSel, (uint32)0, (uint32)1);
		if (nSel)
			_ZoneRegion.setFlip (x, y, 1);
		if ((localMap[1] != NULL)&&(localMap[1]->getCategory("Material") != STRING_NO_CAT_TYPE))
		{
			if (localMap[1]->getCategory("Material") == MatA)
			{
				_ZoneRegion.setRot (x, y, 2);
			}
			else
			{
				_ZoneRegion.setRot (x, y, 0);
			}

			// Update Left and Right transition if any
			//if ((localMap[3] != NULL)&&(localMap[3]->getCategory("TransName") != STRING_NO_CAT_TYPE))
			//{
			//	const string &rNum = pZBE->getCategory("TransNum");
			//	if (rNum == "11")
			//	{
			//		if (_ZoneRegion.getRot(x,y) == 0)
			//			updateTrans (x-1, y, 1, ); // (x,y,RIGHT,
			//	}
			//	localMap[3]->getCategory ("")
			//}
		}
		else
		{
			if (localMap[3]->getCategory("Material") == MatA)
				_ZoneRegion.setRot (x, y, 1);
			else
				_ZoneRegion.setRot (x, y, 3);
		}



		return;
	}
	
}*/


// ---------------------------------------------------------------------------
/*
void CBuilderZone::putAndSolve (sint32 x, sint32 y, CZoneBankElement *pZBE)
{
	uint32 i, j;
	sint32 k, l;
	sint32 m, n;

	// 1st - Suppress already present tiles
	for (j = 0; j < pZBE->getSizeY(); ++j)
	for (i = 0; i < pZBE->getSizeX(); ++i)
	if (pZBE->getMask()[i+j*pZBE->getSizeX()])
	{
		removeAndSolve (x+i, y+j);
	}

	// 2nd - Suppress all stuff around that is not from the same material
	const string &CurMat = pZBE->getCategory ("Material");
	for (j = 0; j < pZBE->getSizeY(); ++j)
	for (i = 0; i < pZBE->getSizeX(); ++i)
	if (pZBE->getMask()[i+j*pZBE->getSizeX()])
	{
		for (k = -1; k <= 1; ++k)
		for (l = -1; l <= 1; ++l)
		{
			const string &rSZone = _ZoneRegion.getName (x+i+l, y+j+k);
			CZoneBankElement *pZBE2 = _ZoneBank.getElementByZoneName (rSZone);

			if (pZBE2 != NULL)
			{
				const string &Mat = pZBE2->getCategory ("Material");
				if (Mat != CurMat)
				{
					removeAndSolve (x+i+l, y+j+k);
				}
			}
		}
	}

	// 3rd - Put the new tile
	for (j = 0; j < pZBE->getSizeY(); ++j)
	for (i = 0; i < pZBE->getSizeX(); ++i)
	if (pZBE->getMask()[i+j*pZBE->getSizeX()])
	{
		_ZoneRegion.set (x+i, y+j, i, j, pZBE->getName());
		_ZoneRegion.setRot (x+i, y+j, 0);
		_ZoneRegion.setFlip (x+i, y+j, 0);
	}

	// 4th - Put direct transition
	for (j = 0; j < pZBE->getSizeY(); ++j)
	for (i = 0; i < pZBE->getSizeX(); ++i)
	if (pZBE->getMask()[i+j*pZBE->getSizeX()])
	{
		for (k = -1; k <= 1; ++k)
		for (l = -1; l <= 1; ++l)
		{
			const string &rSZone = _ZoneRegion.getName (x+i+l, y+j+k);
			if (rSZone == STRING_UNUSED)
			{
				// Calculate the number of material around
				set<string> matNameSet;
				for (m = -1; m <= 1; ++m)
				for (n = -1; n <= 1; ++n)
				{
					const string &rSZone2 = _ZoneRegion.getName (x+i+l+n, y+j+k+m);
					CZoneBankElement *pElt = _ZoneBank.getElementByZoneName (rSZone2);
					if (pElt != NULL)
					{
						const string &rMatName = pElt->getCategory ("Material");
						if (rMatName != STRING_NO_CAT_TYPE)
						{
							matNameSet.insert (rMatName);
						}
					}
				}
				if (matNameSet.size() == 2)
				{
					set<string>::iterator it = matNameSet.begin();
					string sTmp = *it;
					++it;
					sTmp += "_" + *it;
					_ZoneBank.resetSelection ();
					_ZoneBank.addOrSwitch ("TransName", sTmp);
					placeRandomTrans (x+i+l, y+j+k, sTmp);
					it = matNameSet.begin();
					sTmp = *it;
					++it;
					sTmp = *it + "_" + sTmp;
					_ZoneBank.resetSelection ();
					_ZoneBank.addOrSwitch ("TransName", sTmp);
					placeRandomTrans (x+i+l, y+j+k, sTmp);
				}
			}
		}
	}

	// 5th - Put corner transition
}
*/
