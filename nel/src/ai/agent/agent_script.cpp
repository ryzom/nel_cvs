/** \file agent_script.cpp
 *
 * $Id: agent_script.cpp,v 1.73 2001/07/06 08:26:59 chafik Exp $
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
#include "nel/ai/nl_ai.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/agent_manager.h"
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/message_script.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/interpret_message_action.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/agent/agent_nombre.h"
#include "nel/ai/agent/performative.h"
#include "nel/ai/agent/msg_notify.h"
#include "nel/ai/agent/msg_goal.h"
#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/goal.h"
#include "nel/ai/agent/key_agent.h"
#include "nel/ai/agent/list_manager.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/agent/msg_fact.h"

//#define PROFILE
#ifdef PROFILE
#include "nel/misc/time_nl.h"
#endif


namespace NLAIAGENT
{	
	//static CGroupType listBidon;

	NLAISCRIPT::COperandSimpleListOr *CAgentScript::msgType = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::msgPerf = NULL;
	NLAISCRIPT::CParam *CAgentScript::SendParamMessageScript = NULL;
	NLAISCRIPT::CParam *CAgentScript::SendCompParamMessageScript = NULL;
	NLAISCRIPT::COperandSimple *CAgentScript::IdMsgNotifyParentClass = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::IdMsgNotifyParent = NULL;
	NLAISCRIPT::CParam *CAgentScript::ParamRunParentNotify = NULL;
	CAgentScript::CMethodCall **CAgentScript::StaticMethod = NULL;


	void CAgentScript::initAgentScript()
	{

		std::string msgStr;
		std::string scriptName("MsgAgentScript");

		msgStr = std::string("From Message : Define MsgTellCompoment\n{");
		msgStr += std::string("Component:\n");		
		msgStr += std::string("\tString<'CompomentName'>;\n");
		msgStr += std::string("\tMessage<'MsgType'>;\n");
		msgStr += std::string("End\n");

		msgStr += std::string("Constructor(String i; Message msg)\n");
		msgStr += std::string("\tCompomentName = i;\n");
		msgStr += std::string("\tMsgType = msg;\n");
		msgStr += std::string("End\n");
		msgStr += std::string("}\n");
		NLAILINK::buildScript(msgStr,scriptName);

		
		msgType = new NLAISCRIPT::COperandSimpleListOr(3,	
														new NLAIC::CIdentType(CMessageList::IdMessage),
														new NLAIC::CIdentType(CMessageVector::IdMessageVector),
														new NLAIC::CIdentType(NLAISCRIPT::CMessageClass::IdMessageClass));

		msgPerf = new NLAISCRIPT::COperandSimpleListOr(7,	
														new NLAIC::CIdentType(CPExec::IdPExec),
														new NLAIC::CIdentType(CPAchieve::IdPAchieve),
														new NLAIC::CIdentType(CPAsk::IdPAsk),
														new NLAIC::CIdentType(CPBreak::IdPBreak),
														new NLAIC::CIdentType(CPTell::IdPTell),
														new NLAIC::CIdentType(CPKill::IdPKill),
														new NLAIC::CIdentType(CPError::IdPError));


		SendParamMessageScript = new NLAISCRIPT::CParam(2,msgPerf, msgType);

		msgPerf->incRef();
		msgType->incRef();

		SendCompParamMessageScript = new NLAISCRIPT::CParam(3,new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(CStringType::IdStringType)),
														msgPerf, msgType);

		IdMsgNotifyParentClass = new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(NLAISCRIPT::CMsgNotifyParentClass::IdMsgNotifyParentClass));

		IdMsgNotifyParent = new NLAISCRIPT::COperandSimpleListOr(2,
																new NLAIC::CIdentType(NLAISCRIPT::CMsgNotifyParentClass::IdMsgNotifyParentClass),
																new NLAIC::CIdentType(CNotifyParentScript::IdNotifyParentScript));

		ParamRunParentNotify = new NLAISCRIPT::CParam(1,IdMsgNotifyParent);



		StaticMethod = new CAgentScript::CMethodCall *[CAgentScript::TLastM];

		StaticMethod[CAgentScript::TRunAskParentNotify] = new CAgentScript::CMethodCall(	_RUNASK_, 
																						CAgentScript::TRunAskParentNotify, ParamRunParentNotify,
																						CAgentScript::CheckAll,
																						1,
																						new NLAISCRIPT::CObjectUnknown(IdMsgNotifyParentClass));

		IdMsgNotifyParentClass->incRef();		
		ParamRunParentNotify->incRef();
		StaticMethod[CAgentScript::TRunTellParentNotify] = new CAgentScript::CMethodCall(	_RUNTEL_, 
																						CAgentScript::TRunTellParentNotify, ParamRunParentNotify,
																						CAgentScript::CheckAll,
																						1,
																						new NLAISCRIPT::CObjectUnknown(IdMsgNotifyParentClass));

		StaticMethod[CAgentScript::TSend] = new CAgentScript::CMethodCall(	_SEND_, 
																		CAgentScript::TSend, SendParamMessageScript,
																		CAgentScript::CheckAll,
																		2,
																		new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

		StaticMethod[CAgentScript::TSendComponent] = new CAgentScript::CMethodCall(	_SEND_, 
																		CAgentScript::TSendComponent, 
																		SendCompParamMessageScript,CAgentScript::CheckAll,
																		3,
																		new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

		StaticMethod[CAgentScript::TGetChildTag] = new CAgentScript::CMethodCall(	_GETCHILD_,
																				CAgentScript::TGetChildTag, 
																				NULL,
																				CAgentScript::CheckCount,
																				1,
																				new NLAISCRIPT::CObjectUnknown(new 
																				NLAISCRIPT::COperandSimple(
																				new NLAIC::CIdentType(CVectorGroupManager::IdVectorGroupManager))));

		StaticMethod[CAgentScript::TAddChildTag] = new CAgentScript::CMethodCall(	_ADDCHILD_, 
																						CAgentScript::TAddChildTag, 
																						NULL,CAgentScript::CheckCount,
																						2,
																						new NLAISCRIPT::CObjectUnknown(
																						new NLAISCRIPT::COperandSimple(
																						new NLAIC::CIdentType(DigitalType::IdDigitalType))));

		StaticMethod[CAgentScript::TFather] = new CAgentScript::CMethodCall(	_FATHER_, 
																			CAgentScript::TFather, 
																			NULL,CAgentScript::CheckCount,
																			0,
																			new NLAISCRIPT::CObjectUnknown(
																			new NLAISCRIPT::COperandSimple(
																			//check if correct
																			new NLAIC::CIdentType(*IAgent::IdAgent))));	


		StaticMethod[CAgentScript::TSelf] = new CAgentScript::CMethodCall(	_SELF_, 
																		CAgentScript::TSelf, 
																		NULL,CAgentScript::CheckCount,
																		0,
																		new NLAISCRIPT::CObjectUnknown( new NLAISCRIPT::COperandSimple(
																		new NLAIC::CIdentType(CAgentScript::IdAgentScript))));

		StaticMethod[CAgentScript::TGetName] = new CAgentScript::CMethodCall(	_GETNAME_, 
																			CAgentScript::TGetName, 
																			NULL,CAgentScript::CheckCount,
																			1,
																			new NLAISCRIPT::CObjectUnknown(
																			new NLAISCRIPT::COperandSimple(
																			new NLAIC::CIdentType(CStringType::IdStringType))));

		StaticMethod[CAgentScript::TRemoveChild] = new CAgentScript::CMethodCall(	_REMOVECHILD_, 
																				CAgentScript::TRemoveChild, 
																				NULL,CAgentScript::CheckCount,
																				0,
																				new NLAISCRIPT::CObjectUnknown(
																				new NLAISCRIPT::COperandSimple(
																				new NLAIC::CIdentType(DigitalType::IdDigitalType))));		
	}

	void CAgentScript::releaseAgentScript()
	{		
		//SendParamMessageScript->release();
		//IdMsgNotifyParentClass->release();		
	////////////////////////////////////////////////////////////////////////
	// Temp, to be transfered in CGDAgentScript (Goal Driven Agent)
	// IdGoalMsgClass->release();
	// IdGoalMsg->release();
		//ParamGoalMsg->release();
		//ParamCancelGoalMsg->release();
	////////////////////////////////////////////////////////////////////////

		//ParamRunParentNotify->release();
		//SendCompParamMessageScript->release();
		sint i;
		for(i = 0; i < CAgentScript::TLastM; i++)
		{
				delete StaticMethod[i];
		}
		delete StaticMethod;
	}

	CAgentScript::CAgentScript(const CAgentScript &a): IAgentManager(a)
	{
		_iComponents = 0;

		_AgentClass = a._AgentClass;
		if ( a._AgentClass )
		{			
			a._AgentClass->incRef();
		}

		_NbComponents = a._NbComponents;
		if ( _NbComponents != 0 )
		{
			_Components = new IObjectIA *[ _NbComponents ];
			//sint32 nb_scripted = 0;
			for ( int i = 0; i < _NbComponents; i++ )
			{
				_Components[i] = (IObjectIA *)a._Components[i]->clone();		

				if(((const NLAIC::CTypeOfObject &)_Components[i]->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
				{
					///incRef();
					((CAgentScript *)_Components[i])->setParent( (const IWordNumRef *) *this);
				}
			}
		}
		else
			_Components = NULL;

		_AgentManager = a._AgentManager;
		//if(_AgentManager) _AgentManager->incRef();

/*		std::vector<NLAILOGIC::IBaseOperator *>::const_iterator it_o = a._Operators.begin();
		while ( it_o != a._Operators.end() )
		{
			addOperator( *it_o );
			it_o++;
		}*/
	}
	
	CAgentScript::CAgentScript(IAgentManager *manager) : IAgentManager( NULL ), _AgentClass( NULL )
	{
		_iComponents = 0;
		_Components = NULL;
		_NbComponents = 0;		
		_AgentManager = manager;
	}

	CAgentScript::CAgentScript(IAgentManager *manager, IBasicAgent *father,		//The agent's father 
							  std::list<IObjectIA *> &components,				//Static components							  
							  NLAISCRIPT::CAgentClass *agent_class )			//Class
	
	: IAgentManager(father), _AgentClass( agent_class )
	{	
		if ( _AgentClass )
			_AgentClass->incRef();

		_iComponents = 0;

		// Creates the static components array
		_NbComponents = components.size();
		if ( _NbComponents )
		{
			_Components = new IObjectIA *[ _NbComponents ];
			std::list<IObjectIA *>::iterator it_c = components.begin();
			int id_c = 0;
			//sint32 nb_scripted = 0;
			_AgentManager = manager;
			while ( it_c != components.end() )
			{
				IObjectIA *o = (IObjectIA *)*it_c;
				_Components[id_c] = o;

				if(((const NLAIC::CTypeOfObject &)o->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
				{
					((CAgentScript *)o)->setParent( (const IWordNumRef *) *this);
				}

				it_c++;
				id_c++;
			}		
		}
		else
			_Components =NULL;
	}	

	CAgentScript::~CAgentScript()
	{
		if ( _AgentClass )
			_AgentClass->release();

		// destruction of static components
		if(_Components != NULL)
		{
			for ( int i = 0; i < _NbComponents; i++ )
				_Components[i]->release();
			delete[] _Components;
		}
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
		IObjectIA *old_comp = _Components[ index ];
		_Components[ index ] = op;
		op->incRef();
		old_comp->release();



		if(((const NLAIC::CTypeOfObject &) op->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
		{
			((CAgentScript *)op)->setParent( (const IWordNumRef *) *this);
		}
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

	NLAISCRIPT::IOpCode *CAgentScript::getMethode(sint32 inheritance,sint32 index)
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
		return (NLAISCRIPT::IOpCode *)_AgentClass->getBrancheCode(inheritance,index).getCode();
	}

	NLAISCRIPT::IOpCode *CAgentScript::getMethode(sint32 index)
	{
#ifdef NL_DEBUG		
		const char *dbg_class_name = (const char *) getType();
		const char *dbg_base_class_name = (const char *) _AgentClass->getType();

		if ( index >= _AgentClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLAISCRIPT::IOpCode *)_AgentClass->getBrancheCode(index).getCode();
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
		/*if(_AgentClass)
		{
			return _AgentClass->getMethodIndexSize();
		}
		else*/
		{
			return IAgent::getMethodIndexSize() + TLastM;
		}
	}

	const NLAIC::IBasicType *CAgentScript::clone() const
	{		
		CAgentScript *result = new CAgentScript(*this);
		return result;
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

	void CAgentScript::getDebugString(std::string &t) const
	{		
		t += NLAIC::stringGetBuild("class type <%s> ",(const char *)getType());
		if ( _AgentClass )
			t += NLAIC::stringGetBuild("<%s> (scripted)  -StaticComponents:\n",(const char *)_AgentClass->getType());
		else
			t += "<undefined_class> (scripted) -StaticComponents:\n";

			std::string buf;
		for (int i = 0; i < _NbComponents; i++ )
		{
			t += "\t\t";
			if ( _AgentClass->getComponentName(i) )
			{
				t += _AgentClass->getComponentName(i);
			}
			else
				t += "<unnamed>";

			t += "\t\t";
			_Components[i]->getDebugString(buf);
			t += "[ ";
			t += buf;
			t += " ]";
			if(i != (_NbComponents-1)) t += "\n";
		}
	}

	bool CAgentScript::isEqual(const IBasicObjectIA &a) const
	{
		return false;
	}

	void CAgentScript::onKill(IConnectIA *a)
	{		
		tsetDefNameAgent::iterator iter = _DynamicAgentName.begin();
		while( iter != _DynamicAgentName.end() )
		{			
			if((*(*iter).Itr) == a)
			{
				_DynamicAgentName.erase(iter);
				break;
			}
			iter++;
		}
		IAgent::onKill(a);
	}

	IObjectIA::CProcessResult CAgentScript::addDynamicAgent(IBaseGroupType *g)
	{
		CIteratorContener i = g->getIterator();
		CStringType &s = (CStringType &)*i++;
		IBasicAgent *o = (IBasicAgent *)i++;

		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;
		
		o->setParent( (const IWordNumRef *) *this );
		CNotifyParentScript *m = new CNotifyParentScript(this);
		//this->incRef();
		m->setSender(this);
		m->setPerformatif(IMessageBase::PTell);
		((IObjectIA *)o)->sendMessage(m);

		if(((const NLAIC::CTypeOfObject &)o->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
		{
			((CAgentScript *)o)->setAgentManager(this);
		}

		_DynamicAgentName.insert(CKeyAgent(s,addChild(o)));
		
		r.Result = NULL;

		return r;
	}

	IObjectIA::CProcessResult CAgentScript::removeDynamic(NLAIAGENT::IBaseGroupType *g)
	{
		CStringType *s = (CStringType *)g->get();		
		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;
		std::pair<tsetDefNameAgent::iterator,tsetDefNameAgent::iterator>  p = _DynamicAgentName.equal_range(CKeyAgent(*s));

		if(p.first != p.second)
		{	
			tsetDefNameAgent::iterator debut = p.first;
			while(p.first != p.second)
			{
				tsetDefNameAgent::iterator iTmp = p.first;
				p.first ++;

				NLAIAGENT::IBasicAgent *o = *iTmp->Itr;
				_DynamicAgentName.erase(iTmp);				
				removeChild(o);				
			}			
			r.Result = new DigitalType(1.0);
			return r;
		}		
		r.Result = &DigitalType::NullOperator;
		r.Result->incRef();
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::getDynamicAgent(IBaseGroupType *g)
	{		
		CStringType *s = (CStringType *)g->get();
		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;
		std::pair<tsetDefNameAgent::iterator,tsetDefNameAgent::iterator>  p = _DynamicAgentName.equal_range(CKeyAgent(*s));
		
		if(p.first != p.second)
		{			
			sint size = _DynamicAgentName.count(CKeyAgent(*s));
			sint n = 0;
			CVectorGroupManager *x;
			x = new CVectorGroupManager(size);
			while(p.first != p.second)
			{				
				(*p.first->Itr)->incRef();
				x->set(n++, *p.first->Itr);
				p.first++;
			}

			r.Result = x;			
			return r;
			
		}
		r.Result = new CVectorGroupManager();		
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runAskParentNotify(IBaseGroupType *g)
	{
		CNotifyParentScript *m = new CNotifyParentScript((IBasicAgent *)getParent());
		m->setPerformatif(IMessageBase::PTell);
		//this->incRef();
		m->setSender(this);
		IObjectIA::CProcessResult r;
		r.Result = m;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runTellParentNotify(IBaseGroupType *g)
	{	
		sint i;

		CNotifyParentScript *m = new CNotifyParentScript(this);
		m->setPerformatif(IMessageBase::PTell);
		//this->incRef();
		m->setSender(this);

		for(i = 0; i < _NbComponents; i++)
		{
			if(_Components[i] != NULL)
			{				
				CNotifyParentScript *msg = (CNotifyParentScript *)m->clone();
				try
				{
					_Components[i]->sendMessage(msg);					
				}
				catch(NLAIE::IException &)
				{
					msg->release();
				}
			}
		}
		IObjectIA::CProcessResult r;
		r.Result = m;
		return r;
	}	

	IObjectIA::CProcessResult CAgentScript::getDynamicName(NLAIAGENT::IBaseGroupType *g)
	{	
#ifdef NL_DEBUG
		std::string txt;
	g->getDebugString(txt);
#endif

		IObjectIA::CProcessResult r;
		const IObjectIA *o = ((CLocalAgentMail *)g->get())->getHost();
		tsetDefNameAgent::iterator i = _DynamicAgentName.begin();

		while(i != _DynamicAgentName.end())
		{
			CKeyAgent key = *i;
			if( o == *key.Itr )
			{
				CStringType *s = new CStringType(key.Name);
				r.Result = s;
				return r;
			}
			i ++;
		}
		r.Result = new CStringType(CStringVarName("Unknown"));
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

	IObjectIA::CProcessResult CAgentScript::sendMethodCompoment(IObjectIA *param)
	{
		NLAIAGENT::CIteratorContener It = ((IBaseGroupType *)param)->getIterator();
		const CStringType *n = (const CStringType *)It++;
		INombreDefine *p = (INombreDefine *)((IBaseGroupType *)It++);
		IMessageBase *msg = (IMessageBase *)((IBaseGroupType *)It++);
		msg->setPerformatif((IMessageBase::TPerformatif)(sint)p->getNumber());
		msg->incRef();
		return sendMessage(n->getStr(),msg);
	}	
	
	IObjectIA::CProcessResult CAgentScript::sendMessageToDynmaicChild(const IVarName &compName,IObjectIA *msg)
	{
		//tsetDefNameAgent::iterator  p = _DynamicAgentName.find(CKeyAgent(CStringType(compName)));
		std::pair<tsetDefNameAgent::iterator,tsetDefNameAgent::iterator>  p = _DynamicAgentName.equal_range(CKeyAgent(CStringType(compName)));

#ifdef NL_DEBUG
	const char *txt = (const char *)msg->getType();
#endif

		while(p.first != p.second)
		{
			CAgentScript *o = (CAgentScript *)*((*(p.first)).Itr);
#ifdef NL_DEBUG	
	const char *compNameDb = (const char *)o->getType();
#endif
			o->sendMessage(msg);
			p.first ++;
			if(p.first != p.second) msg = (IObjectIA *)msg->clone();
		}

		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CAgentScript::sendMessage(const IVarName &compName,IObjectIA *msg)
	{
#ifdef NL_DEBUG
	const char *txt = (const char *)msg->getType();
	const char *compNameDb = (const char *)compName.getString();
#endif
		if(_AgentClass != NULL)
		{
			int i = getStaticMemberIndex(compName);
			if(i >= 0)
			{
				IObjectIA *comp = (IObjectIA *)getStaticMember(i);
				if(comp != NULL)
				{
					comp->sendMessage(msg);
				}
				else
				{
					sendMessageToDynmaicChild(compName,msg);
				}
			}
			else
			{
				sendMessageToDynmaicChild(compName,msg);
			}
		}
		else			
		{						
			sendMessageToDynmaicChild(compName,msg);
		}
		return IObjectIA::CProcessResult();
	}

#ifdef PROFILE
	NLMISC::TTicks TimeSend = 0;
	NLMISC::TTicks NbSend = 0;		
#endif

	IObjectIA::CProcessResult CAgentScript::sendMessage(IObjectIA *m)
	{
#ifdef NL_DEBUG
	const char *txt = (const char *)m->getType();
	const char *classBase = (const char *)getType();
#endif

#ifdef PROFILE
		NLMISC::TTicks time = NLMISC::CTime::getPerformanceTime();
#endif
		IMessageBase *msg = (IMessageBase *)m;
		//this->incRef();
		msg->setReceiver(this);
		if(msg->getMethodIndex() < 0)
		{			
			if( ((const NLAIC::CTypeOfObject &)m->getType()) & NLAIC::CTypeOfObject::tAgentInterpret)
			{
				//char runMsg[1024];
				//strcpy(runMsg,_RUN_);
				static CStringVarName sRunTell("RunTell");
				static CStringVarName sRunAchieve("RunAchieve");
				static CStringVarName sRunAsk("RunAsk");
				static CStringVarName sRunExec("RunExec");
				static CStringVarName sRunBreak("RunBreak");
				static CStringVarName sRunKill("RunKill");
				static CStringVarName sRunError("RunError");

				CStringVarName *runMsg = NULL;//sRunTell;

				switch(msg->getPerformatif())
				{
				case IMessageBase::PExec:
					//strcat(runMsg,"Exec");
					runMsg = &sRunExec;
					break;
				case IMessageBase::PAchieve:
					//strcat(runMsg,"Achieve");
					runMsg = &sRunAchieve;
					break;
				case IMessageBase::PAsk:
					//strcat(runMsg,"Ask");
					runMsg = &sRunAsk;
					break;
				case IMessageBase::PTell:
					//strcat(runMsg,"Tell");
					runMsg = &sRunTell;
					break;
				case IMessageBase::PBreak:
					runMsg = &sRunBreak;
					break;
				case IMessageBase::PKill:
					runMsg = &sRunKill;
					break;
				case IMessageBase::PError:
					runMsg = &sRunError;
					break;
				}

				CStringVarName &tmp_name = *runMsg;

				NLAISCRIPT::COperandSimple *t = new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(m->getType()));
				NLAISCRIPT::CParam p(1,t);
				
				tQueue r = isMember(NULL,&tmp_name,p);
				if(r.size())
				{
					NLAIAGENT::CIdMethod m = r.top();
					msg->setMethodIndex(0,m.Index);
					//_ScriptMail->addMessage(msg);
				}
				//else return IAgent::sendMessage(msg);
			}
			//else return IAgent::sendMessage(msg);			
		}

		IObjectIA::CProcessResult r = IAgent::sendMessage(msg);

#ifdef PROFILE
		time = NLMISC::CTime::getPerformanceTime() - time;
		//if(time)
		{
			TimeSend += time;
			NbSend ++;			
		}
#endif

		return r;
		//return IObjectIA::CProcessResult();
	}

	void CAgentScript::runChildren()
	{
#ifdef NL_DEBUG	
	const char *classBase = (const char *)getType();
#endif
		// Activation des agents de la partie statique
		for ( int i = 0; i < _NbComponents; i++ )
		{
			IObjectIA *o = _Components[i];
			o->run();
		}

		// Activation des fils
		IAgent::runChildren();
	}

	bool CAgentScript::runChildrenStepByStep()
	{
		if(_iComponents == _NbComponents)
		{
			if(IAgent::runChildrenStepByStep())
			{
				_iComponents = 0;
				return true;				
			}
			else return false;			
		}

		IObjectIA *o = _Components[_iComponents ++];
		o->run();

		return false;
	}

	void CAgentScript::processMessages(IMessageBase *msg,IObjectIA *c)
	{
#ifdef NL_DEBUG
	const char *txt = (const char *)msg->getType();
#endif
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;
		IBaseGroupType *param = new CGroupType();
		msg->incRef();
		param->push(msg);
		context.Stack ++;
		context.Stack[(int)context.Stack] = param;

		int indexM = msg->getMethodIndex() - getBaseMethodCount();
		if(indexM >= 0)
		{
			IObjectIA *code = getMethode(indexM);
			if(code == NULL)
			{
				getMail()->popMessage();
				return;
			}
		}

		NLAISCRIPT::IMethodContext *methodContex;				

		if (context.ContextDebug.Active)
		{
			/*context.ContextDebug.Param.push_back(&listBidon);					
			listBidon.incRef();*/
			methodContex = new NLAISCRIPT::CMethodContextDebug();
		}				
		else
		{
			methodContex = new NLAISCRIPT::CMethodContext();
		}
		const IObjectIA *self = context.Self;
		context.Self = this;
		NLAISCRIPT::CCallMethod opCall(methodContex,msg->getHeritanceIndex(),msg->getMethodIndex());
		opCall.runOpCode(context);
		context.Self = self;
		IMessageBase *returnMsg = (IMessageBase *)context.Stack[(int)context.Stack];
		returnMsg->incRef();
		context.Stack--;
		processContinuation(msg, returnMsg);

		returnMsg->release();
	}

	void CAgentScript::processContinuation(IMessageBase *msg, IMessageBase *returnMsg)
	{
		switch(msg->getPerformatif())
		{
		case IMessageBase::PExec:
			if(msg->getContinuation() != NULL)
			{
				IMessageBase *o = (IMessageBase *)returnMsg->clone();
				o->setMethodIndex(-1,-1);
				//this->incRef();
				o->setSender(this);
				//((IObjectIA *)msg->getContinuation())->incRef();
				o->setReceiver((IObjectIA *)msg->getContinuation());
				((IObjectIA *)msg->getContinuation())->sendMessage(o);
			}
			break;
		case IMessageBase::PAchieve:
			if(msg->getContinuation() != NULL)
			{
				IMessageBase *o = (IMessageBase *)returnMsg->clone();
				o->setMethodIndex(-1,-1);
				//this->incRef();
				o->setSender(this);
				//((IObjectIA *)msg->getContinuation())->incRef();
				o->setReceiver((IObjectIA *)msg->getContinuation());
				((IObjectIA *)msg->getContinuation())->sendMessage(o);
			}
			break;
		case IMessageBase::PAsk:
			{
				IMessageBase *o = (IMessageBase *)returnMsg->clone();
				o->setMethodIndex(-1,-1);
				//this->incRef();
				o->setSender(this);
				o->setPerformatif(IMessageBase::PTell);
				//if(returnMsg->getSender() != NULL) ((IObjectIA *)returnMsg->getSender())->incRef();
				o->setReceiver((IObjectIA *)returnMsg->getSender());
				((IObjectIA *)msg->getSender())->sendMessage(o);


				if(msg->getContinuation() != NULL)
				{
					IMessageBase *o = (IMessageBase *)returnMsg->clone();
					o->setMethodIndex(-1,-1);
					//this->incRef();
					o->setSender(this);
					//if(msg->getContinuation() != NULL) ((IObjectIA *)msg->getContinuation())->incRef();
					o->setReceiver((IObjectIA *)msg->getContinuation());
					o->setPerformatif(IMessageBase::PTell);
					((IObjectIA *)msg->getContinuation())->sendMessage(o);
				}
			}
			break;
		case IMessageBase::PTell:
			if(msg->getContinuation() != NULL)
			{
				IMessageBase *o = (IMessageBase *)returnMsg->clone();
				o->setMethodIndex(-1,-1);
				//this->incRef();
				o->setSender(this);
				//if(msg->getContinuation() != NULL) ((IObjectIA *)msg->getContinuation())->incRef();
				o->setReceiver((IObjectIA *)msg->getContinuation());
				((IObjectIA *)msg->getContinuation())->sendMessage(o);
			}
			break;
		case IMessageBase::PBreak:
			if(msg->getContinuation() != NULL)
			{
				IMessageBase *o = (IMessageBase *)returnMsg->clone();
				o->setMethodIndex(-1,-1);
				//this->incRef();
				o->setSender(this);
				//if(msg->getContinuation() != NULL) ((IObjectIA *)msg->getContinuation())->incRef();
				o->setReceiver((IObjectIA *)msg->getContinuation());
				((IObjectIA *)msg->getContinuation())->sendMessage(o);
			}
			break;
		case IMessageBase::PKill:
			if(msg->getContinuation() != NULL)
			{
				IMessageBase *o = (IMessageBase *)returnMsg->clone();
				o->setMethodIndex(-1,-1);
				//this->incRef();
				o->setSender(this);
				//if(msg->getContinuation() != NULL) ((IObjectIA *)msg->getContinuation())->incRef();
				o->setReceiver((IObjectIA *)msg->getContinuation());
				((IObjectIA *)msg->getContinuation())->sendMessage(o);
			}
			break;

		}
	}

	void CAgentScript::processMessages()
	{
		IObjectIA *c = NULL;
		if( _AgentManager != NULL) c = (IObjectIA *)_AgentManager->getAgentContext();
		else c = NULL;
		while(!getMail()->isEmpty())
		{
			IMessageBase &msg = (IMessageBase &)getMail()->getMessage();
#ifdef NL_DEBUG
		const char *dbg_msg = (const char *) msg.getType();		
#endif
			if(msg.getMethodIndex() >= 0 && c != NULL)
			{
				processMessages(&msg,c);
				getMail()->popMessage();
			}
			else 
			{
				IObjectIA *o = IBasicAgent::run( msg );
				getMail()->popMessage();
			}			
		}
	}

	IObjectIA::CProcessResult CAgentScript::runActivity()
	{
		
		NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *)_AgentManager->getAgentContext();
		context->Self = this;
		runMethodeMember(_AgentClass->getRunMethod(), context);
		
		return ProcessRun;
	}

	bool CAgentScript::haveActivity() const
	{
		return _AgentClass != NULL && (_AgentClass->getRunMethod() >= 0);
	}


#ifdef PROFILE
	NLMISC::TTicks TimeRun = 0;
	NLMISC::TTicks NbRun = 0;		
#endif

	const IObjectIA::CProcessResult &CAgentScript::run()
	{
		
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
		//const NLAIAGENT::IRefrence *dbg_mail_parent = _ScriptMail->getParent();
#endif

#ifdef PROFILE
		NLMISC::TTicks time = NLMISC::CTime::getPerformanceTime();
#endif

		const IObjectIA::CProcessResult &r = IAgentManager::run();

#ifdef PROFILE
		time = NLMISC::CTime::getPerformanceTime() - time;
		//if(time)
		{
			TimeRun = time;
			NbRun = 1;		
		}
#endif

		return r;
				
	}

	const IObjectIA::CProcessResult &CAgentScript::runStep()
	{
		
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
		//const NLAIAGENT::IRefrence *dbg_mail_parent = _ScriptMail->getParent();
#endif

		return IAgentManager::runStep();
				
	}

	void CAgentScript::addOperator(NLAILOGIC::IBaseOperator *op)
	{
//		_Operators.push_back(op);
	}

	void CAgentScript::remOperator(NLAILOGIC::IBaseOperator *op)
	{
/*		std::vector<NLAILOGIC::IBaseOperator *>::iterator it_o = _Operators.begin();
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
		*/
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

		case TSendComponent:
			{
				return sendMethodCompoment(o);
			}

		case TGetChildTag:
			{
				return getDynamicAgent((IBaseGroupType *)o);
			}

		case TAddChildTag:
			{
				return addDynamicAgent((IBaseGroupType *)o);
			}

		case TFather:
			{
				IObjectIA::CProcessResult a;
				a.Result = (IObjectIA *)getParent();
				a.Result->incRef();
				return a;
			}

		case TSelf:
			{
				IObjectIA::CProcessResult a;
				a.Result = new CLocalAgentMail(this);
				return a;
			}
		case TGetName:
			{				
				return getDynamicName((IBaseGroupType *)o);
			}

		case TRemoveChild:
			{
				return removeDynamic((IBaseGroupType *)o);
			}

		case TRunAskParentNotify:
			{				
				return runAskParentNotify((IBaseGroupType *)o);
			}

		case TRunTellParentNotify:
			{				
				return runTellParentNotify((IBaseGroupType *)o);
			}

		/*case TGoal:
			{				
				return runGoalMsg((IBaseGroupType *)o);
			}

		case TCancelGoal:
			{				
				return runCancelGoalMsg((IBaseGroupType *)o);
			}

		case TFact:
			{				
				return runFactMsg((IBaseGroupType *)o);
			}*/

	////////////////////////////////////////////////////////////////////////

		default:
			return IAgent::runMethodeMember(index,o);
				
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

		case TSendComponent:
			{
				return sendMethodCompoment(o);
			}

		case TGetChildTag:
			{
				return getDynamicAgent((IBaseGroupType *)o);
			}

		case TAddChildTag:
			{
				return addDynamicAgent((IBaseGroupType *)o);
			}
		case TFather:
			{
				IObjectIA::CProcessResult a;
				a.Result = (IObjectIA *)getParent();
				//if(a.Result != NULL) 
				a.Result->incRef();
				return a;
			}

		case TSelf:
			{
				IObjectIA::CProcessResult a;
				a.Result = new CLocalAgentMail(this);
				return a;
			}
		case TGetName:
			{				
				return getDynamicName((IBaseGroupType *)o);
			}

		case TRunAskParentNotify:
			{				
				return runAskParentNotify((IBaseGroupType *)o);
			}

		case TRunTellParentNotify:
			{				
				return runTellParentNotify((IBaseGroupType *)o);
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
			/*if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
				listBidon.incRef();
			}*/
		
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
			opPtr = getMethode(inheritance,i);
		}
		
		IObjectIA::CProcessResult r;
		if(opPtr != NULL)
		{
			NLAISCRIPT::IOpCode &op = *opPtr;
			NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
			int ip = (uint32)*context.Code;
			context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
			*context.Code = 0;

			r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);
			// If we are in Debug Mode
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.callStackPop();
			}
			*context.Code = ip;
			context.Code = opTmp;		
		}
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
			/*if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
				listBidon.incRef();
			}*/

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
			opPtr = getMethode(i);
		}
		IObjectIA::CProcessResult r;
		if(opPtr != NULL)
		{
			NLAISCRIPT::IOpCode &op = *opPtr;//getMethode(inheritance,i);
			NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
			int ip;
			if(context.Code != NULL) ip = (uint32)*context.Code;
			else ip =0;
			context.Code = (NLAISCRIPT::CCodeBrancheRun *)opPtr;
			*context.Code = 0;

			/*TProcessStatement k = IObjectIA::ProcessIdle;

			while(k != IObjectIA::ProcessEnd)
			{
				k = op.run(context);	
			}*/		
			r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);

			// If we are in Debug Mode
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.callStackPop();
			}

			*context.Code = ip;
			context.Code = opTmp;		
		}
		return r;
	}

	tQueue CAgentScript::getPrivateMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{		

#ifdef NL_DEBUG
		const char *dgb_meth_name = methodName->getString();
#endif
		/*sint i;
		CAgentScript::TMethodNumDef index = CAgentScript::TLastM;
		for(i = 0; i < CAgentScript::TLastM; i ++)
		{
			if(CAgentScript::StaticMethod[i]->MethodName == *methodName)
			{
				index = (CAgentScript::TMethodNumDef)CAgentScript::StaticMethod[i]->Index;
				switch(CAgentScript::StaticMethod[i]->CheckArgType)
				{
				case CAgentScript::CheckAll:
					{
						double d = ((NLAISCRIPT::CParam &)*CAgentScript::StaticMethod[i]->ArgType).eval((NLAISCRIPT::CParam &)param);
						if(d >= 0.0)
						{								
							tQueue r;
							CAgentScript::StaticMethod[i]->ReturnValue->incRef();
							r.push(CIdMethod(	(IAgent::getMethodIndexSize() + CAgentScript::StaticMethod[i]->Index),
												0.0,
												NULL,
												CAgentScript::StaticMethod[i]->ReturnValue));
							return r;
						}
					}	
					index = CAgentScript::TLastM;
					break;
				

				case CAgentScript::CheckCount:
					{
						if(((NLAISCRIPT::CParam &)param).size() == CAgentScript::StaticMethod[i]->ArgCount)
						{								
							tQueue r;
							CAgentScript::StaticMethod[i]->ReturnValue->incRef();
							r.push(CIdMethod(	(IAgent::getMethodIndexSize() + CAgentScript::StaticMethod[i]->Index),
												0.0,
												NULL,
												CAgentScript::StaticMethod[i]->ReturnValue ));
							return r;
						}
					}
					index = CAgentScript::TLastM;
					break;

				case CAgentScript::DoNotCheck:
					{							
						tQueue r;
						CAgentScript::StaticMethod[i]->ReturnValue->incRef();
						r.push(CIdMethod(	(IAgent::getMethodIndexSize() + CAgentScript::StaticMethod[i]->Index),
											0.0,
											NULL,
											CAgentScript::StaticMethod[i]->ReturnValue));
						return r;						
					}
					index = CAgentScript::TLastM;
					break;
				}
			}			
		}
		return tQueue();*/
		return isTemplateMember(CAgentScript::StaticMethod,CAgentScript::TLastM,IAgent::getMethodIndexSize(),className,methodName,param);		
	}

	tQueue CAgentScript::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{		
#ifdef NL_DEBUG	
		std::string nameM;
		std::string nameP;
		std::string name;
		methodName->getDebugString(nameM);
		param.getDebugString(nameP);
		name = nameM;
		name += nameP;

		const char *dbg_class_name = (const char *) getType();
		const char *dbg_base_class_name = NULL;
		if(_AgentClass) dbg_base_class_name = (const char *) _AgentClass->getType();
#endif
		

 		if(className == NULL)
		{

			tQueue r;
			if(_AgentClass != NULL) 
				r = _AgentClass->getPrivateMember(className,methodName,param);			

			if(r.size()) 
				return r;
			else
			{
				r = getPrivateMember(className,methodName,param);
				if(r.size()) return r;
				else
				{
					return IAgent::isMember(className,methodName,param);
				}
			}
		
		}		
		else 
		if(*className == CStringVarName("Agent"))
		{					
			tQueue r;
			r = getPrivateMember(className,methodName,param);
			if(r.size()) 
				return r;
			else
			{
				r = _AgentClass->getPrivateMember(className,methodName,param);			

				if ( r.size() ) 
					return r;
				else
				{			
					return IAgent::isMember(className,methodName,param);
				}
			}
			
		}
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

///////////////////////////////////////////////////////////////////////////////////////
/*
	
	IObjectIA::CProcessResult CAgentScript::runFactMsg(IBaseGroupType *g)
	{
		NLAILOGIC::CFact *fact = (NLAILOGIC::CFact *) g->get()->clone();
#ifdef NL_DEBUG
		char buffer[1024 * 2];
		fact->getDebugString( buffer );
#endif
		//_FactBase->addFact( fact );

		IObjectIA::CProcessResult r;
		r.Result = NULL;

		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*_AgentManager->getAgentContext();
		context.Stack++;
		context.Stack[(int)context.Stack] = new CFactMsg();
		return IObjectIA::CProcessResult();
	}

	NLAILOGIC::CFactBase &CAgentScript::getFactBase()
	{
		return _FactBase;
	}

	*/
	////////////////////////////////////////////////////////////////////////



	NLAIAGENT::tQueue isTemplateMember(	CAgentScript::CMethodCall **StaticMethod,int count,int shift,
												const NLAIAGENT::IVarName *className,
												const NLAIAGENT::IVarName *methodName,
												const NLAIAGENT::IObjectIA &param)
	{
		int index = count;
		int i;
		for(i = 0; i < count; i ++)
		{
			if(StaticMethod[i]->MethodName == *methodName)
			{
				index = StaticMethod[i]->Index;
				switch(StaticMethod[i]->CheckArgType)
				{
				case CAgentScript::CheckAll:
					{
						double d = ((NLAISCRIPT::CParam &)*StaticMethod[i]->ArgType).eval((NLAISCRIPT::CParam &)param);
						if(d >= 0.0)
						{								
							NLAIAGENT::tQueue r;
							StaticMethod[i]->ReturnValue->incRef();
							r.push(NLAIAGENT::CIdMethod(	(shift + StaticMethod[i]->Index),
															0.0,
															NULL,
															StaticMethod[i]->ReturnValue));
							return r;
						}
					}	
					index = count;
					break;
				

				case CAgentScript::CheckCount:
					{
						if(((NLAISCRIPT::CParam &)param).size() == StaticMethod[i]->ArgCount)
						{								
							NLAIAGENT::tQueue r;
							StaticMethod[i]->ReturnValue->incRef();
							r.push(NLAIAGENT::CIdMethod(	(shift + StaticMethod[i]->Index),
															0.0,
															NULL,
															StaticMethod[i]->ReturnValue ));
							return r;
						}
					}
					index = count;
					break;

				case CAgentScript::DoNotCheck:
					{							
						NLAIAGENT::tQueue r;
						StaticMethod[i]->ReturnValue->incRef();
						r.push(NLAIAGENT::CIdMethod(	(shift + StaticMethod[i]->Index),
														0.0,
														NULL,
														StaticMethod[i]->ReturnValue));
						return r;						
					}
					index = count;
					break;
				}
			}
		}		
		return NLAIAGENT::tQueue();
	}
}
