/** \file big_file.h
 * Big file management
 *
 * $Id: big_file.h,v 1.2 2002/10/25 15:45:59 berenguier Exp $
 */

/* Copyright, 2000, 2002 Nevrax Ltd.
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

#ifndef NL_BIG_FILE_H
#define NL_BIG_FILE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/tds.h"


namespace NLMISC {

/**
 * Big file management
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2002
 */

const uint32 BF_ALWAYS_OPENED		=	0x00000001;
const uint32 BF_CACHE_FILE_ON_OPEN	=	0x00000002;

// ***************************************************************************
class CBigFile
{

public:

	// Retrieve the global instance
	static CBigFile &getInstance ();

	// Add a big file to the manager
	bool add (const std::string &sBigFileName, uint32 nOptions);

	// Remove a big file from the manager
	void remove (const std::string &sBigFileName);

	// List all files in a bigfile
	void list (const std::string &sBigFileName, std::vector<std::string> &vAllFiles);

	// Remove all big files added
	void removeAll ();

	// Used by CIFile to get information about the files within the big file
	FILE* getFile (const std::string &sFileName, uint32 &rFileSize, uint32 &rBigFileOffset, 
					bool &rCacheFileOnOpen, bool &rAlwaysOpened);

// ***************
private:
	class	CThreadFileArray;
	friend class	CThreadFileArray;

	// A ptr to a file.
	struct	CHandleFile
	{
		FILE		*File;
		CHandleFile()
		{
			File= NULL;
		}
	};

	// A class which return a FILE * handle per Thread.
	class	CThreadFileArray
	{
	public:
		CThreadFileArray();

		// Allocate a FileId for a BNP.
		uint32			allocate();
		// Given a BNP File Id, return its FILE* handle for the current thread.
		CHandleFile		&get(uint32 index);

	private:
		// Do it this way because a few limited TDS is possible (64 on NT4)
		CTDS		_TDS;
		// The array is grow only!!
		uint32		_CurrentId;
	};

	// A BNPFile header
	struct BNPFile
	{
		uint32		Size;
		uint32		Pos;
	};

	// A BNP structure
	struct BNP
	{
		// FileName of the BNP. important to open it in getFile() (for other threads or if not always opened).
		std::string						BigFileName;
		// map of files in the BNP.
		std::map<std::string, BNPFile>	Files;
		// Since many seek may be done on a FILE*, each thread should have its own FILE opened.
		uint32							ThreadFileId;
		bool							CacheFileOnOpen;
		bool							AlwaysOpened;
	};
private:

	CBigFile(); // Singleton mode -> access it with the getInstance function

	static CBigFile				*_Singleton;

	// This is an array of CHandleFile, unique to each thread
	CThreadFileArray			_ThreadFileArray;

	std::map<std::string, BNP> _BNPs;
};

} // NLMISC


#endif // NL_BIG_FILE_H

/* End of big_file.h */
