
#ifndef DISPLAY_H
#define DISPLAY_H

#include "nel/misc/vector.h"

class CMainFrame;

class CDisplay : public CView
{
	DECLARE_DYNCREATE(CDisplay)

	CMainFrame *_MainFrame;

public:

	bool	_MouseLeftDown;
	bool	_MouseRightDown;
	bool	_MouseMidDown;
	CPoint	_LastMousePos;

	// World infos
	float	_Factor;
	NLMISC::CVector _Offset;
	NLMISC::CVector	_InitViewMin, _InitViewMax;
	float	_CellSize;
	bool	_DisplayGrid;
	bool	_DisplayLogic;
	bool	_DisplayZone;
	// This is the inworld coordinate of the bounding square
	NLMISC::CVector	_CurViewMin, _CurViewMax;
	NLMISC::CVector	_CurPos;
	sint32	_LastX, _LastY;

	bool _CtrlKeyDown;

public:

	CDisplay();
	
	void init (CMainFrame *pMF); // pMF For update of the statusbar
	void setCellSize (float size);
	void setDisplayGrid (bool bDisp);
	bool getDisplayGrid ();
	void calcCurView();
	NLMISC::CVector convertToWorld(CPoint &p);

	virtual void OnDraw (CDC* pDC);

	afx_msg void OnMouseWheel	(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnMButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove	(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp (UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()

private:

	//void displayGrid();

};

#endif // DISPLAY_H
/////////////////////////////////////////////////////////////////////////////
