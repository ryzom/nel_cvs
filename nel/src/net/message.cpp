/** \file message.cpp
 * CMessage class
 *
 * $Id: message.cpp,v 1.14 2000/12/05 11:10:29 cado Exp $
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

#include "nel/net/message.h"

namespace NLNET
{

uint32	CMessage::_MaxLength = 65536;
uint32	CMessage::_MaxHeaderLength = 64;


/*
 * Constructor
 */
CMessage::CMessage( std::string name, bool inputStream, uint32 defaultcapacity ) :
	CMemStream( inputStream, defaultcapacity ),
	_MsgType( 0 )
{
	if ( name != "" )
	{
		setType( name );
	}
}


/*
 * Copy constructor
 */
CMessage::CMessage( const CMessage& other ) :
	NLMISC::CMemStream( other.isReading() )
{
	operator=( other );
}


/*
 * Assignment
 */
CMessage& CMessage::operator=( const CMessage& other )
{
	CMemStream::operator=( other );
	_MsgType = other._MsgType;
	_MsgName = other._MsgName;
	_TypeIsNumber = other._TypeIsNumber;
	return *this;
}


}
