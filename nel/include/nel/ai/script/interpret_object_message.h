/** \file interpret_object_list.h
 * Class for define an message class in the interpreter.
 *
 * $Id: interpret_object_message.h,v 1.8 2001/02/01 17:15:20 chafik Exp $
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
#ifndef NL_INTERPRET_OBJECT_MESSAGE_H
#define NL_INTERPRET_OBJECT_MESSAGE_H
#include "nel/ai/script/interpret_object_agent.h"

namespace NLAISCRIPT
{
	class CMessageClass: public CAgentClass
	{
	public:
		static const NLAIC::CIdentType IdMessageClass;
	public:
		CMessageClass(const NLAIAGENT::IVarName &);
		CMessageClass(const NLAIC::CIdentType &);
		CMessageClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CMessageClass(const CMessageClass &);
		CMessageClass();

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(char *t) const;
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}

		virtual ~CMessageClass();
	};

	class CMsgNotifyParentClass: public CMessageClass
	{
	public:
		static const NLAIC::CIdentType IdMsgNotifyParentClass;
	public:
		CMsgNotifyParentClass();
		~CMsgNotifyParentClass(){}

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdMsgNotifyParentClass;
		}
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}
	};
}
#endif
