/** \file agent_input.cpp
 * <File description>
 *
 * $Id: agent_input.cpp,v 1.6 2002/03/07 11:09:03 portier Exp $
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


namespace NLAIAGENT
{
	IAgentInput::IAgentInput():_ActiveInput(false),_LocalValue(NULL)
	{	 
	}

	IAgentInput::IAgentInput(IObjectIA *o):_ActiveInput(false),_LocalValue(o)
	{	 
	}

	IAgentInput::IAgentInput(const IAgentInput &c):_ActiveInput(c._ActiveInput),_LocalValue(c._LocalValue != NULL? (IObjectIA *)c._LocalValue->clone() : NULL)
	{
	}

	IAgentInput::~IAgentInput()
	{
		if(_LocalValue != NULL) _LocalValue->release();
	}

	void IAgentInput::addInputConnection(IConnectIA* obj)
	{
		_ActiveInput = true; // The Input become active when an IconnectIA is interested by the change of the value.
		connect(obj);
		_ConnexionList.push(obj);
	}

	void IAgentInput::releaseInputConnexion(IConnectIA* obj)
	{
		removeConnection(obj);
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

	const IObjectIA::CProcessResult IAgentInput::runMsg(COnChangeMsg &msg)
	{
		return IAgentInput::_ConnexionList.sendMessage(&msg);
	}

	void IAgentInput::setValue(IObjectIA *o)
	{
		if(_LocalValue != NULL) _LocalValue->release();
		_LocalValue = o;
	}

	const IObjectIA::CProcessResult&  IAgentInput::run ()
	{
		if (IAgentInput::_ActiveInput)
		{
			const IObjectIA* value = IAgentInput::getValue();
			if (!(*value == *_LocalValue))
			{
				// If the component value has changed, we send a message to the list of interested IConnectIA.
				setValue((IObjectIA*) value);
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


	bool IAgentInput::isEqual(const IBasicObjectIA &a) const
	{
		if(_LocalValue != NULL) return _LocalValue->isEqual(a);
		return false;
	}
} // NLAIAGENT
