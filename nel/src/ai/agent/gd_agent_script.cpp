/** \file gd_agent_script.cpp
 *
 * $Id: gd_agent_script.cpp,v 1.4 2001/05/22 16:08:15 chafik Exp $
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
#include "nel/ai/agent/gd_agent_script.h"
#include "nel/ai/script/gd_agent_class.h"

#include "nel/ai/script/codage.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/msg.h"
#include "nel/ai/e/ai_exception.h"

namespace NLAIAGENT
{
//	static CGroupType listBidon;


	CGDAgentScript::CGDAgentScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CGDAgentClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
//		_FactBase = new NLAILOGIC::CFactBase();
	}	

	CGDAgentScript::CGDAgentScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
//		_FactBase = new NLAILOGIC::CFactBase();
	}

	CGDAgentScript::~CGDAgentScript()
	{
/*		if ( _FactBase )
			_FactBase->release();
			*/
	}


	CGDAgentScript::CGDAgentScript(const CGDAgentScript &c) : CAgentScript(c)
	{
//		_FactBase = (NLAILOGIC::CFactBase *) c._FactBase->clone();
	}

	const NLAIC::IBasicType *CGDAgentScript::clone() const
	{		
		CGDAgentScript *m = new CGDAgentScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *CGDAgentScript::newInstance() const
	{	
		CGDAgentScript *instance;
		if ( _AgentClass )
		{
			instance = (CGDAgentScript *) _AgentClass->buildNewInstance();
		}
		else 
		{	
			instance = new CGDAgentScript(NULL);
		}
		return instance;
	}
	
	/*void CGDAgentScript::getDebugString(std::string &t) const
	{
		if ( _AgentClass )
		{
			std::string buf[1024 * 4];
			_AgentClass->getDebugString(buf);
			t += "<GDAGentScript>");
			t += buf;
		}
		else
			t += "<CGDAgentScript>";
	}*/



	bool CGDAgentScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIC::CIdentType &CGDAgentScript::getType() const
	{
		if ( _AgentClass ) 
			return _AgentClass->getType();
		else
			return IdGDAgentScript;
	}

	void CGDAgentScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		// TODO
	}

	void CGDAgentScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		// TODO
	}
/*
	sint32 CGDAgentScript::getMethodIndexSize() const
	{
		return CAgentScript::getBaseMethodCount();
	}
	*/

/*	IObjectIA::CProcessResult CGDAgentScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
		return CAgentScript::runMethodeMember(heritance,index,params);
	}

  */


/*
	IObjectIA::CProcessResult CGDAgentScript::runMethodBase(int index,IObjectIA *params)
	{	

		index = index - IAgent::getMethodIndexSize();



		if ( index < getBaseMethodCount() )
			return CAgentScript::runMethodeMember(index, params);

		IObjectIA::CProcessResult r;

		char buf[1024];
		getDebugString(buf);

		return r;
	}
*/

/*
	int CGDAgentScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}
*/

/*	tQueue CGDAgentScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

		const char *txt = name->getString();

		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;

		if ( *name == CStringVarName("post") )
		{
		
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + fid_activate, 0.0,NULL, r_type ) );
		}
		return result;
	}
*/

	const IObjectIA::CProcessResult &CGDAgentScript::run()
	{
		return CAgentScript::run();
	}
	///////////////////////////////////////////////////////////

	void CGDAgentScript::addGoal(NLAILOGIC::CGoal *goal)
	{
		// Adds the goal to the goal stack
//		_GoalStack.push_back( goal );

		// Checks the operators
	}
} // NLAIAGENT
