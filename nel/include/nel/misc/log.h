/* log.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: log.h,v 1.6 2000/10/06 10:27:36 lecroart Exp $
 *
 * Interface for CLog
 */

#ifndef NL_LOG_H
#define NL_LOG_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>



namespace NLMISC
{

class IDisplayer;


typedef enum { LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_STAT, LOG_ASSERT } TLogPriority;

typedef std::vector<IDisplayer *> CDisplayers;


/**
 * When display() is called, the logger builds a string a sends it to its attached displayers.
 * \ref log_howto
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CLog
{
public:

	/// Constructor
	CLog( TLogPriority priority=LOG_DEBUG, bool longinfo=false );

	/// Add a new displayer in the log. You have to create the displayer, remove it and delete it when you have finnish with it.
	/// For example, in a 3dDisplayer, you can add the displayer when you want, and the displayer displayer the string if the 3d
	/// screen is available and do nothing otherwise. In this case, if you want, you could leave the displayer all the time.
	void addDisplayer (IDisplayer *displayer);

	/// Remove a displayer. If the displayer doesn't work, you could remove it.
	void removeDisplayer (IDisplayer *displayer);

	/// Sets longinfo (if true, display() will then write a bigger header)
	void setLongInfo( bool longinfo )
	{
		_Long = longinfo;
	}

	/// Sets line and file parameters
	void setParam( uint line, char *file )
	{
		_Line = line;
		_File = file;
	}

	/// Display a string in decorated and final new line form to all attached displayers. Call setParam before.
	void displayNL( const char *format, ... );

	/// Display a string in decorated form to all attached displayers. Call setParam before.
	void display( const char *format, ... );

	/// Display a string with a final new line to all attached displayers
	void displayRawNL( const char *format, ... );

	/// Display a string (and nothing more) to all attached displayers
	void displayRaw( const char *format, ... );

protected:

	std::string priorityStr() const;

private:

	TLogPriority				_Priority;
	uint						_Line;
	char						*_File;
	bool						_Long;
	CDisplayers					_Displayers;
};

} // NLMISC

#endif // NL_LOG_H

/* End of log.h */
