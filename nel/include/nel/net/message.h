/* message.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: message.h,v 1.2 2000/09/18 17:13:15 cado Exp $
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
