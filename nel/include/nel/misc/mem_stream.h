/** \file mem_stream.h
 * CMemStream class
 *
 * $Id: mem_stream.h,v 1.1 2000/12/05 10:03:57 lecroart Exp $
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
class EMemStream : public NLMISC::EStream
{};

/*class EMemStreamTypeNbr : public EMemStream
{
	/// Returns the exception name
	virtual const char	*what() const throw() {return "Bad message type code";}
};

class EMemStreamTypeStr : public EMemStream
{
	/// Returns the exception name
	virtual const char	*what() const throw() {return "Bad message type name";}
};*/

/// This exception is raised when someone tries to serialize in more than there is.
class EStreamOverflow : public EMemStream
{
	/// Returns the exception name
	virtual const char	*what() const throw() {return "Stream Overflow Error";}
};


/// Type of message as a number
typedef sint16 TTypeNum;


/// Vector of uint8
typedef std::vector<uint8> CVector8;

/// Iterator on CVector8
typedef CVector8::iterator It8;


/**
 * MemStream memory stream. Can be serialized to/from (see SerialBuffer()). Can be sent or received
 * over a network, using a CSocket or preferably a CMsgSocket object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMemStream : public NLMISC::IStream
{
public:

	/// Initialization constructor
	CMemStream( std::string type="", bool inputStream=false, uint32 defaultcapacity=0 );

	/// Copy constructor
	CMemStream( const CMemStream& other );

	/// Assignment operator
	CMemStream&		operator=( const CMemStream& other );

	/// Method inherited from IStream
	virtual void	serialBuffer(uint8 *buf, uint len) throw(EStreamOverflow);

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit) throw(EStreamOverflow);

	/// Clears the message
	void			clear();

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

	/// Returns a pointer to the message buffer (read only)
	const uint8		*buffer() const
	{
		return &(*_Buffer.begin());
	}

	/// Returns if the message type is a number (and not a string)
	bool			typeIsNumber() const
	{
		return _TypeIsNumber;
	}

	/// Returns message type code. Valid if typeIsNumber()
	uint16			typeAsNumber() const
	{
		return _MsgType;
	}

	/// Returns message name. Valid if !typeIsNumber()
	std::string		typeAsString() const
	{
		return _MsgName;
	}

	/// Fills the message buffer, for reading
	void			fill( const uint8 *srcbuf, uint32 len );

	/// Sets the message type as a number (in range 0..32767)
	void			setType( const TTypeNum msgtype )
	{
		_MsgType = msgtype;
		_TypeIsNumber = true;
	}

	/// Sets the message type as a string
	void			setType( const std::string& msgname )
	{
		_MsgName = msgname;
		_TypeIsNumber = false;
	}

	/** EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
	 * you MUST fill the number of bytes you specify in "msgsize").
	 * This method prevents from doing one useless buffer copy, using fill().
	 */
	uint8			*bufferToFill( uint32 msgsize );

	/// Returns the maximum message total size (header + payload)
	static uint32	maxLength()
	{
		return CMemStream::_MaxLength;
	}

	/// Returns the maximum message header size
	static uint32	maxHeaderLength()
	{
		return CMemStream::_MaxHeaderLength;
	}

	/// Returns the maximum message payload size
	static uint32	maxPayloadLength()
	{
		return maxLength() - maxHeaderLength();
	}

	/// Returns the maximum message name length
	static uint32	maxNameLength()
	{
		return maxHeaderLength() - sizeof(sint16) - sizeof(uint32);
	}

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

private:

	static uint32	_MaxLength;
	static uint32	_MaxHeaderLength;

	CVector8		_Buffer;
	It8				_BufPos;
	
	TTypeNum		_MsgType;
	std::string		_MsgName;
	bool			_TypeIsNumber;

};

}

#endif // NL_MEM_STREAM_H

/* End of mem_stream.h */
