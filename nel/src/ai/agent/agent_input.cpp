/** \file agent_input.cpp
 * <File description>
 *
 * $Id: agent_input.cpp,v 1.1 2001/03/06 14:10:47 robert Exp $
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

#include "nel/ai/agent/agent_input.h"
#include "nel/ai/agent/msg_on_change.h"


namespace NLAIAGENT
{

IAgentInput::IAgentInput()
{
	_ActiveInput = false;
	_LocalValue = NULL;
}

IAgentInput::~IAgentInput()
{
}

void IAgentInput::addInputConnection(IConnectIA* obj)
{
	_ActiveInput = true; // The Input become active when an IconnectIA is interested by the change of the value.
	connect(obj);
	_ConnexionList.push(obj);
}

void IAgentInput::releaseInputConnexion(IConnectIA* obj)
{
	removeConnection(*obj);
	_ConnexionList.erase(obj);
	if (_ConnexionList.size() == 0)
	{
		_ActiveInput = false;
	}
}

void IAgentInput::onKill(IConnectIA* c)
{
	releaseInputConnexion(c);
}

const IObjectIA::CProcessResult&  IAgentInput::run ()
{
	if (IAgentInput::_ActiveInput)
	{
		const IObjectIA* value = IAgentInput::getValue();
		if (*value == *_LocalValue)
		{
			// If the component value as changed, we send a message to the list of interested IConnectIA.
			_LocalValue = value;
			COnChangeMsg msg;
			IAgentInput::_ConnexionList.sendMessage(&msg);
		}
	}
	return IObjectIA::ProcessRun;
}

void IAgentInput::save(NLMISC::IStream &os)
{
	IConnectIA::save(os);
	os.serial(_ActiveInput);
	os.serial(_ConnexionList);
}

void IAgentInput::load(NLMISC::IStream &is)
{
	IConnectIA::load(is);
	is.serial(_ActiveInput);
	is.serial(_ConnexionList);
}

} // NLAIAGENT
