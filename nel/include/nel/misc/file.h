/* file.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: file.h,v 1.5 2000/10/19 15:21:22 corvazier Exp $
 *
 * Standard File Input/Output.
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
	virtual const char	*what() const throw() {return "File Error";}
};
struct EFileNotOpened : public EFile
{
	virtual const char	*what() const throw() {return "File Not Opened";}
};
struct EReadError : public EFile
{
	virtual const char	*what() const throw() {return "Read Error (End of file??)";}
};
struct EWriteError : public EFile
{
	virtual const char	*what() const throw() {return "Write Error";}
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
	~CIFile();

	/// Open a file for reading. false if failed. close() if a file was opened.
	bool	open(std::string path, bool text=false);

	
public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint64 offset, IStream::TSeekOrigin origin) throw(EStream);
	/// Gt the location of the file pointer
	sint64	getpos () throw(EStream);

protected:
	virtual void		serialBuffer(uint8 *buf, uint len)throw(EReadError);
	virtual void		serialBit(bool &bit) throw(EReadError);

private:
	FILE	*_F;
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
	~COFile();

	/// Open a file for writing. false if failed. close() if a file was opened.
	bool	open(std::string path, bool append=false, bool text=false);

	
public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint64 offset, IStream::TSeekOrigin origin) throw(EStream);
	/// Gt the location of the file pointer
	sint64	getpos () throw(EStream);

	// very useful to serialize string in text mode (without the size)
	virtual void		serialBuffer(uint8 *buf, uint len) throw(EWriteError);

protected:
	virtual void		serialBit(bool &bit) throw(EWriteError);

private:
	FILE	*_F;
};


}


#endif // NL_FILE_H

/* End of file.h */
