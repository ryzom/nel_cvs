/** \file init_3d.cpp
 * 
 *
 * $Id: init_3d.cpp,v 1.1 2001/08/30 10:07:12 corvazier Exp $
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


#include "3d/init_3d.h"
#include <float.h>

namespace NL3D 
{

// Force init
C3dInit initClass;

C3dInit::C3dInit()
{
	// Init for windows
#ifdef NL_OS_WINDOWS

	// Enable FPU exceptions
	_control87 (_EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);

#endif // NL_OS_WINDOWS

}

} // NL3D
