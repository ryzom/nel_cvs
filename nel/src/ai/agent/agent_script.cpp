/** \file agent_script.cpp
 *
 * $Id: agent_script.cpp,v 1.13 2001/01/18 17:53:51 chafik Exp $
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
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/agent_manager.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/message_script.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/agent/agent_nombre.h"

namespace NLAIAGENT
{	
	static CGroupType listBidon;

	static NLAISCRIPT::COperandSimpleListOr *msgType = new NLAISCRIPT::COperandSimpleListOr(3,	
																							new NLAIC::CIdentType(CMessage::IdMessage),
																							new NLAIC::CIdentType(CMessageVector::IdMessageVector),
																							new NLAIC::CIdentType(NLAISCRIPT::CMessageClass::IdMessageClass));

	static NLAISCRIPT::CParam SendParamMessageScript(2, new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(IntegerType::IdIntegerType)), msgType);


	CAgentScript::CMethodCall CAgentScript::StaticMethod[] = 
	{

		CAgentScript::CMethodCall(	_SEND_, 
									CAgentScript::TSend, &SendParamMessageScript,
									CAgentScript::CheckAll,
									2,
									new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid)),

		///Send with continuation arg count must be 3.
		CAgentScript::CMethodCall(	_SEND_, 
									CAgentScript::TSendContinuation, 
									NULL,CAgentScript::CheckCount,
									3,
									new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid)),

		CAgentScript::CMethodCall(	_GETCHILD_, 
									CAgentScript::TGetChildTag, 
									NULL,
									CAgentScript::CheckCount,
									1,
									new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(IAgent::IdAgent)))),

		CAgentScript::CMethodCall(	_ADDCHILD_, 
									CAgentScript::TAddChildTag, 
									NULL,CAgentScript::CheckCount,
									2,
									new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(DigitalType::IdDigitalType))))
	};

	CAgentScript::CAgentScript(const CAgentScript &a): IAgentManager(a)
	{
		if ( a._AgentClass )
		{
			_AgentClass = a._AgentClass;
			a._AgentClass->incRef();
		}

		_NbComponents = a._NbComponents;
		_Components = new IObjectIA *[ _NbComponents ];
		sint32 nb_scripted = 0;
		for ( int i = 0; i < _NbComponents; i++ )
		{
			_Components[i] = (IObjectIA *)a._Components[i]->clone();		

			if(((const NLAIC::CTypeOfObject &)_Components[i]->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
			{
				///incRef();
				((CAgentScript *)_Components[i])->setParent( (const IWordNumRef *) *this);
				( (CScriptMailBox *) ((CAgentScript *)_Components[i])->getLocalMailBox() )->setIndex(nb_scripted);
				nb_scripted++;
			}
		}

		if(	a._ScriptMail) 
		{
			_ScriptMail = (IMailBox *)a._ScriptMail->clone();
			_ScriptMail->setParent( (const IWordNumRef *) *this );
		}
		else 
		{
			_ScriptMail = new CScriptMailBox((const IWordNumRef *) *this);
		}
		
		_AgentManager = a._AgentManager;
		//if(_AgentManager) _AgentManager->incRef();

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
		//MangerIsReferenced = false;
		_ScriptMail = new CScriptMailBox((const IWordNumRef *) *this);
	}

	CAgentScript::CAgentScript(IAgentManager *manager, IBasicAgent *father,//The agent's father 
							  std::list<IObjectIA *> &components,	//Static components							  
							  NLAISCRIPT::CAgentClass *agent_class )	//Class
	
	: IAgentManager(father), _AgentClass( agent_class )
	{	
		if ( _AgentClass )
			_AgentClass->incRef();

		// Creates the static components array
		_NbComponents = components.size();
		_Components = new IObjectIA *[ _NbComponents ];
		std::list<IObjectIA *>::iterator it_c = components.begin();
		int id_c = 0;
		sint32 nb_scripted = 0;
		_AgentManager = manager;
		while ( it_c != components.end() )
		{
			IObjectIA *o = (IObjectIA *)*it_c;
			_Components[id_c] = o;

			if(((const NLAIC::CTypeOfObject &)o->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
			{
				((CAgentScript *)o)->setParent( (const IWordNumRef *) *this);
				( (CScriptMailBox *) ((CAgentScript *)_Components[id_c])->getLocalMailBox() )->setIndex( nb_scripted );
				nb_scripted++;
			}

			it_c++;
			id_c++;
		}		
		_ScriptMail = new CScriptMailBox( (const IWordNumRef *) *this);
		
	}	

	CAgentScript::~CAgentScript()
	{
		if ( _AgentClass )
			_AgentClass->release();

		// destruction of static components
		for ( int i = 0; i < _NbComponents; i++ )
			_Components[i]->release();
		delete[] _Components;
		
		//if(_AgentManager != NULL) _AgentManager->release();
		
		if ( _ScriptMail != NULL )
			_ScriptMail->release();
	}

	void CAgentScript::setAgentManager(IAgentManager *manager)
	{
		//if(_AgentManager != NULL) _AgentManager->release();
		_AgentManager = manager;
		for ( int i = 0; i < _NbComponents; i++ )
		{
			if(((const NLAIC::CTypeOfObject &)_Components[i]->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
			{
				//_AgentManager->incRef();
				((CAgentScript *)_Components[i])->setAgentManager(_AgentManager);
			}
		}
			
	}

	sint32 CAgentScript::getChildMessageIndex(const IMessageBase *msg, sint32 child_index )
	{
		return _AgentClass->getChildMessageIndex( msg, child_index );
	}

	void CAgentScript::setStaticMember(sint32 index,IObjectIA *op)
	{
#ifdef NL_DEBUG
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
#ifdef NL_DEBUG
		if ( index >= _NbComponents )
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return _Components[ index ];
	}

	NLAISCRIPT::IOpCode &CAgentScript::getMethode(sint32 inheritance,sint32 index)
	{
#ifdef NL_DEBUG
		if ( index >= _AgentClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}

		if ( inheritance >= _AgentClass->sizeVTable())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLAISCRIPT::IOpCode &)_AgentClass->getBrancheCode(inheritance,index).getCode();
	}

	NLAISCRIPT::IOpCode &CAgentScript::getMethode(sint32 index)
	{
#ifdef NL_DEBUG
		if ( index >= _AgentClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLAISCRIPT::IOpCode &)_AgentClass->getBrancheCode(index).getCode();
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
			_DynamicAgentName.insert(tPairName(s,addChild(o)));
		}
		r.Result = NULL;

		return r;
	}

	IObjectIA::CProcessResult CAgentScript::sendMethod(IObjectIA *param)
	{
		INombreDefine *p = (INombreDefine *)((IBaseGroupType *)param)->popFront();
		IMessageBase *msg = (IMessageBase *)((IBaseGroupType *)param)->popFront();
		msg->setPerformatif((IMessageBase::TPerformatif)(sint)p->getNumber());
		p->release();
		return sendMessage(msg);
	}

	IObjectIA::CProcessResult CAgentScript::sendMethodContinuation(IObjectIA *param)
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
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*_AgentManager->getAgentContext();
		while(_ScriptMail->getMessageCount())
		{
			IMessageBase &msg = (IMessageBase &)_ScriptMail->getMessage();
			IBaseGroupType *param = new CGroupType();
			msg.incRef();
			param->push(&msg);
			context.Stack ++;
			context.Stack[(int)context.Stack] = param;
			if(msg.getMethodIndex() >= 0)
			{
				//IObjectIA::CProcessResult r;

				NLAISCRIPT::IMethodContext *methodContex;				

				if (context.ContextDebug.Active)
				{
					context.ContextDebug.Param.push_back(&listBidon);					
					listBidon.incRef();
					methodContex = new NLAISCRIPT::CMethodContextDebug();
				}				
				else
				{
					methodContex = new NLAISCRIPT::CMethodContext();
				}
				const IObjectIA *self = context.Self;
				context.Self = this;
				NLAISCRIPT::CCallMethod opCall(methodContex,msg.getHeritanceIndex(),msg.getMethodIndex());
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

#ifdef _DEBUG
		const char *dbg_class_name = (const char *) getType();
		const NLAIAGENT::IRefrence *dbg_mail_parent = _ScriptMail->getParent();
#endif

		_ScriptMail->run();
		getMail()->run();
		runChildren();
		
		processMessages();

		if(_AgentClass->getRunMethod() >= 0) 
		{
			NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *)_AgentManager->getAgentContext();
			context->Self = this;
			runMethodeMember(_AgentClass->getRunMethod(), context);
		}

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
		switch(index - IAgent::getMethodIndexSize())
		{
		case TSend:
			{
				return sendMethod(o);
			}

		case TSendContinuation:
			{
				return sendMethodContinuation(o);
			}

		case TGetChildTag:
			{
				return getDynamicAgent((IBaseGroupType *)o);
			}

		case TAddChildTag:
			{
				return addDynamicAgent((IBaseGroupType *)o);
			}
		default:
			return IAgent::runMethodeMember(heritance,index,o);
		}
		
	}

	IObjectIA::CProcessResult CAgentScript::runMethodBase(int index,IObjectIA *o)
	{		
		switch(index - IAgent::getMethodIndexSize())
		{
		case TSend:
			{
				return sendMethod(o);
			}

		case TSendContinuation:
			{
				return sendMethodContinuation(o);
			}

		case TGetChildTag:
			{
				return getDynamicAgent((IBaseGroupType *)o);
			}

		case TAddChildTag:
			{
				return addDynamicAgent((IBaseGroupType *)o);
			}
		default:
			return IAgent::runMethodeMember(index,o);
		}
	}

	int CAgentScript::getBaseMethodCount() const
	{
		return IAgentManager::getBaseMethodCount() + CAgentScript::TLastM;
	}

	IObjectIA::CProcessResult CAgentScript::runMethodeMember(sint32 inheritance, sint32 index, IObjectIA *c)	
	{

		if(c->getType() != NLAISCRIPT::CCodeContext::IdCodeContext)
		{
			return IAgent::runMethodeMember(inheritance,index, c);
		}
		
		NLAISCRIPT::IOpCode *opPtr = NULL;
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;

		int i = index - getBaseMethodCount();
		if(i < 0)
		{			
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
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
		
		NLAISCRIPT::IOpCode &op = *opPtr;
		NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
		int ip = (uint32)*context.Code;
		context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
		*context.Code = 0;

		/*TProcessStatement k = IObjectIA::ProcessIdle;

		while(k != IObjectIA::ProcessEnd)
		{
			k = op.run(context);
		}*/

		IObjectIA::CProcessResult r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);
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
		if(c->getType() != NLAISCRIPT::CCodeContext::IdCodeContext)
		{
			return IAgent::runMethodeMember(index, c);
		}

		NLAISCRIPT::IOpCode *opPtr = NULL;
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;

		int i = index - getBaseMethodCount();
		if(i < 0)
		{
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
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
		
		NLAISCRIPT::IOpCode &op = *opPtr;//getMethode(inheritance,i);
		NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
		int ip = (uint32)*context.Code;
		context.Code = (NLAISCRIPT::CCodeBrancheRun *)opPtr;		
		*context.Code = 0;

		/*TProcessStatement k = IObjectIA::ProcessIdle;

		while(k != IObjectIA::ProcessEnd)
		{
			k = op.run(context);	
		}*/		
		IObjectIA::CProcessResult r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);

		// If we are in Debug Mode
		if (context.ContextDebug.Active)
		{
			context.ContextDebug.callStackPop();
		}

		*context.Code = ip;
		context.Code = opTmp;		
		return r;
	}

	tQueue CAgentScript::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{			
		CAgentScript::TMethodNumDef index = CAgentScript::TLastM;

		if(className == NULL || *className == CStringVarName((const char *)IdAgentScript))
		{
			sint i;
			for(i = 0; i < CAgentScript::TLastM; i ++)
			{
				if(CAgentScript::StaticMethod[i].MethodName == *methodName)
				{
					index = (CAgentScript::TMethodNumDef)CAgentScript::StaticMethod[i].Index;
					switch(CAgentScript::StaticMethod[i].CheckArgType)
					{
					case CAgentScript::CheckAll:
						{
							double d = ((NLAISCRIPT::CParam &)*CAgentScript::StaticMethod[i].ArgType).eval((NLAISCRIPT::CParam &)param);
							if(d >= 0.0)
							{								
								tQueue r;
								CAgentScript::StaticMethod[i].ReturnValue->incRef();
								r.push(CIdMethod(	(IAgent::getMethodIndexSize() + CAgentScript::StaticMethod[i].Index),
													0.0,
													NULL,
													CAgentScript::StaticMethod[i].ReturnValue));
								return r;
							}
						}
						break;
					

					case CAgentScript::CheckCount:
						{
							if(((NLAISCRIPT::CParam &)param).size() == CAgentScript::StaticMethod[i].ArgCount)
							{								
								tQueue r;
								CAgentScript::StaticMethod[i].ReturnValue->incRef();
								r.push(CIdMethod(	(IAgent::getMethodIndexSize() + CAgentScript::StaticMethod[i].Index),
													0.0,
													NULL,
													CAgentScript::StaticMethod[i].ReturnValue));
								return r;
							}
						}
						break;

					case CAgentScript::DoNotCheck:
						{							
							tQueue r;
							CAgentScript::StaticMethod[i].ReturnValue->incRef();
							r.push(CIdMethod(	(IAgent::getMethodIndexSize() + CAgentScript::StaticMethod[i].Index),
												0.0,
												NULL,
												CAgentScript::StaticMethod[i].ReturnValue));
							return r;						
						}
						break;
					}
				}
				
			}
		}
		if(index == CAgentScript::TLastM) return IAgent::isMember(className,methodName,param);
		return tQueue();
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