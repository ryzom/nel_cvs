/** \file sound_bank.h
 * CSoundBank: a set of sounds
 *
 * $Id: sound_bank.h,v 1.1 2002/06/11 09:41:56 hanappe Exp $
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

#ifndef NL_SOUND_BANK_H
#define NL_SOUND_BANK_H

#include "nel/misc/types_nl.h"
#include <string>
#include <hash_map>

namespace NLSOUND {


class CSound;

// Comparision for const char*
struct eqsoundname
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


/// Sound names hash map
typedef std::hash_map<const char*, CSound*, std::hash<const char*>, eqsoundname> TSoundTable;


/**
 * A set of sounds.
 * \author Peter Hanappe
 * \author Nevrax France
 * \date 2001
 */
class CSoundBank
{
public:

	/// Return the name corresponding to a name. The sample is searched
	/// in all the loaded sound banks.
	static CSound*		get(const char* name);

	/// Return the names of the sounds. The names of all the loaded sound
	/// banks are returned.
	static void			getSoundNames( std::vector<const char *>& names );


	/// Constructor
	CSoundBank(const std::string& path) : _Loaded(false), _Path(path) {};

	/// Destructor
	virtual ~CSoundBank();

	/// Load all the sounds.
	void				load();

	/// Remove all the sounds in this bank.
	void				unload();

	/// Returns true if the sounds in this bank have been loaded.
	bool				isLoaded();

	/// Return a sound corresponding to a name.
	CSound				*getSound(const char* name);

	/// Return the names of the sounds 
	void				getNames( std::vector<const char *>& names );

	/// Return the number of sounds in this bank.
	uint				countSounds();



private:

	// The vector off all loaded sound banks
	static std::set<CSoundBank*>		_Banks;

	// Buffer
	TSoundTable			_Sounds;

	// Sound bank path 
	std::string			_Path;

	// Did we load the buffers.
	bool				_Loaded;

};


/**
 * ESoundFileNotFound
 */

class ESoundBankNotFound : public NLMISC::Exception
{
public:
	ESoundBankNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound bank not found: ")+filename).c_str() ) {}
};

} // NLSOUND


#endif // NL_SOUND_BANK_H

/* End of sound.h */

