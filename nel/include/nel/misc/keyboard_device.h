/** \file keyboard_device.h
 * <File description>
 *
 * $Id: keyboard_device.h,v 1.1 2002/03/28 10:36:37 vizerie Exp $
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

#ifndef NL_KEYBOARD_DEVICE_H
#define NL_KEYBOARD_DEVICE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/events.h"
#include "nel/misc/input_device.h"



namespace NLMISC 
{

/** Gives access to low level keyboard parameters
  * - 'Shift' messages are replaced by RShift and LShift msg.
  * - 'Control' messages are replaced by 'RControl' and 'LControl' msg.
  * - 'Menu' (alternate) messages are replaced by 'RMenu' and 'LMenu' msg.
  */
struct IKeyboardDevice	: public IInputDevice
{	
	/// Max number of supported keys
	enum { NumKeys = 256 };	
	/** Test wether this device can make locale conversions. 
	  * If not, U.S keyboard keys events are returned unless a conversion is provided by calling setLocale.
	  */
	virtual bool				needLocale() const = 0;
	/** set a conversion table that can convert from a US keyboard to local keyboard
	  * For example, it would map from a QWERTY to a AZERTY keyboard.
	  * NB : the given pointer should valid memory as long as this device is used.
	  * This can be NULL if you don't need conversion
	  */
	virtual void				setLocale(TKey conversion[NumKeys]) = 0;
	// Get the locale conversion table, or NULL if none has been set.
	virtual	TKey				*getLocale() const = 0;
};


} // NLMISC


#endif // NL_KEYBOARD_DEVICE_H

/* End of u_keyboard_device.h */
