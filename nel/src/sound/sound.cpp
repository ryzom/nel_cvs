/** \file sound.cpp
 * CSound: a sound buffer and its static properties
 *
 * $Id: sound.cpp,v 1.23 2002/11/04 15:40:44 boucher Exp $
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

#include "stdsound.h"

#include "sound.h"
#include "nel/misc/path.h"
#include "sound_bank.h"

#include "simple_sound.h"
#include "complex_sound.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


// Sound driver
//ISoundDriver	*CSound::_SoundDriver = NULL;


/* To add a static property (or initial value), do the following:
 * 1. Add the CSound member and its accessors (getXxxx() and setProperties()) (in sound.h),
 *    and initialize it in the constructor (in sound.cpp)
 * 2. Increment CSound::_CurrentVersion (in sound.cpp)
 * 3. Serialize the member in CSound::serial(), handling old versions (in sound.cpp)
 * 4. Add it in CSourceUser::setSound() (in source_user.cpp)
 * 5. Add it into the NeL Sources Sound Builder GUI.
 */

// Software current version (used by backward-compatibility support)
//uint			CSound::CurrentVersion = 4;

// Input file version
//uint			CSound::FileVersion = 0;

// Allow to load sound files when corresponding wave file is not present ?
//bool			CSound::_AllowMissingWave = true;



CSound *CSound::createSound(const std::string &filename, NLGEORGES::UFormElm& formRoot)
{
	CSound *ret = NULL;
	string	soundType;

	NLGEORGES::UFormElm *psoundType;

	formRoot.getNodeByName(&psoundType, ".SoundType");
	if (psoundType != NULL)
	{
		std::string dfnName;
		psoundType->getDfnName(dfnName);

		if (dfnName == "simple_sound.dfn")
		{
			ret = new CSimpleSound();
			ret->importForm(filename, formRoot);
		}
		else if (dfnName == "complex_sound.dfn")
		{
			ret = new CComplexSound();
			ret->importForm(filename, formRoot);
		}
		else if (dfnName == "background_sound.dfn")
		{
			ret = new CBackgroundSound();
			ret->importForm(filename, formRoot);
		}
		else
		{
			nlassertex(false, ("SoundType unsuported : %s", dfnName.c_str()));
		}
			
	}
/*	formRoot.getValueByName(soundType, ".SoundType.Dfn");

	if (soundType == "SimpleSound")
	{
		ret = new CSimpleSound();
		ret->importForm(filename, formRoot);
	}
	else if (soundType == "ComplexSound")
	{
		ret = new CComplexSound();
		ret->importForm(filename, formRoot);
	}
	else if (soundType == "BackgroundSound")
	{
*//*		ret = new CBackgroundSound();
		ret->importForm(filename, formRoot);
*//*	}
*/
//	nlassert(ret != NULL);

	return ret;
	
}



/*
 * Constructor
 */
CSound::CSound() : 
	_Gain(1.0f), 
	_Pitch(1.0f),
	_MaxDist(1000000.0f),
	_Priority(MidPri), 
	_Looping(false),
	_ConeInnerAngle(6.283185f), 
	_ConeOuterAngle(6.283185f), 
	_ConeOuterGain( 1.0f )
{
}

CSound::~CSound()
{}


/*
 * Serialize
 */
/*
void				CSound::serial( NLMISC::IStream& s )
{
	// If you change this, increment the version number in serialFileHeader()
	
	// Static properties
	s.serial( _Name );
	s.serial( _Filename );
	s.serial( _Gain );
	if ( (! s.isReading()) || (FileVersion>=2) )
	{
		s.serial( _Pitch );
	}
	if ( (!s.isReading()) || (FileVersion>=3) )
	{
		s.serial( _Looping );
	}
	if ( (!s.isReading()) || (FileVersion>=4) )
	{
		uint8 iprio = (uint8)(_Priority+1) * 16; // allowing future other priorities
		s.serial( iprio );
		_Priority = (TSoundPriority)((iprio/16)-1);
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
			if ( _Filename != "" )
			{
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
			else
			{
				if ( CSound::_AllowMissingWave )
				{
					nlwarning( "AM: Sound %s has no filename specified", _Name.c_str() );
				}
				else
				{
					throw ESoundFileNotFound( "<NoFilename>" );
				}
			}
		}
	}
	else
	{
		// Prevent from writing a blank filename (disabled)
		//if ( _Filename == "" )
		//{
		//	string reason = "Invalid sound filename for " + _Name;
		//	throw EStream( reason );
		//}
	}
}
*/

/*
 * Load the buffer (automatically done by serial())
 */
/*
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
*/

/*
 * Serialize file header
 */
/*
void				CSound::serialFileHeader( NLMISC::IStream& s, uint32& nb )
{
	s.serialCheck( (uint32)'SSN' ); // NeL Source Sounds
	CSound::FileVersion = s.serialVersion( CSound::CurrentVersion );
	if ( CSound::FileVersion == 0 ) // warning: not multithread-compliant
	{
		// Not supporting version 0 anymore
		throw EOlderStream(s);
	}

	s.serial( nb );
}
*/

/*
 * Load several sounds and return the number of sound loaded.
 * If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
 */
/*
uint32				CSound::load( TSoundMap& container, NLMISC::IStream& s, std::vector<std::string> *notfoundfiles )
{
	if ( s.isReading() )
	{
		// Read header
		uint32 nb, i, notfound = 0;
		serialFileHeader( s, nb );
		if ( notfoundfiles != NULL )
		{
			notfoundfiles->clear();
		}

		// Read sounds
		for ( i=0; i!=nb; i++ )
		{
			CSound *sound;
			try
			{
				sound = new CSound();
				s.serial( *sound );
				if ( ! container.insert( make_pair( sound->getName().c_str(), sound ) ).second )
				{
					nlwarning( "AM: Duplicate sound found while loading NSS sound bank" );
				}
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

		// Reset file version
		CSound::FileVersion = CSound::CurrentVersion; // warning: not multithread-compliant : do not serialize in different threads !
		return nb - notfound;
	}
	else
	{
		nlstop;
		return 0;
	}
}
*/

/*
 * Set properties. Returns false if one or more values are invalid (EDIT)
 */
/*
bool				CSound::setProperties( const std::string& name, const std::string& filename,
										   float gain, float pitch, TSoundPriority priority, bool looping, bool detail,
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
		_Gain = gain; _Pitch = pitch; _Priority = priority; _Looping = looping;
		_Detailed = detail; _MinDist = mindist; _MaxDist = maxdist;
		_ConeInnerAngle = innerangle; _ConeOuterAngle = outerangle; _ConeOuterGain = outergain;
		return true;
	}
}
*/
		  
/*
 * Save (output stream only) (EDIT)
 */
/*
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
*/

/**
 * 	Load the sound parameters from georges' form
 */
void				CSound::importForm(const std::string& filename, NLGEORGES::UFormElm& root)
{
/*	// Name
	_Filename = filename;
	_Name = CFile::getFilenameWithoutExtension(filename);	

	// Buffername
	root.getValueByName(_Buffername, ".Filename");
	_Buffername = CFile::getFilenameWithoutExtension(_Buffername);

	// contain % so it need a context to play
	if (_Buffername.find ("%") != string::npos)
	{
		_NeedContext = true;
	}
*/
	// Name
	_Name = CFile::getFilenameWithoutExtension(filename);	

	// InternalConeAngle
	uint32 inner;
	root.getValueByName(inner, ".InternalConeAngle");
	if (inner > 360)
	{
		inner = 360;
	}
	_ConeInnerAngle = (float) (Pi * inner / 180.0f);  // convert to radians

	// ExternalConeAngle
	uint32 outer;
	root.getValueByName(outer, ".ExternalConeAngle");
	if (outer > 360)
	{
		outer = 360;
	}
	_ConeOuterAngle= (float) (Pi * outer / 180.0f);  // convert to radians

	// Loop
	root.getValueByName(_Looping, ".Loop");

	// Gain
	sint32 gain;
	root.getValueByName(gain, ".Gain");
	if (gain > 0)
	{
		gain = 0;
	}
	if (gain < -100)
	{
		gain = -100;
	}
	_Gain = (float) pow(10.0, gain / 20.0); // convert dB to linear gain

	// External gain
	root.getValueByName(gain, ".ExternalGain");
	if (gain > 0)
	{
		gain = 0;
	}
	if (gain < -100)
	{
		gain = -100;
	}
	_ConeOuterGain = (float) pow(10.0, gain / 20.0); // convert dB to linear gain

	// Direction
	float x, y, z;

	root.getValueByName(x, ".Direction.X");
	root.getValueByName(y, ".Direction.Y");
	root.getValueByName(z, ".Direction.Z");
	_Direction = CVector(x, y, z);


	// Pitch
	sint32 trans;
	root.getValueByName(trans, ".Transpose");
	_Pitch =  (float) pow(Sqrt12_2, trans); // convert semi-tones to playback speed

	// Priority
	uint32 prio = 0;
 	root.getValueByName(prio, ".Priority");
	switch (prio)
	{
	case 0: _Priority = LowPri; break;
	default:
	case 1: _Priority = MidPri; break;
	case 2: _Priority = HighPri; break;
	case 3: _Priority = HighestPri;	break;
	}
}



} // NLSOUND
