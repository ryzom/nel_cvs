/** \file ident.cpp
 *
 * $Id: ident.cpp,v 1.20 2002/06/13 19:15:53 portier Exp $
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
#include "nel/ai/agent/agent.h"

namespace NLAIAGENT
{	
	const uint8 CAgentNumber::AgentTypeBit = 0x81;
	const CAgentNumber CAgentNumber::Unknow(0,CAgentNumber::ServerId,CAgentNumber::ServerId);

	uint64 atoiInt64(const char *ident)
	{
		uint64 k = 0;

		while(*ident != 0)
		{
			if(*ident == ':') break;

			switch(*(ident++))
			{
			case '0':
				k |= 0;
				break;
			case '1':
				k |= 1;
				break;
			case '2':
				k |= 2;
				break;
			case '3':
				k |= 3;
				break;
			case '4':
				k |= 4;
				break;
			case '5':
				k |= 5;
				break;
			case '6':
				k |= 6;
				break;
			case '7':
				k |= 7;
				break;
			case '8':
				k |= 8;
				break;
			case '9':
				k |= 9;
				break;
			case 'a':
				k |= 10;
				break;
			case 'b':
				k |= 11;
				break;
			case 'c':
				k |= 12;
				break;
			case 'd':
				k |= 13;
				break;
			case 'e':
				k |= 14;
				break;
			case 'f':
				k |= 15;
				break;

			case 'A':
				k |= 10;
				break;
			case 'B':
				k |= 11;
				break;
			case 'C':
				k |= 12;
				break;
			case 'D':
				k |= 13;
				break;
			case 'E':
				k |= 14;
				break;
			case 'F':
				k |= 15;
				break;

			case 0:
				return k;
			case ':':
				return k;
				break;
			}
			if(*ident != 0 && *ident != ':') k <<= 4;
		}

		return k;
	}	
		

	CLocWordNumRef::~CLocWordNumRef()
	{
		NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
		tMapRef::iterator Itr = a.value().find(_Id);
		if(Itr != a.value().end())
		{				
			a.value().erase(Itr);
		}
		else throw NLAIE::CExceptionIndexHandeledError();
	}

	const NLAIC::CIdentType &CLocWordNumRef::getType() const
	{		
		return *IdLocWordNumRef;
	}

	IRefrence *CLocWordNumRef::getRef(const CNumericIndex &id)
	{
		NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
		tMapRef::iterator Itr = a.value().find(id);
		if(Itr != a.value().end())
		{				
			return (*Itr).second;
		}
		/*else
		{
			throw NLAIE::CExceptionIndexHandeledError();
		}*/		
		return NULL;
	}

	void CLocWordNumRef::Init()
	{
		/*NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
		a.value() = new CLocWordNumRef::tMapRef;*/
	}

	void CLocWordNumRef::clear()
	{			
		/*NLMISC::CSynchronized<tMapRef >::CAccessor a(&_LocRefence);
		if(a.value() )
		{
			delete a.value();
			a.value() = NULL;
		}*/			
	}

	void releaseAgentLib()
	{
		//CLocWordNumRef::clear();
		//CIndexedVarName::releaseClass();
	}

	void initAgentLib()
	{
		CLocWordNumRef::Init();		
		CIndexedVarName::initClass();
	}	
}
