/* file.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: file.cpp,v 1.7 2000/10/18 12:36:55 corvazier Exp $
 *
 * Standard File Input/Output.
 */


#include "nel/misc/file.h"
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




}