/** \file msg_group.cpp
 *
 * $Id: msg_group.cpp,v 1.4 2001/01/10 10:10:08 chafik Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/msg_group.h"

namespace NLAIAGENT
{

	CMessageGroup CMessageGroup::systemGroup(1);
	CMessageGroup CMessageGroup::msgScriptingGroup(2);
	
	const NLAIC::CIdentType &CMessageGroup::getType() const		
	{		
		return IdMessageGroup;
	}

}
