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

#include "nel/ai/logic/ia_assert.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/logic/clause.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/logic/operator.h"
#include "nel/ai/logic/fo_operator.h"
#include "nel/ai/logic/fact.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;
	
	
	IBaseAssert::IBaseAssert(const IVarName &n )
	{
		_Name = (IVarName *) n.clone();
	}

	IBaseAssert::~IBaseAssert()
	{
		if 	( _Name )
			_Name->release();

		sint32 i;
		for (i = 0; i < (sint32) _Inputs.size(); i++ )
			_Inputs[i]->release();	

		for (i = 0; i < (sint32) _Outputs.size(); i++ )
			_Outputs[i]->release();
	}

	void IBaseAssert::addInput(IBaseOperator *op)
	{
		_Inputs.push_back( op );
		op->incRef();
	}

	const std::vector<IBaseOperator *> &IBaseAssert::getInputs()
	{
		return _Inputs;
	}

	void IBaseAssert::addOutput(IBaseOperator *op)
	{
		_Outputs.push_back( op );
	}

	const std::vector<IBaseOperator *> &IBaseAssert::getOutputs()
	{
		return _Outputs;
	}
}