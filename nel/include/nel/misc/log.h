/** \file log.h
 * CLog class
 *
 * $Id: log.h,v 1.14 2001/01/23 17:35:36 cado Exp $
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

#ifndef NL_LOG_H
#define NL_LOG_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>



namespace NLMISC
{

class IDisplayer;


typedef enum { LOG_NO, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_STAT, LOG_ASSERT } TLogPriority;

typedef std::vector<IDisplayer *> CDisplayers;


/**
 * When display() is called, the logger builds a string a sends it to its attached displayers.
 * \ref log_howto
 * \todo cado display() and displayRaw() should save the string and send it only when displayRawNL()
 * (or a flush()-style method) is called.
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CLog
{
public:

	/// Constructor
	CLog( TLogPriority priority=LOG_NO, bool longinfo=false );

	/// Add a new displayer in the log. You have to create the displayer, remove it and delete it when you have finnish with it.
	/// For example, in a 3dDisplayer, you can add the displayer when you want, and the displayer displayer the string if the 3d
	/// screen is available and do nothing otherwise. In this case, if you want, you could leave the displayer all the time.
	void addDisplayer (IDisplayer *displayer);

	/// Remove a displayer. If the displayer doesn't work, you could remove it.
	void removeDisplayer (IDisplayer *displayer);

	/// Returns true if the specified displayer is attached to the log object
	bool attached(IDisplayer *displayer) const ;

	/// Sets the local host name, with has to be determined outside
	static void setLocalHostAndService( const std::string& hostname, const std::string& servicename );

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

	std::string					priorityStr() const;

	static std::string			*_LocalHostAndService;
	
private:

	TLogPriority				_Priority;
	bool						_Long;

	uint						_Line;
	char						*_File;

	CDisplayers					_Displayers;
};

} // NLMISC

#endif // NL_LOG_H

/* End of log.h */
