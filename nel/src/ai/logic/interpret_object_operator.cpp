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

#include "c/registry_class.h"
#include "logic/operator_script.h"
#include "script/lexsupport.h"

#include "logic/interpret_object_operator.h"

namespace NLIASCRIPT
{
	using namespace NLIAAGENT;


	COperatorClass::COperatorClass(const NLIAAGENT::IVarName &n) : CAgentClass(n)
	{
//		setBaseMethodCount(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());		
//		setBaseObjectInstance(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}
	
	COperatorClass::COperatorClass(const NLIAC::CIdentType &id): CAgentClass(id)
	{
//		setBaseMethodCount(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const NLIAAGENT::IVarName &n, const NLIAAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
//		setBaseMethodCount(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const COperatorClass &c) : CAgentClass( c )
	{
//		setBaseMethodCount(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}	

	COperatorClass::COperatorClass()
	{
//		setBaseMethodCount(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
//		setBaseObjectInstance(((NLIAAGENT::COperatorScript *)(NLIAAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));
	}

	const NLIAC::IBasicType *COperatorClass::clone() const
	{
		NLIAC::IBasicType *clone = new COperatorClass(*this);
		clone->incRef();
		return clone;
	}

	const NLIAC::IBasicType *COperatorClass::newInstance() const
	{
		NLIAC::IBasicType *instance = new COperatorClass();
		instance->incRef();
		return instance;
	}

	void COperatorClass::getDebugString(char *t) const
	{
	}

	NLIAAGENT::IObjectIA *COperatorClass::buildNewInstance() const
	{
		// Création des composants statiques
		/*std::list<NLIAAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLIALOGIC::COperatorScript *instance;// = new NLIAAGENT::COperatorScript( components,  (COperatorClass *) this );
		instance->incRef();

		return instance;*/
		return NULL;
	}

	COperatorClass::~COperatorClass()
	{
	}
}