/* displayer.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: displayer.h,v 1.4 2000/10/04 15:27:53 cado Exp $
 *
 * Some easy displayers
 */

#ifndef NL_DISPLAYER_H
#define NL_DISPLAYER_H

#include <string>

namespace NLMISC
{

/**
 * Displayer interface. Used to specialize a displayer to display a string.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IDisplayer
{
public:

	/// Constructor
	IDisplayer() {}

	/// Destructor
	virtual ~IDisplayer() {}

	/// Display the string where it does.
	virtual void display (const std::string& str) = 0;
};



/**
 * Std displayer. Put string to stdout.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CStdDisplayer : virtual public IDisplayer
{
public:

	/// Constructor
	CStdDisplayer() {}

	/// Display the string to stdout and OutputDebugString on Windows
	virtual void display (const std::string& str);
};


/**
 * File displayer. Put string into a file.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CFileDisplayer : virtual public IDisplayer
{
public:

	/// Constructor
	CFileDisplayer(const std::string& fileName) { _FileName = fileName; }

	/// Put the string into the file.
	virtual void display (const std::string& str);

private:
	std::string _FileName;
};

/**
 * Message Box displayer. Put string into a message box.
 * \ref log_howto
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CMsgBoxDisplayer : virtual public IDisplayer
{
public:

	/// Constructor
	CMsgBoxDisplayer() {}

	/// Put the string into the file.
	virtual void display (const std::string& str);
};


};

#endif // NL_DISPLAYER_H

/* End of displayer.h */
