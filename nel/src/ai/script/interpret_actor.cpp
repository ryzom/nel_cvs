#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"

#include "nel/ai/script/interpret_actor.h"
#include "nel/ai/agent/actor_script.h"

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
		return clone;
	}

	const NLAIC::IBasicType *CActorClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CActorClass();
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
		return instance;
	}

	CActorClass::~CActorClass()
	{
	}
}
