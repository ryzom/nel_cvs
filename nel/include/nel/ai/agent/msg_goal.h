/** \file msg_goal.h
 *
 * $Id: msg_goal.h,v 1.1 2001/02/28 09:42:37 portier Exp $
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
#ifndef NL_MSG_GOAL_H
#define NL_MSG_GOAL_H
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/message_script.h"

namespace NLAIAGENT
{
	class CGoalMsg : public CMessageScript 
	{
	public:
		static const NLAIC::CIdentType IdGoalMsg;
	public:
		CGoalMsg(const CGoalMsg &);
		CGoalMsg( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);
		CGoalMsg(NLAISCRIPT::CMessageClass *b = NULL);
		CGoalMsg(IBasicAgent *);

		/// \name NLAIC::IBasicInterface member class. 
		//@{		
		virtual const NLAIC::IBasicType *clone() const;		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return new CGoalMsg();
		}
		const NLAIC::CIdentType &getType() const;
		void getDebugString(char *t) const;
		//@}			

	};
}
#endif
