/* file.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: file.cpp,v 1.8 2000/10/19 15:21:14 corvazier Exp $
 *
 * Standard File Input/Output.
 */


#include "nel/misc/file.h"
#include "nel/misc/debug.h"
using namespace std;

#include <stdio.h>

namespace NLMISC
{


// ======================================================================================================
CIFile::CIFile() : IStream(true, true)
{
	_F=NULL;
}
// ======================================================================================================
CIFile::~CIFile()
{
	close();
}
// ======================================================================================================
bool	CIFile::open(std::string path, bool text)
{
	close();

	char mode[3];
	mode[0] = 'r';
	mode[1] = (text)?'\0':'b';
	mode[2] = '\0';

	_F=fopen(path.c_str(), mode);

	return _F!=NULL;
}
// ======================================================================================================
void	CIFile::close()
{
	if(_F)
	{
		fclose(_F);
		_F=NULL;
	}
	resetPtrTable();
}
// ======================================================================================================
void	CIFile::flush()
{
	if(_F)
	{
		fflush(_F);
	}
}
// ======================================================================================================
void		CIFile::serialBuffer(uint8 *buf, uint len) throw(EReadError)
{
	if(!_F)
		throw	EFileNotOpened();
	if(fread(buf, 1, len, _F) < len)
		throw	EReadError();
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
bool		CIFile::seek (sint64 offset, IStream::TSeekOrigin origin) throw(EStream)
{
	if (_F)
	{
		int origin_c;
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
			nlassert (0);		// no!
		}

		// Warning big file..
		while (offset>0x7fffffff)
		{
			if (fseek (_F, 0x7fffffff, origin_c)!=0)
				return false;
			offset-=0x7fffffff;
		}
		while (offset<(sint32)0xffffffff80000000)
		{
			if (fseek (_F, (sint32)0x80000000, origin_c)!=0)
				return false;
			offset-=(sint32)0xffffffff80000000;
		}
		if (fseek (_F, (sint32)offset, origin_c)!=0)
			return false;
		return true;
	}
	return false;
}
// ======================================================================================================
sint64		CIFile::getpos () throw(EStream)
{
	if (_F)
	{
		fpos_t pos ;
		if (fgetpos( _F, &pos)==0)
		{
			return pos;
		}
	}
	return 0;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
COFile::COFile() : IStream(false, true)
{
	_F=NULL;
}
// ======================================================================================================
COFile::~COFile()
{
	close();
}
// ======================================================================================================
bool	COFile::open(std::string path, bool append, bool text)
{
	close();

	char mode[3];
	mode[0] = (append)?'a':'w';
	mode[1] = (text)?'\0':'b';
	mode[2] = '\0';

	_F=fopen(path.c_str(), mode);

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
void		COFile::serialBuffer(uint8 *buf, uint len) throw(EStream)
{
	if(!_F)
		throw	EFileNotOpened();
	if(fwrite(buf, 1, len, _F) < len)
		throw	EWriteError();
}
// ======================================================================================================
void		COFile::serialBit(bool &bit) throw(EStream)
{
	// Simple for now.
	uint8	v=bit;
	serialBuffer(&v, 1);
}
// ======================================================================================================
bool		COFile::seek (sint64 offset, IStream::TSeekOrigin origin) throw(EStream)
{
	if (_F)
	{
		int origin_c;
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
			nlassert (0);		// no!
		}

		// Warning big file..
		while (offset>0x7fffffff)
		{
			if (fseek (_F, 0x7fffffff, origin_c)!=0)
				return false;
			offset-=0x7fffffff;
		}
		while (offset<(sint32)0xffffffff80000000)
		{
			if (fseek (_F, (sint32)0x80000000, origin_c)!=0)
				return false;
			offset-=(sint32)0xffffffff80000000;
		}
		if (fseek (_F, (sint32)offset, origin_c)!=0)
			return false;
		return true;
	}
	return false;
}
// ======================================================================================================
sint64		COFile::getpos () throw(EStream)
{
	if (_F)
	{
		fpos_t pos ;
		if (fgetpos( _F, &pos)==0)
		{
			return pos;
		}
	}
	return 0;
}




}