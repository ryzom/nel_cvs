/** \file event_mouse_listener.cpp
 * <File description>
 *
 * $Id: mouse_listener.cpp,v 1.4 2001/07/18 11:45:46 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <nel/misc/event_server.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_visual_collision_entity.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/quat.h>
#include <nel/misc/plane.h>


#include "client.h"
#include "mouse_listener.h"
#include "entities.h"
#include "camera.h"

using namespace NLMISC;
using namespace NL3D;

float MouseZoomStep;

float GroundCamLimit = 0.5f;

C3dMouseListener::C3dMouseListener() :  _CurrentModelRotationAxis(zAxis),
										_XModelTranslateEnabled(true),
										_YModelTranslateEnabled(true),
										_ZModelTranslateEnabled(true)
{
	_Matrix.identity();
	_ModelMatrix.identity() ;
	_EnableModelMatrixEdition = false ;
	_HotSpot.set (0,0,0);
	_Viewport.initFullScreen();
	_Frustrum.init (2.f, 2.f, -1.f, 1.f);
	_MouseMode=nelStyle;
	setSpeed (10.f);
	_LastTime=CTime::getLocalTime ();

	_ViewLagBehind = 3.0f;
	_ViewHeight = 2.0f;
	_ViewTargetHeight = 2.0f;
	_AimingState = false;

	_X = 0.5f;
	_Y = 0.5f;
	_InvertedMouse = false;
}



void C3dMouseListener::enableModelTranslationAxis(TAxis axis, bool enabled)
{
	switch (axis)
	{
		case xAxis: _XModelTranslateEnabled = enabled ; break ;
		case yAxis: _YModelTranslateEnabled = enabled ; break ;
		case zAxis: _ZModelTranslateEnabled = enabled ; break ;
	}
}

bool C3dMouseListener::isModelTranslationEnabled(TAxis axis)
{
	switch (axis)
	{
		case xAxis: return _XModelTranslateEnabled ; break ;
		case yAxis: return _YModelTranslateEnabled ; break ;
		case zAxis: return _ZModelTranslateEnabled ; break ;
		default: return false ; break ;
	}	
}


void C3dMouseListener::truncateVect(CVector &v)
{
	if (!_XModelTranslateEnabled) v.x = 0.f ;	
	if (!_YModelTranslateEnabled) v.y = 0.f ;	
	if (!_ZModelTranslateEnabled) v.z = 0.f ;	
}


void C3dMouseListener::operator ()(const CEvent& event)
{
	CEventMouse* mouseEvent=(CEventMouse*)&event;
	if (event==EventMouseMoveId) // && mouseEvent->Button&leftButton!=0)
	{
		bool bRotate=false;
		bool bTranslateXY=false;
		bool bTranslateZ=false;
		bool bZoom=false;
		

		// Rotate Axis
		CVector axis;

//		bRotate=(mouseEvent->Button&leftButton)!=0;
		axis=_Matrix.getPos();

		if (!_EnableModelMatrixEdition)
		{							
			// First in the hotSpot
			CMatrix comeFromHotSpot=_Matrix;
			comeFromHotSpot.setPos (axis);

			// Then turn along the Z axis with X mouse
			CMatrix turnZ;
			turnZ.identity();
			turnZ.rotateZ ((float) Pi*2.f*(_X-mouseEvent->X));

			// Then turn along the X axis with Y mouse
			CMatrix turnX;
			turnX.identity();
			if (_InvertedMouse)
				_ViewHeight += 3.0f*(mouseEvent->Y-_Y);
			else
				_ViewHeight -= 3.0f*(mouseEvent->Y-_Y);

			// Then come back from hotspot
			CMatrix goToHotSpot=comeFromHotSpot;
			goToHotSpot.invert();

			// Make the matrix
			CMatrix negPivot, Pivot;
			negPivot.identity();
			negPivot.setPos (-axis);
			Pivot.identity();
			Pivot.setPos (axis);

			// Make this transformation \\//
			//_Matrix=Pivot*turnZ*negPivot*comeFromHotSpot*turnX*goToHotSpot*_Matrix;
			Pivot*=turnZ;
			Pivot*=negPivot;
			Pivot*=comeFromHotSpot;
			Pivot*=turnX;
			Pivot*=goToHotSpot;

			
			Pivot*=_Matrix;
			_Matrix=Pivot;						
			// Normalize, too much transformation could give an ugly matrix..
			_Matrix.normalize (CMatrix::XYZ);			
		
		}
		else
		{
			CVector pos = _ModelMatrix.getPos() ;
			NLMISC::CQuat r ;
			switch (_CurrentModelRotationAxis)
			{
				case xAxis : r = CQuat(CAngleAxis(_ModelMatrix.getI(), (float) Pi*2.f*(_X-mouseEvent->X))) ; break ;
				case yAxis : r = CQuat(CAngleAxis(_ModelMatrix.getJ(), (float) Pi*2.f*(_X-mouseEvent->X))) ; break ;
				case zAxis : r = CQuat(CAngleAxis(_ModelMatrix.getK(), (float) Pi*2.f*(_X-mouseEvent->X))) ; break ;
			} ;

		
			CMatrix rm ;
			rm.rotate(r) ;
			
			_ModelMatrix = rm * _ModelMatrix ;
			_ModelMatrix.setPos(pos) ;

			_ModelMatrix.normalize (CMatrix::XYZ);
		}

		// Update mouse position
		Driver->setMousePos(0.5f, 0.5f);
		_X = 0.5f;
		_Y = 0.5f;
	}
	else if (event==EventMouseDownId)
	{
		// aim
		_AimingState = true;
	}
	else if (event==EventMouseUpId)
	{
		// throw snowball
		_AimingState = false;
		CVector	Direction = getViewDirection();
		shotSnowball(Self->Id, Self->Position+Direction*100.0f);
	}
	else if (event==EventMouseWheelId)
	{
		CEventMouseWheel* mouseEvent=(CEventMouseWheel*)&event;
		_ViewLagBehind += (mouseEvent->Direction? -MouseZoomStep : +MouseZoomStep);
		if (_ViewLagBehind < 0.5f)
			_ViewLagBehind = 0.5f;
	}
}

void C3dMouseListener::addToServer (CEventServer& server)
{
	server.addListener (EventMouseMoveId, this);
	server.addListener (EventMouseDownId, this);
	server.addListener (EventMouseUpId, this);
	server.addListener (EventMouseWheelId, this);
	_AsyncListener.addToServer (server);
}

void C3dMouseListener::removeFromServer (CEventServer& server)
{
	server.removeListener (EventMouseMoveId, this);
	server.removeListener (EventMouseDownId, this);
	server.removeListener (EventMouseUpId, this);
	server.removeListener (EventMouseWheelId, this);
	_AsyncListener.removeFromServer (server);
}

const NLMISC::CMatrix& C3dMouseListener::getViewMatrix ()
{
	// Return the matrix
	return _Matrix;
}

void C3dMouseListener::updateKeys ()
{
	// CVector
	CVector dir (0,0,0);
	bool find=false;

	// Key pushed ?
	if (_AsyncListener.isKeyDown (KeyUP))
	{
		dir+=CVector (0, 1, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyDOWN))
	{
		dir+=CVector (0, -1, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyRIGHT))
	{
		dir+=CVector (1, 0, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyLEFT))
	{
		dir+=CVector (-1, 0, 0);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyNEXT))
	{
		dir+=CVector (0, 0, -1);
		find=true;
	}
	if (_AsyncListener.isKeyDown (KeyPRIOR))
	{
		dir+=CVector (0, 0, 1);
		find=true;
	}

	// key found ?
	if (find)
	{
		// Time elapsed
		uint32 milli=(uint32)(CTime::getLocalTime ()-_LastTime);

		// Speed
		float dPos=_Speed*(float)milli/1000.f;

		// Good direction
		dir.normalize ();
		dir*=dPos;

		// Orientation
		dir=_Matrix.mulVector (dir);

		// New position
		_Matrix.setPos (_Matrix.getPos ()+dir);
	}

	// Last time
	_LastTime=CTime::getLocalTime ();
}


void	C3dMouseListener::setPosition(const CVector &position)
{
	_Matrix.setPos(position);
}

NLMISC::CVector	C3dMouseListener::getPosition() const
{
	return _Matrix.getPos();
}

void	C3dMouseListener::setOrientation(float angle)
{
	/// todo
}

float	C3dMouseListener::getOrientation()
{
	CVector	j = getViewMatrix().getJ();
	j.z = 0.0f;
	j.normalize();
	return (float)atan2(j.y, j.x);
}

CVector	C3dMouseListener::getViewDirection()
{
	float	angle = getOrientation();
	return CVector((float)cos(angle), (float)sin(angle), (2.0f-_ViewHeight)/_ViewLagBehind).normed();
}


void	C3dMouseListener::updateCamera()
{
	CVector	tpos = getPosition()+CVector(0.0f, 0.0f, _ViewTargetHeight);
	CVector	cpos = getPosition()+CVector(-(float)cos(getOrientation())*_ViewLagBehind, -(float)sin(getOrientation())*_ViewLagBehind, _ViewHeight);
	CVector snapped = cpos,
			normal;
	if (CamCollisionEntity->snapToGround(snapped, normal) && (cpos.z-(snapped.z+GroundCamLimit))*normal.z < 0.0f)
	{
		cpos = snapped+CVector(0.0f, 0.0f, GroundCamLimit);
		_ViewHeight = cpos.z - getPosition().z;
	}
	_Camera->lookAt(cpos, tpos);
}

void	cbUpdateMouseListenerConfig(CConfigFile::CVar &var)
{
	if (var.Name == "MouseInvert") MouseListener->setInvertMouseMode(var.asInt() != 0);
	else if (var.Name == "MouseZoomStep") MouseZoomStep = var.asFloat ();
	else nlwarning ("Unknown variable update %s", var.Name.c_str());
}

void	initMouseListenerConfig()
{
	ConfigFile.setCallback ("MouseInvert", cbUpdateMouseListenerConfig);
	ConfigFile.setCallback ("MouseZoomStep", cbUpdateMouseListenerConfig);

	cbUpdateMouseListenerConfig(ConfigFile.getVar ("MouseInvert"));
	cbUpdateMouseListenerConfig(ConfigFile.getVar ("MouseZoomStep"));
}
