/** \file particle_system_process.cpp
 * <File description>
 *
 * $Id: particle_system_process.cpp,v 1.2 2002/02/28 12:59:50 besson Exp $
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

#include "3d/particle_system_process.h"
#include "3d/particle_system.h"

namespace NL3D {

/////////////////////////////////////////////
// CParticleSystemProcess implementation   //
/////////////////////////////////////////////


CFontGenerator *CParticleSystemProcess::getFontGenerator(void)
{
			nlassert(_Owner) ;
			return _Owner->getFontGenerator() ;
}

const CFontGenerator *CParticleSystemProcess::getFontGenerator(void) const 
{
			nlassert(_Owner) ;
			return _Owner->getFontGenerator() ;
}

CFontManager *CParticleSystemProcess::getFontManager(void)
{
			nlassert(_Owner) ;
			return _Owner->getFontManager() ;
}

const CFontManager *CParticleSystemProcess::getFontManager(void) const 
{
			nlassert(_Owner) ;
			return _Owner->getFontManager() ;
}



void CParticleSystemProcess::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	f.serialVersion(1) ;
	f.serialPtr(_Owner) ;
	f.serial(_SystemBasisEnabled) ;	
}


} // NL3D
