/* file.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: file.cpp,v 1.4 2000/09/13 14:55:02 berenguier Exp $
 *
 * Standard File Input/Output.
 */


#include "nel/misc/file.h"
using namespace std;


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
bool	CIFile::open(std::string path)
{
	close();
	_F=fopen(path.c_str(), "rb");
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
bool	COFile::open(std::string path)
{
	close();
	_F=fopen(path.c_str(), "wb");
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