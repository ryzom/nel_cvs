/* message.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: message.h,v 1.3 2000/09/19 09:14:03 cado Exp $
 *
 * Interface of CMessage
 */

#ifndef NL_MESSAGE_H
#define NL_MESSAGE_H

#include "nel/misc/stream.h"
#include <vector>

namespace NLNET
{

/// This exception is raised when someone tries to serialize in more than there is.
class EStreamOverflow : public NLMISC::EStream
{
	/// Returns the exception name
	virtual const char	*what() const throw() {return "Stream Overflow Error";}

};


/// Iterator on vector<uint8>
typedef std::vector<uint8>::iterator it8;


/**
 * Message memory stream. Can be serialized to/from (see SerialBuffer()). Can be sent or received
 * over a network, using a CSocket object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMessage : public NLMISC::IStream
{
public:

	/// Initialization constructor
	CMessage( bool inputStream=false, uint32 defaultcapacity=32 );

	/// Method inherited from IStream
	virtual void	serialBuffer(uint8 *buf, uint len) throw(EStreamOverflow);

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit) throw(EStreamOverflow);

	/// Clears the message
	void			clear();

	/** Returns the length of the message, in bytes.
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
		return _Buffer.begin();
	}

	/// Returns message type code
	uint16			msgType() const
	{
		return _MsgType;
	}

	/// Returns message name
	std::string		msgName() const
	{
		return _MsgName;
	}

	/// Fills the message buffer, for reading
	void			fill( const uint8 *srcbuf, uint32 len );

	/** Sets the message header values
	 * @param msgtype Message type code, in range 0..32767
	 * @param msgname Message name
	 */
	void			setHeader( sint16 msgtype, const std::string& msgname );

	/** EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
	 * you MUST fill the number of bytes you specify in "msgsize").
	 * This method prevents from doing one useless buffer copy, using fill().
	 */
	uint8			*bufferToFill( uint32 msgsize );

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

	std::vector<uint8>	_Buffer;
	it8					_BufPos;
	sint16				_MsgType;
	std::string			_MsgName;

};

}

#endif // NL_MESSAGE_H

/* End of message.h */
