

/** \file msg_fact.h
 *
 * $Id: msg_fact.h,v 1.4 2003/01/21 11:24:25 chafik Exp $
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

#ifndef NL_MSG_FACT_H
#define NL_MSG_FACT_H
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/message_script.h"

namespace NLAIAGENT
{
	class CFactMsg : public CMessageScript 
	{
	public:
		static const NLAIC::CIdentType IdFactMsg;
	public:
		CFactMsg(const CFactMsg &);
		CFactMsg( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);
		CFactMsg(NLAISCRIPT::CMessageClass *b = NULL);
		CFactMsg(IBasicAgent *);
		virtual ~CFactMsg();


		/// \name NLAIC::IBasicInterface member class. 
		//@{		
		virtual const NLAIC::IBasicType *clone() const;		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return new CFactMsg();
		}
		const NLAIC::CIdentType &getType() const;
		void getDebugString(std::string &t) const;


		TQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const NLAIAGENT::IObjectIA &) const;
		IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
		IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);

		virtual sint32 getBaseMethodCount() const;

		//@}			

	};
}
#endif

