/** \file msg_stream.cpp
 *
 * $Id: msg_stream.cpp,v 1.2 2002/03/25 15:21:55 chafik Exp $
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
#include "nel/ai/agent/msg_stream.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{

	template<class X, class Y>
	X getValue(const IObjectIA *o)
	{		
		return (X)((const Y *)o)->getValue();
	}

	template<class X, class Y>
	Y *getValue(X v)
	{		
		return new Y(v);
	}

//####################
//class CMsgIStream
//####################
	void CMsgIStream::serial(uint8 &b) throw(NLMISC::EStream)
	{		
		b = getValue<uint8, UInt8Type>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(sint8 &b) throw(NLMISC::EStream)
	{
		b = getValue<sint8, CharType>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(uint16 &b) throw(NLMISC::EStream)
	{
		b = getValue<uint16, UInt16Type>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(sint16 &b) throw(NLMISC::EStream)
	{
		b = getValue<sint16, ShortIntegerType>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(uint32 &b) throw(NLMISC::EStream)
	{
		b = getValue<uint32, UInt32Type>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(sint32 &b) throw(NLMISC::EStream)
	{
		b = getValue<sint32, IntegerType>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(uint64 &b) throw(NLMISC::EStream)
	{
		b = getValue<uint64, UInt64Type>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(sint64 &b) throw(NLMISC::EStream)
	{
		b = getValue<sint64, UInt64Type>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(float &b) throw(NLMISC::EStream)
	{
		b = getValue<float, DigitalType>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(double &b) throw(NLMISC::EStream)
	{
		b = getValue<double, DDigitalType>(_It++);
		_Pos ++;
	}

	void CMsgIStream::serial(bool &b) throw(NLMISC::EStream)
	{
		b = getValue<uint8, UInt8Type>(_It++) ? true : false;
		_Pos ++;
	}

#ifndef NL_OS_CYGWIN
	void CMsgIStream::serial(char &b) throw(NLMISC::EStream)
	{
		b = getValue<sint8, CharType>(_It++);
		_Pos ++;
	}
#endif

	void CMsgIStream::serial(std::string &b) throw(NLMISC::EStream)
	{
		CStringType *s= (CStringType *)(_It++);
		b = s->getStr().getString();
		_Pos ++;
	}

	void CMsgIStream::serial(ucstring &b) throw(NLMISC::EStream)
	{
		throw NLMISC::EReadError((const char *)_List.getType());
	}

	void CMsgIStream::serial(IObjectIA &b) throw(NLMISC::EStream)
	{
		b.load(*this);
	}

	void CMsgIStream::serial(IObjectIA* &b) throw(NLMISC::EStream)
	{
		NLAIC::CIdentTypeAlloc id( *this );
		b = (IObjectIA *)id.allocClass();
		b->load(*this);
	}


//####################
//class CMsgIStream
//####################

	void CMsgOStream::serial(uint8 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<uint8, UInt8Type>(b));
	}

	void CMsgOStream::serial(sint8 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<sint8, CharType>(b));
	}

	void CMsgOStream::serial(uint16 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<uint16, UInt16Type>(b));
	}

	void CMsgOStream::serial(sint16 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<sint16, ShortIntegerType>(b));
	}

	void CMsgOStream::serial(uint32 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<uint32, UInt32Type>(b));
	}

	void CMsgOStream::serial(sint32 &b) throw(NLMISC::EStream)
	{		
		_List.push(getValue<sint32, IntegerType>(b));
	}

	void CMsgOStream::serial(uint64 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<uint64, UInt64Type>(b));
	}
	void CMsgOStream::serial(sint64 &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<sint64, UInt64Type>(b));
	}
	void CMsgOStream::serial(float &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<float, DigitalType>(b));
	}
	void CMsgOStream::serial(double &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<double, DDigitalType>(b));
	}

	void CMsgOStream::serial(bool &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<uint8, UInt8Type>(b ? true : false));
	}

#ifndef NL_OS_CYGWIN
	void CMsgOStream::serial(char &b) throw(NLMISC::EStream)
	{
		_List.push(getValue<sint8, CharType>(b));
	}
#endif

	void CMsgOStream::serial(std::string &b) throw(NLMISC::EStream)
	{
		CStringType *s= new CStringType(CStringVarName(b.c_str()));
		_List.push(s);
	}

	void CMsgOStream::serial(ucstring &b) throw(NLMISC::EStream)
	{
		throw NLMISC::EReadError((const char *)_List.getType());
	}

	void CMsgOStream::serial(IObjectIA &b) throw(NLMISC::EStream)
	{
		((NLAIC::CIdentType &)b.getType()).serial(*this);
		b.serial(*this);
	}

	void CMsgOStream::serial(IObjectIA* &b) throw(NLMISC::EStream)
	{
		((NLAIC::CIdentType &)b->getType()).serial(*this);
		b->serial(*this);
	}
}
