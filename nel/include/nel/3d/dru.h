/** \file dru.h
 * Driver Utilities.
 *
 * $Id: dru.h,v 1.7 2000/12/15 16:59:28 cado Exp $
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

#ifndef NL_DRU_H
#define NL_DRU_H

#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"
#include "nel/3d/viewport.h"


#ifdef NL_DEBUG
#define NL3D_DLL_NAME "nel_drv_opengl_win_debug.dll"
#elif defined (NL_RELEASE)
#define NL3D_DLL_NAME "nel_drv_opengl_win.dll"
#else
#error "Unknown dll name"
#endif

namespace NL3D 
{

/// Exception thrown by CDru::createGlDriver.
class EDru : public Exception
{
};

class EDruOpenglDriverNotFound : public EDru
{
	virtual const char	*what() const throw() {return NL3D_DLL_NAME " is not found.";}
};

class EDruOpenglDriverCorrupted : public EDru
{
	virtual const char	*what() const throw() {return "(Bad dll?) Can't get NL3D_createIDriverInstance from " NL3D_DLL_NAME;}
};

class EDruOpenglDriverOldVersion : public EDru
{
	virtual const char	*what() const throw() {return NL3D_DLL_NAME " is a too old version. Ask for a more recent file.";}
};

class EDruOpenglDriverUnknownVersion : public EDru
{
	virtual const char	*what() const throw() {return NL3D_DLL_NAME " is more recent than the application.";}
};

class EDruOpenglDriverCantCreateDriver : public EDru
{
	virtual const char	*what() const throw() {return NL3D_DLL_NAME " can't create driver.";}
};

/// The driver Utilities class of static.
class	CDRU
{
public:
	
	/// Draw a bitmap 2D. Warning: this is slow...
	static void			drawBitmap (float x, float y, float width, float height, class ITexture& texture, IDriver& driver, CViewport viewport=CViewport());

	/// Draw a line in 2D. Warning: this is slow...
	static void			drawLine (float x0, float y0, float x1, float y1, IDriver& driver, CRGBA col= CRGBA(255,255,255,255), CViewport viewport=CViewport());

	/// Portable Function which create a GL Driver (using gl dll...).
	static IDriver		*createGlDriver() throw(EDru);
};

} // NL3D


#endif // NL_DRU_H

/* End of dru.h */
