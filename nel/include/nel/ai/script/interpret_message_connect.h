/** \file interpret_message_action.h
 * Class for define success and failure message in the interpreter.
 *
 * $Id: interpret_message_connect.h,v 1.1 2002/03/11 09:58:52 portier Exp $
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
#ifndef NL_INTERPRET_MESSAGE_CONNECT_H
#define NL_INTERPRET_MESSAGE_CONNECT_H
#include "nel/ai/script/interpret_object_message.h"

namespace NLAISCRIPT
{
	class CConnectObjectValueMsgClass : public CMessageClass
	{
	public:
		static const NLAIC::CIdentType IdConnectObjectValueMsgClass;
	public:
		CConnectObjectValueMsgClass(const NLAIAGENT::IVarName &);
		CConnectObjectValueMsgClass(const NLAIC::CIdentType &);
		CConnectObjectValueMsgClass();
		virtual ~CConnectObjectValueMsgClass(){}

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdConnectObjectValueMsgClass;
		}
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}
	};

/*	class CFailureMsgClass : public CMessageClass
	{
	public:
		static const NLAIC::CIdentType IdFailureMsgClass;
	public:
		CFailureMsgClass(const NLAIAGENT::IVarName &);
		CFailureMsgClass(const NLAIC::CIdentType &);
		CFailureMsgClass();
		virtual ~CFailureMsgClass(){}

		/// \name CAgentClass method.
		//@{
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdFailureMsgClass;
		}
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;		
		//@}
	};
	*/
}
#endif
