// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONE_H
#define BUILDERZONE_H

// ***************************************************************************

#include "nel/misc/vector.h"
#include "nel/misc/uv.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/smart_ptr.h"

#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/material.h"

#include "../lib/zone_bank.h"

#include "builderZoneRegion.h"

#include <string>
#include <vector>

// ***************************************************************************

class CDisplay;
class CToolsZone;

namespace NL3D
{
	class CVertexBuffer;
	class CPrimitiveBlock;
	class CTextureFile;
	class CTextureMem;
	class ITexture;
}

// ***************************************************************************
// CDataBase contains the image database for Nel and Windows
// A big texture (called CacheTexture) contains all zone (called SCacheZone)
// There are 64 cache texture of 1024x1024 (it should be enough)
// An element is composed of zones (the number of zones is equal to the number
// of true in the mask (the mask is in the zone bank))
class CDataBase
{
	struct SCacheTexture
	{
		bool									Enabled;
		NLMISC::CSmartPtr<NL3D::CTextureMem>	Texture;
		std::vector<bool>						FreePlace;
		std::vector<uint8>						PtrMem;

		SCacheTexture();
		bool isFull();
	};

	struct SCacheZone
	{
		NL3D::CTextureMem						*CacheTexture;
		NLMISC::CUV								PosUV;
		uint8									PosX, PosY;
	};

	struct SElement
	{
		uint8									SizeX, SizeY;
		std::vector<SCacheZone>					ZonePieces;
		CBitmap									*WinBitmap;
	};

private:

	CBitmap				*convertToWin (NLMISC::CBitmap *pBitmap);
	NL3D::CTextureFile	*loadTexture (const std::string &fileName);
	NLMISC::CBitmap		*loadBitmap (const std::string &fileName);

private:

	SCacheTexture							_CacheTexture[64];
	std::map<std::string, SElement>			_ZoneDBmap;
	NLMISC::CSmartPtr<NL3D::CTextureFile>	_UnusedTexture;

	uint32									_RefSizeX, _RefSizeY;
	uint32									_RefCacheTextureSizeX, _RefCacheTextureSizeY;
	uint32									_RefCacheTextureNbEltX, _RefCacheTextureNbEltY;

public:

	CDataBase();
	~CDataBase();

	bool				initFromPath (const std::string &Path);
	bool				init (const std::string &Path, NLLIGO::CZoneBank &zb);

	CBitmap				*getBitmap (const std::string &ZoneName);
	NL3D::ITexture		*getTexture (const std::string &ZoneName, sint32 nPosX, sint32 nPosY, 
									NLMISC::CUV &retUVmin, NLMISC::CUV &retUVmax);
};

// ***************************************************************************
// CBuilderZoneStack contains the stack of the state painting
class CBuilderZoneStack
{
	struct SStackElt
	{
		CBuilderZoneRegion	BZRegion;
		CBuilderZoneRegion	*RegionFrom;
		sint32				Pos;
	};

	std::vector<SStackElt>	_Stack;
	sint32					_Head, _Queue, _UndoPos;

public:

	CBuilderZoneStack ();
	void reset ();
	void setRegion (CBuilderZoneRegion* pReg,sint32 nPos);
	void undo ();
	void redo ();
	bool isEmpty ();
};

// ***************************************************************************
// CBuilderZone contains all the shared data between the tools and the engine
// ZoneBank contains the macro zones that is composed of several zones plus a mask
// DataBase contains the graphics for the zones
class CBuilderZone
{
public:

	NLLIGO::CZoneBank			_ZoneBank;

	CDataBase					_DataBase;

	std::vector<CBuilderZoneRegion*>	_ZoneRegions;
	std::vector<std::string>			_ZoneRegionNames;
	std::vector<std::string>			_FullNames;

	sint32								_ZoneRegionSelected;

	sint32						_MinX, _MaxX, _MinY, _MaxY;
	std::vector<bool>			_ZoneMask;

	CDisplay					*_Display;
	CToolsZone					*_ToolsZone;

	CBuilderZoneStack			_StackZone;

public:

	std::string _FilterType1, _FilterValue1;
	std::string _FilterType2, _FilterValue2;
	uint8		_FilterOperator2;				// 0 -> AND, 1 -> OR
	std::string _FilterType3, _FilterValue3;
	uint8		_FilterOperator3;				// 0 -> AND, 1 -> OR
	std::string _FilterType4, _FilterValue4;
	uint8		_FilterOperator4;				// 0 -> AND, 1 -> OR

	bool		_RandomSelection;
	sint32		_CurSelectedZone;

	uint8		_ApplyRot;
	bool		_ApplyRotRan;

	uint8		_ApplyFlip;
	bool		_ApplyFlipRan;

	std::vector<NLLIGO::CZoneBankElement*> _CurrentSelection;

private:

	void				calcMask();
	bool				initZoneBank (const std::string &Path);

public:

	CBuilderZone();

	bool				init (const std::string &sPath, bool bMakeAZone);

	void				setDisplay (CDisplay *pDisp);
	void				setToolsZone (CToolsZone *pTool);
	void				updateToolsZone ();
	bool				load (const char *fileName, const char *path);
	bool				save (const char *fileName);
	void				autoSaveAll ();
	void				newZone (bool bDisplay=true);
	void				unload (uint32 i);

	void				add (const NLMISC::CVector &worldPos);
	void				del (const NLMISC::CVector &worldPos);

	void				undo ();
	void				redo ();
	void				stackReset ();

	// Accessors
	NLLIGO::CZoneBank	&getZoneBank () { return _ZoneBank; }

	void				render (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax);
	void				displayGrid (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax);

	uint32				getNbZoneRegion ();
	const std::string&	getZoneRegionName (uint32 i);
	uint32				getCurZoneRegion ();
	void				setCurZoneRegion (uint32 i);
	bool				getZoneMask (sint32 x, sint32 y);

private:

	// SCacheRender is a simple structure to store triangles for each texture in the scene
	struct SCacheRender
	{
		bool					Used;
		NL3D::CVertexBuffer		VB;
		NL3D::CPrimitiveBlock	PB;
		NL3D::CMaterial			Mat;

		SCacheRender();
	};

	// There are one CacheRender per cacheTexture + texture unused and NULL (no texture)
	SCacheRender _CacheRender[64+2];

};

// ***************************************************************************

#endif // BUILDERZONE_H