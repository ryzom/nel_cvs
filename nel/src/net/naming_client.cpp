/** \file naming_client.cpp
 * CNamingClient
 *
 * $Id: naming_client.cpp,v 1.11 2000/11/27 10:07:07 cado Exp $
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

#include "nel/net/naming_client.h"
#include "nel/net/socket.h"
#include "nel/misc/debug.h"


using namespace NLMISC;

namespace NLNET {


CSocket *CNamingClient::_ClientSock;


/// Config file name
const char		*CNamingClient::NamingServiceAddrFile = "ns.cfg";

/// Default NS host name
const char		*CNamingClient::NamingServiceDefHost = "olivierc";

/// Default NS port
const uint16	CNamingClient::NamingServiceDefPort = 50000;


CConfigFile		*CNamingClient::_ConfigFile = NULL;
CInetAddress	CNamingClient::NamingServiceAddress;
CRegServices	CNamingClient::_RegisteredServices;
bool			CNamingClient::_TransactionMode = true;



/**@name These values must correspond to CallbackArray in the Naming Service.
 * They are used instead of their string equivalents to prevent the NS from sending back
 * binding values.
 */
//@{
const sint16 LK_CBINDEX = 0;
const sint16 LA_CBINDEX = 1;
const sint16 RG_CBINDEX = 2;
const sint16 UN_CBINDEX = 3;
const sint16 QP_CBINDEX = 4;
//@}


/*
 * Destructor. Calls close().
 */
CNamingClient::~CNamingClient()
{
	CNamingClient::finalize();
}


/*
 * Finalization. Unregisters all services registered by registerService() and not unregistered yet.
 */
void CNamingClient::finalize()
{
	// Unregisters all services
	while ( ! _RegisteredServices.empty() )
	{
		CRegServices::iterator irs = _RegisteredServices.begin();
		std::pair<std::string,CInetAddress> p = (*irs); // we duplicate it because we don't want to pass a reference, into unregisterService, to things that it will suppress
		unregisterService( p.first, p.second );
	}

	if ( _ConfigFile != NULL )
	{
		delete _ConfigFile;
		_ConfigFile = NULL;
	}
}


/*
 * Connection to the naming service
 */
void CNamingClient::open()
{
	CNamingClient::_TransactionMode = false;
	doOpen();
	nldebug( "Connected to the naming service" );
}


/*
 * Disconnection from the naming service
 */
void CNamingClient::close()
{
	doClose();
	nldebug( "Disconnected from the naming service" );
	CNamingClient::_TransactionMode = true;
}


/*
 * Call open is _TransactionMode is true
 */
void CNamingClient::openT()
{
	if ( CNamingClient::_TransactionMode )
	{
		CNamingClient::doOpen();
	}
}


/*
 * Call close if _TransactionMode is true
 */
void CNamingClient::closeT()
{
	if ( CNamingClient::_TransactionMode )
	{
		CNamingClient::doClose();
	}
}


/*
 * Callback for dynamic config file change (useless!)
 */
/*void cbNamingServiceAddrChanged()
{
	try
	{
		// Change the address
		CNamingClient::NamingServiceAddress.setByName( CNamingClient::_ConfigFile->getVar( "Host" ).asString() );
		CNamingClient::NamingServiceAddress.setPort( CNamingClient::_ConfigFile->getVar( "Port" ).asInt() );
		nlinfo( "Naming Service is now at %s", CNamingClient::NamingServiceAddress.asIPString().c_str() );
	}
	catch ( EConfigFile& )
	{
		// Do not change the address if the file has a problem
		nlinfo ( "New config file is invalid" );
	}
}*/


/*
 * Performs a socket connection
 */
void CNamingClient::doOpen()
{
	if ( _ConfigFile == NULL )
	{
		_ConfigFile = new CConfigFile();
		try
		{
			CBaseSocket::init(); // for setByName to work
			_ConfigFile->load( CNamingClient::NamingServiceAddrFile );
			CNamingClient::NamingServiceAddress.setByName( _ConfigFile->getVar( "Host" ).asString() );
		}
		catch ( EConfigFile& )
		{
			CNamingClient::NamingServiceAddress.setByName( CNamingClient::NamingServiceDefHost );
		}
		try
		{
			CNamingClient::NamingServiceAddress.setPort( _ConfigFile->getVar( "Port" ).asInt() );
		}
		catch ( EConfigFile& )
		{
			CNamingClient::NamingServiceAddress.setPort( CNamingClient::NamingServiceDefPort );
		}
		//_ConfigFile->setCallback( cbNamingServiceAddrChanged );
		nlinfo( "Naming Service is at %s", CNamingClient::NamingServiceAddress.asIPString().c_str() );
	}
	nldebug("Trying to connect to the Naming Service");
	CNamingClient::_ClientSock = new CSocket();
	CNamingClient::_ClientSock->connect( CNamingClient::NamingServiceAddress );
}


/*
 * Performs a socket disconnection
 */
void CNamingClient::doClose()
{
	if ( CNamingClient::_ClientSock != NULL )
	{
		delete CNamingClient::_ClientSock;
	}
}
	

/* Requests the naming service to choose a port for the service
 * \param name [in] Name of the service
 * \param addr [in] Address of the service (the port can be 0)
 * \return The allocated port number
 */
uint16 CNamingClient::queryServicePort( const std::string& name, const CInetAddress& addr )
{
	CNamingClient::openT();

	// Send request
	CMessage msgout( "" ); //"QP"
	msgout.setType( QP_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	CNamingClient::_ClientSock->send( msgout );

	// Wait for answer
	uint16 port = 0;
	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	msgin.serial( port );

	CNamingClient::closeT();
	nldebug( "Service %s got port %hu", name.c_str(), port );
	return port;
}


/*
 * Register a service within the naming service
 */
void CNamingClient::registerService( const std::string& name, const CInetAddress& addr )
{
	CNamingClient::openT();
	CMessage msgout( "" ); //"RG" );
	msgout.setType( RG_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	CNamingClient::_ClientSock->send( msgout );

	_RegisteredServices.insert( std::make_pair(name,addr) );

	CNamingClient::closeT();
	nldebug( "Registered service %s at %s", name.c_str(), addr.asIPString().c_str() );
}


/*
 * Unregister a service from the naming service
 */
void CNamingClient::unregisterService( const std::string& name, const CInetAddress& addr )
{
	CNamingClient::openT();
	CMessage msgout( "" ); //"UN" );
	msgout.setType( UN_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	CNamingClient::_ClientSock->send( msgout );

	_RegisteredServices.erase( name );

	CNamingClient::closeT();
	nldebug( "Unregistered service %s", name.c_str() );
}



/*
 * Returns true and the address of the specified service if it is found, otherwise returns false
 */
bool CNamingClient::lookup( const std::string& name, CInetAddress& addr, uint16& validitytime )
{
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up for service %s...", name.c_str() );
	CMessage msgout( "" ); // "LK" );
	msgout.setType( LK_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	CNamingClient::_ClientSock->send( msgout );

	// Wait for answer
	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	msgin.serial( validitytime );
	if ( validitytime == 0 )
	{
		nldebug( "Service %s not found", name.c_str() );
		CNamingClient::closeT();
		return false;
	}
	else
	{
		msgin.serial( addr );
		nldebug( "Service %s is at %s", name.c_str(), addr.asIPString().c_str() );
		CNamingClient::closeT();
		return true;
	}
}


/* Tells the Naming Service the specified address does not respond for the specified service
 * and returns true and another address for the service if available, otherwise returns false
 */
bool CNamingClient::lookupAlternate( const std::string& name, CInetAddress& addr, uint16& validitytime )
{
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up again for service %s...", name.c_str() );
	CMessage msgout( "" ); // "LA" );
	msgout.setType( LA_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( addr );
	CNamingClient::_ClientSock->send( msgout );

	// Wait for answer
	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	msgin.serial( validitytime );
	if ( validitytime == 0 )
	{
		nldebug( "Service %s not found", name.c_str() );
		CNamingClient::closeT();
		return false;
	}
	else
	{
		msgin.serial( addr );
		nldebug( "Service %s is at %s", name.c_str(), addr.asIPString().c_str() );
		CNamingClient::closeT();
		return true;
	}
}


} // NLNET
