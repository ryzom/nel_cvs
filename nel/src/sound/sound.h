/** \file sound.h
 * CSound: a sound buffer and its static properties
 *
 * $Id: sound.h,v 1.7 2001/08/28 16:59:34 cado Exp $
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

#ifndef NL_SOUND_H
#define NL_SOUND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include <string>
#include <hash_map>

namespace NLSOUND {


class ISoundDriver;
class IBuffer;
class CSound;


// Comparision for const char*
struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


/// Sound names hash map
typedef std::hash_map<const char*, CSound*, std::hash<const char*>, eqstr> TSoundMap;


/**
 * A sound buffer and its static properties
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CSound
{
public:

	/// Constructor
	CSound();
	/// Destructor
	virtual ~CSound();
	/// Init with sound driver
	static void			init( ISoundDriver *sd )		{ _SoundDriver = sd; }
	/** Allow to load sound files when corresponding wave file is missing
	 * (default: false, i.e. an input serial or a load throws an exception ESoundFileNotFound)
	 */
	static void			allowMissingWave( bool b )		{ _AllowMissingWave = b; }
	/// Serialize
	void				serial( NLMISC::IStream& s );
	/** Load the buffer (automatically done by serial()).
	 *
	 * The filename is searched in the global path (see CPath).
	 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
	 */
	void				loadBuffer( const std::string& filename );
	/// Serialize file header
	static void			serialFileHeader( NLMISC::IStream& s, uint32& nb );
	/** Load several sounds and return the number of sounds loaded
	 * If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
	 */
	static uint32		load( TSoundMap& container, NLMISC::IStream& s, std::vector<std::string> *notfoundfiles=NULL );

	/// Return the buffer
	IBuffer				*getBuffer()					{ return _Buffer; }
	/// Return the gain
	float				getGain() const					{ return _Gain; }
	/// Return the pitch
	float				getPitch() const				{ return _Pitch; }
	/// Return the looping state
	bool				getLooping() const				{ return _Looping; }
	/// Return true if distances and cone are meaningful
	bool				isDetailed() const				{ return _Detailed; }
	/// Return the min distance (if detailed())
	float				getMinDistance() const				{ return _MinDist; }
	/// Return the max distance (if detailed())
	float				getMaxDistance() const				{ return _MaxDist; }
	/// Return the inner angle of the cone
	float				getConeInnerAngle() const			{ return _ConeInnerAngle; }
	/// Return the outer angle of the cone
	float				getConeOuterAngle() const			{ return _ConeOuterAngle; }
	/// Return the outer gain of the cone
	float				getConeOuterGain() const			{ return _ConeOuterGain; }
	/// Return the length of the sound in ms
	uint32				getDuration() const;
	/// Return the filename
	const std::string&	getFilename() const					{ return _Filename; }
	/// Return the name (must be unique)
	const std::string&	getName() const						{ return _Name; }

	/// Set properties. Returns false if one or more values are invalid (EDIT)
	bool				setProperties( const std::string& name, const std::string& filename,
									   float gain, float pitch, bool looping, bool detail,
									   float mindist=1.0f, float maxdist=1000000.0f,
									   float innerangle=6.283185f, float outerangle=6.283185f, // 360°
									   float outergain=1.0f );
	/// Save (output stream only) (EDIT)
	static void			save( const std::vector<CSound*>& container, NLMISC::IStream& s );


private:

	// Sound driver
	static ISoundDriver *_SoundDriver;

	// Support old V1 files
	static bool			_InputIgnorePitch;

	// Support old V2 files
	static bool			_InputIgnoreLooping;

	// Allow to load sound files when corresponding wave file is missing ?
	static bool			_AllowMissingWave;

	// Buffer
	IBuffer				*_Buffer;

	// Static properties
	float				_Gain;	// [0,1]
	float				_Pitch; // ]0,1]
	bool				_Looping;
	bool				_Detailed;
	float				_MinDist, _MaxDist;
	float				_ConeInnerAngle, _ConeOuterAngle, _ConeOuterGain;

	// Sound name and filename (required for output (EDIT))
	std::string			_Filename;
	std::string			_Name;

};


/**
 * ESoundFileNotFound
 */
class ESoundFileNotFound : public NLMISC::Exception
{
public:
	ESoundFileNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound file not found: ")+filename).c_str() ) {}
};


} // NLSOUND


#endif // NL_SOUND_H

/* End of sound.h */
