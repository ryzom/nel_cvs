/** \file interpret_fuzzy_controler.cpp
 * Fuzzy controler class for the scripting language
 *
 * $Id: interpret_fuzzy_controler.cpp,v 1.1 2001/01/05 10:53:49 chafik Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "fuzzy/interpret_fuzzy_controler.h"
#include "fuzzy/fuzzy_script.h"

namespace NLIASCRIPT
{

	CFuzzyControlerClass::CFuzzyControlerClass(const NLIAAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}
	
	CFuzzyControlerClass::CFuzzyControlerClass(const NLIAC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}

	CFuzzyControlerClass::CFuzzyControlerClass(const NLIAAGENT::IVarName &n, const NLIAAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}

	CFuzzyControlerClass::CFuzzyControlerClass(const CFuzzyControlerClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}	

	CFuzzyControlerClass::CFuzzyControlerClass()
	{
		setBaseMethodCount(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLIAFUZZY::CFuzzyControlerScript *)(NLIAFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));
	}


	const NLIAC::IBasicType *CFuzzyControlerClass::clone() const
	{
		NLIAC::IBasicType *clone = new CFuzzyControlerClass(*this);
		clone->incRef();
		return clone;
	}

	const NLIAC::IBasicType *CFuzzyControlerClass::newInstance() const
	{
		NLIAC::IBasicType *instance = new CFuzzyControlerClass();
		instance->incRef();
		return instance;
	}

	void CFuzzyControlerClass::getDebugString(char *t) const
	{
	}

	NLIAAGENT::IObjectIA *CFuzzyControlerClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLIAAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLIAFUZZY::CFuzzyControlerScript *instance = new NLIAFUZZY::CFuzzyControlerScript( components,  (CFuzzyControlerClass *) this );
		instance->incRef();

		return instance;
	}

	CFuzzyControlerClass::~CFuzzyControlerClass()
	{
	}

	const std::vector<NLIAAGENT::IObjectIA *> &CFuzzyControlerClass::getInputs() const
	{
		return _Inputs;
	}

	const std::vector<NLIAAGENT::IObjectIA *> &CFuzzyControlerClass::getOutputs() const
	{
		return _Outputs;
	}

	void CFuzzyControlerClass::addInput(NLIAAGENT::IObjectIA *input)
	{
		_Inputs.push_back( input );
	}

	void CFuzzyControlerClass::addOutput(NLIAAGENT::IObjectIA *output)
	{
		_Outputs.push_back( output );
	}

	const NLIAAGENT::IVarName *CFuzzyControlerClass::getInputName(sint32 id)
	{
		if ( id > (sint32) _InputNames.size() )
		{
			// TODO: throw exception
		}
		return _InputNames[ id ];
	}

	const NLIAAGENT::IVarName *CFuzzyControlerClass::getOutputName(sint32 id)
	{
		if ( id > (sint32) _OutputNames.size() )
		{
			// TODO: throw exception
		}
		return _OutputNames[ id ];
	}
}