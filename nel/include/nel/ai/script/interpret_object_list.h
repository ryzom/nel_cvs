/** \file interpret_object_list.h
 * Class for define list of class interpreted.
 *
 * $Id: interpret_object_list.h,v 1.6 2003/01/21 11:24:25 chafik Exp $
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
#include "nel/ai/script/interpret_object.h"

namespace NLAISCRIPT
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
	class CListClass: public NLAIAGENT::CGroupType
	{
	public:
		CListClass()
		{
		}

		///Find the class defined by a name.
		IClassInterpret *find(const NLAIAGENT::IVarName *name)
		{
			TListType::const_iterator i = getList().begin();
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
