/** \file vertex_program.cpp
 * Vertex program definition
 *
 * $Id: vertex_program.cpp,v 1.4.44.1 2007/03/16 11:09:25 legallo Exp $
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

#include "std3d.h"

#include "vertex_program.h"

#include "driver.h"

namespace NL3D 
{

// ***************************************************************************
IVertexProgramDrvInfos::IVertexProgramDrvInfos (IDriver *drv, ItVtxPrgDrvInfoPtrList it) 
{
	_Driver= drv; 
	_DriverIterator= it;
}


// ***************************************************************************
IVertexProgramDrvInfos::~IVertexProgramDrvInfos ()
{
	_Driver->removeVtxPrgDrvInfoPtr (_DriverIterator);
}


// ***************************************************************************
CVertexProgram::CVertexProgram (const char* program/*, bool isEffectPrg*/)
: IProgram(program/*, isEffectPrg*/)
{
}


// ***************************************************************************
CVertexProgram::~CVertexProgram ()
{
	// Must kill the drv mirror of this VB.
	_DrvInfo.kill();
}

// ***************************************************************************
bool CVertexProgram::convertInASM(TEffectParametersMap & params)
{
	if(isEffectProgram())
	{
		return _DrvInfo->convertInASM(this, params);
	}

	nlwarning("This vertex program isn't an effect");
	return false;
}

} // NL3D
