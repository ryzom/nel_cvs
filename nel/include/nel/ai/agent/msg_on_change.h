/** \file msg_on_change.h
 *	
 *	On change message.
 *	
 * $Id: msg_on_change.h,v 1.4 2003/01/27 16:54:43 chafik Exp $
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
#ifndef NL_MSG_ONCHANGE_H
#define NL_MSG_ONCHANGE_H
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/message_script.h"

namespace NLAIAGENT
{
	/**
	This method is use when an agent attribute have a change.
	*/
	class COnChangeMsg : public CMessageScript 
	{
	public:
		static const NLAIC::CIdentType IdOnChangeMsg;
	public:
		COnChangeMsg(const COnChangeMsg &);
		COnChangeMsg( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);
		COnChangeMsg(NLAISCRIPT::CMessageClass *b = NULL);
		COnChangeMsg(IBasicAgent *);
		~COnChangeMsg()
		{
		}

		/// \name NLAIC::IBasicInterface member class. 
		//@{		
		virtual const NLAIC::IBasicType *clone() const;		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return new COnChangeMsg();
		}
		const NLAIC::CIdentType &getType() const;
		void getDebugString(std::string &t) const;		

		//@}			

	};
}
#endif
