
#include "stdafx.h"

#include "builderZone.h"
#include "display.h"
#include "toolsZone.h"
#include "../lib/primitive.h"

#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "3d/nelu.h"
#include "3d/texture_file.h"
#include "3d/texture_mem.h"

using namespace NLLIGO;
using namespace std;

// Because we cant use the namespace NLMISC or NL3D in global due to some collision (CBitmap,...)
using NLMISC::CIFile;
using NLMISC::Exception;
using NLMISC::CVector;
using NLMISC::CUV;
using NLMISC::CMatrix;
using NLMISC::CSmartPtr;
using NLMISC::CIFile;
using NLMISC::COFile;

using NL3D::ITexture;
using NL3D::CTextureMem;
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
CDataBase::SCacheTexture::SCacheTexture()
{
	Enabled = false;
}

// ---------------------------------------------------------------------------
bool CDataBase::SCacheTexture::isFull()
{
	if (!Enabled)
		return false;
	for (uint32 i = 0; i < FreePlace.size(); ++i)
		if (FreePlace[i])
			return false;
	return true;
}

// ***************************************************************************
// CDataBase
// ***************************************************************************

// ---------------------------------------------------------------------------
CDataBase::CDataBase ()
{
	_RefCacheTextureSizeX = _RefCacheTextureSizeY = 1024; // Size of the texture cache
	_RefSizeX = _RefSizeY = DATABASE_ZONE_SIZE; // Size of a zone in pixel
	_RefCacheTextureNbEltX = _RefCacheTextureSizeX / _RefSizeX;
	_RefCacheTextureNbEltY = _RefCacheTextureSizeY / _RefSizeY;
	for (uint32 i; i < 64; ++i)
		_CacheTexture[i].Enabled = false;
}

// ---------------------------------------------------------------------------
CDataBase::~CDataBase ()
{
}

// ---------------------------------------------------------------------------
bool CDataBase::initFromPath (const string &Path)
{
	/*
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
	*/
	return false;
}

// ---------------------------------------------------------------------------
bool CDataBase::init (const string &Path, CZoneBank &zb)
{
	char sDirBackup[512];
	GetCurrentDirectory (512, sDirBackup);
	SetCurrentDirectory (Path.c_str());
	uint32 i, m, n, o, p;
	uint8 k, l;

	vector<string> ZoneNames;
	zb.getCategoryValues ("zone", ZoneNames);
	for (i = 0; i < ZoneNames.size(); ++i)
	{
		SElement zdbTmp;
		CZoneBankElement *pZBE = zb.getElementByZoneName (ZoneNames[i]);
		// Read the texture file
		string zdbTmpName = ZoneNames[i];
		zdbTmp.SizeX = pZBE->getSizeX ();
		zdbTmp.SizeY = pZBE->getSizeY ();
		const vector<bool> &rMask = pZBE->getMask();

		NLMISC::CBitmap *pBitmap = loadBitmap (zdbTmpName + ".TGA");
		if (pBitmap == NULL)
			continue;

		zdbTmp.WinBitmap = convertToWin (pBitmap);
		pBitmap->flipV ();

		for (l = 0; l < zdbTmp.SizeY; ++l)
		for (k = 0; k < zdbTmp.SizeX; ++k)
		if (rMask[k+l*zdbTmp.SizeX])
		{
			SCacheZone czTmp;

			czTmp.PosX = k;
			czTmp.PosY = l;

			// Found first non full texture cache
			for (m = 0; m < 64; ++m)
			if (_CacheTexture[m].Enabled == false)
			{
				// Create the texture
				_CacheTexture[m].FreePlace.resize (_RefCacheTextureNbEltX*_RefCacheTextureNbEltY, true);
				_CacheTexture[m].Texture = new CTextureMem();
				_CacheTexture[m].PtrMem.resize (4*_RefCacheTextureSizeX*_RefCacheTextureSizeY);
				_CacheTexture[m].Texture->resize (_RefCacheTextureSizeX, _RefCacheTextureSizeY);
				_CacheTexture[m].Texture->setPointer (&_CacheTexture[m].PtrMem[0], 4*_RefCacheTextureSizeX*_RefCacheTextureSizeY,
											false, false);

				_CacheTexture[m].Enabled = true;
				break;
			}
			else
			{
				if (!_CacheTexture[m].isFull())
					break;
			}

			nlassert (m<64);

			// Found first place in this texture

			for (n = 0; n < _CacheTexture[m].FreePlace.size(); ++n)
			if (_CacheTexture[m].FreePlace[n])
			{
				sint32 xSrc = k*_RefSizeX;
				sint32 ySrc = l*_RefSizeY;
				sint32 xDst = (n%_RefCacheTextureNbEltX)*_RefSizeX;
				sint32 yDst = (n/_RefCacheTextureNbEltX)*_RefSizeY;
				uint8 *pSrc = &pBitmap->getPixels()[(xSrc+ySrc*pBitmap->getWidth())*4];
				uint8 *pDst = &_CacheTexture[m].PtrMem[(xDst+yDst*_RefCacheTextureSizeX)*4];
				// Copy part of the bitmap into cache texture
				for (p = 0; p < _RefSizeY; ++p)
				for (o = 0; o < _RefSizeX; ++o)
				{
					pDst[(o+p*_RefCacheTextureSizeX)*4+0] = pSrc[(o+p*pBitmap->getWidth())*4+0];
					pDst[(o+p*_RefCacheTextureSizeX)*4+1] = pSrc[(o+p*pBitmap->getWidth())*4+1];
					pDst[(o+p*_RefCacheTextureSizeX)*4+2] = pSrc[(o+p*pBitmap->getWidth())*4+2];
					pDst[(o+p*_RefCacheTextureSizeX)*4+3] = pSrc[(o+p*pBitmap->getWidth())*4+3];
				}
				czTmp.PosUV.U = ((float)xDst) / ((float)_RefCacheTextureSizeX);
				czTmp.PosUV.V = ((float)yDst) / ((float)_RefCacheTextureSizeY);
				czTmp.CacheTexture = _CacheTexture[m].Texture;
				_CacheTexture[m].FreePlace[n] = false;
				break;
			}
			nlassert (m<_CacheTexture[m].FreePlace.size());
			zdbTmp.ZonePieces.push_back (czTmp);
		}
		// Add the entry in the DataBase
		_ZoneDBmap.insert (pair<string,SElement>(zdbTmpName, zdbTmp));
		delete pBitmap;
	}

	// Upload all textures in VRAM
	for (m = 0; m < 64; ++m)
	if (_CacheTexture[m].Enabled)
		_CacheTexture[m].Texture->touch ();

	_UnusedTexture = loadTexture ("_UNUSED_.TGA");
	SetCurrentDirectory (sDirBackup);
	return true;
}

// ---------------------------------------------------------------------------
CBitmap *CDataBase::getBitmap (const string &ZoneName)
{
	map<string,SElement>::iterator it = _ZoneDBmap.find (ZoneName);
	if (it != _ZoneDBmap.end())
		return it->second.WinBitmap;
	else
		return NULL;
}

// ---------------------------------------------------------------------------
ITexture* CDataBase::getTexture (const string &ZoneName, sint32 nPosX, sint32 nPosY, CUV &retUVmin, CUV &retUVmax)
{
	if (ZoneName == STRING_UNUSED)
	{
		retUVmin.U = 0.0f;
		retUVmin.V = 1.0f - 0.0f;
		retUVmax.U = 1.0f;
		retUVmax.V = 1.0f - 1.0f;
		return _UnusedTexture;
	}

	map<string,SElement>::iterator it = _ZoneDBmap.find (ZoneName);
	if (it != _ZoneDBmap.end())
	{
		SElement &rElt = it->second;
		for (uint32 j = 0; j < rElt.ZonePieces.size(); ++j)
		if ((rElt.ZonePieces[j].PosX == nPosX) && (rElt.ZonePieces[j].PosY == nPosY))
		{
			retUVmin = rElt.ZonePieces[j].PosUV;
			retUVmin.U += 0.5f / ((float)_RefCacheTextureSizeX);
			retUVmin.V += 0.5f / ((float)_RefCacheTextureSizeY);
			retUVmax = retUVmin;
			retUVmax.U += ((float)_RefSizeX-1) / ((float)_RefCacheTextureSizeX);
			retUVmax.V += ((float)_RefSizeY-1) / ((float)_RefCacheTextureSizeY);
			return rElt.ZonePieces[j].CacheTexture;
		}
	}

	return NULL;
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Divisor to initialize windows bitmap
#define WIDTH_DIVISOR	2
#define HEIGHT_DIVISOR	2

CBitmap *CDataBase::convertToWin (NLMISC::CBitmap *pBitmap)
{
	CBitmap *pWinBitmap = new CBitmap;
	vector<uint8> &rPixel = pBitmap->getPixels();
	uint32 nNewWidth = pBitmap->getWidth()/WIDTH_DIVISOR;
	uint32 nNewHeight = pBitmap->getHeight()/HEIGHT_DIVISOR;
	uint8 *pNewPixel = new uint8[nNewHeight*nNewWidth*4];
	uint32 i, j;
	for (j = 0; j < nNewHeight; ++j)
	for (i = 0; i < nNewWidth; ++i)
	{
		pNewPixel[(i+j*nNewWidth)*4+0] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+2];
		pNewPixel[(i+j*nNewWidth)*4+1] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+1];
		pNewPixel[(i+j*nNewWidth)*4+2] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+0];
		pNewPixel[(i+j*nNewWidth)*4+3] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+3];
	}
	pWinBitmap->CreateBitmap (nNewWidth, nNewHeight, 1, 32, pNewPixel);
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

// ---------------------------------------------------------------------------
NLMISC::CBitmap *CDataBase::loadBitmap (const std::string &fileName)
{
	NLMISC::CBitmap *pBitmap = new NLMISC::CBitmap();

	try
	{
		CIFile fileIn;
		fileIn.open (fileName);
		pBitmap->load (fileIn);
	}
	catch (Exception& /*e*/)
	{
// cannot be done		MessageBox (NULL, e.what(), "Warning", MB_OK);
		delete pBitmap;
		return NULL;
	}

	return pBitmap;
}
// ***************************************************************************
// CBuilderZoneStack
// ***************************************************************************

#define BUILDERZONE_STACK_SIZE	32

// ---------------------------------------------------------------------------
CBuilderZoneStack::CBuilderZoneStack()
{
	_Stack.resize (BUILDERZONE_STACK_SIZE); // Depth of the stack
	reset ();
}

// ---------------------------------------------------------------------------
void CBuilderZoneStack::reset ()
{
	_Head = 0;
	_Queue = 0;
	_UndoPos = -1;
}

// ---------------------------------------------------------------------------
void CBuilderZoneStack::setRegion (CBuilderZoneRegion* pReg,sint32 nPos)
{
	if ((_UndoPos+1)%BUILDERZONE_STACK_SIZE != _Queue)
	{
		_Queue = (_UndoPos+1)%BUILDERZONE_STACK_SIZE;
	}
	//_Queue = (_UndoPos+1)%BUILDERZONE_STACK_SIZE;

	// Stack the region
	_Stack[_Queue].BZRegion = *pReg;
	_Stack[_Queue].RegionFrom = pReg;
	_Stack[_Queue].Pos = nPos;

	_UndoPos = _Queue;
	_Queue = (_Queue+1)%BUILDERZONE_STACK_SIZE;
	if (_Head == _Queue)
		_Head = (_Head+1)%BUILDERZONE_STACK_SIZE;
}

// ---------------------------------------------------------------------------
void CBuilderZoneStack::undo ()
{
	if (_UndoPos < 0)
		return;
	// Retrieve the last stacked element
	if (_UndoPos != _Head)
	{
		_UndoPos--;
		if (_UndoPos == -1)
			_UndoPos = BUILDERZONE_STACK_SIZE-1;
	}	
	CBuilderZoneRegion *pReg = _Stack[_UndoPos].RegionFrom;
	*pReg = _Stack[_UndoPos].BZRegion;
}

// ---------------------------------------------------------------------------
void CBuilderZoneStack::redo ()
{
	if (_UndoPos < 0)
		return;
	if ((_UndoPos+1)%BUILDERZONE_STACK_SIZE == _Queue)
		return;
	_UndoPos = (_UndoPos+1)%BUILDERZONE_STACK_SIZE;
	// Retrieve the last stacked element
	CBuilderZoneRegion *pReg = _Stack[_UndoPos].RegionFrom;
	*pReg = _Stack[_UndoPos].BZRegion;
}

// ---------------------------------------------------------------------------
bool CBuilderZoneStack::isEmpty ()
{
	if (_Head == _Queue)
		return true;
	return false;
}

// ***************************************************************************
// CBuilderZone
// ***************************************************************************
// ---------------------------------------------------------------------------
// PRIVATE
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void CBuilderZone::calcMask()
{
	sint32 i;
	sint32 x, y;

	_MinY = _MinX = 1000000;
	_MaxY = _MaxX = -1000000;

	if (_ZoneRegions.size() == 0)
		return;

	for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
	{
		CBuilderZoneRegion *pBZR = _ZoneRegions[i];
		if (_MinX > pBZR->getMinX())
			_MinX = pBZR->getMinX();
		if (_MinY > pBZR->getMinY())
			_MinY = pBZR->getMinY();
		if (_MaxX < pBZR->getMaxX())
			_MaxX = pBZR->getMaxX();
		if (_MaxY < pBZR->getMaxY())
			_MaxY = pBZR->getMaxY();
	}
	
	_ZoneMask.resize ((1+_MaxX-_MinX)*(1+_MaxY-_MinY));
	sint32 stride = (1+_MaxX-_MinX);
	for (y = _MinY; y <= _MaxY; ++y)
	for (x = _MinX; x <= _MaxX; ++x)
	{
		_ZoneMask[x-_MinX+(y-_MinY)*stride] = true;

		for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
		if (i != _ZoneRegionSelected)
		{
			const string &rSZone = _ZoneRegions[i]->getName (x, y);
			if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
			{
				_ZoneMask[x-_MinX+(y-_MinY)*stride] = false;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// PUBLIC
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CBuilderZone::CBuilderZone ()
{
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

	_Display = NULL;
	//_ZoneRegions.push_back (new CBuilderZoneRegion());
	//_ZoneRegionsName.push_back ("__New_Region__");
	//_ZoneRegionSelected = 0;
}

// ---------------------------------------------------------------------------
bool CBuilderZone::init (const string &sPathName, bool makeAZone)
{
	string sZoneBankPath = sPathName;
	sZoneBankPath += "ZoneLigos\\";
	// Init the ZoneBank
	_ZoneBank.reset ();
////////	_ZoneBank.debugInit (sZoneBankPath.c_str());
	initZoneBank (sZoneBankPath);
	
	// Construct the DataBase from the ZoneBank
	string sZoneBitmapPath = sPathName;
	sZoneBitmapPath += "ZoneBitmaps\\";
	_DataBase.init (sZoneBitmapPath.c_str(), _ZoneBank);

	if (makeAZone)
		newZone();
	
	return true;
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
bool CBuilderZone::load (const char *fileName, const char *path)
{
	string sTmp = fileName;
	for (uint32 i = 0; i < _ZoneRegionNames.size(); ++i)
	if (_ZoneRegionNames[i] == sTmp)
	{
		_ZoneRegionSelected = i;
		calcMask ();
		if (_Display)
			_Display->OnDraw (NULL);
		return true;
	}

	newZone (false);
	try
	{
		CIFile fileIn;
		fileIn.open (fileName);
		_ZoneRegions[_ZoneRegionSelected]->serial (fileIn);
		_ZoneRegionNames[_ZoneRegionSelected] = fileName;
		_FullNames[_ZoneRegionSelected] = path;
		_FullNames[_ZoneRegionSelected] += "\\";
		_FullNames[_ZoneRegionSelected] += fileName;
	}
	catch (Exception& e)
	{
		MessageBox (NULL, e.what(), "Warning", MB_OK);
		return false;
	}

	// Check if we can load this zone
	CBuilderZoneRegion *pBZR = _ZoneRegions[_ZoneRegionSelected];
	for (sint32 y = pBZR->getMinY(); y <= pBZR->getMaxY(); ++y)
	for (sint32 x = pBZR->getMinX(); x <= pBZR->getMaxX(); ++x)
	{
		const string &refSZone = pBZR->getName (x, y);
		if (refSZone != STRING_UNUSED)
		{
			for (uint32 i = 0; i < _ZoneRegions.size()-1; ++i)
			{
				const string &sZone = _ZoneRegions[i]->getName (x, y);
				if ((sZone != STRING_UNUSED)&&(sZone != STRING_OUT_OF_BOUND))
				{
					unload (_ZoneRegionSelected);
					MessageBox (NULL, "Cannot add this zone because it overlaps existing ones", 
								"Error", MB_ICONERROR|MB_OK);
					return false;
				}
			}
		}
	}

	_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this);

	calcMask ();
	if (_Display)
		_Display->OnDraw (NULL);
	return true;
}

// ---------------------------------------------------------------------------
bool CBuilderZone::save(const char *fileName)
{
	COFile fileOut;
	fileOut.open (fileName);
	_ZoneRegions[_ZoneRegionSelected]->reduceMin ();
	_ZoneRegions[_ZoneRegionSelected]->serial (fileOut);
	_ZoneRegionNames[_ZoneRegionSelected] = fileName;
	fileOut.close ();
	return true;
}

// ---------------------------------------------------------------------------
void CBuilderZone::autoSaveAll ()
{
	for (uint32 i = 0; i < _ZoneRegions.size(); ++i)
	{
		COFile fileOut;
		fileOut.open (_FullNames[i]);
		_ZoneRegions[i]->reduceMin ();
		_ZoneRegions[i]->serial (fileOut);
		fileOut.close ();
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::newZone (bool bDisplay)
{
	_ZoneRegions.push_back (new CBuilderZoneRegion);
	_ZoneRegionNames.push_back ("__New_Region__");
	_FullNames.push_back ("");
	_ZoneRegionSelected = _ZoneRegions.size() - 1;
	// Select starting point for the moment 0,0
	sint32 i;
	sint32 x = 0, y = 0;
	// If there are some zone already present increase x until free
	for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
	{
		CBuilderZoneRegion *pBZR = _ZoneRegions[i];
		const string &rsZone = pBZR->getName (x, y);
		if ((rsZone != STRING_OUT_OF_BOUND) && (rsZone != STRING_UNUSED))
		{
			++x; i = -1;
		}
	}
	_ZoneRegions[_ZoneRegionSelected]->setStart (x,y);
	calcMask ();
	if ((_Display)&&(bDisplay))
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderZone::unload (uint32 pos)
{
	uint32 i = 0;
	if (_ZoneRegions.size() == 0)
		return;
	delete _ZoneRegions[pos];
	for (i = pos; i < (_ZoneRegions.size()-1); ++i)
	{
		_ZoneRegions[i] = _ZoneRegions[i+1];
		_ZoneRegionNames[i] = _ZoneRegionNames[i+1];
		_FullNames[i] = _FullNames[i+1];
	}
	_ZoneRegions.resize (_ZoneRegions.size()-1);
	_ZoneRegionNames.resize (_ZoneRegionNames.size()-1);
	_FullNames.resize (_FullNames.size()-1);
	if (_ZoneRegionSelected == (sint32)_ZoneRegions.size())
		_ZoneRegionSelected = _ZoneRegions.size()-1;
	calcMask ();
	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderZone::move (sint32 x, sint32 y)
{
	if (_ZoneRegions.size() == 0)
		return;
	_ZoneRegions[_ZoneRegionSelected]->move(x, y);
}

// ---------------------------------------------------------------------------
CBuilderZone::SCacheRender::SCacheRender ()
{
	Used = false;
	Mat.initUnlit ();
	Mat.setBlend (false);
	VB.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag|CVertexBuffer::PrimaryColorFlag);
}

// ---------------------------------------------------------------------------
void CBuilderZone::render (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax)
{
	sint32 i, zoneSelected;

	// Reset the cache
	for (i = 0; i < (64+2); ++i)
	{
		_CacheRender[i].VB.setNumVertices (0);
		_CacheRender[i].PB.setNumTri (0);
	}

	// Select all blocks visible
	float minx = floorf(viewMin.x/_Display->_CellSize)*_Display->_CellSize;
	float miny = floorf(viewMin.y/_Display->_CellSize)*_Display->_CellSize;
	float maxx = ceilf(viewMax.x/_Display->_CellSize)*_Display->_CellSize;
	float maxy = ceilf(viewMax.y/_Display->_CellSize)*_Display->_CellSize;
	CVector pos1, pos2, pos3, pos4;
	CUV uvMin, uvMax;
	sint32 x, y;
	ITexture *pTexture;
	
	while (minx < maxx)
	{
		miny = floorf(viewMin.y/_Display->_CellSize)*_Display->_CellSize;
		while (miny < maxy)
		{
			x = (sint32)floor(minx / _Display->_CellSize);
			y = (sint32)floor(miny / _Display->_CellSize);

			i = 0;
			string sZone = STRING_OUT_OF_BOUND;
			zoneSelected = 0;
			for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
			{
				const string &rSZone = _ZoneRegions[i]->getName (x, y);
				if ((sZone == STRING_OUT_OF_BOUND) && (rSZone == STRING_UNUSED))
				{
					sZone = STRING_UNUSED;
					zoneSelected = i;
				}
				if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
				{
					sZone = rSZone;
					zoneSelected = i;
				}
			}
			CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (sZone);

			if (pZBE == NULL)
				pTexture = _DataBase.getTexture (sZone, 0, 0, uvMin, uvMax);
			else
				pTexture = _DataBase.getTexture (sZone, _ZoneRegions[zoneSelected]->getPosX(x, y), 
												_ZoneRegions[zoneSelected]->getPosY(x, y), uvMin, uvMax);

			// Look if already existing texture exists in the cache
			for (i = 0; i < (64+2); ++i)
			if (_CacheRender[i].Used)
				if (_CacheRender[i].Mat.getTexture(0) == pTexture)
					break;

			if (i == (64+2))
			{
				// Use a new CacheRender slot
				for (i = 0; i < (64+2); ++i)
					if (!_CacheRender[i].Used)
						break;
				nlassert(i<(64+2));
				_CacheRender[i].Used = true;
				_CacheRender[i].Mat.setTexture (0, pTexture);
			}

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

			uint32 nBasePt = _CacheRender[i].VB.getNumVertices();
			_CacheRender[i].VB.setNumVertices (nBasePt+4);
			_CacheRender[i].VB.setVertexCoord (nBasePt+0, pos1);
			_CacheRender[i].VB.setVertexCoord (nBasePt+1, pos2);
			_CacheRender[i].VB.setVertexCoord (nBasePt+2, pos3);
			_CacheRender[i].VB.setVertexCoord (nBasePt+3, pos4);


			uint32 nBaseTri = _CacheRender[i].PB.getNumTri ();
			_CacheRender[i].PB.setNumTri (nBaseTri+2);
			_CacheRender[i].PB.setTri (nBaseTri+0, nBasePt+0, nBasePt+1, nBasePt+2);
			_CacheRender[i].PB.setTri (nBaseTri+1, nBasePt+0, nBasePt+2, nBasePt+3);

			if ((zoneSelected>=0)&&(zoneSelected<_ZoneRegions.size()))
			{
				if (_ZoneRegions[zoneSelected]->getFlip (x, y) == 1)
				{
					float rTmp = uvMin.U;
					uvMin.U = uvMax.U;
					uvMax.U = rTmp;
				}

				_CacheRender[i].VB.setTexCoord (nBasePt+(_ZoneRegions[zoneSelected]->getRot (x, y)+0)%4, 0, CUV(uvMin.U, uvMin.V));
				_CacheRender[i].VB.setTexCoord (nBasePt+(_ZoneRegions[zoneSelected]->getRot (x, y)+1)%4, 0, CUV(uvMax.U, uvMin.V));
				_CacheRender[i].VB.setTexCoord (nBasePt+(_ZoneRegions[zoneSelected]->getRot (x, y)+2)%4, 0, CUV(uvMax.U, uvMax.V));
				_CacheRender[i].VB.setTexCoord (nBasePt+(_ZoneRegions[zoneSelected]->getRot (x, y)+3)%4, 0, CUV(uvMin.U, uvMax.V));
			}
			else
			{
				_CacheRender[i].VB.setTexCoord (nBasePt+0, 0, CUV(uvMin.U, uvMin.V));
				_CacheRender[i].VB.setTexCoord (nBasePt+1, 0, CUV(uvMax.U, uvMin.V));
				_CacheRender[i].VB.setTexCoord (nBasePt+2, 0, CUV(uvMax.U, uvMax.V));
				_CacheRender[i].VB.setTexCoord (nBasePt+3, 0, CUV(uvMin.U, uvMax.V));
			}

			NLMISC::CRGBA color;

			if (getZoneMask(x,y))
				color = NLMISC::CRGBA(255, 255, 255, 255);
			else
				color = NLMISC::CRGBA(127, 127, 127, 255);

			_CacheRender[i].VB.setColor (nBasePt+0, color);
			_CacheRender[i].VB.setColor (nBasePt+1, color);
			_CacheRender[i].VB.setColor (nBasePt+2, color);
			_CacheRender[i].VB.setColor (nBasePt+3, color);

			miny += _Display->_CellSize;
		}
		minx += _Display->_CellSize;
	}

	// Flush the cache to the screen
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewport (CViewport());
	CNELU::Driver->setupViewMatrix (mtx);
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	for (i = 0; i < (64+2); ++i)
	if (_CacheRender[i].Used)
	{
		// Render with driver
		CNELU::Driver->activeVertexBuffer (_CacheRender[i].VB);
		CNELU::Driver->render (_CacheRender[i].PB, _CacheRender[i].Mat);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::displayGrid (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax)
{
	// Select all blocks visible
	float rMinX = floorf (viewMin.x / _Display->_CellSize)*_Display->_CellSize;
	float rMinY = floorf (viewMin.y / _Display->_CellSize)*_Display->_CellSize;
	float rMaxX = ceilf  (viewMax.x / _Display->_CellSize)*_Display->_CellSize;
	float rMaxY = ceilf  (viewMax.y / _Display->_CellSize)*_Display->_CellSize;

	sint32 nMinX = (sint32)floor (rMinX / _Display->_CellSize);
	sint32 nMinY = (sint32)floor (rMinY / _Display->_CellSize);
	sint32 nMaxX = (sint32)floor (rMaxX / _Display->_CellSize);
	sint32 nMaxY = (sint32)floor (rMaxY / _Display->_CellSize);

	static vector<uint8> vBars;
	sint32 nBarsW = (nMaxX-nMinX)+1;
	sint32 nBarsH = (nMaxY-nMinY)+1;
	vBars.resize (nBarsW*nBarsH);
	sint32 x, y, i, j, zoneSelected;
	for (i = 0; i < nBarsW*nBarsH; ++i)
		vBars[i] = 0;


	for (y = nMinY; y <= nMaxY; ++y)
	for (x = nMinX; x <= nMaxX; ++x)
	{

		string sZone = STRING_OUT_OF_BOUND;
		zoneSelected = 0;
		for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
		{
			const string &rSZone = _ZoneRegions[i]->getName (x, y);
			if ((sZone == STRING_OUT_OF_BOUND) && (rSZone == STRING_UNUSED))
			{
				sZone = STRING_UNUSED;
				zoneSelected = i;
			}
			if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
			{
				sZone = rSZone;
				zoneSelected = i;
			}
		}



		//const string &sZone = _ZoneRegion.getName (x, y);
		CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (sZone);
		if (pZBE != NULL)
		if ((pZBE->getSizeX() > 1) || (pZBE->getSizeY() > 1))
		{
			sint32 sizeX = pZBE->getSizeX(), sizeY = pZBE->getSizeY();
			sint32 posX = _ZoneRegions[zoneSelected]->getPosX (x, y), posY = _ZoneRegions[zoneSelected]->getPosY (x, y);
			uint8 rot = _ZoneRegions[zoneSelected]->getRot (x, y);
			uint8 flip = _ZoneRegions[zoneSelected]->getFlip (x, y);
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

			static CBuilderZoneRegion::SPiece sMask;
			sMask.Tab.resize (sizeX*sizeY);
			for(i = 0; i < sizeX*sizeY; ++i)
				sMask.Tab[i] = pZBE->getMask()[i];
			sMask.w = sizeX;
			sMask.h = sizeY;
			_ZoneRegions[zoneSelected]->rotFlip (sMask, rot, flip);

			for (j = 0; j < sMask.h; ++j)
			for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i+j*sMask.w])
			{
				if (((x+deltaX+i-nMinX)>=0) && ((x+deltaX+i-nMinX)<nBarsW) &&
					((y+deltaY+j-nMinY)>=0) && ((y+deltaY+j-nMinY)<nBarsH))
				{
					if ((i > 0) && (sMask.Tab[i-1+j*sMask.w]))
						vBars[x+deltaX+i-nMinX + (y+deltaY+j-nMinY)*nBarsW] |= 1;

					if ((j > 0) && (sMask.Tab[i+(j-1)*sMask.w]))
						vBars[x+deltaX+i-nMinX + (y+deltaY+j-nMinY)*nBarsW] |= 2;
				}
			}
		}
	}

	CVertexBuffer VB;
	CPrimitiveBlock PB;
	CMaterial Mat;

	Mat.initUnlit ();
	Mat.setBlend (false);
	VB.setVertexFormat (CVertexBuffer::PositionFlag);
	VB.setNumVertices ((nBarsW+1)*(nBarsH+1));
	
	for (y = nMinY; y <= nMaxY+1; ++y)
	for (x = nMinX; x <= nMaxX+1; ++x)
	{
		CVector pos;

		pos.x = (x*_Display->_CellSize - viewMin.x)/(viewMax.x-viewMin.x);
		pos.y = 0.0f;
		pos.z = (y*_Display->_CellSize - viewMin.y)/(viewMax.y-viewMin.y);
		VB.setVertexCoord (x-nMinX+(y-nMinY)*(nBarsW+1), pos);
	}

	PB.setNumLine (nBarsW*nBarsH*2);
	uint32 nNbLine = 0;
	for (y = 0; y < nBarsH; ++y)
	for (x = 0; x < nBarsW; ++x)
	{
		// Vertical Line ?
		if ((vBars[x+y*nBarsW] & 1) == 0)
		{
			PB.setLine (nNbLine, x+y*(nBarsW+1), x+(y+1)*(nBarsW+1));
			++nNbLine;
		}

		// Horizontal Line ?
		if ((vBars[x+y*nBarsW] & 2) == 0)
		{
			PB.setLine (nNbLine, x+y*(nBarsW+1), (x+1)+y*(nBarsW+1));
			++nNbLine;
		}
	}
	PB.setNumLine (nNbLine);

	// Render with driver
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewport (CViewport());
	CNELU::Driver->setupViewMatrix (mtx);
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
	CNELU::Driver->activeVertexBuffer (VB);
	CNELU::Driver->render (PB, Mat);
	
}

// ---------------------------------------------------------------------------
void CBuilderZone::add (const CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);
	uint8 rot, flip;

	if (_ZoneRegions.size() == 0)
		return;

	if (_StackZone.isEmpty())
		_StackZone.setRegion (_ZoneRegions[_ZoneRegionSelected], _ZoneRegionSelected);

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
		_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this);
		_ZoneRegions[_ZoneRegionSelected]->add (x, y, rot, flip, pZBE);
	}
	_StackZone.setRegion (_ZoneRegions[_ZoneRegionSelected], _ZoneRegionSelected);
}

// ---------------------------------------------------------------------------
void CBuilderZone::del (const CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);

	if (_ZoneRegions.size() == 0)
		return;

	if (_StackZone.isEmpty())
		_StackZone.setRegion (_ZoneRegions[_ZoneRegionSelected], _ZoneRegionSelected);

	_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this);
	_ZoneRegions[_ZoneRegionSelected]->del (x, y);

	_StackZone.setRegion (_ZoneRegions[_ZoneRegionSelected], _ZoneRegionSelected);
}

// ---------------------------------------------------------------------------
bool CBuilderZone::initZoneBank (const string &sPathName)
{
	char sDirBackup[512];
	GetCurrentDirectory (512, sDirBackup);
	SetCurrentDirectory (sPathName.c_str());
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

// ---------------------------------------------------------------------------
void CBuilderZone::undo ()
{
	_StackZone.undo();
	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderZone::redo ()
{
	_StackZone.redo();
	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderZone::stackReset ()
{
	_StackZone.reset();
}

// ---------------------------------------------------------------------------
uint32 CBuilderZone::getNbZoneRegion ()
{
	return _ZoneRegions.size ();
}

// ---------------------------------------------------------------------------
const string& CBuilderZone::getZoneRegionName (uint32 i)
{
	return _ZoneRegionNames[i];
}

// ---------------------------------------------------------------------------
uint32 CBuilderZone::getCurZoneRegion ()
{
	return _ZoneRegionSelected;
}

// ---------------------------------------------------------------------------
void CBuilderZone::setCurZoneRegion (uint32 sel)
{
	_ZoneRegionSelected = sel;
	calcMask();
}

// ---------------------------------------------------------------------------
bool CBuilderZone::getZoneMask (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return true;
	}
	else
	{
		return _ZoneMask[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)];
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::generate (sint32 nMinX, sint32 nMinY, sint32 nMaxX, sint32 nMaxY, 
							sint32 nZoneBaseX, sint32 nZoneBaseY, const char *MaterialString)
{
	if ((nMinX > nMaxX)	|| (nMinY > nMaxY))
		return;

	for (sint32 j = nMinY; j <= nMaxY; ++j)
	for (sint32 i = nMinX; i <= nMaxX; ++i)
	{
		// Generate zone name
		string ZoneName = NLMISC::toString(-nZoneBaseY-j) + "_";
		ZoneName += ('a' + (nZoneBaseX+i)/26);
		ZoneName += ('a' + (nZoneBaseX+i)%26);
		CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (ZoneName);
		if (pZBE != NULL)
		{
			_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this);
			_ZoneRegions[_ZoneRegionSelected]->add (i, j, 0, 0, pZBE);
		}
	}
}
