#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_fill.h"
#include "paint_undo.h"
#include "paint_vcolor.h"
#include "paint_to_nel.h"
#include <3d/landscape.h>

/*-------------------------------------------------------------------*/

void CFillPatch::fillTile (int mesh, int patch, std::vector<EPM_Mesh> &vectMesh, int tileSet, int rot, int group, bool _256, 
						const CTileBank& bank)
{
	// Only if the landscape is valid
	if (_Landscape)
	{
		// Nel patch changement manager
		CNelPatchChanger nelPatchChg (_Landscape);

		// Clear tile description
		tileDesc descClear;
		descClear.setTile (0, 0, 0, tileIndex (false, 0,0), tileIndex (false, 0,0), tileIndex (false, 0,0));

		// Fill tile description
		tileDesc descFill;					// Four fill descriptor LeftTop, LeftBottom, RightTop, RightBottom

		// Fill with clear ?
		if (tileSet==-1)
			descFill.setTile (0, 0, 0, tileIndex (false, 0,0), tileIndex (false, 0,0), tileIndex (false, 0,0));

		// Tiles numbers
		uint numU=1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesU;
		uint numV=1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesV;

		// For all tiles
		for (uint v=0; v<numV; v+=(1<<(int)_256))
		for (uint u=0; u<numU; u+=(1<<(int)_256))
		{
			// Yes, if we are not in clear mode and not in 256 or if it is a top 256x256 tile
			int nTile=0;

			if (tileSet!=-1)
				nTile=_MouseProc->selectTile (tileSet, false, _256, group, bank);

			// Quit if no tile
			if (nTile==-1)
				return;

			// For all tiles in this tile
			for (uint vv=0; vv<(uint)_256+1; vv++)
			for (uint uu=0; uu<(uint)_256+1; uu++)
			{
				// Get the tile pointer
				EPM_PaintTile *pTile=&_MouseProc->metaTile[mesh][patch*NUM_TILE_SEL+(vv+v)*MAX_TILE_IN_PATCH+uu+u];
				nlassert (pTile->Mesh==mesh);
				nlassert (pTile->patch==patch);

				if (tileSet!=-1)
				{
					// 256 tile ?
					if (_256)
					{
						switch (((uu&1)<<1)|(vv&1))
						{
						case 0:
							// LeftTop
							descFill.setTile (1, ((0-rot)&3)+1, 0, tileIndex (false, nTile, rot), tileIndex (false, 0, 0), tileIndex (false, 0, 0));
							break;
						case 1:
							// LeftBottom
							descFill.setTile (1, ((1-rot)&3)+1, 0, tileIndex (false, nTile, rot), tileIndex (false, 0, 0), tileIndex (false, 0, 0));
							break;
						case 2:
							// RightTop
							descFill.setTile (1, ((3-rot)&3)+1, 0, tileIndex (false, nTile, rot), tileIndex (false, 0, 0), tileIndex (false, 0, 0));
							break;
						case 3:
							// RightBottom
							descFill.setTile (1, ((2-rot)&3)+1, 0, tileIndex (false, nTile, rot), tileIndex (false, 0, 0), tileIndex (false, 0, 0));
							break;
						default:
							nlassert (0);		// no !
						}
					}
					else
						// Set the tile
						descFill.setTile (1, 0, 0, tileIndex (false, nTile, rot), tileIndex (false, 0, 0), tileIndex (false, 0, 0));

					// Check if neighborhood must be cleared
					for (uint n=0; n<4; n++)
					{
						// Neighborhood not in this patch?
						if ((pTile->voisins[n])&&((pTile->voisins[n]->Mesh!=mesh)||(pTile->voisins[n]->patch!=patch)))
						{
							bool compatible=false;

							// Neigborhood not compatible ?
							tileDesc descNei;
							_MouseProc->GetTile (pTile->voisins[n]->Mesh, pTile->voisins[n]->tile, descNei, vectMesh, _Landscape);

							// If empty continue
							if (descNei.getNumLayer()==0)
								continue;

							// Voisin have one layer
							if (descNei.getNumLayer()==1)
							{
								// Check rotation
								if (descNei.getLayer(0).Rotate==((pTile->rotate[n]+rot)&3))
								{
									// Get the tile set
									int neiTileSet;
									int number;
									CTileBank::TTileType type;
									bank.getTileXRef (descNei.getLayer(0).Tile, neiTileSet, number, type);

									// Same tileSet
									if (tileSet==neiTileSet)
										continue;
								}
							}

							// Get the displace index
							tileDesc descOrig;
							_MouseProc->GetTile (pTile->voisins[n]->Mesh, pTile->voisins[n]->tile, descOrig, vectMesh, _Landscape);

							// Not compatible, clear it
							descClear.setDisplace (descOrig.getDisplace());
							_MouseProc->SetTile (pTile->voisins[n]->Mesh, pTile->voisins[n]->tile, descClear, vectMesh, _Landscape, nelPatchChg, NULL);
						}
					}
				}

				// Get the displace index
				tileDesc descOrig;
				_MouseProc->GetTile (mesh, pTile->tile, descOrig, vectMesh, _Landscape);

				// Fill the current tile with the current tileset and rotation
				descFill.setDisplace (descOrig.getDisplace ());
				_MouseProc->SetTile (mesh, pTile->tile, descFill, vectMesh, _Landscape, nelPatchChg, NULL);
			}
		}
		
		// Flush nel chgt
		nelPatchChg.applyChanges (true);
	}
}

/*-------------------------------------------------------------------*/

void CFillPatch::fillColor (int mesh, int patch, std::vector<EPM_Mesh> &vectMesh, const CRGBA& color, uint16 blend, CPaintColor& paintColor)
{
	// Only if the landscape is valid
	if (_Landscape)
	{
		// Nel patch changement manager
		CNelPatchChanger nelPatchChg (_Landscape);

		// Tiles numbers
		uint numU=(1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesU)+1;
		uint numV=(1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesV)+1;

		// For all tiles
		for (uint v=0; v<numV; v++)
		for (uint u=0; u<numU; u++)
		{
			// Set a vertex color with handle of the undo
			paintColor.setVertexColor (mesh, patch, u, v, color, blend, vectMesh, nelPatchChg, true);
		}
		
		// Flush nel chgt
		nelPatchChg.applyChanges (false);
	}
}

/*-------------------------------------------------------------------*/

void CFillPatch::fillDisplace (int mesh, int patch, std::vector<EPM_Mesh> &vectMesh, const CTileBank& bank)
{
	// Only if the landscape is valid
	if (_Landscape)
	{
		// Nel patch changement manager
		CNelPatchChanger nelPatchChg (_Landscape);

		// Tiles numbers
		uint numU=1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesU;
		uint numV=1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesV;

		// For all tiles
		for (uint v=0; v<numV; v++)
		for (uint u=0; u<numU; u++)
		{
			// Get the tile pointer
			EPM_PaintTile *pTile=&_MouseProc->metaTile[mesh][patch*NUM_TILE_SEL+(v)*MAX_TILE_IN_PATCH+u];
			nlassert (pTile->Mesh==mesh);
			nlassert (pTile->patch==patch);

			// Set the displace 
			_MouseProc->PutADisplacetile ( pTile, bank, vectMesh, _Landscape, nelPatchChg);
		}
		
		// Flush nel chgt
		nelPatchChg.applyChanges (true);
	}
}

/*-------------------------------------------------------------------*/

