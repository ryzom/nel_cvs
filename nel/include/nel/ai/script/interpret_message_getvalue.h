/** \file interpret_message_getvalue.h
 * Class definition for GetValue messages
 *
 * $Id: interpret_message_getvalue.h,v 1.1 2001/12/05 10:19:10 portier Exp $
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
//#ifndef NL_MESSAGE_GETVALUE_CLASS
//#define NL_MESSAGE_GETVALUE_CLASS

#include "nel/ai/script/interpret_object_message.h"

namespace NLAISCRIPT
{
	class CGetValueMsgClass : public CMessageClass
	{
	public:
		static const NLAIC::CIdentType IdGetValueMsgClass;
	public:
		CGetValueMsgClass(const NLAIAGENT::IVarName &);
		CGetValueMsgClass(const NLAIC::CIdentType &);
		CGetValueMsgClass();
		virtual ~CGetValueMsgClass(){}

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdGetValueMsgClass;
		}
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}
	};
}
//#endif
