// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERLOGIC_H
#define BUILDERLOGIC_H

// ***************************************************************************

#include "../lib/primitive.h"

#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"

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

class CPrimBuild
{

public:
	
	NLLIGO::CPrimRegion	*PRegion;

	sint32				Type;		// 0 (Point),1 (Path), 2 (Zone),-1 (Not Valid)
	bool				Created;
	sint32				Pos;
	bool				hidden;
	
	CPrimBuild();

};

// ***************************************************************************

class CBuilderLogic
{


	//NLLIGO::CPrimRegion			PRegion;
	std::vector<NLLIGO::CPrimRegion*>	PRegions;
	sint32								RegionSelected;

	// Tools accelerator
	std::map<HTREEITEM, CPrimBuild>		Primitives;

	// Selection
	HTREEITEM					ItemSelected;
	sint32						VertexSelected;
	NLMISC::CRGBA				SelectionCol;

	CDisplay					*_Display;
	CToolsLogic					*_ToolsLogic;

public:

	CBuilderLogic();
	void setDisplay (CDisplay *pDisp);
	void setToolsLogic	(CToolsLogic *pTool);
	void updateToolsLogic ();
	bool load (const char *fileName);
	bool save (uint32 nPos, const char *fileName);
	void newZone ();
	void unload (uint32 pos);
	uint32 getNbZoneRegion();
	const std::string &getZoneRegionName(uint32 nPos);

	void insertPoint (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void insertPath (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void insertZone (uint32 nPos, HTREEITEM item, const char *Name, const char *LayerName);
	void del (HTREEITEM item);
	void hide (HTREEITEM item);
	void hideAll (uint32 nPos, sint32 nID, bool bHide);
	void regionHideAll (uint32 nPos, bool bHide); // (bHide == false) -> unhide

	const char* getName (HTREEITEM item);
	const char* getLayerName (HTREEITEM item);
	bool isHidden (HTREEITEM item);
	void setName (HTREEITEM item, const char* pStr);
	void setLayerName (HTREEITEM item, const char* pStr);

	// Operation on Selected PrimBuild
	HTREEITEM getSelPB ();
	void setSelPB (HTREEITEM item);
	void createVertexOnSelPB (NLMISC::CVector &v, uint32 atPos = 0);
	bool selectVertexOnSelPB (NLMISC::CVector &selMin, NLMISC::CVector &selMax);
	void setSelVertexOnSelPB (NLMISC::CVector &v);
	void delSelVertexOnSelPB ();

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
	void renderDrawLine (NLMISC::CVector &pos, NLMISC::CVector &pos2, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);
	void renderDrawTriangle (NLMISC::CVector &pos, NLMISC::CVector &pos2, NLMISC::CVector &pos3, NL3D::CVertexBuffer *pVB, NL3D::CPrimitiveBlock *pPB);

};

// ***************************************************************************

#endif // BUILDERLOGIC_H