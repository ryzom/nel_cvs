/** \file ident.cpp
 *
 * $Id: ident.cpp,v 1.14 2001/12/04 12:53:21 chafik Exp $
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

	uint8 CAgentNumber::ServerID = 0;

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

	CAgentNumber::CAgentNumber(const char *i)
	{
		char *ident = (char*)i;
		char *id;
		char *creator;
		char *dyn;
		id = ident;

		sint n = 0;
		while(*(ident++) != ':');		
		creator = ident;
		while(*(ident++) != ':');		
		dyn = ident;	

		AgentNumber = atoiInt64(id);
		CreatorId = atoiInt64(creator);
		DynamicId = atoiInt64(dyn);
	}
	

	void CAgentNumber::getDebugString(std::string &str) const 
	{											
		char b[256];
		memset(b,0,255);
		memset(b,'0',18);
		sint n;
		uint64 x = AgentNumber;
		char baseTable[] = "0123456789abcdef";
		for(n = 7; n < 18; n ++)
		{
			b[18 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[18 - 6] = ':';
		x = CreatorId;
		for(n = 4; n < 6; n ++)
		{				
			b[18 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		b[18 - 3] = ':';

		x = DynamicId;
		for(n = 1; n < 3; n ++)
		{							
			b[18 - n] = baseTable[(x & 15)];
			x >>= 4;
		}
		str += std::string(b);
	}

	CLocWordNumRef::~CLocWordNumRef()
	{
		tMapRef::iterator Itr = CLocWordNumRef::_LocRefence->find(_Id);
		if(Itr != CLocWordNumRef::_LocRefence->end())
		{				
			CLocWordNumRef::_LocRefence->erase(Itr);
		}
		else throw NLAIE::CExceptionIndexHandeledError();
	}

	const NLAIC::CIdentType &CLocWordNumRef::getType() const
	{		
		return *IdLocWordNumRef;
	}

	IRefrence *CLocWordNumRef::getRef(const CNumericIndex &id)
	{
		tMapRef::iterator Itr = CLocWordNumRef::_LocRefence->find(id);
		if(Itr != CLocWordNumRef::_LocRefence->end())
		{				
			return (*Itr).second;
		}
		//else throw NLAIE::CExceptionIndexHandeledError();
		return NULL;
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
