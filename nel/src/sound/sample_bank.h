/** \file sample_bank.h
 * CSampleBank: a set of sound samples
 *
 * $Id: sample_bank.h,v 1.1 2002/06/04 10:02:52 hanappe Exp $
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
#include "nel/misc/stream.h"
#include "nel/sound/u_source.h"
#include <string>
#include <hash_map>

namespace NLSOUND {


class ISoundDriver;
class IBuffer;


// Comparision for const char*
struct eqname
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


/// Sound names hash map
typedef std::hash_map<const char*, IBuffer*, std::hash<const char*>, eqname> TSoundTable;


/**
 * A set of sound samples.
 * \author Peter Hanappe
 * \author Nevrax France
 * \date 2001
 */
class CSampleBank
{
public:

	/// Constructor
	CSampleBank(const std::string& path, ISoundDriver *sd);

	/// Destructor
	virtual ~CSampleBank();

	/** Load all the sound buffers.
	 *
	 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
	 */
	void				load();

	/// Unload all the sound buffers in this bank.
	void				unload();

	/// Returns true if the sounds in this bank have been loaded.
	bool				isLoaded();

	/// Return a sound buffer corresponding to a name.
	IBuffer				*getBuffer(const char* name);

	/// Return the number of buffers in this bank.
	uint				countBuffers();

	/// Return the size of this bank in bytes.
	uint				getSize();

	/// Return the filename
	const std::string&	getPath() const						{ return _Path; }

	/// Return the name (must be unique)
	const std::string&	getName() const						{ return _Name; }



private:

	// Sound driver
	ISoundDriver		*_SoundDriver;

	// Buffer
	TSoundTable			_Buffers;

	// Sound bank name and path 
	std::string			_Path;
	std::string			_Name;

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

