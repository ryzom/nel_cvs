// ---------------------------------------------------------------------------
// DFNGRID.H
// ---------------------------------------------------------------------------

#ifndef __DFNGRID_H__
#define __DFNGRID_H__

// ---------------------------------------------------------------------------

#include "dfndoc.h"
#include "SuperGridCtrl.h"
#include "nel/misc/types_nl.h"
#include <vector>
#include <string>

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
class CDfnGrid : public CSuperGridCtrl
{
public:
	
	CDfnGrid ();

	virtual int OnCreate (LPCREATESTRUCT lpCreateStruct);
	virtual void OnControlLButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht);
	virtual void OnControlRButtonDown (UINT nFlags, CPoint point, LVHITTESTINFO& ht);
	afx_msg void OnRButtonDown (UINT nFlags, CPoint pt);
	virtual BOOL OnVkReturn();

	afx_msg void OnAdd ();
	afx_msg void OnDel ();
	virtual void OnUpdateListViewItem (CTreeItem* lpItem, LV_ITEM *plvItem);


	void convertToDfn (CDfnDoc::TDfn &out);
	void makeFromDfn (CDfnDoc::TDfn &in);

	DECLARE_MESSAGE_MAP()

private:

	CComboBox* ShowList(int nItem, int nCol, CStringList *lstItems, bool bScrollBar);
	int CalcHorzExtent(CWnd* pWnd, CStringList *pList);

	void GetAllTypAndDfn ();
	void parseAdd(const std::string &path,std::vector<std::string> &dfntyplist);

private:

	uint32 _NbItems;
	sint32 _ItemSelected;

	CImageList m_image;

	std::vector<std::string> _DfnTypList;
};

#endif // __DFNGRID_H__