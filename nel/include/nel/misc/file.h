/** \file file.cpp
 * Standard File Input/Output
 *
 * $Id: file.h,v 1.11 2001/03/05 09:15:41 corvazier Exp $
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

#ifndef NL_FILE_H
#define NL_FILE_H

#include "nel/misc/stream.h"



namespace NLMISC
{

// ======================================================================================================
/**
 * File Exception.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct EFile : public EStream
{
public:
	EFile () { _Reason = "Unknown file error"; }
	EFile (const std::string& filename) { _Reason = "Unknown file error in '"+filename+"'"; }
};

struct EFileNotOpened : public EFile
{
	EFileNotOpened( const std::string& filename )
	{
		_Reason = "File '"+filename+"' not opened";
	}
};

struct EReadError : public EFile
{
	EReadError( const std::string& filename )
	{
		_Reason = "Read error in file '" +filename+"' (End of file??)";
	}
};

struct EWriteError : public EFile
{
	EWriteError( const std::string& filename )
	{
		_Reason = "Write Error in file '" +filename+"'";
	}
};


// ======================================================================================================
/**
 * Input File.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CIFile : public IStream
{
public:		// Basic Usage.
	/// Object. NB: destructor close() the stream.
	CIFile();
	CIFile(const std::string &path, bool text=false);
	~CIFile();

	/// Open a file for reading. false if failed. close() if a file was opened.
	bool	open(const std::string &path, bool text=false);

	
public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint32 offset, IStream::TSeekOrigin origin) throw(EStream);
	/// Gt the location of the file pointer
	sint32	getPos () throw(EStream);

protected:
	virtual void		serialBuffer(uint8 *buf, uint len)throw(EReadError);
	virtual void		serialBit(bool &bit) throw(EReadError);

private:
	FILE	*_F;
	std::string _FileName;
};


// ======================================================================================================
/**
 * Output File.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class COFile : public IStream
{
public:		// Basic Usage.
	/// Object. NB: destructor close() the stream.
	COFile();
	COFile(const std::string &path, bool append=false, bool text=false);
	~COFile();

	/// Open a file for writing. false if failed. close() if a file was opened.
	bool	open(const std::string &path, bool append=false, bool text=false);

	
public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint32 offset, IStream::TSeekOrigin origin) throw(EStream);
	/// Gt the location of the file pointer
	sint32	getPos () throw(EStream);

	// very useful to serialize string in text mode (without the size)
	virtual void		serialBuffer(uint8 *buf, uint len) throw(EWriteError);

protected:
	virtual void		serialBit(bool &bit) throw(EWriteError);

private:
	FILE	*_F;
	std::string _FileName;
};


}


#endif // NL_FILE_H

/* End of file.h */
