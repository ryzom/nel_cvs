/** \file agent_script.cpp
 *
 * $Id: agent_script.cpp,v 1.139 2003/01/21 11:24:39 chafik Exp $
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
#include "nel/ai/script/interpret_message_getvalue.h"
#include "nel/ai/script/interpret_message_setvalue.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/agent/agent_nombre.h"
#include "nel/ai/agent/performative.h"
#include "nel/ai/agent/msg_notify.h"
#include "nel/ai/agent/msg_goal.h"
#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/agent/msg_setvalue.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/goal.h"
#include "nel/ai/agent/key_agent.h"
#include "nel/ai/agent/list_manager.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/agent/msg_fact.h"
#include "nel/ai/script/libcode.h"

//#define PROFILE
#ifdef PROFILE
#include "nel/misc/time_nl.h"
#endif


namespace NLAIAGENT
{	
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::msgType = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::msgPerf = NULL;
	NLAISCRIPT::CParam *CAgentScript::SendParamMessageScript = NULL;
	NLAISCRIPT::CParam *CAgentScript::SendCompParamMessageScript = NULL;
	NLAISCRIPT::COperandSimple *CAgentScript::IdMsgNotifyParentClass = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::IdMsgNotifyParent = NULL;
	NLAISCRIPT::CParam *CAgentScript::ParamRunParentNotify = NULL;
	CAgentScript::CMethodCall **CAgentScript::StaticMethod = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::ParamIdGetValueMsg = NULL;
	NLAISCRIPT::CParam *CAgentScript::ParamGetValueMsg = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::ParamIdSetValueMsg = NULL;
	NLAISCRIPT::CParam *CAgentScript::ParamSetValueMsg = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::ParamIdTellComponentMsg = NULL;
	NLAISCRIPT::CParam *CAgentScript::ParamTellComponentMsg = NULL;
	NLAISCRIPT::COperandSimpleListOr *CAgentScript::ParamIdInitComponentMsg = NULL;
	NLAISCRIPT::CParam *CAgentScript::ParamInitComponentMsg = NULL;

	const NLAIAGENT::IVarName *CAgentScript::getClassName() const
	{
		const NLAIAGENT::IVarName *classname;
		if ( _AgentClass != NULL )
		{
			classname = _AgentClass->getClassName();
		}
		else
			classname = new CStringVarName("<unknown>"); // Memory leak ?

		return classname;
	}

	void CAgentScript::initAgentScript()
	{

		std::string msgStr;
		std::string scriptName("MsgAgentScript");
		msgStr = std::string("From Message : Define MsgTellComponent\n{");
		msgStr += std::string("Component:\n");		
		msgStr += std::string("\tString<'CompomentName'>;\n");
		msgStr += std::string("\tMessage<'MsgType'>;\n");
		msgStr += std::string("End\n");


		msgStr += std::string("Constructor(String i; Message msg)\n");
		msgStr += std::string("\tCompomentName = i;\n");
		msgStr += std::string("\tMsgType = msg;\n");
		msgStr += std::string("End\n");
		msgStr += std::string("}\n");
		//NLAILINK::buildScript(msgStr,scriptName);

		msgStr += std::string("From Message : Define MsgInitCompoment\n{");
		msgStr += std::string("Component:\n");				
		msgStr += std::string("End\n}\n");

		msgStr += std::string("From Message : Define MsgDebugString\n{");
		msgStr += std::string("Component:\n");
		msgStr += std::string("End\n}\n");

		
		NLAILINK::buildScript(msgStr,scriptName);

		static NLAIC::CIdentType idMsgTellComponentType ("MsgTellComponent");
		static NLAIC::CIdentType idMsgInitComponentType ("MsgInitCompoment");
		static NLAIC::CIdentType idMsgDebugString ("MsgDebugString");

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
		
		NLAISCRIPT::COperandSimple *idMsgTellComponent = new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(idMsgTellComponentType));
		NLAISCRIPT::CParam *ParamTellComponentMsg = new NLAISCRIPT::CParam(1,idMsgTellComponent);

		NLAISCRIPT::COperandSimple *idOpMsgDebugString = new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(idMsgDebugString));
		NLAISCRIPT::CParam *ParamMsgDebugString = new NLAISCRIPT::CParam(1,idOpMsgDebugString);

		CAgentScript::ParamIdGetValueMsg = new NLAISCRIPT::COperandSimpleListOr(2,
																  new NLAIC::CIdentType(NLAIAGENT::CGetValueMsg::IdGetValueMsg),
																  new NLAIC::CIdentType(NLAISCRIPT::CGetValueMsgClass::IdGetValueMsgClass)	);

		CAgentScript::ParamGetValueMsg = new NLAISCRIPT::CParam(1,ParamIdGetValueMsg);

		CAgentScript::ParamIdSetValueMsg = new NLAISCRIPT::COperandSimpleListOr(2,
																  new NLAIC::CIdentType(NLAIAGENT::CSetValueMsg::IdSetValueMsg),
																  new NLAIC::CIdentType(NLAISCRIPT::CSetValueMsgClass::IdSetValueMsgClass)	);

		CAgentScript::ParamSetValueMsg = new NLAISCRIPT::CParam(1,ParamIdSetValueMsg);

///
		CAgentScript::ParamIdInitComponentMsg = new NLAISCRIPT::COperandSimpleListOr(2,
																  new NLAIC::CIdentType(idMsgInitComponentType),
																  new NLAIC::CIdentType(idMsgInitComponentType)	);

		CAgentScript::ParamInitComponentMsg = new NLAISCRIPT::CParam(1,ParamIdInitComponentMsg);
////

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
		
		idMsgTellComponent->incRef();
		StaticMethod[CAgentScript::TRunTellComponent] = new CAgentScript::CMethodCall(	_RUNTEL_, 
																						CAgentScript::TRunTellComponent, ParamTellComponentMsg,
																						CAgentScript::CheckAll,
																						1,
																						new NLAISCRIPT::CObjectUnknown(idMsgTellComponent));

		idOpMsgDebugString->incRef();
		StaticMethod[CAgentScript::TRunAskDebugString] = new CAgentScript::CMethodCall(	_RUNASK_, 
																						CAgentScript::TRunAskDebugString, ParamMsgDebugString,
																						CAgentScript::CheckAll,
																						1,
																						new NLAISCRIPT::CObjectUnknown(idOpMsgDebugString));

		/*idOpMsgDebugString->incRef();
		ParamMsgDebugString->incRef();
		StaticMethod[CAgentScript::TRunAskDebugString] = new CAgentScript::CMethodCall(	_RUNTEL_, 
																						CAgentScript::TRunTellComponent, ParamMsgDebugString,
																						CAgentScript::CheckAll,
																						1,
																						new NLAISCRIPT::CObjectUnknown(idOpMsgDebugString));*/

		ParamTellComponentMsg->incRef();
		idMsgTellComponent->incRef();
		StaticMethod[CAgentScript::TRunAskComponent] = new CAgentScript::CMethodCall(	_RUNASK_, 
																						CAgentScript::TRunAskComponent, ParamTellComponentMsg,
																						CAgentScript::CheckAll,
																						1,
																						new NLAISCRIPT::CObjectUnknown(idMsgTellComponent));

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
																						NULL,CAgentScript::DoNotCheck,
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
																			0,
																			new NLAISCRIPT::CObjectUnknown(
																			new NLAISCRIPT::COperandSimple(
																			new NLAIC::CIdentType(CStringType::IdStringType))));

		StaticMethod[CAgentScript::TGetClassName] = new CAgentScript::CMethodCall(	"GetClassName", 
																			CAgentScript::TGetClassName, 
																			NULL,CAgentScript::CheckCount,
																			0,
																			new NLAISCRIPT::CObjectUnknown(
																			new NLAISCRIPT::COperandSimple(
																			new NLAIC::CIdentType(CStringType::IdStringType))));

		StaticMethod[CAgentScript::TIsInherited] = new CAgentScript::CMethodCall(	"IsInherited", 
																			CAgentScript::TIsInherited, 
																			NULL,CAgentScript::CheckCount,
																			1,
																			new NLAISCRIPT::CObjectUnknown(
																			new NLAISCRIPT::COperandSimple(
																			new NLAIC::CIdentType(NLAILOGIC::CBoolType::IdBoolType))));

		StaticMethod[CAgentScript::TRemoveChild] = new CAgentScript::CMethodCall(	_REMOVECHILD_, 
																				CAgentScript::TRemoveChild, 
																				NULL,CAgentScript::CheckCount,
																				0,
																				new NLAISCRIPT::CObjectUnknown(
																				new NLAISCRIPT::COperandSimple(																
																				new NLAIC::CIdentType(DigitalType::IdDigitalType))));		

		StaticMethod[CAgentScript::TSetStatic] = new CAgentScript::CMethodCall(	_SETSTATIC_, 
																				CAgentScript::TSetStatic, NULL,
																				CAgentScript::CheckCount,
																				2,
																				new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid)) ;


		StaticMethod[CAgentScript::TGetValue] = new CAgentScript::CMethodCall(	_RUNASK_, 
																				CAgentScript::TGetValue, ParamGetValueMsg,
																				CAgentScript::CheckAll,
																				1,
																				new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid)) ;

		StaticMethod[CAgentScript::TSetValue] = new CAgentScript::CMethodCall(	_RUNTEL_, 
																				CAgentScript::TSetValue, ParamSetValueMsg,
																				CAgentScript::CheckAll,
																				1,
																				new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid)) ;

		ParamIdInitComponentMsg->incRef();
		StaticMethod[CAgentScript::TInitComponent] = new CAgentScript::CMethodCall(	_RUNTEL_, 
																				CAgentScript::TInitComponent, ParamInitComponentMsg,
																				CAgentScript::CheckAll,
																				1,
																				new NLAISCRIPT::CObjectUnknown(ParamIdInitComponentMsg)) ;

		NLAISCRIPT::CParam *ParamString =  new NLAISCRIPT::CParam(1,new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(CStringType::IdStringType)));
		StaticMethod[CAgentScript::TIsEUU] = new CAgentScript::CMethodCall(	_ISA_,
																		CAgentScript::TIsEUU,
																		ParamString,CAgentScript::CheckAll,
																		0, new NLAISCRIPT::CObjectUnknown(	new NLAISCRIPT::COperandSimple(
																		   new NLAIC::CIdentType(NLAIAGENT::UInt32Type::IdUInt32Type))));
		ParamString->incRef();
		StaticMethod[CAgentScript::TAddSet] = new CAgentScript::CMethodCall(_ADD_SET_,
																			CAgentScript::TAddSet,
																			ParamString,CAgentScript::CheckAll,
																			0, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));		


		StaticMethod[CAgentScript::TDeflautProccessMsg] = new CAgentScript::CMethodCall("DeflautProccessMsg",
																						CAgentScript::TDeflautProccessMsg, NULL,
																						CAgentScript::CheckCount,
																						1,
																						new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));

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
		/*if ( a._AgentClass )
		{			
			a._AgentClass->incRef();
		}*/

		_NbComponents = a._NbComponents;
		if ( _NbComponents != 0 )
		{
			_Components = new IObjectIA *[ _NbComponents ];

			//sint32 nb_scripted = 0;
			for ( int i = 0; i < _NbComponents; i++ )
			{
				_Components[i] = (IObjectIA *)a._Components[i]->clone();		

				uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
				const NLAIC::CTypeOfObject &t = (const NLAIC::CTypeOfObject &)_Components[i]->getType();
				if( (t.getValue() & b ) == b)
				{					
					((CAgentScript *)_Components[i])->setParent( (const IWordNumRef *) *this);
				}
			}
		}
		else
			_Components = NULL;

		_AgentManager = a._AgentManager;

		mapSet = a.mapSet;
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
		/*if ( _AgentClass )
			_AgentClass->incRef();*/

		_iComponents = 0;

		_AgentManager = manager;
		// Creates the static components array
//		_NbComponents = components.size();
/*
		if ( _NbComponents )
		{
			_Components = new IObjectIA *[ _NbComponents ];
			std::list<IObjectIA *>::iterator it_c = components.begin();
			int id_c = 0;
			//sint32 nb_scripted = 0;
			while ( it_c != components.end() )
			{
				IObjectIA *o = (IObjectIA *)*it_c;
				_Components[id_c] = o;

				uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
				const NLAIC::CTypeOfObject &t = o->getType();

				if((t.getValue() & b) == b)
				{
					((CAgentScript *)o)->setParent( (const IWordNumRef *) *this);
				}

				it_c++;
				id_c++;
			}		
		}
		else
		{
			_Components =NULL;
			_AgentManager = manager;
		}
		*/
		createComponents( components );
	}	

	CAgentScript::~CAgentScript()
	{
		Kill();
		/*if ( _AgentClass )
			_AgentClass->release();*/

		// destruction of static components
		if(_Components != NULL)
		{
			for ( int i = 0; i < _NbComponents; i++ )
			{
#ifdef NL_DEBUG
				const IObjectIA *o = _Components[i];
#endif
				_Components[i]->release();
			}
			delete[] _Components;
		}		
	}

	void CAgentScript::setAgentManager(IAgentManager *manager)
	{
		//if(_AgentManager != NULL) _AgentManager->release();
		_AgentManager = manager;
		for ( int i = 0; i < _NbComponents; i++ )
		{			

			uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
			const NLAIC::CTypeOfObject &t = (const NLAIC::CTypeOfObject &)_Components[i]->getType();
			if( (t.getValue() & b ) == b)			
			{				
				((CAgentScript *)_Components[i])->setAgentManager(_AgentManager);			
			}
		}
	}

	sint32 CAgentScript::getChildMessageIndex(const IMessageBase *msg, sint32 child_index )
	{
		return _AgentClass->getChildMessageIndex( msg, child_index );
	}

	bool CAgentScript::setStaticMember(sint32 index,IObjectIA *op)
	{
#ifdef NL_DEBUG
		if ( index >= _NbComponents )
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		IObjectIA *old_comp = _Components[ index ];
		_Components[ index ] = op;		

		uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
		const NLAIC::CTypeOfObject &t = op->getType();
		if((t.getValue() & b) == b)
		{
			((CAgentScript *)op)->setParent( (const IWordNumRef *) *this);
		}
		if(op != old_comp)  
		{
			old_comp->release();
			return false;
		}
		return true;
	}

	sint32 CAgentScript::getStaticMemberSize() const
	{
		return _NbComponents;//_AgentClass->getStaticMemberSize();
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
		if ( _AgentClass != NULL )
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
		else 
			return NULL;
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

		if( _AgentManager != NULL)
		{
			const NLAISCRIPT::CParam p;
			static CStringVarName debugStringF("GetDebugString");

			TQueue r = isMember(NULL,&debugStringF,p);
			if(r.size())
			{
				NLAISCRIPT::CCodeContext *c = (NLAISCRIPT::CCodeContext *)_AgentManager->getAgentContext()->clone();
				NLAIAGENT::CIdMethod m = r.top();
				//msg->setMethodIndex(0,m.Index);	
				c->Self = this;

				IBaseGroupType *param = new CGroupType();								
				(*c).Stack ++;
				(*c).Stack[(int)(*c).Stack] = param;
				NLAISCRIPT::IMethodContext *methodContex = new NLAISCRIPT::CMethodContext();
				NLAISCRIPT::CCallMethod opCall(methodContex,0,m.Index);
				opCall.runOpCode(*c);												
				const NLAIAGENT::CStringType *returnMsg = (const NLAIAGENT::CStringType *)c->Stack[(int)(*c).Stack];				
				t += returnMsg->getStr().getString();
				(*c).Stack--;
				c->release();
				return;
			}
		}
		
		t += NLAIC::stringGetBuild("class type <%s> ",(const char *)getType());
		if ( _AgentClass )
			t += NLAIC::stringGetBuild("<%s> (scripted)  -StaticComponents:\n",(const char *)_AgentClass->getType());
		else
			t += "<undefined_class> (scripted) -StaticComponents:\n";
		for (int i = 0; i < _NbComponents; i++ )
		{
			std::string buf;
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
		TSetDefNameAgent::iterator iter = _DynamicAgentName.begin();
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

	///Test the set of the agent.
	bool CAgentScript::isa(const std::string &s) const
	{
		std::set<std::string>::iterator it = mapSet.find(s);
		if(it != mapSet.end()) return true;
		else
		if(isClassInheritedFrom(CStringVarName(s.c_str())) >= 0) 
			return true;
		else
			if(s == (const char *) getType()) 
				return true;
		
		return false;
	}

	///Add a set for the agent.
	void CAgentScript::addSet(const std::string &s)
	{
		mapSet.insert(s);
	}

	IObjectIA::CProcessResult CAgentScript::addDynamicAgent(IBaseGroupType *g)
	{
		CIteratorContener i = g->getIterator();
		CStringType &s = (CStringType &)*i++;
		IBasicAgent *a = (IBasicAgent *)i++;
		IBasicAgent *o = a;
		sint n = 1;

#ifdef NL_DEBUG
		std::string name;
		const char *type;
		s.getDebugString(name);

		type = (const char *)a->getType();
		const char *tname = name.c_str();
#endif
		if(!i.isInEnd())
		{
			n = (sint)((NLAIAGENT::INombreDefine *)i++)->getNumber();
		}

		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;

		while(n --)
		{
			o->setParent( (const IWordNumRef *) *this );
			CNotifyParentScript *m = new CNotifyParentScript(this);
			//this->incRef();
			m->setSender(this);
			m->setPerformatif(IMessageBase::PTell);
			((IObjectIA *)o)->sendMessage(m);

			uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
			const NLAIC::CTypeOfObject &t = o->getType();

			if((t.getValue() & b) == b)
			{
				((CAgentScript *)o)->setAgentManager(this);
			}

			_DynamicAgentName.insert(CKeyAgent(s,addChild(o)));
			if(n) o = (IBasicAgent *)a->clone();
		}
		
		r.Result = NULL;

		return r;
	}

	IObjectIA::CProcessResult CAgentScript::addDynamicAgent(const CStringType &name, IBasicAgent *agent)
	{
#ifdef NL_DEBUG
		std::string dbg_name;
		const char *type;
		name.getDebugString( dbg_name );
		type = (const char *)agent->getType();
		const char *tname = dbg_name.c_str();
#endif
		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;

		agent->setParent( (const IWordNumRef *) *this );
		CNotifyParentScript *m = new CNotifyParentScript(this);
		m->setSender(this);
		m->setPerformatif(IMessageBase::PTell);
		((IObjectIA *) agent )->sendMessage(m);
		uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
		const NLAIC::CTypeOfObject &t = agent->getType();
		if((t.getValue() & b) == b)
		{
			((CAgentScript *) agent )->setAgentManager(this);
		}
		_DynamicAgentName.insert(CKeyAgent(name,addChild( agent )));
		
		r.Result = NULL;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::removeDynamic(NLAIAGENT::IBaseGroupType *g)
	{
		CStringType *s = (CStringType *)g->getFront();
		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;
		std::pair<TSetDefNameAgent::iterator,TSetDefNameAgent::iterator>  p = _DynamicAgentName.equal_range(CKeyAgent(*s));

		if(p.first != p.second)
		{	
			while(p.first != p.second)
			{
				TSetDefNameAgent::iterator iTmp = p.first;
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


	void CAgentScript::removeDynamic(NLAIAGENT::IBasicAgent *ag)
	{		
		
		TSetDefNameAgent::iterator p = _DynamicAgentName.begin();
		
		while(p != _DynamicAgentName.end())
		{
			if((*(*p).Itr) == ag)
			{
				_DynamicAgentName.erase(p);				
				removeChild(ag);			
				break;
			}
			p ++;
		}					
	}

	IObjectIA::CProcessResult CAgentScript::getDynamicAgent(IBaseGroupType *g)
	{		
		CStringType *s = (CStringType *)g->get();
		IObjectIA::CProcessResult r;
		r.ResultState = IObjectIA::ProcessIdle;
		std::pair<TSetDefNameAgent::iterator,TSetDefNameAgent::iterator>  p = _DynamicAgentName.equal_range(CKeyAgent(*s));
		
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

	IObjectIA::CProcessResult CAgentScript::runAskDebugString(IBaseGroupType *g)
	{

#ifdef NL_DEBUG
		const char *text = (const char *)getType();
		const char *textP = (const char *)getParent()->getType();
#endif
		static NLAIC::CIdentType idMsgDebugString ("MsgDebugString");

		NLAIAGENT::IMessageBase &mOriginal = (NLAIAGENT::IMessageBase &)*g->get();

		IMessageBase *m = (IMessageBase *)idMsgDebugString.allocClass();
		m->setPerformatif(IMessageBase::PTell);		
		m->setSender(this);

		std::string str;
		getDebugString(str);

		m->push(new CStringType(str));

		if(mOriginal.getSender() != NULL)
								((IObjectIA *)mOriginal.getSender())->sendMessage((IObjectIA *)m);

		IObjectIA::CProcessResult r;

		//m->incRef();
		r.Result = m;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runTellParentNotify(IBaseGroupType *g)
	{	
		sint i;

#ifdef NL_DEBUG
		const char *text = (const char *)getType();
		const char *textP = (const char *)getParent()->getType();
#endif
		CNotifyParentScript *m = new CNotifyParentScript(this);
		m->setPerformatif(IMessageBase::PTell);		
		m->setSender(this);

		for(i = 0; i < _NbComponents; i++)
		{
			if(_Components[i] != NULL)
			{				
				CNotifyParentScript *msg = (CNotifyParentScript *)m->clone();
				try
				{
					uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
					const NLAIC::CTypeOfObject &t = _Components[i]->getType();

					if((t.getValue() & b) == b)
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

	IObjectIA::CProcessResult CAgentScript::runTellComponent(IBaseGroupType *g)
	{	
		NLAIAGENT::IMessageBase &mOriginal = (NLAIAGENT::IMessageBase &)*g->get();
		CStringType *c = (CStringType *)mOriginal[(sint32)0];
		NLAIAGENT::IMessageBase *m = (NLAIAGENT::IMessageBase *)mOriginal[(sint32)1];
		m->incRef();
		m->setSender((IObjectIA *)mOriginal.getSender());
		m->setPerformatif(mOriginal.getPerformatif());
		sendMessage(c->getStr(), (IObjectIA *)m);

		IObjectIA::CProcessResult r;

		m->incRef();
		r.Result = m;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runAskGetValue(IBaseGroupType *g)
	{
		NLAIAGENT::IMessageBase &msg_result = (NLAIAGENT::IMessageBase &)*g->get();
		msg_result.incRef();

		CStringType *comp_name = (CStringType *)msg_result[(sint32)0];
		
		msg_result.setPerformatif(IMessageBase::PTell);
		msg_result.setMethodIndex(-1,-1);
		msg_result.setContinuation( (IObjectIA *) msg_result.getSender() );
		
		sint32 index = _AgentClass->getInheritedStaticMemberIndex(  comp_name->getStr()  );
		if ( index != -1 )
		{
			msg_result.set( 1, _Components[ index ] );
		}
		else
		{
			// Component not foud: return error msg
		}

		IObjectIA::CProcessResult r;
		r.Result = &msg_result;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runTellSetValue(IBaseGroupType *g)
	{
		NLAIAGENT::IMessageBase &msg_result = (NLAIAGENT::IMessageBase &)*g->get();
		msg_result.incRef();

		while ( msg_result.size() )
		{
			CStringType *comp_name = (CStringType *) msg_result[ (sint32) 0 ];
			IObjectIA *comp_val = (IObjectIA *) msg_result[ (sint32) 1 ];
			
			sint32 index = getStaticMemberIndex( comp_name->getStr() );
			if ( index != -1 )
			{
				// Sets the component to the new value
				setStaticMember( index, comp_val );		
			}
			else
			{
				// Component not found: creates it
				
			}
		}
		IObjectIA::CProcessResult r;
		msg_result.incRef();
		r.Result = &msg_result;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runInitComponent(IBaseGroupType *g)
	{
		NLAIAGENT::IMessageBase &msg_result = (NLAIAGENT::IMessageBase &)*g->get();
		msg_result.incRef();


		// Cleans previous components
		if ( _Components != NULL )
		{
			for ( int i = 0; i < _NbComponents; i++ )
				_Components[i]->release();
		}

		// Creates a new component array
		_NbComponents = (sint32) msg_result.size() / 3;
		_Components = new IObjectIA *[ _NbComponents ];

		int test = 0;

		for ( int i = 0; i < msg_result.size() ; i += 3 )
		{
			CStringType *comp_name = (CStringType *) msg_result[ (sint32) i ];
			IObjectIA *comp_val = (IObjectIA *) msg_result[ (sint32) (i + 2) ];
			
			sint32 index = getStaticMemberIndex( comp_name->getStr() ); //_AgentClass->getInheritedStaticMemberIndex(  comp_name->getStr()  );
			if ( index != -1 )
			{
				// Sets the component to the new value
				setStaticMember( index, comp_val );		
			}
			else
			{
				setStaticMember( (sint32) (i / 3) , comp_val );
			}
			test++;
		}

		IObjectIA::CProcessResult r;
		msg_result.incRef();
		r.Result = &msg_result;
		return r;
	}

	IObjectIA::CProcessResult CAgentScript::runInitClass(IBaseGroupType *g)
	{
		const char * class_name = ((NLAIAGENT::CStringVarName *)g->get())->getString();
		NLAIC::CIdentType id_class( class_name );	

		

		/*
		NLAIAGENT::IMessageBase &msg_result = (NLAIAGENT::IMessageBase &)*g->get();
		msg_result.incRef();


		// Cleans previous components
		if ( _Components != NULL )
		{
			for ( int i = 0; i < _NbComponents; i++ )
				_Components[i]->release();
		}

		// Creates a new component array
		_NbComponents = (sint32) msg_result.size() / 3;
		_Components = new IObjectIA *[ _NbComponents ];

		int test = 0;

		for ( int i = 0; i < msg_result.size() ; i += 3 )
		{
			CStringType *comp_name = (CStringType *) msg_result[ (sint32) i ];
			CStringType *comp_type = (CStringType *) msg_result[ (sint32) (i + 1) ];
			IObjectIA *comp_val = (IObjectIA *) msg_result[ (sint32) (i + 2) ];
			
			sint32 index = getStaticMemberIndex( comp_name->getStr() ); //_AgentClass->getInheritedStaticMemberIndex(  comp_name->getStr()  );
			if ( index != -1 )
			{
				// Sets the component to the new value
				setStaticMember( index, comp_val );		
			}
			else
			{
				setStaticMember( (sint32) (i / 3) , comp_val );
			}
			test++;
		}
		*/
		IObjectIA::CProcessResult r;
		r.Result = NULL;
		return r;
	}




	IObjectIA::CProcessResult CAgentScript::getDynamicName(NLAIAGENT::IBaseGroupType *g)
	{

		IObjectIA::CProcessResult r;
		const IObjectIA *o = ((CLocalAgentMail *)g->get())->getHost();
		TSetDefNameAgent::iterator i = _DynamicAgentName.begin();

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

	IObjectIA::CProcessResult CAgentScript::sendBroadCast(IObjectIA *m)
	{
		TSetDefNameAgent::iterator i = _DynamicAgentName.begin();

		if(i != _DynamicAgentName.end()) 
		{
			((IObjectIA *)(*(*(i)).Itr))->sendMessage(m);
			i ++;

			while(i != _DynamicAgentName.end())
			{
				m->incRef();
				IObjectIA *o = ((IObjectIA *)(*(*(i)).Itr));
				o->sendMessage(m);
				i++;
			}
		}

		return IObjectIA::CProcessResult();
	}
	
	IObjectIA::CProcessResult CAgentScript::sendMessageToDynmaicChild(const IVarName &compName,IObjectIA *msg)
	{
		//TSetDefNameAgent::iterator  p = _DynamicAgentName.find(CKeyAgent(CStringType(compName)));
		std::pair<TSetDefNameAgent::iterator,TSetDefNameAgent::iterator>  p = _DynamicAgentName.equal_range(CKeyAgent(CStringType(compName)));

#ifdef NL_DEBUG
		const char *txt = (const char *)msg->getType();
		std::string txtName;
		compName.getDebugString(txtName);
#endif

		IObjectIA::CProcessResult r;

		if(p.first != p.second)
		{
			while(p.first != p.second)
			{
				CAgentScript *o = (CAgentScript *)*((*(p.first)).Itr);
				o->sendMessage(msg);
				p.first ++;
				if(p.first != p.second) msg = (IObjectIA *)msg->clone();
			}
		}
		else
		{
			r.ResultState = processError;
			r.Result = NULL;			
		}

		return r;
	}

	IObjectIA::CProcessResult CAgentScript::sendMessage(const IVarName &compName,IObjectIA *msg)
	{
#ifdef NL_DEBUG
	const char *txt = (const char *)msg->getType();
	const char *compNameDb = (const char *)compName.getString();
	//nlinfo("MSG %s %4x", txt, this);
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
					if(sendMessageToDynmaicChild(compName,msg).ResultState == processError) msg->release();
				}
			}
			else
			{
				if(sendMessageToDynmaicChild(compName,msg).ResultState == processError) msg->release();
			}
		}
		else			
		{						
			if(sendMessageToDynmaicChild(compName,msg).ResultState == processError) msg->release();
		}
		return IObjectIA::CProcessResult();
	}

	IObjectIA::CProcessResult CAgentScript::sendMessage(IObjectIA *m)
	{
#ifdef NL_DEBUG
	const char *txt = (const char *)m->getType();
	const char *classBase = (const char *)getType();
	//nlinfo("MSG %s %4x", txt, m);
#endif
		IMessageBase *msg = (IMessageBase *)m;
		//this->incRef();
		msg->setReceiver(this);
		if(msg->getMethodIndex() < 0)
		{			
			uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tMessage;
			const NLAIC::CTypeOfObject &t = m->getType();

			if((t.getValue() & b) == b)			
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

				default:
					throw;
					break;
				}

				CStringVarName &tmp_name = *runMsg;

				NLAISCRIPT::COperandSimple *t = new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(m->getType()));
				NLAISCRIPT::CParam p(1,t);
				
				TQueue r = isMember(NULL,&tmp_name,p);
				if(r.size())
				{
					NLAIAGENT::CIdMethod m = r.top();
					msg->setMethodIndex(0,m.Index);
				}				
			}			
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
		int i = _NbComponents;

		IObjectIA **com = _Components;

		while(i --)
			(*com ++)->run();		

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
	bool dbugB = false;	
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
			default: break;
		}
	}

	void CAgentScript::processMessages()
	{
		IObjectIA *c = NULL;
		if( _AgentManager != NULL) 
			c = (IObjectIA *)_AgentManager->getAgentContext();
		else 
			c = NULL;

		IMailBox *mail = getMail();

		
		//const IMailBox::TListMessage &l = mail->getMesseageListe();
		sint n = mail->getMessageCount();

		while(/*l.begin() != l.end()*/n--)
		{
			IMessageBase &msg = (IMessageBase &)mail->getMessage();
#ifdef NL_DEBUG
		const char *dbg_this = (const char *) getType();
		const char *dbg_msg = (const char *) msg.getType();
#endif
			
			if(msg.getMethodIndex() >= 0 && c != NULL)
			{
				sint indexM = msg.getMethodIndex() - IAgent::getMethodIndexSize();
				IObjectIA *o;
				if(indexM != CAgentScript::TDeflautProccessMsg) processMessages(&msg,c);
				else 
				{
					o = IBasicAgent::run( msg );
					if(o != NULL) o->release();
				}
				if ( mail->getMessageCount() != 0 )
					mail->popMessage();
			}
			else 
			{
				try
				{
					IObjectIA *o = IBasicAgent::run( msg );
					if (o != NULL) 
						o->release();
					mail->popMessage();
				}
				catch(NLAIE::CExceptionNotImplemented &e)
				{
					mail->popMessage();
					throw NLAIE::CExceptionNotImplemented(e.what());
				}
			}			
		}
	}

	IObjectIA::CProcessResult CAgentScript::runActivity()
	{
		
		NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *)_AgentManager->getAgentContext();
		context->Self = this;
		if(_AgentClass->getRunMethod() >= 0) runMethodeMember(_AgentClass->getRunMethod(), context);
		
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
				IObjectIA::CProcessResult r;
				const CAgentScript *p = (CAgentScript *)getParent();
				TSetDefNameAgent::iterator i = p->_DynamicAgentName.begin();

				while(i != p->_DynamicAgentName.end())
				{
					CKeyAgent key = *i;
					if( this == *key.Itr )
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

		case TGetClassName:
			{
				IObjectIA::CProcessResult r;
				if ( _AgentClass != NULL )
				{
					const NLAIAGENT::IVarName *classname = getClassName();
					r.Result = new CStringType( *classname );
				}
				else
					r.Result = new CStringType( CStringVarName("<unknown>"));				
				return r;
			}

		case TIsInherited:
			{
				IObjectIA::CProcessResult r;
				if ( _AgentClass != NULL )
				{
					CGroupType *param = (CGroupType *) o;
					CStringType *comp_name = (CStringType *)((IBaseGroupType *)param)->popFront();
					if ( isClassInheritedFrom( comp_name->getStr() ) != -1 )
						r.Result = new NLAILOGIC::CBoolType( true );
					else
						r.Result = new NLAILOGIC::CBoolType( false );
				}
				else
					r.Result = new NLAILOGIC::CBoolType( false );
				
				return r;
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

		case TRunAskDebugString:
			{
				return runAskDebugString((IBaseGroupType *)o);
			}

		case TRunAskComponent:
		case TRunTellComponent:
			{				
				return runTellComponent((IBaseGroupType *)o);
			}


		case TSetStatic:
			{
				CGroupType *param = (CGroupType *) o;
				CStringType *comp_name = (CStringType *)((IBaseGroupType *)param)->popFront();
				IObjectIA *value = (IObjectIA *)((IBaseGroupType *)param)->popFront();
				int index = _AgentClass->getComponentIndex(  comp_name->getStr()  );
				_AgentClass->updateStaticMember(index, value);
				return IObjectIA::CProcessResult();
			}

		case TGetValue:
			{
				return runAskGetValue( (IBaseGroupType *) o );
			}

		case TSetValue:
			{
				return runTellSetValue( (IBaseGroupType *) o );
			}

		case TInitComponent:
			{
				return runInitComponent( (IBaseGroupType *) o );
			}


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
				IObjectIA::CProcessResult r;
				const CAgentScript *p = (CAgentScript *)getParent();
				TSetDefNameAgent::iterator i = p->_DynamicAgentName.begin();

				while(i != p->_DynamicAgentName.end())
				{
					CKeyAgent key = *i;
					if( this == *key.Itr )
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

		case TGetClassName:
			{
				IObjectIA::CProcessResult r;
				const NLAIAGENT::IVarName *classname = getClassName();
				r.Result = new CStringType( *classname );

				r.Result->incRef();
				return r;
			}

		case TIsInherited:
			{
				IObjectIA::CProcessResult r;
				if ( _AgentClass != NULL )
				{
					CGroupType *param = (CGroupType *) o;
					CStringType *comp_name = (CStringType *)((IBaseGroupType *)param)->popFront();
					if ( isClassInheritedFrom( comp_name->getStr() ) != -1 )
						r.Result = new NLAILOGIC::CBoolType( true );
					else
						r.Result = new NLAILOGIC::CBoolType( false );
					
				}
				else
					r.Result = new NLAILOGIC::CBoolType( false );

				r.Result->incRef();
				return r;
			}

		case TRunAskParentNotify:
			{				
				return runAskParentNotify((IBaseGroupType *)o);
			}

		case TRunTellParentNotify:
			{				
				return runTellParentNotify((IBaseGroupType *)o);
			}

		case TRunAskDebugString:
			{
				return runAskDebugString((IBaseGroupType *)o);
			}

		case TRunAskComponent:
		case TRunTellComponent:
			{				
				return runTellComponent((IBaseGroupType *)o);
			}
		case TSetStatic:
			{
				CGroupType *param = (CGroupType *) o;
				CStringType *comp_name = (CStringType *)((IBaseGroupType *)param)->popFront();
				IObjectIA *value = (IObjectIA *)((IBaseGroupType *)param)->popFront();
				int index = _AgentClass->getInheritedStaticMemberIndex( comp_name->getStr() );
				_AgentClass->updateStaticMember(index, value);
				IObjectIA::CProcessResult a;
				a.Result = NULL;
				return a;
			}

		case TGetValue:
			{
				return runAskGetValue( (IBaseGroupType *) o );
			}

		case TSetValue:
			{
				return runTellSetValue( (IBaseGroupType *) o );
			}

		case TInitComponent:
			{
				return runInitComponent( (IBaseGroupType *) o );
			}

		case TIsEUU:
			{
				CGroupType *param = (CGroupType *) o;
				std::string s(((NLAIAGENT::CStringType *)param->get())->getStr().getString());
				NLAIAGENT::IObjectIA::CProcessResult r;
				
				if(isa(s))				
					r.Result = new NLAIAGENT::DigitalType(1.0);				
				else
					r.Result = new NLAIAGENT::DigitalType(0.0);
				return r;
			}			

		case TAddSet:
			{
				CGroupType *param = (CGroupType *) o;
				std::string s(((NLAIAGENT::CStringType *)param->get())->getStr().getString());				
				addSet(s);

				return NLAIAGENT::IObjectIA::CProcessResult();
			}			

		default:
			return IAgent::runMethodeMember(index,o);
		}
	}

	std::string CAgentScript::getMethodeMemberDebugString(sint32 h, sint32 id) const
	{
		int i = id - getBaseMethodCount();
		if(i >= 0)
		{
			std::string name;
			if(h)
			{
				_AgentClass->getBrancheCode(h,i).getName().getDebugString(name);
				_AgentClass->getBrancheCode(h,i).getParam().getDebugString(name);
			}
			else
			{
				_AgentClass->getBrancheCode(i).getName().getDebugString(name);
				_AgentClass->getBrancheCode(i).getParam().getDebugString(name);
			}
			return name;
		}

		switch(id - IAgent::getMethodIndexSize())
		{
		case TSend:
			{
				return std::string("CAgentScript::sendMessage(IMessage)");
			}

		case TSendComponent:
			{
				return std::string("CAgentScript::sendMessage(String,IMessage)");				
			}

		case TGetChildTag:
			{
				return std::string("CAgentScript::getDynamicAgent(String)");
			}

		case TAddChildTag:
			{
				return std::string("CAgentScript::addDynamicAgent(String,IAgent)");				
			}
		case TFather:
			{
				return std::string("CAgentScript::Father()");				
			}

		case TSelf:
			{
				return std::string("CAgentScript::Self()");
			}
		case TGetName:
			{				
				return std::string("CAgentScript::GetAgentName()");
			}

		case TRunAskParentNotify:
			{	
				return std::string("CAgentScript::runAskParentNotify(CNotifyParent)");
			}

		case TRunTellParentNotify:
			{				
				return std::string("CAgentScript::runTellarentNotify(CNotifyParent)");
			}

		case TRunAskComponent:
		case TRunTellComponent:
			{				
				return std::string("CAgentScript::runTell/AskCompoment(MsgTellCompoment)");
			}
		case TSetStatic:
			{								
				return std::string("CAgentScript::updateStaticMember(String, IObjectIA *)");				
			}

		case TGetValue:
			{				
				return std::string("CAgentScript::runAskGetValue(MsgGetValue)");
			}

		case TSetValue:
			{				
				return std::string("CAgentScript::runTellSetValue(MsgGetValue)");
			}

		case TInitComponent:
			{				
				return std::string("CAgentScript::runInitComponent(MsgGetValue)");
			}


		default:
			return IAgentManager::getMethodeMemberDebugString(h,id);		
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
#ifdef NL_DEBUG
		std::string nameDbg;
		_AgentClass->getBrancheCode(inheritance,i).getName().getDebugString(nameDbg);
		_AgentClass->getBrancheCode(inheritance,i).getParam().getDebugString(nameDbg);
#endif
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
			NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
			int ip;
			if(context.Code != NULL) ip = (uint32)*context.Code;
			else ip =0;
			context.Code = (NLAISCRIPT::CCodeBrancheRun *)opPtr;
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

	TQueue CAgentScript::getPrivateMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{		

#ifdef NL_DEBUG
		const char *dgb_meth_name = methodName->getString();
#endif
		return isTemplateMember(CAgentScript::StaticMethod,CAgentScript::TLastM,IAgent::getMethodIndexSize(),className,methodName,param);		
	}

	TQueue CAgentScript::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{
		

 		if(className == NULL)
		{

			TQueue r;
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
					r = IAgent::isMember(className,methodName,param);
					if(r.size())
						return r;
					else
						return isDeflautProccessMsg(className,methodName,param);
				}
			}
		
		}		
		else 
		if(*className == CStringVarName("Agent"))
		{					
			TQueue r;
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
					r = IAgent::isMember(className,methodName,param);
					if(r.size())
						return r;
					else
						return isDeflautProccessMsg(className,methodName,param);
				}
			}
			
		}
		return TQueue();
	}

	TQueue CAgentScript::isDeflautProccessMsg(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{
		const char *name = methodName->getString();
		if(name[0] == 'R' && name[1] == 'u' && name[2] == 'n')
		{
			static CStringVarName runMsgName[7] = {"RunTell", "RunAchieve", "RunAsk", "RunExec", "RunBreak", "RunKill", "RunError"};
			sint i;
			for(i = 0; i < 7; i ++)
			{
				if(*methodName == runMsgName[i])
				{
					TQueue r;			
					CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
					r.push(CIdMethod(TDeflautProccessMsg + IAgent::getMethodIndexSize(),0.0,NULL,c));
					return r;
				}
			}
		}

		return TQueue();
	}

	sint32 CAgentScript::isClassInheritedFrom(const IVarName &name) const
	{
		return _AgentClass->isClassInheritedFrom( name );
	}	

	sint32 CAgentScript::getStaticMemberIndex(const IVarName &name) const
	{
		return _AgentClass->getInheritedStaticMemberIndex(name);
	}

	NLAIAGENT::TQueue isTemplateMember(	CAgentScript::CMethodCall **StaticMethod,int count,int shift,
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
							NLAIAGENT::TQueue r;
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
							NLAIAGENT::TQueue r;
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
						NLAIAGENT::TQueue r;
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
		return NLAIAGENT::TQueue();
	}

	void CAgentScript::createComponents(std::list<IObjectIA *> &components)
	{
		_NbComponents = components.size();
		if ( _NbComponents )
		{
			_Components = new IObjectIA *[ _NbComponents ];
			std::list<IObjectIA *>::iterator it_c = components.begin();
			int id_c = 0;
			while ( it_c != components.end() )
			{
				IObjectIA *o = (IObjectIA *)*it_c;
				_Components[id_c] = o;

				uint b = NLAIC::CTypeOfObject::tInterpret | NLAIC::CTypeOfObject::tAgent;
				const NLAIC::CTypeOfObject &t = o->getType();

				if((t.getValue() & b) == b)
				{
					((CAgentScript *)o)->setParent( (const IWordNumRef *) *this);
				}

				it_c++;
				id_c++;
			}		
		}
		else
			_Components = NULL;
	}

	void CAgentScript::callConstructor()
	{
		NLAIAGENT::CStringVarName constructor_func_name("Constructor");
		sint32 id_func = getClass()->findMethod( constructor_func_name, NLAISCRIPT::CParam() );

#ifdef NL_DEBUG

		const char * ttt = (const char *) getType();
#endif
		if ( id_func != -1 )
		{	
			NLAISCRIPT::CStackPointer stack;
			NLAISCRIPT::CStackPointer heap;
			NLAISCRIPT::CCodeContext codeContext(stack, heap, NULL, this, NLAISCRIPT::CCallPrint::inputOutput);
			codeContext.Self = this;
			NLAISCRIPT::CCodeBrancheRun *o = (NLAISCRIPT::CCodeBrancheRun *) getClass()->getBrancheCode( id_func ).getCode();			
			codeContext.Code = o;
			int ip;
			if(codeContext.Code != NULL) ip = (uint32)*codeContext.Code;
			else ip =0;

			*codeContext.Code = 0;

			(void)o->run(codeContext);

			*codeContext.Code = ip;

		}
	}

	void CAgentScript::callFunction(std::string &f_name, NLAIAGENT::IObjectIA *p)
	{
		NLAIAGENT::CStringVarName func_name( f_name.c_str() );
		sint32 id_func = getClass()->findMethod( func_name, NLAISCRIPT::CParam() );

#ifdef NL_DEBUG
		const char * ttt = (const char *) getType();
#endif
		if ( id_func != -1 )
		{	
			NLAISCRIPT::CStackPointer stack;
			NLAISCRIPT::CStackPointer heap;
			NLAISCRIPT::CCodeContext codeContext(stack, heap, NULL, this, NLAISCRIPT::CCallPrint::inputOutput);
			codeContext.Self = this;
			NLAISCRIPT::CCodeBrancheRun *o = (NLAISCRIPT::CCodeBrancheRun *) getClass()->getBrancheCode( id_func ).getCode();			
			codeContext.Code = o;
			int ip;
			if(codeContext.Code != NULL) ip = (uint32)*codeContext.Code;
			else ip =0;

			*codeContext.Code = 0;

			(void)o->run(codeContext);

			*codeContext.Code = ip;

		}
	}
}
