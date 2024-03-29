// View.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "View.h"
#include "SelectionTerritoire.h"
#include "select_rotation.h"
#include "Browse.h"
#include "popup.h"
#include <direct.h>
//#include "ListGroup.h"
#include "nel/../../src/3d/tile_bank.h"
//#include "ViewPopup.h"
#include "pic\readpic.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/


#ifdef NL_NEW
	#undef new
#endif

using namespace NL3D;

extern CTileBank tileBank2;

BEGIN_MESSAGE_MAP(CTView, CStatic)
	//{{AFX_MSG_MAP(CTView)
	ON_WM_DROPFILES()
	ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFont *normal_font = NULL;
//int SortTile = 0; int sortMode = 1; //or, and
__int64 flagGroupSort = 0; int showNULL = 0;
//ViewPopup *theViewPopup = 0;

bool zouille ()
{
	return (GetAsyncKeyState(VK_F2)&(1<<15)) != 0;
};

// Rotate a buffer
void rotateBuffer (uint &Width, uint &Height, std::vector<NLMISC::CBGRA>& Tampon)
{
	// Make a copy
	std::vector<NLMISC::CBGRA> copy=Tampon;

	// Rotate
	for (uint y=0; y<Width; y++)
	{
		// Line offset
		uint tmp=y*Width;
		for (uint x=0; x<Width; x++)
		{
			Tampon[y+(Width-x-1)*Height]=copy[x+tmp];
		}
	}

	// New size
	uint tmp=Width;
	Width=Height;
	Height=tmp;
}

/////////////////////////////////////////////////////////////////////////////
// CTView
//Attention : windows veut que le buffer image commence du bas vers le haut
int _LoadBitmap(const std::string& path,LPBITMAPINFO BitmapInfo, std::vector<NLMISC::CBGRA>&Tampon, std::vector<NLMISC::CBGRA>* Alpha, int rot)
{
	//vector<NLMISC::CBGRA> Tampon;
	uint Width;
	uint Height;
	if (PIC_LoadPic(path, Tampon, Width, Height))
	{
		BitmapInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		BitmapInfo->bmiHeader.biWidth=Width;
		BitmapInfo->bmiHeader.biHeight=-(int)Height;
		BitmapInfo->bmiHeader.biPlanes=1;
		BitmapInfo->bmiHeader.biBitCount=32;
		BitmapInfo->bmiHeader.biCompression=BI_RGB;
		BitmapInfo->bmiHeader.biSizeImage=0;
		BitmapInfo->bmiHeader.biXPelsPerMeter=1;
		BitmapInfo->bmiHeader.biYPelsPerMeter=1;
		BitmapInfo->bmiHeader.biClrUsed=0;
		BitmapInfo->bmiHeader.biClrImportant=0;
		BitmapInfo->bmiColors->rgbBlue=0;
		BitmapInfo->bmiColors->rgbRed=0;
		BitmapInfo->bmiColors->rgbGreen=0;

		while (rot)
		{
			// Rotate the buffer
			rotateBuffer (Width, Height, Tampon);
			rot--;
		}

		if ((Alpha)&&(Alpha->size()==Tampon.size()))
		{
			// Pre mul RGB componates by Alpha one
			int nPixelCount=(int)(Width*Height);
			for (int p=0; p<nPixelCount; p++)
			{
				// Invert alpha ?
				int alpha=(*Alpha)[p].A;
				Tampon[p].R=(uint8)(((int)Tampon[p].R*alpha)>>8);
				Tampon[p].G=(uint8)(((int)Tampon[p].G*alpha)>>8);
				Tampon[p].B=(uint8)(((int)Tampon[p].B*alpha)>>8);
			}
		}

		return 1;
	}
	else
		return 0;
}






//TileInfo
TileInfo::TileInfo()
{
	nightLoaded = 0;
	alphaLoaded = 0;
	loaded = 0;
	Selected = 0; 
}



//TileList
TileList::TileList()
{
	last_id = 0; 
	_tileSet = -1;

	// Add 48 transitions
	int i;
	for (i=0; i<CTileSet::count; i++)
	{
		TileInfo info;
		info.id = i;
	
		theListTransition.push_back (info);
	}

	// Add 16 displacements
	for (i=0; i<CTileSet::CountDisplace; i++)
	{
		TileInfo info;
		info.id = i;
	
		theListDisplacement.push_back (info);
	}
}


int TileList::addTile128 ()
{

	int index;
	tileBank2.getTileSet (_tileSet)->addTile128 (index, tileBank2);
	nlassert (index==(sint)theList128.size());

	TileInfo info;
	info.id = index;
	theList128.push_back (info);

	return index;
}

int TileList::addTile256 ()
{
	int index;
	tileBank2.getTileSet (_tileSet)->addTile256 (index, tileBank2);
	nlassert (index==(sint)theList256.size());

	TileInfo info;
	info.id = index;
	theList256.push_back (info);

	return index;
}

bool RemovePath (std::string& path, const char* absolutePathToRemplace);

int TileList::setTile128 (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBank2.getAbsPath ().c_str()))
	{
		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!PIC_LoadPic(tileBank2.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return (int)(MessageBox (NULL, ((tileBank2.getAbsPath ()+troncated)+"\nContinue ?").c_str(), "Can't load bitmap.", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);

			CTileSet::TError error;
			int pixel=-1;
			int composante=4;
			if (type == CTile::additive)
				error=CTileSet::ok;
			else
				error=tileBank2.getTileSet(_tileSet)->checkTile128 (type, border, pixel, composante);
			if ((error!=CTileSet::ok)&&(error!=CTileSet::addFirstA128128)&&!zouille ())
			{
				char sTmp[512];
				static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
				sprintf (sTmp, "%s\nPixel: %d (%s)", CTileSet::getErrorMessage (error), pixel, comp[composante]);
				return (int)(MessageBox (NULL, (std::string(sTmp)+"\nContinue ?").c_str(), "Can't add tile", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
			}
			else
			{
				if (error==CTileSet::addFirstA128128)
					tileBank2.getTileSet(_tileSet)->setBorder (type, border);

				tileBank2.getTileSet(_tileSet)->setTile128 (tile, troncated, type, tileBank2);
				switch (type)
				{
				case CTile::diffuse:
					theList128[tile].loaded=0;
					break;
				case CTile::additive:
					theList128[tile].nightLoaded=0;
					break;
				case CTile::alpha:
					theList128[tile].alphaLoaded=0;
					break;
				}
				theList128[tile].Load (tileBank2.getTileSet(_tileSet)->getTile128(tile), NULL);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		char msg[512];
		sprintf (msg, "The bitmap %s is not in the absolute path %s.", name.c_str(), tileBank2.getAbsPath ().c_str());
		return (int)(MessageBox (NULL, (std::string (msg)+"\nContinue ?").c_str(), "Load error", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
	}

	return 1;
}

int TileList::setTile256 (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBank2.getAbsPath ().c_str()))
	{
		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!PIC_LoadPic(tileBank2.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return (int)(MessageBox (NULL, ((tileBank2.getAbsPath ()+troncated)+"\nContinue ?").c_str(), "Can't load bitmap.", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);
			
			CTileSet::TError error;
			int pixel=-1;
			int composante=4;

			if (type == CTile::additive)
				error=CTileSet::ok;
			else
				error=tileBank2.getTileSet(_tileSet)->checkTile256 (type, border, pixel, composante);
			if ((error!=CTileSet::ok)&&!zouille())
			{
				char sTmp[512];
				static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
				sprintf (sTmp, "%s\nPixel: %d (%s)", CTileSet::getErrorMessage (error), pixel, comp[composante]);
				return (int)(MessageBox (NULL, (std::string(sTmp)+"\nContinue ?").c_str(), "Can't add tile", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
			}
			else
			{
				tileBank2.getTileSet(_tileSet)->setTile256 (tile, troncated, type, tileBank2);
						switch (type)
				{
				case CTile::diffuse:
					theList256[tile].loaded=0;
					break;
				case CTile::additive:
					theList256[tile].nightLoaded=0;
					break;
				case CTile::alpha:
					theList256[tile].alphaLoaded=0;
					break;
				}
				theList256[tile].Load (tileBank2.getTileSet(_tileSet)->getTile256(tile), NULL);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		char msg[512];
		sprintf (msg, "The bitmap %s is not in the absolute path %s.", name.c_str(), tileBank2.getAbsPath ().c_str());
		return (int)(MessageBox (NULL, (std::string (msg)+"\nContinue ?").c_str(), "Load error", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
	}

	return 1;
}

int TileList::setTileTransition (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBank2.getAbsPath ().c_str()))
	{
		// No alpha, use setTileTransitionAlpha
		nlassert (CTile::alpha!=type);

		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!PIC_LoadPic(tileBank2.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return (int)(MessageBox (NULL, ((tileBank2.getAbsPath ()+troncated)+"\nContinue ?").c_str(), "Can't load bitmap.", MB_YESNO|MB_ICONEXCLAMATION)==IDYES);
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);
			
			CTileSet::TError error;
			int pixel=-1;
			int composante=4;
			if (type == CTile::additive)
				error=CTileSet::ok;
			else
				error=tileBank2.getTileSet(_tileSet)->checkTile128 (type, border, pixel, composante);
			if ((error!=CTileSet::ok)&&(error!=CTileSet::addFirstA128128)&&!zouille ())
			{
				char sTmp[512];
				static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
				sprintf (sTmp, "%s\nPixel: %d (%s)", CTileSet::getErrorMessage (error), pixel, comp[composante]);
				return MessageBox (NULL, (std::string(sTmp)+"\nContinue ?").c_str(), "Can't add tile", MB_YESNO|MB_ICONEXCLAMATION)==IDYES;
			}
			else
			{
				if (error==CTileSet::addFirstA128128)
					tileBank2.getTileSet(_tileSet)->setBorder (type, border);
				tileBank2.getTileSet(_tileSet)->setTileTransition ((CTileSet::TTransition)tile, troncated, type, tileBank2, border);
				switch (type)
				{
				case CTile::diffuse:
					theListTransition[tile].loaded=0;
					break;
				case CTile::additive:
					theListTransition[tile].nightLoaded=0;
					break;
				case CTile::alpha:
					theListTransition[tile].alphaLoaded=0;
					break;
				}
				theListTransition[tile].Load (tileBank2.getTileSet(_tileSet)->getTransition(tile)->getTile(), 
					&theListTransition[tile].alphaBits);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		char msg[512];
		sprintf (msg, "The bitmap %s is not in the absolute path %s.", name.c_str(), tileBank2.getAbsPath ().c_str());
		MessageBox (NULL, msg, "Load error", MB_OK|MB_ICONEXCLAMATION);
	}

	return 1;
}

int TileList::setDisplacement (int tile, const std::string& name)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBank2.getAbsPath ().c_str()))
	{
		// load it
		if (troncated!="")
		{
			// not loaded
			theListDisplacement[tile].loaded=0;

			if (!_LoadBitmap(tileBank2.getAbsPath() + troncated, &theListDisplacement[tile].BmpInfo, theListDisplacement[tile].Bits, NULL, 0))
				MessageBox (NULL, (tileBank2.getAbsPath() + troncated).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
			else
			{
				// Check the size
				if ((theListDisplacement[tile].BmpInfo.bmiHeader.biWidth!=32)||(-theListDisplacement[tile].BmpInfo.bmiHeader.biHeight!=32))
				{
					// Error message
					MessageBox (NULL, "Invalid size: displacement map must be 32x32 8 bits.", troncated.c_str(),
						MB_OK|MB_ICONEXCLAMATION);

					// Free the bitmap
					theListDisplacement[tile].Bits.resize (0);
				}
				else
				{
					// Loaded
					theListDisplacement[tile].loaded=1;

					// change the file name of the displacement map
					tileBank2.getTileSet(_tileSet)->setDisplacement ((CTileSet::TDisplacement)tile, troncated, tileBank2);
				}
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		char msg[512];
		sprintf (msg, "The bitmap %s is not in the absolute path %s.", name.c_str(), tileBank2.getAbsPath ().c_str());
		MessageBox (NULL, msg, "Load error", MB_OK|MB_ICONEXCLAMATION);
	}

	return 1;
}

int TileList::setTileTransitionAlpha (int tile, const std::string& name, int rot)
{
	// Remove the absolute path from the path name
	std::string troncated=name;
	if (RemovePath (troncated, tileBank2.getAbsPath ().c_str()))
	{
		vector<NLMISC::CBGRA> tampon;
		uint Width;
		uint Height;
		if (!PIC_LoadPic(tileBank2.getAbsPath ()+troncated, tampon, Width, Height))
		{
			return MessageBox (NULL, ((tileBank2.getAbsPath ()+troncated)+"\nContinue ?").c_str(), "Can't load bitmap.", MB_YESNO|MB_ICONEXCLAMATION)==IDYES;
		}
		else
		{
			CTileBorder border;
			border.set (Width, Height, tampon);

			// rotate the border
			int rotBis=rot;
			while (rotBis)
			{
				border.rotate ();
				rotBis--;
			}
			
			CTileSet::TError error;
			int indexError;
			int pixel=-1;
			int composante=4;
			if (((error=tileBank2.getTileSet(_tileSet)->checkTileTransition ((CTileSet::TTransition)tile, CTile::alpha, border, indexError,
				pixel, composante))!=CTileSet::ok)&&!zouille ())
			{
				char sMsg[512];
				if ((error==CTileSet::topInterfaceProblem)||(error==CTileSet::bottomInterfaceProblem)||(error==CTileSet::leftInterfaceProblem)
					||(error==CTileSet::rightInterfaceProblem)||(error==CTileSet::topBottomNotTheSame)||(error==CTileSet::rightLeftNotTheSame)
					||(error==CTileSet::topInterfaceProblem))
				{
					static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
					if (indexError!=-1)
						sprintf (sMsg, "%s\nIncompatible with tile nb %d\nPixel: %d (%s)", CTileSet::getErrorMessage (error), indexError,
							pixel, comp[composante]);
					else
						sprintf (sMsg, "%s\nIncompatible with the 128x128 tile\nPixel: %d (%s)", CTileSet::getErrorMessage (error),
							pixel, comp[composante]);
				}
				else
					sprintf (sMsg, "%s\nIncompatible filled tile", CTileSet::getErrorMessage (error));
				
				return MessageBox (NULL, (std::string(sMsg)+"\nContinue ?").c_str(), "Can't add tile", MB_YESNO|MB_ICONEXCLAMATION)==IDYES;
			}
			else
			{
				tileBank2.getTileSet(_tileSet)->setTileTransitionAlpha ((CTileSet::TTransition)tile, troncated, tileBank2, border, rot);
				theListTransition[tile].alphaLoaded=0;
				theListTransition[tile].Load (tileBank2.getTileSet(_tileSet)->getTransition(tile)->getTile(), NULL);
			}
		}
	}
	else
	{
		// Error: bitmap not in the absolute path..
		char msg[512];
		sprintf (msg, "The bitmap %s is not in the absolute path %s.", name.c_str(), tileBank2.getAbsPath ().c_str());
		MessageBox (NULL, msg, "Load error", MB_OK|MB_ICONEXCLAMATION);
	}

	return 1;
}

void TileList::removeTile128 (int index)
{
	tileBank2.getTileSet (_tileSet)->removeTile128 (index, tileBank2);
	theList[0].erase (theList[0].begin()+index);
	for (int i=0; i<(sint)theList[0].size(); i++)
	{
		theList[0][i].id=i;
	}
}

void TileList::removeTile256 (int index)
{
	tileBank2.getTileSet (_tileSet)->removeTile256 (index, tileBank2);
	theList[1].erase (theList[1].begin()+index);
	for (int i=0; i<(sint)theList[1].size(); i++)
	{
		theList[1][i].id=i;
	}
}

void TileList::clearTile128 (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
	case CTile::diffuse:
		theList128[index].loaded=0;
		theList128[index].Bits.resize(0);
		break;
	case CTile::additive:
		theList128[index].nightLoaded=0;
		theList128[index].nightBits.resize(0);
		break;
	case CTile::alpha:
		theList128[index].alphaLoaded=0;
		theList128[index].alphaBits.resize(0);
		break;
	}
	tileBank2.getTileSet (_tileSet)->clearTile128 (index, bitmap, tileBank2);
}

void TileList::clearTile256 (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
	case CTile::diffuse:
		theList256[index].loaded=0;
		theList256[index].Bits.resize(0);
		break;
	case CTile::additive:
		theList256[index].nightLoaded=0;
		theList256[index].nightBits.resize(0);
		break;
	case CTile::alpha:
		theList256[index].alphaLoaded=0;
		theList256[index].alphaBits.resize(0);
		break;
	}
	tileBank2.getTileSet (_tileSet)->clearTile256 (index, bitmap, tileBank2);
}

void TileList::clearTransition (int index, CTile::TBitmap bitmap)
{
	switch (bitmap)
	{
	case CTile::diffuse:
		theListTransition[index].loaded=0;
		theListTransition[index].Bits.resize(0);
		break;
	case CTile::additive:
		theListTransition[index].nightLoaded=0;
		theListTransition[index].nightBits.resize(0);
		break;
	case CTile::alpha:
		theListTransition[index].alphaLoaded=0;
		theListTransition[index].alphaBits.resize(0);
		break;
	}
	tileBank2.getTileSet (_tileSet)->clearTransition ((CTileSet::TTransition)index, bitmap, tileBank2);
}

void TileList::clearDisplacement (int index)
{
	// Clear the displacement map filename
	tileBank2.getTileSet (_tileSet)->clearDisplacement ((CTileSet::TDisplacement)index, tileBank2);
	theListDisplacement[index].loaded=0;
	theListDisplacement[index].Bits.resize(0);
}

tilelist::iterator TileList::GetFirst(int n)
{
	//UpdateLF();
	return theList[n].begin();
}

tilelist::iterator TileList::GetLast(int n)
{
	//UpdateLF();
	return theList[n].end();
}

int TileList::GetSize(int n)
{
	//UpdateLF();
	return theList[n].size();
}

void TileInfo::Delete ()
{
	loaded=0;
	nightLoaded=0;
	alphaLoaded=0;
}


tilelist::iterator TileList::Get(int i, int n)
{
	return theList[n].begin()+i;
}

const std::string& TileInfo::getRelativeFileName (CTile::TBitmap type, int index)
{
	return tileBank2.getTile (index)->getRelativeFileName (type);
}

bool TileInfo::Load (int index, std::vector<NLMISC::CBGRA>* Alpha)
{
	bool bRes=true;
	if (!loaded && getRelativeFileName (CTile::diffuse, index)!="")
	{
		if (!_LoadBitmap(tileBank2.getAbsPath() + getRelativeFileName (CTile::diffuse, index), &BmpInfo, Bits, Alpha, 0))
		{
			bRes=false;
			MessageBox (NULL, (tileBank2.getAbsPath() + getRelativeFileName (CTile::diffuse, index)).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
		}
		else
			loaded=1;
	}
	if (!nightLoaded && getRelativeFileName (CTile::additive, index)!="")
	{
		if (!_LoadBitmap(tileBank2.getAbsPath() + getRelativeFileName (CTile::additive, index), &nightBmpInfo, nightBits, Alpha, 0))
		{
			bRes=false;
			MessageBox (NULL, (tileBank2.getAbsPath() + getRelativeFileName (CTile::additive, index)).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
		}
		else
			nightLoaded=1;
	}
	if (!alphaLoaded && getRelativeFileName (CTile::alpha, index)!="")
	{
		if (!_LoadBitmap(tileBank2.getAbsPath() + getRelativeFileName (CTile::alpha, index), &alphaBmpInfo, alphaBits, NULL, 
			tileBank2.getTile (index)->getRotAlpha ()))
		{
			bRes=false;
			MessageBox (NULL, (tileBank2.getAbsPath() + getRelativeFileName (CTile::alpha, index)).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
		}
		else
			alphaLoaded=1;
	}
	return bRes;
}

void TileList::Reload(int first, int count, int n) //recharge en memoire une tranche de tiles
{
	for (int i=first; i<count; i++)
	{
		switch (n)
		{
		case 0:
			theList[n][i].Load (tileBank2.getTileSet(_tileSet)->getTile128 (i), NULL);
			break;
		case 1:
			theList[n][i].Load (tileBank2.getTileSet(_tileSet)->getTile256 (i), NULL);
			break;
		case 2:
			{
				int index=tileBank2.getTileSet(_tileSet)->getTransition (i)->getTile();
				if (index!=-1)
					theList[n][i].Load (index, &theListTransition[i].alphaBits);
			}
			break;
		}
	}
}

//CTView
CTView::CTView()
{
	sizetile_x = SIZE_SMALL; sizetile_y = SIZE_SMALL;
	sizetext_y = 14;
	spacing_x = SPACING_SMALL_X; spacing_y = SPACING_SMALL_Y; 
	Zoom=3; Texture = 1; Sort = 1; InfoTexte = 1;
	count_ = 0; ViewTileMode = 0;
	scrollpos = 0; lastVBarPos = 0;
	smEdgeList = 0; bPopup = 0;
	//theViewPopup = 0;
}

CTView::~CTView()
{
}

void CTView::Init(int _land, int n)
{	
	InfoList._tileSet=_land;
	UpdateSize(n);
	pipo_buffer = (void *)new char[sizetile_x * sizetile_y * 3];
	bmp = new CBitmap;
	bmp->CreateBitmap(sizetile_x,sizetile_y,1,24,pipo_buffer);
	pImList = new CImageList;
	pImList->Create(sizetile_x,sizetile_y,ILC_COLOR24,0,1);
	pImList->Add(bmp,(CBitmap*)NULL);
	char *defautpath = ((SelectionTerritoire*)GetParent()->GetParent())->DefautPath.GetBuffer(256);
	count_=1;
}

void CTView::Delete()
{
	count_=0; pImList = 0;
}

int CTView::GetNbTileLine(void)
{
	RECT rect; GetClientRect(&rect);
	return ( max ( 1, int( rect.right - rect.left - spacing_x ) / ( sizeicon_x + spacing_x ) ) );
}

int CTView::GetNbTileColumn(void)
{
	RECT rect; GetClientRect(&rect);
	int deb;
	if (scrollpos<spacing_y) deb = -scrollpos;
	else deb = (scrollpos - spacing_y)%(sizeicon_y + spacing_y);
	if (deb>sizeicon_y) deb -=sizeicon_y + spacing_y;
	int ret= ((rect.bottom - rect.top /*- spacing_y*/ + deb)/(sizeicon_y + spacing_y)) +1 ;
	return max (1, ret);
}

void CTView::GetVisibility(int &First,int &Last, int n) //retourne l'indice du premier et du dernier item visible dans la fenetre
{
	int y;
	int i = GetNbTileLine(); int j = GetNbTileColumn();
	if (scrollpos<spacing_y) y = 0;
	else y = (scrollpos - spacing_y)%(sizeicon_y + spacing_y);
	First = ((scrollpos - spacing_y)/(sizeicon_y + spacing_y));
	if (y>sizeicon_y) First++;
	First *= i;
	Last = First + i*j -1;
	if (InfoList.GetSize(n)>0 && Last>=InfoList.GetSize(n)) Last = InfoList.GetSize(n)-1;
	if (First>Last) First = Last;
}

int CTView::GetIndex(LPPOINT pt, int n) //retourne l'index d'un incone a partir de sa position dans le fenetre
//si le curseur n'est pas sur un icon, retourne -1
{
	POINT pts = *pt;
	pts.y += scrollpos;
	RECT rect; GetClientRect(&rect);
	int i = GetNbTileLine();
	int lf = (rect.right - rect.left) - (GetNbTileLine()*(spacing_x+sizeicon_x) + spacing_x);
	if (pts.x > (rect.right - rect.left - lf)) return -1;
	if (pts.x<0) return -1;
	if (pts.y<spacing_y) return -1;
	int y = (pts.y - spacing_y)%(spacing_y + sizeicon_y);
	if (y>sizeicon_y) return -1;
	int il = (pts.y - spacing_y)/(spacing_y + sizeicon_y);
	int x = (pts.x - spacing_x)%(spacing_x + sizeicon_x);
	if (x>sizeicon_x) return -1;
	int ic = (pts.x - spacing_x)/(spacing_x + sizeicon_x);
	int ret = ic + il*i;
	if (ret<InfoList.GetSize(n) && ret>=0) return ret;
	else return -1;
}

POINT CTView::GetPos(int i) //fonction inverse de GetIndex
{
	POINT ret;
	int nl = max (1, GetNbTileLine());
	ret.x = (i%nl)*(spacing_x + sizeicon_x) + spacing_x;
	ret.y = (i/nl)*(spacing_y + sizeicon_y) + spacing_y - scrollpos;	
	return ret;
}

void CTView::UpdateSelection(LPRECT rect_,int mode, int n) //rect : coordonnees du rectangle de selection dans la fenetre parent
{	
	RECT client,*rect; GetWindowRect(&client);
	rect = new RECT; memcpy(rect,rect_,sizeof(RECT));
	//on se met dans le repere de CTView
	POINT pt; pt.x = rect->left; pt.y = rect->top;
	GetParent()->ClientToScreen(&pt);
	ScreenToClient(&pt);
	rect->left = pt.x;
	rect->top = pt.y;	
	pt.x = rect->right; pt.y = rect->bottom;
	GetParent()->ClientToScreen(&pt);
	ScreenToClient(&pt);
	rect->right = pt.x;
	rect->bottom = pt.y;
	rect->top += scrollpos; rect->bottom += scrollpos;

	if (rect->left<300)
	{
		int toto = 0;
	}

	//on clip
	if (rect->left<0) rect->left = 0;
	if (rect->top<scrollpos) rect->top = scrollpos;
	int lf = (client.right - client.left) - (GetNbTileLine()*(spacing_x+sizeicon_x) + spacing_x);
	if (rect->right>(client.right-client.left-spacing_x-lf)) 
	{
		rect->right = client.right - client.left - spacing_x - lf;
	}
	if ((rect->bottom-scrollpos)>(client.bottom-client.top)) rect->bottom=client.bottom-client.top+scrollpos;	
	
	if (rect->left<spacing_x) rect->left = spacing_x;
	if ((rect->top-scrollpos)<spacing_y) rect->top = spacing_y + scrollpos;
	int rx = (rect->left - spacing_x)%(spacing_x + sizeicon_x);
	rect->left -= rx;
	if (rx>=sizeicon_x)
	{
		rect->left += sizeicon_x + spacing_x;
		if (rect->left>=(client.right - client.left)) rect->left = (client.right - client.left) - spacing_x;
	}
	int ry = (rect->top - spacing_y)%(spacing_y + sizeicon_y);
	rect->top -= ry;
	if (ry>=sizeicon_y)
	{
		rect->top += sizeicon_y + spacing_y;
		if ((rect->top-scrollpos)>=(client.bottom - client.top)) rect->top = (client.bottom - client.top) - spacing_y + scrollpos;
	}

	
	if (rect->right<spacing_x || (rect->bottom-scrollpos)<spacing_y) return;
	rx = (rect->right - spacing_x)%(spacing_x + sizeicon_x);
	rect->right += (sizeicon_x - rx -1 );
	ry = (rect->bottom - spacing_y)%(spacing_y + sizeicon_y);
	rect->bottom += (sizeicon_y - ry -1 );

	if (rect->bottom<rect->top || rect->left>rect->right) return;
	pt; pt.x = rect->left; pt.y = rect->top; pt.y -=scrollpos;
	int index = GetIndex(&pt, n);
	tilelist::iterator p = InfoList.GetFirst(n);	
	CDC *pDC = GetDC();
	if (index==-1 && !(mode&MK_SHIFT))
	{
		for (int i = 0;i<InfoList.GetSize(n); i++)
		{
			if (p->Selected==1)
			{
				p->Selected = 0;
				DrawTile(p,pDC,0,n);
			}
			p++;
		}
		if (pDC) 
			::ReleaseDC(*this,*pDC);
		return;
	}
	for (int i = 0;i<index;i++) 
	{
		if (p==InfoList.GetLast(n))
		{
			if (pDC!=NULL) 
				::ReleaseDC(*this,*pDC);
			return;
		}
		if (p->Selected==1) 
		{
			p->Selected = 0;
			DrawTile(p,pDC,0,n);
		}
		else if (p->Selected&4)
		{
			if (p->Selected) p->Selected=2;
			else p->Selected = 3;
			DrawTile(p,pDC,0,n);
		}
		p++;
	}
	int nbline = GetNbTileLine();
	int incd = index%nbline;
	for (int j = rect->top;j<=rect->bottom;j+=sizeicon_y + spacing_y)
	{
		int k = 0;
		if (j!=rect->top)
		{
			for (;k<incd;k++) 
			{
				if (p==InfoList.GetLast(n))
				{
					if (pDC!=NULL) 
						::ReleaseDC(*this,*pDC);
					return;
				}
				if (p->Selected==1)
				{
					p->Selected = 0; 
					DrawTile(p,pDC,0,n);
				}				
				else if (p->Selected&4)
				{
					if (p->Selected&3) p->Selected=2;
					else p->Selected = 3;
					int k = !(p->Selected&1);
					DrawTile(p,pDC,0,n);
				}
				p++;
			}
		}
		else k = incd;
		for (int i = rect->left;i<=rect->right;i+=sizeicon_x + spacing_x)
		{
			if (p==InfoList.GetLast(n)) 
			{
				if (pDC!=NULL) 
					::ReleaseDC(*this,*pDC);
				return;
			}
			if (!(mode&MK_CONTROL))
			{
				if (p->Selected!=1)
				{
					p->Selected = 1;
					DrawTile(p,pDC,0,n);
				}
			}
			else
			{				
				if ((p->Selected&4)==0)
				{
					int k = p->Selected;
					p->Selected = (p->Selected&3)?4:5;				
					k = p->Selected;
					DrawTile(p,pDC,0,n);
				}
			}
			p++;
			k++;
		}
		for (;k<nbline;k++) 
		{
			if (p==InfoList.GetLast(n)) 
			{
				if (pDC!=NULL) 
					::ReleaseDC(*this,*pDC);
				return;
			}
			if (p->Selected==1) 
			{
				p->Selected = 0; 
				DrawTile(p,pDC,0,n);
			} 
			else if (p->Selected&4)
			{
				if (p->Selected&3) p->Selected=2;
				else p->Selected = 3;
				DrawTile(p,pDC,0,n);
			}
			p++;
		}
	}
	for (;p!=InfoList.GetLast(n);p++) 
	{
		if (p->Selected==1) 
		{
			p->Selected = 0;
			DrawTile(p,pDC,0,n);
		}
		else if (p->Selected&4)
		{
			if (p->Selected&3) p->Selected=2;
			else p->Selected = 3;
			DrawTile(p,pDC,0,n);
		}
	}
	if (pDC!=NULL) 
		::ReleaseDC(*this,*pDC);
}
		
void CTView::DeleteTile(tilelist::iterator p)
{
	p->loaded = 0;
	p->nightLoaded = 0;
	p->alphaLoaded = 0;
}

void CTView::UpdateBar(int iFirst,int iLast, int n)
{
	int i = GetNbTileColumn(); 
	int j = max (1, GetNbTileLine());
	int nbline = InfoList.GetSize(n)/j+1;
	int posline = iFirst/j;
	SCROLLINFO scr;
	scr.fMask = SIF_ALL ^ SIF_POS;
	scr.nMin = 0; scr.nMax = SCROLL_MAX; 
	if (nbline==0) {scr.nPage = SCROLL_MAX; scr.nPos = 0;}
	else 
	{
		scr.nPage = (SCROLL_MAX*i)/nbline;
	}
	GetParent()->SetScrollInfo(SB_VERT,&scr);
}
	



int CTView::IsSelected(int i)
{
	return 0;
}

void CTView::UpdateSize(int n)
{
	spacing_tile_text = 3;
	if (Zoom==1) {sizetile_x = sizetile_y = SIZE_SMALL; spacing_x = SPACING_SMALL_X; spacing_y = SPACING_SMALL_Y;}
	if (Zoom==2) {sizetile_x = sizetile_y = SIZE_NORMAL; spacing_x = SPACING_NORMAL_X; spacing_y = SPACING_NORMAL_Y;}
	if (Zoom==3) {sizetile_x = sizetile_y = SIZE_BIG; spacing_x = SPACING_BIG_X; spacing_y = SPACING_BIG_Y;}
	if (n==1)
	{
		sizetile_x *= 2;
		sizetile_y *= 2;
	}
	sizeicon_x = sizetile_x; sizeicon_y = sizetile_y + sizetext_y + spacing_tile_text;
}

int debug = 0;

void CTView::OnPaint() 
{
	Browse *parent = (Browse*)this->GetParent();
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	RECT rect; GetClientRect(&rect);	
	CBrush brush (GetSysColor(COLOR_3DFACE));
	
	
	dc.FillRect(&rect,&brush);
	
	if (InfoList.GetSize(parent->m_128x128)==0) return;
	
	debug=(debug+1)&1;

	if (debug==1)
	{
		debug = 1;
	}

	CRgn clip;
	clip.CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	dc.SelectClipRgn(&clip);
	
	CRgn update;
	if (GetUpdateRgn(&update)==NULLREGION)
	{
		::ReleaseDC(*this,dc);
		return;
	}

	GetVisibility(iFV, iLV, parent->m_128x128);
	UpdateBar(iFV, iLV, parent->m_128x128);
	if (!normal_font) 
	{
		normal_font = new CFont;
		normal_font->CreateFont(-10,0,0,0,FW_THIN,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,NULL);
	}
	
	tilelist::iterator p = InfoList.GetFirst(parent->m_128x128);
	int i;
	for (i=0;i<iFV;i++) p++;
	for (i=iFV;i<=iLV;i++) 
		DrawTile(p++,&dc,0, parent->m_128x128);
	::ReleaseDC (*this, dc);
}

void CTView::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	//first : on verifie s'il les tiles doivent etre inseres
	if (!lockInsertion)
	{
		Browse *parent = (Browse*)this->GetParent();
		char FileName[256];
		int count=DragQueryFile(hDropInfo,0xffffffff,FileName,256); //count = nombre de fichiers dans le drop
		

		POINT pos;
		DragQueryPoint(hDropInfo,&pos); //retrieve cursor position
		CDC *pDC = GetDC();
		for (int i=0;i<count;i++) 
		{
			DragQueryFile(hDropInfo,i,FileName,256);
			switch (parent->m_128x128)
			{
			case 0:
				{
					int index=InfoList.addTile128 ();
					if (InfoList.setTile128 (index, FileName, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha)))
					{
						tilelist::iterator it = InfoList.GetLast(parent->m_128x128);
						it--;
					}
					else
					{
						InfoList.removeTile128 (index);
					}
				}
				break;
			case 1:
				{
					int index=InfoList.addTile256 ();
					if (InfoList.setTile256 (index, FileName, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha)))
					{
						tilelist::iterator it = InfoList.GetLast(parent->m_128x128);
						it--;
					}
					else
						InfoList.removeTile256 (index);
				}
				break;
			case 2:
				{
				}
				break;
			}
		}
		
		int iFV,iLV;
		GetVisibility(iFV, iLV, parent->m_128x128);
		UpdateBar(iFV, iLV, parent->m_128x128);
		Invalidate ();
	}
	CStatic::OnDropFiles(hDropInfo);
}

void CTView::DrawTile(tilelist::iterator i,CDC *pDC,int clear, int n)
{
	RECT rect; GetClientRect(&rect);	
	CBrush brush (GetSysColor(COLOR_3DFACE));
	if (InfoList.GetSize(n)==0) 
	{
		return;
	}

	LPBITMAPINFO bmpinf;
	std::string pth; 
	std::vector<NLMISC::CBGRA> *bits;
	int loaded;

	switch (n)
	{
	case 0:
		pth = i->getRelativeFileName ((CTile::TBitmap)(Texture-1), tileBank2.getTileSet (InfoList._tileSet)->getTile128 (i->id));
		break;
	case 1:
		pth = i->getRelativeFileName ((CTile::TBitmap)(Texture-1), tileBank2.getTileSet (InfoList._tileSet)->getTile256 (i->id));
		break;
	case 2:
		{
			int index=tileBank2.getTileSet (InfoList._tileSet)->getTransition (i->id)->getTile();
			if (index!=-1)
				pth = i->getRelativeFileName ((CTile::TBitmap)(Texture-1), index);
			else
				pth = "";
		}
		break;
	case 3:
		pth = tileBank2.getDisplacementMap (tileBank2.getTileSet (InfoList._tileSet)->getDisplacementTile ((CTileSet::TDisplacement)i->id));
		break;
	}

	switch(Texture)
	{
		case 1:
			bmpinf = &(i->BmpInfo);
			bits = &i->Bits;
			loaded = i->loaded;
			break;
		case 2:
			bmpinf = &(i->nightBmpInfo);
			bits = &i->nightBits;
			loaded = i->nightLoaded;
			break;
		case 3:
			bmpinf = &(i->alphaBmpInfo);
			bits = &i->alphaBits;
			loaded = i->alphaLoaded;
			break;
	}

	CRgn clip;
	clip.CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	pDC->SelectClipRgn(&clip);
	
	// Select a font
	CFont *pOldFont=pDC->SelectObject(normal_font);
	
	POINT pt;
	pt = GetPos(i->id);
	RECT rect_txt; 
	rect_txt.top = pt.y;
	rect_txt.bottom = pt.y + sizeicon_y + spacing_y;
	rect_txt.left = pt.x; rect_txt.right = pt.x + sizeicon_x + spacing_x;

	// Turn every other pixel to black
	COLORREF clrBk = pDC->SetBkColor( GetSysColor(COLOR_3DFACE) );
	COLORREF clrText = pDC->SetTextColor( RGB(0,0,0) );
	
	if (clear) pDC->FillRect(&rect_txt,&brush);
	
	if (!loaded)
	{
		pDC->FillSolidRect( pt.x, pt.y, sizetile_x, sizetile_y, GetSysColor(COLOR_3DFACE) );
		pDC->MoveTo (pt.x,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y+sizetile_y);
		pDC->MoveTo (pt.x+sizetile_x,pt.y);
		pDC->LineTo (pt.x,pt.y+sizetile_y);
		pDC->MoveTo (pt.x,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y+sizetile_y);
		pDC->LineTo (pt.x,pt.y+sizetile_y);
		pDC->LineTo (pt.x,pt.y);

		pDC->MoveTo (pt.x+1,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y+sizetile_y-1);
		pDC->MoveTo (pt.x,pt.y+1);
		pDC->LineTo (pt.x+sizetile_x-1,pt.y+sizetile_y);
		
		pDC->MoveTo (pt.x+sizetile_x-1,pt.y);
		pDC->LineTo (pt.x,pt.y+sizetile_y-1);
		pDC->MoveTo (pt.x+sizetile_x,pt.y+1);
		pDC->LineTo (pt.x+1,pt.y+sizetile_y);
	}
	else
	{
		StretchDIBits(pDC->m_hDC,pt.x,pt.y,
			sizetile_x,sizetile_y,0,0,
			bmpinf->bmiHeader.biWidth,
			-bmpinf->bmiHeader.biHeight,
			&*bits->begin(),bmpinf,DIB_RGB_COLORS,SRCCOPY);
	}
			
	char temp[100];
	char Name[256]; Name[0] = 0;
	if (InfoTexte==2)
	{
		_splitpath(pth.c_str(),temp,temp,Name,temp);		
	}
	else if (InfoTexte==3)
	{
		__int64 mask = 1;
	}
	else if (InfoTexte==1)
	{
		sprintf(Name,"%d",i->id);
	}
	rect_txt.top = pt.y + sizetile_y + spacing_tile_text;
	rect_txt.bottom += rect_txt.top + sizetext_y;
	rect_txt.left -= spacing_x;
	pDC->DrawText(Name,strlen(Name),&rect_txt,DT_CENTER | DT_SINGLELINE);

	// Restore the device context
	pDC->SetBkColor( clrBk );
	pDC->SetTextColor( clrText );

	if (i->Selected&3)
	{
		CRect rc;
		rc.left = pt.x; rc.top = pt.y; rc.right = rc.left + sizetile_x; rc.bottom = rc.top + sizetile_y;
		ShadeRect(pDC,rc);
	}

	// Invalidate flag button
	Browse *parent = (Browse*)this->GetParent();
	parent->UpdateFlags ();

	// Release the font
	pDC->SelectObject(pOldFont);
}

void CTView::ShadeRect( CDC *pDC, CRect& rect )

{
     // Bit pattern for a monochrome brush with every
     // other pixel turned off
     WORD Bits[8] = { 0x0055, 0x00aa, 0x0055, 0x00aa,
                      0x0055, 0x00aa, 0x0055, 0x00aa };

	CBitmap bmBrush;
	CBrush brush; // (GetSysColor(COLOR_3DFACE));

	// Need a monochrome pattern bitmap
	bmBrush.CreateBitmap( 8, 8, 1, 1, &Bits );

	// Create the pattern brush
	brush.CreatePatternBrush( &bmBrush );

	CBrush *pOldBrush = pDC->SelectObject( &brush );

	// Turn every other pixel to black
	COLORREF clrBk = pDC->SetBkColor( GetSysColor(COLOR_3DFACE) );
	COLORREF clrText = pDC->SetTextColor( RGB(0,0,0) );
	// 0x00A000C9 is the ROP code to AND the brush with the destination
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
		 (DWORD)0x00A000C9);                     //DPa - raster code

	pDC->SetBkColor( clrBk );
	pDC->SetTextColor( clrText );
	clrBk = pDC->SetBkColor( RGB(0,0,0) );
	clrText = pDC->SetTextColor( GetSysColor(COLOR_HIGHLIGHT) );
	// 0x00FA0089 is the ROP code to OR the brush with the destination
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
		 (DWORD)0x00FA0089);                     //DPo - raster code

	// Restore the device context
	pDC->SelectObject( pOldBrush );
	pDC->SetBkColor( clrBk );
	pDC->SetTextColor( clrText );
}


//code modifie des sources des MFC =)))
void CTView::DrawDragRect(CDC *pDC,LPCRECT lpRect, SIZE size,
	LPCRECT lpRectLast, SIZE sizeLast, CBrush* pBrush, CBrush* pBrushLast)
{
	// first, determine the update region and select it
	CRgn rgnNew;
	CRgn rgnOutside, rgnInside;
	CRect rect;
	if (lpRect)
	{
		rgnOutside.CreateRectRgnIndirect(lpRect);
		rect = *lpRect;
		rect.InflateRect(-size.cx, -size.cy);
		rect.IntersectRect(rect, lpRect);
		rgnInside.CreateRectRgnIndirect(rect);
		rgnNew.CreateRectRgn(0, 0, 0, 0);
		rgnNew.CombineRgn(&rgnOutside, &rgnInside, RGN_XOR);
	}

	CBrush* pBrushOld = NULL;
	if (pBrush == NULL)
		pBrush = CDC::GetHalftoneBrush();
	if (pBrushLast == NULL)
		pBrushLast = pBrush;

	CRgn rgnLast, rgnUpdate;
	if (lpRectLast != NULL/* && lpRect!=NULL*/)
	{
		// find difference between new region and old region
		rgnLast.CreateRectRgn(0, 0, 0, 0);
		if (lpRect==NULL) rgnOutside.CreateRectRgnIndirect(lpRectLast);
		else rgnOutside.SetRectRgn(lpRectLast);
		rect = *lpRectLast;
		rect.InflateRect(-sizeLast.cx, -sizeLast.cy);
		rect.IntersectRect(rect, lpRectLast);
		if (lpRect==NULL) rgnInside.CreateRectRgnIndirect(rect);
		else rgnInside.SetRectRgn(rect);
		rgnLast.CombineRgn(&rgnOutside, &rgnInside, RGN_XOR);

		// only diff them if brushes are the same
		if (pBrush->m_hObject == pBrushLast->m_hObject)
		{
			rgnUpdate.CreateRectRgn(0, 0, 0, 0);
			rgnUpdate.CombineRgn(&rgnLast, &rgnNew, RGN_XOR);
		}
	}
	if (!lpRect || (pBrush->m_hObject != pBrushLast->m_hObject && lpRectLast != NULL))
	{
		// brushes are different -- erase old region first
		pDC->SelectClipRgn(&rgnLast);
		pDC->GetClipBox(&rect);
		pBrushOld = pDC->SelectObject(pBrushLast);
		pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
		pDC->SelectObject(pBrushOld);
	}
	// draw into the update/new region
	if (lpRect)
	{
		pDC->SelectClipRgn(rgnUpdate.m_hObject != NULL ? &rgnUpdate : &rgnNew);
		pDC->GetClipBox(&rect);
		pBrushOld = pDC->SelectObject(pBrush);
		pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
		pDC->SelectObject(pBrushOld);
	}
	pDC->SelectClipRgn(NULL);
}

void CTView::RemoveSelection(int n)
{
	for (tilelist::iterator p = InfoList.theList[n].begin();p!=InfoList.theList[n].end();++p)
	{
		p->Selected = 0;
	}
}

void CTView::InsertItemInCtrlList(tilelist::iterator iFirst,tilelist::iterator iLast)
{
/*	int iItem = InfoList.GetSize();
	for (tilelist::iterator i=iFirst;i!=iLast;++i)
	{
		char num[10];
		sprintf(num,"%d",iItem);
		InsertItem(iItem,num,0);
		SetItemData(iItem++,(DWORD)(*i));
	}*/
}



LRESULT CTView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message==WM_MOUSEMOVE)
	{
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
	}
	if (message==WM_DRAWITEM)
	{
		int toto=0;
	}
	if (message==WM_CLOSE) Delete();
	if (message==WM_COMMAND) //The user had selected an item on popup menu
	{
		Browse *parent = (Browse*)this->GetParent();
		int id=LOWORD(wParam);
		if (id==ID_MENU_SUPR_TILE)
		{
			for (int i=0; i<InfoList.GetSize(parent->m_128x128); i++)
			{
				if (InfoList.Get(i, parent->m_128x128)->Selected)
				{
					switch (parent->m_128x128)
					{
					case 0:
						InfoList.removeTile128 (i);
						break;
					case 1:
						InfoList.removeTile256 (i);
						break;
					default:
						nlassert (0); // no!
					}
				}
			}
			bPopup = 0;
		}
		else if (id==ID_MENU_SUPR_BITMAP)
		{
			for (int i=0; i<InfoList.GetSize(parent->m_128x128); i++)
			{
				if (InfoList.Get(i, parent->m_128x128)->Selected)
				{
					switch (parent->m_128x128)
					{
					case 0:
						InfoList.clearTile128 (i, (CTile::TBitmap)(Texture-1));
						break;
					case 1:
						InfoList.clearTile256 (i, (CTile::TBitmap)(Texture-1));
						break;
					case 2:
						InfoList.clearTransition (i, (CTile::TBitmap)(Texture-1));
						break;
					case 3:
						InfoList.clearDisplacement (i);
						break;
					default:
						nlassert (0); // no!
					}
				}
			}
			bPopup = 0;
		}
		else if ((id==ID_MENU_ADD)||(id==ID_MENU_REPLACE))
		{
			_chdir (LastPath.c_str());
			CFileDialog load(true, NULL, LastPath.c_str(), OFN_ENABLESIZING | OFN_ALLOWMULTISELECT,
				"Targa bitmap (*.tga)|*.tga|All files (*.*)|*.*||",NULL);
			load.m_ofn.lpstrFile = new char[10000]; //le buffer contient la list de tous les noms de fichier
			load.m_ofn.lpstrFile[0] = 0;
			//avec 10ko on devrait tranquille ... si l'ensemble des noms des fichiers depassent 10000 cara, l'insertion n'a pas lieu
			load.m_ofn.nMaxFile = 10000-1;
			if (load.DoModal()==IDOK)
			{
				// Ok ?
				int ok=1;

				//AfxMessageBox ("toto");
				POSITION p = load.GetStartPosition(); //la doc dit que p=NULL quand il n'y a pas de selection : c'est faux, genial les MFC
				while (p)
				{
					CString str = load.GetNextPathName(p);

					char sDrive[256];
					char sPath[256];
					_splitpath (str, sDrive, sPath, NULL, NULL);
					LastPath=string (sDrive)+string (sPath);

					if (str!=CString(""))
					{
						int index=0;
						const char *pathname = (LPCTSTR)str;

						// Add mode, to the end of the list
						if (id==ID_MENU_ADD)
						{
							// Index of the new tile
							int index;

							// Index in the list
							switch (parent->m_128x128)
							{
							case 0:
								// Add a 128 tile
								index=InfoList.addTile128 ();

								// Set the tile
								if ((ok=InfoList.setTile128 (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha)))==0)
									// If prb, remove it
									InfoList.removeTile128 (index);

								break;
							case 1:
								// Add a 128 tile
								index=InfoList.addTile256 ();

								// Set the tile
								if ((ok=InfoList.setTile256 (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha)))==0)
									// If prb, remove it
									InfoList.removeTile256 (index);

								break;
							case 2:
								nlassert (0);		// no, can't add transition
								break;
							}
						}
						else
						{
							// Must be a replace mode.
							nlassert (id==ID_MENU_REPLACE);

							// For each tile
							for (int index=0; index<InfoList.GetSize(parent->m_128x128); index++)
							{
								// If selected
								if (InfoList.Get(index, parent->m_128x128)->Selected)
								{
									switch (parent->m_128x128)
									{
									case 0:
										// Set the 128 tile
										ok=InfoList.setTile128 (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha));
										break;
									case 1:
										// Set the 256 tile
										ok=InfoList.setTile256 (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha));
										break;
									case 2:
										// Alpha texture ?
										if (Texture!=3)
										{
											ok=InfoList.setTileTransition (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::alpha));
										}
										// Alpha!
										else
										{
											// Select rotation
											SelectRotation selectRotation;
											if (selectRotation.DoModal()==IDOK)
											{
												// Set the alpha tile with the good rotation
												ok=InfoList.setTileTransitionAlpha (index, pathname, selectRotation.RotSelected);
											}
										}
										break;
									case 3:
										// Displacement
										ok=InfoList.setDisplacement (index, pathname);
										break;
									default:
										nlassert (0); // no!
									}
								}

								// Reload last inserted tile
								InfoList.Reload(index, 1, parent->m_128x128);
								InfoList.Get(index, parent->m_128x128)->Selected = 0;

								// Stop ?
								if (!ok)
									break;
							}
						}
					}
					// Stop ?
					if (!ok)
						break;
				}
			}
			delete load.m_ofn.lpstrFile;
			//InfoList.Sort();
			bPopup = 0;
		}
		else if (id==12)
		{
			ViewTileMode = 1;
			Browse *parent = (Browse*) this->GetParent();
			parent->SendMessage(WM_PAINT,0,0);
			bPopup = 0;
		}
		this->RedrawWindow();
	}
	return CStatic::WindowProc(message, wParam, lParam);
}
	

void CTView::OnRButtonDown(UINT nFlags, CPoint point) 
{	
	// TODO: Add your message handler code here and/or call default
	if (!lockInsertion)
	{
		Browse *parent = (Browse*)this->GetParent();
		RECT wndpos; CMenu popup;
		GetParent()->GetWindowRect(&wndpos);
		popup.CreatePopupMenu();

		int c = 0;
		for (tilelist::iterator p = InfoList.GetFirst(parent->m_128x128);p!=InfoList.GetLast(parent->m_128x128);++p)
		{
			if (p->Selected) c++;
		}		
				
		if (!ViewTileMode)
		{
			popup.AppendMenu(parent->m_128x128<2 ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_ADD,"Add...");
			popup.AppendMenu(c>0 ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_REPLACE, "Replace...");
			popup.AppendMenu(c>0 ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_SUPR_BITMAP, "Del bitmap");
			popup.AppendMenu((c>0 && parent->m_128x128<2) ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_SUPR_TILE, "Del tile");
		}
		else
		{
		}
		bPopup = 1;
		popup.TrackPopupMenu(TPM_LEFTALIGN,MousePos.x+wndpos.left,MousePos.y+wndpos.top,GetParent(),NULL);
	}
	CStatic::OnRButtonDown(nFlags, point);
}

void CTView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SendMessage(WM_PAINT,0,0);
	CStatic::OnLButtonDown(nFlags, point);
}


void CTView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
}

