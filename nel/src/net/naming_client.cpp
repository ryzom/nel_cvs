/** \file naming_client.cpp
 * CNamingClient
 *
 * $Id: naming_client.cpp,v 1.40 2001/12/10 14:34:31 lecroart Exp $
 *
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
#include "nel/net/callback_client.h"

#include "nel/misc/command.h"
#include "nel/misc/debug.h"

#include <sstream>


using namespace std;
using namespace NLMISC;


namespace NLNET {

CCallbackClient *CNamingClient::_Connection = NULL;
CNamingClient::TRegServices CNamingClient::_RegisteredServices;

static TBroadcastCallback _RegistrationBroadcastCallback = NULL;
static TBroadcastCallback _UnregistrationBroadcastCallback = NULL;

uint	CNamingClient::_ThreadId = 0xFFFFFFFF;

std::list<CNamingClient::CServiceEntry>	CNamingClient::RegisteredServices;
NLMISC::CMutex CNamingClient::RegisteredServicesMutex("CNamingClient::RegisteredServicesMutex");

void CNamingClient::setRegistrationBroadcastCallback (TBroadcastCallback cb)
{
	_RegistrationBroadcastCallback = cb;	
}

void CNamingClient::setUnregistrationBroadcastCallback (TBroadcastCallback cb)
{
	_UnregistrationBroadcastCallback = cb;	
}

//

//

static bool Registered;
static TServiceId RegisteredSuccess;
static TServiceId RegisteredSID;

static void cbRegister (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	msgin.serial (RegisteredSuccess);
	if (RegisteredSuccess) msgin.serial (RegisteredSID);
	Registered = true;

	if (RegisteredSuccess)
		nlinfo("Received the answer of the registration (%hu)", (uint16)RegisteredSID);
	else
		nlinfo("Received the answer of the registration: can't register");
}

//

static bool QueryPort;
static uint16 QueryPortPort;

static void cbQueryPort (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	msgin.serial (QueryPortPort);
	QueryPort = true;

	nlinfo ("Received the answer of the query port (%hu)", QueryPortPort);
}

//

static bool FirstRegisteredBroadcast;

void cbRegisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	TServiceId size;
	string name;
	TServiceId sid;
	CInetAddress addr;

	msgin.serial (size);

	for (TServiceId i = 0; i < size; i++)
	{
		msgin.serial (name);
		msgin.serial (sid);
		msgin.serial (addr);

		// add it in the list

		CNamingClient::RegisteredServicesMutex.enter ();
		CNamingClient::RegisteredServices.push_back (CNamingClient::CServiceEntry (name, sid, addr));
		CNamingClient::RegisteredServicesMutex.leave ();

		nlinfo ("Registration Broadcast of the service %s-%hu '%s'", name.c_str(), (uint16)sid, addr.asString().c_str());

		if (_RegistrationBroadcastCallback != NULL)
			_RegistrationBroadcastCallback (name, sid, addr);
	}

	FirstRegisteredBroadcast = true;

	CNamingClient::displayRegisteredServices ();
}
	
//

void cbUnregisterBroadcast (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string name;
	TServiceId sid;
	CInetAddress addr;

	msgin.serial (name);
	msgin.serial (sid);
	msgin.serial (addr);

	// remove it in the list, if the service is not found, ignore it

	CNamingClient::RegisteredServicesMutex.enter ();
	for (std::list<CNamingClient::CServiceEntry>::iterator it = CNamingClient::RegisteredServices.begin(); it != CNamingClient::RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid)
		{
			// check the structure
			nlassert ((*it).Name == name);
			nlassert ((*it).Addr == addr);

			CNamingClient::RegisteredServices.erase (it);
			break;
		}
	}
	CNamingClient::RegisteredServicesMutex.leave ();

	nlinfo ("Unregistration Broadcast of the service %s-%hu", name.c_str(), (uint16)sid);

	if (_UnregistrationBroadcastCallback != NULL)
		_UnregistrationBroadcastCallback (name, sid, addr);

	CNamingClient::displayRegisteredServices ();
}
	
//

static TCallbackItem NamingClientCallbackArray[] =
{
	{ "RG", cbRegister },
	{ "QP", cbQueryPort },
//	{ "LK", cbLookup },

	{ "RGB", cbRegisterBroadcast },
	{ "UNB", cbUnregisterBroadcast },
};

void CNamingClient::connect( const CInetAddress &addr, CCallbackNetBase::TRecordingState rec )
{
	nlassert (_Connection == NULL || _Connection != NULL && !_Connection->connected ());
	_ThreadId = getThreadId ();

	if (_Connection == NULL)
	{
		_Connection = new CCallbackClient( rec, "naming_client.nmr" );
		_Connection->addCallbackArray (NamingClientCallbackArray, sizeof (NamingClientCallbackArray) / sizeof (NamingClientCallbackArray[0]));
	}

	_Connection->connect (addr);

	// wait the message that contains all already connected services

	FirstRegisteredBroadcast = false;
	while (!FirstRegisteredBroadcast && _Connection->connected ())
	{
		_Connection->update (-1);
	}
}


void CNamingClient::disconnect ()
{
	checkThreadId ();
	
	if (_Connection != NULL)
	{
		if (_Connection->connected ())
		{
			_Connection->disconnect ();
		}
		delete _Connection;
		_Connection = NULL;
	}

	_RegisteredServices.clear ();

	// we don't call unregisterAllServices because when the naming service will see the disconnection,
	// it'll automatically unregister all services registered by this client.
}

TServiceId CNamingClient::registerService (const std::string &name, const CInetAddress &addr)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "RG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serial (const_cast<CInetAddress&>(addr));
	TServiceId sid = 0;
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer of the naming service "RG"
	Registered = false;
	while (!Registered)
		_Connection->update ();

	if (RegisteredSuccess)
	{
		_RegisteredServices.insert (make_pair (RegisteredSID, name));
		nldebug ("NC: Registered service %s-%hu at %s", name.c_str(), (uint16)RegisteredSID, addr.asString().c_str());
	}
	else
	{
		nlerror ("NC: Naming service refused to register service %s at %s", name.c_str(), addr.asString().c_str());
	}

	return RegisteredSID;
}

bool CNamingClient::registerServiceWithSId (const std::string &name, const CInetAddress &addr, TServiceId sid)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "RG");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serial (const_cast<CInetAddress&>(addr));
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer of the naming service "RGI"
	Registered = false;
	while (!Registered)
		_Connection->update ();

	if (RegisteredSuccess)
	{
		_RegisteredServices.insert (make_pair (RegisteredSID, name));
		nldebug ("NC: Registered service with sid %s-%hu at %s", name.c_str(), (uint16)RegisteredSID, addr.asString().c_str());
	}
	else
	{
		nlerror ("NC: Naming service refused to register service with sid %s at %s", name.c_str(), addr.asString().c_str());
	}

	return RegisteredSuccess == 1;
/*
	nldebug("bool CNamingClient::registerServiceWithSId( const std::string& name, const CInetAddress& addr, TServiceId sid )");

	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	bool ok;
	msgin.serial( ok );

	//CNamingClient::closeT();

	if ( ok )
	{
		_RegisteredServices.insert( std::make_pair(sid,name) );
		nldebug( "Registered service %s-%hu at %s", name.c_str(), (uint16)sid, addr.asString().c_str() );
		return true;
	}
	else
	{
		nldebug( "Cannot register service %s-%hu: service identifier unavailable", name.c_str(), (uint16)sid );
		return false;
	}
	return false;
*/}


void CNamingClient::unregisterService (TServiceId sid)
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "UNI");
	msgout.serial (sid);
	_Connection->send (msgout);

	nldebug ("NC: Unregistering service %s-%hu", _RegisteredServices[sid].c_str(), sid);
	_RegisteredServices.erase (sid);
}

void CNamingClient::unregisterAllServices ()
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	while (!_RegisteredServices.empty())
	{
		TRegServices::iterator irs = _RegisteredServices.begin();
		TServiceId sid = (*irs).first;
		unregisterService (sid);
	}
}

uint16 CNamingClient::queryServicePort ()
{
	checkThreadId ();
	nlassert (_Connection != NULL && _Connection->connected ());

	CMessage msgout (_Connection->getSIDA(), "QP");
	_Connection->send (msgout);

	// wait the answer of the naming service "QP"
	QueryPort = false;
	while (!QueryPort)
		_Connection->update ();

	nldebug ("NC: Got port %hu", QueryPortPort);

	return QueryPortPort;
}

/*
 * Helper function for lookup() and loopupAlternate() and lookupAll
 */
/*void CNamingClient::doReceiveLookupAnswer (const std::string &name, std::vector<CInetAddress> &addrs)
{
	// wait the answer of the naming service "LK"
	Lookup = false;
	while (!Lookup)
	{
		_Connection->update ();
		nlSleep( 1 );
	}

	nldebug ("NC: Got lookup answer");

	addrs = LookupAddrs;

	if (addrs.size()==0)
		nldebug ("NC: Service %s is not found", name.c_str());
	else if (addrs.size()==1)
		nldebug ("NC: Service %s is at %s", name.c_str(), addrs[0].asString().c_str());
	else
		nldebug ("NC: %d Services %s found", addrs.size(), name.c_str());
}*/

bool CNamingClient::lookup (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	vector<CInetAddress> addrs;
	find (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

/*	
bool CNamingClient::lookup (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// send the query to naming_service
	CMessage msgout (_Connection->getSIDA(), "LK");
	msgout.serial (const_cast<std::string&>(name));
	_Connection->send (msgout);

	// wait the answer
	vector<CInetAddress> addrs;
	doReceiveLookupAnswer (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}
*/


bool CNamingClient::lookup (TServiceId sid, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	vector<CInetAddress> addrs;
	find (sid, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];
	
	return true;
}

/*
bool CNamingClient::lookup (TServiceId sid, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// send the query to naming_service
	CMessage msgout (_Connection->getSIDA(), "LKI");
	msgout.serial (sid);
	_Connection->send (msgout);

	// wait the answer
	vector<CInetAddress> addrs;
	doReceiveLookupAnswer (toString(sid), addrs);

if (addrs.size()==0) return false;

nlassert (addrs.size()==1);
	addr = addrs[0];
	
	return true;
}
*/

/// \todo ace: now the lookupAlternate doesn't say to the naming service that this addr is bad so the NS can't remove it from his list. find a solution
bool CNamingClient::lookupAlternate (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// remove it from his local list
	
	RegisteredServicesMutex.enter ();
	for (std::list<CServiceEntry>::iterator it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).Addr == addr)
		{
			RegisteredServices.erase (it);
			break;
		}
	}
	RegisteredServicesMutex.leave ();

	vector<CInetAddress> addrs;
	find (name, addrs);

	if (addrs.size()==0)
		return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];

	return true;
}

/*
bool CNamingClient::lookupAlternate (const std::string &name, CInetAddress &addr)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// send the query to naming_service
	CMessage msgout (_Connection->getSIDA(), "LA");
	msgout.serial (const_cast<std::string&>(name));
	msgout.serial (addr);
	_Connection->send (msgout);

	// wait the answer
	vector<CInetAddress> addrs;
	doReceiveLookupAnswer (name, addrs);

	if (addrs.size()==0) return false;

	nlassert (addrs.size()==1);
	addr = addrs[0];
	return true;
}
*/

void CNamingClient::lookupAll (const std::string &name, std::vector<CInetAddress> &addrs)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	find (name, addrs);
}

/*
void CNamingClient::lookupAll (const std::string &name, std::vector<CInetAddress> &addrs)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// send the query to naming_service
	CMessage msgout (_Connection->getSIDA(), "LKA");
	msgout.serial (const_cast<std::string&>(name));
	_Connection->send (msgout);

	// wait the answer
	doReceiveLookupAnswer (name, addrs);
}
*/

bool CNamingClient::lookupAndConnect (const std::string &name, CCallbackClient &sock)
{
	nlassert (_Connection != NULL && _Connection->connected ());

	// look up for service
	CInetAddress servaddr;
	
	// if service not found, return false
	if (!CNamingClient::lookup (name, servaddr))
		return false;

	do
	{
		try
		{
			// try to connect to the server
			sock.connect (servaddr);

			// connection succeeded
			return true;
		}
		catch (ESocketConnectionFailed &e)
		{
			nldebug( "Connection to %s failed: %s, tring another service if available", servaddr.asString().c_str(), e.what() );

			// try another server and if service is not found, return false
			if (!CNamingClient::lookupAlternate (name, servaddr))
				return false;
		}
	}
	while (true);
}



void CNamingClient::update ()
{
	checkThreadId ();
	// get message for naming service (new registration for example)
	if (_Connection != NULL && _Connection->connected ())
		_Connection->update ();
}

void CNamingClient::checkThreadId ()
{
	if (getThreadId () != _ThreadId)
	{
		nlerror ("You try to access to the CNamingClient with 2 differents thread (%d and %d)", _ThreadId, getThreadId());
	}
}

NLMISC_COMMAND(services, "displays registered services", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Display the %d registered services :", CNamingClient::getRegisteredServices().size());
	for (std::list<CNamingClient::CServiceEntry>::const_iterator it = CNamingClient::getRegisteredServices().begin(); it != CNamingClient::getRegisteredServices().end(); it++)
	{
		log.displayNL (" > %s-%hu '%s'", (*it).Name.c_str(), (uint16)(*it).SId, (*it).Addr.asString().c_str());
	}
	log.displayNL ("End ot the list");

	return true;
}


////////////////////////////////////////
////////////////////////////////////////
///////// OLD NAMING CLIENT ////////////
////////////////////////////////////////
////////////////////////////////////////



/*
CTcpSock *CNamingClient::_ClientSock;


/// Config file name
const char		*CNamingClient::NamingServiceAddrFile = "ns.cfg";

/// Default NS host name
const char		*CNamingClient::NamingServiceDefHost = "itsalive.nevrax.org";

/// Default NS port
const uint16	CNamingClient::NamingServiceDefPort = 50000;


CConfigFile		*CNamingClient::_ConfigFile = NULL;
CInetAddress	CNamingClient::NamingServiceAddress;
CRegServices	CNamingClient::_RegisteredServices;
*/
//bool			CNamingClient::_TransactionMode = true;



/**@name These values must correspond to CallbackArray in the Naming Service.
 * They are used instead of their string equivalents to prevent the NS from sending back
 * binding values.
 */
//@{
/*const sint16 LK_CBINDEX = 0;
const sint16 LA_CBINDEX = 1;
const sint16 RG_CBINDEX = 2;
const sint16 UN_CBINDEX = 3;

const sint16 QP_CBINDEX = 4;

const sint16 LKI_CBINDEX = 5;
const sint16 LAI_CBINDEX = 6;
const sint16 RGI_CBINDEX = 7;
const sint16 UNI_CBINDEX = 8;

const sint16 LKA_CBINDEX = 9;
const sint16 LKS_CBINDEX = 10;
*///@}

/*
/// Returns the string corresponding to a service identifier
string sIdToString( TServiceId sid )
{
	stringstream ss;
	ss << sid;
	return ss.str();
}
*/


/*
 * Finalization. Unregisters all services registered by registerService() and not unregistered yet.
 */
/*void CNamingClient::finalize()
{
	nldebug ("void CNamingClient::finalize()");
	// Unregisters all services
	while ( ! _RegisteredServices.empty() )
	{
		CRegServices::iterator irs = _RegisteredServices.begin();
		TServiceId sid = (*irs).first;
		unregisterService( sid );
	}

	if ( _ConfigFile != NULL )
	{
		delete _ConfigFile;
		_ConfigFile = NULL;
	}
}
*/

/*
 * Connection to the naming service
 */
/*void CNamingClient::open()
{
	nldebug("void CNamingClient::open()");
	CNamingClient::_TransactionMode = false;
	doOpen();
	nldebug( "Connected to the naming service" );
}
*/


/*
 * Disconnection from the naming service
 */
/*void CNamingClient::close()
{
	nldebug("void CNamingClient::close()");
	doClose();
	nldebug( "Disconnected from the naming service" );
	CNamingClient::_TransactionMode = true;
	
}*/


/*
 * Call open is _TransactionMode is true
 */
/*void CNamingClient::openT()
{
	nldebug("void CNamingClient::openT()");
	if ( CNamingClient::_TransactionMode )
	{
		CNamingClient::doOpen();
	}
}*/


/*
 * Call close if _TransactionMode is true
 */
/*void CNamingClient::closeT()
{
	nldebug("void CNamingClient::closeT()");
	if ( CNamingClient::_TransactionMode )
	{
		CNamingClient::doClose();
	}
}*/


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
		nlinfo( "Naming Service is now at %s", CNamingClient::NamingServiceAddress.asString().c_str() );
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
/*void CNamingClient::doOpen()
{
	nldebug("void CNamingClient::doOpen()");
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
//		nldebug( "Naming Service is at %s", CNamingClient::NamingServiceAddress.asString().c_str() );
	}
	
	nldebug("Trying to connect to the Naming Service");
	CNamingClient::_ClientSock = new CSocket();
	CNamingClient::_ClientSock->connect( CNamingClient::NamingServiceAddress );
	
}*/


/*
 * Performs a socket disconnection
 */
/*void CNamingClient::doClose()
{
	nldebug("void CNamingClient::doClose()");
	if ( CNamingClient::_ClientSock != NULL )
	{
		delete CNamingClient::_ClientSock;
	}
}*/
	

/* Requests the naming service to choose a port for the service
 * \param name [in] Name of the service
 * \param addr [in] Address of the service (the port can be 0)
 * \return The allocated port number
 */
/*uint16 CNamingClient::queryServicePort( const std::string& name, const CInetAddress& addr )
{
	nldebug("uint16 CNamingClient::queryServicePort( const std::string& name, const CInetAddress& addr )");
	CNamingClient::openT();

	// Send request
	CMessage msgout( "" ); //"QP"
	msgout.setType( QP_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	CNamingClient::_ClientSock->send( msgout );

	// Wait for answer (warning: it can receive a bad answer if another request has been done before and not processed)
	uint16 port = 0;
	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	msgin.serial( port );

	CNamingClient::closeT();
	nldebug( "Service %s got port %hu", name.c_str(), port );
	return port;
	return 0;
}
*/

/* Register a service within the naming service.
 * Returns the service identifier assigned by the NS (or 0 if it failed)
 */
/*TServiceId CNamingClient::registerService( const std::string& name, const CInetAddress& addr )
{
	nldebug("TServiceId CNamingClient::registerService( const std::string& name, const CInetAddress& addr )");
	//CNamingClient::openT();
	if ( CNamingClient::_TransactionMode )
	{
		nlerror( "Cannot (un)register a service in transaction mode. See CNamingClient doc" );
	}

	CMessage msgout( "" ); //"RG" );
	msgout.setType( RG_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	CNamingClient::_ClientSock->send( msgout );

	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	TServiceId sid;
	msgin.serial( sid );

	_RegisteredServices.insert( std::make_pair(sid,name) );

	//CNamingClient::closeT();
	nldebug( "Registered service %s-%hu at %s", name.c_str(), (uint16)sid, addr.asString().c_str() );

	return sid;
}
*/

/* Register a service within the naming service, using a specified service identifier.
 * Returns false if the service identifier is unavailable i.e. the registration failed.
 */
/*bool CNamingClient::registerServiceWithSId( const std::string& name, const CInetAddress& addr, TServiceId sid )
{
	nldebug("bool CNamingClient::registerServiceWithSId( const std::string& name, const CInetAddress& addr, TServiceId sid )");
	//CNamingClient::openT();
	if ( CNamingClient::_TransactionMode )
	{
		nlerror( "Cannot (un)register a service in transaction mode. See CNamingClient doc" );
	}

	CMessage msgout( "" ); //"RGI" );
	msgout.setType( RGI_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	msgout.serial( sid );
	CNamingClient::_ClientSock->send( msgout );

	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	bool ok;
	msgin.serial( ok );

	//CNamingClient::closeT();

	if ( ok )
	{
		_RegisteredServices.insert( std::make_pair(sid,name) );
		nldebug( "Registered service %s-%hu at %s", name.c_str(), (uint16)sid, addr.asString().c_str() );
		return true;
	}
	else
	{
		nldebug( "Cannot register service %s-%hu: service identifier unavailable", name.c_str(), (uint16)sid );
		return false;
	}
	return false;
}*/


/*
 * Unregister a service from the naming service, by name & address (*deprecated*)
 */
/*void CNamingClient::unregisterService( const std::string& name, const CInetAddress& addr )
{
	//CNamingClient::openT();
	if ( CNamingClient::_TransactionMode )
	{
		nlerror( "CNamingClient: Do not (un)register a service in transaction mode, use open() instead and close() at the end of the program (see IService::main())" );
	}

	CMessage msgout( "" ); //"UN" );
	msgout.setType( UN_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( const_cast<CInetAddress&>(addr) );
	CNamingClient::_ClientSock->send( msgout );

	_RegisteredServices.erase( name );

	//CNamingClient::closeT();
	nldebug( "Unregistered service %s", name.c_str() );
}*/


/*
 * Unregister a service from the naming service, by name & service identifier
 */
/*void CNamingClient::unregisterService( TServiceId sid )
{
	nldebug("void CNamingClient::unregisterService( TServiceId sid )");
	//CNamingClient::openT();
	if ( CNamingClient::_TransactionMode )
	{
		nlerror( "Cannot (un)register a service in transaction mode. See CNamingClient doc" );
	}

	CMessage msgout( "" ); //"UNI" );
	msgout.setType( UNI_CBINDEX );
	msgout.serial( sid );
	CNamingClient::_ClientSock->send( msgout );

	nldebug( "Unregistering service %s-%hu", _RegisteredServices[sid].c_str(), sid );
	_RegisteredServices.erase( sid );

	//CNamingClient::closeT();
	
}
*/

/*
 * Helper function for lookup() and loopupAlternate()
 */
/*bool CNamingClient::doReceiveLookupAnswer( const std::string& name, CInetAddress& addr, uint16& validitytime )
{
	nldebug("bool CNamingClient::doReceiveLookupAnswer( const std::string& name, CInetAddress& addr, uint16& validitytime )");
	// Wait for answer (warning: it can receive a bad answer if another request has been done before and not processed)
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
		nldebug( "Service %s is at %s", name.c_str(), addr.asString().c_str() );
		CNamingClient::closeT();
		return true;
	}
	
	return false;
}*/


/*
 * Returns true and the address of the specified service if it is found, otherwise returns false
 */
/*bool CNamingClient::lookup( const std::string& name, CInetAddress& addr, uint16& validitytime )
{
	nldebug("bool CNamingClient::lookup( const std::string& name, CInetAddress& addr, uint16& validitytime )");
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up for service %s...", name.c_str() );
	CMessage msgout( "" ); // "LK" );
	msgout.setType( LK_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	CNamingClient::_ClientSock->send( msgout );

	return doReceiveLookupAnswer( name, addr, validitytime );
	return false;
}*/


/*
 * Same as lookup(const string&, CInetAddress&, uint16&)
 */
/*bool CNamingClient::lookup( TServiceId sid, CInetAddress& addr, uint16& validitytime )
{
	nldebug("bool CNamingClient::lookup( TServiceId sid, CInetAddress& addr, uint16& validitytime )");
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up for service %hu...", (uint16)sid );
	CMessage msgout( "" ); // "LKI" );
	msgout.setType( LKI_CBINDEX );
	msgout.serial( sid );
	CNamingClient::_ClientSock->send( msgout );

	return doReceiveLookupAnswer( sIdToString(sid), addr, validitytime );
	return false;
}*/


/* Tells the Naming Service the specified address does not respond for the specified service
 * and returns true and another address for the service if available, otherwise returns false
 */
/*bool CNamingClient::lookupAlternate( const std::string& name, CInetAddress& addr, uint16& validitytime )
{
	nldebug("bool CNamingClient::lookupAlternate( const std::string& name, CInetAddress& addr, uint16& validitytime )");
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up again for service %s...", name.c_str() );
	CMessage msgout( "" ); // "LA" );
	msgout.setType( LA_CBINDEX );
	msgout.serial( const_cast<std::string&>(name) );
	msgout.serial( addr );
	CNamingClient::_ClientSock->send( msgout );

	return doReceiveLookupAnswer( name, addr, validitytime );
	return false;
}*/


/*
 * Same as lookupAlternate(const string&, CInetAddress&, uint16&)
 */
/*bool CNamingClient::lookupAlternate( TServiceId sid, CInetAddress& addr, uint16& validitytime )
{
	nldebug("bool CNamingClient::lookupAlternate( TServiceId sid, CInetAddress& addr, uint16& validitytime )");
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up again for service %hu...", (uint16)sid );
	CMessage msgout( "" ); // "LAI" );
	msgout.setType( LAI_CBINDEX );
	msgout.serial( sid );
	msgout.serial( addr );
	CNamingClient::_ClientSock->send( msgout );

	return doReceiveLookupAnswer( sIdToString(sid), addr, validitytime );
	return false;
}*/


/*
 * Returns all services corresponding to the specified name.
 * Ex: lookupAll( "AS", addresses );
 */
/*void CNamingClient::lookupAll( const std::string& name, std::vector<CInetAddress>& addresses )
{
	nldebug("void CNamingClient::lookupAll( const std::string& name, std::vector<CInetAddress>& addresses )");
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up all services %s (addresses only)", name.c_str() );
	CMessage msgout( "" ); // "LKA"
	msgout.setType( LKA_CBINDEX );
	msgout.serial( const_cast<string&>(name) );
	CNamingClient::_ClientSock->send( msgout );

	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	msgin.serialCont( addresses );
	
}*/


/*
 * Returns all services corresponding to the specified name with service id as key
 * Ex: lookupAll( "AS", addressmap );
 */
/*void CNamingClient::lookupAllServices( const std::string& name, std::map<TServiceId,CInetAddress>& addressmap )
{
	nldebug("void CNamingClient::lookupAllServices( const std::string& name, std::map<TServiceId,CInetAddress>& addressmap )");
	CNamingClient::openT();

	// Send request
	nldebug( "Looking-up all services %s", name.c_str() );
	CMessage msgout( "" ); // "LKS"
	msgout.setType( LKS_CBINDEX );
	msgout.serial( const_cast<string&>(name) );
	CNamingClient::_ClientSock->send( msgout );

	CMessage msgin( "", true );
	CNamingClient::_ClientSock->receive( msgin );
	msgin.serialCont( addressmap );
	
}*/


/*
 * Obtains a socket connected to a server providing the service \e name.
 */
/*bool CNamingClient::lookupAndConnect( const std::string& name, CTcpSock& sock, uint16& validitytime )
{
	nldebug("bool CNamingClient::lookupAndConnect( const std::string& name, CTcpSock& sock, uint16& validitytime )");
	// Look up for service
	CInetAddress servaddr;
	if ( CNamingClient::lookup( name, servaddr, validitytime ) )
	{
		// Try to connect to the server
		bool service_ok = false;
		while ( ! service_ok )
		{
			try
			{
				sock.connect( servaddr );
				service_ok = true;
			}
			catch ( ESocketConnectionFailed& e )
			{
				// If the connection failed, inform the Naming Service and try another server
				nldebug( "Connection to %s failed: %s", servaddr.asString().c_str(), e.what() );
				if ( ! CNamingClient::lookupAlternate( name, servaddr, validitytime ) )
				{
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		return false;
	}
	return false;
}*/


/*
 * See lookupAndConnect( const std::string&, CSocket&, uint16& )
 */
/*bool CNamingClient::lookupAndConnect( const std::string& name, CCallbackClient& sock, uint16& validitytime )
{
	nldebug("bool CNamingClient::lookupAndConnect( const std::string& name, CCallbackClient& sock, uint16& validitytime )");
	// Look up for service
	CInetAddress servaddr;
	if ( CNamingClient::lookup( name, servaddr, validitytime ) )
	{
		// Try to connect to the server
		bool service_ok = false;
		while ( ! service_ok )
		{
			try
			{
				sock.connect( servaddr );
				service_ok = true;
			}
			catch ( ESocketConnectionFailed& e )
			{
				// If the connection failed, inform the Naming Service and try another server
				nldebug( "Connection to %s failed: %s", servaddr.asString().c_str(), e.what() );
				if ( ! CNamingClient::lookupAlternate( name, servaddr, validitytime ) )
				{
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		return false;
	}
	return false;
}*/


} // NLNET
