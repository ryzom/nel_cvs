/** \file mem_stream.h
 * From memory serialization implementation of IStream using ASCII format (look at stream.h)
 *
 * $Id: mem_stream.h,v 1.12 2001/06/21 12:35:16 lecroart Exp $
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

#ifndef NL_MEM_STREAM_H
#define NL_MEM_STREAM_H

#include "nel/misc/stream.h"
#include <vector>

namespace NLMISC
{

/// Exception class for CMemStream
struct EMemStream : public NLMISC::EStream
{
	EMemStream( const std::string& str ) : EStream( str ) {}
};

/*class EMemStreamTypeNbr : public EMemStream
{
	EMemStreamTypeNbr() : EMemStream( "Bad message type code" ) {}
};

class EMemStreamTypeStr : public EMemStream
{
	EMemStreamTypeStr() : EMemStream( "Bad message type name" ) {}
};*/

/// This exception is raised when someone tries to serialize in more than there is.
struct EStreamOverflow : public EMemStream
{
	EStreamOverflow() : EMemStream( "Stream Overflow Error" ) {}
};


/// Vector of uint8
typedef std::vector<uint8> CVector8;

/// Iterator on CVector8
typedef CVector8::iterator It8;


/**
 * MemStream memory stream (see also NLNET::CMessage).
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMemStream : public NLMISC::IStream
{
public:

	/// Initialization constructor
	CMemStream( bool inputStream=false, uint32 defaultcapacity=0 );

	/// Copy constructor
	CMemStream( const CMemStream& other );

	/// Assignment operator
	CMemStream&		operator=( const CMemStream& other );

	/// Method inherited from IStream
	virtual void	serialBuffer(uint8 *buf, uint len);

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit);

	/** 
	 * Moves the stream pointer to a specified location.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return true if seek sucessfull.
	 * \see ESeekNotSupported SeekOrigin getPos
	 */
	virtual bool	seek (sint32 offset, TSeekOrigin origin) throw(EStream);

	/** 
	 * Get the location of the stream pointer.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return the new offset regarding from the origin.
	 * \see ESeekNotSupported SeekOrigin seek
	 */
	virtual sint32	getPos () throw(EStream);

	/// Clears the message
	virtual void	clear();

	/** Returns the length (size) of the message, in bytes.
	 * If isReading(), it is the number of bytes that can be read,
	 * otherwise it is the number of bytes that have been written.
	 */
	uint32			length() const
	{
		if ( isReading() )
		{
			return lengthR();
		}
		else
		{
			return lengthS();
		}
	}

	/** Returns a pointer to the message buffer (read only)
	 * Returns NULL if the buffer is empty
	 */
	const uint8		*buffer() const
	{
		if ( _Buffer.empty() )
		{
			return NULL;
		}
		else
		{
			return &(*_Buffer.begin());
		}
	}

	/// Returns the message buffer (read only)
	const CVector8&	bufferAsVector() const
	{
		return _Buffer;
	}

	/// Returns the vector for external filling
	CVector8&	bufferAsVector()
	{
		return _Buffer;
	}

	// When you fill the buffer externaly (using bufferAsVector) you have to reset the BufPos calling this method
	void resetBufPos() { _BufPos = _Buffer.begin(); }

	/// Fills the message buffer, for reading
	void			fill( const uint8 *srcbuf, uint32 len );


	/** EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
	 * you MUST fill the number of bytes you specify in "msgsize").
	 * This method prevents from doing one useless buffer copy, using fill().
	 */
	uint8			*bufferToFill( uint32 msgsize );

	/// Transforms the message from input to output or from output to input
	void			invert();

protected:

	/// Returns the serialized length (number of bytes written or read)
	uint32			lengthS() const
	{
		return _BufPos-_Buffer.begin();

	}

	/// Returns the "read" message size (number of bytes to read)
	uint32			lengthR() const
	{
		return _Buffer.size();
	}

	CVector8		_Buffer;
	It8				_BufPos;
	
};

}

#endif // NL_MEM_STREAM_H

/* End of mem_stream.h */
