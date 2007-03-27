/** \file vertex_program.h
 * Vertex program definition
 *
 * $Id: vertex_program.h,v 1.3.78.2 2007/03/27 14:01:47 legallo Exp $
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

#ifndef NL_VERTEX_PROGRAM_H
#define NL_VERTEX_PROGRAM_H

#include "program.h" 
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

#include <list>


namespace NL3D {

// List typedef.
class	IDriver;
class	IVertexProgramDrvInfos;
typedef	std::list<IVertexProgramDrvInfos*>	TVtxPrgDrvInfoPtrList;
typedef	TVtxPrgDrvInfoPtrList::iterator		ItVtxPrgDrvInfoPtrList;

// Class for interaction of vertex program with Driver.
// IVertexProgramDrvInfos represent the real data of the vertex program, stored into the driver (eg: just a GLint for opengl).
class IVertexProgramDrvInfos : public NLMISC::CRefCount
{
private:
	IDriver					*_Driver;
	ItVtxPrgDrvInfoPtrList	_DriverIterator;

public:
	IVertexProgramDrvInfos (IDriver *drv, ItVtxPrgDrvInfoPtrList it);
	// The virtual dtor is important.
	virtual ~IVertexProgramDrvInfos(void);
};

/**
 * This class is a vertex program.
 *
 * D3D / OPENGL compatibility notes:
 * ---------------------------------
 *
 * To make your program compatible with D3D and OPENGL nel drivers, please follow thoses directives to write your vertex programs
 *
 * - Use only v[0], v[1] etc.. syntax for input registers. Don't use v0, v1 or v[OPOS] etc..
 * - Use only c[0], c[1] etc.. syntax for constant registers. Don't use c0, c1 etc..
 * - Use only o[HPOS], o[COL0] etc.. syntax for output registers. Don't use oPos, oD0 etc..
 * - Use only uppercase for registers R1, R2 etc.. Don't use lowercase r1, r2 etc..
 * - Use a semicolon to delineate instructions.
 * - Use ARL instruction to load the adress register and not MOV.
 * - Don't use the NOP instruction.
 * - Don't use macros.
 *
 * -> Thoses programs work without any change under OpenGL.
 * -> Direct3D driver implementation will have to modify the syntax on the fly before the setup like this:
 *   - "v[0]" must be changed in "v0" etc..
 *   - "o[HPOS]" must be changed in oPos etc..
 *   - Semicolon must be changed in line return character.
 *   - ARL instruction must be changed in MOV.
 *
 * Behaviour of LOG may change depending on implementation: You can only expect to have dest.z = log2(abs(src.w)).
 * LIT may or may not clamp the specular exponent to [-128, 128] (not done when EXT_vertex_shader is used for example ..) 
 *
 * Depending on the implementation, some optimizations can be achieved by masking the unused output values of instructions
 * as LIT, EXPP ..
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CVertexProgram : public IProgram
{
public:

	/// Constructor
	CVertexProgram (const char* program, bool isEffectPrg = false);

	/// Destructor
	virtual ~CVertexProgram ();

	/// The driver informations. For the driver implementation only.
	NLMISC::CRefPtr<IVertexProgramDrvInfos>		_DrvInfo;

	const char * getASMProfile() { return "vs_1_1"; } ;
};


} // NL3D


#endif // NL_VERTEX_PROGRAM_H

/* End of vertex_program.h */
