
#ifndef TOOLSZONE_H
#define TOOLSZONE_H

// ***************************************************************************

#include <afxcview.h>
#include "resource.h"

#include <string>
#include <vector>

// ***************************************************************************

class CMainFrame;

// ***************************************************************************

class CToolsZone : public CFormView
{
	DECLARE_DYNCREATE(CToolsZone)
		
	CMainFrame *_MainFrame;
	HTREEITEM _PointItem, _PathItem, _ZoneItem;

	bool _TreeCreated;
	
private:

	void addToAllCatTypeCB (const std::string &Name);
	void updateComboPairAndFilter (int CatTypeId, int CatValueId, std::string *pFilterType);

public:
	
	CToolsZone();

	CTreeCtrl *getTreeCtrl();
	
	void init (CMainFrame *pMF);
	void uninit ();

	// Event handlers
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnSelectCatType1 ();
	afx_msg void OnSelectCatType2 ();
	afx_msg void OnSelectCatType3 ();
	afx_msg void OnSelectCatType4 ();
	afx_msg void OnSelectCatValue1 ();
	afx_msg void OnSelectCatValue2 ();
	afx_msg void OnSelectCatValue3 ();
	afx_msg void OnSelectCatValue4 ();
	afx_msg void OnSelectAnd2 ();
	afx_msg void OnSelectOr2 ();
	afx_msg void OnSelectAnd3 ();
	afx_msg void OnSelectOr3 ();
	afx_msg void OnSelectAnd4 ();
	afx_msg void OnSelectOr4 ();
	afx_msg void OnSelectRandom ();
	afx_msg void OnSelChanged (LPNMHDR pnmhdr, LRESULT *pLResult);

	DECLARE_MESSAGE_MAP()
};

// ***************************************************************************

#endif // TOOLSZONE_H
