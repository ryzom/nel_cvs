/** \file events.h
 * Events
 *
 * $Id: events.h,v 1.5 2000/11/13 10:02:25 corvazier Exp $
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

#ifndef NL_EVENTS_H
#define NL_EVENTS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include <map>
#include <list>

namespace NLMISC {

/*===================================================================*/

/**
 * CEvent
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEvent : public CClassId
{
protected:
	CEvent (const CClassId& classId) : CClassId (classId)
	{
	}
};

const CClassId EventKeyDownId (0x3c2643da, 0x43f802a1);
const CClassId EventKeyUpId (0x1e62e85, 0x68a35d46);
const CClassId EventCharId (0x552255fe, 0x75a2373f);
const CClassId EventMouseDownId (0x35b7878, 0x5d4a0f86);
const CClassId EventMouseUpId (0xcce1f7e, 0x7ed344d7);
const CClassId EventActivateId (0x7da66b0a, 0x1ef74519);
enum TKey 
{
	Key0				='0',
	Key1				='1',
	Key2				='2',
	Key3				='3',
	Key4				='4',
	Key5				='5',
	Key6				='6',
	Key7				='7',
	Key8				='8',
	Key9				='9',
	KeyA				='A',
	KeyB				='B',
	KeyC				='C',
	KeyD				='D',
	KeyE				='E',
	KeyF				='F',
	KeyG				='G',
	KeyH				='H',
	KeyI				='I',
	KeyJ				='J',
	KeyK				='K',
	KeyL				='L',
	KeyM				='M',
	KeyN				='N',
	KeyO				='O',
	KeyP				='P',
	KeyQ				='Q',
	KeyR				='R',
	KeyS				='S',
	KeyT				='T',
	KeyU				='U',
	KeyV				='V',
	KeyW				='W',
	KeyX				='X',
	KeyY				='Y',
	KeyZ				='Z',
	KeyLBUTTON        =0x01,
	KeyRBUTTON        =0x02,
	KeyCANCEL         =0x03,
	KeyMBUTTON        =0x04,    /* NOT contiguous with L & RBUTTON */
	KeyBACK           =0x08,
	KeyTAB            =0x09,
	KeyCLEAR          =0x0C,
	KeyRETURN         =0x0D,
	KeySHIFT          =0x10,
	KeyCONTROL        =0x11,
	KeyMENU           =0x12,
	KeyPAUSE          =0x13,
	KeyCAPITAL        =0x14,
	KeyKANA           =0x15,
	KeyHANGEUL        =0x15,	  /* old name - should be here for compatibility */
	KeyHANGUL         =0x15,
	KeyJUNJA          =0x17,
	KeyFINAL          =0x18,
	KeyHANJA          =0x19,
	KeyKANJI          =0x19,
	KeyESCAPE         =0x1B,
	KeyCONVERT        =0x1C,
	KeyNONCONVERT     =0x1D,
	KeyACCEPT         =0x1E,
	KeyMODECHANGE     =0x1F,
	KeySPACE          =0x20,
	KeyPRIOR          =0x21,
	KeyNEXT           =0x22,
	KeyEND            =0x23,
	KeyHOME           =0x24,
	KeyLEFT           =0x25,
	KeyUP             =0x26,
	KeyRIGHT          =0x27,
	KeyDOWN           =0x28,
	KeySELECT         =0x29,
	KeyPRINT          =0x2A,
	KeyEXECUTE        =0x2B,
	KeySNAPSHOT       =0x2C,
	KeyINSERT         =0x2D,
	KeyDELETE         =0x2E,
	KeyHELP           =0x2F,
	KeyLWIN           =0x5B,
	KeyRWIN           =0x5C,
	KeyAPPS           =0x5D,
	KeyNUMPAD0        =0x60,
	KeyNUMPAD1        =0x61,
	KeyNUMPAD2        =0x62,
	KeyNUMPAD3        =0x63,
	KeyNUMPAD4        =0x64,
	KeyNUMPAD5        =0x65,
	KeyNUMPAD6        =0x66,
	KeyNUMPAD7        =0x67,
	KeyNUMPAD8        =0x68,
	KeyNUMPAD9        =0x69,
	KeyMULTIPLY       =0x6A,
	KeyADD            =0x6B,
	KeySEPARATOR      =0x6C,
	KeySUBTRACT       =0x6D,
	KeyDECIMAL        =0x6E,
	KeyDIVIDE         =0x6F,
	KeyF1             =0x70,
	KeyF2             =0x71,
	KeyF3             =0x72,
	KeyF4             =0x73,
	KeyF5             =0x74,
	KeyF6             =0x75,
	KeyF7             =0x76,
	KeyF8             =0x77,
	KeyF9             =0x78,
	KeyF10            =0x79,
	KeyF11            =0x7A,
	KeyF12            =0x7B,
	KeyF13            =0x7C,
	KeyF14            =0x7D,
	KeyF15            =0x7E,
	KeyF16            =0x7F,
	KeyF17            =0x80,
	KeyF18            =0x81,
	KeyF19            =0x82,
	KeyF20            =0x83,
	KeyF21            =0x84,
	KeyF22            =0x85,
	KeyF23            =0x86,
	KeyF24            =0x87,
	KeyNUMLOCK        =0x90,
	KeySCROLL         =0x91,
	KeyLSHIFT         =0xA0,
	KeyRSHIFT         =0xA1,
	KeyLCONTROL       =0xA2,
	KeyRCONTROL       =0xA3,
	KeyLMENU          =0xA4,
	KeyRMENU          =0xA5,
	KeyPROCESSKEY     =0xE5,
	KeyATTN           =0xF6,
	KeyCRSEL          =0xF7,
	KeyEXSEL          =0xF8,
	KeyEREOF          =0xF9,
	KeyPLAY           =0xFA,
	KeyZOOM           =0xFB,
	KeyNONAME         =0xFC,
	KeyPA1            =0xFD,
	KeyOEM_CLEAR      =0xFE,
	KeyCount          =0xFF
};

/**
 * CEventKeyDown
 */
class CEventKeyDown : public CEvent
{
public:
	CEventKeyDown (TKey key) : CEvent (EventKeyDownId)
	{
		Key=key;
	}
	TKey Key;
};

/**
 * CEventKeyUp
 */
class CEventKeyUp : public CEvent
{
public:
	CEventKeyUp (TKey key) : CEvent (EventKeyUpId)
	{
		Key=key;
	}
	TKey Key;
};

/**
 * CEventChar
 */
class CEventChar : public CEvent
{
public:
	CEventChar (ucchar c) : CEvent (EventCharId)
	{
		Char=c;
	}
	ucchar Char;
};


/**
 * CEventMouseDown
 */
class CEventMouseDown : public CEvent
{
public:
	uint X,Y;

	CEventMouseDown (uint x, uint y) : CEvent (EventMouseDownId)
	{
		X = x;
		Y = y;
	}
	
};


/**
 * CEventMouseUp
 */
class CEventMouseUp : public CEvent
{
public:
	uint X,Y;

	CEventMouseUp (uint x, uint y) : CEvent (EventMouseUpId)
	{
		X = x;
		Y = y;
	}	
};

/**
 * CEventActivate. Called when window is actived / disactived.
 */
class CEventActivate : public CEvent
{
public:
	/**
	  * True if window is actived, false if it is disactived.
	  */
	bool Activate;

	/**
	  * Create a activate message. Notify the activation disactivation of a window.
	  * \param activate is True if window is actived, false if it is disactived.
	  */
	CEventActivate (bool activate) : CEvent (EventActivateId)
	{
		Activate = activate;
	}
};

} // NLMISC


#endif // NL_EVENTS_H

/* End of events.h */