#include "stdafx.h"

using namespace NL3D;
using namespace NLMISC;

class CPaintColor
{
public:
	// Go, paint !
	CPaintColor (PaintPatchMod *pobj, CLandscape	*landscape, CTileUndo *undo, EPM_PaintMouseProc *mouseProc)
	{
		_PObj=pobj;
		_Landscape=landscape;
		_Undo=undo;
		_MouseProc=mouseProc;
	}

	// Go, paint !
	void paint (int mesh, int tile, const CVector& hit, std::vector<EPM_Mesh> &vectMesh);

	// Set a vertex color with handle of the undo
	void setVertexColor (int mesh, int patch, int s, int t, const CRGBA& newColor, uint16 blend, std::vector<EPM_Mesh> &vectMesh, 
		CNelPatchChanger& nelPatchChg, bool undo);

	// picj a vertex of a patch
	void pickVertexColor (int mesh, int patch, int s, int t, CVector& pos, CRGBA& color, std::vector<EPM_Mesh> &vectMesh);

private:

	// Recurcive function to paint a tile
	void paintATile (EPM_PaintTile *pTile, std::set<EPM_PaintTile*>& visited, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, 
		std::set<int>& modified, CNelPatchChanger& nelPatchChg);

	// Paint a vertex of a patch
	void paintAVertex (int mesh, int patch, int s, int t, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, CNelPatchChanger& nelPatchChg);

private:
	PaintPatchMod			*_PObj;
	CLandscape				*_Landscape;
	CTileUndo				*_Undo;
	EPM_PaintMouseProc		*_MouseProc;
};
