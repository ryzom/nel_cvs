#include "stdafx.h"
#include "display.h"

#include <3d/nelu.h>
#include <3d/init_3d.h>
#include <3d/register_3d.h>
#include <3d/scene.h>

#include <nel/misc/common.h>

#include "mainfrm.h"

using namespace NL3D;

// ---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CDisplay, CView)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CDisplay, CView)

// ---------------------------------------------------------------------------
CDisplay::CDisplay ()
{
	_MouseRightDown = false;
	_MouseLeftDown = false;
	_MouseMidDown = false;
	_Factor = 1.0f;
	_Offset = CVector(0.0f, 0.0f, 0.0f);
	_DisplayGrid = true;
	_DisplayLogic = true;
	_DisplayZone = true;
	_LastX = _LastY = -10000;
	_CtrlKeyDown = false;
}

// ---------------------------------------------------------------------------
void CDisplay::init (CMainFrame *pMF)
{
	_MainFrame = pMF;
	pMF->_PRegionBuilder.setDisplay (this);
	pMF->_ZoneBuilder.setDisplay (this);
	registerSerial3d ();
	CScene::registerBasics ();
	init3d ();

	//CNELU::init (512, 512, CViewport(), 32, true, this->m_hWnd);
	//NL3D::registerSerial3d();
	CNELU::initDriver(512, 512, 32, true, this->m_hWnd);
	CNELU::initScene(CViewport());


	// setMatrixMode2D11
	CFrustum f(0.0f,1.0f,0.0f,1.0f,-1.0f,1.0f,false);
	CVector		I(1,0,0);
	CVector		J(0,0,1);
	CVector		K(0,-1,0);
	CMatrix		ViewMatrix, ModelMatrix;
	ViewMatrix.identity ();
	ViewMatrix.setRot (I,J,K, true);
	ModelMatrix.identity ();

	CNELU::Driver->setFrustum (f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
	CNELU::Driver->setupViewMatrix (ViewMatrix);
	CNELU::Driver->setupModelMatrix (ModelMatrix);
	CNELU::Driver->activate ();
	CNELU::clearBuffers ();
	CNELU::swapBuffers ();
	OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CDisplay::setCellSize (float size)
{
	_CellSize = size;
	_InitViewMax = CVector(5.5f*_CellSize, 5.5f*_CellSize, 0.0f);
	_InitViewMin = CVector(-5.5f*_CellSize, -5.5f*_CellSize, 0.0f);
}

// ---------------------------------------------------------------------------
void CDisplay::setDisplayGrid (bool bDisp)
{
	_DisplayGrid = bDisp;
	OnPaint();
}

// ---------------------------------------------------------------------------
bool CDisplay::getDisplayGrid ()
{
	return _DisplayGrid;
}

// ---------------------------------------------------------------------------
void CDisplay::calcCurView()
{
	_CurViewMin = _InitViewMin * _Factor + _Offset;
	_CurViewMax = _InitViewMax * _Factor + _Offset;
}

// ---------------------------------------------------------------------------
CVector CDisplay::convertToWorld (CPoint&p)
{
	CVector ret;
	CRect rect;
	
	GetClientRect (rect);
	calcCurView ();
	ret.x = ((float)p.x) / ((float)rect.Width());
	ret.x = ret.x*(_CurViewMax.x-_CurViewMin.x) + _CurViewMin.x;
	ret.y = 1.0f - (((float)p.y) / ((float)rect.Height()));
	ret.y = ret.y*(_CurViewMax.y-_CurViewMin.y) + _CurViewMin.y;
	ret.z = 0.0f;
	return ret;
}

// ---------------------------------------------------------------------------
void CDisplay::OnDraw (CDC* pDC)
{
	if (CNELU::Driver == NULL)
		return;

	CNELU::clearBuffers ();

	// Display the grid
	calcCurView ();

	if (_DisplayZone)
		_MainFrame->_ZoneBuilder.render (_CurViewMin, _CurViewMax);

	if (_DisplayGrid)
		_MainFrame->_ZoneBuilder.displayGrid (_CurViewMin, _CurViewMax);

	if (_DisplayLogic)
		_MainFrame->_PRegionBuilder.render (_CurViewMin, _CurViewMax);

	CNELU::Scene.render ();
	
	CNELU::swapBuffers ();
}

// ---------------------------------------------------------------------------
void CDisplay::OnMouseWheel (UINT nFlags, short zDelta, CPoint point)
{
	if (zDelta > 0)
	{
		if (_Factor >= 1.0f)
			_Factor += 1.0f;
		else
			_Factor += 0.05f;
	}
	else
	{
		if (_Factor > 1.0f)
			_Factor -= 1.0f;
		else
			_Factor -= 0.05f;
	}
	NLMISC::clamp (_Factor, 0.05f, 5.0f);
	OnPaint();
}

// ---------------------------------------------------------------------------
void CDisplay::OnMButtonDown (UINT nFlags, CPoint point)
{
	_MouseMidDown = true;
	_LastMousePos = point;
	OnPaint();
}

// ---------------------------------------------------------------------------
void CDisplay::OnMButtonUp (UINT nFlags, CPoint point)
{
	_MouseMidDown = false;
}

// ---------------------------------------------------------------------------
void CDisplay::OnLButtonDown (UINT nFlags, CPoint point)
{
	_CurPos = convertToWorld (point);
	if (_MainFrame->_Mode == 0) // Mode Zone
	{
		_MainFrame->_ZoneBuilder.add (_CurPos);
		_LastX = (sint32)floor (_CurPos.x / _CellSize);
		_LastY = (sint32)floor (_CurPos.y / _CellSize);
	}
	
	if (_MainFrame->_Mode == 1) // Mode Logic
	{
		if (_MainFrame->_PRegionBuilder.getSelPB() != NULL)
		{
			CVector p = convertToWorld (point);
			// Select a vertex
			bool bSelected = false;
			CVector selMin = CVector(	p.x-(_CurViewMax.x-_CurViewMin.x)*0.01f,
										p.y-(_CurViewMax.y-_CurViewMin.y)*0.01f, 0.0f );
			CVector selMax = CVector(	p.x+(_CurViewMax.x-_CurViewMin.x)*0.01f,
										p.y+(_CurViewMax.y-_CurViewMin.y)*0.01f, 0.0f );
			bSelected = _MainFrame->_PRegionBuilder.selectVertexOnSelPB (selMin, selMax);
			// if we cant select a vertex create one
			if (!bSelected)
			{
				_MainFrame->_PRegionBuilder.createVertexOnSelPB (p);
			}
		}
	}
	_MouseLeftDown = true;
	_LastMousePos = point;
	OnPaint();
}

// ---------------------------------------------------------------------------
void CDisplay::OnLButtonUp (UINT nFlags, CPoint point)
{
	_MouseLeftDown = false;
}

// ---------------------------------------------------------------------------
void CDisplay::OnRButtonDown (UINT nFlags, CPoint point)
{
	_CurPos = convertToWorld (point);
	if (_MainFrame->_Mode == 0) // Mode Zone
	{
		_MainFrame->_ZoneBuilder.del (_CurPos);
		_LastX = (sint32)floor (_CurPos.x / _CellSize);
		_LastY = (sint32)floor (_CurPos.y / _CellSize);
	}

	if (_MainFrame->_Mode == 1) // Mode Logic
	{
		if (_MainFrame->_PRegionBuilder.getSelPB() != NULL)
		{
			CVector p = convertToWorld (point);

			// Select a vertex
			bool bSelected = false;
			CVector selMin = CVector(	p.x-(_CurViewMax.x-_CurViewMin.x)*0.01f,
										p.y-(_CurViewMax.y-_CurViewMin.y)*0.01f, 0.0f );
			CVector selMax = CVector(	p.x+(_CurViewMax.x-_CurViewMin.x)*0.01f,
										p.y+(_CurViewMax.y-_CurViewMin.y)*0.01f, 0.0f );
			bSelected = _MainFrame->_PRegionBuilder.selectVertexOnSelPB (selMin, selMax);

			if (bSelected)
			{
				_MainFrame->_PRegionBuilder.delSelVertexOnSelPB ();
			}
		}
	}
	_MouseRightDown = true;
	OnPaint();
}

// ---------------------------------------------------------------------------
void CDisplay::OnRButtonUp (UINT nFlags, CPoint point)
{
	_MouseRightDown = false;
}

// ---------------------------------------------------------------------------
void CDisplay::OnMouseMove (UINT nFlags, CPoint point)
{
	// Get the current position in world
	_CurPos = convertToWorld (point);

	if (_MouseLeftDown)
	{
		if (_MainFrame->_Mode == 0) // Mode Zone
		{
			sint32 x = (sint32)floor (_CurPos.x / _CellSize);
			sint32 y = (sint32)floor (_CurPos.y / _CellSize);
			if ((x!=_LastX)||(y!=_LastY))
			{
				_LastX = x;
				_LastY = y;
				_MainFrame->_ZoneBuilder.add (_CurPos);
			}
		}
	
		if (_MainFrame->_Mode == 1) // Mode Logic
			_MainFrame->_PRegionBuilder.setSelVertexOnSelPB (_CurPos);
		_LastMousePos = point;
		OnPaint();
	}

	if (_MouseRightDown)
	{
		if (_MainFrame->_Mode == 0) // Mode Zone
		{
			sint32 x = (sint32)floor (_CurPos.x / _CellSize);
			sint32 y = (sint32)floor (_CurPos.y / _CellSize);
			if ((x!=_LastX)||(y!=_LastY))
			{
				_LastX = x;
				_LastY = y;
				_MainFrame->_ZoneBuilder.del (_CurPos);
			}
		}
		OnPaint();
	}

	// View displacement
	if (_MouseMidDown)
	{
		CRect rect;
		GetClientRect (rect);

		float dx = ((float)(point.x-_LastMousePos.x)) / ((float)rect.Width());
		float dy = -((float)(point.y-_LastMousePos.y)) / ((float)rect.Height());

		_Offset.x -= dx * (_InitViewMax.x-_InitViewMin.x) * _Factor;
		_Offset.y -= dy * (_InitViewMax.y-_InitViewMin.y) * _Factor;

		_LastMousePos = point;
		OnPaint();
	}

	// Display the current position in the world in the status bar
	_MainFrame->displayCoordinates (_CurPos);
}
// ---------------------------------------------------------------------------
void CDisplay::OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 17)
		_CtrlKeyDown = true;
	if (_CtrlKeyDown)
	{
		if (nChar == 90)
		{
			_MainFrame->onMenuModeUndo ();
		}
		if (nChar == 89)
		{
			_MainFrame->onMenuModeRedo ();
		}
	}
}

// ---------------------------------------------------------------------------
void CDisplay::OnKeyUp (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 17)
		_CtrlKeyDown = false;
}