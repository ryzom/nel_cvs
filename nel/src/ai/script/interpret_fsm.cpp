#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/script/interpret_fsm.h"
#include "nel/ai/logic/fsm_script.h"
#include "nel/ai/logic/fsm_seq_script.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/script/codage.h"

namespace NLAISCRIPT
{
	CFsmClass::CFsmClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass())));		
	}
	
	CFsmClass::CFsmClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass())));		
	}

	CFsmClass::CFsmClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass())));		
	}

	CFsmClass::CFsmClass(const CFsmClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass())));		
	}	

	CFsmClass::CFsmClass()
	{
		setBaseMethodCount(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass())));
	}

	const NLAIC::IBasicType *CFsmClass::clone() const
	{
		NLAIC::IBasicType *clone = new CFsmClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *CFsmClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CFsmClass(*this);
		return instance;
	}

	void CFsmClass::getDebugString(std::string &t) const
	{
		CAgentClass::getDebugString(t);
	}

	NLAIAGENT::IObjectIA *CFsmClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::CFsmScript *instance = new NLAIAGENT::CFsmScript( NULL, NULL ,components,  (CFsmClass *) this );

		return instance;
	}

	CFsmClass::~CFsmClass()
	{
	}

	void CFsmClass::success()
	{
	}

	void CFsmClass::failure()
	{
	}

	void CFsmClass::success(NLAILOGIC::IBaseOperator *)
	{
	}

	void CFsmClass::failure(NLAILOGIC::IBaseOperator *)
	{
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CSeqFsmClass::CSeqFsmClass(const NLAIAGENT::IVarName &n) : CFsmClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass())));		
	}
	
	CSeqFsmClass::CSeqFsmClass(const NLAIC::CIdentType &id): CFsmClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass())));		
	}

	CSeqFsmClass::CSeqFsmClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CFsmClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass())));		
	}

/*	CSeqFsmClass::CSeqFsmClass(const CSeqFsmClass &c) : CFsmClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CFsmScript *)(NLAIAGENT::CFsmScript::IdFsmScript.getFactory()->getClass())));		
	}	
	*/

	CSeqFsmClass::CSeqFsmClass()
	{
		setBaseMethodCount(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::CSeqFsmScript *)(NLAIAGENT::CSeqFsmScript::IdSeqFsmScript.getFactory()->getClass())));		
	}

	const NLAIC::IBasicType *CSeqFsmClass::clone() const
	{
		NLAIC::IBasicType *clone = new CSeqFsmClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *CSeqFsmClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CSeqFsmClass();
		return instance;
	}

	void CSeqFsmClass::getDebugString(char *t) const
	{
		strcpy( t, "<CSeqFsmClass>\n");
//		int i;
/*		for ( i = 0; i < (int) _Vars.size(); i++ )
		{
			char buf[1024];
			_Vars[i]->getDebugString(buf);
			strcat(t,"   -");
			strcat(t, buf);
			strcat(t,"\n");
		}
		*/
	}

	NLAIAGENT::IObjectIA *CSeqFsmClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::CSeqFsmScript *instance = new NLAIAGENT::CSeqFsmScript( NULL, NULL ,components,  (CSeqFsmClass *) this );

		return instance;
	}

	const NLAIAGENT::IVarName *CSeqFsmClass::getStep(sint32 step)
	{
		return _Steps[ step ];
	}

	CSeqFsmClass::~CSeqFsmClass()
	{
		for ( int i = 0; i < (int) _Steps.size(); i++ )
		{
			((NLAIAGENT::IVarName *)_Steps[i])->release();
		}
	}

	void CSeqFsmClass::success()
	{
	}

	void CSeqFsmClass::failure()
	{
	}

	void CSeqFsmClass::success(NLAILOGIC::IBaseOperator *)
	{
	}

	void CSeqFsmClass::failure(NLAILOGIC::IBaseOperator *)
	{
	}

	void CSeqFsmClass::addStep(const NLAIAGENT::IVarName &step_name)
	{
		_Steps.push_back( (NLAIAGENT::IVarName *) step_name.clone() );
	}
}
