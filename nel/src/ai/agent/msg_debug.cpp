/** \file msg_debug.cpp
 *
 * $Id: msg_debug.cpp,v 1.1 2001/03/07 13:07:40 chafik Exp $
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

#include "nel/ai/agent/msg_debug.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/interpret_object_message.h"

namespace NLAIAGENT
{
	CMsgDebug::CMsgDebug( std::list<IObjectIA *> &l, NLAISCRIPT::CMessageClass *b):CMessageScript(l,b)
	{		
	}
	CMsgDebug::CMsgDebug(NLAISCRIPT::CMessageClass *b):CMessageScript(b)
	{				
	}

	CMsgDebug::CMsgDebug(IBasicAgent *agent):
			CMessageScript((NLAISCRIPT::CMessageClass *)NLAISCRIPT::CMsgNotifyParentClass::IdMsgNotifyParentClass.getFactory()->getClass())
	{				
 	}

	CMsgDebug::CMsgDebug(const CMsgDebug &m): CMessageScript(m)
	{
	}

	const NLAIC::IBasicType *CMsgDebug::clone() const
	{
		//CLocalAgentMail *g = (CLocalAgentMail *)get();
		return new CMsgDebug(*this);
	}

	const NLAIC::CIdentType &CMsgDebug::getType() const
	{
		if ( getCreatorClass() ) 
			return getCreatorClass()->getType();
		else
			return IdMsgDebug;
	}	

	void CMsgDebug::getDebugString(char *t) const
	{		
		sprintf(t,"CMsgDebug");		
	}
}