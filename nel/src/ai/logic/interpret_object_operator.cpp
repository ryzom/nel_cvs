/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX <MODULE_NAME>.
 * NEVRAX <MODULE_NAME> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX <MODULE_NAME> is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX <MODULE_NAME>; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/ai/c/registry_class.h"
#include "nel/ai/logic/operator_script.h"
#include "nel/ai/script/lexsupport.h"

#include "nel/ai/logic/interpret_object_operator.h"

namespace NLAISCRIPT
{
	using namespace NLAIAGENT;


	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
//		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());		
//		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}
	
	COperatorClass::COperatorClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
//		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
//		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const COperatorClass &c) : CAgentClass( c )
	{
//		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}	

	COperatorClass::COperatorClass()
	{
//		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));
	}

	const NLAIC::IBasicType *COperatorClass::clone() const
	{
		NLAIC::IBasicType *clone = new COperatorClass(*this);
		clone->incRef();
		return clone;
	}

	const NLAIC::IBasicType *COperatorClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new COperatorClass();
		instance->incRef();
		return instance;
	}

	void COperatorClass::getDebugString(char *t) const
	{
	}

	NLAIAGENT::IObjectIA *COperatorClass::buildNewInstance() const
	{
		// Création des composants statiques
		/*std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAILOGIC::COperatorScript *instance;// = new NLAIAGENT::COperatorScript( components,  (COperatorClass *) this );
		instance->incRef();

		return instance;*/
		return NULL;
	}

	COperatorClass::~COperatorClass()
	{
	}
}
