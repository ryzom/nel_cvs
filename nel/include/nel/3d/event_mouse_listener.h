/** \file event_mouse_listener.h
 * <File description>
 *
 * $Id: event_mouse_listener.h,v 1.2 2000/12/06 14:32:24 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_EVENT_MOUSE_LISTENER_H
#define NL_EVENT_MOUSE_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/matrix.h"
#include "nel/3d/viewport.h"
#include "nel/3d/frustum.h"


namespace NL3D 
{


using NLMISC::CVector;
using NLMISC::CMatrix;


/**
 * CEvent3dMouseListener is a listener that handle a 3d matrix with mouse events.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CEvent3dMouseListener : public NLMISC::IEventListener
{
public:
	/**
	  * Mouse mode.
	  * There is two move modes: 3d editor style and NeL style. Default mode is NeL style.
	  * 
	  * (nelStyle) NeL style is:
	  * MouseRotateHotSpot:			CTRL + RIGHTMOUSE
	  * MouseTranslateXYHotSpot:	CTRL + LEFTMOUSE
	  * MouseTranslateZHotSpot:		CTRL + SHIFT + LEFTMOUSE
	  * MouseZoomHotSpot:			ALT + LEFTMOUSE
	  * 
	  * (edit3dStyle) 3d editor style is:
	  * MouseRotateHotSpot:			ALT + MIDDLEMOUSE
	  * MouseTranslateXYHotSpot:	MIDDLEMOUSE
	  * MouseTranslateZHotSpot:		CTRL + MIDDLEMOUSE
	  */
	enum TMouseMode { nelStyle, edit3d };

	/** 
	  * Constructor. 
	  * You should call setViewMatrix + setFrustrum or setCamera, setViewport, setHotStop and setMouseMode to initialize
	  * the whole object. By default, the viewmatrix is identity, the frustrum is (1,1,1,1,1), the hot spot is (0,0,0) 
	  * and the viewport is fullscreen. The mouse mode is set to the NelStyle.
	  */
	CEvent3dMouseListener();

	/// \name Setup

	/** 
	  * Set the current view matrix to use.
	  * \param matrix is the matrix to set.
	  * \see setWithCamera() getViewMatrix()
	  */
	void setViewMatrix (const NLMISC::CMatrix& matrix)
	{
		_ViewMatrix=matrix;
	}

	/** 
	  * Set the current frustrum to use.
	  * \param left is the left border of the frustrum.
	  * \param right is the right border of the frustrum.
	  * \param top is the top border of the frustrum.
	  * \param bottom is the bottom border of the frustrum.
	  * \param depth is the depth coordinate left, right, top and bottom values.
	  * \see setWithCamera()
	  */
	void setFrustrum (float left, float right, float bottom, float top, float depth)
	{
		_Left=left;
		_Right=right;
		_Bottom=bottom;
		_Top=top;
		_Depth=depth;
	}

	/** 
	  * Set the view matrix and the frustrum with a camera.
	  * \param camMat the camera matrix.
	  * \param camFrust the camera frutum.
	  * \see setViewMatrix() setFrustrum()
	  */
	void setWithCamera  (const CMatrix &camMat, const CFrustum &camFrust);

	/** 
	  * Set the viewport in use in the window. By default, the viewport is fullwindow.
	  * \param viewport is the viewport to use. All events outside the viewport are ignored.
	  */
	void setViewport (const NL3D::CViewport& viewport)
	{
		_Viewport=viewport;
	}

	/** 
	  * Set the current hot spot.
	  * \param hotSpot is the target to use when the mouse move. It can be for exemple the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	void setHotSpot (const CVector& hotSpot)
	{
		_HotSpot=hotSpot;
	}

	/** 
	  * Set the mouse mode.
	  * \param mouseMode is the mode you want to use.
	  * \see TMouseMode
	  */
	void setMouseMode(TMouseMode mouseMode)
	{
		_MouseMode=mouseMode;
	}

	/// \name Get

	/**
	  * Get the current view matrix. This matrix is updated with mouse events.
	  * \return The current view matrix.
	  * \see setViewMatrix()
	  */
	const NLMISC::CMatrix& getViewMatrix () const
	{
		return _ViewMatrix;
	}

	/** 
	  * Register the listener to the server.
	  */
	void addToServer (NLMISC::CEventServer& server);

	/** 
	  * Unregister the listener to the server.
	  */
	void removeFromServer (NLMISC::CEventServer& server);

private:
	/// Internal use
	virtual void operator ()(const NLMISC::CEvent& event);

	CMatrix		_ViewMatrix;
	CVector		_HotSpot;
	NL3D::CViewport		_Viewport;
	float		_Top;
	float		_Bottom;
	float		_Left;
	float		_Right;
	float		_Depth;
	bool		_LeftPushed;
	bool		_MiddlePushed;
	bool		_RightPushed;
	float		_X;
	float		_Y;
	TMouseMode	_MouseMode;
}; // NL3D

}

#endif // NL_EVENT_MOUSE_LISTENER_H

/* End of event_mouse_listener.h */
