/** \file big_file.cpp
 * Big file management
 *
 * $Id: big_file.cpp,v 1.14 2004/01/15 17:39:40 lecroart Exp $
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
using namespace NLMISC;

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
	{
		nlwarning ("CBigFile::add : bigfile %s already added.", bigfilenamealone.c_str());
		return false;
	}

	// Allocate a new ThreadSafe FileId for this bnp.
	bnpTmp.ThreadFileId= _ThreadFileArray.allocate();

	// Get a ThreadSafe handle on the file
	CHandleFile		&handle= _ThreadFileArray.get(bnpTmp.ThreadFileId);
	// Open the big file.
	handle.File = fopen (sBigFileName.c_str(), "rb");
	if (handle.File == NULL)
		return false;
	uint32 nFileSize=CFile::getFileSize (handle.File);
	//nlfseek64 (handle.File, 0, SEEK_END);
	//uint32 nFileSize = ftell (handle.File);

	// Result
	if (nlfseek64 (handle.File, nFileSize-4, SEEK_SET) != 0)
		return false;

	uint32 nOffsetFromBegining;
	if (fread (&nOffsetFromBegining, sizeof(uint32), 1, handle.File) != 1)
		return false;

	if (nlfseek64 (handle.File, nOffsetFromBegining, SEEK_SET) != 0)
		return false;

	// Read the file count
	uint32 nNbFile;
	if (fread (&nNbFile, sizeof(uint32), 1, handle.File) != 1)
		return false;
	map<string,BNPFile> tempMap;
	for (uint32 i = 0; i < nNbFile; ++i)
	{
		char FileName[256];
		uint8 nStringSize;
		if (fread (&nStringSize, 1, 1, handle.File) != 1)
			return false;
		
		if (fread (FileName, 1, nStringSize, handle.File) != nStringSize)
			return false;

		FileName[nStringSize] = 0;
		uint32 nFileSize2;
		if (fread (&nFileSize2, sizeof(uint32), 1, handle.File) != 1)
			return false;

		uint32 nFilePos;
		if (fread (&nFilePos, sizeof(uint32), 1, handle.File) != 1)
			return false;

		BNPFile bnpfTmp;
		bnpfTmp.Pos = nFilePos;
		bnpfTmp.Size = nFileSize2;
		tempMap.insert (make_pair(strlwr(string(FileName)), bnpfTmp));
	}

	if (nlfseek64 (handle.File, 0, SEEK_SET) != 0)
		return false;

	// Convert temp map
	if (nNbFile > 0)
	{
		uint nSize = 0, nNb = 0;
		map<string,BNPFile>::iterator it = tempMap.begin();
		while (it != tempMap.end())
		{
			nSize += it->first.size() + 1;
			nNb++;
			it++;
		}

		bnpTmp.FileNames = new char[nSize];
		memset(bnpTmp.FileNames, 0, nSize);
		bnpTmp.Files.resize(nNb);

		it = tempMap.begin();
		nSize = 0;
		nNb = 0;
		while (it != tempMap.end())
		{
			strcpy(bnpTmp.FileNames+nSize, it->first.c_str());
			
			bnpTmp.Files[nNb].Name = bnpTmp.FileNames+nSize;
			bnpTmp.Files[nNb].Size = it->second.Size;
			bnpTmp.Files[nNb].Pos = it->second.Pos;

			nSize += it->first.size() + 1;
			nNb++;
			it++;
		}
	}
	// End of temp map conversion

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
		/* \todo trap : can make the CPath crash. CPath must be informed that the files in bigfiles have been removed
			this is because CPath use memory of CBigFile if it runs in memoryCompressed mode */
		delete [] rbnp.FileNames;
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
	vector<BNPFile>::iterator it = rbnp.Files.begin();
	while (it != rbnp.Files.end())
	{
		vAllFiles.push_back (string(it->Name)); // Add the name of the file to the return vector
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
		nlwarning ("BF: Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	for (i = 0; i < nPos; ++i)
		zeBigFileName += lwrFileName[i];
	++i; // Skip @
	for (; i < lwrFileName.size(); ++i)
		zeFileName += lwrFileName[i];

	if (_BNPs.find (zeBigFileName) == _BNPs.end())
	{
		nlwarning ("BF: Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	BNP &rbnp = _BNPs.find (zeBigFileName)->second;
	if (rbnp.Files.size() == 0)
	{
		nlwarning ("BF: Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	vector<BNPFile>::iterator itNBPFile;
	itNBPFile = lower_bound(rbnp.Files.begin(), rbnp.Files.end(), zeFileName.c_str(), CBNPFileComp());
	if (itNBPFile != rbnp.Files.end())
	{
		if (strcmp(itNBPFile->Name, zeFileName.c_str()) != 0)
		{
			nlwarning ("BF: Couldn't load '%s'", sFileName.c_str());
			return NULL;
		}
	}
	else
	{
		nlwarning ("BF: Couldn't load '%s'", sFileName.c_str());
		return NULL;
	}

	BNPFile &rbnpfile = *itNBPFile;

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

// ***************************************************************************
char *CBigFile::getFileNamePtr(const std::string &sFileName, const std::string &sBigFileName)
{
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	if (_BNPs.find(bigfilenamealone) != _BNPs.end())
	{
		BNP &rbnp = _BNPs.find (bigfilenamealone)->second;
		vector<BNPFile>::iterator itNBPFile;
		if (rbnp.Files.size() == 0)
			return NULL;
		string lwrFileName = strlwr (sFileName);
		itNBPFile = lower_bound(rbnp.Files.begin(), rbnp.Files.end(), lwrFileName.c_str(), CBNPFileComp());
		if (itNBPFile != rbnp.Files.end())
		{
			if (strcmp(itNBPFile->Name, lwrFileName.c_str()) == 0)
			{
				return itNBPFile->Name;
			}
		}
	}

	return NULL;
}


} // namespace NLMISC
