/** \file register_class.cpp
 *
 * $Id: registry_class.cpp,v 1.3 2001/01/08 14:42:11 valignat Exp $
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
#include <iostream.h>
#include <stdio.h>

#include "nel/ai/e/ia_exception.h" 
#include "nel/ai/c/registry_class.h"

namespace NLAIC
{
	CRegistry *registry = NULL;//new tRegistry;

	CRegistry *getRegistry()
	{
		if(registry == NULL) 
		{
			return (registry = new CRegistry);
		}
		else
			return registry;
	}

	sint32 registerClass(const CIdentType &ident,const IClassCFactory &factoryClass)
	{
		return registry->registerClass(ident,factoryClass);
	}

	IBasicInterface *createInstance(const CIdentType &ident)
	{
		return registry->createInstance(ident);
	}

	void initRegistry()
	{
		if ( registry == NULL ) registry= new CRegistry;
	}
	void releaseRegistry()
	{
		delete registry;
	}
}