// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONE_H
#define BUILDERZONE_H

// ***************************************************************************

#include "nel/misc/vector.h"
#include "nel/misc/uv.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/smart_ptr.h"

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
// CBuilderZone contains all the shared data between the tools and the engine
// ZoneBank contains the macro zones that is composed of several zones plus a mask
// DataBase contains the graphics for the zones
class CBuilderZone
{
	NLLIGO::CZoneBank			_ZoneBank;

	CDataBase					_DataBase;

	CBuilderZoneRegion			_ZoneRegion;

	CDisplay					*_Display;
	CToolsZone					*_ToolsZone;

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

public:

	CBuilderZone();
	void				setDisplay (CDisplay *pDisp);
	void				setToolsZone (CToolsZone *pTool);
	void				updateToolsZone ();
	bool				load (const char *fileName);
	bool				save (const char *fileName);

	void				add (NLMISC::CVector &worldPos);
	void				del (NLMISC::CVector &worldPos);
	bool				initZoneBank (const std::string &Path);

	// Accessors
	NLLIGO::CZoneBank	&getZoneBank () { return _ZoneBank; }

	void				render (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);
	void				displayGrid (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);

};

// ***************************************************************************

#endif // BUILDERZONE_H