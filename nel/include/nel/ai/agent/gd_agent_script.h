/** \file gd_agent_script.h
 * goal drivent scripted agents with a FactBase and a goal stack
 *
 * $Id: gd_agent_script.h,v 1.1 2001/02/28 09:42:37 portier Exp $
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


#ifndef NL_GD_AGENT_SCRIPT_H
#define NL_GD_AGENT_SCRIPT_H


#include "nel/ai/script/gd_agent_class.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/gd_agent_script.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/goal.h"

namespace NLAIAGENT
{	

	class CGDAgentScript  : public CAgentScript
	{

		public:
			// Builds and actor with its father
			CGDAgentScript(IAgentManager *, bool activated = false);
			// copy constructor
			CGDAgentScript(const CGDAgentScript &);
			CGDAgentScript(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CGDAgentClass *);
			virtual ~CGDAgentScript();

			static const NLAIC::CIdentType IdGDAgentScript;					

//			virtual int getBaseMethodCount() const;

			/// Inherited functions
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
//			virtual void getDebugString(char *t) const;
			virtual bool isEqual(const IBasicObjectIA &a) const;
//			virtual void processMessages();
			virtual const CProcessResult &run();

//			virtual IObjectIA *run(const IMessageBase &msg);
//			virtual	CProcessResult sendMessage(IObjectIA *);
			virtual const NLAIC::CIdentType &getType() const;

			virtual void save(NLMISC::IStream &os);		
			virtual void load(NLMISC::IStream &is);		

//			virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
//			virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);

//			virtual tQueue isMember(const NLAIAGENT::IVarName *, const NLAIAGENT::IVarName *, const IObjectIA &) const;
//			virtual IObjectIA::CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
//			virtual IObjectIA::CProcessResult runMethodeMember(sint32, IObjectIA *);
//			virtual sint32 getMethodIndexSize() const;

//			void getFatherComponent(IVarName &);

			//////////////////////////////////////////////////////////////////////////
//			NLAILOGIC::CFactBase *getFactBase();
			void addGoal(NLAILOGIC::CGoal *);

/*			virtual NLAILOGIC::CFactBase &getFactBase()
			{
//				return _FactBase;
			}
*/
	};

}	// NLAIAGENT

#endif // NL_GD_AGENT_SCRIPT_H

