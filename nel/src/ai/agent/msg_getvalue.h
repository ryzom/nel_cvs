/** \file msg_getvalue.h
 * Message to ask an agent for its value
 *
 * $Id: msg_getvalue.h,v 1.1 2001/12/05 10:13:53 portier Exp $
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

#include "nel/ai/include/agent/message_script.h"
#include "nel/ai/include/script/interpret_object_message.h"

namespace NLAIAGENT {

	class CMsgGetValue : public CMessageScript 
	{
	public:
		static const NLAIC::CIdentType IdMsgGetValue;
	public:
		CMsgGetValue(const CMsgGetValue &);
		CMsgGetValue( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);
		CMsgGetValue(NLAISCRIPT::CMessageClass *b = NULL);
		CMsgGetValue(IBasicAgent *);
		virtual ~CMsgGetValue();


		/// \name NLAIC::IBasicInterface member class. 
		//@{		
		virtual const NLAIC::IBasicType *clone() const;		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return new CMsgGetValue();
		}
		const NLAIC::CIdentType &getType() const;
		void getDebugString(std::string &t) const;


		virtual tQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const NLAIAGENT::IObjectIA &) const;
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *);

		virtual sint32 getBaseMethodCount() const;
		//@}			
	};


} // NLAIAGENT