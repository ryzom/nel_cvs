/* net_displayer.h
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: net_displayer.h,v 1.6 2000/10/04 15:27:53 cado Exp $
 *
 * Interface for CNetDisplayer
 */

#ifndef NL_NET_DISPLAYER_H
#define NL_NET_DISPLAYER_H

#include "nel/misc/displayer.h"
#include "nel/net/socket.h"


namespace NLNET {


/**
 * Net Displayer. Sends the strings to a logger server.
 * \ref log_howto
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CNetDisplayer : public NLMISC::IDisplayer
{
public:

	/// Constructor
	CNetDisplayer( const CInetAddress& logServerAddr );

	/// Destructor
	virtual ~CNetDisplayer();
		
	/** Sends the string to the logging server
	 * \warning If not connected, tries to connect to the logging server each call. It can slow down your program a lot.
	 */
	virtual void display (const std::string& str);

private:

	CInetAddress	_ServerAddr;
	CSocket			_Server;
};


} // NLNET


#endif // NL_NET_DISPLAYER_H

/* End of net_displayer.h */
