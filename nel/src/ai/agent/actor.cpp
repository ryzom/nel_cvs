#include "nel/ai/agent/actor.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"

namespace NLAIAGENT
{
	/// Default constructor
	CActor::CActor() : IAgent(NULL)
	{
	}

	/// Builds and actor with its father
	CActor::CActor(IAgent *father, bool activated) : IAgent( father )
	{
		_IsActivated = activated;
	}

	/// copy constructor
	CActor::CActor(const CActor &c) : IAgent( c )
	{
		_IsActivated = c._IsActivated;
	}

	CActor::~CActor()
	{
//		for (int i = 0; i < c._Transitions.size(); i++ )
		//	_Transitions[i]->release();
	}

	/// Returns true if the actor has a token
	bool CActor::isActivated()
	{
		return _IsActivated;
	}

	/// Activates the actor
	void CActor::activate()
	{
		if ( !_IsActivated )
		{
			onActivate();
			_IsActivated = true;
		}
	}

	/// Unactivates the actor
	void CActor::unActivate()
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
	void CActor::forwardActivity(CActor *receiver, bool stay_active)
	{
		receiver->activate();

		if ( !stay_active )
			unActivate();
	}
		
	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActor::forwardActivity(std::vector<CActor *> &actors, bool stay_active)
	{
		std::vector<CActor *>::iterator it_act = actors.begin();
		while ( it_act != actors.end() )
		{
			( *it_act )->activate();
			it_act++;
		}
		// TODO: Envoi de message "activate" 
		if ( !stay_active )
			unActivate();
	}

	/// Adds a transition, ie a condition and a list of newly activated agents
	void CActor::addTransition(NLAILOGIC::IBaseCond *cond, std::vector<CActor *> &outputs, bool stay_alive)
	{

	}

//	void CActor::addTransition(CTransition *trans)
//	{
//		_Transitions.push_back( trans );
//	}

	/// Callback called at the activation of the actor
	void CActor::onActivate()
	{
		// Default behaviour: do nothing
	}
	/// Callback called when the agent is unactivated
	void CActor::onUnActivate()
	{
		// default behaviour: do nothing
	}

	const NLAIC::IBasicType *CActor::clone() const
	{		
		CActor *m = new CActor(*this);
		return m;
	}		

	const NLAIC::IBasicType *CActor::newInstance() const
	{	
		NLAIC::IBasicInterface *m;
		if ( getParent() != NULL ) 
			m = new	CActor((IAgent *)getParent());
		else 
			m = new CActor(NULL);
		return m;
	}	

	void CActor::getDebugString(std::string &t) const
	{
		t = "CActor ";
		if ( _IsActivated )
			t += "<active>";
		else
			t += "<idle>";
	}

	bool CActor::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	void CActor::processMessages()
	{
		IAgent::processMessages();
/*		while(getMail()->getMessageCount())
		{
			const IMessageBase &msg = getMail()->getMessage();				
			run( msg );
			getMail()->popMessage();
		}*/
	}

	const IObjectIA::CProcessResult &CActor::run()
	{
		if ( _IsActivated )
		{
			return IAgent::run();
		}
		else
			return IObjectIA::ProcessRun;
				
	}

	/*IObjectIA *CActor::run(const IMessageBase &msg)
	{
		return IAgent::run( msg );
	}*/

	const NLAIC::CIdentType &CActor::getType() const
	{		
		return IdActor;
	}

	void CActor::save(NLMISC::IStream &os)
	{
		IAgent::save(os);
		os.serial( (bool &) _IsActivated );
		
	}

	void CActor::load(NLMISC::IStream &is)
	{
		IAgent::load(is);
	}

	TQueue CActor::isMember(const IVarName *className,const NLAIAGENT::IVarName *name,const IObjectIA &param) const
	{
		TQueue result;

		result = IAgent::isMember( className, name, param );

		if ( result.size() )
			return result;

//		if(className != NULL) 
//		{
			if ( *name == CStringVarName("activate") )
			{
				CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod( fid_activate + IAgent::getMethodIndexSize(), 0.0,NULL, r_type ) );
			}

			if ( *name == CStringVarName("unActivate") )
			{
				CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod( fid_unActivate + IAgent::getMethodIndexSize(), 0.0,NULL, r_type ) );
			}

			if ( *name == CStringVarName("forwardActivity") )
			{
				CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod( fid_forwardActivity + IAgent::getMethodIndexSize(), 0.0,NULL, r_type ) );
			}
//		}
		return result;
	}

	// Executes a method from its index id and with its parameters
	IObjectIA::CProcessResult CActor::runMethodeMember(sint32 id, IObjectIA *params)
	{
		if ( id < IAgent::getMethodIndexSize() )
			return IAgent::runMethodeMember(id, params);

		IObjectIA::CProcessResult r;

#ifdef NL_DEBUG
		std::string buf;
		getDebugString(buf);
#endif

		id = id - IAgent::getMethodIndexSize();
		
		if ( id == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( id == fid_unActivate )
		{
			unActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( id == fid_forwardActivity )
		{
			std::vector<CActor *> forwarded;
			if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
			{
				IBaseGroupType *fw = (IBaseGroupType *) ( ((NLAIAGENT::IBaseGroupType *)params) )->getFront();
				( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
				while ( fw->size() )
				{
					forwarded.push_back( (CActor *) fw->getFront() );
					fw->popFront();
				}
				forwardActivity( forwarded, false );
			}
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
		return r;
	}

	sint32 CActor::getMethodIndexSize() const
	{
		return IAgent::getMethodIndexSize() + 2;
	}
}
