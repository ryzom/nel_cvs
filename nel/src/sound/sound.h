/** \file sound.h
 * CSound: a sound buffer and its static properties
 *
 * $Id: sound.h,v 1.16.2.1 2003/04/24 14:05:44 boucher Exp $
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
#include "nel/misc/string_mapper.h"
#include "nel/sound/u_source.h"
#include "nel/georges/u_form_elm.h"
#include <string>
#include <hash_map>

namespace NLSOUND {


class ISoundDriver;
class IBuffer;
class CSound;


/// Sound names hash map
//typedef std::hash_map<std::string, CSound*> TSoundMap;
typedef std::hash_map<NLMISC::TStringId, CSound*> TSoundMap;

/// Sound names set (for ambiant sounds)
typedef std::set<CSound*> TSoundSet;

const double Sqrt12_2 = 1.0594630943592952645618252949463;  // 2^1/12

/**
 * A sound base class and its static properties
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CSound
{
	friend class CAudioMixerUser;
public:
	/// Factory for specialized sound.
	static CSound *createSound(const std::string &filename, NLGEORGES::UFormElm& formRoot);

	enum TSOUND_TYPE
	{
		SOUND_SIMPLE,
		SOUND_COMPLEX,
		SOUND_BACKGROUND,
		SOUND_CONTEXT
	};


	/// Constructor
	CSound();
	/// Destructor
	virtual ~CSound();

	/// Get the type of the sound.
	virtual TSOUND_TYPE getSoundType() =0;

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Return the looping state
	bool				getLooping() const				{ return _Looping; }
	/// Return the gain
	float				getGain() const					{ return _Gain; }
	/// Return the pitch
	float				getPitch() const				{ return _Pitch; }
	/// Return the initial priority
	TSoundPriority		getPriority() const				{ return _Priority; }
	/// Return true if cone is meaningful
	virtual bool		isDetailed() const = 0;
	/// Return the inner angle of the cone
	float				getConeInnerAngle() const			{ return _ConeInnerAngle; }
	/// Return the outer angle of the cone
	float				getConeOuterAngle() const			{ return _ConeOuterAngle; }
	/// Return the outer gain of the cone
	float				getConeOuterGain() const			{ return _ConeOuterGain; }
	/// Return the direction vector.
	const NLMISC::CVector &getDirectionVector()const		{ return _Direction;}
	/// Return the length of the sound in ms
	virtual uint32		getDuration() = 0;
	/// Return the name (must be unique)
	const NLMISC::TStringId&	getName() const						{ return _Name; }
	/// Return the max distance (if detailed())
	virtual float		getMaxDistance() const				{ return _MaxDist; }

	/// Set looping
	void				setLooping( bool looping ) { _Looping = looping; }

	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const =0;
	 

	virtual void		serial(NLMISC::IStream &s);

	NLMISC::TStringId	getUserVarControler() { return _UserVarControler; }

	bool				operator<( const CSound& otherSound ) const
	{
		return NLMISC::CStringMapper::unmap(_Name) < NLMISC::CStringMapper::unmap(otherSound._Name);
	}

protected:

	// Static properties
	float				_Gain;	// [0,1]
	float				_Pitch; // ]0,1]
	TSoundPriority		_Priority;
	float				_ConeInnerAngle, _ConeOuterAngle, _ConeOuterGain;
	NLMISC::CVector		_Direction;

	bool				_Looping;

	/// Clipping distance for complex or backgound sound.
	float				_MaxDist;

	// Sound name.
	NLMISC::TStringId	_Name;
	/// An optional user var controler.
	NLMISC::TStringId	_UserVarControler;

};


/**
 * ESoundFileNotFound
 */
class ESoundFileNotFound : public NLMISC::Exception
{
public:
	ESoundFileNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound file not found, or invalid file format: ")+filename).c_str() ) {}
};


} // NLSOUND


#endif // NL_SOUND_H

/* End of sound.h */

