/** \file agent_script.cpp
 *
 * $Id: agent_script.cpp,v 1.3 2001/01/08 10:51:01 chafik Exp $
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
#include "agent/agent_script.h"
#include "agent/agent_manager.h"
#include "script/interpret_object_agent.h"
#include "script/codage.h"
#include "script/type_def.h"
#include "script/object_unknown.h"
#include "agent/agent_method_def.h"
#include "agent/message_script.h"
#include "script/interpret_object_message.h"
#include "script/interpret_object_agent.h"

namespace NLAIAGENT
{
	const int CAgentScript::sendTag = 0;
	const int CAgentScript::getChildTag = 1;
	const int CAgentScript::addChildTag = 2;
	const int CAgentScript::baseMethodIn = CAgentScript::addChildTag + 1;
	static CGroupType listBidon;

	CAgentScript::CAgentScript(const CAgentScript &a): IAgentManager(a)
	{
		if ( a._AgentClass )
		{
			_AgentClass = a._AgentClass;
			a._AgentClass->incRef();
		}

		_NbComponents = a._NbComponents;
		_Components = new IObjectIA *[ _NbComponents ];
		for ( int i = 0; i < _NbComponents; i++ )
			_Components[i] = (IObjectIA *)a._Components[i]->clone();		

		if(	a._ScriptMail) 
			_ScriptMail = (IMailBox *)a._ScriptMail->clone();
		
		_AgentManager = a._AgentManager;
		if(_AgentManager) _AgentManager->incRef();

		std::vector<NLAILOGIC::IBaseOperator *>::const_iterator it_o = a._Operators.begin();
		while ( it_o != a._Operators.end() )
		{
			addOperator( *it_o );
			it_o++;
		}
			
	}
	
	CAgentScript::CAgentScript(IAgentManager *manager) : IAgentManager( NULL ), _AgentClass( NULL )
	{
		_Components = NULL;
		_NbComponents = 0;		
		_AgentManager = manager;
		_ScriptMail = new CSimpleLocalMailBox((const IWordNumRef *)*this);
		_ScriptMail->incRef();
	}

	CAgentScript::CAgentScript(IAgentManager *manager, IBasicAgent *father,//The agent's father 
							  std::list<IObjectIA *> &components,	//Static components							  
							  NLIASCRIPT::CAgentClass *agent_class )	//Class
	
	: IAgentManager(father), _AgentClass( agent_class )
	{	
		if ( _AgentClass )
			_AgentClass->incRef();

		// Creates the static components array
		_NbComponents = components.size();
		_Components = new IObjectIA *[ _NbComponents ];
		std::list<IObjectIA *>::iterator it_c = components.begin();
		int id_c = 0;
		_AgentManager = manager;
		while ( it_c != components.end() )
		{
			IObjectIA *o = (IObjectIA *)*it_c;
			_Components[id_c] = o;
			it_c++;
			id_c++;

			/*if(((const NLAIC::CTypeOfObject &)o->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
			{
				incRef();
				((CAgentScript *)o)->setAgentManager(this);
			}*/
		}		
		_ScriptMail = new CSimpleLocalMailBox((const IWordNumRef *)*this);
		_ScriptMail->incRef();
	}	

	CAgentScript::~CAgentScript()
	{
		if ( _AgentClass )
			_AgentClass->release();

		// destruction of static components
		for ( int i = 0; i < _NbComponents; i++ )
			_Components[i]->release();
		delete[] _Components;
		
		if(_AgentManager != NULL) _AgentManager->release();
		_ScriptMail->release();
	}

	void CAgentScript::setAgentManager(IAgentManager *manager)
	{
		if(_AgentManager != NULL) _AgentManager->release();
		_AgentManager = manager;
		for ( int i = 0; i < _NbComponents; i++ )
		{
			if(((const NLAIC::CTypeOfObject &)_Components[i]->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
			{
				_AgentManager->incRef();
				((CAgentScript *)_Components[i])->setAgentManager(_AgentManager);
			}
		}
			
	}

	void CAgentScript::setStaticMember(sint32 index,IObjectIA *op)
	{
#ifdef _DEBUG
		if ( index >= _NbComponents )
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		_Components[ index ]->release();
		_Components[ index ] = op;
	}

	sint32 CAgentScript::getStaticMemberSize() const
	{
		return _AgentClass->getStaticMemberSize();
	}

	
	const IObjectIA *CAgentScript::getStaticMember(sint32 index) const
	{
#ifdef _DEBUG
		if ( index >= _NbComponents )
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return _Components[ index ];
	}

	NLIASCRIPT::IOpCode &CAgentScript::getMethode(sint32 inheritance,sint32 index)
	{
#ifdef _DEBUG
		if ( index >= _AgentClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}

		if ( inheritance >= _AgentClass->sizeVTable())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLIASCRIPT::IOpCode &)_AgentClass->getBrancheCode(inheritance,index).getCode();
	}

	NLIASCRIPT::IOpCode &CAgentScript::getMethode(sint32 index)
	{
#ifdef _DEBUG
		if ( index >= _AgentClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLIASCRIPT::IOpCode &)_AgentClass->getBrancheCode(index).getCode();
	}

	void CAgentScript::save(NLMISC::IStream &os)
	{
		IBasicAgent::save(os);
		sint32 size = _NbComponents;
		os.serial( size );
		for ( int i = 0; i < _NbComponents; i++ )
		{
			os.serial( (NLAIC::CIdentType &) _Components[i]->getType() );
			_Components[i]->save(os);
		}		
	}	

	void CAgentScript::load(NLMISC::IStream &is)
	{
		IBasicAgent::load(is);
		
		// Loads static components
		sint32 size;
		is.serial( size );
		_NbComponents = size;

		if(_Components)
		{
			for ( int i = 0; i < _NbComponents; i++ )
							_Components[i]->release();
			delete[] _Components;
		}
		_Components = new IObjectIA *[ _NbComponents ];

		for ( int i = 0; i < _NbComponents; i++ )
		{
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			IObjectIA *tmp_c = (IObjectIA *)id.allocClass();
			tmp_c->load(is);
			_Components[i] = tmp_c;
		}
	}

	sint32 CAgentScript::getMethodIndexSize() const
	{
		return _AgentClass->getMethodIndexSize();
	}

	const NLAIC::IBasicType *CAgentScript::clone() const
	{		
		CAgentScript *result = new CAgentScript(*this);
		result->incRef();		
		return result;
		// TODO: copie des fonctions
	}

	const NLAIC::IBasicType *CAgentScript::newInstance() const
	{
		CAgentScript *instance;
		if ( _AgentClass )
		{
			instance = (CAgentScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new CAgentScript(NULL);
			instance->incRef();
		}
		return instance;
	}

	void CAgentScript::getDebugString(char *t) const
	{
		if ( _AgentClass )
			sprintf(t,"<%s> (scripted)  -StaticComponents:\n",(const char *)_AgentClass->getType());
		else
			strcat(t,"<undefined_class> (scripted) -StaticComponents:\n");

		char buf[1024*8];
		for (int i = 0; i < _NbComponents; i++ )
		{
			strcat(t, "\t\t");
			if ( _AgentClass->getComponentName(i) )
			{
				strcat(t, _AgentClass->getComponentName(i) );
			}
			else
				strcat(t, "<unnamed>");

			strcat(t, "\t\t");
			_Components[i]->getDebugString(buf);
			strcat(t, "[ ");
			strcat(t, buf);
			strcat(t, " ]");
			if(i != (_NbComponents-1)) strcat(t, "\n");
		}
	}

	bool CAgentScript::isEqual(const IBasicObjectIA &a) const
	{
		return false;
	}

	void CAgentScript::onKill(IConnectIA *a)
	{
		//eraseFromList<IBasicAgent *>(&_AgentList,(IBasicAgent *)a);		
		IAgent::onKill(a);
	}

	IObjectIA::CProcessResult CAgentScript::getDynamicAgent(IBaseGroupType *g)
	{		
		CStringType *s = (CStringType *)g->get();
		tmapDefNameAgent::iterator i = _DynamicAgentName.find(*s);
		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;

		if(i != _DynamicAgentName.end())
		{			
			r.Result = (IObjectIA *)*((*i).second);
			r.Result->incRef();
			return r;
		}		
		r.Result = &DigitalType::NullOperator;
		r.Result->incRef();

		return r;
	}

	IObjectIA::CProcessResult CAgentScript::addDynamicAgent(IBaseGroupType *g)
	{
		CIteratorContener i = g->getIterator();
		CStringType &s = (CStringType &)*i++;
		IBasicAgent *o = (IBasicAgent *)i++;

		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;

		tmapDefNameAgent::iterator it = _DynamicAgentName.find(s);
		if(it == _DynamicAgentName.end())
		{			
			//o->incRef();
			_DynamicAgentName.insert(tPairName(s,addChild(o)));
		}
		r.Result = NULL;

		return r;
	}

	IObjectIA::CProcessResult CAgentScript::sendMethod(IObjectIA *param)
	{
		IMessageBase *msg = (IMessageBase *)((IBaseGroupType *)param)->pop();		
		return sendMessage(msg);
	}

	IObjectIA::CProcessResult CAgentScript::sendMessage(IObjectIA *m)
	{
		IMessageBase *msg = (IMessageBase *)m;
		msg->setSender(this);
		if(msg->getMethodIndex() >= 0)
		{
			_ScriptMail->addMessage(msg);			
		}		
		else 
		{
			return IAgent::sendMessage(msg);
		}
		return IObjectIA::CProcessResult();
	}

	void CAgentScript::runChildren()
	{
		// Activation des agents de la partie statique
		for ( int i = 0; i < _NbComponents; i++ )
		{
			_Components[i]->run();
		}

		// Activation des fils
		IAgent::runChildren();
	}

	void CAgentScript::processMessages()
	{		
		NLIASCRIPT::CCodeContext &context = (NLIASCRIPT::CCodeContext &)*_AgentManager->getAgentContext();
		while(_ScriptMail->getMessageCount())
		{
			IMessageBase &msg = (IMessageBase &)_ScriptMail->getMessage();
			IBaseGroupType *param = new CGroupType();
			param->incRef();
			msg.incRef();
			param->push(&msg);
			context.Stack ++;
			context.Stack[(int)context.Stack] = param;
			if(msg.getMethodIndex() >= 0)
			{
				//IObjectIA::CProcessResult r;

				NLIASCRIPT::IMethodContext *methodContex;				

				if (context.ContextDebug.Active)
				{
					context.ContextDebug.Param.push_back(&listBidon);
					listBidon.incRef();
					listBidon.incRef();
					methodContex = new NLIASCRIPT::CMethodContextDebug();
				}				
				else
				{
					methodContex = new NLIASCRIPT::CMethodContext();
				}
				const IObjectIA *self = context.Self;
				context.Self = this;
				NLIASCRIPT::CCallMethod opCall(methodContex,msg.getHeritanceIndex(),msg.getMethodIndex());
				opCall.runOpCode(context);
				context.Self = self;				
			}
			_ScriptMail->popMessage();
		}
		IAgent::processMessages();
	}

	const IObjectIA::CProcessResult &CAgentScript::run()
	{
		setState(processBuzzy,NULL);

		runChildren();

		_ScriptMail->run();
		getMail()->run();

		if(_AgentClass->getRunMethod() >= 0) 
		{
			NLIASCRIPT::CCodeContext *context = (NLIASCRIPT::CCodeContext *)_AgentManager->getAgentContext();
			context->Self = this;
			runMethodeMember(_AgentClass->getRunMethod(), context);
		}
		processMessages();

		setState(processIdle,NULL);
		return getState();
	}

	void CAgentScript::addOperator(NLAILOGIC::IBaseOperator *op)
	{
		_Operators.push_back(op);
	}

	void CAgentScript::remOperator(NLAILOGIC::IBaseOperator *op)
	{
		std::vector<NLAILOGIC::IBaseOperator *>::iterator it_o = _Operators.begin();
		while ( it_o != _Operators.end() )
		{
			if ( op == *it_o )
			{
				_Operators.erase( it_o );
				return;
			}
			it_o++;
		}
		char buf[2048];
		op->getDebugString(buf);
		throw NLAIE::CExceptionObjectNotFoundError(buf);
	}	

	const NLAIC::CIdentType &CAgentScript::getType() const
	{
		if ( _AgentClass ) 
			return _AgentClass->getType();
		else
			return IdAgentScript;
	}

	IObjectIA::CProcessResult CAgentScript::runMethodBase(int heritance, int index,IObjectIA *o)
	{
		if(index == sendTag)
		{			
			return sendMethod(o);
		}
		else
		if(index == getChildTag)
		{
			return getDynamicAgent((IBaseGroupType *)o);			
		}
		else
		if(index == addChildTag)
		{
			return addDynamicAgent((IBaseGroupType *)o);			
		}
		return IAgent::runMethodeMember(heritance,index,o);
	}

	IObjectIA::CProcessResult CAgentScript::runMethodBase(int index,IObjectIA *o)
	{		
		int i = index - IAgent::getMethodIndexSize();
		if(i == sendTag)
		{
			return sendMethod(o);
		}
		else
		if(i == getChildTag)
		{
			return getDynamicAgent((IBaseGroupType *)o);			
		}
		else
		if(i == addChildTag)
		{
			return addDynamicAgent((IBaseGroupType *)o);			
		}
		else return IAgent::runMethodeMember(index,o);
	}

	int CAgentScript::getBaseMethodCount() const
	{
		return IAgentManager::getBaseMethodCount() + baseMethodIn;
	}

	IObjectIA::CProcessResult CAgentScript::runMethodeMember(sint32 inheritance, sint32 index, IObjectIA *c)	
	{

		if(c->getType() != NLIASCRIPT::CCodeContext::IdCodeContext)
		{
			return IAgent::runMethodeMember(inheritance,index, c);
		}
		
		NLIASCRIPT::IOpCode *opPtr = NULL;
		NLIASCRIPT::CCodeContext &context = (NLIASCRIPT::CCodeContext &)*c;

		int i = index - getBaseMethodCount();
		if(i < 0)
		{			
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
				listBidon.incRef();
				listBidon.incRef();
			}
		
			IObjectIA::CProcessResult r = runMethodBase(index,(IObjectIA *)context.Param.back());
			if(r.Result != NULL)
			{
				context.Stack++;
				context.Stack[(int)context.Stack] = r.Result;
			}
			r.Result = NULL;
			return r;
		}
		else
		{
			opPtr = &getMethode(inheritance,i);
		}
		
		NLIASCRIPT::IOpCode &op = *opPtr;
		NLIASCRIPT::CCodeBrancheRun *opTmp = context.Code;
		int ip = (uint32)*context.Code;
		context.Code = (NLIASCRIPT::CCodeBrancheRun *)&op;		
		*context.Code = 0;

		/*TProcessStatement k = IObjectIA::ProcessIdle;

		while(k != IObjectIA::ProcessEnd)
		{
			k = op.run(context);
		}*/

		IObjectIA::CProcessResult r = ((NLIASCRIPT::ICodeBranche *)opPtr)->run(context);
		// If we are in Debug Mode
		if (context.ContextDebug.Active)
		{
			context.ContextDebug.callStackPop();
		}
		*context.Code = ip;
		context.Code = opTmp;		
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runMethodeMember(sint32 index, IObjectIA *c)	
	{
		if(c->getType() != NLIASCRIPT::CCodeContext::IdCodeContext)
		{
			return IAgent::runMethodeMember(index, c);
		}

		NLIASCRIPT::IOpCode *opPtr = NULL;
		NLIASCRIPT::CCodeContext &context = (NLIASCRIPT::CCodeContext &)*c;

		int i = index - getBaseMethodCount();
		if(i < 0)
		{
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
				listBidon.incRef();
				listBidon.incRef();
			}

			IObjectIA::CProcessResult r = runMethodBase(index,(IObjectIA *)context.Param.back());
			if(r.Result != NULL)
			{
				context.Stack++;
				context.Stack[(int)context.Stack] = r.Result;
			}
			r.Result = NULL;
			return r;
		}
		else
		{
			opPtr = &getMethode(i);
		}
		
		NLIASCRIPT::IOpCode &op = *opPtr;//getMethode(inheritance,i);
		NLIASCRIPT::CCodeBrancheRun *opTmp = context.Code;
		int ip = (uint32)*context.Code;
		context.Code = (NLIASCRIPT::CCodeBrancheRun *)opPtr;		
		*context.Code = 0;

		/*TProcessStatement k = IObjectIA::ProcessIdle;

		while(k != IObjectIA::ProcessEnd)
		{
			k = op.run(context);	
		}*/		
		IObjectIA::CProcessResult r = ((NLIASCRIPT::ICodeBranche *)opPtr)->run(context);

		// If we are in Debug Mode
		if (context.ContextDebug.Active)
		{
			context.ContextDebug.callStackPop();
		}

		*context.Code = ip;
		context.Code = opTmp;
		/*IObjectIA::CProcessResult r;
		r.Result = NULL;
		r.ResultState = k;*/
		return r;
	}

	tQueue CAgentScript::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{		
		CStringVarName send(_SEND_);
		CStringVarName gChild(_GETCHILD_);
		CStringVarName aChild(_ADDCHILD_);

		if(*methodName == send)
		{
			tQueue r;			
			NLIASCRIPT::CParam p;
			NLIASCRIPT::COperandSimple m(new NLAIC::CIdentType(NLIASCRIPT::CMessageClass::IdMessageClass));
			m.incRef();
			m.incRef();
			p.push(&m);
#ifdef _DEBUG
	char txt1[1024*4];
	char txt2[1024*4];

	param.getDebugString(txt1);
	p.getDebugString(txt2);
#endif			

			if(p.eval( (const NLIASCRIPT::CParam &)param ) >= 0.0)
			{
				NLIASCRIPT::COperandVoid typeR;
				NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
				t->incRef();
				r.push(CIdMethod((IAgent::getMethodIndexSize() + sendTag),0.0,NULL,t));
			}			
			return r;
		}
		else
		if(*methodName == gChild)
		{
			tQueue r;			

			NLIASCRIPT::CParam p;
			NLIASCRIPT::COperandSimple m(new NLAIC::CIdentType(CStringType::IdStringType));
			m.incRef();
			m.incRef();
			p.push(&m);		
					
			if(p.eval( (const NLIASCRIPT::CParam &)param ) >= 0.0)
			{
				NLIASCRIPT::COperandSimple typeR(new NLAIC::CIdentType(IAgent::IdAgent));
				NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
				t->incRef();
				
				r.push(CIdMethod((IAgent::getMethodIndexSize() + getChildTag),0.0,NULL,t));
			}
				
			return r;		
		}
		else
		if(*methodName == aChild)
		{
			tQueue r;	
			NLIASCRIPT::CParam p;
			NLIASCRIPT::COperandSimple m(new NLAIC::CIdentType(CStringType::IdStringType));
			NLIASCRIPT::COperandSimple n(new NLAIC::CIdentType("Agent"));

			m.incRef();
			m.incRef();
			p.push(&m);

			n.incRef();
			n.incRef();
			p.push(&n);

			if(p.eval( (const NLIASCRIPT::CParam &)param ) >= 0.0)
			{
				NLIASCRIPT::COperandSimple typeR(new NLAIC::CIdentType(DigitalType::IdDigitalType));
				NLIASCRIPT::CObjectUnknown *t = new NLIASCRIPT::CObjectUnknown((NLIASCRIPT::IOpType *)typeR.clone());
				t->incRef();
				r.push(CIdMethod((IAgent::getMethodIndexSize() + addChildTag),0.0,NULL,t));
			}
			return r;
		}
		else
		if(_AgentClass != NULL)
		{
			tQueue r = _AgentClass->isMember(className,methodName,param);
			if(r.size() != 0) return r;
		}

		return IAgent::isMember(className,methodName,param);
	}

	sint32 CAgentScript::isClassInheritedFrom(const IVarName &name) const
	{
		return _AgentClass->isClassInheritedFrom( name );
	}	

	sint32 CAgentScript::getStaticMemberIndex(const IVarName &name) const
	{
		return _AgentClass->getInheritedStaticMemberIndex(name);
	}
}