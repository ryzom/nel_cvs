/* file.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: file.h,v 1.1 2000/09/12 08:15:39 berenguier Exp $
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
	bool	open(std::string path);

	
public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();

protected:
	virtual void		serial(uint8 *buf, uint len) throw(EStream);
	virtual void		serialBit(bool &bit) throw(EStream);

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
	bool	open(std::string path);

	
public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();

protected:
	virtual void		serial(uint8 *buf, uint len) throw(EStream);
	virtual void		serialBit(bool &bit) throw(EStream);

private:
	FILE	*_F;
};


}


#endif // NL_FILE_H

/* End of file.h */
