// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERLOGIC_H
#define BUILDERLOGIC_H

// ***************************************************************************

#include "../lib/primitive.h"

#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"

#include "edit_stack.h"

#include <map>

// ***************************************************************************

class CDisplay;
class CToolsLogic;

namespace NL3D
{
	class CVertexBuffer;
	class CPrimitiveBlock;
}

// ***************************************************************************

struct SPrimBuild
{
	NLLIGO::CPrimRegion	*PRegion;

	sint32				Type;		// 0 (Point),1 (Path), 2 (Zone),-1 (Not Valid)
	bool				Created;
	sint32				Pos;		// Position in a PRegion array (VPoints, VPaths or vZones
	bool				hidden;
	
	SPrimBuild();
};

// ***************************************************************************

class CBuilderLogic
{


	//NLLIGO::CPrimRegion			PRegion;
	std::vector<bool>					_MustAskSaves;
	std::vector<NLLIGO::CPrimRegion*>	_PRegions;
	std::vector<std::string>			_FullNames;
	sint32								_RegionSelected;

	// Tools accelerator
	std::map<HTREEITEM, SPrimBuild>		_Primitives;

	// Selection
	HTREEITEM							_ItemSelected;
	std::string							_ItemSelectedName;
	std::vector<sint32>					_VerticesSelected;
	NLMISC::CRGBA						_SelectionCol;

	CDisplay							*_Display;
	CToolsLogic							*_ToolsLogic;

	CEditStack<NLLIGO::CPrimRegion>		_StackPR;

public:

	CBuilderLogic();
	void setDisplay (CDisplay *pDisp);
	void setToolsLogic	(CToolsLogic *pTool);
	void uninit ();
	void updateToolsLogic ();
	bool load (const char *fileName, const char *path);
	bool save (uint32 nPos, const char *fileName);
	void autoSaveAll ();
	void newZone ();
	void unload (uint32 pos);
	uint32 getNbZoneRegion();
	const std::string &getZoneRegionName (uint32 nPos);
	void move (const std::string &name, float x, float y);
	
	void undo (); // undo/redo interface
	void redo ();
	void stackReset ();

	void insertPoint (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void insertPath (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void insertZone (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void del (HTREEITEM item);
	void hide (HTREEITEM item);
	void hideAll (uint32 nPos, sint32 nID, bool bHide);
	void regionHideAll (uint32 nPos, bool bHide); // (bHide == false) -> unhide
	void regionHideType (uint32 nPos, const std::string &Type, bool bHide); // (bHide == false) -> unhide
	int  getMaxPostfix (const char *prefix);
	bool isAlreadyExisting (const char *sPrimitiveName);

	const char* getName (HTREEITEM item);
	const char* getLayerName (HTREEITEM item);
	bool isHidden (HTREEITEM item);
	void setName (HTREEITEM item, const char *pStr);
	void setLayerName (HTREEITEM item, const char *pStr);

	// Operation on Selected PrimBuild
	HTREEITEM getSelPB ();
	void setSelPB (HTREEITEM item);
	void createVertexOnSelPB (NLMISC::CVector &v, uint32 atPos = 0);
	bool selectVerticesOnSelPB (NLMISC::CVector &selMin, NLMISC::CVector &selMax);
	bool isSelection();
	void setSelVerticesOnSelPB (NLMISC::CVector &v);
	void delSelVerticesOnSelPB ();
	void stackSelPB ();

	void render (NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);

private:

	// Help for rendering
	bool isInTriangleOrEdge(	double x, double y, 
								double xt1, double yt1, 
								double xt2, double yt2, 
								double xt3, double yt3 );
	void convertToScreen (NLMISC::CVector* pVec, sint nNbVec, NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);
	bool clip (NLMISC::CVector *pVec, uint32 nNbVec, NLMISC::CVector &viewMin, NLMISC::CVector &viewMax);
	void renderDrawPoint (NLMISC::CVector &pos, NLMISC::CRGBA &col, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	void renderDrawLine (NLMISC::CVector &pos, NLMISC::CVector &pos2, NLMISC::CRGBA &col, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	void renderDrawTriangle (NLMISC::CVector &pos, NLMISC::CVector &pos2, NLMISC::CVector &pos3, NLMISC::CRGBA &col, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	NLMISC::CRGBA findColor(const std::string &LayerName);

	void askSaveRegion (int i);

};

// ***************************************************************************

#endif // BUILDERLOGIC_H