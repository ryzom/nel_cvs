/** \file patch_rdr_pass.cpp
 * <File description>
 *
 * $Id: patch_rdr_pass.cpp,v 1.6 2001/02/20 11:05:05 berenguier Exp $
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

#include "nel/3d/patch_rdr_pass.h"
#include "nel/3d/primitive_block.h"


namespace NL3D 
{


// ***************************************************************************
// Primitives Indices reallocation. must be >3 (see below..)
static sint		GlobalTriListBlockRealloc= 1024;


// ***************************************************************************
sint			CPatchRdrPass::MaxGlobalIndex= GlobalTriListBlockRealloc;
std::vector<uint32>	CPatchRdrPass::GlobalTriList(CPatchRdrPass::MaxGlobalIndex);
// For CPatchRdrPass::getStartPointer(), must init to not 0 list size.

	
// ***************************************************************************
CPatchRdrPass::CPatchRdrPass()
{
	RefCount= 0;
	resetMaxTriList();
	resetTriList();
}

	
// ***************************************************************************
void			CPatchRdrPass::resetMaxTriList()
{
	NMaxTris=0;
	StartIndex= 0;
}

// ***************************************************************************
void			CPatchRdrPass::addMaxTris(sint ntris)
{
	NMaxTris+= ntris;
	MaxGlobalIndex+= ntris*3;
	// Realloc if necessary.
	if((sint)GlobalTriList.size() < MaxGlobalIndex)
	{
		GlobalTriList.resize(GlobalTriList.size() + NL3D::GlobalTriListBlockRealloc);
	}
}


// ***************************************************************************
void			CPatchRdrPass::computeStartIndex()
{
	StartIndex= MaxGlobalIndex;
	MaxGlobalIndex+= NMaxTris*3;
}


// ***************************************************************************
void			CPatchRdrPass::resetTriList()
{
	NTris=0;
	CurIndex=StartIndex;
}


// ***************************************************************************
void	CPatchRdrPass::resetGlobalIndex()
{
	MaxGlobalIndex= 0;
}



} // NL3D
