
#ifndef TOOLSLOGIC_H
#define TOOLSLOGIC_H

// ***************************************************************************

#include <afxcview.h>
#include "resource.h"
#include "color_button.h"

// ***************************************************************************

class CMainFrame;

// ***************************************************************************

class CToolsLogic : public CTreeView
{
	DECLARE_DYNCREATE(CToolsLogic)

	struct SRegionInfo
	{
		std::string		Name;
		HTREEITEM		RegionItem;
		HTREEITEM		PointItem, PathItem, ZoneItem;
	};

	CMainFrame					*_MainFrame;
	std::vector<SRegionInfo>	_RegionsInfo;

public:

	CToolsLogic();
	
	void init (CMainFrame *pMF);
	void reset ();
	void uninit ();

	uint32 createNewRegion (const std::string &name);
	HTREEITEM addPoint (uint32 nRegion, const std::string &name);
	HTREEITEM addPath  (uint32 nRegion, const std::string &name);
	HTREEITEM addZone  (uint32 nRegion, const std::string &name);
	void expandAll (uint32 nRegion);


	afx_msg void OnRButtonDown (UINT nFlags, CPoint point);	
	afx_msg void OnSelChanged (LPNMHDR pnmhdr, LRESULT *pLResult);

	// Contextual menu handlers
	afx_msg void OnMenuCreate ();
	afx_msg void OnMenuDelete ();
	afx_msg void OnMenuProperties ();
	afx_msg void OnMenuHideAll ();
	afx_msg void OnMenuUnhideAll ();
	afx_msg void OnMenuHide ();
	afx_msg void OnMenuRegionHideAll ();
	afx_msg void OnMenuRegionUnhideAll ();
	
	DECLARE_MESSAGE_MAP()

};

// ***************************************************************************

class CCreateDialog : public CDialog
{
	
public:

	char Name[128], LayerName[128];
	CColorButton ColorBut;
	CRGBA colorInit;

public:

	CCreateDialog(CWnd*pParent);

	virtual BOOL OnInitDialog ();
	virtual void DoDataExchange (CDataExchange* pDX);

	afx_msg void OnColorButton ();

	DECLARE_MESSAGE_MAP()

};

// ***************************************************************************

#endif // TOOLSLOGIC_H
