/** \file buffer_al.h
 * OpenAL buffer
 *
 * $Id: buffer_al.h,v 1.6 2002/11/04 15:40:44 boucher Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_BUFFER_AL_H
#define NL_BUFFER_AL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "sound/driver/buffer.h"

#include "AL/al.h"

namespace NLSOUND {


/**
 * OpenAL buffer
 *
 * A buffer can be filled with data. An OpenAL buffer cannot be streamed in, i.e.
 * isFillMoreSupported() returns false (instead, streaming would be implemented
 * by buffer queueing).
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CBufferAL : public IBuffer
{
public:

	/// Constructor
	CBufferAL( ALuint buffername=0 );
	/// Destructor
	virtual				~CBufferAL();

	/// Set the sample format. Example: freq=44100
	virtual void		setFormat( TSampleFormat format, uint freq );
	/// Set the buffer size and fill the buffer. Return true if ok. Call setFormat() first.
	virtual bool		fillBuffer( void *src, uint32 bufsize );

	/// Return the size of the buffer, in bytes
	virtual uint32		getSize() const;
	/// Return the duration (in ms) of the sample in the buffer
	virtual float		getDuration() const;
	/// Return true if the buffer is stereo, false if mono
	virtual bool		isStereo() const;
	/// Return the format and frequency
	virtual void		getFormat( TSampleFormat& format, uint& freq ) const;


	/** Return true if the buffer is able to be fill part by part, false if it must be filled in one call
	 * (OpenAL 1.0 -> false)
	 */
	virtual bool		isFillMoreSupported() const				{ return false; }
	/// Force the buffer size without filling data (if isFillMoreSupported() only)
	virtual void		setSize( uint32 size )					{ throw ESoundDriverNotSupp(); }
	/** Fill the buffer partially (if isFillMoreSupported() only),
	 * beginning at the pos changed by a previous call to fillMore().
	 * If the pos+srcsize exceeds the buffer size, the exceeding data is put at the beginning
	 * of the buffer. srcsize must be smaller than the buffer size.
	 */
	virtual bool		fillMore( void *src, uint32 srcsize )	{ throw ESoundDriverNotSupp(); }

	
	/// Return the buffer name (as an int)
	ALuint				bufferName()							{ return _BufferName; }
	
	/// Return the name of the buffer (as a string)
	virtual const std::string&	getName()							{ return _Name; }

	/// Set the name of the buffer
	virtual void		setName(std::string& name)				{ _Name = name; }

	virtual void		presetName(const std::string &bufferName);

private:

	// Buffer name
	ALuint				_BufferName;

	// Buffer name as string
	std::string			_Name;

	// Sample format
	ALenum				_SampleFormat;

	// Frequency
	ALuint				_Frequency;
};


} // NLSOUND


#endif // NL_BUFFER_AL_H

/* End of buffer_al.h */
