/** \file shader.h
 * <File description>
 *
 * $Id: shader.h,v 1.3 2001/01/08 17:58:29 corvazier Exp $
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

#ifndef NL_SHADER_H
#define NL_SHADER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"


namespace NL3D {

using NLMISC::CRefCount;

/**
 * <Class description>
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// --------------------------------------------------

class IShader : public CRefCount
{
protected:
public:
	// The virtual dtor is important.
	virtual ~IShader() {};

};

} // NL3D


#endif // NL_SHADER_H

/* End of shader.h */
