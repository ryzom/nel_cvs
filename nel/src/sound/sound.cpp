/** \file sound.cpp
 * CSound: a sound buffer and its static properties
 *
 * $Id: sound.cpp,v 1.3 2001/07/18 17:15:09 cado Exp $
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

#include "sound.h"
#include "driver/sound_driver.h"
#include "driver/buffer.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


// Sound driver
ISoundDriver *CSound::_SoundDriver = NULL;


/*
 * Constructor
 */
CSound::CSound() : _Buffer(NULL), _Gain(1.0), _Detailed(false),
	_MinDist(1.0f), _MaxDist(1000000.0f),
	_ConeInnerAngle(6.283185f), _ConeOuterAngle(6.283185f), _ConeOuterGain( 1.0f )
{
}


/*
 * Destructor
 */
CSound::~CSound()
{
	if ( _Buffer != NULL )
	{
		delete _Buffer;
	}
}


/*
 * Return the length of the sound in ms
 */
uint32				CSound::getDuration() const
{
	if ( _Buffer == NULL )
	{
		return 0;
	}
	else
	{
		return (uint32)(_Buffer->getDuration());
	}
}


/*
 * Serialize
 */
void				CSound::serial( NLMISC::IStream& s )
{
	// If you change this, increment the version number in serialFileHeader()
	
	// Static properties
	s.serial( _Gain );
	s.serial( _Detailed );
	if ( _Detailed )
	{
		s.serial( _MinDist );
		s.serial( _MaxDist );
		s.serial( _ConeInnerAngle );
		s.serial( _ConeOuterAngle );
		s.serial( _ConeOuterGain );
	}
	s.serial( _Filename );
	
	if ( s.isReading() )
	{
		// Load file (input only)
		nlassert( _SoundDriver != NULL );
		nlassert ( _Filename != "" );
		_Buffer = _SoundDriver->createBuffer();
		if ( ! _SoundDriver->loadWavFile( _Buffer, const_cast<char*>(_Filename.c_str()) ) )
		{
			throw ESoundFileNotFound( _Filename );
		}
	}
	else
	{
		// Prevent from writing a blank filename
		if ( _Filename == "" )
		{
			throw EStream( "AM: Invalid sound filename" );
		}
	}
}


/*
 * Serialize file header
 */
void				CSound::serialFileHeader( NLMISC::IStream& s, uint32& nb )
{
	s.serialCheck( (uint32)'SSN' ); // NeL Source Sounds
	s.serialVersion( 0 );
	s.serial( nb );
}


/*
 * Load several sounds and return the number of sound loaded
 */
uint32				CSound::load( std::vector<CSound*>& container, NLMISC::IStream& s )
{
	if ( s.isReading() )
	{
		uint32 nb, i, notfound=0;
		serialFileHeader( s, nb );
		for ( i=0; i!=nb; i++ )
		{
			CSound *sound;
			try
			{
				sound = new CSound();
				s.serial( *sound );
				container.push_back( sound );
			}
			catch ( ESoundFileNotFound& e )
			{
				delete sound;
				notfound++;
				nlwarning( "AM: %s", e.what() );
			}
		}
		return nb - notfound;
	}
	else
	{
		nlstop;
		return 0;
	}
}


/*
 * Set properties (EDIT)
 */
void				CSound::setProperties( const std::string& filename, float gain, bool detail,
										   float mindist, float maxdist,
										   float innerangle, float outerangle, float outergain )
{
	_Filename = filename; _Gain = gain; _Detailed = detail; _MinDist = mindist; _MaxDist = maxdist;
	_ConeInnerAngle = innerangle; _ConeOuterAngle = outerangle; _ConeOuterGain = outergain;
}

		  
/*
 * Save (output stream only) (EDIT)
 */
void				CSound::save( const std::vector<CSound*>& container, NLMISC::IStream& s )
{
	nlassert( ! s.isReading() );

	uint32 nb=container.size(), i;
	serialFileHeader( s, nb );
	for ( i=0; i!=nb; i++ )
	{
		s.serial( const_cast<CSound&>(*container[i]) );
	}
}


} // NLSOUND
