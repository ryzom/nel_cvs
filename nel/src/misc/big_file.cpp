/** \file big_file.cpp
 * Big file management
 *
 * $Id: big_file.cpp,v 1.6 2002/11/18 10:03:06 berenguier Exp $
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

#include "stdmisc.h"

#include "nel/misc/big_file.h"
#include "nel/misc/path.h"

using namespace std;

namespace NLMISC {

CBigFile *CBigFile::_Singleton = NULL;


// ***************************************************************************
CBigFile::CThreadFileArray::CThreadFileArray()
{
	_CurrentId= 0;
}
// ***************************************************************************
uint32						CBigFile::CThreadFileArray::allocate()
{
	return _CurrentId++;
}
// ***************************************************************************
CBigFile::CHandleFile		&CBigFile::CThreadFileArray::get(uint32 index)
{
	// If the thread struct ptr is NULL, must allocate it.
	vector<CHandleFile>		*ptr= (vector<CHandleFile>*)_TDS.getPointer();
	if(ptr==NULL)
	{
		ptr= new vector<CHandleFile>;
		_TDS.setPointer(ptr);
	}

	// if the vector is not allocated, allocate it (empty entries filled with NULL => not opened FILE* in this thread)
	if(index>=ptr->size())
	{
		ptr->resize(index+1);
	}

	return (*ptr)[index];
}


// ***************************************************************************
CBigFile::CBigFile ()
{
}

// ***************************************************************************
CBigFile &CBigFile::getInstance ()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CBigFile();
	}
	return *_Singleton;
}

// ***************************************************************************
bool CBigFile::add (const std::string &sBigFileName, uint32 nOptions)
{
	BNP bnpTmp;

	bnpTmp.BigFileName= sBigFileName;

	// Is already the same bigfile name ?
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	if (_BNPs.find(bigfilenamealone) != _BNPs.end())
		return false;

	// Allocate a new ThreadSafe FileId for this bnp.
	bnpTmp.ThreadFileId= _ThreadFileArray.allocate();

	// Get a ThreadSafe handle on the file
	CHandleFile		&handle= _ThreadFileArray.get(bnpTmp.ThreadFileId);
	// Open the big file.
	handle.File = fopen (sBigFileName.c_str(), "rb");
	if (handle.File == NULL)
		return false;
	fseek (handle.File, 0, SEEK_END);
	uint32 nFileSize = ftell (handle.File);
	fseek (handle.File, nFileSize-4, SEEK_SET);
	uint32 nOffsetFromBegining;
	fread (&nOffsetFromBegining, sizeof(uint32), 1, handle.File);
	fseek (handle.File, nOffsetFromBegining, SEEK_SET);
	uint32 nNbFile;
	fread (&nNbFile, sizeof(uint32), 1, handle.File);
	for (uint32 i = 0; i < nNbFile; ++i)
	{
		char FileName[256];
		uint8 nStringSize;
		fread (&nStringSize, 1, 1, handle.File);
		fread (FileName, 1, nStringSize, handle.File);
		FileName[nStringSize] = 0;
		uint32 nFileSize;
		fread (&nFileSize, sizeof(uint32), 1, handle.File);
		uint32 nFilePos;
		fread (&nFilePos, sizeof(uint32), 1, handle.File);
		BNPFile bnpfTmp;
		bnpfTmp.Pos = nFilePos;
		bnpfTmp.Size = nFileSize;
		bnpTmp.Files.insert (make_pair(strlwr(string(FileName)), bnpfTmp));
	}
	fseek (handle.File, 0, SEEK_SET);

	if (nOptions&BF_CACHE_FILE_ON_OPEN)
		bnpTmp.CacheFileOnOpen = true;
	else
		bnpTmp.CacheFileOnOpen = false;

	if (!(nOptions&BF_ALWAYS_OPENED))
	{
		fclose (handle.File);
		handle.File = NULL;
		bnpTmp.AlwaysOpened = false;
	}
	else
	{
		bnpTmp.AlwaysOpened = true;
	}

	_BNPs.insert (make_pair(strlwr(bigfilenamealone), bnpTmp));

	return true;
}

// ***************************************************************************
void CBigFile::remove (const std::string &sBigFileName)
{
	if (_BNPs.find (sBigFileName) != _BNPs.end())
	{
		map<string, BNP>::iterator it = _BNPs.find (sBigFileName);
		BNP &rbnp = it->second;
		/* \todo yoyo: THERE is a MAJOR drawback here: Only the FILE * of the current (surely main) thread
			is closed. other FILE* in other threads are still opened. This is not a big issue (system close the FILE* 
			at the end of the process) and this is important so AsyncLoading of a currentTask can end up correclty 
			(without an intermediate fclose()).
		*/
		// Get a ThreadSafe handle on the file
		CHandleFile		&handle= _ThreadFileArray.get(rbnp.ThreadFileId);
		// close it if needed
		if (handle.File != NULL)
		{
			fclose (handle.File);
			handle.File= NULL;
		}
		_BNPs.erase (it);
	}
}

// ***************************************************************************
bool CBigFile::isBigFileAdded(const std::string &sBigFileName)
{
	// Is already the same bigfile name ?
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	return _BNPs.find(bigfilenamealone) != _BNPs.end();
}

// ***************************************************************************
void CBigFile::list (const std::string &sBigFileName, std::vector<std::string> &vAllFiles)
{
	string lwrFileName = strlwr (sBigFileName);
	if (_BNPs.find (lwrFileName) == _BNPs.end())
		return;
	vAllFiles.clear ();
	BNP &rbnp = _BNPs.find (lwrFileName)->second;
	map<string,BNPFile>::iterator it = rbnp.Files.begin();
	while (it != rbnp.Files.end())
	{
		vAllFiles.push_back (it->first); // Add the name of the file to the return vector
		++it;
	}
}

// ***************************************************************************
void CBigFile::removeAll ()
{
	while (_BNPs.begin() != _BNPs.end())
	{
		remove (_BNPs.begin()->first);
	}
}

// ***************************************************************************
FILE* CBigFile::getFile (const std::string &sFileName, uint32 &rFileSize, 
						 uint32 &rBigFileOffset, bool &rCacheFileOnOpen, bool &rAlwaysOpened)
{
	string zeFileName, zeBigFileName, lwrFileName = strlwr (sFileName);
	uint32 i, nPos = sFileName.find ('@');
	if (nPos == string::npos)
	{
		nlwarning ("Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	for (i = 0; i < nPos; ++i)
		zeBigFileName += lwrFileName[i];
	++i; // Skip @
	for (; i < lwrFileName.size(); ++i)
		zeFileName += lwrFileName[i];

	if (_BNPs.find (zeBigFileName) == _BNPs.end())
	{
		nlwarning ("Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	BNP &rbnp = _BNPs.find (zeBigFileName)->second;
	if (rbnp.Files.find (zeFileName) == rbnp.Files.end())
	{
		nlwarning ("Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	BNPFile &rbnpfile = rbnp.Files.find (zeFileName)->second;

	// Get a ThreadSafe handle on the file
	CHandleFile		&handle= _ThreadFileArray.get(rbnp.ThreadFileId);
	/* If not opened, open it now. There is 2 reason for it to be not opened: 
		rbnp.AlwaysOpened==false, or it is a new thread which use it for the first time.
	*/
	if(handle.File== NULL)
	{
		handle.File = fopen (rbnp.BigFileName.c_str(), "rb");
		if (handle.File == NULL)
			return NULL;
	}

	rCacheFileOnOpen = rbnp.CacheFileOnOpen;
	rAlwaysOpened = rbnp.AlwaysOpened;
	rBigFileOffset = rbnpfile.Pos;
	rFileSize = rbnpfile.Size;
	return handle.File;
}

} // namespace NLMISC
