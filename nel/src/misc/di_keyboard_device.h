/** \file di_keyboard_device.h
 * <File description>
 *
 * $Id: di_keyboard_device.h,v 1.1 2002/03/28 10:30:15 vizerie Exp $
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

#ifndef NL_DI_KEYBOARD_H
#define NL_DI_KEYBOARD_H

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/input_device_server.h"
#include "nel/misc/keyboard_device.h"
#include "nel/misc/di_event_emitter.h"



namespace NLMISC
{

class CWinEventEmitter;

//
struct EDirectInputNoKeyboard : public EDirectInput
{
	EDirectInputNoKeyboard() : EDirectInput("No keyboard found") {}
};



/**
 * Direct Input implementation of a keyboard.
 * \see CDIEventEmitter
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CDIKeyboard : public IKeyboardDevice
{
public:
	bool					ShiftPressed, CtrlPressed, AltPressed;
public:
	///\name Object
	//@{
		/** Create a keyboard device, that must then be deleted by the caller
		  * An optionnal WinEventEmiter can be provided, so that its flags can be in sync
		  * with a win32 keyboard flags (shift, ctrl, and alt)
		  */
		static CDIKeyboard *createKeyboardDevice(IDirectInput8 *di8,
												 HWND hwnd,
												 CDIEventEmitter *diEventEmitter,
												 CWinEventEmitter *we = NULL
												) throw(EDirectInput);
		// dtor
		virtual ~CDIKeyboard();
	//@}

	///\name From IInputDevice
	//@{		
		virtual bool		setBufferSize(uint size);		
		virtual uint		getBufferSize() const;
	//@}

	///\name Keyboard params, from IKeyboardDevice
	//@{				
		virtual bool			needLocale() const { return true; }
		virtual void			setLocale(TKey conversion[NumKeys]) { _LocaleConvTab = conversion; }
		virtual	TKey			*getLocale() const { return _LocaleConvTab; }
	//@}

	TMouseButton	buildKeyboardFlags() const;
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	LPDIRECTINPUTDEVICE8		_Keyboard;		
	uint						_KeyboardBufferSize;	
	bool						_KeyState[NumKeys];	
	TKey				*_LocaleConvTab;
	CWinEventEmitter	*_WE;
	//
	CDIEventEmitter				*_DIEventEmitter;
	static TKey	DIKeyToNelKeyTab[NumKeys];
private:
	/// ctor
	CDIKeyboard(CWinEventEmitter	*we);	
	/** Convert a Direct Input Key to a NeL Key.
	  * The result is then converted through the locale tab if one has been set
	  */
	TKey			DIKeyToNelKey(uint diKey);
	TKeyButton		buildKeyButtonsFlags() const;
	void			keyTriggered(bool pressed, uint key, CEventServer *server);
	TMouseButton	buildKeyboardButtonFlags() const;	
	///\name From IInputDevice
	//@{
		virtual void		poll(CInputDeviceServer *dev);	
		virtual void		submit(IInputDeviceEvent *deviceEvent, CEventServer *server);		
	//@}
};


} // NLMISC


#endif // NL_OS_WINDOWS

#endif // NL_DI_KEYBOARD_H

/* End of di_keyboard.h */
