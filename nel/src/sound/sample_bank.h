/** \file sample_bank.h
 * CSampleBank: a set of samples
 *
 * $Id: sample_bank.h,v 1.2 2002/06/11 09:40:54 hanappe Exp $
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

#ifndef NL_SAMPLE_BANK_H
#define NL_SAMPLE_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/sound/u_source.h"
#include <string>
#include <hash_map>

namespace NLSOUND {


class ISoundDriver;
class IBuffer;
class CSampleBank;


// Comparision for const char*
struct eqname
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


/// Sound names hash map
typedef std::hash_map<const char*, IBuffer*, std::hash<const char*>, eqname> TSampleTable;


/**
 * A set of samples.
 * \author Peter Hanappe
 * \author Nevrax France
 * \date 2001
 */
class CSampleBank
{
public:

	/// Return the name corresponding to a name. The sample is searched
	// in all the loaded sample banks.
	static IBuffer*		get(const char* name);

	/// Constructor
	CSampleBank(const std::string& path, ISoundDriver *sd);

	/// Destructor
	virtual ~CSampleBank();

	/** Load all the samples.
	 *
	 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
	 */
	void				load();

	/// Unload all the samples in this bank.
	void				unload();

	/// Returns true if the samples in this bank have been loaded.
	bool				isLoaded();

	/// Return a samples corresponding to a name.
	IBuffer				*getSample(const char* name);

	/// Return the number of samples in this bank.
	uint				countSamples();

	/// Return the size of this bank in bytes.
	uint				getSize();

	/// Return the filename
	const std::string&	getPath() const						{ return _Path; }

	/// Return the name (must be unique)
	const std::string&	getName() const						{ return _Name; }



private:

	// The map off all loaded sample banks
	static std::set<CSampleBank*>		_Banks;

	// Sound driver
	ISoundDriver		*_SoundDriver;

	// Hashtable with samples
	TSampleTable		_Samples;

	// Sample bank name and path 
	std::string			_Path;
	std::string			_Name;

	// Did we load the buffers.
	bool				_Loaded;

};


/**
 * ESoundFileNotFound
 */

class ESampleBankNotFound : public NLMISC::Exception
{
public:
	ESampleBankNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sample bank not found: ")+filename).c_str() ) {}
};

} // NLSOUND


#endif // NL_SAMPLE_BANK_H

/* End of sound.h */

