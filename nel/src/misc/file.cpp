/** \file file.cpp
 * Standard File Input/Output
 *
 * $Id: file.cpp,v 1.26 2002/10/10 12:43:05 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include "nel/misc/big_file.h"

using namespace std;

namespace NLMISC
{

uint32 CIFile::_NbBytesSerialized = 0;
uint32 CIFile::_NbBytesLoaded = 0;

// ======================================================================================================
CIFile::CIFile() : IStream(true)
{
	_F = NULL;
	_Cache = NULL;
	_ReadPos = 0;
	_FileSize = 0;
	_BigFileOffset = 0;
	_IsInBigFile = false;
	_CacheFileOnOpen = false;
	_IsAsyncLoading = false;
	_AllowBNPCacheFileOnOpen= true;
}

// ======================================================================================================
CIFile::CIFile(const std::string &path, bool text) : IStream(true)
{
	_F=NULL;
	_Cache = NULL;
	_ReadPos = 0;
	_FileSize = 0;
	_BigFileOffset = 0;
	_IsInBigFile = false;
	_CacheFileOnOpen = false;
	_IsAsyncLoading = false;
	_AllowBNPCacheFileOnOpen= true;
	open(path, text);
}

// ======================================================================================================
CIFile::~CIFile()
{
	close();
}


// ======================================================================================================
void		CIFile::loadIntoCache()
{
	const uint32 READPACKETSIZE = 64 * 1024;
	const uint32 INTERPACKETSLEEP = 5;

	_Cache = new uint8[_FileSize];
	if(!_IsAsyncLoading)
	{
		fread (_Cache, _FileSize, 1, _F);
	}
	else
	{
		uint	index= 0;
		while(index<_FileSize)
		{
			if( _NbBytesLoaded + (_FileSize-index) > READPACKETSIZE )
			{
				sint	n= READPACKETSIZE-_NbBytesLoaded;
				n= max(n, 1);
				fread (_Cache+index, n, 1, _F);
				index+= n;

				nlSleep (INTERPACKETSLEEP);
				_NbBytesLoaded= 0;
			}
			else
			{
				uint	n= _FileSize-index;
				fread (_Cache+index, n, 1, _F);
				_NbBytesLoaded+= n;
				index+= n;
			}
		}
	}
}


// ======================================================================================================
bool		CIFile::open(const std::string &path, bool text)
{
	close();

	// can't open empty filename
	if(path.empty ())
		return false;

	char mode[3];
	mode[0] = 'r';
	mode[1] = (text)?'\0':'b';
	mode[2] = '\0';

	_FileName = path;
	_ReadPos = 0;

	// Bigfile access requested ?
	if (path.find('@') != string::npos)
	{
		_IsInBigFile = true;
		if(_AllowBNPCacheFileOnOpen)
		{
			_F = CBigFile::getInstance().getFile (path, _FileSize, _BigFileOffset, _CacheFileOnOpen, _AlwaysOpened);
		}
		else
		{
			bool	dummy;
			_F = CBigFile::getInstance().getFile (path, _FileSize, _BigFileOffset, dummy, _AlwaysOpened);
		}
		if(_F != NULL)
		{
			// Start to load the bigfile at the file offset.
			fseek (_F, _BigFileOffset, SEEK_SET);

			// Load into cache ?
			if (_CacheFileOnOpen)
			{
				// load file in the cache
				loadIntoCache();

				if (!_AlwaysOpened)
				{
					fclose (_F);
					_F = NULL;
				}
				return (_Cache != NULL);
			}
		}
	}
	else
	{
		_IsInBigFile = false;
		_BigFileOffset = 0;
		_AlwaysOpened = false;
		_F = fopen (path.c_str(), mode);
		if (_F != NULL)
		{
			fseek (_F, 0, SEEK_END);
			_FileSize = ftell(_F);
			fseek (_F, 0, SEEK_SET);
		}
		else
		{
			_FileSize = 0;
		}
		
		if ((_CacheFileOnOpen) && (_F != NULL))
		{
			// load file in the cache
			loadIntoCache();

			fclose (_F);
			_F = NULL;
			return (_Cache != NULL);
		}
	}

	return (_F != NULL);
}

// ======================================================================================================
void		CIFile::setCacheFileOnOpen (bool newState)
{
	_CacheFileOnOpen = newState;
}

// ======================================================================================================
void		CIFile::setAsyncLoading (bool newState)
{
	_IsAsyncLoading = true;
}


// ======================================================================================================
void		CIFile::close()
{
	if (_CacheFileOnOpen)
	{
		delete _Cache;
		_Cache = NULL;
	}
	else
	{
		if (_IsInBigFile)
		{
			if (!_AlwaysOpened)
			{
				if (_F)
				{
					fclose (_F);
					_F = NULL;
				}
			}
		}
		else
		{
			if (_F)
			{
				fclose (_F);
				_F = NULL;
			}
		}
	}
	resetPtrTable();
}

// ======================================================================================================
void		CIFile::flush()
{
	if (_CacheFileOnOpen)
	{
	}
	else
	{
		if (_F)
		{
			fflush (_F);
		}
	}
}

// ======================================================================================================
void		CIFile::getline (char *buffer, uint32 bufferSize)
{
	if (bufferSize == 0)
		return;

	uint read = 0;
	while (true)
	{
		if (read == bufferSize -1)
		{
			*buffer = '\0';
			return;
		}

		try
		{
			// read one byte
			serialBuffer ((uint8 *)buffer, 1);
		}
		catch (EFile &)
		{
			*buffer = '\0';
			return;
		}

		if (*buffer == '\n')
		{
			*buffer = '\0';
			return;
		}

		// skip '\r' char
		if (*buffer != '\r')
			buffer++;
	}

}


// ======================================================================================================
bool		CIFile::eof ()
{
	return _ReadPos >= (sint32)_FileSize;
}

// ======================================================================================================
void		CIFile::serialBuffer(uint8 *buf, uint len) throw(EReadError)
{
	// Check the read pos
	if ((_ReadPos < 0) || ((_ReadPos+len) > _FileSize))
		throw EReadError (_FileName);
	if ((_CacheFileOnOpen) && (_Cache == NULL))
		throw EFileNotOpened (_FileName);
	if ((!_CacheFileOnOpen) && (_F == NULL))
		throw EFileNotOpened (_FileName);

	if (_IsAsyncLoading)
	{
		_NbBytesSerialized += len;
		if (_NbBytesSerialized > 64 * 1024)
		{
			nlSleep (5);
			_NbBytesSerialized = 0;
		}
	}

	if (_CacheFileOnOpen)
	{
		memcpy (buf, _Cache + _ReadPos, len);
		_ReadPos += len;
	}
	else
	{
		if (fread(buf, 1, len, _F) < len)
			throw EReadError(_FileName);
		_ReadPos += len;
	}
}

// ======================================================================================================
void		CIFile::serialBit(bool &bit) throw(EReadError)
{
	// Simple for now.
	uint8	v=bit;
	serialBuffer(&v, 1);
	bit=(v!=0);
}

// ======================================================================================================
bool		CIFile::seek (sint32 offset, IStream::TSeekOrigin origin) throw(EStream)
{
	if ((_CacheFileOnOpen) && (_Cache == NULL))
		return false;
	if ((!_CacheFileOnOpen) && (_F == NULL))
		return false;

	switch (origin)
	{
		case IStream::begin:
			_ReadPos = offset;
		break;
		case IStream::current:
			_ReadPos = _ReadPos + offset;
		break;
		case IStream::end:
			_ReadPos = _FileSize + offset; 
		break;
		default:
			nlstop;
	}

	if (_CacheFileOnOpen)
		return true;

	// seek in the file. NB: if not in bigfile, _BigFileOffset==0.
	if (fseek(_F, _BigFileOffset+_ReadPos, SEEK_SET) != 0)
		return false;
	return true;
}

// ======================================================================================================
sint32		CIFile::getPos () throw(EStream)
{
	return _ReadPos;
}


// ======================================================================================================
std::string	CIFile::getStreamName() const
{
	return _FileName;
}


// ======================================================================================================
void	CIFile::allowBNPCacheFileOnOpen(bool newState)
{
	_AllowBNPCacheFileOnOpen= newState;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
COFile::COFile() : IStream(false)
{
	_F=NULL;
	_FileName = "";
}

// ======================================================================================================
COFile::COFile(const std::string &path, bool append, bool text) : IStream(false)
{
	_F=NULL;
	open(path, append, text);
}

// ======================================================================================================
COFile::~COFile()
{
	close();
}
// ======================================================================================================
bool	COFile::open(const std::string &path, bool append, bool text)
{
	close();

	// can't open empty filename
	if(path.empty ())
		return false;

	char mode[3];
	mode[0] = (append)?'a':'w';
	mode[1] = (text)?'\0':'b';
	mode[2] = '\0';

	_F=fopen(path.c_str(), mode);
	_FileName = path;

	return _F!=NULL;
}
// ======================================================================================================
void	COFile::close()
{
	if(_F)
	{
		fclose(_F);
		_F=NULL;
	}
	resetPtrTable();
}
// ======================================================================================================
void	COFile::flush()
{
	if(_F)
	{
		fflush(_F);
	}
}


// ======================================================================================================
void		COFile::serialBuffer(uint8 *buf, uint len) throw(EWriteError)
{
	if(!_F)
		throw	EFileNotOpened(_FileName);
	if(fwrite(buf, 1, len, _F) < len)
		throw	EWriteError(_FileName);
}
// ======================================================================================================
void		COFile::serialBit(bool &bit) throw(EWriteError)
{
	// Simple for now.
	uint8	v=bit;
	serialBuffer(&v, 1);
}
// ======================================================================================================
bool		COFile::seek (sint32 offset, IStream::TSeekOrigin origin) throw(EStream)
{
	if (_F)
	{
		int origin_c = SEEK_SET;
		switch (origin)
		{
		case IStream::begin:
			origin_c=SEEK_SET;
			break;
		case IStream::current:
			origin_c=SEEK_CUR;
			break;
		case IStream::end:
			origin_c=SEEK_END;
			break;
		default:
			nlstop;
		}

		if (fseek (_F, offset, origin_c)!=0)
			return false;
		return true;
	}
	return false;
}
// ======================================================================================================
sint32		COFile::getPos () throw(EStream)
{
	if (_F)
	{
		return ftell (_F);
	}
	return 0;
}

// ======================================================================================================
std::string		COFile::getStreamName() const
{
	return _FileName;
}


}
