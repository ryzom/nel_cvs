#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_vcolor.h"
#include "paint_undo.h"
#include "paint_to_nel.h"
#include <3d/landscape.h>

// User color 1 and 2
extern COLORREF color1;
extern COLORREF color2;
extern float opa1;
extern float opa2;
extern float hard1;
extern float hard2;
CRGBA maxToNel (COLORREF ref);

/*-------------------------------------------------------------------*/

float distance (PaintPatchMod *pobj)
{
	return COLOR_BRUSH_MIN+(COLOR_BRUSH_MAX-COLOR_BRUSH_MIN)*(float)pobj->ColorBushSize/(float)COLOR_BRUSH_STEP;
}
 
 /*-------------------------------------------------------------------*/

void CPaintColor::paint (int mesh, int tile, const CVector& hit, std::vector<EPM_Mesh> &vectMesh)
{
	// Only if the landscape is valid
	if (_Landscape)
	{
		// Set of visited tiles
		std::set<EPM_PaintTile*> visited;

		// Set of modified meshes.
		std::set<int> modified;

		// Nel patch changement manager
		CNelPatchChanger nelPatchChg (_Landscape);

		// Start at the first tile
		paintATile (&_MouseProc->metaTile[mesh][tile], visited, hit, vectMesh, modified, nelPatchChg);

		// Flush nel chgt
		nelPatchChg.applyChanges (false);
	}
}

/*-------------------------------------------------------------------*/

void CPaintColor::paintATile (EPM_PaintTile *pTile, std::set<EPM_PaintTile*>& visited, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, 
							  std::set<int>& modified, CNelPatchChanger& nelPatchChg)
{
	// Check some args
	nlassert (_PObj->brushSize>=0);
	nlassert (_PObj->brushSize<3);

	// Already visited ?
	if (visited.find (pTile)!=visited.end())
		return;

	// Check if we are in patch subobject and if this patch is selected
	if ((vectMesh[pTile->Mesh].PMesh->selLevel==EP_PATCH)&&(!vectMesh[pTile->Mesh].PMesh->patchSel[pTile->patch]))
		return;

	// Check if this tile is in the range of the brush
	if ((pTile->Center-hit).norm()>distance (_PObj)+pTile->Radius)
		return;

	// Paint the upper left vertex
	paintAVertex (pTile->Mesh, pTile->patch, pTile->u, pTile->v, hit, vectMesh, nelPatchChg);
	
	// Tile on a right, bottom or right bottom border ?
	bool borderRight=pTile->u == (1<<vectMesh[pTile->Mesh].RMesh->getUIPatch (pTile->patch).NbTilesU)-1;
	bool borderBottom=pTile->v == (1<<vectMesh[pTile->Mesh].RMesh->getUIPatch (pTile->patch).NbTilesV)-1;
	if (borderRight)
		paintAVertex (pTile->Mesh, pTile->patch, pTile->u+1, pTile->v, hit, vectMesh, nelPatchChg);
	if (borderBottom)
		paintAVertex (pTile->Mesh, pTile->patch, pTile->u, pTile->v+1, hit, vectMesh, nelPatchChg);
	if (borderRight&&borderBottom)
		paintAVertex (pTile->Mesh, pTile->patch, pTile->u+1, pTile->v+1, hit, vectMesh, nelPatchChg);

	// Visited
	visited.insert (pTile);

	// Modified
	modified.insert (pTile->Mesh);

	// Visite the neighborhood
	for (int neightbor=0; neightbor<4; neightbor++)
	{
		// Is there a neighbor ?
		if (pTile->voisins[neightbor])
			// Ok, recusive call
			paintATile (pTile->voisins[neightbor], visited, hit, vectMesh, modified, nelPatchChg);
	}
}

/*-------------------------------------------------------------------*/

void CPaintColor::paintAVertex (int mesh, int patch, int s, int t, const CVector& hit, std::vector<EPM_Mesh> &vectMesh, CNelPatchChanger& nelPatchChg)
{
	// Check some args
	nlassert (_PObj->brushSize>=0);
	nlassert (_PObj->brushSize<3);

	// Get the zone for this mesh
	const CZone* zone=_Landscape->getZone (mesh);
	nlassert (zone);

	// Get the patch pointer
	const CPatch *pPatch=zone->getPatch(patch);

	// Eval the vertex position
	CVector vertexPos=pPatch->computeVertex ((float)s/(float)pPatch->getOrderS(), (float)t/(float)pPatch->getOrderT());

	// Compute dist from the brush
	float distBrush=(hit-vertexPos).norm();

	// Check if it is in distance
	if (distBrush<=distance (_PObj))
	{
		// *** Compute new vertex color

		// Get the old value
		CRGBA old;
		vectMesh[mesh].RMesh->getVertexColor (patch, s, t, old);

		// Blend with distance
		float blendDist=(distance (_PObj)-distBrush)/distance (_PObj);

		// Blend the two colors
		float finalFactor=256.f*opa1*((1.f-hard1)*blendDist+hard1);
		uint16 blend=(uint16)(std::max (std::min (finalFactor, 256.f), 0.f) );

		// Set the vertex color
		setVertexColor (mesh, patch, s, t, maxToNel (color1), blend, vectMesh, nelPatchChg, true);
	}
}

/*-------------------------------------------------------------------*/

void CPaintColor::pickVertexColor (int mesh, int patch, int s, int t, CVector& pos, CRGBA& color, std::vector<EPM_Mesh> &vectMesh)
{
	// Check some args
	nlassert (_PObj->brushSize>=0);
	nlassert (_PObj->brushSize<3);

	// Get the zone for this mesh
	const CZone* zone=_Landscape->getZone (mesh);
	nlassert (zone);

	// Get the patch pointer
	const CPatch *pPatch=zone->getPatch(patch);

	// Eval the vertex position
	pos=pPatch->computeVertex ((float)s/(float)pPatch->getOrderS(), (float)t/(float)pPatch->getOrderT());

	// Get the old value
	vectMesh[mesh].RMesh->getVertexColor (patch, s, t, color);
}

/*-------------------------------------------------------------------*/

void CPaintColor::setVertexColor (int mesh, int patch, int s, int t, const CRGBA& newColor, uint16 blend, std::vector<EPM_Mesh> &vectMesh, 
								  CNelPatchChanger& nelPatchChg, bool undo)
{
	// Get the old value
	CRGBA old;
	vectMesh[mesh].RMesh->getVertexColor (patch, s, t, old);

	if (undo)
	{
		// Create an undo element
		CUndoElement elmt (mesh, CUndoStruct (patch, s, t, old, 256), CUndoStruct (patch, s, t, newColor, blend));

		// Put an undo entry
		_Undo->toUndo (elmt);
	}

	// Get the patch
	std::vector<CTileColor>& copyZone = *nelPatchChg.getColorArray  (mesh, patch);

	// Get the Nel patch mesh
	RPatchMesh *pMesh=vectMesh[mesh].RMesh;

	// Get order S of this patch
	int OrderS=(1<<pMesh->getUIPatch (patch).NbTilesU)+1;

	// Blend color
	CRGBA color;
	color.blendFromui ( old, newColor, blend );
	
	// Set the color in 
	copyZone[s+t*OrderS].Color565=color.get565();

	// Set the color
	vectMesh[mesh].RMesh->setVertexColor (patch, s, t, color);
}

/*-------------------------------------------------------------------*/

