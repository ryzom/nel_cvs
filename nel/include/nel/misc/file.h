/** \file file.h
 * From file serialization implementation of IStream using binary format (look at stream.h)
 *
 * $Id: file.h,v 1.23 2003/06/03 13:05:02 corvazier Exp $
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
	EFile () : EStream( "Unknown file error" ) {}
	EFile (const std::string& filename) : EStream( "Unknown file error in '"+filename+"'" ) {}
	EFile (const std::string& text, bool ) : EStream( text ) {}
};

struct EFileNotOpened : public EFile
{
	EFileNotOpened( const std::string& filename ) : EFile( "File '"+filename+"' not opened", true ) {}
};

struct EReadError : public EFile
{
	EReadError( const std::string& filename ) : EFile( "Read error in file '" +filename+"' (End of file?)", true ) {}
};

struct EWriteError : public EFile
{
	EWriteError( const std::string& filename ) : EFile( "Write Error in file '" +filename+"'", true ) {}
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
	bool	open (const std::string &path, bool text=false);

	/// Set the cache file on open option (default behaviour is false (file is not cached at opening)
	void	setCacheFileOnOpen (bool newState);

	/** If the file is opened with a big file, CacheFileOnOpen is replaced with big file option. Except if
	 *	allowBNPCacheFileOnOpen(false) is called. true is default.
	 */
	void	allowBNPCacheFileOnOpen(bool newState);

	/// Set the async loading state (to go to sleep 5 ms after 100 Ko serialized)
	void	setAsyncLoading (bool newState);

public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint32 offset, IStream::TSeekOrigin origin) throw(EStream);
	/// Gt the location of the file pointer
	sint32	getPos () throw(EStream);

	// Imp the Name of the stream as the name of the file.
	virtual std::string		getStreamName() const;

	// same function that in ifstream
	// return a string separated by \n or eof, used to parsing text file
	void getline (char *buffer, uint32 bufferSize);

	// return the size of the file
	uint32 getFileSize () { return _FileSize; }

	// return true if there s nothing more to read (same as ifstream)
	bool eof ();

	virtual void		serialBuffer(uint8 *buf, uint len)throw(EReadError);

	/// \name Statistics
	
	/// Get the number of file open from the begining of the application. Files can be in a big file.
	static uint32	getNumFileOpen() {return _FileOpened;}
	
	/// Get the number of read acces to a file.
	static uint32	getNumFileRead() {return _FileRead;}
	
	/// Get the number of byte read from the file system since the application start.
	static uint32	getReadFromFile() {return _ReadFromFile;}
	
	/// Get the number of byte being reading from the file system at the moment.
	static uint32	getReadingFromFile() {return _ReadingFromFile;}
	
	/// Get the last 40 files opened. The files can be in a big file.
	static void		dump (std::vector<std::string> &result);
	
protected:
	virtual void		serialBit(bool &bit) throw(EReadError);

private:
	FILE		*_F;
	std::string _FileName;

	// Async
	static uint32 _NbBytesSerialized;
	static uint32 _NbBytesLoaded;

	// Stats
	static uint32 _FileOpened;
	static uint32 _FileRead;
	static uint32 _ReadFromFile;
	static uint32 _ReadingFromFile;
	static CSynchronized<std::list<std::string> > _OpenedFiles;
	
	bool _IsAsyncLoading;

	// Cache 
	bool	_CacheFileOnOpen;
	bool	_AllowBNPCacheFileOnOpen;
	uint8	*_Cache;
	sint32	_ReadPos;
	uint32	_FileSize;

	// Big file
	bool	_AlwaysOpened;
	bool	_IsInBigFile;
	uint32	_BigFileOffset;

	// Load async if needed in the cache.
	void	loadIntoCache();
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

	// Imp the Name of the stream as the name of the file.
	virtual std::string		getStreamName() const;

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
