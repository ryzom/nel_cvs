/** \file main_agent_script.h
 * class for the man agent.
 *
 * $Id: main_agent_script.h,v 1.7 2001/02/08 17:27:45 chafik Exp $
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

#ifndef NL_MAIN_AGENTS_SCRIPT_H
#define NL_MAIN_AGENTS_SCRIPT_H

#include "nel/ai/agent/agent_manager.h"


namespace NLAIAGENT
{	

	struct CAgentNumber;
	class NLAISCRIPT::CCodeContext;
	class NLAISCRIPT::CStackPointer;

	/**	
	This class is the main agent, this main agent is a reception centre for all agent in a serveur. Wen agent is in it, he can access to an script
	context to execute script code.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/	
	class IMainAgent : public CAgentScript 
	{
	public:
		IMainAgent(const IMainAgent &a):CAgentScript(a){}
		IMainAgent(IAgentManager *a):CAgentScript(a){}
		IMainAgent():CAgentScript(NULL){}


		virtual	IObjectIA::CProcessResult sendMessage(const CAgentNumber &) = 0;
		

	};

	/**	
	concrete IMainAgent class.

	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000	
	*/	
	class CMainAgentScript : public IMainAgent 
	{
	
	private:

		///Context for the script.
		NLAISCRIPT::CCodeContext	*_CodeContext;
		///Heap and stack for the script.
		NLAISCRIPT::CStackPointer	*_Stack,*_Heap;
		
	public:
		static const NLAIC::CIdentType IdMainAgentScript;
			
	public:		
		virtual int getBaseMethodCount() const;
		virtual const IObjectIA *getAgentContext() const;

	public:
		CMainAgentScript(const CMainAgentScript &);
		CMainAgentScript(IAgentManager *,NLAIC::IIO *io);
		CMainAgentScript(NLAIC::IIO *io);
		virtual ~CMainAgentScript();

		const NLAIC::CIdentType &getType() const
		{
			return IdMainAgentScript;
		}

		virtual IMessageBase *runExec(const IMessageBase &);

		virtual IObjectIA::CProcessResult addDynamicAgent(IBaseGroupType *g);

		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;

		virtual	void CMainAgentScript::processMessages();
		virtual	IObjectIA::CProcessResult sendMessage(IObjectIA *);
		virtual	IObjectIA::CProcessResult sendMessage(const CAgentNumber &)
		{			
			char text[2048*8];			
			sprintf(text,"virtual IObjectIA::CProcessResult sendMessage(const CAgentNumber &) note implementaited for the '%s' class",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
		}
		//virtual IObjectIA *run(const IMessageBase &); ///throw throw Exc::CExceptionNotImplemented;		
		virtual const IObjectIA::CProcessResult &run();
	};	
}
#endif
