/** \file digital.cpp
 *
 * $Id: digital.cpp,v 1.10 2001/12/19 14:35:45 robert Exp $
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
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{

	// DigitalType
	DigitalType DigitalType::NullOperator = DigitalType(0.0);
	
	const NLAIC::CIdentType &DigitalType::getType() const
	{		
		return IdDigitalType;
	}
	
	const IObjectIA::CProcessResult &DigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}
	
	// BorneDigitalType
	const IObjectIA::CProcessResult &BorneDigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &BorneDigitalType::getType() const
	{		
		return IdBorneDigitalType;
	}	

	// DDigitalType
	const NLAIC::CIdentType &DDigitalType::getType() const
	{		
		return IdDDigitalType;
	}
	
	const IObjectIA::CProcessResult &DDigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}
	
	// BornedDDigitalType
	const IObjectIA::CProcessResult &BorneDDigitalType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &BorneDDigitalType::getType() const
	{		
		return IdBorneDDigitalType;
	}	

	// IntegerType
	const IObjectIA::CProcessResult &IntegerType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &IntegerType::getType() const
	{		
		return IdIntegerType;
	}	

	// ShortIntegerType
	const IObjectIA::CProcessResult &ShortIntegerType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &ShortIntegerType::getType() const
	{		
		return IdShortIntegerType;
	}	

	// CharType
	const IObjectIA::CProcessResult &CharType::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &CharType::getType() const
	{		
		return IdCharType;
	}	

	// UInt32Type
	const IObjectIA::CProcessResult &UInt32Type::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &UInt32Type::getType() const
	{		
		return IdUInt32Type;
	}	

	// UInt8Type
	const IObjectIA::CProcessResult &UInt8Type::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &UInt8Type::getType() const
	{		
		return IdUInt8Type;
	}	

	// UInt16Type
	const IObjectIA::CProcessResult &UInt16Type::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &UInt16Type::getType() const
	{		
		return IdUInt16Type;
	}	

	// UInt64Type
	const IObjectIA::CProcessResult &UInt64Type::run()
	{
		return IObjectIA::ProcessRun;
	}

	const NLAIC::CIdentType &UInt64Type::getType() const
	{		
		return IdUInt64Type;
	}	

	double UInt64Type::getNumber() const
	{
		throw NLAIE::CExceptionNotImplemented("No getNumber on a uint64.");
		return 0.0;
	}
}
