/** \file msg_action.h
 *
 * $Id: msg_action.h,v 1.5 2003/01/21 11:24:25 chafik Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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
#ifndef NL_MSG_ACTION_H
#define NL_MSG_ACTION_H
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/message_script.h"

namespace NLAIAGENT
{
	class CSuccessMsg : public CMessageScript 
	{
	public:
		static const NLAIC::CIdentType IdSuccessMsg;
	public:
		CSuccessMsg(const CSuccessMsg &);
		CSuccessMsg( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);
		CSuccessMsg(NLAISCRIPT::CMessageClass *b = NULL);
		CSuccessMsg(IBasicAgent *);
		virtual ~CSuccessMsg();


		/// \name NLAIC::IBasicInterface member class. 
		//@{		
		virtual const NLAIC::IBasicType *clone() const;		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return new CSuccessMsg();
		}
		const NLAIC::CIdentType &getType() const;
		void getDebugString(std::string &t) const;


		virtual TQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const NLAIAGENT::IObjectIA &) const;
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);

		virtual sint32 getBaseMethodCount() const;

		//@}			

	};

	class CFailureMsg : public CMessageScript 
	{
	public:
		static const NLAIC::CIdentType IdFailureMsg;
	public:
		CFailureMsg(const CFailureMsg &);
		CFailureMsg( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);
		CFailureMsg(NLAISCRIPT::CMessageClass *b = NULL);
		CFailureMsg(IBasicAgent *);
		virtual ~CFailureMsg();


		/// \name NLAIC::IBasicInterface member class. 
		//@{		
		virtual const NLAIC::IBasicType *clone() const;		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return new CFailureMsg();
		}
		const NLAIC::CIdentType &getType() const;
		void getDebugString(std::string &t) const;


		virtual TQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const NLAIAGENT::IObjectIA &) const;
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);

		virtual sint32 getBaseMethodCount() const;

		//@}			

	};

}
#endif
