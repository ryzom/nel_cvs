/** \file bit_mem_stream.h
 * Bit-oriented memory stream
 *
 * $Id: bit_mem_stream.h,v 1.1 2001/10/05 16:23:21 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_BIT_MEM_STREAM_H
#define NL_BIT_MEM_STREAM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/mem_stream.h"


namespace NLMISC {


/**
 * Bit-oriented memory stream
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBitMemStream : public CMemStream
{
public:

	/// Constructor
	CBitMemStream( bool inputStream=false, uint32 defaultcapacity=32 );

	/// Copy constructor
	CBitMemStream( const CBitMemStream& other );

	/// Transforms the message from input to output or from output to input
	virtual void	invert();

	/// Serialize one bit
	//virtual void	serialBit( bool& bit );

	/// Serialize only the nbits lower bits of value
	virtual void	serial( uint32& value, uint nbits );

	// Serialize a bit vector
	//virtual void	serial( const std::vector<bool>& bitvect );

	// TODO: other common types

protected:

	uint			_FreeBits; // from 8 downto 1
};


} // NLMISC


#endif // NL_BIT_MEM_STREAM_H

/* End of bit_mem_stream.h */
