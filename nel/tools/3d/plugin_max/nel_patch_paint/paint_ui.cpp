#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_ui.h"
#include "resource.h"
#include "3d/texture_file.h"
#include "3d/texture_mem.h"
#include "nel/misc/config_file.h"

/*-------------------------------------------------------------------*/

// Def Keys
uint PainterKeys[KeyCounter]=
{
	KeySPACE,
	KeyP,
	KeyF5,
	KeyF6,
	KeyF7,
	KeyF8,
	KeyF1,
	KeyF2,
	KeyF3,
	KeyX,
	KeyINSERT,
	KeyDELETE,
	KeyW,
	KeyF,
	KeyV,
	KeyB,
	KeyC,
	KeyPRIOR,
	KeyNEXT,
	KeyHOME,
	KeyEND,
	KeyF11,
	KeyA,
};

// Keys
const char* PainterKeysName[KeyCounter]=
{
	"Select",
	"Pick",
	"Fill0",
	"Fill1",
	"Fill2",
	"Fill3",
	"ModeTile",
	"ModeColor",
	"ModeDisplace",
	"ToggleColor",
	"SizeUp",
	"SizeDown",
	"ToggleTileSize",
	"GroupUp",
	"GroupDown",
	"BackgroundColor",
	"ToggleArrows",
	"HardnessUp",
	"HardnessDown",
	"OpacityUp",
	"OpacityDown",
	"Zouille",
	"AutomaticLighting"
};

// Load ini file

void LoadKeyCfg ();

/*-------------------------------------------------------------------*/

extern const unsigned char _small[];
extern const unsigned int _smallSize;
extern const unsigned char medium[];
extern const unsigned int mediumSize;
extern const unsigned char large[];
extern const unsigned int largeSize;
extern const unsigned char _256[];
extern const unsigned int _256Size;
extern const unsigned char _128[];
extern const unsigned int _128Size;
extern const unsigned char _0[];
extern const unsigned int _0Size;
extern const unsigned char _1[];
extern const unsigned int _1Size;
extern const unsigned char _2[];
extern const unsigned int _2Size;
extern const unsigned char _3[];
extern const unsigned int _3Size;
extern const unsigned char _4[];
extern const unsigned int _4Size;
extern const unsigned char _5[];
extern const unsigned int _5Size;
extern const unsigned char _6[];
extern const unsigned int _6Size;
extern const unsigned char _7[];
extern const unsigned int _7Size;
extern const unsigned char _8[];
extern const unsigned int _8Size;
extern const unsigned char _9[];
extern const unsigned int _9Size;
extern const unsigned char _10[];
extern const unsigned int _10Size;
extern const unsigned char _11[];
extern const unsigned int _11Size;
extern const unsigned char all[];
extern const unsigned int allSize;
extern const unsigned char light[];
extern const unsigned int lightSize;

/*-------------------------------------------------------------------*/

COLORREF backGround=0x808080;
COLORREF color1=0xffffff;
COLORREF color2=0x0;
float opa1=1.f;
float opa2=1.f;
float hard1=1.f;
float hard2=1.f;

/*-------------------------------------------------------------------*/

void CTileSetCont::build (CTileBank& bank, uint tileSet)
{
	// TileSet ref
	CTileSet* set=bank.getTileSet (tileSet);
	
	// Find a main bitmap with a valid name
	if (set->getNumTile128())
	{
		// Get the name
		std::string fileName=bank.getAbsPath()+bank.getTile (set->getTile128(0))->getRelativeFileName (CTile::diffuse);

		// Valid name?
		if (fileName!="")
		{
			// Create it
			MainBitmap=new CTextureFile (fileName);
		}
	}

	// Build group bitmaps
	for (int group=0; group<NL3D_CTILE_NUM_GROUP; group++)
	{
		int tile;

		// Look for a 128 tile in this group
		for (tile=0; tile<set->getNumTile128(); tile++)
		{
			// Tile pointer
			CTile* pTile=bank.getTile (set->getTile128 (tile));

			// Look for a tile of the group
			if (pTile->getGroupFlags ()&(1<<group))
			{
				// Get the name
				std::string fileName=bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse);

				// Valid name?
				if (fileName!="")
				{
					// Create it
					if (GroupBitmap[group]==NULL)
						GroupBitmap[group]=new CTextureFile (fileName);

					// Add to the group list
					GroupTile128[group].push_back (tile);
				}
			}
		}

		// Look for a 256 tile in this group
		for (tile=0; tile<set->getNumTile256(); tile++)
		{
			// Tile pointer
			CTile* pTile=bank.getTile (set->getTile256 (tile));

			// Look for a tile of the group
			if (pTile->getGroupFlags ()&(1<<group))
			{
				// Get the name
				std::string fileName=bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse);

				// Valid name?
				if (fileName!="")
				{
					// Create it
					if (GroupBitmap[group]==NULL)
						GroupBitmap[group]=new CTextureFile (fileName);

					// Add to the group list
					GroupTile256[group].push_back (tile);
				}
			}
		}
	}

	// Current index
	for (uint displace=0; displace<CTileSet::CountDisplace; displace++)
	{
		// Get the name
		std::string fileName=bank.getDisplacementMap (set->getDisplacementTile ((CTileSet::TDisplacement)displace));
		if (fileName=="EmptyDisplacementMap")
			fileName="";

		// Valid name?
		if (fileName!="")
		{
			// Create it
			DisplaceBitmap[displace]=new CTextureFile (bank.getAbsPath()+fileName);
			DisplaceBitmap[displace]->loadGrayscaleAsAlpha (false);
		}
	}
}

/*-------------------------------------------------------------------*/

CBankCont::CBankCont (CTileBank& bank, HINSTANCE hInstance)
{
	// Allocate bitmaps
	_smallBitmap	=	new CTextureMem ((uint8*)_small, _smallSize, false);
	mediumBitmap	=	new CTextureMem ((uint8*)medium, mediumSize, false);
	largeBitmap		=	new CTextureMem ((uint8*)large, largeSize, false);
	_256Bitmap		=	new CTextureMem ((uint8*)_256, _256Size, false);
	_128Bitmap		=	new CTextureMem ((uint8*)_128, _128Size, false);
	_0Bitmap		=	new CTextureMem ((uint8*)_0, _0Size, false);
	_1Bitmap		=	new CTextureMem ((uint8*)_1, _1Size, false);
	_2Bitmap		=	new CTextureMem ((uint8*)_2, _2Size, false);
	_3Bitmap		=	new CTextureMem ((uint8*)_3, _3Size, false);
	_4Bitmap		=	new CTextureMem ((uint8*)_4, _4Size, false);
	_5Bitmap		=	new CTextureMem ((uint8*)_5, _5Size, false);
	_6Bitmap		=	new CTextureMem ((uint8*)_6, _6Size, false);
	_7Bitmap		=	new CTextureMem ((uint8*)_7, _7Size, false);
	_8Bitmap		=	new CTextureMem ((uint8*)_8, _8Size, false);
	_9Bitmap		=	new CTextureMem ((uint8*)_9, _9Size, false);
	_10Bitmap		=	new CTextureMem ((uint8*)_10, _10Size, false);
	_11Bitmap		=	new CTextureMem ((uint8*)_11, _11Size, false);
	allBitmap		=	new CTextureMem ((uint8*)all, allSize, false);
	lightBitmap		=	new CTextureMem ((uint8*)light, lightSize, false);

	// Resize the tileset array
	TileSet.resize (bank.getTileSetCount());

	// For each tileSet, build the cont
	for (int tileSet=0; tileSet<bank.getTileSetCount(); tileSet++)
		TileSet[tileSet].build (bank, tileSet);

	// Load cursors
	HCur = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_PICK_COLOR));
	HFill = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_FILL));
	HTrick = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_TRICK));
}

/*-------------------------------------------------------------------*/

// Open a pick color dialog and select a color
void chooseAColor ()
{
	// Call the color picker dialog
	static COLORREF arrayColor[16];
	CHOOSECOLOR cc;
	
	// Reset the struct
	memset (&cc, 0, sizeof(CHOOSECOLOR));

	// Fill the struct
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.rgbResult=color1;
	cc.lpCustColors=arrayColor;
	cc.Flags=CC_RGBINIT|CC_ANYCOLOR|CC_FULLOPEN;

	// Open it
	if (ChooseColor (&cc))
		// Set the color
		color1=cc.rgbResult;
}

/*-------------------------------------------------------------------*/

// Set background color
void setBackgroundColor ()
{
	// Call the color picker dialog
	static COLORREF arrayColor[16];
	CHOOSECOLOR cc;
	
	// Reset the struct
	memset (&cc, 0, sizeof(CHOOSECOLOR));

	// Fill the struct
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.rgbResult=backGround;
	cc.lpCustColors=arrayColor;
	cc.Flags=CC_RGBINIT|CC_ANYCOLOR|CC_FULLOPEN;

	// Open it
	if (ChooseColor (&cc))
		// Set the color
		backGround=cc.rgbResult;
}

/*-------------------------------------------------------------------*/

void LoadKeyCfg ()
{
	// Path of the dll
	HMODULE hModule = GetModuleHandle("neleditpatch.dlm");
	if (hModule)
	{
		char sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);
		if (res)
		{
			// split path
			char drive[256];
			char dir[256];
			_splitpath (sModulePath, drive, dir, NULL, NULL);

			// Make a new path
			char cgfPath[256];
			_makepath (cgfPath, drive, dir, "keys", ".cfg");

	
			CConfigFile cf;

			// Load and parse "test.txt" file
			cf.load (cgfPath);
			
			// For each keys
			for (uint key=0; key<KeyCounter; key++)
			{
				// go
				try
				{
					// Get the foo variable (suppose it's a string variable)
					CConfigFile::CVar &value= cf.getVar (PainterKeysName[key]);

					// Get value
					PainterKeys[key]=value.asInt ();
				}
				catch (EConfigFile &e)
				{
					// Something goes wrong... catch that
					const char* what=e.what();
				}
			}
		}
	}
}