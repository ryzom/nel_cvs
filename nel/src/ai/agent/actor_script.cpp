#include "nel/ai/agent/actor_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"

namespace NLAIAGENT
{
	CActorScript::CActorScript(const CActorScript &m) : CAgentScript(m)
	{
		_IsActivated = m._IsActivated;
	}

	CActorScript::CActorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CActorClass *actor_class )
	
	: CAgentScript(manager, father, components, actor_class )
	{	
	}	

	CActorScript::CActorScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
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
	void CActorScript::forwardActivity(CActorScript *receiver, bool stay_active)
	{
		receiver->activate();

		if ( !stay_active )
			unActivate();
	}
		
	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::forwardActivity(std::vector<CActorScript *> &actors, bool stay_active)
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
		m->incRef();
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
			instance->incRef();
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

	IObjectIA *CActorScript::run(const IMessageBase &msg)
	{
		return IAgent::run( msg );
	}

	const NLAIC::CIdentType &CActorScript::getType() const
	{		
		return IdActorScript;
	}

	void CActorScript::save(NLMISC::IStream &os)
	{
		IAgent::save(os);
		uint8 b = (uint8 ) _IsActivated;

		os.serial( b );
		
	}

	void CActorScript::load(NLMISC::IStream &is)
	{
		IAgent::load(is);
		
	}


	sint32 CActorScript::getMethodIndexSize() const
	{
		return CAgentScript::getMethodIndexSize() + 3;
	}

	IObjectIA::CProcessResult CActorScript::runMethodeMember(sint32 index, IObjectIA *c)	
	{
		
		return CAgentScript::runMethodeMember(index,c);
	}

	tQueue CActorScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		
		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;

//		if(className != NULL) 
//		{
			if ( *name == CStringVarName("activate") )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				r_type->incRef();
				result.push( NLAIAGENT::CIdMethod( fid_activate + IAgent::getMethodIndexSize(), 0.0,NULL, r_type ) );
			}

			if ( *name == CStringVarName("unActivate") )
			{
				CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				r_type->incRef();
				result.push( NLAIAGENT::CIdMethod( fid_unActivate + IAgent::getMethodIndexSize(), 0.0,NULL, r_type ) );
			}

			if ( *name == CStringVarName("forwardActivity") )
			{
				CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				r_type->incRef();
				result.push( NLAIAGENT::CIdMethod( fid_forwardActivity + IAgent::getMethodIndexSize(), 0.0,NULL, r_type ) );
			}
//		}
		
		if(_AgentClass != NULL)
		{
			tQueue r = _AgentClass->isMember(className, name, param);
			if(r.size() != 0) return r;
		}

		return IAgent::isMember(className, name, param);
	}
}
