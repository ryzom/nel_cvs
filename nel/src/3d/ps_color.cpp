/** \file ps_color.cpp
 * <File description>
 *
 * $Id: ps_color.cpp,v 1.8 2002/02/28 12:59:50 besson Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "std3d.h"

#include "3d/ps_color.h"
#include "3d/ps_register_color_attribs.h"


namespace NL3D {

using NLMISC::CRGBA ;

CRGBA CPSColorGradient::_DefaultGradient[] = { CRGBA::White, CRGBA::Black } ;


///======================================================================================
CPSColorGradient::CPSColorGradient(const CRGBA *colorTab, uint32 nbValues, uint32 nbStages, float nbCycles) 
				: CPSValueGradient<CRGBA>(nbCycles)
{
	_F.setValues(colorTab, nbValues, nbStages) ;
}

///======================================================================================
void PSRegisterColorAttribs()
{
	NLMISC_REGISTER_CLASS(CPSColorBlender);
	NLMISC_REGISTER_CLASS(CPSColorMemory);
	NLMISC_REGISTER_CLASS(CPSColorBinOp);
	NLMISC_REGISTER_CLASS(CPSColorBlenderExact);
	NLMISC_REGISTER_CLASS(CPSColorGradient);
}

} // NL3D
