/** \file ps_macro.h
 * Some macros used by the particle systems for rendering.
 *
 * $Id: ps_macro.h,v 1.2 2002/02/28 13:06:02 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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


/// NOTE: for private use, do not include this

#ifndef NL_PS_MACRO_H
#define NL_PS_MACRO_H

#include "nel/misc/types_nl.h"

/// this macro is used to see whether a write in a vertex buffer is correct
#ifdef NL_DEBUG
	#define CHECK_VERTEX_BUFFER(vb, pt) nlassert((uint8 *) (pt) >= (uint8 *) (vb).getVertexCoordPointer()  \
										&& (uint8 *) (pt) < ((uint8 *) (vb).getVertexCoordPointer() + (vb).getVertexSize() * (vb).getNumVertices()));
#else
	#define CHECK_VERTEX_BUFFER
#endif

// this macro check the memory integrity (windows platform for now). It may be useful after violent vb access
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	#include <crtdbg.h>
	#define PARTICLES_CHECK_MEM
//  nlassert(_CrtCheckMemory());    
#else
	#define PARTICLES_CHECK_MEM
#endif

#endif
