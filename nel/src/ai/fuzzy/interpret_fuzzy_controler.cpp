/** \file interpret_fuzzy_controler.cpp
 * Fuzzy controler class for the scripting language
 *
 * $Id: interpret_fuzzy_controler.cpp,v 1.2 2001/01/08 10:48:01 chafik Exp $
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

	CFuzzyControlerClass::CFuzzyControlerClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}
	
	CFuzzyControlerClass::CFuzzyControlerClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}

	CFuzzyControlerClass::CFuzzyControlerClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}

	CFuzzyControlerClass::CFuzzyControlerClass(const CFuzzyControlerClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));		
	}	

	CFuzzyControlerClass::CFuzzyControlerClass()
	{
		setBaseMethodCount(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIFUZZY::CFuzzyControlerScript *)(NLAIFUZZY::CFuzzyControlerScript::IdFuzzyControlerScript.getFactory()->getClass())));
	}


	const NLAIC::IBasicType *CFuzzyControlerClass::clone() const
	{
		NLAIC::IBasicType *clone = new CFuzzyControlerClass(*this);
		clone->incRef();
		return clone;
	}

	const NLAIC::IBasicType *CFuzzyControlerClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new CFuzzyControlerClass();
		instance->incRef();
		return instance;
	}

	void CFuzzyControlerClass::getDebugString(char *t) const
	{
	}

	NLAIAGENT::IObjectIA *CFuzzyControlerClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIFUZZY::CFuzzyControlerScript *instance = new NLAIFUZZY::CFuzzyControlerScript( components,  (CFuzzyControlerClass *) this );
		instance->incRef();

		return instance;
	}

	CFuzzyControlerClass::~CFuzzyControlerClass()
	{
	}

	const std::vector<NLAIAGENT::IObjectIA *> &CFuzzyControlerClass::getInputs() const
	{
		return _Inputs;
	}

	const std::vector<NLAIAGENT::IObjectIA *> &CFuzzyControlerClass::getOutputs() const
	{
		return _Outputs;
	}

	void CFuzzyControlerClass::addInput(NLAIAGENT::IObjectIA *input)
	{
		_Inputs.push_back( input );
	}

	void CFuzzyControlerClass::addOutput(NLAIAGENT::IObjectIA *output)
	{
		_Outputs.push_back( output );
	}

	const NLAIAGENT::IVarName *CFuzzyControlerClass::getInputName(sint32 id)
	{
		if ( id > (sint32) _InputNames.size() )
		{
			// TODO: throw exception
		}
		return _InputNames[ id ];
	}

	const NLAIAGENT::IVarName *CFuzzyControlerClass::getOutputName(sint32 id)
	{
		if ( id > (sint32) _OutputNames.size() )
		{
			// TODO: throw exception
		}
		return _OutputNames[ id ];
	}
}