/** \file var_mem.cpp
 *
 * $Id: var_mem.cpp,v 1.1 2001/05/31 15:11:59 chafik Exp $
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

#include "nel/ai/logic/var_mem.h"
namespace NLAILOGIC
{
	const NLAIC::CIdentType *CVarMem::IdVarMem = NULL;

	void CVarMem::initClass()
	{
		CVarMem h;
		CVarMem::IdVarMem = new NLAIC::CIdentType (	"VarMem", 
													NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)h), 
													NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
													NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
	}

	void CVarMem::releaseClass()
	{
		delete CVarMem::IdVarMem;
	}

}
