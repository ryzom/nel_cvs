#include "c/registry_class.h"
#include "agent/actor_script.h"
#include "script/lexsupport.h"

#include "script/interpret_actor.h"
#include "agent/actor_script.h"

namespace NLAISCRIPT
{
	CActorClass::CActorClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}
	
	CActorClass::CActorClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}

	CActorClass::CActorClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}

	CActorClass::CActorClass(const CActorClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}	

	CActorClass::CActorClass()
	{
		setBaseMethodCount(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CActorScript *)(NLAIAGENT::CActorScript::IdActorScript.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *CActorClass::clone() const
	{
		NLAIC::IBasicType *clone = new CActorClass(*this);
		clone->incRef();
		return clone;
	}

	const NLAIC::IBasicType *CActorClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CActorClass();
		instance->incRef();
		return instance;
	}

	void CActorClass::getDebugString(char *t) const
	{
	}

	NLAIAGENT::IObjectIA *CActorClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::CActorScript *instance = new NLAIAGENT::CActorScript( NULL, NULL ,components,  (CActorClass *) this );
		instance->incRef();

		return instance;
	}

	CActorClass::~CActorClass()
	{
	}
}
