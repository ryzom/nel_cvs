// View.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "View.h"
#include "SelectionTerritoire.h"
#include "Browse.h"
#include "popup.h"
#include <direct.h>
//#include "ListGroup.h"
#include <nel/3d/tile_bank.h>
//#include "ViewPopup.h"
#include "pic\readpic.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

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

/////////////////////////////////////////////////////////////////////////////
// CTView
//Attention : windows veut que le buffer image commence du bas vers le haut
int _LoadBitmap(const std::string& path,LPBITMAPINFO BitmapInfo,std::vector<NLMISC::CBGRA>&Tampon, bool bMulAlpha, bool bInvertAlpha)//charge une image (bmp pour le moment, tga,png,jpg plus tard ?)
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

		if (bMulAlpha)
		{
			// Pre mul RGB componates by Alpha one
			int nPixelCount=(int)(Width*Height);
			for (int p=0; p<nPixelCount; p++)
			{
				// Invert alpha ?
				int alpha=bInvertAlpha?255-Tampon[p].A:Tampon[p].A;
				Tampon[p].R=(uint8)(((int)Tampon[p].R*alpha)>>8);
				Tampon[p].G=(uint8)(((int)Tampon[p].G*alpha)>>8);
				Tampon[p].B=(uint8)(((int)Tampon[p].B*alpha)>>8);
			}
		}

		return 1;
	}
	else
		return 0;

	/*int pathsize = strlen(path);
	if (pathsize<=3) return 0; //le fichier ne contient pas d'extension
	FILE *ptr = fopen(path,"rb");
	if (!ptr) return 0;
	if (!strcmp(path+pathsize-3,"bmp")) //bmp ...
	{
		BITMAPFILEHEADER header; 	//TODO : gerer les cas d'erreur
		fread(&header,1,sizeof(BITMAPFILEHEADER),ptr);
		fread(BitmapInfo,1,sizeof(BITMAPINFOHEADER),ptr);
		if (BitmapInfo->bmiHeader.biBitCount!=TILE_BPP) return 0; //on ne lit que les 24 bits
		int size=(BitmapInfo->bmiHeader.biHeight*BitmapInfo->bmiHeader.biHeight*BitmapInfo->bmiHeader.biBitCount)>>3;
		BitmapInfo->bmiHeader.biCompression=BI_RGB;
		BitmapInfo->bmiHeader.biSizeImage=size;
		BitmapInfo->bmiHeader.biXPelsPerMeter=1;
		BitmapInfo->bmiHeader.biYPelsPerMeter=1;
		BitmapInfo->bmiHeader.biClrImportant=0;
		BitmapInfo->bmiHeader.biClrUsed=0;
		if (*Bits) delete *Bits;
		*Bits=(void*) new char[size];
		fread(*Bits,1,size,ptr);
	}
	else 
	{
		fclose(ptr);
		return 0;
	}
	fclose(ptr);
	return 1;*/
}






//TileInfo
TileInfo::TileInfo()
{
	nightLoaded = 0;
	bumpLoaded = 0;
	loaded = 0;
	//=h=g=b=d=0; 
	//Bits = NULL; 
	//number = 0;
	Selected = 0; 
	//path = 0; 
	//groupFlag = 0; 
	//bumpPath = 0; 
	//nightPath = 0;
	//bumpBits  = 0;
	//nightBits = 0;
}

/*TileInfo::TileInfo(const TileInfo &a)
{
	*this = a;
	if (a.Bits)
	{
		Bits = (char*)new char[a.BmpInfo.bmiHeader.biSizeImage];
		memcpy(Bits,a.Bits,a.BmpInfo.bmiHeader.biSizeImage);
	}
	if (a.bumpBits) 
	{
		bumpBits = (char*)new char[a.BmpInfo.bmiHeader.biSizeImage];
		memcpy(bumpBits,a.bumpBits,a.BmpInfo.bmiHeader.biSizeImage);
	}
	if (a.nightBits) 
	{
		nightBits = (char*)new char[a.BmpInfo.bmiHeader.biSizeImage];
		memcpy(nightBits,a.nightBits,a.BmpInfo.bmiHeader.biSizeImage);
	}
	if (a.path) 
	{
		path = new char[strlen(a.path)+1];
		strcpy(path,a.path);
	}
	if (a.bumpPath)
	{
		bumpPath = new char[strlen(a.bumpPath)+1];
		strcpy(bumpPath,a.bumpPath);
	}
	if (a.nightPath)
	{
		nightPath = new char[strlen(a.nightPath)+1];
		strcpy(nightPath,a.nightPath);
	}
}*/

/*TileInfo::~TileInfo()
{
	if (Bits) 
	{
		delete Bits;
	}
	if (nightBits) 
	{
		delete nightBits;
	}
	if (bumpBits) 
	{
		delete bumpBits;
	}
	if (path) 
	{
		delete path;
	}
	if (nightPath) 
	{
		delete nightPath;
	}
	if (bumpPath) 
	{
		delete bumpPath;
	}
}*/

/*int TileInfo::operator < (const TileInfo &a) const
{
	switch(SortTile)
	{
	case 0:
		return (id<a.id);
	case 1:
		return (groupFlag<a.groupFlag);
	}
	return 1;
}*/






//Edge
/*_Edge::_Edge()
{
	line = 0;
}

_Edge::~_Edge()
{
	if (line) delete line;
}

_Edge::_Edge(const _Edge& edge)
{
	line = new char[edge.size*3];
	memcpy(line,edge.line,edge.size*3);
	size = edge.size;
}

int _Edge::operator == (const _Edge & ed) const
{
	if (ed.size!=size) return 0;
	for (int i=0;i<size*3;i++)
	{
		if (ed.line[i]!=line[i]) return 0;
	}
	return 1;
}

void _Edge::CreateH(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biWidth-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3+(tile->BmpInfo.bmiHeader.biWidth)*(tile->BmpInfo.bmiHeader.biHeight-1)*3];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3+1+(tile->BmpInfo.bmiHeader.biWidth)*(tile->BmpInfo.bmiHeader.biHeight-1)*3];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3+2+(tile->BmpInfo.bmiHeader.biWidth)*(tile->BmpInfo.bmiHeader.biHeight-1)*3];
	}
}

void _Edge::CreateB(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biWidth-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3+1];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3+2];
	}
}

void _Edge::CreateG(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biHeight-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+1];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+2];
	}
}

void _Edge::CreateD(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biHeight-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+(tile->BmpInfo.bmiHeader.biWidth-1)*3];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+1+(tile->BmpInfo.bmiHeader.biWidth-1)*3];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+2+(tile->BmpInfo.bmiHeader.biWidth-1)*3];
	}
}*/








//TileList
TileList::TileList()
{
	last_id = 0; 
	smAcces = 0;
	_tileSet = -1;

	for (int i=0; i<CTileSet::count; i++)
	{
		TileInfo info;
		info.id = i;
		/*info.path=NULL;
		info.nightPath=NULL;
		info.bumpPath=NULL;*/
	
		theListTransition.push_back (info);
	}

	//oldFlag = -1;
}

/*int TileList::Add(const char *path,const char *pathNight,const char *pathBump, int hh, int bb, int gg, int dd, unsigned int flags)
{
	while (smAcces) {}
	smAcces = 1;
	TileInfo info;
	info.id = theList.size();
	info.number = theList.size();
	info.groupFlag = flags; //flagGroupSort;
	info.path=NULL;
	info.nightPath=NULL;
	info.bumpPath=NULL;
	info.h=hh;
	info.b=bb;
	info.g=gg;
	info.d=dd;
	if (path)
	{
		info.path = new char[strlen(path)+1];
		strcpy( info.path, path);
	}
	if (pathNight)
	{
		info.nightPath = new char[strlen(pathNight)+1];
		strcpy( info.nightPath, pathNight);
	}
	if (pathBump)
	{
		info.bumpPath = new char[strlen(pathBump)+1];
		strcpy( info.bumpPath, pathBump);
	}
	theList.insert(theList.end(),info);
	//if (i==NULL) i=theList.begin();
	//else i++;
	smAcces = 0;
	return 1;
}*/

int TileList::addTile128 ()
{
	while (smAcces) {}
	smAcces = 1;

	int index;
	tileBank2.getTileSet (_tileSet)->addTile128 (index, tileBank2);
	nlassert (index==(sint)theList128.size());

	TileInfo info;
	info.id = index;
	/*info.path=NULL;
	info.nightPath=NULL;
	info.bumpPath=NULL;
	info.path=NULL;*/
	theList128.push_back (info);

	smAcces = 0;
	return index;
}

int TileList::addTile256 ()
{
	while (smAcces) {} 
	smAcces = 1;
	
	int index;
	tileBank2.getTileSet (_tileSet)->addTile256 (index, tileBank2);
	nlassert (index==(sint)theList256.size());

	TileInfo info;
	info.id = index;
	/*info.path=NULL;
	info.nightPath=NULL;
	info.bumpPath=NULL;
	info.path=NULL;*/
	theList256.push_back (info);

	smAcces = 0;
	return index;
}

int TileList::setTile128 (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	while (smAcces) {}
	smAcces = 1;

	vector<NLMISC::CBGRA> tampon;
	uint Width;
	uint Height;
	if (!PIC_LoadPic(name, tampon, Width, Height))
	{
		smAcces = 0;
		MessageBox (NULL, name.c_str(), "Can't load bitmap.", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
	else
	{
		CTileBorder border;
		border.set (Width, Height, tampon);

		CTileSet::TError error;
		int pixel=-1;
		int composante=4;
		error=tileBank2.getTileSet(_tileSet)->checkTile128 (type, border, pixel, composante);
		if ((error!=CTileSet::ok)&&(error!=CTileSet::addFirstA128128))
		{
			smAcces = 0;
			char sTmp[512];
			static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
			sprintf (sTmp, "%s\nPixel: %d (%s)", CTileSet::getErrorMessage (error), pixel, comp[composante]);
			MessageBox (NULL, sTmp, "Can't add tile", MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}
		else
		{
			if (error==CTileSet::addFirstA128128)
				tileBank2.getTileSet(_tileSet)->setBorder (type, border);
			tileBank2.getTileSet(_tileSet)->setTile128 (tile, name, type, tileBank2);
			switch (type)
			{
			case CTile::diffuse:
				theList128[tile].loaded=0;
				break;
			case CTile::additive:
				theList128[tile].nightLoaded=0;
				break;
			case CTile::bump:
				theList128[tile].bumpLoaded=0;
				break;
			}
			theList128[tile].Load (tileBank2.getTileSet(_tileSet)->getTile128(tile), false, false);
		}
	}

	smAcces = 0;
	return 1;
}

int TileList::setTile256 (int tile, const std::string& name, NL3D::CTile::TBitmap type)
{
	while (smAcces) {}
	smAcces = 1;
	vector<NLMISC::CBGRA> tampon;
	uint Width;
	uint Height;
	if (!PIC_LoadPic(name, tampon, Width, Height))
	{
		smAcces = 0;
		MessageBox (NULL, name.c_str(), "Can't load bitmap.", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
	else
	{
		CTileBorder border;
		border.set (Width, Height, tampon);
		
		CTileSet::TError error;
		int pixel=-1;
		int composante=4;
		if ((error=tileBank2.getTileSet(_tileSet)->checkTile256 (type, border, pixel, composante))!=CTileSet::ok)
		{
			smAcces = 0;
			char sTmp[512];
			static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
			sprintf (sTmp, "%s\nPixel: %d (%s)", CTileSet::getErrorMessage (error), pixel, comp[composante]);
			MessageBox (NULL, sTmp, "Can't add tile", MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}
		else
		{
			tileBank2.getTileSet(_tileSet)->setTile256 (tile, name, type, tileBank2);
					switch (type)
			{
			case CTile::diffuse:
				theList256[tile].loaded=0;
				break;
			case CTile::additive:
				theList256[tile].nightLoaded=0;
				break;
			case CTile::bump:
				theList256[tile].bumpLoaded=0;
				break;
			}
			theList256[tile].Load (tileBank2.getTileSet(_tileSet)->getTile256(tile), false, false);
		}
	}

	smAcces = 0;
	return 1;
}

int TileList::setTileTransition (int tile, const std::string& name, NL3D::CTile::TBitmap type, bool bInvert)
{
	while (smAcces) {}
	smAcces = 1;

	vector<NLMISC::CBGRA> tampon;
	uint Width;
	uint Height;
	if (!PIC_LoadPic(name, tampon, Width, Height))
	{
		smAcces = 0;
		MessageBox (NULL, name.c_str(), "Can't load bitmap.", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
	else
	{
		CTileBorder border;
		border.set (Width, Height, tampon);
		
		CTileSet::TError error;
		int indexError;
		int pixel=-1;
		int composante=4;
		if ((error=tileBank2.getTileSet(_tileSet)->checkTileTransition ((CTileSet::TTransition)tile, type, border, indexError,
			pixel, composante, bInvert))!=CTileSet::ok)
		{
			smAcces = 0;
			char sMsg[512];
			if ((error==CTileSet::topInterfaceProblem)||(error==CTileSet::bottomInterfaceProblem)||(error==CTileSet::leftInterfaceProblem)
				||(error==CTileSet::rightInterfaceProblem)||(error==CTileSet::topBottomNotTheSame)||(error==CTileSet::rightLeftNotTheSame)
				||(error==CTileSet::topInterfaceProblem))
			{
				static const char* comp[]={"Red", "Green", "Blue", "Alpha", ""};
				if (indexError!=-1)
					sprintf (sMsg, "%s\nIncompatible with tile n°%d\nPixel: %d (%s)", CTileSet::getErrorMessage (error), indexError,
						pixel, comp[composante]);
				else
					sprintf (sMsg, "%s\nIncompatible with the 128x128 tile\nPixel: %d (%s)", CTileSet::getErrorMessage (error),
						pixel, comp[composante]);
				}
			else
				sprintf (sMsg, "%s\nIncompatible filled tile", CTileSet::getErrorMessage (error));
			if (bInvert)
			{
				char addInvert[256];
				sprintf (addInvert, "\n\nWarning: This tile is added with inverted alpha!");
				strcat (sMsg, addInvert);
			}
			MessageBox (NULL, sMsg, "Can't add tile", MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}
		else
		{
			tileBank2.getTileSet(_tileSet)->setTileTransition ((CTileSet::TTransition)tile, name, type, tileBank2, border, bInvert);
			switch (type)
			{
			case CTile::diffuse:
				theListTransition[tile].loaded=0;
				break;
			case CTile::additive:
				theListTransition[tile].nightLoaded=0;
				break;
			case CTile::bump:
				theListTransition[tile].bumpLoaded=0;
				break;
			}
			theListTransition[tile].Load (tileBank2.getTileSet(_tileSet)->getTransition(tile)->getTile(), true, bInvert);
		}
	}

	smAcces = 0;
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
	case CTile::bump:
		theList128[index].bumpLoaded=0;
		theList128[index].bumpBits.resize(0);
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
	case CTile::bump:
		theList256[index].bumpLoaded=0;
		theList256[index].bumpBits.resize(0);
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
	case CTile::bump:
		theListTransition[index].bumpLoaded=0;
		theListTransition[index].bumpBits.resize(0);
		break;
	}
	tileBank2.getTileSet (_tileSet)->clearTransition ((CTileSet::TTransition)index, bitmap, tileBank2);
}

/*void TileList::UpdateLF(void)
{
	if (oldFlag!=flagGroupSort || oldsize!=(int)theList.size() || oldShowNULL!=showNULL || oldSortMode!=sortMode) 
	{
		oldFlag = flagGroupSort;
		oldSortMode = sortMode;
		oldsize = theList.size();
		oldShowNULL = showNULL;		
		if (showNULL)
		{
		}
		else
		{
			if (!oldFlag)
			{
			}
			else if (sortMode) //and
			{
			}
			else //or
			{
			}			
		}
	}
}*/

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

/*void TileList::Sort(void)
{
	if (!sortMode && !showNULL)
	{
		int temp = SortTile,bDec=1;
		SortTile = 0;
		theList.sort();
		SortTile = temp;
		tilelist::iterator p = theList.end();
		size = 0;
		p--;
		for (int i = 0;i<(int)theList.size();i++)
		{
			if (p->groupFlag&flagGroupSort)
			{
				tilelist::iterator next = p;
				next--;
				theList.splice(theList.begin(),theList,p);
				p = next;
				size++;
				bDec=0;
			}
			else 
			{
				p--;
			}
		}
		for (p = theList.begin();p!=theList.end();++p)
		{
			int k = p->number;
		}
	}
	else 
	{
		theList.sort();
	}
	int k = 0;
	for (tilelist::iterator p = GetFirst();p!=GetLast();++p) p->number = k++;
}*/

void TileInfo::Delete ()
{
	loaded=0;
	nightLoaded=0;
	bumpLoaded=0;
}


tilelist::iterator TileList::Get(int i, int n)
{
	return theList[n].begin()+i;
	/*tilelist::iterator p = GetFirst();
	for (int k = 0; k<i ;k++)
	{
		if (p==GetLast()) 
			return p;
		p++;
	}
	return p;*/
}

/*void TileList::DeleteAll()
{
	int size = GetSize();
	if (size==0) return;
	Delete(GetFirst()++,size);
}*/

const std::string& TileInfo::getFileName (CTile::TBitmap type, int index)
{
	return tileBank2.getTile (index)->getFileName (type);
}

bool TileInfo::Load (int index, bool bMulAlpha, bool bInvertAlpha)
{
	bool bRes=true;
	if (!loaded && getFileName (CTile::diffuse, index)!="")
	{
		if (!_LoadBitmap(getFileName (CTile::diffuse, index), &BmpInfo, Bits, bMulAlpha, bInvertAlpha))
		{
			bRes=false;
			MessageBox (NULL, getFileName (CTile::diffuse, index).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
		}
		loaded=1;
	}
	if (!nightLoaded && getFileName (CTile::additive, index)!="")
	{
		if (!_LoadBitmap(getFileName (CTile::additive, index), &nightBmpInfo, nightBits, bMulAlpha, false))
		{
			bRes=false;
			MessageBox (NULL, getFileName (CTile::additive, index).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
		}
		nightLoaded=1;
	}
	if (!bumpLoaded && getFileName (CTile::bump, index)!="")
	{
		if (!_LoadBitmap(getFileName (CTile::bump, index), &bumpBmpInfo, bumpBits, bMulAlpha, false))
		{
			bRes=false;
			MessageBox (NULL, getFileName (CTile::bump, index).c_str(), "Can't load file", MB_OK|MB_ICONEXCLAMATION);
		}
		bumpLoaded=1;
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
			theList[n][i].Load (tileBank2.getTileSet(_tileSet)->getTile128 (i), false, false);
			break;
		case 1:
			theList[n][i].Load (tileBank2.getTileSet(_tileSet)->getTile256 (i), false, false);
			break;
		case 2:
			{
				int index=tileBank2.getTileSet(_tileSet)->getTransition (i)->getTile();
				if (index!=-1)
				{
					theList[n][i].Load (index, true, tileBank2.getTile (index)->isInvert());
				}
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
	char name[256];
	char *defautpath = ((SelectionTerritoire*)GetParent()->GetParent())->DefautPath.GetBuffer(256);
	sprintf(name,"%s%s",defautpath,"croix.bmp");
	if (_LoadBitmap(std::string((const char*)name),&TileCroix.BmpInfo,TileCroix.Bits, false, false))
	{
		int size=TileCroix.BmpInfo.bmiHeader.biHeight*TileCroix.BmpInfo.bmiHeader.biWidth*TileCroix.BmpInfo.bmiHeader.biBitCount/8;
		char *temp = new char[size];
	}
	count_=1;
}

void CTView::Delete()
{
	count_=0; pImList = 0;
}

int CTView::GetNbTileLine(void)
{
	RECT rect; GetClientRect(&rect);
	return (max (1, (rect.right - rect.left - spacing_x)/(sizeicon_x + spacing_x)));
}

int CTView::GetNbTileColumn(void)
{
	RECT rect; GetClientRect(&rect);
	int deb;
	if (scrollpos<spacing_y) deb = -scrollpos;
	else deb = (scrollpos - spacing_y)%(sizeicon_y + spacing_y);
	if (deb>sizeicon_y) deb -=sizeicon_y + spacing_y;
	int ret= ((rect.bottom - rect.top /*- spacing_y*/ + deb)/(sizeicon_y + spacing_y)) +1 ;
//	if (((rect.bottom - rect.top - spacing_y + deb)%(sizeicon_y + spacing_y))) ret++;
//	if (deb>0) ret++;
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
	CDC *pDC = NULL;
	if (index==-1 && !(mode&MK_SHIFT))
	{
		for (int i = 0;i<InfoList.GetSize(n); i++)
		{
			if (p->Selected==1)
			{
				if (pDC==NULL) pDC = GetDC();
				p->Selected = 0;
				DrawTile(p,pDC,0,n);
			}
			p++;
		}
		if (pDC) ::ReleaseDC(*this,*pDC);
		return;
	}
	for (int i = 0;i<index;i++) 
	{
		if (p==InfoList.GetLast(n)) return;
		if (p->Selected==1) 
		{
			p->Selected = 0;
			if (pDC==NULL) pDC = GetDC();				
			DrawTile(p,pDC,0,n);
		}
		else if (p->Selected&4)
		{
			if (p->Selected) p->Selected=2;
			else p->Selected = 3;
			if (pDC==NULL) pDC = GetDC();				
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
				if (p==InfoList.GetLast(n)) return; 
				if (p->Selected==1)
				{
					p->Selected = 0; 
					if (pDC==NULL) pDC = GetDC(); 	
					DrawTile(p,pDC,0,n);
				}				
				else if (p->Selected&4)
				{
					if (p->Selected&3) p->Selected=2;
					else p->Selected = 3;
					int k = !(p->Selected&1);
					if (pDC==NULL) pDC = GetDC();				
					DrawTile(p,pDC,0,n);
				}
				p++;
			}
		}
		else k = incd;
		for (int i = rect->left;i<=rect->right;i+=sizeicon_x + spacing_x)
		{
			if (p==InfoList.GetLast(n)) return;
			if (!(mode&MK_CONTROL))
			{
				if (p->Selected!=1)
				{
					p->Selected = 1;
					if (pDC==NULL) pDC = GetDC();				
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
					if (pDC==NULL) pDC = GetDC();				
					DrawTile(p,pDC,0,n);
				}
			}
			p++;
			k++;
		}
		for (;k<nbline;k++) 
		{
			if (p==InfoList.GetLast(n)) return; 
			if (p->Selected==1) 
			{
				p->Selected = 0; 
				if (pDC==NULL) pDC = GetDC(); 
				DrawTile(p,pDC,0,n);
			} 
			else if (p->Selected&4)
			{
				if (p->Selected&3) p->Selected=2;
				else p->Selected = 3;
				if (pDC==NULL) pDC = GetDC();				
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
			if (pDC==NULL) pDC = GetDC();				
			DrawTile(p,pDC,0,n);
		}
		else if (p->Selected&4)
		{
			if (p->Selected&3) p->Selected=2;
			else p->Selected = 3;
			if (pDC==NULL) pDC = GetDC();				
			DrawTile(p,pDC,0,n);
		}
	}
	if (pDC!=NULL) ::ReleaseDC(*this,*pDC);
}
		
/*void CTView::CheckTile(TileInfo *theTile)
{
	_Edge h,b,g,d;
	if (!theTile->Bits) return;
	b.CreateB(theTile);
	h.CreateH(theTile);
	g.CreateG(theTile);
	d.CreateD(theTile);

	_Edge test = b;

	int found=0;
	int i=0;
	for (edgelist::iterator p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (b==*p)
		{
			found=1;
			theTile->b=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),b);
		theTile->b=i;
	}

	found=0;
	i=0;
	for (p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (h==*p)
		{
			found=1;
			theTile->h=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),h);
		theTile->h=i;
	}

	found=0;
	i=0;
	for (p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (g==*p)
		{
			found=1;
			theTile->g=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),g);
		theTile->g=i;
	}

	found=0;
	i=0;
	for (p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (d==*p)
		{
			found=1;
			theTile->d=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),d);
		theTile->d=i;
	}
}*/

/*tilelist::iterator CTView::GetTileSelection(tilelist::iterator i)
{
	tilelist::iterator ret;
	while (i!=InfoList.GetLast() && !i->Selected) 
		i++;
	return i;
}*/

void CTView::DeleteTile(tilelist::iterator p)
{
	p->loaded = 0;
	p->nightLoaded = 0;
	p->bumpLoaded = 0;
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
/*	POSITION pos = this->GetFirstSelectedItemPosition();
	for (int k=0;k<GetSelectedCount();k++)
	{
		if (GetNextSelectedItem(pos)==i) return 1;
	}*/
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
	CBrush brush;
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
		normal_font->CreateFont(0,0,0,0,FW_THIN,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,NULL);
	}
	
	tilelist::iterator p = InfoList.GetFirst(parent->m_128x128);
	for (int i=0;i<iFV;i++) p++;
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
					if (InfoList.setTile128 (index, FileName, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::bump)))
					{
						tilelist::iterator it = InfoList.GetLast(parent->m_128x128);
						it--;
						//InfoList.Reload(&*it,1);
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
					if (InfoList.setTile256 (index, FileName, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::bump)))
					{
						tilelist::iterator it = InfoList.GetLast(parent->m_128x128);
						it--;
						//InfoList.Reload(&*it,1);
					}
					else
						InfoList.removeTile256 (index);
				}
				break;
			case 2:
				{
					/*if (InfoList.setTileTransition (index, FileName, CTile::diffuse)
					{
						tilelist::iterator it = InfoList.GetLast(parent->m_128x128);
						it--;
						InfoList.Reload(it,1);
					}*/
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
	while (InfoList.smAcces) {}
	InfoList.smAcces = 1;
	RECT rect; GetClientRect(&rect);	
	CBrush brush;
	if (InfoList.GetSize(n)==0) 
	{
		InfoList.smAcces = 0; return;
	}
	
	LPBITMAPINFO bmpinf;
	std::string pth; 
	std::vector<NLMISC::CBGRA> *bits;
	int loaded;

	switch (n)
	{
	case 0:
		pth = i->getFileName ((CTile::TBitmap)(Texture-1), tileBank2.getTileSet (InfoList._tileSet)->getTile128 (i->id));
		break;
	case 1:
		pth = i->getFileName ((CTile::TBitmap)(Texture-1), tileBank2.getTileSet (InfoList._tileSet)->getTile256 (i->id));
		break;
	case 2:
		{
			int index=tileBank2.getTileSet (InfoList._tileSet)->getTransition (i->id)->getTile();
			if (index!=-1)
				pth = i->getFileName ((CTile::TBitmap)(Texture-1), index);
			else
				pth = "";
		}
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
			bmpinf = &(i->bumpBmpInfo);
			bits = &i->bumpBits;
			loaded = i->bumpLoaded;
			break;
	}

	CRgn clip;
	clip.CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	pDC->SelectClipRgn(&clip);
	
	pDC->SelectObject(normal_font);
	
	POINT pt;
	pt = GetPos(i->id);
	RECT rect_txt; 
	rect_txt.top = pt.y;
	rect_txt.bottom = pt.y + sizeicon_y + spacing_y;
	rect_txt.left = pt.x; rect_txt.right = pt.x + sizeicon_x + spacing_x;
	if (clear) pDC->FillRect(&rect_txt,&brush);
	if (!loaded)
	{
		pDC->FillSolidRect( pt.x, pt.y, sizetile_x, sizetile_y, 0xffffffff);
		pDC->MoveTo (pt.x,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y+sizetile_y);
		pDC->MoveTo (pt.x+sizetile_x,pt.y);
		pDC->LineTo (pt.x,pt.y+sizetile_y);
		pDC->MoveTo (pt.x,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y);
		pDC->LineTo (pt.x+sizetile_x,pt.y+sizetile_y);
		pDC->LineTo (pt.x,pt.y+sizetile_y);
		pDC->LineTo (pt.x,pt.y);
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
		/*CComboBox *clist = (CComboBox*)GetParent()->GetDlgItem (IDC_LISTTYPE);
		for (int c = 0;c<64;c++)
		{
			if (i->groupFlag&mask)
			{
				CString str;
				ASSERT(c<clist->GetCount());
				
				clist->GetLBText(c+2,str);
				strcat(Name,(LPCSTR)str);
				strcat(Name," ");
			}
			mask<<=1;
		}*/
	}
	else if (InfoTexte==1)
	{
		sprintf(Name,"%d",i->id);
	}
	rect_txt.top = pt.y + sizetile_y + spacing_tile_text;
	rect_txt.bottom += rect_txt.top + sizetext_y;
	rect_txt.left -= spacing_x;
	pDC->DrawText(Name,strlen(Name),&rect_txt,DT_CENTER | DT_SINGLELINE);

	if (i->Selected&3)
	{
		CRect rc;
		rc.left = pt.x; rc.top = pt.y; rc.right = rc.left + sizetile_x; rc.bottom = rc.top + sizetile_y;
		ShadeRect(pDC,rc);
	}
	InfoList.smAcces = 0;
}

void CTView::ShadeRect( CDC *pDC, CRect& rect )

{
     // Bit pattern for a monochrome brush with every
     // other pixel turned off
     WORD Bits[8] = { 0x0055, 0x00aa, 0x0055, 0x00aa,
                      0x0055, 0x00aa, 0x0055, 0x00aa };

     CBitmap bmBrush;
     CBrush brush;

     // Need a monochrome pattern bitmap
     bmBrush.CreateBitmap( 8, 8, 1, 1, &Bits );

     // Create the pattern brush
     brush.CreatePatternBrush( &bmBrush );

     CBrush *pOldBrush = pDC->SelectObject( &brush );

     // Turn every other pixel to black
     COLORREF clrBk = pDC->SetBkColor( RGB(255,255,255) );
     COLORREF clrText = pDC->SetTextColor( RGB(0,0,0) );
     // 0x00A000C9 is the ROP code to AND the brush with the destination
     pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
             (DWORD)0x00A000C9);                     //DPa - raster code

     pDC->SetBkColor( RGB(0,0,0) );
     pDC->SetTextColor( GetSysColor(COLOR_HIGHLIGHT) );
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
		pBrushOld = NULL;
	}
	// draw into the update/new region
	if (lpRect)
	{
		pDC->SelectClipRgn(rgnUpdate.m_hObject != NULL ? &rgnUpdate : &rgnNew);
		pDC->GetClipBox(&rect);
		pBrushOld = pDC->SelectObject(pBrush);
		pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
	}
	// cleanup DC
	if (pBrushOld != NULL)
		pDC->SelectObject(pBrushOld);
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
				//AfxMessageBox ("toto");
				POSITION p = load.GetStartPosition(); //la doc dit que p=NULL quand il n'y a pas de selection : c'est faux, genial les MFC
				//tilelist::iterator pos = InfoList.GetFirst(parent->m_128x128);
				int index=0;
				/*CDC *pDC = GetDC();
				::ReleaseDC(*this,*pDC);*/
				int pushBack = 0;
				while (p)
				{
					CString str = load.GetNextPathName(p);

					char sDrive[256];
					char sPath[256];
					_splitpath (str, sDrive, sPath, NULL, NULL);
					LastPath=string (sDrive)+string (sPath);

					if (str!=CString(""))
					{
						const char *pathname = (LPCTSTR)str;
/*						InfoList.Add(pathname);
						tilelist::iterator it = InfoList.theList.end();
						it--;
						InfoList.Reload(GetDC(),it,1);
						CheckTile(&(*it));*/

						// Add at the end
						if (id==ID_MENU_ADD)
							index=InfoList.GetSize(parent->m_128x128);

						if (index!=InfoList.GetSize(parent->m_128x128))
						{
							while (!pushBack && !InfoList.Get(index, parent->m_128x128)->Selected) 
							{
								index++;
								if (index==InfoList.GetSize(parent->m_128x128))
									break;
							}
						}
//						TileInfo info;
						if ((index==InfoList.GetSize(parent->m_128x128))&&(id==ID_MENU_ADD))
						{
							pushBack = 1;
							switch (parent->m_128x128)
							{
							case 0:
								index=InfoList.addTile128 ();
								break;
							case 1:
								index=InfoList.addTile256 ();
								break;
							case 2:
								// pos=InfoList.Get (InfoList.addTileTransition (), 2);
								index=InfoList.GetSize(parent->m_128x128);
								break;
							}
						}
						if (index!=InfoList.GetSize(parent->m_128x128))
						{
							switch (parent->m_128x128)
							{
							case 0:								
								if (!InfoList.setTile128 (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::bump)))
								{
									if (id==ID_MENU_ADD)
									{
										InfoList.removeTile128 (index);
									}
								}
								break;
							case 1:
								if (!InfoList.setTile256 (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::bump)))
								{
									if (id==ID_MENU_ADD)
									{
										InfoList.removeTile256 (index);
									}
								}
								break;
							case 2:
								{
									nlassert (id!=ID_MENU_ADD);
									bool bInvert=false;
									if (MessageBox ("Do you want to add this transition tile with inverted alpha ?", "Tile edit", MB_YESNO|MB_ICONQUESTION)==IDYES)
										bInvert=true;
									InfoList.setTileTransition (index, pathname, Texture==1?CTile::diffuse:(Texture==2?CTile::additive:CTile::bump), bInvert);
								}
								break;
							default:
								nlassert (0); // no!
							}
							if (index!=InfoList.GetSize(parent->m_128x128))
							{
								InfoList.Reload(index, 1, parent->m_128x128);
								InfoList.Get(index, parent->m_128x128)->Selected = 0;
								index++;
							}
						}
					}
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
/*			POSITION sel = GetFirstSelectedItemPosition();
			int i = GetNextSelectedItem(sel);
			parent->TileSelected = (TileInfo*)GetItemData(i);*/
			parent->SendMessage(WM_PAINT,0,0);
			//UpdateBuffer();
			bPopup = 0;
		}
		/*else if (id==15)
		{
			ListGroup sel(this);
			sel.DoModal();
			if (sel.bSelection)
			{
				for (tilelist::iterator p = InfoList.GetFirst();p!=InfoList.GetLast();++p)
				{
					if (p->Selected) p->groupFlag = sel.iSelection;
				}
			}
			bPopup = 0;
			//InfoList.Sort();
		}*/
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
			popup.AppendMenu(parent->m_128x128!=2 ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_ADD,"Add...");
			popup.AppendMenu(c>0 ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_REPLACE, "Replace...");
			popup.AppendMenu(c>0 ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_SUPR_BITMAP, "Del bitmap");
			popup.AppendMenu((c>0 && parent->m_128x128!=2) ? MF_STRING : MF_STRING | MF_GRAYED, ID_MENU_SUPR_TILE, "Del tile");
			//popup.AppendMenu(c==1 ? MF_STRING : MF_STRING | MF_GRAYED,12,"Voir les tiles");
			//popup.AppendMenu(c>0 ? MF_STRING : MF_STRING | MF_GRAYED,15,"Group...");
		}
		else
		{
			//popup.AppendMenu(c==1 ? MF_STRING : MF_STRING | MF_GRAYED,12,"Voir les tiles");
			//popup.AppendMenu(MF_STRING,14,"Revenir au mode normal");
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
	/*if (nFlags&MK_CONTROL) return;
	RECT r; r.left = MousePos.x; r.top = MousePos.y;
	GetParent()->ClientToScreen(&r);
	POINT pt; pt.x = r.left; pt.y = r.top;
	ScreenToClient(&pt);
	int index = GetIndex(&pt);
	tilelist::iterator p = InfoList.GetFirst();
	if (index==-1 || InfoList.GetSize()<=index) return;
	for (int i = 0;i<index;i++) p++;
	if (index!=-1)
	{
		theViewPopup = new ViewPopup(&(*p),this);
		char wndName[256];
		if (p->path)
		{
			sprintf(wndName,"Tile %d %s",p->id,p->path);
		}
		else
		{
			sprintf(wndName,"Tile %d",p->id);
		}
		__int64 mask=1; int first = 1;
		for (i = 0;i<64;i++)
		{
			if (p->groupFlag&mask)
			{
				if (first) {first = 0; strcat(wndName,", group ");}
				//CComboBox *clist = (CComboBox*)GetParent()->GetDlgItem(IDC_LISTTYPE);
				CString str; 
				CWnd *w = GetParent();
				//int k = clist->GetCount();
				//clist->GetLBText(2 + i,str);
				//strcat(wndName,(LPCSTR)str);
				//strcat(wndName," ");
			}
			mask<<=1;
		}
		strcpy(theViewPopup->wndName,wndName);
		theViewPopup->DoModal();
	}
	*/
}

