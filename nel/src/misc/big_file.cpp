/** \file big_file.cpp
 * Big file management
 *
 * $Id: big_file.cpp,v 1.1 2002/04/24 08:14:14 besson Exp $
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

// ======================================================================================================
CBigFile::CBigFile ()
{
}

// ======================================================================================================
CBigFile &CBigFile::getInstance ()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CBigFile();
	}
	return *_Singleton;
}

// ======================================================================================================
bool CBigFile::add (const std::string &sBigFileName, uint32 nOptions)
{
	BNP bnpTmp;

	// Is already the same bigfile name ?
	string bigfilenamealone = CFile::getFilename (sBigFileName);
	if (_BNPs.find(bigfilenamealone) != _BNPs.end())
		return false;
	bnpTmp.Handle = fopen (sBigFileName.c_str(), "rb");
	if (bnpTmp.Handle == NULL)
		return false;
	fseek (bnpTmp.Handle, 0, SEEK_END);
	uint32 nFileSize = ftell (bnpTmp.Handle);
	fseek (bnpTmp.Handle, nFileSize-4, SEEK_SET);
	uint32 nOffsetFromBegining;
	fread (&nOffsetFromBegining, sizeof(uint32), 1, bnpTmp.Handle);
	fseek (bnpTmp.Handle, nOffsetFromBegining, SEEK_SET);
	uint32 nNbFile;
	fread (&nNbFile, sizeof(uint32), 1, bnpTmp.Handle);
	for (uint32 i = 0; i < nNbFile; ++i)
	{
		char FileName[256];
		uint8 nStringSize;
		fread (&nStringSize, 1, 1, bnpTmp.Handle);
		fread (FileName, 1, nStringSize, bnpTmp.Handle);
		FileName[nStringSize] = 0;
		uint32 nFileSize;
		fread (&nFileSize, sizeof(uint32), 1, bnpTmp.Handle);
		uint32 nFilePos;
		fread (&nFilePos, sizeof(uint32), 1, bnpTmp.Handle);
		BNPFile bnpfTmp;
		bnpfTmp.Pos = nFilePos;
		bnpfTmp.Size = nFileSize;
		bnpTmp.Files.insert (make_pair(strlwr(string(FileName)), bnpfTmp));
	}
	fseek (bnpTmp.Handle, 0, SEEK_SET);

	if (nOptions&BF_CACHE_FILE_ON_OPEN)
		bnpTmp.CacheFileOnOpen = true;
	else
		bnpTmp.CacheFileOnOpen = false;

	if (!(nOptions&BF_ALWAYS_OPENED))
	{
		fclose (bnpTmp.Handle);
		bnpTmp.Handle = NULL;
		bnpTmp.AlwaysOpened = false;
	}
	else
	{
		bnpTmp.AlwaysOpened = true;
	}

	_BNPs.insert (make_pair(strlwr(bigfilenamealone), bnpTmp));

	return true;
}

// ======================================================================================================
void CBigFile::remove (const std::string &sBigFileName)
{
	if (_BNPs.find (sBigFileName) != _BNPs.end())
	{
		map<string, BNP>::iterator it = _BNPs.find (sBigFileName);
		BNP &rbnp = it->second;
		if (rbnp.Handle != NULL)
		{
			fclose (rbnp.Handle);
		}
		_BNPs.erase (it);
	}
}

// ======================================================================================================
void CBigFile::list (const std::string &sBigFileName, std::vector<std::string> &vAllFiles)
{
	string zeFileName, zeBigFileName, lwrFileName = strlwr (sBigFileName);
	if (_BNPs.find (zeBigFileName) == _BNPs.end())
		return;
	vAllFiles.clear ();
	BNP &rbnp = _BNPs.find (zeBigFileName)->second;
	map<string,BNPFile>::iterator it = rbnp.Files.begin();
	while (it != rbnp.Files.end())
	{
		vAllFiles.push_back (it->first); // Add the name of the file to the return vector
		++it;
	}
}

// ======================================================================================================
void CBigFile::removeAll ()
{
	while (_BNPs.begin() != _BNPs.end())
	{
		remove (_BNPs.begin()->first);
	}
}

// ======================================================================================================
	
FILE* CBigFile::getFile (const std::string &sFileName, uint32 &rFileSize, 
						 uint32 &rBigFileOffset, bool &rCacheFileOnOpen, bool &rAlwaysOpened)
{
	string zeFileName, zeBigFileName, lwrFileName = strlwr (sFileName);
	sint32 i, nPos = sFileName.find ('@');
	if (nPos == string::npos)
		return NULL;

	for (i = 0; i < nPos; ++i)
		zeBigFileName += lwrFileName[i];
	++i; // Skip @
	for (; i < (sint32)lwrFileName.size(); ++i)
		zeFileName += lwrFileName[i];

	if (_BNPs.find (zeBigFileName) == _BNPs.end())
		return NULL;

	BNP &rbnp = _BNPs.find (zeBigFileName)->second;
	if (rbnp.Files.find (zeFileName) == rbnp.Files.end())
		return NULL;

	BNPFile &rbnpfile = rbnp.Files.find (zeFileName)->second;

	rCacheFileOnOpen = rbnp.CacheFileOnOpen;
	rAlwaysOpened = rbnp.AlwaysOpened;
	rBigFileOffset = rbnpfile.Pos;
	rFileSize = rbnpfile.Size;
	return rbnp.Handle;
}

} // namespace NLMISC