/** \file di_mouse.cpp
 * <File description>
 *
 * $Id: di_mouse_device.cpp,v 1.2 2002/04/10 12:41:49 vizerie Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "stdmisc.h"

#include "misc/di_mouse_device.h"
#include "nel/misc/game_device_events.h"


#ifdef NL_OS_WINDOWS

#include <memory>
#include <algorithm>




namespace NLMISC
{




//======================================================
CDIMouse::CDIMouse() : _MessageMode(RawMode),
					   _MouseSpeed(1.0f),
					   _Mouse(NULL),
					   _XAcc(0),
					   _YAcc(0),
					   _XMousePos(0),
					   _YMousePos(0),
					   _LastMouseButtonClicked(-1),
					   _DoubleClickDelay(300),
					   _XFactor(1.f),
					   _YFactor(1.f),
					   OldDIXPos(0),
					   OldDIYPos(0),
					   OldDIZPos(0),
					   _XRaw(0),
					   _YRaw(0),
					   _FirstX(true),
   					   _FirstY(true)

{
	std::fill(_MouseButtons, _MouseButtons + MaxNumMouseButtons, 0);
	std::fill(_MouseAxisMode, _MouseAxisMode + NumMouseAxis, Raw);
	_MouseFrame.setWH(0, 0, 640, 480);	
}

//======================================================
CDIMouse::~CDIMouse()
{
	if (_Mouse)
	{
		_Mouse->Unacquire();
		_Mouse->Release();
	}
}

//======================================================
void	    CDIMouse::setMouseMode(TAxis axis, TAxisMode axisMode)
{	
	nlassert(axisMode < AxisModeLast);
	nlassert(axis < AxisLast);
	_MouseAxisMode[axis] = axisMode;
	clampMouseAxis();
}

//======================================================
CDIMouse::TAxisMode	CDIMouse::getMouseMode(TAxis axis) const
{	
	nlassert(axis < NumMouseAxis);	
	return _MouseAxisMode[axis];
}

//======================================================
void		CDIMouse::setMouseSpeed(float speed)
{
	nlassert(_MessageMode == NormalMode);
	nlassert(speed > 0);
	_MouseSpeed = speed;
}

//======================================================
bool	CDIMouse::setBufferSize(uint size)
{
	nlassert(size > 0);
	nlassert(_Mouse);
	_Mouse->Unacquire();	
	DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = size;
	HRESULT					r = _Mouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
	if (r != DI_OK)	return false;
	_MouseBufferSize = size;
	return true;
}

//======================================================
uint				CDIMouse::getBufferSize() const { return _MouseBufferSize; }

//======================================================
void	CDIMouse::setMousePos(float x, float y)
{	
	nlassert(_MessageMode == NormalMode);
	_XMousePos = (sint32) (x * (1 << 16));
	_YMousePos = (sint32) (y * (1 << 16));
}

//======================================================
CDIMouse *CDIMouse::createMouseDevice(IDirectInput8 *di8, HWND hwnd, CDIEventEmitter *diEventEmitter) throw(EDirectInput)
{
	std::auto_ptr<CDIMouse> mouse(new CDIMouse);
	mouse->_DIEventEmitter = diEventEmitter;
	HRESULT result = di8->CreateDevice(GUID_SysMouse, &(mouse->_Mouse), NULL);
	if (result != DI_OK) throw EDirectInputNoMouse();
	result = mouse->_Mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (result != DI_OK) throw EDirectInputCooperativeLevelFailed();
	mouse->_Mouse->SetDataFormat(&c_dfDIMouse2);	
	mouse->setBufferSize(64);


	/** we want an absolute mouse mode, so that, if the event buffer get full, we can retrieve the right position
	  */
	DIPROPDWORD prop;
	prop.diph.dwSize = sizeof(DIPROPDWORD);
	prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	prop.diph.dwHow = DIPH_DEVICE;
	prop.diph.dwObj = 0;
	prop.dwData = DIPROPAXISMODE_ABS;
	HRESULT r = mouse->_Mouse->SetProperty(DIPROP_AXISMODE, &prop.diph);
	nlassert(r == DI_OK); // should always succeed...
	//
	mouse->_Mouse->Acquire();	
	mouse->_hWnd = hwnd;
	return mouse.release();
}

//======================================================
float				CDIMouse::getMouseSpeed() const 
{	
	nlassert(_MessageMode == NormalMode);
	return _MouseSpeed; 
}

//======================================================
const CRect &CDIMouse::getMouseFrame() const 
{ 
	nlassert(_MessageMode == NormalMode);
	return _MouseFrame; 
}

//======================================================
uint				CDIMouse::getDoubleClickDelay() const { return _DoubleClickDelay; }

//======================================================
inline void	CDIMouse::clampMouseAxis()
{
	if (_MouseAxisMode[XAxis] == Clamped) clamp(_XMousePos, (sint32) _MouseFrame.X  << 16, (sint32) (_MouseFrame.X + _MouseFrame.Width - 1) << 16);
	if (_MouseAxisMode[YAxis] == Clamped) clamp(_YMousePos, (sint32) _MouseFrame.Y << 16, (sint32) (_MouseFrame.X + _MouseFrame.Height - 1) << 16);	
}

//======================================================
void CDIMouse::poll(CInputDeviceServer *dev)
{
	nlassert(_Mouse);
	nlassert(_MouseBufferSize > 0);
	static std::vector<DIDEVICEOBJECTDATA> datas;
	datas.resize(_MouseBufferSize);
	DWORD numElements = _MouseBufferSize;	
	HRESULT result = _Mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
	if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST)
	{		
		result = _Mouse->Acquire();
		HRESULT result = _Mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
		if (result != DI_OK) return;
	}
	else if (result != DI_OK) return;
		 
	if (::IsWindowEnabled(_hWnd) && ::IsWindowVisible(_hWnd))
	{
		for(uint k = 0; k < numElements; ++k)
		{
			CDIEvent *die = new CDIEvent;
			die->Emitter = this;
			die->Datas = datas[k];
			dev->submitEvent(die);
		}
	}
}


//======================================================================
TMouseButton CDIMouse::buildMouseButtonFlags() const
{
	return (TMouseButton) (
						   _DIEventEmitter->buildKeyboardButtonFlags()
						   | (_MouseButtons[0] ? leftButton   : 0)
						   | (_MouseButtons[1] ? rightButton  : 0)
						   | (_MouseButtons[2] ? middleButton : 0)
						  );
}

//======================================================
TMouseButton CDIMouse::buildMouseSingleButtonFlags(uint button)
{
	static const TMouseButton mb[] = { leftButton, rightButton, middleButton };
	nlassert(button < MaxNumMouseButtons);
	return (TMouseButton) (_DIEventEmitter->buildKeyboardButtonFlags() | mb[button]);						   						  
}

//======================================================
void CDIMouse::onButtonClicked(uint button, CEventServer *server, uint32 date)
{
	// check for double click
	if (_LastMouseButtonClicked == (sint) button)
	{
		if (date - _MouseButtonsLastClickDate < _DoubleClickDelay)
		{
			CEventMouseDblClk *emdc 
			= new CEventMouseDblClk((float) (_XMousePos >> 16),
										    (float) (_YMousePos >> 16),
										    buildMouseSingleButtonFlags(button),
										    _DIEventEmitter);
			server->postEvent(emdc);
			_LastMouseButtonClicked = -1;
		}
		else
		{
			_MouseButtonsLastClickDate = date;
		}
	}
	else
	{
		_LastMouseButtonClicked = button;
		_MouseButtonsLastClickDate = date;
	}
}

//======================================================
void CDIMouse::processButton(uint button, bool pressed, CEventServer *server, uint32 date)
{
	updateMove(server);
	if (pressed)
	{
		CEventMouseDown *emd = 
		new CEventMouseDown((float) (_XMousePos >> 16),
								    (float) (_YMousePos >> 16),
								    buildMouseSingleButtonFlags(button),
								    _DIEventEmitter);
		server->postEvent(emd);
	}
	else
	{
		CEventMouseUp *emu = 
		new CEventMouseUp((float) (_XMousePos >> 16),
								  (float) (_YMousePos >> 16),
								  buildMouseSingleButtonFlags(button),
								  _DIEventEmitter);
		server->postEvent(emu);
		onButtonClicked(button, server, date);
	}
	_MouseButtons[button] = pressed;
}

//======================================================
void CDIMouse::submit(IInputDeviceEvent *deviceEvent, CEventServer *server)
{
	CDIEvent *die = safe_cast<CDIEvent *>(deviceEvent);
	bool	pressed;
	switch(die->Datas.dwOfs)
	{
		case	DIMOFS_X:
		{
			if (!_FirstX)
			{
				sint dep = (sint32) die->Datas.dwData - OldDIXPos;				
				_XAcc += dep;				
			}
			else
			{
				_FirstX = false;
			}
			OldDIXPos = (sint32) die->Datas.dwData;
		}
		break;
		case	DIMOFS_Y:
		{
			if (!_FirstY)
			{
				sint dep = (sint32) die->Datas.dwData - OldDIYPos;				
				_YAcc -= dep;				
			}
			else
			{	
				_FirstY = false;
			}
			OldDIYPos = (sint32) die->Datas.dwData;
		}
		break;
		case	DIMOFS_Z:
		{
			updateMove(server);
			sint dep = die->Datas.dwData - OldDIZPos;
			OldDIZPos = (sint32) die->Datas.dwData;
			CEventMouseWheel *emw = 
			new CEventMouseWheel((float) (_XMousePos >> 16),
										 (float) (_XMousePos >> 16),										 
										 buildMouseButtonFlags(),
										 dep > 0,
										 _DIEventEmitter);
			server->postEvent(emw);
		}
		break;
		case	DIMOFS_BUTTON0:	/* left button */			
			pressed = (die->Datas.dwData & 0x80) != 0;
			processButton(0, pressed, server, die->Datas.dwTimeStamp);
		break;
		case	DIMOFS_BUTTON1: /* right button */
			pressed = (die->Datas.dwData & 0x80) != 0;
			processButton(1, pressed, server, die->Datas.dwTimeStamp);
		break;
		case	DIMOFS_BUTTON2: /* middle button */
			pressed = (die->Datas.dwData & 0x80) != 0;
			processButton(2, pressed, server, die->Datas.dwTimeStamp);
		break;
		default:
			return;
		break;
	}
}

//======================================================
void	CDIMouse::updateMove(CEventServer *server)
{
	if (_XAcc != 0 || _YAcc != 0)
	{
		if (_MessageMode == NormalMode)
		{
			_XMousePos += (sint32) (_MouseSpeed * _XAcc * (1 << 16)); 
			_YMousePos += (sint32) (_MouseSpeed * _YAcc * (1 << 16)); 			
			clampMouseAxis();
			CEventMouseMove *emm = new CEventMouseMove(_XFactor * (float) (_XMousePos >> 16), _YFactor *(float) (_YMousePos >> 16), buildMouseButtonFlags(), _DIEventEmitter);
			server->postEvent(emm);
		}
		else
		{			
			_XRaw += _XAcc;
			_YRaw += _YAcc;
			CGDMouseMove *emm = new CGDMouseMove(_DIEventEmitter, this, _XRaw, _YRaw);
			server->postEvent(emm);			
		}
		_XAcc = _YAcc = 0;
	}
}

//======================================================
void CDIMouse::transitionOccured(CEventServer *server, const IInputDeviceEvent *)
{
	updateMove(server);	
}

//======================================================
void	CDIMouse::setButton(uint button, bool pushed)
{
	nlassert(button < MaxNumMouseButtons);
	_MouseButtons[button] = pushed;
}

//======================================================
bool	CDIMouse::getButton(uint button) const
{
	nlassert(button < MaxNumMouseButtons);
	return _MouseButtons[button];
}

//======================================================
void	CDIMouse::setDoubleClickDelay(uint ms)
{
	nlassert(ms > 0);
	_DoubleClickDelay = ms;
}

//======================================================
void	CDIMouse::setMouseFrame(const CRect &rect)
{
	nlassert(_MessageMode == NormalMode);
	_MouseFrame = rect;
}

//======================================================
void	CDIMouse::setMessagesMode(TMessageMode mode)
{ 
	nlassert(mode < AxisModeLast); 
	_MessageMode = mode;	
	if (mode == RawMode)
	{
		_FirstX = _FirstY = false;
	}	
}


} // NLMISC

#endif // NL_OS_WINDOWS


