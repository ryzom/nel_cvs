/** \file dru.cpp
 * Driver Utilities.
 *
 * $Id: dru.cpp,v 1.7 2000/12/01 15:16:49 corvazier Exp $
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

#include "nel/misc/types_nl.h"

#include "nel/3d/dru.h"
#include "nel/3d/driver.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

namespace NL3D 
{


typedef IDriver* (*IDRV_CREATE_PROC)(void); 


IDriver		*CDRU::createGlDriver()
{
#ifdef NL_OS_WINDOWS

	// WINDOWS code.
	HINSTANCE			hInst;
	IDRV_CREATE_PROC	createDriver;

	hInst=LoadLibrary("driver_opengl.dll");
	if (!hInst)
	{
		nlerror("Can't load driver_opengl.dll");
	}

	createDriver=(IDRV_CREATE_PROC)GetProcAddress(hInst,"NL3D_createIDriverInstance");
	if (!createDriver)
	{
		nlerror("Can't get NL3D_createIDriverInstance from driver_opengl.dll (bad dll?)");
	}

	IDriver		*ret= createDriver();
	if (!ret)
	{
		nlerror("Can't create IDriver Instance from driver_opengl.dll (bad dll?)");
	}
	return ret;

#else // NL_OS_WINDOWS

	return NULL;	// Not yet implemented..

#endif // NL_OS_WINDOWS

}


} // NL3D

