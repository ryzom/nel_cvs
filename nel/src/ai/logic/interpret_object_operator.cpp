#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/logic/operator_script.h"

namespace NLAISCRIPT

{
	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}
	
	COperatorClass::COperatorClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const COperatorClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}	

	COperatorClass::COperatorClass()
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *COperatorClass::clone() const
	{
		NLAIC::IBasicType *clone = new COperatorClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *COperatorClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new COperatorClass();
		return instance;
	}

	void COperatorClass::getDebugString(char *t) const
	{
	}

	NLAIAGENT::IObjectIA *COperatorClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::COperatorScript *instance = new NLAIAGENT::COperatorScript( NULL, NULL ,components,  (COperatorClass *) this );
		return instance;
	}

	COperatorClass::~COperatorClass()
	{
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	/// Verifies if the preconditions are validated
	bool CFirstOrderOperator::isValid(CFactBase *fb)
	{		
		std::list<CFact *> *facts = new std::list<CFact *>;
		for (sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			std::list<CFact *> *fa = fb->getAssertFacts( _Conds[i] );
			while ( fa->size() )
			{
				facts->push_back( fa->front() );
				fa->pop_front();
			}
			delete fa;
		}
		std::list<CFact *> *res = propagate( *facts );
		bool is_valid = !res->empty();
		while ( res->size() )
		{
			char buffer[2054];
			res->front()->getDebugString( buffer );
			res->front()->release();
			res->pop_front();
		}
		delete res;

		while ( facts->size() )
		{
			facts->front()->release();
			facts->pop_front();	
		}
		delete facts;

		return is_valid;
	}


	float CFirstOrderOperator::priority() const
	{
		return 0.0;
	}

	void CFirstOrderOperator::success()
	{
	}

	void CFirstOrderOperator::failure()
	{
	}

	void CFirstOrderOperator::success(IBaseOperator *)
	{
	}

	void CFirstOrderOperator::failure(IBaseOperator *)
	{
	}

  */
}
