// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONE_H
#define BUILDERZONE_H

// ***************************************************************************

#define STRING_UNUSED		"< UNUSED >"
#define STRING_OUT_OF_BOUND "< OOB >"

// ***************************************************************************

#include "nel/misc/vector.h"
#include "nel/misc/smart_ptr.h"

#include "../lib/zone_bank.h"

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
}

// ***************************************************************************

class CDataBase
{
	struct SElement
	{
		std::string								Name;
		NLMISC::CSmartPtr<NL3D::CTextureFile>	Texture;
		CBitmap									*WinBitmap;
	};

private:

	CBitmap				*convertToWin (NL3D::CTextureFile *pTexture);
	NL3D::CTextureFile	*loadTexture (const std::string &fileName);

private:

	std::vector<SElement>					_ZoneDB;
	NLMISC::CSmartPtr<NL3D::CTextureFile>	UnusedTexture;

public:

	CDataBase();
	~CDataBase();

	bool				initFromPath (const std::string &Path);
	bool				init (const std::string &Path, NLLIGO::CZoneBank &zb);

	CBitmap				*getBitmap (const std::string &ZoneName);
	NL3D::CTextureFile	*getTexture (const std::string &ZoneName);
};

// ***************************************************************************

class CZoneRegion
{

	std::vector<std::string>	_Zones;
	sint32						_MinX, _MinY;
	sint32						_MaxX, _MaxY;

	static std::string			_StringOutOfBound;

	void				resize (sint32 newMinX, sint32 newMaxX, sint32 newMinY, sint32 newMaxY);

public:

	CZoneRegion();

	void				set (sint32 x, sint32 y, const std::string &ZoneName);
	const std::string	&get (sint32 x, sint32 y);
	void				reduceMin ();
};

// ***************************************************************************

class CBuilderZone
{
	NLLIGO::CZoneBank			_ZoneBank;

	CImageList					*_ImageList;
	CDataBase					_DataBase;

	CZoneRegion					_ZoneRegion;

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
	std::string _CurSelectedZone;

public:

	CBuilderZone();
	void				setDisplay (CDisplay *pDisp);
	void				setToolsZone (CToolsZone *pTool);
	void				updateToolsZone ();
	bool				load(const char *fileName);
	bool				save(const char *fileName);

	void				add (NLMISC::CVector &worldPos);
	void				del (NLMISC::CVector &worldPos);

	// Accessors
	NLLIGO::CZoneBank	&getZoneBank() { return _ZoneBank; }

	void				render (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);

};

// ***************************************************************************

#endif // BUILDERZONE_H