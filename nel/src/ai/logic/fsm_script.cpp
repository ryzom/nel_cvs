#include "nel/ai/logic/fsm_script.h"
#include "nel/ai/agent/msg_action.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
//#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/interpret_message_action.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	/////////////////////////////////////////////////////////////
	// Succes and failure messages declaration
/*
	NLAISCRIPT::COperandSimpleListOr *CFsmScript::ParamIdSuccessMsg = NULL;
	NLAISCRIPT::CParam *CFsmScript::ParamSuccessMsg = NULL;
	NLAISCRIPT::COperandSimpleListOr *CFsmScript::ParamIdFailureMsg = NULL;
	NLAISCRIPT::CParam *CFsmScript::ParamFailureMsg = NULL;

	void CFsmScript::initClass()
	{
		CFsmScript::ParamIdSuccessMsg = new NLAISCRIPT::COperandSimpleListOr(2,
															  new NLAIC::CIdentType(NLAIAGENT::CSuccessMsg::IdSuccessMsg),
															  new NLAIC::CIdentType(NLAISCRIPT::CSuccessMsgClass::IdSuccessMsgClass)	);		
		CFsmScript::ParamSuccessMsg = new NLAISCRIPT::CParam(1,ParamIdSuccessMsg);		
		CFsmScript::ParamIdFailureMsg = new NLAISCRIPT::COperandSimpleListOr(2,
																  new NLAIC::CIdentType(NLAIAGENT::CFailureMsg::IdFailureMsg),
																  new NLAIC::CIdentType(NLAISCRIPT::CFailureMsgClass::IdFailureMsgClass)	);

		CFsmScript::ParamFailureMsg = new NLAISCRIPT::CParam(1,ParamIdFailureMsg);
	}
	void CFsmScript::releaseClass()
	{		
		CFsmScript::ParamSuccessMsg->release();		
		CFsmScript::ParamFailureMsg->release();
	}
*/
	/////////////////////////////////////////////////////////////


	CFsmScript::CFsmScript(const CFsmScript &a) : CActorScript(a)
	{
/*		std::vector<IBasicAgent *>::const_iterator it_c = a._ActiveChilds.begin();
		while ( it_c != a._ActiveChilds.end() )
		{
			_ActiveChilds.push_back( (IBasicAgent *) (*it_c)->clone() );
			it_c++;
		}
		*/
	}

	CFsmScript::CFsmScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CFsmClass *actor_class )
	: CActorScript(manager, father, components, actor_class )
	{	
	}	

	CFsmScript::CFsmScript(IAgentManager *manager, bool stay_alive) : CActorScript( manager, stay_alive )
	{
	}

	CFsmScript::~CFsmScript()
	{
/*		ParamIdSuccessMsg->release();
		ParamSuccessMsg->release();
		ParamIdFailureMsg->release();
		ParamFailureMsg->release();
		*/
	}

	const NLAIC::IBasicType *CFsmScript::clone() const
	{		
		CFsmScript *m = new CFsmScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *CFsmScript::newInstance() const
	{	
		CFsmScript *instance;
		if ( _AgentClass )
		{
			instance = (CFsmScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new CFsmScript(NULL);
		}
		return instance;
	}
	
	void CFsmScript::getDebugString(std::string &t) const
	{
		if ( _AgentClass )
		{
			std::string buf;
			_AgentClass->getDebugString(buf);
			t += buf;
		}
		else
			t += "<CFsmScript>";
	}

	bool CFsmScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIC::CIdentType &CFsmScript::getType() const
	{		
		return IdFsmScript;
	}

	void CFsmScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		// TODO
	}

	void CFsmScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		// TODO
	}

	sint32 CFsmScript::getMethodIndexSize() const
	{
		return CAgentScript::getBaseMethodCount();
	}

/*	IObjectIA::CProcessResult CFsmScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;
*/
/*		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
*/
/*		return CAgentScript::runMethodeBase(heritance,index,params);
	}

*/

	IObjectIA::CProcessResult CFsmScript::runMethodBase(int index,IObjectIA *params)
	{	


		int i = index - IAgent::getMethodIndexSize();

		if ( i < getBaseMethodCount() )
			return CAgentScript::runMethodBase(index, params);

		switch ( i )
		{
			case 1:
				onSuccess(params);
				break;
			case 2:
				onFailure(params);
				break;
		}

		IObjectIA::CProcessResult r;

#ifdef NL_DEBUG
		std::string buf;
		getDebugString(buf);
#endif

		return r;
	}

	int CFsmScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}


	TQueue CFsmScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		
#ifdef NL_DEBUG
		const char *dbg_func_name = name->getString();
#endif

		TQueue result = CAgentScript::isMember( className, name, param);

		static NLAIAGENT::CStringVarName run_tell("RunTell");
		static NLAIAGENT::CStringVarName get_active("getActive");

		if ( result.size() )
			return result;

		// Processes succes and failure functions
		if ( *name ==  run_tell)
		{
			double d;
			d = ((NLAISCRIPT::CParam &)*ParamSuccessMsg).eval((NLAISCRIPT::CParam &)param);
			if ( d >= 0.0 )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + 1, 0.0,NULL, r_type ) );
			}

			d = ((NLAISCRIPT::CParam &)*ParamFailureMsg).eval((NLAISCRIPT::CParam &)param);
			if ( d >= 0.0 )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + 2, 0.0,NULL, r_type ) );
			}
		}

		// Returns a list of active states
		if ( *name == get_active )
		{
			// Todo
		}

		return result;
	}

	const IObjectIA::CProcessResult &CFsmScript::run()
	{
		return CAgentScript::run();
	}

	void CFsmScript::activate(IBasicAgent *child)
	{
//		_ActiveChilds.push_back(child);
	}

	void CFsmScript::unactivate(IBasicAgent *child)
	{
/*		std::vector<IBasicAgent *>::iterator it_c = _ActiveChilds.begin();
		while ( it_c != _ActiveChilds.end() )
		{
			if ( child == *it_c )
			{
				_ActiveChilds.erase( it_c );
				return;
			}
			it_c++;
		}
		*/
	}

	void CFsmScript::runChildren()
	{
#ifdef NL_DEBUG	
	const char *classBase = (const char *)getType();
#endif
/*		std::vector<IBasicAgent *>::iterator it_c = _ActiveChilds.begin();
		while ( it_c != _ActiveChilds.end() )
		{
			IBasicAgent *child = *it_c;
			(*it_c)->run();
			it_c++;
		}
		*/

		// Activation des fils
		CAgentScript::runChildren();
	}

/*
	void CFsmScript::onSuccess( IObjectIA *)
	{
		// Envoi d'un message succès au père
	}

	void CFsmScript::onFailure( IObjectIA *)
	{
		// Envoi d'un message echec au père
	}
	*/

	void CFsmScript::setTopLevel(NLAIAGENT::CAgentScript *tl)
	{
		_TopLevel = tl;

		for (int i = 0; i < _NbComponents; i++ )
		{
			if ( _Components[i]->isClassInheritedFrom( NLAIAGENT::CStringVarName("Actor") ) != -1 )
			{
				if ( _TopLevel )
					( (CActorScript *)_Components[i] )->setTopLevel( _TopLevel );
				else
					( (CActorScript *)_Components[i] )->setTopLevel( this );
			}
		}
	}
}
