/** \file dru.h
 * Driver Utilities.
 *
 * $Id: dru.h,v 1.4 2000/12/04 10:13:09 corvazier Exp $
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


namespace NL3D 
{

/// Exception thrown by CDru::createGlDriver.
class EDru : public Exception
{
};

class EDruOpenglDriverNotFound : public EDru
{
	virtual const char	*what() const throw() {return "driver_opengl.dll is missing found.";}
};

class EDruOpenglDriverCorrupted : public EDru
{
	virtual const char	*what() const throw() {return "Can't get NL3D_createIDriverInstance from driver_opengl.dll (bad dll?)";}
};

class EDruOpenglDriverOldVersion : public EDru
{
	virtual const char	*what() const throw() {return "driver_opengl.dll is a too old version. Ask for a more recent file.";}
};

class EDruOpenglDriverUnknownVersion : public EDru
{
	virtual const char	*what() const throw() {return "driver_opengl.dll is more recent than the application.";}
};

class EDruOpenglDriverCantCreateDriver : public EDru
{
	virtual const char	*what() const throw() {return "driver_opengl.dll can't create driver.";}
};

/// The driver Utilities class of static.
class	CDRU
{
public:
	
	static void			drawBitmap (float x, float y, float width, float height, class ITexture& texture, IDriver& driver, CViewport viewport=CViewport());

	/// Portable Function which create a GL Driver (using gl dll...).
	static IDriver		*createGlDriver() throw(EDru);
};

} // NL3D


#endif // NL_DRU_H

/* End of dru.h */
