/** \file sound.cpp
 * CSound: a sound buffer and its static properties
 *
 * $Id: sound.cpp,v 1.10 2001/09/03 14:40:45 cado Exp $
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
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


// Sound driver
ISoundDriver	*CSound::_SoundDriver = NULL;


// Support old V1 files
bool			CSound::_InputIgnorePitch = false;

// Support old V2 files
bool			CSound::_InputIgnoreLooping = false;

// Allow to load sound files when corresponding wave file is not present ?
bool			CSound::_AllowMissingWave = false;


/*
 * Constructor
 */
CSound::CSound() : _Buffer(NULL), _Gain(1.0f), _Pitch(1.0f), _Looping(false),
	_Detailed(false), _MinDist(1.0f), _MaxDist(1000000.0f),
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
	s.serial( _Name );
	s.serial( _Filename );
	s.serial( _Gain );
	if ( ! (s.isReading() && CSound::_InputIgnorePitch) )
	{
		s.serial( _Pitch );
	}
	if ( ! (s.isReading() && CSound::_InputIgnoreLooping) )
	{
		s.serial( _Looping );
	}
	s.serial( _Detailed );
	if ( _Detailed )
	{
		s.serial( _MinDist );
		s.serial( _MaxDist );
		s.serial( _ConeInnerAngle );
		s.serial( _ConeOuterAngle );
		s.serial( _ConeOuterGain );
	}
	
	if ( s.isReading() )
	{
		// Load file (input only)
		if ( _SoundDriver != NULL )
		{
			nlassert ( _Filename != "" );
			try
			{
				loadBuffer( _Filename );
			}
			catch ( Exception& e )
			{
				if ( CSound::_AllowMissingWave )
				{
					nlwarning( "AM: %s", e.what() );
				}
				else
				{
					throw ESoundFileNotFound( _Filename );
				}
			}
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
 * Load the buffer (automatically done by serial())
 */
void				CSound::loadBuffer( const std::string& filename )
{
	_Buffer = _SoundDriver->createBuffer();
	try
	{
		if ( ! _SoundDriver->loadWavFile( _Buffer, CPath::lookup( _Filename ).c_str() ) )
		{
			throw ESoundFileNotFound( _Filename );
		}
	}
	catch ( Exception& )
	{
		_Buffer = NULL;
		throw;
	}
}


/*
 * Serialize file header
 */
void				CSound::serialFileHeader( NLMISC::IStream& s, uint32& nb )
{
	s.serialCheck( (uint32)'SSN' ); // NeL Source Sounds
	uint ver = s.serialVersion( 3 );
	if ( ver < 3 )
	{
		switch ( ver )
		{
		case 1:
			// Supporting old version 1
			CSound::_InputIgnorePitch = true; // warning: not multithread-compliant : do not serialize in different threads !
			// no break
		case 2:
			// Supporting old version 2
			CSound::_InputIgnoreLooping = true; // same
			break;
		default:
			// Not supporting version 0 anymore
			throw EOlderStream();
		}
	}
	// CSound::_InputIgnorePitch is reset to false at the end of load()

	s.serial( nb );
}


/*
 * Load several sounds and return the number of sound loaded.
 * If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
 */
uint32				CSound::load( TSoundMap& container, NLMISC::IStream& s, std::vector<std::string> *notfoundfiles )
{
	if ( s.isReading() )
	{
		uint32 nb, i, notfound = 0;
		serialFileHeader( s, nb );
		if ( notfoundfiles != NULL )
		{
			notfoundfiles->clear();
		}
		for ( i=0; i!=nb; i++ )
		{
			CSound *sound;
			try
			{
				sound = new CSound();
				s.serial( *sound );
				nlassert( ! sound->getName().empty() );
				container[ sound->getName().c_str() ] = sound;
			}
			catch ( ESoundFileNotFound& e )
			{
				notfound++;
				if ( notfoundfiles != NULL )
				{
					notfoundfiles->push_back( sound->getFilename() );
				}
				delete sound;
				nlwarning( "AM: %s", e.what() );
			}
		}
		CSound::_InputIgnorePitch = false; // warning: not multithread-compliant : do not serialize in different threads !
		CSound::_InputIgnoreLooping = false;
		return nb - notfound;
	}
	else
	{
		nlstop;
		return 0;
	}
}


/*
 * Set properties. Returns false if one or more values are invalid (EDIT)
 */
bool				CSound::setProperties( const std::string& name, const std::string& filename,
										   float gain, float pitch, bool looping, bool detail,
										   float mindist, float maxdist,
										   float innerangle, float outerangle, float outergain )
{
	if ( ((_Gain < 0) && (_Gain > 1))
	  || ((_Pitch <= 0) && (_Pitch > 1 ))
	  || (_MinDist < 0)
	  || (_MaxDist < 0)
	  || ((_ConeOuterGain < 0) && (_ConeOuterGain > 1)) )
	{
		return false;
	}
	else
	{
		_Name = name; _Filename = filename;
		_Gain = gain; _Pitch = pitch; _Looping = looping;
		_Detailed = detail; _MinDist = mindist; _MaxDist = maxdist;
		_ConeInnerAngle = innerangle; _ConeOuterAngle = outerangle; _ConeOuterGain = outergain;
		return true;
	}
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
