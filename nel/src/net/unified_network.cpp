/** \file unified_network.cpp
 * Network engine, layer 5 with no multithread support
 *
 * $Id: unified_network.cpp,v 1.47 2002/08/23 07:58:05 lecroart Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#include "stdnet.h"

#include "nel/net/unified_network.h"

using namespace std;
using namespace NLMISC;


namespace NLNET {

CLog test(CLog::LOG_INFO);
CFileDisplayer fd;

static uint ThreadCreator = 0;

static const uint64 AppIdDeadConnection = 0xDEAD;

//#define AUTOCHECK_DISPLAY nlwarning
#define AUTOCHECK_DISPLAY CUnifiedNetwork::getInstance()->displayInternalTables (), nlerror

// ace retirer ca
static string allstuffs;

//
// Callbacks from NAMING SERVICE
//

// when a service registers
void	uNetRegistrationBroadcast(const string &name, TServiceId sid, const vector<CInetAddress> &addr)
{
	nldebug ("HNETL5: + naming %s-%hu '%s'", name.c_str(), (uint16)sid, vectorCInetAddressToString(addr).c_str ());

	allstuffs += "+naming "+name+"-"+toString(sid)+"\n";
	test.displayNL ("+naming %s-%hu", name.c_str (), (uint16)sid);

	CUnifiedNetwork *uni= CUnifiedNetwork::getInstance();

	if (uni->_SId == sid)
	{
		allstuffs += "itsme!!!\n";
		test.displayNL ("itsme!!!");
		// it's me! don't add me!!!
		return;
	}

	// add the unified connection

	if(sid >= uni->_IdCnx.size ())
		uni->_IdCnx.resize (sid+1);

	if (uni->_IdCnx[sid].State == CUnifiedNetwork::CUnifiedConnection::NotUsed)
	{
		uni->_IdCnx[sid] = CUnifiedNetwork::CUnifiedConnection(name, sid);
		uni->_UsedConnection.push_back (sid);
	}

	if (!uni->_IdCnx[sid].ExtAddress.empty ()) AUTOCHECK_DISPLAY ("HNETL5: %s-%hu already inserted in the table with '%s'", name.c_str(), (uint16)sid, vectorCInetAddressToString (uni->_IdCnx[sid].ExtAddress).c_str ());


	// set the list of external addresses

	nlassert (!addr.empty());

	uni->_IdCnx[sid].ExtAddress = addr;

	// associate nid with ext address
	uni->_IdCnx[sid].setupNetworkAssociation (uni->_NetworkAssociations, uni->_DefaultNetwork);
}

// when a service unregisters
void	uNetUnregistrationBroadcast(const string &name, TServiceId sid, const vector<CInetAddress> &addr)
{
	nldebug ("HNETL5: - naming %s-%hu '%s'", name.c_str(), (uint16)sid, vectorCInetAddressToString (addr).c_str ());

	allstuffs += "-naming "+name+"-"+toString(sid)+"\n";
	test.displayNL ("-naming %s-%hu", name.c_str (), (uint16)sid);

	// get the service connection
	CUnifiedNetwork *uni = CUnifiedNetwork::getInstance();

	CUnifiedNetwork::CUnifiedConnection *uc = uni->getUnifiedConnection (sid);
	if (uc == 0) return;	// should never happen, the getUnifiedConnection() will generate a AUTOCHECK_DISPLAY

	// call the user callback
	CUnifiedNetwork::TNameMappedCallback::iterator	it2 = uni->_DownCallbacks.find(uc->ServiceName);

	if (it2 != uni->_DownCallbacks.end())
	{
		// call it
		TUnifiedNetCallback	cb = (*it2).second.first;
		cb(uc->ServiceName, uc->ServiceId, (*it2).second.second);
	}

	for (uint c = 0; c < uni->_DownUniCallback.size (); c++)
	{
		if (uni->_DownUniCallback[c].first != NULL)
			uni->_DownUniCallback[c].first(uc->ServiceName, uc->ServiceId, uni->_DownUniCallback[c].second);
	}

	if(!uc->Connection.empty ())
	{
		// set all connection to dead, now, all messages received on this socket will be ignored and closed
		for (uint i = 0; i < uc->Connection.size (); ++i)
		{
			if (uc->Connection[i].valid())
				uc->Connection[i].setAppId (AppIdDeadConnection);
		}

		//
		// It's the first connection that added the _NamedCnx so if there s no connection, no need to
		// remove entry in _NamedCnx
		//

		uni->removeNamedCnx (uc->ServiceName, uc->ServiceId);
	}

	// remove the _UsedConnection
	bool found = false;
	for (vector<uint16>::iterator it = uni->_UsedConnection.begin (); it != uni->_UsedConnection.end(); it++)
	{
		if (*it == uc->ServiceId)
		{
			found = true;
			uni->_UsedConnection.erase (it);
			break;
		}
	}
	if (!found) AUTOCHECK_DISPLAY ("HNETL5: can't find the sid %hu in the _UsedConnection", uc->ServiceId);

	// reset the unified connection
	uc->reset ();
}


//
// Callbacks from connection/disconnection services
//

void	uncbConnection(TSockId from, void *arg)
{
	nlinfo ("HNETL5: + connec '%s'", from->asString().c_str());

	from->setAppId (AppIdDeadConnection);
}

void	uncbDisconnection(TSockId from, void *arg)
{
	if(from->appId () == AppIdDeadConnection)
	{
		nlinfo ("HNETL5: - connec '%s'", from->asString().c_str());
			test.displayNL ("-connect dead conenction");
	}
	else
	{
		CUnifiedNetwork	*uni = CUnifiedNetwork::getInstance();
		uint16			sid = (uint16)from->appId();
		CUnifiedNetwork::CUnifiedConnection *uc = uni->getUnifiedConnection (sid);
		if (uc == 0)
		{
			nlinfo ("HNETL5: - connec '%s' sid %hu", from->asString().c_str(), sid);
			test.displayNL ("-connect '%s' %hu", from->asString ().c_str (), sid);
		}
		else
		{
			nlinfo ("HNETL5: - connec '%s' %s-%hu", from->asString().c_str(), uc->ServiceName.c_str (), sid);
			allstuffs += "-connect "+uc->ServiceName+"-"+toString(sid)+"\n";
			test.displayNL ("-connect %s-%hu", uc->ServiceName.c_str (), (uint16)(uc->ServiceId));

			if (uc->IsExternal && !uc->AutoRetry)
			{
				// If it s a external service with no auto retry, remove the connection

				// remove the _UsedConnection
				bool found = false;
				for (vector<uint16>::iterator it = uni->_UsedConnection.begin (); it != uni->_UsedConnection.end(); it++)
				{
					if (*it == uc->ServiceId)
					{
						found = true;
						uni->_UsedConnection.erase (it);
						break;
					}
				}
				if (!found) AUTOCHECK_DISPLAY ("HNETL5: can't find the sid %hu in the _UsedConnection", uc->ServiceId);
				
				uc->reset ();
			}
			else if (!uc->IsExternal)
			{
				// reset the connection
				uint i;
				for (i = 0; i < uc->Connection.size (); i++)
				{
					if (uc->Connection[i].valid() && uc->Connection[i].CbNetBase->getSockId(uc->Connection[i].HostId) == from)
					{
						if (uc->Connection[i].IsServerConnection)
						{
							// we have to remove the stuffs now because HostId will not be accessible later
							uc->Connection[i].reset();
						}
						else
						{
							// if it s a client, we can't delete now because the callback client is currently in use
							// only disconnect
							if(uc->Connection[i].CbNetBase->connected ())
							{
								uc->Connection[i].CbNetBase->disconnect (uc->Connection[i].HostId);
							}
						}
						break;
					}
				}
				if (i == uc->Connection.size ())
				{
					AUTOCHECK_DISPLAY ("HNETL5: received a disconnection from a service but the connection is not in my list!");
				}
			}
		}

		from->setAppId (AppIdDeadConnection);
	}
}

//
// Callback from identication services
//

void	uncbServiceIdentification(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string		inSName;
	uint16		inSid;

	if (from->appId () != AppIdDeadConnection)
		AUTOCHECK_DISPLAY ("HNETL5: received a connec ident from an unknown connection 0x%"NL_I64"X", from->appId ());

	// recover the service name and id
	msgin.serial(inSName);
	msgin.serial(inSid);
	uint8 pos;
	msgin.serial (pos);

	nlinfo ("HNETL5: + connec ident '%s' %s-%hu pos %hu", from->asString().c_str(), inSName.c_str(), inSid, (uint16)pos);

	allstuffs += "+rconnect "+inSName+"-"+toString(inSid)+" pos "+toString((uint16)pos)+"\n";
	test.displayNL ("+rconnect %s-%hu pos %hu", inSName.c_str (), (uint16)inSid, (uint16)pos);

	from->setAppId(inSid);

	// add a new connection to the list
	CUnifiedNetwork		*uni= CUnifiedNetwork::getInstance();

	if(inSid >= uni->_IdCnx.size ())
	{
		uni->_IdCnx.resize (inSid+1);
	}

	switch(uni->_IdCnx[inSid].State)
	{
	case CUnifiedNetwork::CUnifiedConnection::NotUsed:		// add the new unified connection
		uni->_IdCnx[inSid] = CUnifiedNetwork::CUnifiedConnection(inSName, inSid);
		uni->_UsedConnection.push_back (inSid);
		break;
	default:
		break;
	}

	// add the connection to the already inserted unified connection
	if (pos >= uni->_IdCnx[inSid].Connection.size ())
		uni->_IdCnx[inSid].Connection.resize(pos+1);
	uni->_IdCnx[inSid].Connection[pos] = CUnifiedNetwork::CUnifiedConnection::TConnection(&netbase, from);


	// ace temp to savoir comment c est possible ce cas la
	if (uni->_IdCnx[inSid].Connection.size() == 3)
	{
		CUnifiedNetwork::CUnifiedConnection *uc = &uni->_IdCnx[inSid];
		nlstop;
		nlinfo ("ext addr %s", vectorCInetAddressToString (uc->ExtAddress).c_str ());
		for(uint i = 0; i < uc->Connection.size(); i++)
			nlinfo ("cnx %s", uc->Connection[i].HostId->asString ().c_str ());
		nlinfo ("%s", allstuffs.c_str ());
	}

	// send the callback to the user with the first connection
	if (uni->_IdCnx[inSid].Connection.size () == 1)
	{
		// insert the name in the map to be able to send message with the name
		uni->addNamedCnx (inSName, inSid);

		// now we warn the user
		CUnifiedNetwork::TNameMappedCallback::iterator	it = uni->_UpCallbacks.find(inSName);
		if (it != uni->_UpCallbacks.end())
		{
			// call it
			TUnifiedNetCallback	cb = (*it).second.first;
			cb(inSName, inSid, (*it).second.second);
		}

		for (uint c = 0; c < uni->_UpUniCallback.size (); c++)
		{
			if (uni->_UpUniCallback[c].first != NULL)
				uni->_UpUniCallback[c].first (inSName, inSid, uni->_UpUniCallback[c].second);
		}
	}
}

// the callbacks wrapper
void	uncbMsgProcessing(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	if (from->appId() == AppIdDeadConnection)
	{
		AUTOCHECK_DISPLAY ("HNETL5: Receive a message from a dead connection, ignoring it");
		return;
	}

	CUnifiedNetwork									*uni = CUnifiedNetwork::getInstance();
	uint16											sid = (uint16)from->appId();
	CUnifiedNetwork::TMsgMappedCallback::iterator	itcb;

	itcb = uni->_Callbacks.find(msgin.getName());
	if (itcb == uni->_Callbacks.end())
	{
		// the callback doesn't exist
		nlwarning ("HNETL5: Can't find callback '%s' called by service %hu", msgin.getName().c_str(), sid);
	}
	else
	{
		CUnifiedNetwork::CUnifiedConnection *uc = uni->getUnifiedConnection (sid);

		if (uc == 0)
		{
			nlwarning ("HNETL5: Received a message from a service %hu that is not ready (bad appid? 0x%"NL_I64"X)", sid, from->appId ());
			return;
		}
		(*itcb).second (msgin, uc->ServiceName, sid);
	}
}


TCallbackItem	unServerCbArray[] =
{
	{ "UN_SIDENT", uncbServiceIdentification }
};


//
//
//

void	CUnifiedNetwork::init(const CInetAddress *addr, CCallbackNetBase::TRecordingState rec,
							  const string &shortName, uint16 port, TServiceId &sid)
{
	//DebugLog->addNegativeFilter ("HNETL5");

	if (_Initialised)
	{
		AUTOCHECK_DISPLAY ("HNETL5: Unified network layer already initialized");
		return;
	}

	ThreadCreator = NLMISC::getThreadId();

	vector<CInetAddress> laddr = CInetAddress::localAddresses();

	_RecordingState = rec;
	_Name = shortName;
	_SId = sid;
	
	if (addr != 0)
		_NamingServiceAddr = *addr;

	// if the address isn't 0, uses the naming service
	if (_NamingServiceAddr.isValid ())
	{
		// connect the callback to know when a new service comes in or goes down
		CNamingClient::setRegistrationBroadcastCallback(uNetRegistrationBroadcast);
		CNamingClient::setUnregistrationBroadcastCallback(uNetUnregistrationBroadcast);

		// connect to the naming service (may generate a ESocketConnectionFailed exception)
		CNamingClient::connect(_NamingServiceAddr, _RecordingState, laddr);

		if (port == 0)
			port = CNamingClient::queryServicePort ();
	}

	// setup the server callback
	_ServerPort = port;

	nlassert (_CbServer == NULL);
	_CbServer = new CCallbackServer;
	_CbServer->init(port);
	_CbServer->addCallbackArray(unServerCbArray, 1);				// the service ident callback
	_CbServer->setDefaultCallback(uncbMsgProcessing);				// the default callback wrapper
	_CbServer->setConnectionCallback(uncbConnection, NULL);
	_CbServer->setDisconnectionCallback(uncbDisconnection, NULL);

	if (CNamingClient::connected())
	{
		// register the service
		for (uint i = 0; i < laddr.size(); i++)
			laddr[i].setPort(_ServerPort);

		if (_SId == 0)
		{
			CNamingClient::registerService(_Name, laddr, _SId);
		}
		else
		{
			CNamingClient::registerServiceWithSId(_Name, laddr, _SId);
		}

		sid = _SId;
	}

	string fn = _Name+"_"+toString(_SId)+".log";
	fd.setParam (fn);
	test.addDisplayer (&fd);
	test.displayNL ("**************INIT***************");

	nlinfo ("HNETL5: Server '%s' added, registered and listen to port %hu", shortName.c_str (), port);

	_Initialised = true;
}

void	CUnifiedNetwork::connect()
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::connect() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());


	if (CNamingClient::connected())
	{
		// get the services list
		const list<CNamingClient::CServiceEntry>	&services = CNamingClient::getRegisteredServices();

		// connects to the registered services
		list<CNamingClient::CServiceEntry>::const_iterator	its;

		// don't connect to itself
		for (its = services.begin(); its != services.end(); ++its)
		{
			if (_SId != (*its).SId)
			{
				// add service with name, address, ident, not external, service id, and not autoretry (obsolete)
				// we put the last true because the name callback should already inserted it by uNetRegistrationBroadcast()
				addService((*its).Name, (*its).Addr, true, false, (*its).SId, false, true);
			}
			else
			{
				// don't process services received after mine because they'll connect to me
				break;
			}
		}
	}
}

void	CUnifiedNetwork::release()
{
	if (!_Initialised)
		return;

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	// disconnect all clients
	_CbServer->disconnect(InvalidSockId);
	delete _CbServer;
	_CbServer = 0;

	// disconnect all connections to servers
	for (uint i = 0; i<_IdCnx.size(); ++i)
	{
		if (_IdCnx[i].State != CUnifiedNetwork::CUnifiedConnection::NotUsed)
		{
			for(uint j = 0 ; j < _IdCnx[i].Connection.size (); j++)
			{
				if (_IdCnx[i].Connection[j].valid() && !_IdCnx[i].Connection[j].IsServerConnection)
				{
					if (_IdCnx[i].Connection[j].CbNetBase->connected ())
						_IdCnx[i].Connection[j].CbNetBase->disconnect();
				
					delete _IdCnx[i].Connection[j].CbNetBase;
				}
			}
			_IdCnx[i].Connection.clear ();
		}
	}

	// clear all other data
	_IdCnx.clear();
	_UsedConnection.clear ();
	_NamedCnx.clear();
	_UpCallbacks.clear();
	_DownCallbacks.clear();
	_Callbacks.clear();

	// disconnect the connection with the naming service
	if (CNamingClient::connected ())
		CNamingClient::disconnect ();
}

void	CUnifiedNetwork::addService(const string &name, const CInetAddress &addr, bool sendId, bool external, uint16 sid, bool autoRetry, bool shouldBeAlreayInserted)
{
	vector <CInetAddress> addrs;
	addrs.push_back (addr);
	addService (name, addrs, sendId, external, sid, autoRetry, shouldBeAlreayInserted);
}

void	CUnifiedNetwork::addService(const string &name, const vector<CInetAddress> &addr, bool sendId, bool external, uint16 sid, bool autoRetry, bool shouldBeAlreayInserted)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::addService() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	if (external)
		sid = _ExtSId++;

	nlinfo("HNETL5: addService %s-%hu '%s'", name.c_str(), sid, vectorCInetAddressToString(addr).c_str());

	allstuffs += "addService "+name+"-"+toString(sid)+"\n";
	test.displayNL ("+service %s-%hu", name.c_str (), (uint16)sid);

	if (external && addr.size () != 1)
	{
		AUTOCHECK_DISPLAY ("HNETL5: Can't add external service with more than one connection");
	}

	// add the entry in the unified connection table

	if (sid >= _IdCnx.size())
		_IdCnx.resize(sid+1);

	CUnifiedConnection	*uc = &_IdCnx[sid];

	// at this point it s possible that the service already added in the _IdCnx by the uNetRegistrationBroadcast()

	if (shouldBeAlreayInserted && _IdCnx[sid].State == CUnifiedNetwork::CUnifiedConnection::NotUsed)  AUTOCHECK_DISPLAY ("HNETL5: the unified connection should already set by the naming reg broadcast and is not (%hu)", sid);
	if (!shouldBeAlreayInserted && _IdCnx[sid].State == CUnifiedNetwork::CUnifiedConnection::Ready)  AUTOCHECK_DISPLAY ("HNETL5: the unified connection should not already set but is (%hu)", sid);

	if (_IdCnx[sid].State == CUnifiedNetwork::CUnifiedConnection::NotUsed)
	{
		*uc = CUnifiedConnection(name, sid);
		_UsedConnection.push_back (sid);
	}
	else
	{
		// If the entry already set, check that all is correct
		if (name != uc->ServiceName) AUTOCHECK_DISPLAY ("HNETL5: name are different in addService %s %s", name.c_str (), uc->ServiceName.c_str ());
		if (sid != uc->ServiceId) AUTOCHECK_DISPLAY ("HNETL5: sid are different in addService %hu %hu", sid, uc->ServiceId);
		if (addr != uc->ExtAddress) AUTOCHECK_DISPLAY ("HNETL5: external addr are different in addService '%s' '%s'", vectorCInetAddressToString(addr).c_str(), vectorCInetAddressToString(uc->ExtAddress).c_str ());
	}
	uc->IsExternal = external;
	uc->AutoRetry = autoRetry;
	uc->SendId = sendId;
	uc->ExtAddress = addr;
	nlassert (!addr.empty());

	// associate nid with ext address
	uc->setupNetworkAssociation (_NetworkAssociations, _DefaultNetwork);

	// connect to all connection
	bool	connectSuccess;
	
	for (uint i = 0; i < addr.size(); i++)
	{
		// create a new connection with the service, setup callback and connect
		CCallbackClient	*cbc = new CCallbackClient();
		cbc->setDisconnectionCallback(uncbDisconnection, NULL);
		cbc->setDefaultCallback(uncbMsgProcessing);
		cbc->getSockId()->setAppId(sid);

		try
		{
			cbc->connect(addr[i]);
			connectSuccess = true;
			
			allstuffs += "+lconnect "+name+"-"+toString(sid)+"\n";
			test.displayNL ("+lconnect %s-%hu", name.c_str (), (uint16)sid);
		}
		catch (ESocketConnectionFailed &e)
		{
			nlwarning ("HNETL5: can't connect to %s (sid %u) now (%s) '%s'", name.c_str(), sid, e.what (), addr[i].asString ().c_str());
			connectSuccess = false;

			allstuffs += "+lconnect failed "+name+"-"+toString((uint16)sid)+"\n";
			test.displayNL ("+lconnect failed %s-%hu", name.c_str (), (uint16)sid);
		}

		if (!connectSuccess && !autoRetry)
		{
			nlwarning ("HNETL5: Can't add service because no retry and can't connect");
			delete cbc;
		}
		else
		{
			uc->Connection.push_back (CUnifiedNetwork::CUnifiedConnection::TConnection(cbc));

			// temp to savoir comment c est possible ce cas la
			nlassert (uc->Connection.size()<3);

			nlinfo ("%s", allstuffs.c_str ());
		}

		if (connectSuccess && sendId)
		{
			// send identification to the service
			CMessage	msg("UN_SIDENT");
			uint16		ssid = _SId;
			msg.serial(_Name);
			msg.serial(ssid);	// serializes a 16 bits service id
			uint8 pos = i;
			msg.serial(pos);	// send the position in the connection table
			cbc->send (msg);
		}
	}

	if (addr.size () != uc->Connection.size())
	{
		nlwarning ("HNETL5: Can't connect to all connections to the service %d/%d", addr.size (), uc->Connection.size());
	}
	
	if (uc->Connection.size() > 0)
	{
		// add the name only if at least one connection is ok
		addNamedCnx (name, sid);

		// call the connection callback associated to this service
		TNameMappedCallback::iterator	itcb = _UpCallbacks.find(name);
		if (itcb != _UpCallbacks.end() && (*itcb).second.first != NULL)
		{
			TUnifiedNetCallback	cb = (*itcb).second.first;
			cb(name, sid, (*itcb).second.second);
		}

		if (!external)
		{
			for (uint i = 0; i < _UpUniCallback.size (); i++)
			{
				if (_UpUniCallback[i].first != NULL)
					_UpUniCallback[i].first (name, sid, _UpUniCallback[i].second);
			}
		}
	}

	nldebug ("HNETL5: addService was successful");
}
//
//
//

#define	TIME_BLOCK(tick, instr) \
{ \
	TTicks	_time_block_before = CTime::getPerformanceTime(); \
	instr ; \
	TTicks	_time_block_after = CTime::getPerformanceTime(); \
	tick += (_time_block_after - _before); \
}

void	CUnifiedNetwork::update(TTime timeout)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::update() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	bool	enableRetry;	// true every 5 seconds to reconnect if necessary

	// Compute the real timeout based on the next update timeout
	TTime t0 = CTime::getLocalTime ();

	if (timeout > 0)
	{
		if (_NextUpdateTime == 0)
		{
			_NextUpdateTime = t0 + timeout;
		}
		else
		{
			TTime err = t0 - _NextUpdateTime;
			_NextUpdateTime += timeout;

			// if we are too late, resync to the next value
			while (err > timeout)
			{
				err -= timeout;
				_NextUpdateTime += timeout;
			}
			
			timeout -= err;
			if (timeout < 0) timeout = 0;
		}
	}

	// check if we need to retry to connect to the client
	if ((enableRetry = (t0-_LastRetry > 5000)))
		_LastRetry = t0;

	// Try to reconnect to the naming service if connection lost
	if (_NamingServiceAddr.isValid ())
	{
		if (CNamingClient::connected ())
		{
			CNamingClient::update ();
		}
		else if (enableRetry)
		{
			try
			{
				vector<CInetAddress> laddr = CInetAddress::localAddresses();
				CNamingClient::connect (_NamingServiceAddr, _RecordingState, laddr);
				// re-register the service
				for (uint i = 0; i < laddr.size(); i++)
					laddr[i].setPort(_ServerPort);
				CNamingClient::resendRegisteration (_Name, laddr, _SId);
			}
			catch (ESocketConnectionFailed &)
			{
				nlwarning ("HNETL5: Could not connect to the Naming Service (%s). Retrying in a few seconds...", _NamingServiceAddr.asString().c_str());
			}
		}
	}

	while (true)
	{
		// update all server connections
		_CbServer->update(0);

		// update all client connections
		for (uint k = 0; k<_UsedConnection.size(); ++k)
		{
			CUnifiedConnection &uc = _IdCnx[_UsedConnection[k]];
			nlassert (uc.State == CUnifiedNetwork::CUnifiedConnection::Ready);
			for (uint j = 0; j < uc.Connection.size (); j++)
			{
				if (!uc.Connection[j].valid())
					continue;

				if (uc.Connection[j].IsServerConnection)
					continue;

				if (uc.Connection[j].CbNetBase->connected ())
				{
					uc.Connection[j].CbNetBase->update(0);
				}
				else if (enableRetry && uc.AutoRetry)
				{
					try
					{
						CCallbackClient *cbc = (CCallbackClient *)uc.Connection[j].CbNetBase;
						cbc->connect(uc.ExtAddress[j]);
						uc.Connection[j].CbNetBase->getSockId()->setAppId(uc.ServiceId);
						nlinfo ("HNETL5: reconnection to %s-%hu success", uc.ServiceName.c_str(), uc.ServiceId);

						// call the user callback

						CUnifiedNetwork::TNameMappedCallback::iterator	it = _UpCallbacks.find(uc.ServiceName);
						if (it != _UpCallbacks.end())
						{
							// call it
							TUnifiedNetCallback	cb = (*it).second.first;
							cb(uc.ServiceName, uc.ServiceId, (*it).second.second);
						}

						for (uint c = 0; c < _UpUniCallback.size (); c++)
						{
							if (_UpUniCallback[c].first != NULL)
								_UpUniCallback[c].first (uc.ServiceName, uc.ServiceId, _UpUniCallback[c].second);
						}

					}
					catch (ESocketConnectionFailed &e)
					{
						nlinfo ("HNETL5: can't connect to %s-%hu now (%s)", uc.ServiceName.c_str(), uc.ServiceId, e.what ());
					}
				}
			}
		}

		enableRetry = false;

		// If it's the end, don't nlSleep()
		if (CTime::getLocalTime() - t0 > timeout)
			break;
		
		// Enable windows multithreading before rescanning all connections
		nlSleep (1);
	}

	autoCheck();
}

//
//
//
uint8 CUnifiedNetwork::findConnectionId (uint32 sid, uint8 nid)
{
	if (_IdCnx[sid].Connection.size () == 0)
	{
		nlwarning ("HNETL5: Can't send message to %s because no connection are available", _IdCnx[sid].ServiceName.c_str ());
		return 0xFF;
	}

	// by default, connection id will be the default one
	uint8 connectionId = _IdCnx[sid].DefaultNetwork;

	if (nid == 0xFF)
	{
		nlinfo ("HNETL5: nid %hu, will use the default connection %hu", (uint16)nid, (uint16)connectionId);
	}
	else if (nid >= _IdCnx[sid].NetworkConnectionAssociations.size())
	{
		nlwarning ("HNETL5: No net association for nid %hu, use the default connection %hu", (uint16)nid, (uint16)connectionId);
	}
	else
	{
		if (_IdCnx[sid].NetworkConnectionAssociations[nid] >= _IdCnx[sid].Connection.size ())
		{
			nlwarning ("HNETL5: Can't send message to %s because nid %d point on a bad connection (%d and only have %d cnx), use default connection", _IdCnx[sid].ServiceName.c_str (), nid, connectionId, _IdCnx[sid].Connection.size ());
		}
		else
		{
			connectionId = _IdCnx[sid].NetworkConnectionAssociations[nid];
		}
	}

retry:
	if (connectionId >= _IdCnx[sid].Connection.size() || !_IdCnx[sid].Connection[connectionId].valid() || !_IdCnx[sid].Connection[connectionId].CbNetBase->connected())
	{
		if(connectionId != 0)
		{
			nlwarning ("HNETL5: Can't send message to %s because connection is not valid or connected, use default connection", _IdCnx[sid].ServiceName.c_str ());
			connectionId = 0;
			goto retry;
		}
		else
		{
			nlwarning ("HNETL5: Can't send message to %s because default connection is not exist, valid or connected", _IdCnx[sid].ServiceName.c_str ());
			return 0xFF;
		}
	}
	return connectionId;
}


//
//
//

void	CUnifiedNetwork::send(const string &serviceName, const CMessage &msgout, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(const string&, const CMessage&) whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	TNameMappedConnection::const_iterator								it;
	pair<TNameMappedConnection::const_iterator,TNameMappedConnection::const_iterator>	range;
	range = _NamedCnx.equal_range(serviceName);

	bool found = false;
	if (range.first != _NamedCnx.end())
	{
		for (it=range.first; it!=range.second; ++it)
		{
			uint16	sid = (*it).second;
			if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
			{
				// should never happen
				AUTOCHECK_DISPLAY ("HNETL5: Can't send to the service '%s' because it was in the _NamedCnx but not in _IdCnx", serviceName.c_str ());
				return;
			}

			found = true;

			uint8 connectionId = findConnectionId (sid, nid);
			if (connectionId == 0xff)	// failed
				continue;

			nlinfo("HNETL5: send message to %s using nid %d cnx %d / %s", serviceName.c_str (), nid, connectionId, connectionId<_IdCnx[sid].ExtAddress.size ()?_IdCnx[sid].ExtAddress[connectionId].asString().c_str():"???");
			_IdCnx[sid].Connection[connectionId].CbNetBase->send (msgout, _IdCnx[sid].Connection[connectionId].HostId);
		}
	}

	if (!found)
		nlwarning ("HNETL5: can't find service %s to send message %s", serviceName.c_str(), msgout.getName().c_str());
}

void	CUnifiedNetwork::send(uint16 sid, const CMessage &msgout, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(uint16, const CMessage&) whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
	{
		// should never happen
		AUTOCHECK_DISPLAY ("HNETL5: Can't send to the service '%hu' because not in _IdCnx", sid);
		return;
	}

	uint8 connectionId = findConnectionId (sid, nid);
	if (connectionId == 0xff)	// failed
		return;

	_IdCnx[sid].Connection[connectionId].CbNetBase->send (msgout, _IdCnx[sid].Connection[connectionId].HostId);
}

void	CUnifiedNetwork::send(const CMessage &msgout, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(const CMessage&) whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	uint	i;
	for (i=0; i<_IdCnx.size(); ++i)
	{
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
		{
			if (_IdCnx[i].Connection.size () == 0 || !_IdCnx[i].Connection[0].valid() || !_IdCnx[i].Connection[0].CbNetBase->connected())
			{
				nldebug ("HNETL5: Can't send message to %u because no connection available", i);
				continue;
			}

			uint8 connectionId = findConnectionId (i, nid);
			if (connectionId == 0xff)	// failed
				continue;

			_IdCnx[i].Connection[connectionId].CbNetBase->send (msgout, _IdCnx[i].Connection[connectionId].HostId);
		}
	}
}


//
//
//

void	CUnifiedNetwork::addCallbackArray (const TUnifiedCallbackItem *callbackarray, CStringIdArray::TStringId arraysize)
{
	uint	i;

	for (i=0; i<(uint)arraysize; ++i)
		_Callbacks.insert(make_pair(string(callbackarray[i].Key),callbackarray[i].Callback));
}


void	CUnifiedNetwork::setServiceUpCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg, bool back)
{
	if (serviceName == "*")
	{
		nlassert (cb != NULL);
		if (back)
			_UpUniCallback.push_back (make_pair(cb, arg));
		else
			_UpUniCallback.insert (_UpUniCallback.begin(), make_pair(cb, arg));

		return;
	}

	TNameMappedCallback::iterator	it = _UpCallbacks.find(serviceName);
	if (it == _UpCallbacks.end())
	{
		TCallbackArgItem	ncb;
		ncb.first = NULL;
		ncb.second = NULL;
		it = _UpCallbacks.insert(make_pair(serviceName, ncb));
	}

	(*it).second.first = cb;
	(*it).second.second = arg;
}

void	CUnifiedNetwork::setServiceDownCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg, bool back)
{
	if (serviceName == "*")
	{
		nlassert (cb != NULL);
		if (back)
			_DownUniCallback.push_back (make_pair(cb, arg));
		else
			_DownUniCallback.insert (_DownUniCallback.begin(), make_pair(cb, arg));

		return;
	}

	TNameMappedCallback::iterator	it = _DownCallbacks.find(serviceName);
	if (it == _DownCallbacks.end())
	{
		TCallbackArgItem	ncb;
		ncb.first = NULL;
		ncb.second = NULL;
		it = _DownCallbacks.insert(make_pair(serviceName, ncb));
	}

	(*it).second.first = cb;
	(*it).second.second = arg;
}

//
//
//

uint64 CUnifiedNetwork::getBytesSent ()
{
	uint64	sent = 0;
	uint	i,j;
	/// \todo ace optim: use the _UsedConnection for quick access
	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connection.size (); ++j)
				if(_IdCnx[i].Connection[j].valid () && !_IdCnx[i].Connection[j].IsServerConnection)
					sent += _IdCnx[i].Connection[j].CbNetBase->getBytesSent();

	sent += _CbServer->getBytesSent();
	return sent;
}

uint64 CUnifiedNetwork::getBytesReceived ()
{
	uint64	received = 0;
	uint	i,j;
	/// \todo ace optim: use the _UsedConnection for quick access
	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connection.size (); ++j)
				if(_IdCnx[i].Connection[j].valid () && !_IdCnx[i].Connection[j].IsServerConnection)
					received += _IdCnx[i].Connection[j].CbNetBase->getBytesReceived();

	received += _CbServer->getBytesReceived();
	return received;
}

uint64 CUnifiedNetwork::getSendQueueSize ()
{
	uint64	sent = 0;
	uint	i,j;
	/// \todo ace optim: use the _UsedConnection for quick access
	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connection.size (); ++j)
				if(_IdCnx[i].Connection[j].valid () && !_IdCnx[i].Connection[j].IsServerConnection)
					sent += _IdCnx[i].Connection[j].CbNetBase->getSendQueueSize();

	sent += _CbServer->getSendQueueSize();
	return sent;
}

uint64 CUnifiedNetwork::getReceiveQueueSize ()
{
	uint64	received = 0;
	uint	i,j;
	/// \todo ace optim: use the _UsedConnection for quick access
	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connection.size (); ++j)
				if(_IdCnx[i].Connection[j].valid () && !_IdCnx[i].Connection[j].IsServerConnection)
					received += _IdCnx[i].Connection[j].CbNetBase->getReceiveQueueSize();

	received += _CbServer->getReceiveQueueSize();
	return received;
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(const std::string &name, TSockId &host)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::getNetBase() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	sint	count = _NamedCnx.count(name);
	
	if (count <= 0)
	{
		AUTOCHECK_DISPLAY ("HNETL5: couldn't access the service %s", name.c_str());
		return NULL;
	}
	else if (count > 1)
	{
		AUTOCHECK_DISPLAY ("HNETL5: %d services %s to get CCallbackNetBase", count, name.c_str());
	}

	TNameMappedConnection::const_iterator	itnmc = _NamedCnx.find(name);

// ace todo can send on a selected connection
	nlassert (_IdCnx[(*itnmc).second].Connection.size () > 0 && _IdCnx[(*itnmc).second].Connection[0].valid());
	host = _IdCnx[(*itnmc).second].Connection[0].HostId;
	return _IdCnx[(*itnmc).second].Connection[0].CbNetBase;
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(TServiceId sid, TSockId &host)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::getNetBase() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
	{
		// should never happen
		AUTOCHECK_DISPLAY ("HNETL5: Can't get net base to the service '%hu' because not in _IdCnx", sid);
		host = InvalidSockId;
		return NULL;
	}

// ace todo can send on a selected connection
	nlassert (_IdCnx[sid].Connection.size () > 0 && _IdCnx[sid].Connection[0].valid());
	host = _IdCnx[sid].Connection[0].HostId;
	return _IdCnx[sid].Connection[0].CbNetBase;
}

TUnifiedMsgCallback CUnifiedNetwork::findCallback (const std::string &callbackName)
{
	TMsgMappedCallback::iterator	itcb = _Callbacks.find(callbackName);
	if (itcb == _Callbacks.end())
		return NULL;
	else
		return (*itcb).second;
}

//
//
//

CUnifiedNetwork	*CUnifiedNetwork::_Instance = NULL;

CUnifiedNetwork	*CUnifiedNetwork::getInstance ()
{
	if (_Instance == NULL)
		_Instance = new CUnifiedNetwork();

	return _Instance;
}

bool CUnifiedNetwork::isUsed ()
{
	return (_Instance != NULL);
}

//
//
//

CUnifiedNetwork::CUnifiedConnection	*CUnifiedNetwork::getUnifiedConnection (uint16 sid)
{
	if (sid < _IdCnx.size () && _IdCnx[sid].State == CUnifiedConnection::Ready)
	{
		if (sid != _IdCnx[sid].ServiceId)
		{
			AUTOCHECK_DISPLAY ("Sid index %hu is not the same that in the entry", sid, _IdCnx[sid].ServiceId);
		}
		return &_IdCnx[sid];
	}
	else
	{
		AUTOCHECK_DISPLAY ("Try to get a bad unified connection (sid %hu is not in the table)", sid);
		return NULL;
	}
}

void	CUnifiedNetwork::autoCheck()
{
	uint i, j;

	for (i = 0; i < _IdCnx.size (); i++)
	{
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
		{
			_IdCnx[i].AutoCheck = 1;
		}
		else
		{
			_IdCnx[i].AutoCheck = 0;
		}
	}

	TNameMappedConnection::iterator	itn;
	for (itn = _NamedCnx.begin(); itn != _NamedCnx.end(); ++itn)
	{
		if ((*itn).first != _IdCnx[(*itn).second].ServiceName) AUTOCHECK_DISPLAY ("HLNET5: problem with name syncro between _NameCnx and _IdCnx '%s' '%s' '%d'", (*itn).first.c_str(), _IdCnx[(*itn).second].ServiceName.c_str (), (*itn).second);
		if (_IdCnx[(*itn).second].AutoCheck == 0)  AUTOCHECK_DISPLAY ("HLNET5: problem with name syncro between _NameCnx and _IdCnx '%s' '%d'", (*itn).first.c_str(), _IdCnx[(*itn).second].ServiceName.c_str (),(*itn).second);
		if (_IdCnx[(*itn).second].AutoCheck > 1)  AUTOCHECK_DISPLAY ("HLNET5: problem with name syncro between _NameCnx and _IdCnx '%s' '%d' more than one entry in named with the same name", (*itn).first.c_str(), _IdCnx[(*itn).second].ServiceName.c_str (),(*itn).second);
		_IdCnx[(*itn).second].AutoCheck++;
	}

	for (i = 0; i < _UsedConnection.size (); i++)
	{
		if (_IdCnx[_UsedConnection[i]].State != CUnifiedNetwork::CUnifiedConnection::Ready) AUTOCHECK_DISPLAY ("HLNET5: problem with the _UsedConnection syncro sid %d is not used in _IdCnx", _UsedConnection[i]);
	}
	
	for (i = 0; i < _IdCnx.size (); i++)
	{
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
		{
			for (j = 0; j < _UsedConnection.size (); j++)
			{
				if (_UsedConnection[j] == i) break;
			}
			if (j == _UsedConnection.size ()) AUTOCHECK_DISPLAY ("HLNET5: problem with the _UsedConnection syncro sid %d is not in _UsedConnection", i);
		}
	}
	
	for (i = 0; i < _IdCnx.size (); i++)
	{
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::NotUsed)
		{
			if (_IdCnx[i].ServiceName != "DEAD") AUTOCHECK_DISPLAY ("HLNET5: sid %d name should be DEAD and is '%s'", i, _IdCnx[i].ServiceName.c_str ());
			if (_IdCnx[i].ServiceId != 0xDEAD) AUTOCHECK_DISPLAY ("HLNET5: sid %d sid should be 0xDEAD and is 0x%X", i, _IdCnx[i].ServiceId);
			if (!_IdCnx[i].Connection.empty ()) AUTOCHECK_DISPLAY ("HLNET5: sid %d connection size should be 0 and is %d", i, _IdCnx[i].Connection.size ());
			if (!_IdCnx[i].ExtAddress.empty ()) AUTOCHECK_DISPLAY ("HLNET5: sid %d ext addr size should be 0 and is %d", i, _IdCnx[i].ExtAddress.size ());
			if (_IdCnx[i].AutoCheck != 0) AUTOCHECK_DISPLAY ("HLNET5: sid %d prob with syncro with _NamedCnx", i);
		}
		else if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
		{
			if (_IdCnx[i].ServiceId != i) AUTOCHECK_DISPLAY ("HNETL5: Bad syncro sid index sid entry for %d %d", i, _IdCnx[i].ServiceId);

			if (_IdCnx[i].ServiceName == "DEAD") AUTOCHECK_DISPLAY ("HLNET5: sid %d name should not be DEAD and is '%s'", i, _IdCnx[i].ServiceName.c_str ());
			if (_IdCnx[i].ServiceId == 0xDEAD) AUTOCHECK_DISPLAY ("HLNET5: sid %d sid should not be 0xDEAD and is 0x%X", i, _IdCnx[i].ServiceId);
			if (!_IdCnx[i].ExtAddress.empty () && _IdCnx[i].Connection.size () > _IdCnx[i].ExtAddress.size()) AUTOCHECK_DISPLAY ("HLNET5: sid %d ext addr size should not be 0 and is %d", i, _IdCnx[i].ExtAddress.size ());

			if (_IdCnx[i].AutoRetry == true && _IdCnx[i].Connection.size () > 1) AUTOCHECK_DISPLAY ("HLNET5: sid %d auto retry with more than one connection %d", i, _IdCnx[i].Connection.size ());
			if (_IdCnx[i].AutoRetry == true && _IdCnx[i].IsExternal == false) AUTOCHECK_DISPLAY ("HLNET5: sid %d auto retry with internal connection", i);
			if (_IdCnx[i].AutoRetry == true && _IdCnx[i].Connection[0].valid() == false) AUTOCHECK_DISPLAY ("HLNET5: sid %d auto retry with invalid connection", i);

			for (j = 0; j < _IdCnx[i].Connection.size (); j++)
			{
				if (_IdCnx[i].Connection[j].valid() && !_IdCnx[i].Connection[j].IsServerConnection && _IdCnx[i].Connection[j].CbNetBase->connected () && _IdCnx[i].Connection[j].getAppId() != i) AUTOCHECK_DISPLAY ("HLNET5: sid %d bad appid %"NL_I64"X", i, _IdCnx[i].Connection[j].getAppId());
			}

			for (j = 0; j < _IdCnx[i].NetworkConnectionAssociations.size (); j++)
			{
				if (_IdCnx[i].NetworkConnectionAssociations[j] != 0)
				{
					if (_NetworkAssociations[j] != _IdCnx[i].ExtAddress[_IdCnx[i].NetworkConnectionAssociations[j]].internalNetAddress ()) AUTOCHECK_DISPLAY ("HLNET5: sid %d nid %d have address 0x%08x and is not the good connection net 0x%08x", i, j, _NetworkAssociations[j], _IdCnx[i].ExtAddress[_IdCnx[i].NetworkConnectionAssociations[j]].internalNetAddress ());
				}
			}
		}
	}
}


void CUnifiedNetwork::displayInternalTables (NLMISC::CLog *log)
{
	uint i, j;
	log->displayNL ("%d Named Connections:", _NamedCnx.size ());
	for (TNameMappedConnection::iterator it = _NamedCnx.begin(); it != _NamedCnx.end (); it++)
	{
		log->displayNL ("> '%s' -> %hu", (*it).first.c_str(), (*it).second);
	}

	uint nbused = 0;
	for (i = 0; i < _IdCnx.size (); i++)
	{
		if(_IdCnx[i].State != CUnifiedNetwork::CUnifiedConnection::NotUsed)
			nbused++;
	}

	log->displayNL ("%u/%u Unified Connections:", nbused, _IdCnx.size ());
	for (i = 0; i < _IdCnx.size (); i++)
	{
		if(_IdCnx[i].State != CUnifiedNetwork::CUnifiedConnection::NotUsed)
		{
			log->displayNL ("> %s-%hu %s %s %s (%d extaddr %d cnx)", _IdCnx[i].ServiceName.c_str (), _IdCnx[i].ServiceId, _IdCnx[i].IsExternal?"ext":"int", _IdCnx[i].AutoRetry?"autoretry":"noautoretry", _IdCnx[i].SendId?"sendid":"nosendid", _IdCnx[i].ExtAddress.size (), _IdCnx[i].Connection.size ());
			uint maxc = _IdCnx[i].Connection.size ();
			if(_IdCnx[i].Connection.size () <= _IdCnx[i].ExtAddress.size ())
				maxc = _IdCnx[i].ExtAddress.size ();

			for (j = 0; j < maxc; j++)
			{
				string base;
				if(j < _IdCnx[i].ExtAddress.size ())
				{
					base += _IdCnx[i].ExtAddress[j].asString ();
				}
				else
				{
					base += "notvalid";
				}

				string ext;
				if(j < _IdCnx[i].Connection.size () && _IdCnx[i].Connection[j].valid())
				{
					if(_IdCnx[i].Connection[j].IsServerConnection)
					{
						ext += "server ";
					}
					else
					{
						ext += "client ";
					}
					ext += _IdCnx[i].Connection[j].CbNetBase->getSockId (_IdCnx[i].Connection[j].HostId)->asString ();
					ext += " appid:" + toString(_IdCnx[i].Connection[j].getAppId());
					if (_IdCnx[i].Connection[j].CbNetBase->connected ())
						ext += " connected";
					else
						ext += " notconnected";
				}
				else
				{
					ext += "notvalid";
				}

				log->displayNL ("     - %s %s", base.c_str (), ext.c_str ());
			}
			for (j = 0; j < _IdCnx[i].NetworkConnectionAssociations.size (); j++)
			{
				log->displayNL ("     * nid %d -> cnxn %hu", j, (uint16)_IdCnx[i].NetworkConnectionAssociations[j]);
			}
		}
	}

	log->displayNL ("%u Used Unified Connections:", _UsedConnection.size());
	for (i = 0; i < _UsedConnection.size (); i++)
	{
		log->displayNL ("> %hu", _UsedConnection[i]);
	}

	log->displayNL ("%u Network Associations:", _NetworkAssociations.size());
	for (i = 0; i < _NetworkAssociations.size (); i++)
	{
		log->displayNL ("> 0x%08x -> '%s'", _NetworkAssociations[i], internalIPAddressToString (_NetworkAssociations[i]).c_str ());
	}
}

void CUnifiedNetwork::addNamedCnx (const std::string &name, uint16 sid)
{
	// check if not already inserted
	CUnifiedNetwork::TNameMappedConnection::iterator											it;
	pair<CUnifiedNetwork::TNameMappedConnection::iterator,CUnifiedNetwork::TNameMappedConnection::iterator>	range;
	range = _NamedCnx.equal_range(name);

	if (range.first != range.second)
	{
		for (it=range.first; it!=range.second && (*it).second!=sid; ++it)
			;

		if (it != range.second)
		{
			AUTOCHECK_DISPLAY ("Try to add 2 times the same connection %s-%hu", name.c_str(), sid);
			return;
		}
	}


	// insert the name in the map to be able to send message with the name
	_NamedCnx.insert(make_pair(name, sid));

	allstuffs += "+name "+name+"-"+toString(sid)+"\n";
	test.displayNL ("+name %s-%hu", name.c_str (), sid);
}

void CUnifiedNetwork::removeNamedCnx (const std::string &name, uint16 sid)
{
	// get all map nodes of that service name
	CUnifiedNetwork::TNameMappedConnection::iterator											it;
	pair<CUnifiedNetwork::TNameMappedConnection::iterator,CUnifiedNetwork::TNameMappedConnection::iterator>	range;
	range = _NamedCnx.equal_range(name);

	// assume not empty
	if (range.first == range.second)
	{
		AUTOCHECK_DISPLAY ("The unified connection %s-%hu wasn't on the _NamedCnx", name.c_str(), sid);
		return;
	}

	// select good service id
	for (it=range.first; it!=range.second && (*it).second!=sid; ++it)
		;

	// assume id exists
	if (it == range.second)
	{
		AUTOCHECK_DISPLAY ("The unified connection %s-%hu wasn't on the _NamedCnx", name.c_str(), sid);
		return;
	}

	// remove service for map
	_NamedCnx.erase(it);

	allstuffs += "-name "+name+"-"+toString(sid)+"\n";
	test.displayNL ("-name %s-%hu", name.c_str (), sid);
}

void CUnifiedNetwork::addNetworkAssociation (const string &networkName, uint8 nid)
{
	if (nid >= _NetworkAssociations.size ())
		_NetworkAssociations.resize (nid+1, 0xFF);

	_NetworkAssociations[nid] = stringToInternalIPAddress (networkName);
	nlinfo ("HNETL5: Associate network '%s' 0x%08x '%s' to nid %hu", networkName.c_str(), _NetworkAssociations[nid], internalIPAddressToString (_NetworkAssociations[nid]).c_str(), (uint16)nid);
}



//
//
//


//
// Commands
//

static bool createMessage (CMessage &msgout, const vector<string> &args, CLog &log)
{
	for (uint i = 2; i < args.size (); i+=2)
	{
		string type = args[i+0];
		string value = args[i+1];

			 if (type == "s8")			{ sint8  v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "s16")			{ sint16 v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "s32")			{ sint32 v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "s64")			{ sint64 v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "u8")			{ uint8  v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "u16")			{ uint16 v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "u32")			{ uint32 v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "u64")			{ uint64 v = atoi(value.c_str()); msgout.serial (v); }
		else if (type == "f")			{ float  v = (float)atof(value.c_str()); msgout.serial (v); }
		else if (type == "d")			{ double v = atof(value.c_str()); msgout.serial (v); }
		else if (type == "b")			{ bool v = atoi(value.c_str()) == 1; msgout.serial (v); }
		else if (type == "s")			{ msgout.serial (value); }
		else { log.displayNL ("type '%s' is not a valid type", type.c_str()); return false; }
	}
	return true;
}

/*
 * Simulate a message that comes from the network.
 *
 * for the bool (b type), you must set the value to 1 or 0
 * for the string (s type), we don't manage space inside a string
 * for stl containers, you have first to put a u32 type that is the size of the container and after all elements
 * (ex: if you want to put a vector<uint16> that have 3 elements: u32 3 u16 10 u16 11 u16 12)
 *
 * ex: msgin 128 REGISTER u32 10 u32 541 u32 45
 * You'll receive a fake message REGISTER that seems to come from the service number 128 with 3 uint32.
 *
 */

NLMISC_COMMAND(msgin, "Simulate an input message from another service (ex: msgin 128 REGISTER u32 10 b 1 f 1.5)", "<ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*")
{
	if(args.size() < 2) return false;
	
	uint16 serviceId = atoi (args[0].c_str());
	string serviceName = args[0].c_str();
	string messageName = args[1].c_str();
	
	if (serviceId > 255)
	{
		log.displayNL ("Service Id %d must be between [1;255]", serviceId);
		return false;
	}
	
	if ((args.size()-2) % 2 != 0)
	{
		log.displayNL ("The number of parameter must be a multiple of 2");
		return false;
	}

	CMessage msg (messageName);
	msg.clear ();

	if (!createMessage (msg, args, log))
		return false;

	msg.invert ();

	TUnifiedMsgCallback cb = CUnifiedNetwork::getInstance()->findCallback (messageName);
	
	if (cb == NULL)
	{
		log.displayNL ("Callback for message '%s' is not found", messageName.c_str());
	}
	else
	{
		cb (msg, serviceName, serviceId);
	}
	
		
	return true;
}

/*
 * Create a message and send it to the specified service
 *
 * for the bool (b type), you must set the value to 1 or 0
 * for the string (s type), we don't manage space inside a string
 * for stl containers, you have first to put a u32 type that is the size of the container and after all elements
 * (ex: if you want to put a vector<uint16> that have 3 elements: u32 3 u16 10 u16 11 u16 12)
 *
 * ex: msgout 128 REGISTER u32 10 u32 541 u32 45
 * You'll send a real message REGISTER to the service number 128 with 3 uint32.
 *
 */

NLMISC_COMMAND(msgout, "Send a message to a specified service (ex: msgout 128 REGISTER u32 10 b 1 f 1.5)", "<ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*")
{
	if(args.size() < 2) return false;
	
	uint16 serviceId = atoi (args[0].c_str());
	string serviceName = args[0].c_str();
	string messageName = args[1].c_str();
	
	if (serviceId > 255)
	{
		log.displayNL ("Service Id %d must be between [1;255]", serviceId);
		return false;
	}
	
	if ((args.size()-2) % 2 != 0)
	{
		log.displayNL ("The number of parameter must be a multiple of 2");
		return false;
	}

	CMessage msg (messageName);

	if (!createMessage (msg, args, log))
		return false;

	TSockId host = InvalidSockId;
	CCallbackNetBase *cnb = NULL;

	if (serviceId != 0)
		cnb = CUnifiedNetwork::getInstance()->getNetBase ((uint8)serviceId, host);
	else
		cnb = CUnifiedNetwork::getInstance()->getNetBase (serviceName, host);

	if (cnb == NULL)
	{
		log.displayNL ("'%s' is a bad <ServiceId> or <ServiceName>", args[0].c_str());
		return false;
	}

	cnb->send (msg, host);
	
	return true;
}

NLMISC_COMMAND(l5InternalTables, "displays internal table of network layer5", "")
{
	if(args.size() != 0) return false;

	if (!CUnifiedNetwork::isUsed ())
	{
		log.displayNL("Can't display internal table because layer5 is not used");
		return false;
	}

	CUnifiedNetwork::getInstance ()->displayInternalTables(&log);

	return true;
}

} // NLNET
