/** \file naming_service.h
 * Naming Service (NS)
 *
 * $Id: naming_service.h,v 1.6 2001/11/27 17:31:28 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_NAMING_SERVICE_H
#define NL_NAMING_SERVICE_H

#include "nel/net/service.h"
#include "nel/net/inet_address.h"

//#include <map>
//#include <set>

/************
namespace NLNET
{
	class CMsgSocket;
}
*/

/**
 * Internet Addresses plus a reference counter and a service identifier
 */
/*class CInetAddressRef : public NLNET::CInetAddress
{
public:

	/// Constructor
	CInetAddressRef();

	/// Alt. constructor
	CInetAddressRef( const std::string& hostName, uint16 port=0, NLNET::TServiceId sid=0 );

	/// Copy constructor
	CInetAddressRef( const CInetAddressRef& other );

	/// Cast from CInetAddress
	CInetAddressRef( const NLNET::CInetAddress& other );

	/// Assignment operator
	CInetAddressRef&	operator=( const CInetAddressRef& other );

	uint32				refCounter() const				{ return _RefCounter; }
	void				incCounter()					{ _RefCounter++; }
	void				decCounter()					{ _RefCounter--; }

	NLNET::TServiceId	sId() const						{ return _SId; }
	void				setSId( NLNET::TServiceId sid )	{ _SId = sid; }

private:
	
	uint32				_RefCounter;
	NLNET::TServiceId	_SId;
};
*/

/// Naming multimap
//typedef std::multimap<std::string,CInetAddressRef> CNamingMap;

/// Address set
//typedef std::set<NLNET::CInetAddress> CAddressSet;

/// Service identifier map
//typedef std::map<NLNET::TServiceId,CNamingMap::iterator> CSIdMap;

/**
 * Naming Service (NS).
 * The client side is NLNET::CNamingClient.
 * See doc of .cpp for the protocol.
 *
 * \todo Cado: Use RefCount to prevent from flooding a particular service
 * \author Olivier Cado
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CNamingService : public NLNET::IService
{
public:

	/// Validity time in seconds
	static const uint16				ValidTime;

	/// Base SId for SId allocation
	static const NLNET::TServiceId	BaseSId;
	
	/// Base port number for port number allocation
	/// Ports begin at 51000 and finnish to 52000 (note: in this implementation there can be no more than 1000 services)
	static uint16					MinBasePort, MaxBasePort;

	void init ();
	
	bool update ();
};

#endif // NL_NAMING_SERVICE_H

/* End of naming_service.h */
