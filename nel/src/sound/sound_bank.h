/** \file sound_bank.h
 * CSoundBank: a set of sounds
 *
 * $Id: sound_bank.h,v 1.6.2.1 2003/04/24 14:05:44 boucher Exp $
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
#include "nel/misc/string_mapper.h"
#include "audio_mixer_user.h"
#include <string>
#include <hash_map>

namespace NLSOUND {


class CSound;
class CSimpleSound;



/**
 * A set of sounds.
 * \author Peter Hanappe
 * \authoe Boris Boucher
 * \author Nevrax France
 * \date 2001
 */
class CSoundBank
{
public:

	void registerBufferAssoc(CSimpleSound *sound, IBuffer *buffer);
	void unregisterBufferAssoc(CSimpleSound *sound, IBuffer *buffer);

	/** Called by CSampleBank when a sample(buffer) is unloaded.
	 *	Remove link from CSound to unloaded IBuffer.
	 */
	void	bufferUnloaded(const NLMISC::TStringId &bufferName);
	/** Called by CSampleBank when a sample(buffer) is loaded.
	 *	Regenerate link between CSound and IBuffer.
	 */
	void	bufferLoaded(const NLMISC::TStringId &bufferName, IBuffer *buffer);

	/// Destructor
	virtual ~CSoundBank();

	/// Load all the sounds.
	void				load();

	/// Remove all the sounds in this bank.
	void				unload();

	/// Returns true if the sounds in this bank have been loaded.
	bool				isLoaded();

	/// Return a sound corresponding to a name.
	CSound				*getSound(const NLMISC::TStringId &name);

	/// Return the names of the sounds 
	void				getNames( std::vector<NLMISC::TStringId> &names );

	/// Return the number of sounds in this bank.
	uint				countSounds();

	/// Release all the loaded sound banks.
//	static void			releaseAll();

	static CSoundBank	*instance();
	static void			release();

	void				addSound(CSound *sound);
	void				removeSound(const NLMISC::TStringId &name);


private:
	/// Constructor
	CSoundBank() : _Loaded(false) {};


	/// CSoundBank singleton instance.
	static CSoundBank		*_Instance;

	typedef std::hash_set<class CSimpleSound*, THashPtr<CSimpleSound*> >	TSimpleSoundContainer;
//	typedef std::hash_map<std::string, TSimpleSoundContainer >				TBufferAssocContainer;
	typedef std::hash_map<NLMISC::TStringId, TSimpleSoundContainer >		TBufferAssocContainer;
	/// Sound names hash map
//	typedef std::hash_map<std::string, CSound*>								TSoundTable;
	typedef std::hash_map<NLMISC::TStringId, CSound*>						TSoundTable;

	/// Assoc from buffer to sound. Used for sound unloading.
	TBufferAssocContainer		_BufferAssoc;

	// Buffer
	TSoundTable					_Sounds;

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

