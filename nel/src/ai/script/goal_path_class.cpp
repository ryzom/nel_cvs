#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"

#include "nel/ai/script/goal_path_class.h"
#include "nel/ai/logic/goal_path.h"

namespace NLAISCRIPT
{
	CGoalPathClass::CGoalPathClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass())));		
	}
	
	CGoalPathClass::CGoalPathClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass())));		
	}

	CGoalPathClass::CGoalPathClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass())));		
	}

	CGoalPathClass::CGoalPathClass(const CGoalPathClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass())));		
	}	

	CGoalPathClass::CGoalPathClass()
	{
		setBaseMethodCount(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAILOGIC::CGoalPath *)(NLAILOGIC::CGoalPath::IdGoalPath.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *CGoalPathClass::clone() const
	{
		NLAIC::IBasicType *clone = new CGoalPathClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *CGoalPathClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CGoalPathClass();
		return instance;
	}

	void CGoalPathClass::getDebugString(std::string &t) const
	{
	}

	NLAIAGENT::IObjectIA *CGoalPathClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAILOGIC::CGoalPath *instance = new NLAILOGIC::CGoalPath( NULL, NULL ,components,  (CGoalPathClass *) this );
		return instance;
	}

	CGoalPathClass::~CGoalPathClass()
	{
	}
}
