// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONE_H
#define BUILDERZONE_H

// ***************************************************************************

#include "nel/misc/vector.h"
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
}

// ***************************************************************************
// CDataBase contains the image database for Nel and Windows
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
// CBuilderZone contains all the shared data between the tools and the motor
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
	/*
	void				putAndSolve (sint32 x, sint32 y, NLLIGO::CZoneBankElement *pZBE);
	void				removeAndSolve (sint32 x, sint32 y);
	void				placeRandomTrans (sint32 x, sint32 y, std::string TransNameVal);
	void				setTrans (sint32 x, sint32 y, NLLIGO::CZoneBankElement *pZBE);
	*/
	bool				initZoneBank (const std::string &Path);

	// Accessors
	NLLIGO::CZoneBank	&getZoneBank () { return _ZoneBank; }

	void				render (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);

};

// ***************************************************************************

#endif // BUILDERZONE_H