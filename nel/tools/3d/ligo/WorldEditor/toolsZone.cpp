#include "stdafx.h"
#include "toolsZone.h"

#include "mainfrm.h"

#include <string>

using namespace std;

// ---------------------------------------------------------------------------

IMPLEMENT_DYNCREATE (CToolsZone, CFormView)

// ---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CToolsZone, CFormView)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_SIZE ()
	ON_CBN_SELCHANGE (IDC_CATTYPE1, OnSelectCatType1)
	ON_CBN_SELCHANGE (IDC_CATTYPE2, OnSelectCatType2)
	ON_CBN_SELCHANGE (IDC_CATTYPE3, OnSelectCatType3)
	ON_CBN_SELCHANGE (IDC_CATTYPE4, OnSelectCatType4)
	ON_CBN_SELCHANGE (IDC_CATVALUE1, OnSelectCatValue1)
	ON_CBN_SELCHANGE (IDC_CATVALUE2, OnSelectCatValue2)
	ON_CBN_SELCHANGE (IDC_CATVALUE3, OnSelectCatValue3)
	ON_CBN_SELCHANGE (IDC_CATVALUE4, OnSelectCatValue4)
	ON_BN_CLICKED (IDC_AND2, OnSelectAnd2)
	ON_BN_CLICKED (IDC_OR2, OnSelectOr2)
	ON_BN_CLICKED (IDC_AND3, OnSelectAnd3)
	ON_BN_CLICKED (IDC_OR3, OnSelectOr3)
	ON_BN_CLICKED (IDC_AND4, OnSelectAnd4)
	ON_BN_CLICKED (IDC_OR4, OnSelectOr4)
	ON_BN_CLICKED (IDC_RANDOM, OnSelectRandom)	
	ON_NOTIFY (TVN_SELCHANGED, IDC_TREE, OnSelChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
CToolsZone::CToolsZone() : CFormView(IDD_TOOLS_ZONE)
{
	_TreeCreated = false;
}

// ---------------------------------------------------------------------------
CTreeCtrl *CToolsZone::getTreeCtrl()
{
	return (CTreeCtrl*)GetDlgItem (IDC_TREE);
}

// ---------------------------------------------------------------------------
void CToolsZone::addToAllCatTypeCB (const string &Name)
{
	CComboBox* pCB;
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE1);
	pCB->AddString (Name.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE2);
	pCB->AddString (Name.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE3);
	pCB->AddString (Name.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE4);
	pCB->AddString (Name.c_str());
}

// ---------------------------------------------------------------------------
void CToolsZone::init (CMainFrame *pMF)
{
	_MainFrame = pMF;
	_MainFrame->_ZoneBuilder.setToolsZone (this);

	addToAllCatTypeCB (STRING_UNUSED);
	vector<string> allCategoryTypes;
	_MainFrame->_ZoneBuilder.getZoneBank().getCategoriesType (allCategoryTypes);
	for(uint32 i = 0; i < allCategoryTypes.size(); ++i)
		addToAllCatTypeCB (allCategoryTypes[i]);

	// Select right category types
	CComboBox* pCB;
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE1);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterType1.c_str());	
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE2);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterType2.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE3);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterType3.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE4);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterType4.c_str());	

	updateComboPairAndFilter (IDC_CATTYPE1, IDC_CATVALUE1, &_MainFrame->_ZoneBuilder._FilterType1);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE1);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterValue1.c_str());
	updateComboPairAndFilter (IDC_CATTYPE2, IDC_CATVALUE2, &_MainFrame->_ZoneBuilder._FilterType2);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE2);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterValue2.c_str());
	updateComboPairAndFilter (IDC_CATTYPE3, IDC_CATVALUE3, &_MainFrame->_ZoneBuilder._FilterType3);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE3);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterValue3.c_str());
	updateComboPairAndFilter (IDC_CATTYPE4, IDC_CATVALUE4, &_MainFrame->_ZoneBuilder._FilterType4);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE4);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder._FilterValue4.c_str());

	// Select right operators
	CButton *pButAnd, *pButOr;
	pButAnd = (CButton*)GetDlgItem(IDC_AND2); 
	pButOr =(CButton*)GetDlgItem(IDC_OR2);
	if (_MainFrame->_ZoneBuilder._FilterOperator2 == 0)
		pButAnd->SetCheck (1);
	else
		pButOr->SetCheck (1);

	pButAnd = (CButton*)GetDlgItem(IDC_AND3); 
	pButOr =(CButton*)GetDlgItem(IDC_OR3);
	if (_MainFrame->_ZoneBuilder._FilterOperator3 == 0)
		pButAnd->SetCheck (1);
	else
		pButOr->SetCheck (1);

	pButAnd = (CButton*)GetDlgItem(IDC_AND4); 
	pButOr =(CButton*)GetDlgItem(IDC_OR4);
	if (_MainFrame->_ZoneBuilder._FilterOperator4 == 0)
		pButAnd->SetCheck (1);
	else
		pButOr->SetCheck (1);

	CButton *pButRan = (CButton*)GetDlgItem(IDC_RANDOM); 
	if (_MainFrame->_ZoneBuilder._RandomSelection)
		pButRan->SetCheck (1);
	else
		pButRan->SetCheck (0);
	
	_TreeCreated = true;
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::uninit()
{
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSize (UINT nType, int cx, int cy)
{
	CFormView::OnSize (nType, cx, cy);
	// Resize tree ctrl to fill the whole view.
	if (_TreeCreated)
		getTreeCtrl()->MoveWindow (10, 140, cx-20, cy-150);
}

// ---------------------------------------------------------------------------
void CToolsZone::updateComboPairAndFilter (int CatTypeId, int CatValueId, string *pFilterType)
{
	uint32 i;
	char sTmp[256];
	CComboBox *pCBType, *pCBValue;
	pCBType = (CComboBox*)GetDlgItem (CatTypeId);
	pCBType->GetLBText (pCBType->GetCurSel(), sTmp);
	*pFilterType = sTmp;
	pCBValue = (CComboBox*)GetDlgItem (CatValueId);
	pCBValue->ResetContent ();

	if (*pFilterType == STRING_UNUSED)
		return;

	vector<string> allCategoryValues;
	_MainFrame->_ZoneBuilder.getZoneBank().getCategoryValues (*pFilterType, allCategoryValues);
	for(i = 0; i < allCategoryValues.size(); ++i)
		pCBValue->AddString (allCategoryValues[i].c_str());
	pCBValue->SetCurSel (0);
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType1 ()
{
	updateComboPairAndFilter (IDC_CATTYPE1, IDC_CATVALUE1, &_MainFrame->_ZoneBuilder._FilterType1);
	OnSelectCatValue1 ();
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType2 ()
{
	updateComboPairAndFilter (IDC_CATTYPE2, IDC_CATVALUE2, &_MainFrame->_ZoneBuilder._FilterType2);
	OnSelectCatValue2 ();
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType3 ()
{
	updateComboPairAndFilter (IDC_CATTYPE3, IDC_CATVALUE3, &_MainFrame->_ZoneBuilder._FilterType3);
	OnSelectCatValue3 ();
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType4 ()
{
	updateComboPairAndFilter (IDC_CATTYPE4, IDC_CATVALUE4, &_MainFrame->_ZoneBuilder._FilterType4);
	OnSelectCatValue4 ();
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue1()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE1);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder._FilterValue1 = sTmp;
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue2()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE2);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder._FilterValue2 = sTmp;
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue3()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE3);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder._FilterValue3 = sTmp;
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue4()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE4);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder._FilterValue4 = sTmp;
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectAnd2 ()
{
	if (_MainFrame->_ZoneBuilder._FilterOperator2 == 0) return;
	_MainFrame->_ZoneBuilder._FilterOperator2 = 0; // And
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectOr2 ()
{
	if (_MainFrame->_ZoneBuilder._FilterOperator2 == 1) return;
	_MainFrame->_ZoneBuilder._FilterOperator2 = 1; // Or
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectAnd3 ()
{
	if (_MainFrame->_ZoneBuilder._FilterOperator3 == 0) return;
	_MainFrame->_ZoneBuilder._FilterOperator3 = 0; // And
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectOr3 ()
{
	if (_MainFrame->_ZoneBuilder._FilterOperator3 == 1) return;
	_MainFrame->_ZoneBuilder._FilterOperator3 = 1; // Or
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectAnd4 ()
{
	if (_MainFrame->_ZoneBuilder._FilterOperator3 == 0) return;
	_MainFrame->_ZoneBuilder._FilterOperator4 = 0; // And
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectOr4 ()
{
	if (_MainFrame->_ZoneBuilder._FilterOperator4 == 1) return;
	_MainFrame->_ZoneBuilder._FilterOperator4 = 1; // Or
	_MainFrame->_ZoneBuilder.updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRandom()
{
	_MainFrame->_ZoneBuilder._RandomSelection = !_MainFrame->_ZoneBuilder._RandomSelection;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelChanged (LPNMHDR pnmhdr, LRESULT *pLResult)
{
	HTREEITEM hItem = getTreeCtrl()->GetSelectedItem();

	if (hItem != NULL)
	{
		// Select the item
		getTreeCtrl()->Select(hItem, TVGN_CARET);
		_MainFrame->_ZoneBuilder._CurSelectedZone = (LPCTSTR)getTreeCtrl()->GetItemText(hItem);
	}
	else
		_MainFrame->_ZoneBuilder._CurSelectedZone = STRING_UNUSED;
}
