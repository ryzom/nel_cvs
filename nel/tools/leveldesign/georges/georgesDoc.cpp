// ---------------------------------------------------------------------------
//
// GeorgesDoc.cpp : implementation of the CGeorgesDoc class
//
// ---------------------------------------------------------------------------

#include "stdafx.h"
#include "Georges.h"

#include "GeorgesDoc.h"

#include "../georges_lib/Loader.h"
#include "../georges_lib/Form.h"
#include "../georges_lib/FormFile.h"
#include "../georges_lib/FormBodyEltAtom.h"
#include "../georges_lib/FormBodyEltList.h"
#include "../georges_lib/FormBodyEltStruct.h"
#include "../georges_lib/Item.h"

#include "share.h"

// ---------------------------------------------------------------------------

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP (CGeorgesDoc, CDocument)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE (CGeorgesDoc, CDocument)

// ---------------------------------------------------------------------------
CGeorgesDoc::CGeorgesDoc ()
{
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	CLoader* ploader = pApp->GetLoader ();

	SetDirLevel (pApp->GetDirLevel());
	SetDirPrototype (pApp->GetDirPrototype());
	SetDirDfnTyp (pApp->GetDirDfnTyp());

	CurItem.SetLoader (ploader);
	FileLock = NULL;
}

// ---------------------------------------------------------------------------
CGeorgesDoc::~CGeorgesDoc ()
{
}

// ----------------
// Stack operations
// ----------------

// ---------------------------------------------------------------------------
void CGeorgesDoc::Undo ()
{
	if (itur != UndoRedo.begin())
		--itur;
	CurItem.MakeItem (*itur);
	UpdateAllViews (0);
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::Redo ()
{
	if (itur != UndoRedo.end())
		++itur;
	if (itur != UndoRedo.end())
	{
		CurItem.MakeItem (*itur);
		UpdateAllViews (0);
	}
	else
		--itur;
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::Push ()
{
	++itur;
	UndoRedo.erase (itur, UndoRedo.end());
	CForm form;
	CurItem.MakeForm (form);
	UndoRedo.push_back (form);
	itur = UndoRedo.end ();
	--itur;
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::ResetUndoRedo()
{
	UndoRedo.clear ();
	CForm form;
	CurItem.MakeForm (form);
	UndoRedo.push_back (form);
	itur = UndoRedo.end ();
	--itur;
}

// ---------------
// Item operations
// ---------------

// * Add *
// *******

// ---------------------------------------------------------------------------
void CGeorgesDoc::AddList (uint32 nIndex)
{
	CurItem.AddList (nIndex);
	SetModifiedFlag (TRUE);
	Push ();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::AddParent (uint32 nIndex)
{
	CurItem.AddParent (nIndex); 
	CurItem.VirtualSaveLoad ();
	UpdateAllViews (NULL);
	SetModifiedFlag (TRUE);
	Push ();
}

// * Del *
// *******

// ---------------------------------------------------------------------------
void CGeorgesDoc::DelListChild( uint32 nIndex )
{
	CurItem.DelListChild( nIndex ); 
	SetModifiedFlag( TRUE );
	Push();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::DelParent( uint32 nIndex )
{
	CurItem.DelParent (nIndex);
	CurItem.VirtualSaveLoad ();
	UpdateAllViews (NULL);
	SetModifiedFlag (TRUE);
	Push ();
}

// * Set *
// *******

// ---------------------------------------------------------------------------
void CGeorgesDoc::SetItemValue (uint32 nIndex, const CString sName)
{
	CurItem.SetCurrentValue (nIndex+1, CStringEx(LPCTSTR(sName)));
	SetModifiedFlag (TRUE);
	Push ();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::SetItemParent (uint32 nIndex, const CString sName)
{
	CurItem.SetParent (nIndex, CStringEx(LPCTSTR(sName)));
	CurItem.VirtualSaveLoad();
	UpdateAllViews (NULL);
	SetModifiedFlag (TRUE);
	Push ();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::SetItemActivity (uint32 nIndex, const CString sBool)
{
	CurItem.SetActivity (nIndex, CStringEx(LPCTSTR(sBool)));
	CurItem.VirtualSaveLoad ();
	UpdateAllViews (NULL);
	SetModifiedFlag (TRUE);
	Push ();
}

// * Get *
// *******

// ---------------------------------------------------------------------------
uint32 CGeorgesDoc::GetItemNbElt () const
{
	return CurItem.GetNbElt()-1;
}

// ---------------------------------------------------------------------------
uint32 CGeorgesDoc::GetItemNbElt (uint32 nIndex) const
{
	return CurItem.GetNbElt (nIndex+1);
}

// ---------------------------------------------------------------------------
uint32 CGeorgesDoc::GetItemNbParent () const
{
	return CurItem.GetNbParents ();
}

// ---------------------------------------------------------------------------
uint32 CGeorgesDoc::GetItemInfos (uint32 nIndex) const
{
	return CurItem.GetInfos (nIndex+1);
}

// ---------------------------------------------------------------------------
CString CGeorgesDoc::GetItemName (uint32 nIndex) const
{
	return CString(CurItem.GetName(nIndex+1).c_str());
}

// ---------------------------------------------------------------------------
CString CGeorgesDoc::GetItemCurrentResult (uint32 nIndex) const
{
	return CString(CurItem.GetCurrentResult(nIndex+1).c_str());
}

// ---------------------------------------------------------------------------
CString CGeorgesDoc::GetItemCurrentValue (uint32 nIndex) const
{
	return CString(CurItem.GetCurrentValue(nIndex+1).c_str());
}

// ---------------------------------------------------------------------------
CString CGeorgesDoc::GetItemFormula (uint32 nIndex) const
{
	return CString(CurItem.GetFormula(nIndex+1).c_str());
}

// ---------------------------------------------------------------------------
CString CGeorgesDoc::GetItemActivity (uint32 nIndex) const
{
	return CString(CurItem.GetActivity(nIndex).c_str());
}

// ---------------------------------------------------------------------------
CString CGeorgesDoc::GetItemParent (uint32 nIndex) const
{
	return CString (CurItem.GetParent(nIndex).c_str());
}

// ---------------------------------------------------------------------------
bool CGeorgesDoc::IsItemEnum (uint32 nIndex) const
{
	return CurItem.IsEnum (nIndex+1);
}

// ---------------------------------------------------------------------------
bool CGeorgesDoc::IsItemPredef (uint32 nIndex) const
{
	return CurItem.IsPredef (nIndex+1);
}

// ---------------------------------------------------------------------------
bool CGeorgesDoc::CanEditItem (uint32 nIndex) const
{
	return CurItem.CanEdit (nIndex+1);
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::GetItemListPredef (uint32 nIndex, CStringList* _slist) const
{
	if (!_slist)
		return;
	vector<CStringEx> vsx; 
	vsx.push_back (CStringEx());
	CurItem.GetListPredef (nIndex+1, vsx);
	for (vector<CStringEx>::const_iterator it = vsx.begin(); it != vsx.end(); ++it)
		_slist->AddTail (it->c_str());
}

// ---------------------------------------------------------------------------
#ifdef _DEBUG

// ---------------------------------------------------------------------------
void CGeorgesDoc::AssertValid () const
{
	CDocument::AssertValid ();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::Dump (CDumpContext& dc) const
{
	CDocument::Dump (dc);
}

#endif //_DEBUG
// ---------------------------------------------------------------------------

// ----------------------
// Directories management
// ----------------------

// * Get *
// *******

// ---------------------------------------------------------------------------
CStringEx CGeorgesDoc::GetDirLevel () const
{
	return DirLevel;
}

// ---------------------------------------------------------------------------
CStringEx CGeorgesDoc::GetDirPrototype () const
{
	return DirPrototype;
}

// ---------------------------------------------------------------------------
CStringEx CGeorgesDoc::GetDirDfnTyp () const
{
	return DirDfnTyp;
}

// * Set *
// *******

// ---------------------------------------------------------------------------
void CGeorgesDoc::SetDirLevel (const CStringEx &_sxDirectory)
{
	DirLevel = _sxDirectory;
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::SetDirPrototype (const CStringEx &_sxDirectory)
{
	DirPrototype = _sxDirectory;
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::SetDirDfnTyp (const CStringEx &_sxDirectory)
{
	DirDfnTyp = _sxDirectory;
}

// -----------------
// New / Load / Save
// -----------------

// ---------------------------------------------------------------------------
BOOL CGeorgesDoc::OnNewDocument()
{
	DocumentName = CStringEx();
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	if (!CDocument::OnNewDocument())
		return FALSE;

	CFileDialog Dlg( true );
	int s = Dlg.m_ofn.Flags;
	Dlg.m_ofn.Flags |= OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;
	Dlg.m_ofn.lpstrTitle  = "Choosing a DFN file";
	Dlg.m_ofn.lpstrFilter = "Define files\0*.dfn";
	Dlg.m_ofn.lpstrInitialDir = DirDfnTyp.c_str ();
	int nRet = -1;
	nRet = Dlg.DoModal();

	if( nRet != IDOK )
		return FALSE;

	CString fn = Dlg.GetFileName( );
	LPTSTR p = fn.GetBuffer( 1024 );
	CStringEx sxdfn( p );
	fn.ReleaseBuffer();

	NewDocument (sxdfn);
	return TRUE;
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::NewDocument( const CStringEx _sxfilename )
{
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	pApp->SetDirLevel (GetDirLevel());
	pApp->SetDirPrototype (GetDirPrototype());
	pApp->SetDirDfnTyp (GetDirDfnTyp());
	DeleteContents ();
	CurItem.New (_sxfilename);
	ResetUndoRedo ();
	SetModifiedFlag (FALSE);
}

// ---------------------------------------------------------------------------
BOOL CGeorgesDoc::OnOpenDocument (LPCTSTR lpszPathName) 
{
	DocumentName = CStringEx(lpszPathName);
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	pApp->SetDirLevel (GetDirLevel());
	pApp->SetDirPrototype (GetDirPrototype());
	pApp->SetDirDfnTyp (GetDirDfnTyp());
	DeleteContents ();

	// Test if the file is already locked ?
	if ((FileLock = _fsopen(lpszPathName, "r", _SH_DENYRW)) == NULL)
	{
		string msg = string("File ") + lpszPathName + string(" is already in use");
		pApp->m_pMainWnd->MessageBox (msg.c_str(), "Error", MB_ICONERROR|MB_OK);
		return FALSE;
	}
	else
	{
		fclose (FileLock);
	}

	try
	{
		CurItem.Load (CStringEx(lpszPathName));
		SetModifiedFlag (FALSE);
	}
	catch (NLMISC::Exception &e)
	{
		pApp->m_pMainWnd->MessageBox (e.what(), "CGeorgesDoc::OnOpenDocument", MB_ICONERROR|MB_OK);
		return FALSE;
	}

	ResetUndoRedo();

	// Put a lock onto the file
	FileLock = _fsopen (lpszPathName, "r", _SH_DENYRW);

	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CGeorgesDoc::OnSaveDocument (LPCTSTR lpszPathName) 
{
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());

	pApp->SetDirLevel (GetDirLevel());
	pApp->SetDirPrototype (GetDirPrototype());
	pApp->SetDirDfnTyp (GetDirDfnTyp());

	// Unlock
	if (FileLock != NULL)
		fclose (FileLock);

	// Save
	try
	{
		CurItem.Save (CStringEx(lpszPathName));
		SetModifiedFlag (FALSE);
		if (CStringEx(lpszPathName) != DocumentName)
		{
			DocumentName = CStringEx (lpszPathName);
			DeleteContents ();
			CurItem.Load (DocumentName);
			UpdateAllViews (NULL);
		}
	}
	catch(NLMISC::Exception &e)
	{
		pApp->m_pMainWnd->MessageBox (e.what(), "CGeorgesDoc::OnSaveDocument", MB_ICONERROR|MB_OK);
		return FALSE;
	}

	ResetUndoRedo ();

	// Relock file
	FileLock = _fsopen (lpszPathName, "r", _SH_DENYRW);

	return TRUE;
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::OnCloseDocument () 
{
	if (FileLock != NULL)
		fclose (FileLock);
	DeleteContents ();
	CDocument::OnCloseDocument ();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::DeleteContents() 
{
	CurItem.Clear();
}

// ---------------------------------------------------------------------------
void CGeorgesDoc::UpdateDocument() 
{
	bool b = CurItem.Update();
	SetModifiedFlag( IsModified() || b );
	UpdateAllViews (NULL);
}







/* // Oldies : Create typ and dfn in the program itself
#if 0
	std::vector< std::pair< CStringEx, CStringEx > > lpsx;
	std::vector< std::pair< CStringEx, CStringEx > > lpsx2;

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "true" ),							CStringEx( "true" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "false" ),							CStringEx( "false" ) ) );
	papp->GetLoader()->MakeTyp( "U:/dfn/ENUM_boolean.typ", "string", "BOOL", "true", "", "", "false", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "uint" ),							CStringEx( "uint" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "sint" ),							CStringEx( "sint" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "double" ),							CStringEx( "double" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "filename" ),						CStringEx( "filename" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "string" ),							CStringEx( "string" ) ) );
	papp->GetLoader()->MakeTyp( "U:/dfn/ENUM_type.typ", "string", "TYPE", "true", "", "", "uint", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Designation" ),					CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Substitute" ),					CStringEx( "string.typ" ) ) );
	papp->GetLoader()->MakeDfn( "U:/dfn/predef.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Type" ),						CStringEx( "ENUM_type.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Lowlimit" ),					CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Highlimit" ),					CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "DefaultValue" ),				CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Formula" ),						CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Enum" ),						CStringEx( "ENUM_boolean.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Predef" ),						CStringEx( "list< predef.dfn >" ) ) );
	papp->GetLoader()->MakeDfn( "U:/dfn/typ.dfn", &lpsx );


	// Definition des proprietes d'un vegetable
	lpsx.push_back( std::make_pair( CStringEx( "Include_patats" ),						CStringEx( "list< patat_name.typ >" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Exclude_patats" ),						CStringEx( "list< patat_name.typ >" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Plants" ),								CStringEx( "list< plant_instance.dfn >" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Jitter_Pos" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Scale_Min" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Scale_Max" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Put_On_Water" ),						CStringEx( "boolean.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Water_Height" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Random_Seed" ),							CStringEx( "uint32.typ" ) ) );
	papp->GetLoader()->MakeDfn( "c:/vegetable.dfn", &lpsx );


	// définition des propriétés possibles d'une plante
	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "File name" ),							CStringEx( "plant_name.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Density" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Falloff" ),								CStringEx( "float.typ" ) ) );
	papp->GetLoader()->MakeDfn( "U:/dfn/plant_instance.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Name" ),								CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Shape" ),								CStringEx( "filename.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Shadow" ),								CStringEx( "filename.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Collision_radius" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Bunding_radius" ),						CStringEx( "float.typ" ) ) );
	papp->GetLoader()->MakeDfn( "U:/dfn/plant.dfn", &lpsx );

	// définition globale d'une plante
//	lpsx.clear();
//	lpsx.push_back( std::make_pair( CStringEx( "Glaieul.plant" ),						CStringEx( "Glaieul.plant" ) ) );
//	lpsx.push_back( std::make_pair( CStringEx( "Hortensia.plant" ),						CStringEx( "Hortensia.plant" ) ) );
//	papp->GetLoader()->MakeTyp( "U:/dfn/plant_name.typ", "string", "PLANT", "true", "", "", "Glaieul.plant", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "PatatFrite" ),							CStringEx( "PatatFrite" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "PatatVapeur" ),							CStringEx( "PatatVapeur" ) ) );
	papp->GetLoader()->MakeTyp( "U:/dfn/patat_name.typ", "string", "PATAT", "true", "", "", "PatatFrite", &lpsx, &lpsx2 );

	std::vector< CStringEx > lsx;
	lsx.push_back( "fPatateFrite" );
	lsx.push_back( "fPatatePuree" );
	lsx.push_back( "fPatateVapeur" );
	lsx.push_back( "fPatateSautee" );
	papp->GetLoader()->SetTypPredef( "U:/dfn/patat_name.typ", lsx );
	papp->UpdateAllDocument();
#endif
*/
	

/*
	std::list< std::pair< CStringEx, CStringEx > > lpsx;

	lpsx.push_back( std::make_pair( CStringEx( "Nom_de_famille" ),		CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Prenom" ),				CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Peuple" ),				CStringEx( "enum_peuple.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Sexe" ),				CStringEx( "enum_sexe.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Date_de_naissance" ),	CStringEx( "uint64.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Esperance_de_vie" ),	CStringEx( "uint64.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Role" ),				CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Level" ),				CStringEx( "uint16.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Caracteristiques" ),	CStringEx( "caracteristiques.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Scores_derives" ),		CStringEx( "scorederive.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Fame" ),				CStringEx( "fame.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Clades" ),				CStringEx( "clade.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Pacte_kamique" ),		CStringEx( "enum_pactekamique.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Pacte_survie" ),		CStringEx( "pactesurvie.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Competences" ),			CStringEx( "competences.dfn" ) ) );
	loader.MakeDfn( "U:/dfn/feuilleperso.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Force" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Constitution" ),		CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Agilite" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Volonte" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Equilibre" ),			CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Intelligence" ),		CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Charisme" ),			CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Empathie" ),			CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Perception" ),			CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/caracteristiques.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "(PV)_Points_de_vie" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RPV)_Recuperation_PV" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(PS)_Points_de_seve" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RPS)_Recuperation_PS" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(PF)_Points_de_fatigue" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RPF)_Recuperation_PF" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(PO)_Poids" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(CP)_Charge_portable" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RPOI)_Resistance_maladie_poison" ),	CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RMAG)_Resistance_magie" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RTEC)_Resistance_effet_tech_humain" ),	CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RCHO)_Resistance_choc" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(RPEU)_Resistance_peur" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(VIT)_Vitesse_de_deplacement" ),		CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/scorederive.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "(FAM1)_Fame_du_peuple" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(FAM1)_Fame_peuple2" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "(FAM1)_Fame_tribu_X" ),					CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/fame.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Clade principal" ),						CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Clade Honorifique 1" ),					CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Clade Honorifique 2" ),					CStringEx( "string.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Clade Honorifique 3" ),					CStringEx( "string.typ" ) ) );
	loader.MakeDfn( "U:/dfn/clade.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Type" ),								CStringEx( "enum_pactesurvie.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Level" ),								CStringEx( "uint8.typ" ) ) );
	loader.MakeDfn( "U:/dfn/pactesurvie.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Recoltes" ),							CStringEx( "cp_recolte.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Survie" ),								CStringEx( "cp_survie.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Civilisations" ),						CStringEx( "cp_civilisations.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Ryzom" ),								CStringEx( "cp_ryzom.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_melee" ),						CStringEx( "cp_combat_melee.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_distance" ),						CStringEx( "cp_combat_distance.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Magie" ),								CStringEx( "cp_magie.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Deplacement" ),							CStringEx( "cp_deplacement.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Animaux" ),								CStringEx( "cp_animaux.dfn" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Faber" ),								CStringEx( "cp_faber.dfn" ) ) );
	loader.MakeDfn( "U:/dfn/competences.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Recolte" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Equarissage" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Peche" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Plongee" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Labour" ),								CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_recolte.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Conn_desert" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_jungle" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_lacs" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_primes_racines" ),					CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_survie.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Conn_fyros" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_tryckers" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_matis" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_zorai" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_dragon" ),							CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_civilisations.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Conn_caravane" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_rocktabou" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_kittins" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_goo" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_kamis" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_myst_tryckers" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_myst_matis" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_myst_zorai" ),						CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_ryzom.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Combat_dague" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_epee" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_hache" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_masse" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_lance" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_baton" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_grande_epee" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_grande_hache" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_grande_masse" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_pique" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_mains_nues" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_coup_de_pied" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Combat_contact_equestre" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Maniement_Rondache" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Maniement_Ecu" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Deux_armes" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Esquive" ),								CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_combat_melee.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Tir_arc" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Tir_grand_arc" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Tir_pistolet" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Tir_fusil" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Tir_arme_lourde" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Tir_roquette" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Lancer_grenade" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Tir_equestre" ),						CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_combat_distance.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Conn_mandragore" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_dryade" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conn_sylve" ),							CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_magie.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Course" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Nage" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Equitation" ),							CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_deplacement.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Dressage" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Conduite_animal_de_bat" ),				CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_animaux.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Taille_armes_de_contact" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Assemblage_armes_de_jet" ),				CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Façonnage_armure" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Travail_du_textile" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Travail_du_cuir" ),						CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Travail des_bijoux" ),					CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Cuisinier" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Mecanicien" ),							CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Macon" ),								CStringEx( "float.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Apothicaire" ),							CStringEx( "float.typ" ) ) );
	loader.MakeDfn( "U:/dfn/cp_faber.dfn", &lpsx );
	lpsx.clear();

	std::list< std::pair< CStringEx, CStringEx > > lpsx2;
	loader.MakeTyp( "U:/dfn/string.typ", "string", "string", "false", "", "", "", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/float.typ", "double", "float", "false", "0.0", "3.402823466e+38F", "0.0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/string.typ", "string", "file name", "false", "", "", "", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/uint64.typ", "uint", "uint64", "false", "0", "18446744073709551615", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/uint32.typ", "uint", "uint32", "false", "0", "4294967295", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/uint16.typ", "uint", "uint16", "false", "0", "65535", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/uint8.typ",  "uint", "uint8",  "false", "0", "255", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/sint64.typ", "sint", "sint64", "false", "-9223372036854775808", "9223372036854775807", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/sint32.typ", "sint", "sint32", "false", "-2147483648", "2147483647", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/sint16.typ", "sint", "sint16", "false", "-32768", "32767", "0", &lpsx, &lpsx2 );
	loader.MakeTyp( "U:/dfn/sint8.typ",  "sint", "sint8",  "false", "-128", "127", "0", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Fyros" ),				CStringEx( "0" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Trycker" ),				CStringEx( "1" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Kalab" ),				CStringEx( "2" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Matis" ),				CStringEx( "3" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Kitin" ),				CStringEx( "4" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Zorai" ),				CStringEx( "5" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Noon" ),				CStringEx( "6" ) ) );
	loader.MakeTyp( "U:/dfn/enum_peuple.typ", "uint", "ENUM Peuple Fyros", "true", "0", "6", "Fyros", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Male" ),				CStringEx( "0" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Femelle" ),				CStringEx( "1" ) ) );
	loader.MakeTyp( "U:/dfn/enum_sexe.typ", "uint", "ENUM Sexe", "true", "0", "1", "Male", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Rien" ),				CStringEx( "0" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "PacteA" ),				CStringEx( "1" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "PacteB" ),				CStringEx( "2" ) ) );
	loader.MakeTyp( "U::/dfn/enum_pactekamique.typ", "uint", "ENUM Pacte kamique", "true", "0", "2", "Rien", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Rien" ),				CStringEx( "0" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Caravane" ),			CStringEx( "1" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Kami" ),				CStringEx( "2" ) ) );
	loader.MakeTyp( "U:/dfn/enum_pactesurvie.typ", "uint", "ENUM Pacte survie", "true", "0", "2", "Rien", &lpsx, &lpsx2 );
	return true );
*/

/*
	std::list< std::pair< CStringEx, CStringEx > > lpsx;
	std::list< std::pair< CStringEx, CStringEx > > lpsx2;

	lpsx.push_back( std::make_pair( CStringEx( "Type" ),				CStringEx( "enum_ai_type.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Quantity" ),			CStringEx( "uint32.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Spawn_zone" ),			CStringEx( "patat_name.typ" ) ) );
	loader.MakeDfn( "U:/dfn/AI.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Boundary" ),			CStringEx( "patat_name.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "PathFinding_mode" ),	CStringEx( "enum_ai_pathfinding.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Special_location" ),	CStringEx( "list<Special_location.dfn>" ) ) );
	loader.MakeDfn( "U:/dfn/AIMGR.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Type" ),				CStringEx( "enum_ai_special_location_type.typ" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Location" ),			CStringEx( "patat_name.typ" ) ) );
	loader.MakeDfn( "U:/dfn/Special_location.dfn", &lpsx );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Walker_happy" ),		CStringEx( "Walker_happy" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Walker_unhappy" ),		CStringEx( "Walker_unhappy" ) ) );
	loader.MakeTyp( "U:/dfn/enum_ai_type.typ", "string", "ENUM AI TYPE", "true", "", "", "Walker_happy", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Town" ),				CStringEx( "Town" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Outdoors" ),			CStringEx( "Outdoors" ) ) );
	loader.MakeTyp( "U:/dfn/enum_ai_pathfinding.typ", "string", "ENUM AI", "true", "", "", "Town", &lpsx, &lpsx2 );
	
	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Place_marche" ),		CStringEx( "Place_marche" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Marchand_eau" ),		CStringEx( "Marchand_eau" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Agora" ),				CStringEx( "Agora" ) ) );
	loader.MakeTyp( "U:/dfn/enum_ai_special_location_type.typ", "string", "ENUM AI SPLOC", "true", "", "", "Place_marche", &lpsx, &lpsx2 );

	lpsx.clear();
	lpsx.push_back( std::make_pair( CStringEx( "Patat1" ),				CStringEx( "Patat1" ) ) );
	lpsx.push_back( std::make_pair( CStringEx( "Patat2" ),				CStringEx( "Patat2" ) ) );
	loader.MakeTyp( "U:/dfn/patat_name.typ", "string", "PATAT", "true", "", "", "Patat1", &lpsx, &lpsx2 );
	return TRUE );
*/
