/** \file agent_manager.h
 * class for mange agent.
 *
 * $Id: agent_manager.h,v 1.4 2003/02/05 16:05:53 chafik Exp $
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

#ifndef NL_AGENTS_MANAGER_H
#define NL_AGENTS_MANAGER_H

namespace NLAIAGENT
{
	/**
	Base class for manage agent.
	
	We know that at this moment we have two kind of agent, agent that it define with the c++ and the script agent.
	The script need a context, the context for a script is the stack, the heap, the debug parametre ,...
	The c++ agent can neead a context, it coud be the server where he work or ather think define by any programmer.
	Well this class define this ideas and resolve ours problmem by the introduction of an agent manger.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IAgentManager : public IAgent
	{
	public:

		///Construct manager with a parent.
		IAgentManager(IBasicAgent *parent):IAgent(parent)
		{
		}

		///Construct manager with a parent and a mail box.
		IAgentManager(IBasicAgent *parent,IMailBox *m):IAgent(parent,m)
		{
		}

		///Copy constructor.
		IAgentManager(const IAgentManager &a):IAgent(a)
		{
		}
		/** 
			Get the context for an agent.
			At present if the context is an CCodeContext.
			Programmers have to test the type of the IObjectIA returned by method to know how type of context is.
		*/
		virtual const IObjectIA *getAgentContext() const = 0;

		/// Return the nomber of internal C++ hard coded method that the class can process.
		virtual int getBaseMethodCount() const
		{
			return IAgent::getMethodIndexSize();
		}		

		virtual~IAgentManager()
		{
		}

	};
}

#endif
