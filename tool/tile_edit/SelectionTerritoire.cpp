// SelectionTerritoire.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DialogEditList.h"
#include "resource.h"
#include "SelectionTerritoire.h"
#include "GetVal.h"
#include "Browse.h"
#include "nel/3d/tile_bank.h"
#include "nel/misc/stream.h"
#include "nel/misc/file.h"

using namespace NL3D;
using namespace NLMISC;

#define REGKEY_MAINFILE "MAINFILE"

CTileBank tileBank;
CTileBank tileBank2;

void Start(void) //main function
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	SelectionTerritoire Sel;

	/*HKEY regkey; char mainfile[256];
	mainfile[0] = 0;
	if (RegOpenKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{
		unsigned long value;
		if (RegQueryValueEx(regkey,REGKEY_MAINFILE,0,&value,(unsigned char*)mainfile,&value)==ERROR_SUCCESS)
		{		
			char ext[256],drive[256],dir[256],name[256];
			_splitpath(mainfile,drive,dir,name,ext);
			Sel.MainFileName = name;
			Sel.MainFileName += ext;
			Sel.DefautPath = drive;
			Sel.DefautPath += dir;
			Sel.MainFileOk = 1;		
		}
		else 
		{
			RegCloseKey(regkey);
			Sel.MainFileOk = 0;
		}
	}
	else Sel.MainFileOk = 0;*/

	Sel.DoModal();

	//pb avec CFile.Open : 'veut po me charger filename ...	
	/*if (RegOpenKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{
		CString complet = Sel.DefautPath + Sel.MainFileName;
		RegSetValueEx(regkey,REGKEY_MAINFILE,0,REG_SZ,(unsigned char*)(LPCSTR)complet,complet.GetLength()+1);
		RegCloseKey(regkey);
	}*/
}	

/////////////////////////////////////////////////////////////////////////////
// SelectionTerritoire dialog


SelectionTerritoire::SelectionTerritoire(CWnd* pParent /*=NULL*/)
	: CDialog(SelectionTerritoire::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectionTerritoire)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SelectionTerritoire::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectionTerritoire)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SelectionTerritoire, CDialog)
	//{{AFX_MSG_MAP(SelectionTerritoire)
	ON_BN_CLICKED(IDC_ADD_TERRITOIRE, OnAddTerritoire)
	ON_BN_CLICKED(IDC_EDIT_TERRITOIRE, OnEditTerritoire)
	ON_BN_CLICKED(IDC_REMOVE_TERRITOIRE, OnRemoveTerritoire)
	ON_BN_CLICKED(IDC_ADD_TILESET, OnAddTileSet)
	ON_BN_CLICKED(IDC_EDIT_TILESET, OnEditTileSet)
	ON_BN_CLICKED(IDC_EDIT_CHILDREN, OnEditChildren)
	ON_BN_CLICKED(IDC_REMOVE_TILESET, OnRemoveTileSet)
	ON_BN_CLICKED(IDC_EDIT_MONTER, OnMonter)
	ON_BN_CLICKED(IDC_EDIT_DESCENDRE, OnDescendre)
	ON_BN_CLICKED(ID_SELECT, OnSelect)
	ON_BN_CLICKED(ID_SAVE, OnSave)
	ON_BN_CLICKED(ID_SAVE_AS, OnSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectionTerritoire message handlers

void SelectionTerritoire::OnAddTerritoire() 
{
	// TODO: Add your control notification handler code here
	GetVal GetStr;
	GetStr.DoModal();
	if (GetStr.NameOk && GetStr.name[0])
	{
		CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
		if (list->FindStringExact(0,GetStr.name)!=LB_ERR)
		{
			MessageBox("Ce nom existe deja","Erreur",MB_ICONERROR);
		}
		else
		{
			list->InsertString(-1, GetStr.name);
			tileBank.addLand (GetStr.name);
		}
	}
}

class EditTerr : public CDialogEditList
{
public:
	EditTerr (int land)
	{
		_land=land;
	}
private:
	virtual void OnInit ()
	{
		UpdateData ();
		char sTitle[512];
		sprintf (sTitle, "Tile sets use by %s", tileBank.getLand(_land)->getName().c_str());
		SetWindowText (sTitle);
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			m_ctrlCombo.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
			if (tileBank.getLand(_land)->isTileSet (tileBank.getTileSet(i)->getName()))
				m_ctrlList.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
		}
		UpdateData (FALSE);
	}
	virtual void OnOk ()
	{
		UpdateData ();
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			// remove tile set
			tileBank.getLand(_land)->removeTileSet (tileBank.getTileSet(i)->getName());
		}
		for (i=0; i<m_ctrlList.GetCount(); i++)
		{
			CString rString;
			m_ctrlList.GetText(i, rString);
			tileBank.getLand(_land)->addTileSet ((const char*)rString);
		}
		UpdateData (FALSE);
	}
	int _land;
};

void SelectionTerritoire::OnEditTerritoire() 
{
	// TODO: Add your control notification handler code here

	CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		EditTerr edit(index);
		edit.DoModal();
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Erreur",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnRemoveTerritoire() 
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		tileBank.removeLand (nindex);
		list->DeleteString(nindex);
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}





void SelectionTerritoire::OnAddTileSet() 
{
	// TODO: Add your control notification handler code here
	GetVal GetStr;
	GetStr.DoModal();
	if (GetStr.NameOk && GetStr.name[0])
	{
		CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
		if (list->FindStringExact(0,GetStr.name)!=LB_ERR)
		{
			MessageBox("Ce nom existe deja","Erreur",MB_ICONERROR);
		}
		else
		{
			list->InsertString(-1, GetStr.name);
			tileBank.addTileSet (GetStr.name);
		}
	}
}

class EditTileSet : public CDialogEditList
{
public:
	EditTileSet (int tileSet)
	{
		_tileSet=tileSet;
	}
private:
	virtual void OnInit ()
	{
		UpdateData ();
		char sTitle[512];
		sprintf (sTitle, "Children of the tile set %s", tileBank.getTileSet(_tileSet)->getName().c_str());
		SetWindowText (sTitle);
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			if (i!=_tileSet)
				m_ctrlCombo.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
			if (tileBank.getTileSet(_tileSet)->isChild (tileBank.getTileSet(i)->getName()))
				m_ctrlList.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
		}
		UpdateData (FALSE);
	}
	virtual void OnOk ()
	{
		UpdateData ();
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			// remove tile set
			tileBank.getTileSet(_tileSet)->removeChild (tileBank.getTileSet(i)->getName());
		}
		for (i=0; i<m_ctrlList.GetCount(); i++)
		{
			CString rString;
			m_ctrlList.GetText(i, rString);
			tileBank.getTileSet(_tileSet)->addChild ((const char*)rString);
		}
		UpdateData (FALSE);
	}
	int _tileSet;
};

void SelectionTerritoire::OnEditChildren() 
{
	// TODO: Add your control notification handler code here

	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		EditTileSet set (index);
		set.DoModal();
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Erreur",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnEditTileSet() 
{
	// TODO: Add your control notification handler code here

	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		tileBank2=tileBank;
		Browse plop(index, this);
		list->GetText (index,CurrentTerritory);
		if (plop.DoModal()==IDOK)
		{
			tileBank=tileBank2;
			// Resize land
			/*int nsize=plop.m_ctrl.InfoList.GetSize();
			CTileBankLand *pLand=tileBank.getLand(index);
			pLand->resizeTile (nsize);

			// Recopy data
			for (int i=0; i<nsize; i++)
			{
				CTileBankTile* pTile=pLand->getTile (i);
				tilelist::iterator itTile=plop.m_ctrl.InfoList.Get(i);
				pTile->removeTypeFlags (0xffffffff);
				pTile->addTypeFlags ((uint32)itTile->groupFlag);
				pTile->setTransition (CTileBankTile::north, itTile->h);
				pTile->setTransition (CTileBankTile::south, itTile->b);
				pTile->setTransition (CTileBankTile::east, itTile->d);
				pTile->setTransition (CTileBankTile::west, itTile->g);
				if (itTile->path)
					pTile->setFileName (CTileBankTile::diffuse, itTile->path);
				else
					pTile->setFileName (CTileBankTile::diffuse, "");
				if (itTile->nightPath)
					pTile->setFileName (CTileBankTile::additive, itTile->nightPath);
				else
					pTile->setFileName (CTileBankTile::additive, "");
				if (itTile->bumpPath)
					pTile->setFileName (CTileBankTile::bump, itTile->bumpPath);
				else
					pTile->setFileName (CTileBankTile::bump, "");
			}*/
		}
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Erreur",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnRemoveTileSet() 
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		tileBank.removeTileSet(nindex);
		list->DeleteString(nindex);
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnMonter()
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		if (nindex>0)
		{
			tileBank.xchgTileset (nindex, nindex-1);

			// xchg the name
			CString tmp1, tmp2;
			list->GetText(nindex-1, tmp1);
			list->GetText(nindex, tmp2);
			
			list->DeleteString (nindex-1);
			list->DeleteString (nindex-1);

			list->InsertString (nindex-1, tmp1);
			list->InsertString (nindex-1, tmp2);

			list->SetCurSel(nindex-1);
		}
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnDescendre()
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		if (nindex<(list->GetCount()-1))
		{
			tileBank.xchgTileset (nindex, nindex+1);
		
			// xchg the name
			CString tmp1, tmp2;
			list->GetText(nindex, tmp1);
			list->GetText(nindex+1, tmp2);

			list->DeleteString (nindex);
			list->DeleteString (nindex);

			list->InsertString (nindex, tmp1);
			list->InsertString (nindex, tmp2);

			list->SetCurSel(nindex+1);
		}
	}
	else
	{
		MessageBox("Aucun territoire selectionne","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnSelect() 
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
	CListBox *list2=(CListBox*)GetDlgItem(IDC_TILE_SET);
	static char BASED_CODE szFilter[] = 
		"NeL tile bank files (*.bank)|*.bank|All Files (*.*)|*.*||";
 	CFileDialog sFile(true, "bank", "main.bank",0, szFilter, this);
	if (sFile.DoModal()==IDOK)
	{
		POSITION p = sFile.GetStartPosition();
		CString str = sFile.GetNextPathName(p);
		char *temp = str.GetBuffer(256);
		if (temp)
		{
			CIFile stream;
			if (stream.open ((const char*)str))
			{
				list->ResetContent ();
				list2->ResetContent ();
				tileBank.clear();
				tileBank.serial (stream);
			}
			
			for (int i=0; i<tileBank.getLandCount(); i++)
			{
				// Add to the list
				list->AddString(tileBank.getLand(i)->getName().c_str());
			}

			for (i=0; i<tileBank.getTileSetCount(); i++)
			{
				// Add to the list
				list2->AddString(tileBank.getTileSet(i)->getName().c_str());
			}

			char drive[256],name[256],path[256],ext[256];
			_splitpath(temp,drive,path,name,ext);
			MainFileName = name;
			MainFileName += ext;
			DefautPath = drive;
			DefautPath += path;
			
			MainFileOk = 1;
			CButton *button = (CButton*)GetDlgItem(IDC_ADD_TERRITOIRE);
			button->EnableWindow(true);
			button = (CButton*)GetDlgItem(IDC_REMOVE_TERRITOIRE);
			button->EnableWindow(true);
			button = (CButton*)GetDlgItem(IDC_EDIT_TERRITOIRE);
			button->EnableWindow(true);
			button = (CButton*)GetDlgItem(ID_SAVE);
			button->EnableWindow(true);
		}
	}
}

LRESULT SelectionTerritoire::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message==WM_INITDIALOG)
	{
		/*if (!MainFileOk)
		{
			CButton *button = (CButton*)GetDlgItem(IDC_ADD_TERRITOIRE);
			//button->EnableWindow(false);
			button = (CButton*)GetDlgItem(IDC_REMOVE_TERRITOIRE);
			//button->EnableWindow(false);
			button = (CButton*)GetDlgItem(IDC_EDIT_TERRITOIRE);
			//button->EnableWindow(false);
		}
		else
		{
			CString str = DefautPath + MainFileName;
			char *path = str.GetBuffer(256);
			FILE *ptr = fopen(path,"rt");
			char text[100];
			CListBox *list = (CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
			if (ptr && list)
			{				
				int i=0;
				while (!feof(ptr))
				{
					fscanf(ptr,"%s\n",text);
					list->InsertString(i++,text);
				}
			}
		}*/
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void SelectionTerritoire::OnOK() 
{
	// TODO: Add extra validation here
	if (::MessageBox (NULL, "Are you sure you want to quit?", "Quit", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
			CDialog::OnOK();
	}
}

void SelectionTerritoire::OnSave()
{
	// TODO: Add extra validation here
	CString str = DefautPath + MainFileName;
	Save (str);
}

void SelectionTerritoire::Save(const char* path)
{
	// TODO: Add extra validation here
	//if (MainFileOk) //le fichier main est correct : on enregistre
	{
		COFile stream;
		if (stream.open ((const char*)path))
		{
			tileBank.serial (stream);
		}

		/*FILE * ptr = fopen(path,"wt");
		if (ptr)
		{
			CListBox *list = (CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
			char text[100];
			for (int i=0;i<list->GetCount();i++)
			{
				list->GetText(i,text);
				fprintf(ptr,"%s\n",text);
			}
			fclose(ptr);
			CButton *button = (CButton*)GetDlgItem(ID_SAVE);
			button->EnableWindow(true);

			char drive[256],name[256],path2[256],ext[256];
			_splitpath(path,drive,path2,name,ext);
			MainFileName = name;
			MainFileName += ext;
			DefautPath = drive;
			DefautPath += path2;

			return;
		}
		MessageBox("Impossible de sauvegarder","Erreur");*/
	}
}

void SelectionTerritoire::OnSaveAs()
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = 
		"NeL tile bank files (*.bank)|*.bank|All Files (*.*)|*.*||";
 	CFileDialog sFile(false, "bank", DefautPath+MainFileName, 0, szFilter, this);
	if (sFile.DoModal()==IDOK)
	{
		Save (sFile.GetPathName());
		MainFileOk = 1;
		CButton *button = (CButton*)GetDlgItem(IDC_ADD_TERRITOIRE);
		button->EnableWindow(true);
		button = (CButton*)GetDlgItem(IDC_REMOVE_TERRITOIRE);
		button->EnableWindow(true);
		button = (CButton*)GetDlgItem(IDC_EDIT_TERRITOIRE);
		button->EnableWindow(true);
		button = (CButton*)GetDlgItem(ID_SAVE);
		button->EnableWindow(true);
	}
}

void SelectionTerritoire::OnCancel() 
{
	// TODO: Add extra cleanup here
	if (::MessageBox (NULL, "Are you sure you want to quit?", "Quit", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
		CDialog::OnCancel();
	}
}
