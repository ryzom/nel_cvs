/** \file unified_network.cpp
 * Network engine, layer 5, base
 *
 * $Id: unified_network.cpp,v 1.44 2002/07/26 09:01:18 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#define L5_APPID_MARK(a) (((uint64)a)<<32)
#define L5_APPID_UNMARKED(a) ((uint32)a)

namespace NLNET {

//
inline void	CUnifiedNetwork::enterReentrant()
{
	if (_MThreadId == 0xFFFFFFFF || _MThreadId != NLMISC::getThreadId())
	{
/*		if (_MThreadId != 0xFFFFFFFF)
			nldebug("Waiting for thread %d to leave the mutex", _MThreadId);
*/		_Mutex.enter();
		_MThreadId = NLMISC::getThreadId();
	}
	++_MutexCount;
//	nldebug("Enter reentrant CS (_MutexCount=%d)", _MutexCount);
}

inline void	CUnifiedNetwork::leaveReentrant()
{
//	nldebug("Leave reentrant CS (_MutexCount=%d)", _MutexCount);
	--_MutexCount;
	if (_MutexCount == 0)
	{
		_MThreadId = 0xFFFFFFFF;
		_Mutex.leave();
	}
}


// Service registration callbacks

// when a service registers
void	uNetRegistrationBroadcast(const string &name, TServiceId sid, const CInetAddress &addr)
{
	nlinfo ("L5UN: + naming %s %d", name.c_str(), sid);
}

// when a service unregisters
void	uNetUnregistrationBroadcast(const string &name, TServiceId sid, const CInetAddress &addr)
{
	nlinfo ("L5UN: - naming %s %d", name.c_str(), sid);

	// get the service connection
	CUnifiedNetwork						*instance = CUnifiedNetwork::getInstance();

	// get an access to the value
	CRWSynchronized<std::vector<CUnifiedNetwork::CUnifiedConnection> >::CReadAccessor	access(&(instance->_IdCnx));

	// check if we already have a connection that we didn't process already
	uint i;
	for (i=0; i<instance->_ConnectionStack.size(); ++i)
	{
		// check with incoming sid, cause connection table might not be initialised
		if (instance->_ConnectionStack[i].SId == sid)
		{
			// we found a connection not processed
			break;
		}
	}

	if (i < instance->_ConnectionStack.size())
	{
		// remove it from the table
		instance->_ConnectionStack.erase (instance->_ConnectionStack.begin()+i);
	}
	else
	{
		// now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value

		// if the service is not in the array, it means that we are not interested by this service, do nothing
		if (sid >= access.value().size() || !access.value ()[sid].EntryUsed)
			return;

		// adds the connection to the disconnection stack
		instance->_DisconnectionStack.push_back(access.value ()[sid].ServiceId);
	}

	const CUnifiedNetwork::CUnifiedConnection	&cnx = access.value ()[sid];

	// set the service disconnected (avoid sending message to a disconnected service)
	if (instance->_TempDisconnectionTable.size() <= cnx.ServiceId)
		instance->_TempDisconnectionTable.resize(cnx.ServiceId+1, false);
	instance->_TempDisconnectionTable[cnx.ServiceId] = true;


	// get the deconnection callback
	CUnifiedNetwork::TNameMappedCallback::iterator	it = instance->_DownCallbacks.find(cnx.ServiceName);

	if (it != instance->_DownCallbacks.end())
	{
		// call it
		TUnifiedNetCallback	cb = (*it).second.first;
		cb(cnx.ServiceName, cnx.ServiceId, (*it).second.second);
	}

	// call the generic callback
	if (!cnx.IsExternal)
	{
		for (uint c = 0; c < instance->_DownUniCallback.size (); c++)
		{
			if (instance->_DownUniCallback[c].first != NULL)
				instance->_DownUniCallback[c].first(cnx.ServiceName, cnx.ServiceId, instance->_DownUniCallback[c].second);
		}
	}

	nldebug("Received disconnection signal from service %s %d (appId=%d) from naming service", cnx.ServiceName.c_str(), cnx.ServiceId, sid);
}

// Service up/down callbacks

void	uncbConnection(TSockId from, void *arg)
{
	nlinfo ("L5UN: + connec %s", from->asString().c_str());
	// set the service id to an unknown value, besause we don't know yet
	// which service is connecting at this moment.
	nlinfo("L5UNDBG: uncbConnection(from=%s, arg=%p): setAppId(0xFFFFFF) (previous appId=%"NL_I64"x)", from->asString().c_str(), arg, from->appId());
	from->setAppId (0xFFFFFF+L5_APPID_MARK(0x0100));

	nldebug("Received Connection signal from %s connection", from->asString().c_str());
}

void	uncbDisconnection(TSockId from, void *arg)
{
	if (L5_APPID_UNMARKED(from->appId()) == 0xFFFFFF)
	{
		nlinfo ("L5UN: - connec UKN");
		return;
	}

	CUnifiedNetwork						*instance = CUnifiedNetwork::getInstance();
	uint16								sid = (uint16)from->appId();
	// get an access to the value
	CRWSynchronized<std::vector<CUnifiedNetwork::CUnifiedConnection> >::CReadAccessor	access(&(instance->_IdCnx));

	// now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
	const CUnifiedNetwork::CUnifiedConnection	&cnx = access.value ()[sid];

	nlinfo ("L5UN: - connec %s %s %d", from->asString().c_str(), cnx.ServiceName.c_str(), sid);

	if (instance->_TempDisconnectionTable.size() <= cnx.ServiceId)
		instance->_TempDisconnectionTable.resize(cnx.ServiceId+1, false);
	instance->_TempDisconnectionTable[cnx.ServiceId] = true;

	nldebug("Received disconnection signal from service %s %d (appId=%d) connection", cnx.ServiceName.c_str(), cnx.ServiceId, sid);
	
	return;

////////////////////
////////////////////
////////////////////

	if (L5_APPID_UNMARKED(from->appId()) == 0xFFFFFF)
	{
		// the service didn't identified before disconnection
	}
	else
	{
		// get the service connection
		CUnifiedNetwork						*instance = CUnifiedNetwork::getInstance();
		uint16								sid = (uint16)from->appId();

		// get an access to the value
		CRWSynchronized<std::vector<CUnifiedNetwork::CUnifiedConnection> >::CReadAccessor	access(&(instance->_IdCnx));

		// now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
		const CUnifiedNetwork::CUnifiedConnection	&cnx = access.value ()[sid];

		// check if we already have a connection that we didn't process already
		uint i;
		for (i=0; i<instance->_ConnectionStack.size(); ++i)
		{
			// check with incoming sid, cause connection table might not be initialised
			if (instance->_ConnectionStack[i].SId == sid)
			{
				// we found a connection not processed
				break;
			}
		}

		// assume no double deconnection
		nlassert(instance->_TempDisconnectionTable.size() <= sid || !instance->_TempDisconnectionTable[sid]);

		if (i < instance->_ConnectionStack.size())
		{
			// remove it from the table
			instance->_ConnectionStack.erase (instance->_ConnectionStack.begin()+i);
		}
		else
		{
			// adds the connection to the disconnection stack
			instance->_DisconnectionStack.push_back(cnx.ServiceId);
		}

		// set the service disconnected (avoid sending message to a disconnected service)
		if (instance->_TempDisconnectionTable.size() <= cnx.ServiceId)
			instance->_TempDisconnectionTable.resize(cnx.ServiceId+1, false);
		instance->_TempDisconnectionTable[cnx.ServiceId] = true;

/*
		if (instance->_TempDisconnectionTable.size() <= cnx.ServiceId)
		{
			uint	i = instance->_TempDisconnectionTable.size();
			instance->_TempDisconnectionTable.resize(cnx.ServiceId+1, false);
			for (; i <= cnx.ServiceId; ++i)
				instance->_TempDisconnectionTable[i] = false;
		}
		instance->_TempDisconnectionTable[cnx.ServiceId] = true;
*/

		// get the deconnection callback
		CUnifiedNetwork::TNameMappedCallback::iterator	it = instance->_DownCallbacks.find(cnx.ServiceName);

		if (it != instance->_DownCallbacks.end())
		{
			// call it
			TUnifiedNetCallback	cb = (*it).second.first;
			cb(cnx.ServiceName, cnx.ServiceId, (*it).second.second);
		}

		// call the generic callback
		if (!cnx.IsExternal)
		{
			for (uint c = 0; c < instance->_DownUniCallback.size (); c++)
			{
				if (instance->_DownUniCallback[c].first != NULL)
					instance->_DownUniCallback[c].first(cnx.ServiceName, cnx.ServiceId, instance->_DownUniCallback[c].second);
			}
		}

		nldebug("Received disconnection signal from service %s %d (appId=%d)", cnx.ServiceName.c_str(), cnx.ServiceId, (uint16)from->appId());
	}

}

//
//
//

void	uncbServiceIdentification(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string		inSName;
	uint16		inSid;

	// recover the service name and id
	msgin.serial(inSName);
	msgin.serial(inSid);

	nlinfo ("L5UN: + connec ident %s %s %d", from->asString().c_str(), inSName.c_str(), inSid);
	
	nlinfo("Received identification from service %s %u", inSName.c_str(), inSid);

	// setup the sock with the sid.
	nlinfo("L5UNDBG: uncbServiceIdentification(from=%s): setAppId(inSid=%d) (previous appId=%"NL_I64"x)", from->asString().c_str(), inSid, from->appId());
	from->setAppId(inSid+L5_APPID_MARK(0x0201));

	// add a new connection to the list
	CUnifiedNetwork		*instance = CUnifiedNetwork::getInstance();

	// check if we already have a connection that we didn't process already
	uint i;
	for (i=0; i<instance->_DisconnectionStack.size(); ++i)
	{
		if (instance->_DisconnectionStack[i] == inSid)
		{
			// we found a disconnection not processed
			break;
		}
	}

	if (i < instance->_DisconnectionStack.size())
	{
		// remove it from the table
		instance->_DisconnectionStack.erase (instance->_DisconnectionStack.begin()+i);
		instance->_ConnectionStack.push_back(CUnifiedNetwork::CConnectionId(inSName, inSid, from, false));

//		nlinfo ("**************************************************************************** OKAY DCNX->CNX in the same loop");
		// assume previous disconnection
		nlassert(instance->_TempDisconnectionTable.size() > inSid && instance->_TempDisconnectionTable[inSid]);
		instance->_TempDisconnectionTable[inSid] = false;
	}
	else
	{
		// assume no previous disconnection
		//nlassert(instance->_TempDisconnectionTable.size() <= inSid || !instance->_TempDisconnectionTable[inSid]);
		instance->_ConnectionStack.push_back(CUnifiedNetwork::CConnectionId(inSName, inSid, from, true));
	}

	// get the connection callback
	CUnifiedNetwork::TNameMappedCallback::iterator	it = instance->_UpCallbacks.find(inSName);
	if (it != instance->_UpCallbacks.end())
	{
		// call it
		TUnifiedNetCallback	cb = (*it).second.first;
		cb(inSName, inSid, (*it).second.second);
	}

	for (uint c = 0; c < instance->_UpUniCallback.size (); c++)
	{
		if (instance->_UpUniCallback[c].first != NULL)
			instance->_UpUniCallback[c].first (inSName, inSid, instance->_UpUniCallback[c].second);
	}
}

// the callbacks wrapper
void	uncbMsgProcessing(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	if (L5_APPID_UNMARKED(from->appId()) != 0xFFFFFF)
	{
		CUnifiedNetwork									*inst = CUnifiedNetwork::getInstance();
		uint16											sid = (uint16)from->appId();
		CUnifiedNetwork::TMsgMappedCallback::iterator	itcb;

		itcb = inst->_Callbacks.find(msgin.getName());
		if (itcb == inst->_Callbacks.end())
		{
			// the callback doesn't exist
			nlwarning("HNETL5: can't find callback %s called by service %d", msgin.getName().c_str(), sid);
		}
		else
		{
			// call the callback
			CRWSynchronized< std::vector<CUnifiedNetwork::CUnifiedConnection> >::CReadAccessor	idAccess(&(inst->_IdCnx));

			/// TEMP check Ben
//			nlassert(idAccess.value()[sid].EntryUsed);
//			nlassert(idAccess.value()[sid].ServiceId == sid);

			if (idAccess.value()[sid].ServiceId == sid)
			{
				(*itcb).second (msgin, idAccess.value()[sid].ServiceName, sid);
			}
			else if (idAccess.value()[sid].ServiceId == 0xDEAD)
			{
				// looks for service in the connection stack
				uint	i;
				for (i=0; i<inst->_ConnectionStack.size(); ++i)
					if (inst->_ConnectionStack[i].SId == sid && !inst->_TempDisconnectionTable[sid])
						break;

				if (i == inst->_ConnectionStack.size())
				{
					nlwarning("HNETL5: received a message from service %d which could not be indentified by its name", sid);
				}
				else
				{
					(*itcb).second (msgin, inst->_ConnectionStack[i].SName, sid);
				}
			}
			else
			{
				nlwarning("HNETL5: received a message from service %d which has a wrong sid in table", sid);
				(*itcb).second (msgin, idAccess.value()[sid].ServiceName, sid);
			}
		}
	}
	else
	{
		// service not yet identified
		nlwarning("HNETL5: received the message %s from a not yet identified service", msgin.getName().c_str());
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
//	InfoLog->addPositiveFilter ("L5UN");
//	DebugLog->addNegativeFilter (" ");

	DebugLog->addNegativeFilter ("L5UN");

	_RecordingState = rec;
	_Name = shortName;
	_SId = sid;
	
	if (addr != NULL)
		_NamingServiceAddr = *addr;

	// if the address isn't null, uses the naming service
	if (_NamingServiceAddr.isValid ())
	{
		// connect to the naming service (may generate a ESocketConnectionFailed exception)
		CNamingClient::connect(_NamingServiceAddr, _RecordingState);

		// connect the callback to know when a new service comes in or goes down
		CNamingClient::setRegistrationBroadcastCallback(uNetRegistrationBroadcast);
		CNamingClient::setUnregistrationBroadcastCallback(uNetUnregistrationBroadcast);

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
		CInetAddress laddr = CInetAddress::localHost ();
		laddr.setPort(_ServerPort);
		if (_SId == 0)
		{
			_SId = CNamingClient::registerService(_Name, laddr);
		}
		else
		{
			CNamingClient::registerServiceWithSId(_Name, laddr, _SId);
		}

		sid = _SId;
	}

	nlinfo ("HNETL5: Server '%s' added, registered and listen to port %hu", shortName.c_str (), port);

	_Initialised = true;
}

void	CUnifiedNetwork::connect()
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::connect() whereas it is not initialised yet"));

	if (CNamingClient::connected())
	{
		// get the services list
		const list<CNamingClient::CServiceEntry>	&services = CNamingClient::getRegisteredServices();

		// connects to the registered services
		list<CNamingClient::CServiceEntry>::const_iterator	its;
		TNameMappedCallback::iterator						itcb;

		// don't connect to itself
		for (its = services.begin(); its != services.end(); ++its)
		{
			if (_SId != (*its).SId)
			{
				// add service with name, address, ident, not external, service id, and not autoretry (obsolete)
				addService((*its).Name, (*its).Addr, true, false, (*its).SId, false);
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

	CRWSynchronized< std::vector<CUnifiedConnection> >::CWriteAccessor	idAccess(&_IdCnx);
	CRWSynchronized<TNameMappedConnection>::CWriteAccessor				nameAccess(&_NamedCnx);

	uint	i;

	// disconnect all clients
	_CbServer->disconnect(InvalidSockId);
	delete _CbServer;
	_CbServer = NULL;

	// disconnect all connections to servers
	for (i=0; i<idAccess.value().size(); ++i)
	{
		if (idAccess.value()[i].EntryUsed && !idAccess.value()[i].IsServerConnection)
		{
			if (idAccess.value()[i].Connection.CbClient->connected ())
				idAccess.value()[i].Connection.CbClient->disconnect();

			delete idAccess.value()[i].Connection.CbClient;
			idAccess.value()[i].Connection.CbClient = NULL;
		}
	}

	// clear all other data
	idAccess.value().clear();
	nameAccess.value().clear();
	_UpCallbacks.clear();
	_DownCallbacks.clear();
	_Callbacks.clear();

	// disconnect the connection with the naming service
	if (CNamingClient::connected ())
		CNamingClient::disconnect ();
}

void	CUnifiedNetwork::addService(const string &name, const CInetAddress &addr, bool sendId, bool external, uint16 sid, bool autoRetry)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::addService() whereas it is not initialised yet"));

	if (external)
		sid = _ExtSId++;

	// create a new connection with the service, setup callback and connect
	CCallbackClient	*cbc = new CCallbackClient();
	cbc->setDisconnectionCallback(uncbDisconnection, NULL);
	cbc->setDefaultCallback(uncbMsgProcessing);

	bool	connectSuccess = false;
	if (_TempDisconnectionTable.size() <= sid)
		_TempDisconnectionTable.resize(sid+1);

	try
	{
		cbc->connect(addr);
		nlinfo("L5UNDBG: addService(name=%s, addr=%s): setAppId(sid=%d) (previous appId=%"NL_I64"x)", name.c_str(), addr.asString().c_str(), sid, cbc->getSockId()->appId());
		cbc->getSockId()->setAppId(sid+L5_APPID_MARK(0x0202));
		connectSuccess = true;
	}
	catch (ESocketConnectionFailed &e)
	{
		nlwarning ("HNETL5: can't connect to %s (sid %u) now (%s)", name.c_str(), sid, e.what ());
	}

	if (!connectSuccess && !autoRetry)
		return;

	{
		CRWSynchronized<TNameMappedConnection>::CWriteAccessor				nameAccess(&_NamedCnx);
		CRWSynchronized< std::vector<CUnifiedConnection> >::CWriteAccessor	idAccess(&_IdCnx);

		// add the connection to the list (and setup the callbacks for this connection)
		if (sid >= idAccess.value().size())
			idAccess.value().resize(sid+1);

		CUnifiedConnection	&cnx = idAccess.value()[sid];

		nlassert(!cnx.EntryUsed);

		cnx = CUnifiedConnection(name, sid, cbc);
		nameAccess.value().insert(make_pair(name, sid));
		cnx.IsConnected = connectSuccess;
		cnx.IsExternal = external;
		cnx.AutoRetry = autoRetry;
		cnx.ExtAddress = addr;
		cnx.SendId = sendId;
	}

	if (connectSuccess)
	{
		if (sendId)
		{
			// send identification to the service
			CMessage	msg("UN_SIDENT");
			uint16		ssid = _SId;
			msg.serial(_Name);
			msg.serial(ssid);	// serializes a 16 bits service id
			send(sid, msg);
		}

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

	bool	enableRetry;	// true every 5 seconds to reconnect if necessary

//	nldebug("In CUnifiedNetwork::update()");
	enterReentrant();
	{

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

		_ConnectionRetriesStack.clear();
		_ConnectionStack.clear();
		_DisconnectionStack.clear();


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
					CNamingClient::connect (_NamingServiceAddr, _RecordingState);
					// re-register the service
					CInetAddress laddr = CInetAddress::localHost ();
					laddr.setPort(_ServerPort);
					CNamingClient::resendRegisteration (_Name, laddr, _SId);
				}
				catch (ESocketConnectionFailed &)
				{
					nlwarning ("Could not connect to the Naming Service (%s). Retrying in a few seconds...", _NamingServiceAddr.asString().c_str());
				}
			}
		}

		// lock read access to the connections
		CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
		const vector<CUnifiedConnection>									&connections = idAccess.value();

		while (true)
		{
			// update all server connections
			_CbServer->update(0);

			// update all client connections
			for (uint i = 0; i<connections.size(); ++i)
			{
				const CUnifiedConnection	&cnx = connections[i];
				if (cnx.EntryUsed && !cnx.IsServerConnection)
				{
					// if connected, update
					if (cnx.IsConnected)
					{
						cnx.Connection.CbClient->update(0);
					}
					else if (enableRetry)
					{
						// assume autoretry
						// entryUsed && not connected and not auto retry => illegal state !!
						nlassert(cnx.AutoRetry);
						_ConnectionRetriesStack.push_back(i);
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
	}

	autoCheck();
	updateConnectionTable();

	leaveReentrant();
//	nldebug("Out CUnifiedNetwork::update()");

	nldebug ("L5UN:   update --------------------------------------------");
}


void	CUnifiedNetwork::updateConnectionTable()
{
	{
		// First: recreate connections, setup connection tables etc.
		CRWSynchronized<TNameMappedConnection>::CWriteAccessor				nameAccess(&_NamedCnx);
		CRWSynchronized< std::vector<CUnifiedConnection> >::CWriteAccessor	idAccess(&_IdCnx);
		uint	i;

		// try to reconnect
		for (i=0; i<_ConnectionRetriesStack.size(); ++i)
		{
			CUnifiedConnection	&cnx = idAccess.value()[_ConnectionRetriesStack[i]];
			// assume not connected and not a server connection
			nlassert(cnx.EntryUsed);
			nlassert(!cnx.IsConnected);
			nlassert(!cnx.IsServerConnection);
			nlassert(cnx.IsExternal);

			try
			{
				cnx.Connection.CbClient->connect(cnx.ExtAddress);
				nlinfo("L5UNDBG: updateConnectionTable(): in connection retry (cnx.ServiceName=%s, cnx.ServiceId=%d) setAppId(cnx.ServiceId=%d) (previous appId=%"NL_I64"x)", cnx.ServiceName.c_str(), cnx.ServiceId, cnx.ServiceId, cnx.Connection.CbClient->getSockId()->appId());
				cnx.Connection.CbClient->getSockId()->setAppId(cnx.ServiceId+L5_APPID_MARK(0x0203));
				cnx.IsConnected = true;
			}
			catch (ESocketConnectionFailed &e)
			{
				nlinfo ("HNETL5: can't connect to %s (sid %u) now (%s)", cnx.ServiceName.c_str(), cnx.ServiceId, e.what ());
			}
		}

		// remove all flaged connections
		for (i=0; i<_DisconnectionStack.size(); ++i)
		{
			CUnifiedConnection	&cnx = idAccess.value()[_DisconnectionStack[i]];

			nlassert(cnx.EntryUsed);
			nlassert(_TempDisconnectionTable[_DisconnectionStack[i]]);

			_TempDisconnectionTable[_DisconnectionStack[i]] = false;

			// flag the connection as unavailable
			cnx.IsConnected = false;

			if (!cnx.IsExternal || !cnx.AutoRetry)
			{
				// assume no auto retry for internal services
				nlassert(cnx.IsExternal || !cnx.AutoRetry);

				// if the cnx isn't extern or if no autoretry, delete it.
				if (!cnx.IsServerConnection)
				{
					nlinfo("L5UNDBG: updateConnectionTable(): delete client connection %s %d (in disconnection stack)", cnx.ServiceName.c_str(), cnx.ServiceId);
					delete cnx.Connection.CbClient;
				}

				// get all map nodes of that service name
				TNameMappedConnection::iterator											it;
				pair<TNameMappedConnection::iterator,TNameMappedConnection::iterator>	range;
				range = nameAccess.value().equal_range(cnx.ServiceName);

				// assume not empty
				nlassert(range.first != range.second);

				// select good service id
				for (it=range.first; it!=range.second && (*it).second!=cnx.ServiceId; ++it)
					;

				nlinfo ("L5UN: - update %s %d", cnx.ServiceName.c_str(), cnx.ServiceId);

				// assume id exists
				nlassert(it != range.second);
				// remove service for map
				nameAccess.value().erase(it);

				cnx.reset();
			}
		}

		// add new connections
		for (i=0; i<_ConnectionStack.size(); ++i)
		{
			if (_ConnectionStack[i].SId >= idAccess.value().size())
				idAccess.value().resize(_ConnectionStack[i].SId+1);

			CUnifiedConnection	&cnx = idAccess.value()[_ConnectionStack[i].SId];

			if (_ConnectionStack[i].NeedInsert)
			{
				nlassert(!cnx.EntryUsed);

				cnx = CUnifiedConnection(_ConnectionStack[i].SName, _ConnectionStack[i].SId, _ConnectionStack[i].SHost);
				cnx.IsConnected = true;
				nameAccess.value().insert(make_pair(_ConnectionStack[i].SName, _ConnectionStack[i].SId));

				nlinfo ("L5UN: + update %s %d", _ConnectionStack[i].SName.c_str(), _ConnectionStack[i].SId);
			}
			else
			{
				// only remap the from and server
				// entry is used
				nlassert(cnx.EntryUsed);
				// if was a callback  client before (!IsServerConnection), delete the callback client
				if (!cnx.IsServerConnection)
				{
					nlinfo("L5UNDBG: updateConnectionTable(): delete client connection %s %d (in connection stack, remove previous connection)", cnx.ServiceName.c_str(), cnx.ServiceId);
					delete cnx.Connection.CbClient;
					cnx.Connection.CbClient = NULL;
				}
				cnx.IsServerConnection = true;
				cnx.Connection.HostId = _ConnectionStack[i].SHost;

				nlinfo ("L5UN: # update %s %d", _ConnectionStack[i].SName.c_str(), _ConnectionStack[i].SId);
			}

			nldebug("HNETL5: Updated table with connection to %s %d", cnx.ServiceName.c_str(), cnx.ServiceId);

			cnx.AutoRetry = false;
		}
	}

	//
	//

	{
		// Second: send identification, callbacks etc.
		CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
		uint	i;

		// send id and callbacks
		for (i=0; i<_ConnectionRetriesStack.size(); ++i)
		{
			const CUnifiedConnection	&cnx = idAccess.value()[_ConnectionRetriesStack[i]];
			nlassert(!cnx.IsServerConnection);

			if (cnx.IsConnected)
			{
				if (cnx.SendId)
				{
					// send identification to the service
					CMessage	msg("UN_SIDENT");
					uint16		ssid = _SId;
					msg.serial(_Name);
					msg.serial(ssid);	// serializes a 16 bits service id
					send(cnx.ServiceId, msg);
				}

				// call the connection callback associated to this service
				TNameMappedCallback::iterator	itcb = _UpCallbacks.find(cnx.ServiceName);
				if (itcb != _UpCallbacks.end() && (*itcb).second.first != NULL)
				{
					TUnifiedNetCallback	cb = (*itcb).second.first;
					cb(cnx.ServiceName, cnx.ServiceId, (*itcb).second.second);
				}

				if (!cnx.IsExternal)
				{
					for (uint i = 0; i < _UpUniCallback.size (); i++)
					{
						if (_UpUniCallback[i].first != NULL)
							_UpUniCallback[i].first (cnx.ServiceName, cnx.ServiceId, _UpUniCallback[i].second);
					}
				}
			}
		}
	}
}

//
//
//

void	CUnifiedNetwork::send(const string &serviceName, const CMessage &msgout)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(const string&, const CMessage&) whereas it is not initialised yet"));

	enterReentrant();
	{
		CRWSynchronized<TNameMappedConnection>::CReadAccessor				nameAccess(&_NamedCnx);
		CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
		TNameMappedConnection												&namedCnx = const_cast<TNameMappedConnection&>(nameAccess.value());

		TNameMappedConnection::const_iterator								it;
		pair<TNameMappedConnection::const_iterator,TNameMappedConnection::const_iterator>	range;
		range = namedCnx.equal_range(serviceName);

		bool	found = false;
		if (range.first != nameAccess.value().end())
		{
			for (it=range.first; it!=range.second; ++it)
			{
				uint16	sid = (*it).second;
				const CUnifiedConnection	&connection = idAccess.value()[sid];

				nlassert(connection.EntryUsed);
				if (!connection.IsConnected || _TempDisconnectionTable[sid])
					continue;

				found = true;

				if (connection.IsServerConnection)
					_CbServer->send(msgout, connection.Connection.HostId);
				else
					connection.Connection.CbClient->send(msgout);

			}
		}

		// looks for waiting connections
		uint	i;
		for (i=0; i<_ConnectionStack.size(); ++i)
		{
			uint16	sid = _ConnectionStack[i].SId;
			if (_ConnectionStack[i].SName == serviceName && !_TempDisconnectionTable[sid])
			{
				found = true;
				_CbServer->send(msgout, _ConnectionStack[i].SHost);
			}
		}

		if (!found)
			nlwarning("HNETL5: can't find service %s to send message %s", serviceName.c_str(), msgout.getName().c_str());
	}
	leaveReentrant();
}

void	CUnifiedNetwork::send(uint16 sid, const CMessage &msgout)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(uint16, const CMessage&) whereas it is not initialised yet"));

	enterReentrant();
	{
		CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);

		if (_TempDisconnectionTable[sid])
		{
			nlwarning("HNETL5: service %d just got disconnected", sid);
			goto SendCSLeave;
		}

		if (sid>=idAccess.value().size() || !idAccess.value()[sid].EntryUsed || !idAccess.value()[sid].EntryUsed)
		{
			uint	i;
			for (i=0; i<_ConnectionStack.size(); ++i)
			{
				if (_ConnectionStack[i].SId == sid)
				{
					_CbServer->send(msgout, _ConnectionStack[i].SHost);
					goto SendCSLeave;
				}
			}
			nlwarning("HNETL5: incorrect service id %d to send message %s", sid, msgout.getName().c_str());
		}
		else
		{
			const CUnifiedConnection	&connection = idAccess.value()[sid];

			if (connection.IsServerConnection)
				_CbServer->send(msgout, connection.Connection.HostId);
			else
				connection.Connection.CbClient->send(msgout);
		}
	}
SendCSLeave:
	leaveReentrant();
}

void	CUnifiedNetwork::send(const CMessage &msgout)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(const CMessage&) whereas it is not initialised yet"));

	enterReentrant();
	{
		CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
		const std::vector<CUnifiedConnection>								&connections = idAccess.value();

		uint	i;
		for (i=0; i<connections.size(); ++i)
			if (connections[i].EntryUsed && connections[i].IsConnected)
				if (connections[i].IsServerConnection)
					_CbServer->send(msgout, connections[i].Connection.HostId);
				else
					connections[i].Connection.CbClient->send(msgout);

		for (i=0; i<_ConnectionStack.size(); ++i)
			_CbServer->send(msgout, _ConnectionStack[i].SHost);
	}
	leaveReentrant();
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
	uint	i;
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
	for (i=0; i<idAccess.value().size(); ++i)
		if (idAccess.value()[i].EntryUsed && !idAccess.value()[i].IsServerConnection)
			sent += idAccess.value()[i].Connection.CbClient->getBytesSent();

	sent += _CbServer->getBytesSent();
	return sent;
}

uint64 CUnifiedNetwork::getBytesReceived ()
{
	uint64	received = 0;
	uint	i;
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
	for (i=0; i<idAccess.value().size(); ++i)
		if (idAccess.value()[i].EntryUsed && !idAccess.value()[i].IsServerConnection)
			received += idAccess.value()[i].Connection.CbClient->getBytesReceived();

	received += _CbServer->getBytesReceived();
	return received;
}

uint64 CUnifiedNetwork::getSendQueueSize ()
{
	uint64	sent = 0;
	uint	i;
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
	for (i=0; i<idAccess.value().size(); ++i)
		if (idAccess.value()[i].EntryUsed && !idAccess.value()[i].IsServerConnection)
			sent += idAccess.value()[i].Connection.CbClient->getSendQueueSize();

	sent += _CbServer->getSendQueueSize();
	return sent;
}

uint64 CUnifiedNetwork::getReceiveQueueSize ()
{
	uint64	received = 0;
	uint	i;
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
	for (i=0; i<idAccess.value().size(); ++i)
		if (idAccess.value()[i].EntryUsed && !idAccess.value()[i].IsServerConnection)
			received += idAccess.value()[i].Connection.CbClient->getReceiveQueueSize();

	received += _CbServer->getReceiveQueueSize();
	return received;
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(const std::string &name, TSockId &host)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::getNetBase() whereas it is not initialised yet"));

	CRWSynchronized<TNameMappedConnection>::CReadAccessor				nameAccess(&_NamedCnx);
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);

	sint	count = nameAccess.value().count(name);
	
	if (count <= 0)
	{
		// service might not be in table yet, look in _ConnectionStack
		uint	i;
		for (i=0; i<_ConnectionStack.size(); ++i)
			if (_ConnectionStack[i].SName == name && !_TempDisconnectionTable[_ConnectionStack[i].SId])
			{
				host = _ConnectionStack[i].SHost;
				return _CbServer;
			}

		nlwarning("HNETL5: couldn't access the service %s", name.c_str());
		return NULL;
	}
	else if (count > 1)
	{
		nlwarning("HNETL5: %d services %s to get CCallbackNetBase", count, name.c_str());
	}

	TNameMappedConnection::const_iterator	itnmc = nameAccess.value().find(name);

	const CUnifiedConnection	&cnx = idAccess.value()[(*itnmc).second];

	if (_TempDisconnectionTable[cnx.ServiceId])
	{
		nlwarning("HNETL5: service %s just got disconnected", name.c_str());
		return NULL;
	}

	if (cnx.IsServerConnection)
	{
		host = cnx.Connection.HostId;
		return _CbServer;
	}
	else
	{
		host = InvalidSockId;
		return cnx.Connection.CbClient;
	}
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(TServiceId sid, TSockId &host)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::getNetBase() whereas it is not initialised yet"));

	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);

	if (sid>=idAccess.value().size() || !idAccess.value()[sid].EntryUsed || !idAccess.value()[sid].EntryUsed)
	{
		// service might not be in table yet, look in _ConnectionStack
		uint	i;
		for (i=0; i<_ConnectionStack.size(); ++i)
			if (_ConnectionStack[i].SId == sid && !_TempDisconnectionTable[sid])
			{
				host = _ConnectionStack[i].SHost;
				return _CbServer;
			}

		nlwarning("HNETL5: incorrect service id %d to get netbase", sid);
		return NULL;
	}

	const CUnifiedConnection	&cnx = idAccess.value()[sid];

	if (_TempDisconnectionTable[cnx.ServiceId])
	{
		nlwarning("HNETL5: service %d just got disconnected", sid);
		return NULL;
	}

	if (cnx.IsServerConnection)
	{
		host = cnx.Connection.HostId;
		return _CbServer;
	}
	else
	{
		host = InvalidSockId;
		return cnx.Connection.CbClient;
	}
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

void	CUnifiedNetwork::autoCheck()
{
	static bool stopCheck = false;

	if(stopCheck)
		return;

	CRWSynchronized<TNameMappedConnection>::CWriteAccessor				nameAccess(&_NamedCnx);
	CRWSynchronized< std::vector<CUnifiedConnection> >::CWriteAccessor	idAccess(&_IdCnx);

	TNameMappedConnection::iterator	itn;
	for (itn = nameAccess.value().begin(); itn != nameAccess.value().end(); ++itn)
	{
		nlassert((*itn).first == idAccess.value()[(*itn).second].ServiceName);
		nlassert(!idAccess.value()[(*itn).second].AutoCheck);
		idAccess.value()[(*itn).second].AutoCheck = true;
	}

	uint	i, j;
	for (i=0; i<idAccess.value().size(); ++i)
	{
		if (!idAccess.value()[i].EntryUsed)
		{
			nlassert(idAccess.value()[i].ServiceName == "DEAD");
			nlassert(idAccess.value()[i].ServiceId == (uint16)(0xDEAD));
			continue;
		}

		nlassert(idAccess.value()[i].ServiceId == i);
		nlassert(idAccess.value()[i].AutoRetry || idAccess.value()[i].IsConnected);
		nlassert(idAccess.value()[i].AutoCheck);
		idAccess.value()[i].AutoCheck = false;

		bool	stillConnected = true;
		for (j=0; j<_DisconnectionStack.size(); ++j)
			if (_DisconnectionStack[j] == i)
			{
				stillConnected = false;
				break;
			}

		if (i < _TempDisconnectionTable.size() && _TempDisconnectionTable[i])
			continue;

		if (!stillConnected || !idAccess.value()[i].IsConnected)
			continue;

		uint64 appId = (idAccess.value()[i].IsServerConnection) ?
			(idAccess.value()[i].Connection.HostId->appId()) :
			(idAccess.value()[i].Connection.CbClient->getSockId()->appId());

		// ca crash ici des fois kan on quitte violement
		if (i != (uint16)appId)
		{
			nlwarning("i != appId !! -- (i = %x) != (appId = %"NL_I64"x)", i, appId);
			if (DefaultMemDisplayer)
				DefaultMemDisplayer->write();

			// ace: if this happen, we stop check because it can assert after
			stopCheck = true;
			return;
		}
	}

	for (i=0; i<_ConnectionStack.size(); ++i)
	{
		nlassert(!(idAccess.value()[_ConnectionStack[i].SId].EntryUsed) || _ConnectionStack[i].NeedInsert);
		nlassert(!_TempDisconnectionTable[_ConnectionStack[i].SId]);
	}

	for (i=0; i<_DisconnectionStack.size(); ++i)
	{
		nlassert(idAccess.value()[_DisconnectionStack[i]].EntryUsed);
		nlassert(_TempDisconnectionTable[_DisconnectionStack[i]]);
	}
}

//
//
//

void	CUnifiedNetwork::CUnifiedConnection::reset()
{
	ServiceName = "DEAD";
	ServiceId = 0xDEAD;
	IsServerConnection = false;
	EntryUsed = false;
	IsConnected = false;
	IsExternal = false;
	AutoRetry = false;
	SendId = false;
	Connection.CbClient = NULL;
	Connection.HostId = InvalidSockId;
	AutoCheck = false;
}


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
		else if (type == "float")		{ float  v = (float)atof(value.c_str()); msgout.serial (v); }
		else if (type == "double")		{ double v = atof(value.c_str()); msgout.serial (v); }
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

NLMISC_COMMAND(msgin, "Simulate an input message from another service", "<ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*")
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
 * ex: msgin 128 REGISTER u32 10 u32 541 u32 45
 * You'll send a real message REGISTER to the service number 128 with 3 uint32.
 *
 */

NLMISC_COMMAND(msgout, "Send a message to a specified service", "<ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*")
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

} // NLNET
