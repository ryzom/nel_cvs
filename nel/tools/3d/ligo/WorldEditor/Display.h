
#ifndef DISPLAY_H
#define DISPLAY_H

#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"

#include "3d/font_manager.h"
#include "3d/text_context.h"

class CMainFrame;

class CDisplay : public CView
{
	DECLARE_DYNCREATE(CDisplay)

	CMainFrame *_MainFrame;

	// Text
	NL3D::CFontManager	_FontManager;
	NL3D::CTextContext	_TextContext;
	std::string		_TextToDisplay;

public:

	bool	_MouseLeftDown;
	bool	_MouseRightDown;
	bool	_MouseMidDown;
	bool	_MouseMoved;
	bool	_MouseMoving;
	uint32	_Counter;
	bool	_MouseOnSelection;
	CPoint	_LastMousePos;
	NLMISC::CVector	_SelectionMin, _SelectionMax;
	
	// World infos
	float	_Factor;
	NLMISC::CVector _Offset;
	NLMISC::CVector	_InitViewMin, _InitViewMax;
	float	_CellSize;
	bool	_DisplayGrid;
	bool	_DisplayLogic;
	bool	_DisplayZone;
	bool	_DisplaySelection;
	// This is the inworld coordinate of the bounding square
	NLMISC::CVector	_CurViewMin, _CurViewMax;
	NLMISC::CVector	_CurPos;
	sint32	_LastX, _LastY;

	bool _CtrlKeyDown;
	NLMISC::CRGBA _BackgroundColor;


public:

	CDisplay();
	
	void init (CMainFrame *pMF); // pMF For update of the statusbar
	void setCellSize (float size);

	void setBackgroundColor (NLMISC::CRGBA &col);
	NLMISC::CRGBA getBackgroundColor ();

	void setDisplayGrid (bool bDisp);
	bool getDisplayGrid ();
	void calcCurView();
	NLMISC::CVector convertToWorld(CPoint &p);

	void renderSelection ();

	virtual void OnDraw (CDC* pDC);

private:

	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy ();

	afx_msg void OnMouseWheel	(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnMButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove	(UINT nFlags, CPoint point);
	afx_msg void OnChar (UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnTimer (UINT nIDEvent);

	DECLARE_MESSAGE_MAP()

private:

	//void displayGrid();

};

#endif // DISPLAY_H
/////////////////////////////////////////////////////////////////////////////
