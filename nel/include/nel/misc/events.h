/** \file events.h
 * Events
 *
 * $Id: events.h,v 1.1 2000/11/09 16:17:03 coutelas Exp $
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

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

const uint Key0				='0';
const uint Key1				='1';
const uint Key2				='2';
const uint Key3				='3';
const uint Key4				='4';
const uint Key5				='5';
const uint Key6				='6';
const uint Key7				='7';
const uint Key8				='8';
const uint Key9				='9';
const uint KeyA				='A';
const uint KeyB				='B';
const uint KeyC				='C';
const uint KeyD				='D';
const uint KeyE				='E';
const uint KeyF				='F';
const uint KeyG				='G';
const uint KeyH				='H';
const uint KeyI				='I';
const uint KeyJ				='J';
const uint KeyK				='K';
const uint KeyL				='L';
const uint KeyM				='M';
const uint KeyN				='N';
const uint KeyO				='O';
const uint KeyP				='P';
const uint KeyQ				='Q';
const uint KeyR				='R';
const uint KeyS				='S';
const uint KeyT				='T';
const uint KeyU				='U';
const uint KeyV				='V';
const uint KeyW				='W';
const uint KeyX				='X';
const uint KeyY				='Y';
const uint KeyZ				='Z';
const uint KeyLBUTTON        =0x01;
const uint KeyRBUTTON        =0x02;
const uint KeyCANCEL         =0x03;
const uint KeyMBUTTON        =0x04;    /* NOT contiguous with L & RBUTTON */
const uint KeyBACK           =0x08;
const uint KeyTAB            =0x09;
const uint KeyCLEAR          =0x0C;
const uint KeyRETURN         =0x0D;
const uint KeySHIFT          =0x10;
const uint KeyCONTROL        =0x11;
const uint KeyMENU           =0x12;
const uint KeyPAUSE          =0x13;
const uint KeyCAPITAL        =0x14;
const uint KeyKANA           =0x15;
const uint KeyHANGEUL        =0x15;	  /* old name - should be here for compatibility */
const uint KeyHANGUL         =0x15;
const uint KeyJUNJA          =0x17;
const uint KeyFINAL          =0x18;
const uint KeyHANJA          =0x19;
const uint KeyKANJI          =0x19;
const uint KeyESCAPE         =0x1B;
const uint KeyCONVERT        =0x1C;
const uint KeyNONCONVERT     =0x1D;
const uint KeyACCEPT         =0x1E;
const uint KeyMODECHANGE     =0x1F;
const uint KeySPACE          =0x20;
const uint KeyPRIOR          =0x21;
const uint KeyNEXT           =0x22;
const uint KeyEND            =0x23;
const uint KeyHOME           =0x24;
const uint KeyLEFT           =0x25;
const uint KeyUP             =0x26;
const uint KeyRIGHT          =0x27;
const uint KeyDOWN           =0x28;
const uint KeySELECT         =0x29;
const uint KeyPRINT          =0x2A;
const uint KeyEXECUTE        =0x2B;
const uint KeySNAPSHOT       =0x2C;
const uint KeyINSERT         =0x2D;
const uint KeyDELETE         =0x2E;
const uint KeyHELP           =0x2F;
const uint KeyLWIN           =0x5B;
const uint KeyRWIN           =0x5C;
const uint KeyAPPS           =0x5D;
const uint KeyNUMPAD0        =0x60;
const uint KeyNUMPAD1        =0x61;
const uint KeyNUMPAD2        =0x62;
const uint KeyNUMPAD3        =0x63;
const uint KeyNUMPAD4        =0x64;
const uint KeyNUMPAD5        =0x65;
const uint KeyNUMPAD6        =0x66;
const uint KeyNUMPAD7        =0x67;
const uint KeyNUMPAD8        =0x68;
const uint KeyNUMPAD9        =0x69;
const uint KeyMULTIPLY       =0x6A;
const uint KeyADD            =0x6B;
const uint KeySEPARATOR      =0x6C;
const uint KeySUBTRACT       =0x6D;
const uint KeyDECIMAL        =0x6E;
const uint KeyDIVIDE         =0x6F;
const uint KeyF1             =0x70;
const uint KeyF2             =0x71;
const uint KeyF3             =0x72;
const uint KeyF4             =0x73;
const uint KeyF5             =0x74;
const uint KeyF6             =0x75;
const uint KeyF7             =0x76;
const uint KeyF8             =0x77;
const uint KeyF9             =0x78;
const uint KeyF10            =0x79;
const uint KeyF11            =0x7A;
const uint KeyF12            =0x7B;
const uint KeyF13            =0x7C;
const uint KeyF14            =0x7D;
const uint KeyF15            =0x7E;
const uint KeyF16            =0x7F;
const uint KeyF17            =0x80;
const uint KeyF18            =0x81;
const uint KeyF19            =0x82;
const uint KeyF20            =0x83;
const uint KeyF21            =0x84;
const uint KeyF22            =0x85;
const uint KeyF23            =0x86;
const uint KeyF24            =0x87;
const uint KeyNUMLOCK        =0x90;
const uint KeySCROLL         =0x91;
const uint VK_LSHIFT         =0xA0;
const uint VK_RSHIFT         =0xA1;
const uint VK_LCONTROL       =0xA2;
const uint VK_RCONTROL       =0xA3;
const uint VK_LMENU          =0xA4;
const uint VK_RMENU          =0xA5;
const uint VK_PROCESSKEY     =0xE5;
const uint VK_ATTN           =0xF6;
const uint VK_CRSEL          =0xF7;
const uint VK_EXSEL          =0xF8;
const uint VK_EREOF          =0xF9;
const uint VK_PLAY           =0xFA;
const uint VK_ZOOM           =0xFB;
const uint VK_NONAME         =0xFC;
const uint VK_PA1            =0xFD;
const uint VK_OEM_CLEAR      =0xFE;

/**
 * CEvent
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventKeyDown : public CEvent
{
public:
	CEventKeyDown (uint key) : CEvent (EventKeyDownId)
	{
		Key=key;
	}
	uint Key;
};




} // NLMISC


#endif // NL_EVENTS_H

/* End of events.h */