/** \file interpret_message_action.h
 * Class for define success and failure message in the interpreter.
 *
 * $Id: interpret_message_action.h,v 1.1 2001/03/07 11:26:12 chafik Exp $
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
#ifndef NL_INTERPRET_MESSAGE_ACTION_H
#define NL_INTERPRET_MESSAGE_ACTION_H
#include "nel/ai/script/interpret_object_message.h"

namespace NLAISCRIPT
{
	class CMsgSuccessClass : public CMessageClass
	{
	public:
		static const NLAIC::CIdentType IdMsgSuccessClass;
	public:
		CMsgSuccessClass(const NLAIC::CIdentType &);
		CMsgSuccessClass();
		~CMsgSuccessClass(){}

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdMsgSuccessClass;
		}
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}
	};

	class CMsgFailureClass : public CMessageClass
	{
	public:
		static const NLAIC::CIdentType IdMsgFailureClass;
	public:
		CMsgFailureClass(const NLAIC::CIdentType &);
		CMsgFailureClass();
		~CMsgFailureClass(){}

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdMsgFailureClass;
		}
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}
	};
}
#endif