/** \file agent_input.h
 * An interface giving a direct value from an agent component or telling when the value changed.
 *
 * $Id: agent_input.h,v 1.5 2003/01/23 15:40:55 chafik Exp $
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

#ifndef NL_AGENT_INPUT_H
#define NL_AGENT_INPUT_H

#include "nel/ai/c/abstract_interface.h"
#include "nel/ai/agent/baseai.h"
#include "nel/ai/agent/list_manager.h"
#include "nel/ai/agent/msg_on_change.h"

namespace NLAIAGENT
{

	/**
	 * An interface giving a direct value from an agent component or telling when the value changed.
	 * \author Gabriel ROBERT
	 * \author Nevrax France
	 * \date 2001
	 */
	class IAgentInput : public IConnectIA
	{
	protected :
		bool				_ActiveInput;	// True if _ConnexionList not empty.
		IObjectIA			*_LocalValue;	// Last value of the component for know if his value as changed.
		CGroupType			_ConnexionList;	// A list of IConnectIA interested by the change of the value.	

	public:

		///Constructor
		IAgentInput();

		IAgentInput(const IAgentInput &);

		///Constructor with an value
		IAgentInput(IObjectIA *);

		/// Destructor
		virtual ~IAgentInput();

		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual const CProcessResult &getState() const
		{
			return IObjectIA::ProcessRun;
		}
		virtual void setState(TProcessStatement s, IObjectIA *result) {}	

		/// Add an obj to the list of IconnectIA interested by the value change.
		void addInputConnection(IConnectIA* obj);

		/// Remove an obj to the list of IconnectIA interested by the value change.
		void releaseInputConnexion(IConnectIA* obj);

		/// Return the value managed by this IAgentInput. Must be Reimplemented.
		virtual const IObjectIA* getValue() const {return _LocalValue;}

		virtual void setValue(IObjectIA *);

		virtual	const CProcessResult runMsg(COnChangeMsg &msg);	

		/// \name IObjectIA member method. 
		//@{
		virtual	const CProcessResult&run ();
		//@}
		
		/// \name IConnectIA member method. 
		//@{
		virtual void onKill(IConnectIA* c);
		//@}

		/// \name IBasicInterface member method. 
		//@{
		virtual void save(NLMISC::IStream &);
		virtual void load(NLMISC::IStream &);
		//@}
	};


} // NLAIAGENT


#endif // NL_AGENT_INPUT_H

/* End of agent_input.h */
