/** \file file.cpp
 * Standard File Input/Output
 *
 * $Id: file.cpp,v 1.9 2000/10/24 15:24:33 lecroart Exp $
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