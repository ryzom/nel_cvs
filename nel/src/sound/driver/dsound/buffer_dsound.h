/** \file buffer_dsound.h
 * DSound buffer
 *
 * $Id: buffer_dsound.h,v 1.3 2002/11/04 15:40:44 boucher Exp $
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

#ifndef NL_BUFFER_DSOUND_H
#define NL_BUFFER_DSOUND_H


#include "sound/driver/buffer.h"


namespace NLSOUND {


/**
 *  Buffer for the DSound implementation of the audio driver.
 *
 * A buffer represents a sound file loaded in RAM. 
 *
 * \author Peter Hanappe, Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CBufferDSound : public IBuffer
{
public:

	/// Constructor
	CBufferDSound();
	/// Destructor
	virtual	~CBufferDSound();

	virtual void			presetName(const std::string &bufferName);

	/// Set the sample format. Example: freq=44100
	virtual void			setFormat( TSampleFormat format, uint freq );

    /// Set the buffer size and fill the buffer. Return true if ok. Call setFormat() first.
	virtual bool			fillBuffer( void *src, uint32 bufsize );

	/// Return the size of the buffer, in bytes
    virtual uint32			getSize() const				{ return _Size; }

	/// Return the duration (in ms) of the sample in the buffer
    virtual float			getDuration() const; 

    /// Return true if the buffer is stereo, false if mono
    virtual bool			isStereo() const			{ return (_Format == Stereo8) || (_Format == Stereo16); }

    /// Return the format and frequency
    virtual void			getFormat( TSampleFormat& format, uint& freq ) const   { format = _Format; freq = _Freq; }


	/** Return true if the buffer is able to be fill part by part, false if it must be filled in one call
	 * (OpenAL 1.0 -> false)
	 */
	virtual bool			isFillMoreSupported() const	{ return false; }

    /// Force the buffer size without filling data (if isFillMoreSupported() only)
	//virtual void		setSize( uint32 size )					{ throw ESoundDriverNotSupp(); }

    /** Fill the buffer partially (if isFillMoreSupported() only),
	 * beginning at the pos changed by a previous call to fillMore().
	 * If the pos+srcsize exceeds the buffer size, the exceeding data is put at the beginning
	 * of the buffer. srcsize must be smaller than the buffer size.
	 */
	//virtual bool		fillMore( void *src, uint32 srcsize )	{ throw ESoundDriverNotSupp(); }

	
    /// Load a sound file in the buffer. Throws an exception is an error occurs.
	virtual bool			loadWavFile(const char* filename);

	/// Return a pointer to the sample data
    virtual uint8*			getData() const				{ return _Data; }

	/// Return the name of the buffer
	virtual const std::string&	getName()					{ return _Name; }

private:

	std::string		_Name;

    uint8*			_Data;

    uint32			_Size; 

    TSampleFormat	_Format;

    uint			_Freq;


#if USE_LOCDEFER
    LPDIRECTSOUNDBUFFER _SecondaryBuffer;
    LPDIRECTSOUND3DBUFFER _3DBuffer;
#endif

};


} // NLSOUND


#endif // NL_BUFFER_DSOUND_H

