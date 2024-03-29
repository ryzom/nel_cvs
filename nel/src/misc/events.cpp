/** \file events.cpp
 * Events
 *
 * $Id: events.cpp,v 1.4 2004/03/12 16:42:06 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/events.h"
#include "nel/misc/string_conversion.h"


namespace NLMISC {


// ***************************************************************************
// The conversion table
static const CStringConversion<TKey>::CPair stringTable [] =
{ 
	// BUFFER
	{ "Key0", Key0 },
	{ "Key1", Key1 },
	{ "Key2", Key2 },
	{ "Key3", Key3 },
	{ "Key4", Key4 },
	{ "Key5", Key5 },
	{ "Key6", Key6 },
	{ "Key7", Key7 },
	{ "Key8", Key8 },
	{ "Key9", Key9 },
	{ "KeyA", KeyA },
	{ "KeyB", KeyB },
	{ "KeyC", KeyC },
	{ "KeyD", KeyD },
	{ "KeyE", KeyE },
	{ "KeyF", KeyF },
	{ "KeyG", KeyG },
	{ "KeyH", KeyH },
	{ "KeyI", KeyI },
	{ "KeyJ", KeyJ },
	{ "KeyK", KeyK },
	{ "KeyL", KeyL },
	{ "KeyM", KeyM },
	{ "KeyN", KeyN },
	{ "KeyO", KeyO },
	{ "KeyP", KeyP },
	{ "KeyQ", KeyQ },
	{ "KeyR", KeyR },
	{ "KeyS", KeyS },
	{ "KeyT", KeyT },
	{ "KeyU", KeyU },
	{ "KeyV", KeyV },
	{ "KeyW", KeyW },
	{ "KeyX", KeyX },
	{ "KeyY", KeyY },
	{ "KeyZ", KeyZ },
	{ "KeyLBUTTON", KeyLBUTTON },
	{ "KeyRBUTTON", KeyRBUTTON },
	{ "KeyCANCEL", KeyCANCEL },
	{ "KeyMBUTTON", KeyMBUTTON },
	{ "KeyBACK", KeyBACK },
	{ "KeyTAB", KeyTAB },
	{ "KeyCLEAR", KeyCLEAR },
	{ "KeyRETURN", KeyRETURN },
	{ "KeySHIFT", KeySHIFT },
	{ "KeyCONTROL", KeyCONTROL },
	{ "KeyMENU", KeyMENU },
	{ "KeyPAUSE", KeyPAUSE },
	{ "KeyCAPITAL", KeyCAPITAL },
	{ "KeyKANA", KeyKANA },
	{ "KeyHANGEUL", KeyHANGEUL },
	{ "KeyHANGUL", KeyHANGUL },
	{ "KeyJUNJA", KeyJUNJA },
	{ "KeyFINAL", KeyFINAL },
	{ "KeyHANJA", KeyHANJA },
	{ "KeyKANJI", KeyKANJI },
	{ "KeyESCAPE", KeyESCAPE },
	{ "KeyCONVERT", KeyCONVERT },
	{ "KeyNONCONVERT", KeyNONCONVERT },
	{ "KeyACCEPT", KeyACCEPT },
	{ "KeyMODECHANGE", KeyMODECHANGE },
	{ "KeySPACE", KeySPACE },
	{ "KeyPRIOR", KeyPRIOR },
	{ "KeyNEXT", KeyNEXT },
	{ "KeyEND", KeyEND },
	{ "KeyHOME", KeyHOME },
	{ "KeyLEFT", KeyLEFT },
	{ "KeyUP", KeyUP },
	{ "KeyRIGHT", KeyRIGHT },
	{ "KeyDOWN", KeyDOWN },
	{ "KeySELECT", KeySELECT },
	{ "KeyPRINT", KeyPRINT },
	{ "KeyEXECUTE", KeyEXECUTE },
	{ "KeySNAPSHOT", KeySNAPSHOT },
	{ "KeyINSERT", KeyINSERT },
	{ "KeyDELETE", KeyDELETE },
	{ "KeyHELP", KeyHELP },
	{ "KeyLWIN", KeyLWIN },
	{ "KeyRWIN", KeyRWIN },
	{ "KeyAPPS", KeyAPPS },
	{ "KeyNUMPAD0", KeyNUMPAD0 },
	{ "KeyNUMPAD1", KeyNUMPAD1 },
	{ "KeyNUMPAD2", KeyNUMPAD2 },
	{ "KeyNUMPAD3", KeyNUMPAD3 },
	{ "KeyNUMPAD4", KeyNUMPAD4 },
	{ "KeyNUMPAD5", KeyNUMPAD5 },
	{ "KeyNUMPAD6", KeyNUMPAD6 },
	{ "KeyNUMPAD7", KeyNUMPAD7 },
	{ "KeyNUMPAD8", KeyNUMPAD8 },
	{ "KeyNUMPAD9", KeyNUMPAD9 },
	{ "KeyMULTIPLY", KeyMULTIPLY },
	{ "KeyADD", KeyADD },
	{ "KeySEPARATOR", KeySEPARATOR },
	{ "KeySUBTRACT", KeySUBTRACT },
	{ "KeyDECIMAL", KeyDECIMAL },
	{ "KeyDIVIDE", KeyDIVIDE },
	{ "KeyF1", KeyF1 },
	{ "KeyF2", KeyF2 },
	{ "KeyF3", KeyF3 },
	{ "KeyF4", KeyF4 },
	{ "KeyF5", KeyF5 },
	{ "KeyF6", KeyF6 },
	{ "KeyF7", KeyF7 },
	{ "KeyF8", KeyF8 },
	{ "KeyF9", KeyF9 },
	{ "KeyF10", KeyF10 },
	{ "KeyF11", KeyF11 },
	{ "KeyF12", KeyF12 },
	{ "KeyF13", KeyF13 },
	{ "KeyF14", KeyF14 },
	{ "KeyF15", KeyF15 },
	{ "KeyF16", KeyF16 },
	{ "KeyF17", KeyF17 },
	{ "KeyF18", KeyF18 },
	{ "KeyF19", KeyF19 },
	{ "KeyF20", KeyF20 },
	{ "KeyF21", KeyF21 },
	{ "KeyF22", KeyF22 },
	{ "KeyF23", KeyF23 },
	{ "KeyF24", KeyF24 },
	{ "KeyNUMLOCK", KeyNUMLOCK },
	{ "KeySCROLL", KeySCROLL },
	{ "KeyLSHIFT", KeyLSHIFT },
	{ "KeyRSHIFT", KeyRSHIFT },
	{ "KeyLCONTROL", KeyLCONTROL },
	{ "KeyRCONTROL", KeyRCONTROL },
	{ "KeyLMENU", KeyLMENU },
	{ "KeyRMENU", KeyRMENU },
	{ "KeySEMICOLON", KeySEMICOLON },
	{ "KeyEQUALS", KeyEQUALS },
	{ "KeyCOMMA", KeyCOMMA },
	{ "KeyDASH", KeyDASH },
	{ "KeyPERIOD", KeyPERIOD },
	{ "KeySLASH", KeySLASH },
	{ "KeyTILDE", KeyTILDE },
	{ "KeyLBRACKET", KeyLBRACKET },
	{ "KeyBACKSLASH", KeyBACKSLASH },
	{ "KeyRBRACKET", KeyRBRACKET },
	{ "KeyAPOSTROPHE", KeyAPOSTROPHE },
	{ "KeyPROCESSKEY", KeyPROCESSKEY },
	{ "KeyATTN", KeyATTN },
	{ "KeyCRSEL", KeyCRSEL },
	{ "KeyEXSEL", KeyEXSEL },
	{ "KeyEREOF", KeyEREOF },
	{ "KeyPLAY", KeyPLAY },
	{ "KeyZOOM", KeyZOOM },
	{ "KeyNONAME", KeyNONAME },
	{ "KeyPA1", KeyPA1 },
	{ "KeyOEM_CLEAR", KeyOEM_CLEAR },
};


static	CStringConversion<TKey> KeyConversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  KeyCount);


// ***************************************************************************
TKey				CEventKey::getKeyFromString(const std::string &str)
{
	return KeyConversion.fromString(str);
}

// ***************************************************************************
const std::string	&CEventKey::getStringFromKey(TKey k)
{
	return KeyConversion.toString(k);
}



} // NLMISC
