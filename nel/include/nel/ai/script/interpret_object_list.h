/** \file interpret_object_list.h
 * Class for define list of class interpreted.
 *
 * $Id: interpret_object_list.h,v 1.2 2001/01/05 16:05:57 chafik Exp $
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
#ifndef NL_INTERPRET_OBJECT_LIST_H
#define NL_INTERPRET_OBJECT_LIST_H
#include "script/interpret_object.h"

namespace NLIASCRIPT
{
	/**
	* Class IAgentMultiClass.
	* 
	* This class define list of class interpreted.
	* 
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CListClass: public NLIAAGENT::CGroupType
	{
	public:
		CListClass()
		{
		}

		///Find the class defined by a name.
		IClassInterpret *find(const NLIAAGENT::IVarName *name)
		{
			tListType::const_iterator i = getList().begin();
			while(i != getList().end())
			{
				IClassInterpret *o = (IClassInterpret *)*i++;
				if( *o->getClassName() == *name) return o;
			}
			return NULL;
		}
		virtual ~CListClass()
		{
		}
	};
}
#endif
