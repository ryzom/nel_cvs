#include "nel/ai/agent/actor_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	CActorScript::CActorScript(const CActorScript &a) : CAgentScript(a)
	{
		_IsActivated = a._IsActivated;
		_OnActivateIndex = -1;
	}

	CActorScript::CActorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CActorClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
		_IsActivated = false;
	}	

	CActorScript::CActorScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
		_IsActivated = false;
	}

	CActorScript::~CActorScript()
	{
	}

	/// Returns true if the actor has a token
	bool CActorScript::isActivated()
	{
		return _IsActivated;
	}

	/// Activates the actor
	void CActorScript::activate()
	{
		if ( !_IsActivated )
		{
			onActivate();
			_IsActivated = true;
		}
	}

	/// Unactivates the actor
	void CActorScript::unActivate()
	{
		if ( _IsActivated )
		{
			onUnActivate();
			_IsActivated = false;
		}
	}

	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::switchActor(CActorScript *receiver, bool stay_active)
	{
		receiver->activate();

		if ( !stay_active )
			unActivate();
	}
		
	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::switchActor(std::vector<CActorScript *> &actors, bool stay_active)
	{
		std::vector<CActorScript *>::iterator it_act = actors.begin();
		while ( it_act != actors.end() )
		{
			( *it_act )->activate();
			it_act++;
		}
		// TODO: Envoi de message "activate" 
		if ( !stay_active )
			unActivate();
	}

	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::switchActor(std::vector<CComponentHandle *> &handles, bool stay_active)
	{
		std::vector<CComponentHandle *>::iterator it_handle = handles.begin();
		while ( it_handle != handles.end() )
		{
			( (CActorScript *)( *it_handle )->getValue() )->activate();
			it_handle++;
		}
		// TODO: Envoi de message "activate" 
		if ( !stay_active )
			unActivate();
	}


	/// Callback called at the activation of the actor
	void CActorScript::onActivate()
	{
		// Default behaviour: do nothing
	}

	/// Callback called when the agent is unactivated
	void CActorScript::onUnActivate()
	{
		// default behaviour: do nothing
	}

	const NLAIC::IBasicType *CActorScript::clone() const
	{		
		CActorScript *m = new CActorScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *CActorScript::newInstance() const
	{	
		CActorScript *instance;
		if ( _AgentClass )
		{
			instance = (CActorScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new CActorScript(NULL);
		}
		return instance;
	}
	
	void CActorScript::getDebugString(char *t) const
	{
		strcpy(t,"CActorScript ");
		if ( _IsActivated )
			strcat(t, "<active>");
		else
			strcat(t, "<idle>");
	}

	bool CActorScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	IObjectIA::CProcessResult CActorScript::sendMessage(IObjectIA *m)
	{
		return CAgentScript::sendMessage(m);
	}

	void CActorScript::processMessages()
	{
		if ( _IsActivated )
			CAgentScript::processMessages();
	}

	const IObjectIA::CProcessResult &CActorScript::run()
	{
		if ( _IsActivated )
		{
			return CAgentScript::run();
		}
		else
			return IObjectIA::ProcessRun;
	}

/*	IObjectIA *CActorScript::run(const IMessageBase &msg)
	{
		return CAgentScript::run( msg );
	}
*/
	const NLAIC::CIdentType &CActorScript::getType() const
	{		
		return IdActorScript;
	}

	void CActorScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		os.serial( (bool) _IsActivated );
	}

	void CActorScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		is.serial( _IsActivated );
	}

	sint32 CActorScript::getMethodIndexSize() const
	{
//		if ( _AgentClass )
//			return CAgentScript::getMethodIndexSize() 
//		else
		return CAgentScript::getBaseMethodCount() + fid_switch;
	}

//	virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);


	IObjectIA::CProcessResult CActorScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_onActivate )
		{
			onActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_unActivate )
		{
			unActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_onUnActivate )
		{
			onUnActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_switch )
		{
			std::vector<CStringType *> handles;
			if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
			{
				IBaseGroupType *fw = (IBaseGroupType *) ( ((NLAIAGENT::IBaseGroupType *)params) )->getFront();
				( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
				while ( fw->size() )
				{
					handles.push_back( (CStringType *) fw->getFront() );
					fw->popFront();
				}
				std::vector<CComponentHandle *> switched;
				for ( int i = 0; i < (int) handles.size(); i++)
					switched.push_back( new CComponentHandle(  handles[ i ]->getStr() , (IAgent *) getParent() ) );

				switchActor( switched, false );
			}
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
		return CAgentScript::runMethodeMember(heritance,index,params);
	}



	IObjectIA::CProcessResult CActorScript::runMethodBase(int index,IObjectIA *params)
	{	

		index = index - IAgent::getMethodIndexSize();
/*

		if ( index < getBaseMethodCount() )
			return CAgentScript::runMethodeMember(index, params);
*/
		IObjectIA::CProcessResult r;

		char buf[1024];
		getDebugString(buf);

//		index = index - getBaseMethodCount();
		
		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_onActivate )
		{
			onActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_unActivate )
		{
			unActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_onUnActivate )
		{
			onUnActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_switch )
		{
			std::vector<CStringType *> handles;
			if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
			{
#ifdef _DEBUG
				const char *dbg_param_type = (const char *) params->getType();
				char dbg_param_string[1024 * 8];
				params->getDebugString(dbg_param_string);
#endif
				const IObjectIA *fw = ( ((NLAIAGENT::IBaseGroupType *)params) )->getFront();
#ifdef _DEBUG
				const char *dbg_param_front_type = (const char *) fw->getType();
#endif

				( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
//				while ( fw->size() )
//				{
					handles.push_back( (CStringType *) fw);
//					fw->popFront();
//				}

				std::vector<CComponentHandle *> switched;
				for ( int i = 0; i < (int) handles.size(); i++)
					switched.push_back( new CComponentHandle( handles[ i ]->getStr(), (IAgent *) getParent() ) );
				switchActor( switched, false );
			}
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
		return r;
	}

	int CActorScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount() + fid_last;
	}


	tQueue CActorScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

		const char *txt = name->getString();

		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;

		if ( *name == CStringVarName("activate") )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + fid_activate, 0.0,NULL, r_type ) );
		}

		if ( *name == CStringVarName("onActivate") )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_onActivate , 0.0,NULL, r_type ) );
		}


		if ( *name == CStringVarName("unActivate") )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_onUnActivate, 0.0,NULL, r_type ) );
		}

		if ( *name == CStringVarName("onUnActivate") )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_unActivate, 0.0,NULL, r_type ) );
		}

		if ( *name == CStringVarName("switch") )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_switch, 0.0, NULL, r_type ) );
		}

		if(_AgentClass != NULL)
		{
			tQueue r = _AgentClass->isMember(className, name, param);
			if(r.size() != 0) return r;
		}
		return result;
	}
}