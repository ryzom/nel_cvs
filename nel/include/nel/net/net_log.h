/** \file net_log.h
 * Implementation of the CLog (look at log.h) that display on a network connection (deprecated)
 *
 * $Id: net_log.h,v 1.7 2002/10/24 08:44:31 lecroart Exp $
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



/**************************************************************************
********************* THIS CLASS IS DEPRECATED ****************************
**************************************************************************/





#ifndef NL_NET_LOG_H
#define NL_NET_LOG_H

#include "nel/misc/types_nl.h"
#include "nel/misc/log.h"
#include <string>

namespace NLNET {


/**
 * Logger for network transfers
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CNetLog : public NLMISC::CLog
{
public:

	/// Constructor
	CNetLog();

	/// Sets the name of the running service
	void	setServiceName( const char *name );

	/// Log an output transfer (send)
	void	output( const char *srchost, uint8 msgnum, const char *desthost,
					const char *msgname, uint32 msgsize );

	/// Log an input transfer (receive)
	void	input( const char *srchost, uint8 msgnum, const char *desthost );

	/*/// Enables or disable logging.
	void	setLogging( bool logging )
	{
		_Logging = logging;
	}*/

private:

	std::string _ProcessId;

};


extern CNetLog NetLog;

#define nlsetnet (servicename,displayer)
#define nlnetoutput NetLog.output
#define nlnetinput NetLog.input


} // NLNET


#endif // NL_NET_LOG_H

/* End of net_log.h */
