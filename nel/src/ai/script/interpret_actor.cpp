#include "c/registry_class.h"
#include "agent/actor_script.h"
#include "script/lexsupport.h"

#include "script/interpret_actor.h"
#include "agent/actor_script.h"

namespace NLIASCRIPT
{
	CActorClass::CActorClass(const NLIAAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}
	
	CActorClass::CActorClass(const NLIAC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}

	CActorClass::CActorClass(const NLIAAGENT::IVarName &n, const NLIAAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}

	CActorClass::CActorClass(const CActorClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass())));		
	}	

	CActorClass::CActorClass()
	{
		setBaseMethodCount(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAAGENT::CActorScript *)(NLIAAGENT::CActorScript::IdActorScript.getFactory()->getClass())));
	}


	const NLIAC::IBasicType *CActorClass::clone() const
	{
		NLIAC::IBasicType *clone = new CActorClass(*this);
		clone->incRef();
		return clone;
	}

	const NLIAC::IBasicType *CActorClass::newInstance() const
	{
		NLIAC::IBasicType *instance = new CActorClass();
		instance->incRef();
		return instance;
	}

	void CActorClass::getDebugString(char *t) const
	{
	}

	NLIAAGENT::IObjectIA *CActorClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLIAAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLIAAGENT::CActorScript *instance = new NLIAAGENT::CActorScript( NULL, NULL ,components,  (CActorClass *) this );
		instance->incRef();

		return instance;
	}

	CActorClass::~CActorClass()
	{
	}
}
