/** \file di_keyboard.cpp
 * <File description>
 *
 * $Id: di_keyboard_device.cpp,v 1.1 2002/03/28 10:30:15 vizerie Exp $
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



#include "stdmiSc.h"
#include "misc/di_keyboard_device.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/win_event_emitter.h"
#include <memory>
#include <DX8/dinput.h>


namespace NLMISC 
{

// tab used to do a conversion from DX key code to Nel keys enums

struct CKeyConv
{
	uint DIKey;
	TKey NelKey;
	const char *KeyName;
};

// this is used to build a conversion table
static const CKeyConv DIToNel[] =
{	
	{DIK_1, Key1, "1"},
	{DIK_2, Key2, "2"},
	{DIK_3, Key3, "3"},
	{DIK_4, Key4, "4"},
	{DIK_5, Key5, "5"},
	{DIK_6, Key6, "6"},
	{DIK_7, Key7, "7"},
	{DIK_8, Key8, "8"},
	{DIK_9, Key9, "9"},
	{DIK_0, Key0, "0"},
	//
	{DIK_A, KeyA, "A"},
	{DIK_B, KeyB, "B"},
	{DIK_C, KeyC, "C"},
	{DIK_D, KeyD, "D"},
	{DIK_E, KeyE, "E"},
	{DIK_F, KeyF, "F"},
	{DIK_G, KeyG, "G"},
	{DIK_H, KeyH, "H"},
	{DIK_I, KeyI, "I"},
	{DIK_J, KeyJ, "J"},
	{DIK_K, KeyK, "K"},
	{DIK_L, KeyL, "L"},
	{DIK_M, KeyM, "M"},
	{DIK_N, KeyN, "N"},
	{DIK_O, KeyO, "O"},
	{DIK_P, KeyP, "P"},
	{DIK_Q, KeyQ, "Q"},
	{DIK_R, KeyR, "R"},
	{DIK_S, KeyS, "S"},
	{DIK_T, KeyT, "T"},
	{DIK_U, KeyU, "U"},
	{DIK_V, KeyV, "V"},
	{DIK_W, KeyW, "W"},
	{DIK_X, KeyX, "X"},
	{DIK_Y, KeyY, "Y"},
	{DIK_Z, KeyZ, "Z"},
	//
	{DIK_F1, KeyF1, "F1"},
	{DIK_F2, KeyF2, "F2"},
	{DIK_F3, KeyF3, "F3"},
	{DIK_F4, KeyF4, "F4"},
	{DIK_F5, KeyF5, "F5"},
	{DIK_F6, KeyF6, "F6"},
	{DIK_F7, KeyF7, "F7"},
	{DIK_F8, KeyF8, "F8"},
	{DIK_F9, KeyF9, "F9"},
	{DIK_F10, KeyF10, "F10"},
	{DIK_F11, KeyF11, "F11"},
	{DIK_F12, KeyF12, "F12"},
	{DIK_F13, KeyF13, "F13"},
	{DIK_F14, KeyF14, "F14"},
	{DIK_F15, KeyF15, "F15"},		
	//
	{DIK_NUMPAD0, KeyNUMPAD0, "NUMPAD0"},
	{DIK_NUMPAD1, KeyNUMPAD1, "NUMPAD1"},
	{DIK_NUMPAD2, KeyNUMPAD2, "NUMPAD2"},
	{DIK_NUMPAD3, KeyNUMPAD3, "NUMPAD3"},
	{DIK_NUMPAD4, KeyNUMPAD4, "NUMPAD4"},
	{DIK_NUMPAD5, KeyNUMPAD5, "NUMPAD5"},
	{DIK_NUMPAD6, KeyNUMPAD6, "NUMPAD6"},
	{DIK_NUMPAD7, KeyNUMPAD7, "NUMPAD7"},
	{DIK_NUMPAD8, KeyNUMPAD8, "NUMPAD8"},
	{DIK_NUMPAD9, KeyNUMPAD9, "NUMPAD9"},	
	{DIK_ADD, KeyADD, "+"},	
	{DIK_DECIMAL, KeyDECIMAL, "."},
	{DIK_SUBTRACT, KeySUBTRACT, "-"},
	{DIK_DIVIDE, KeyDIVIDE, "/"},
	{DIK_MULTIPLY, KeyMULTIPLY, "*"},
	//
	{DIK_LSHIFT, KeyLSHIFT, "LEFT SHIFT"},
	{DIK_RSHIFT, KeyRSHIFT, "RIGHT SHIFT"},	
	{DIK_LCONTROL, KeyLCONTROL, "LEFT CONTROL"},
	{DIK_RCONTROL, KeyRCONTROL, "RIGHT CONTROL"},
	{DIK_LMENU, KeyLMENU, "ALT"},
	{DIK_RMENU, KeyRMENU, "ALT GR"},
	//
	{DIK_SPACE, KeySPACE, "SPACE"},
	//
	{DIK_ESCAPE, KeyESCAPE, "ESCAPE"},
	//	
	{DIK_UP, KeyUP, "UP"},
	{DIK_PRIOR, KeyPRIOR, "PRIOR"},
	{DIK_LEFT, KeyLEFT, "LEFT"},
	{DIK_RIGHT, KeyRIGHT, "RIGHT"},
	{DIK_END, KeyEND, "END"},
	{DIK_DOWN, KeyDOWN, "DOWN"},
	{DIK_NEXT, KeyNEXT, "NEXT"},
	{DIK_INSERT, KeyINSERT, "INSERT"},
	{DIK_DELETE, KeyDELETE, "DELETE"},
	{DIK_HOME, KeyHOME, "HOME"},
	{DIK_LWIN, KeyLWIN, "LEFT WIN"},
	{DIK_RWIN, KeyRWIN, "RIGHT WIN"},
	{DIK_APPS, KeyAPPS, "APPS"},
	//
	{DIK_KANJI, KeyKANJI, "KANJI"},
	{DIK_KANA, KeyKANA, "KANA"},
	//
	{DIK_TAB, KeyTAB, "TAB"},
	{DIK_CAPITAL, KeyCAPITAL, "CAPITAL"},
	{DIK_BACK, KeyBACK, "BACK"},
	//
	{DIK_SYSRQ, KeySNAPSHOT, "SNAPSHOT"},
	{DIK_SCROLL, KeySCROLL, "SCROLL"},
	{DIK_PAUSE, KeyPAUSE, "PAUSE"},
	//
	{DIK_NUMLOCK, KeyNUMLOCK, "NUMLOCK"},
	//
	{DIK_RETURN, KeyRETURN, "RETURN"},
	//
	{DIK_NUMPADENTER, KeyRETURN, "ENTER"},
	//
	{DIK_CONVERT, KeyCONVERT, "CONVERT"},
	{DIK_NOCONVERT, KeyNONCONVERT, "NOCONVERT"},

	// other conversions
/*	{DIK_GRAVE, (TKey) 0xDE, "GRAVE"},
	{DIK_LBRACKET, (TKey) 0xDD, "LBRACKET"},
	{DIK_RBRACKET, (TKey) 0xBA, "RBRACKET"},	
	{DIK_APOSTROPHE, (TKey) 0xC0, "APOSTROPHE"},
	{DIK_BACKSLASH, (TKey) 0xDC, "BACKSLASH"},
	{DIK_OEM_102, (TKey) 0xE2, "OEM 102"},
	{DIK_MINUS, (TKey) 0xDB, "MINUS"},
	{DIK_EQUALS, (TKey) 0xBB, "EQUALS"},
	{DIK_COMMA, (TKey) 0xBE, "COMMA"},           
	{DIK_PERIOD, (TKey) 0xBF, "PERIOD"},
	{DIK_SLASH, (TKey) 0xDF, "SLASH"},
	{DIK_SEMICOLON, (TKey) 0x4d, "SEMICOLON"}*/
};


///========================================================================
TKey	CDIKeyboard::DIKeyToNelKeyTab[CDIKeyboard::NumKeys];

///========================================================================
CDIKeyboard::CDIKeyboard(CWinEventEmitter	*we)	
:	_Keyboard(NULL),
	ShiftPressed(false),
	CtrlPressed(false),
	AltPressed(false),
	_LocaleConvTab(NULL),
	_WE(we)
								
{
	std::fill(_KeyState, _KeyState + NumKeys, 0);	
}

///========================================================================
CDIKeyboard::~CDIKeyboard()
{
	if (_Keyboard) 
	{
		_Keyboard->Unacquire();
		_Keyboard->Release();
	}
}

///========================================================================
CDIKeyboard *CDIKeyboard::createKeyboardDevice(IDirectInput8 *di8,
											   HWND hwnd,
											   CDIEventEmitter *diEventEmitter,
											   CWinEventEmitter *we
											  ) throw(EDirectInput)
{
	std::auto_ptr<CDIKeyboard> kb(new CDIKeyboard(we));
	kb->_DIEventEmitter = diEventEmitter;
	HRESULT result = di8->CreateDevice(GUID_SysKeyboard, &kb->_Keyboard, NULL);
	if (result != DI_OK) throw EDirectInputNoKeyboard();
	result = kb->_Keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (result != DI_OK) throw EDirectInputCooperativeLevelFailed();
	result = kb->_Keyboard->SetDataFormat(&c_dfDIKeyboard);
	kb->setBufferSize(16);
	kb->_Keyboard->Acquire();	
	return kb.release();
}

///========================================================================
void CDIKeyboard::poll(CInputDeviceServer *dev)
{
	nlassert(_Keyboard);
	nlassert(_KeyboardBufferSize > 0);
	static std::vector<DIDEVICEOBJECTDATA> datas;
	datas.resize(_KeyboardBufferSize);
	DWORD numElements = _KeyboardBufferSize;
	HRESULT result = _Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
	if (result == DIERR_NOTACQUIRED || result == DIERR_INPUTLOST)
	{
		result = _Keyboard->Acquire();
		if (result != DI_OK) return;
		result = _Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &datas[0], &numElements, 0);
		if (result != DI_OK) return;
	}
	else if (result != DI_OK)
	{	
		return;
	}
		
		 
	for	(uint k = 0; k < numElements; ++k)
	{
		CDIEvent *die = new CDIEvent;
		die->Emitter = this;
		die->Datas = datas[k];
		dev->submitEvent(die);
	}
}


///========================================================================
TKeyButton CDIKeyboard::buildKeyButtonsFlags() const
{
	return (TKeyButton) ( (_KeyState[ShiftPressed] ? shiftKeyButton : 0)
								  | (AltPressed ? altKeyButton : 0)
								  | (CtrlPressed ? ctrlKeyButton : 0)
								);
}

///========================================================================
void CDIKeyboard::keyTriggered(bool pressed, uint key, CEventServer *server)
{
	#if 0
		nlinfo("direct input code = %x", key);
		const uint numPairs = sizeof(DIToNel) / sizeof(CKeyConv);
		for (uint k = 0; k < numPairs; ++k)
		{
			if (DIToNel[k].DIKey == key)
			{
				nlinfo(DIToNel[k].KeyName);
			}
		}
	#endif
	CEventKey *ek;
	TKey nelKey = DIKeyToNelKey(key);
	if (nelKey == KeyCount) return;
	if (pressed) ek = new CEventKeyDown(nelKey, buildKeyButtonsFlags(), _KeyState[key], _DIEventEmitter);
				 else ek = new CEventKeyUp(nelKey, buildKeyButtonsFlags(), _DIEventEmitter);
	server->postEvent(ek);
	_KeyState[key] = pressed;	
}

///========================================================================
void CDIKeyboard::submit(IInputDeviceEvent *deviceEvent, CEventServer *server)
{
	CDIEvent *die = safe_cast<CDIEvent *>(deviceEvent);
	bool pressed = (die->Datas.dwData & 0x80) != 0;
	switch (die->Datas.dwOfs)
	{
		case DIK_LSHIFT:
		case DIK_RSHIFT:
			keyTriggered(pressed, (uint) die->Datas.dwOfs, server);
			ShiftPressed = _KeyState[DIK_LSHIFT] || _KeyState[DIK_RSHIFT];
			//if (_WE) _WE->_ShiftButton = ShiftPressed;			
		break;
		case DIK_RMENU:
		case DIK_LMENU:
			keyTriggered(pressed, (uint) die->Datas.dwOfs, server);
			AltPressed = _KeyState[DIK_LMENU] || _KeyState[DIK_RMENU];
			//if (_WE) _WE->_AltButton = AltPressed;			
		break;
		case DIK_LCONTROL:
		case DIK_RCONTROL:
			keyTriggered(pressed, (uint) die->Datas.dwOfs, server);
			CtrlPressed = _KeyState[DIK_LCONTROL] || _KeyState[DIK_RCONTROL];
			//if (_WE) _WE->_CtrlButton = CtrlPressed;			
		break;		
		default:
			keyTriggered(pressed, (uint) die->Datas.dwOfs, server);
		break;
	}	
}

///========================================================================
TMouseButton	CDIKeyboard::buildKeyboardButtonFlags() const
{
	nlassert(_Keyboard);
	return _DIEventEmitter->buildKeyboardButtonFlags();
}

///========================================================================
bool	CDIKeyboard::setBufferSize(uint size)
{
	nlassert(size > 0);
	nlassert(_Keyboard);
	_Keyboard->Unacquire();	
	DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = size;
	HRESULT					r = _Keyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
	if (r != DI_OK)			return false;	
	_KeyboardBufferSize = size;
	return true;
}

///========================================================================
uint	CDIKeyboard::getBufferSize() const
{
	return _KeyboardBufferSize;
}

///========================================================================
TKey CDIKeyboard::DIKeyToNelKey(uint diKey)
{
	static tableBuilt = false;
	if (!tableBuilt)
	{
		uint k;
		for (k = 0; k < NumKeys; ++k)
		{
			DIKeyToNelKeyTab[k] = KeyCount; // set as not a valid key by default	
		}
		const uint numPairs = sizeof(DIToNel) / sizeof(CKeyConv);
		for (k = 0; k < numPairs; ++k)
		{
			DIKeyToNelKeyTab[DIToNel[k].DIKey] = DIToNel[k].NelKey;
		}
		tableBuilt = true;
	}
	nlassert(diKey < NumKeys);
	return _LocaleConvTab == NULL ? DIKeyToNelKeyTab[diKey]
								  : _LocaleConvTab[DIKeyToNelKeyTab[diKey]];
}
	
} // NLMISC


#endif NL_OS_WINDOWS
