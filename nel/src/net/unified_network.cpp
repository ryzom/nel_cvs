/** \file unified_network.cpp
 * Network engine, layer 5, base
 *
 * $Id: unified_network.cpp,v 1.5 2001/11/13 11:58:42 lecroart Exp $
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

#include "nel/net/unified_network.h"



using namespace std;
using namespace NLMISC;

namespace NLNET {

// Service registration callbacks

// when a service registers
void	uNetRegistrationBroadcast(const string &name, TServiceId sid, const CInetAddress &addr)
{
}

// when a service unregisters
void	uNetUnregistrationBroadcast(const string &name, TServiceId sid, const CInetAddress &addr)
{
}

// Service up/down callbacks

void	cbConnection(TSockId from, void *arg)
{
	// set the service id to an unknown value, besause we don't know yet
	// which service is connecting at this moment.
	from->setAppId (0xFFFFFF);
}

void	cbDisconnection(TSockId from, void *arg)
{
	if (from->appId() == 0xFFFFFF)
	{
		// the service didn't identified before disconnection
	}
	else
	{
		// get the service connection
		CUnifiedNetwork						*instance = CUnifiedNetwork::getInstance();

		// get an access to the value
		CRWSynchronized<std::vector<CUnifiedNetwork::CUnifiedConnection> >::CReadAccessor	access(&(instance->_IdCnx));

		// now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
		const CUnifiedNetwork::CUnifiedConnection	&cnx = access.value ()[(uint16)from->appId()];

		// get the deconnection callback
		CUnifiedNetwork::TNameMappedCallback::iterator	it = instance->_DownCallbacks.find(cnx.ServiceName);

		if (it != instance->_DownCallbacks.end())
		{
			// call it
			TUnifiedNetCallback	cb = (*it).second.first;
			cb(cnx.ServiceName, cnx.ServiceId, (*it).second.second);
		}

		// call the generic callback
		if (instance->_DownUniCallback.first != NULL)
		{
			instance->_DownUniCallback.first(cnx.ServiceName, cnx.ServiceId, instance->_DownUniCallback.second);
		}

		// adds the connection to the disconnection stack
		instance->_DisconnectionStack.push_back(cnx.ServiceId);
	}

}

//
//
//

void	cbServiceIdentification(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string		inSName;
	uint16		inSid;

	// recover the service name and id
	msgin.serial(inSName);
	msgin.serial(inSid);

	nlinfo("Received identification from service %s %u", inSName.c_str(), inSid);

	// setup the sock with the sid.
	from->setAppId(inSid);

	// add a new connection to the list
	CUnifiedNetwork		*instance = CUnifiedNetwork::getInstance();

	instance->_ConnectionStack.push_back(CUnifiedNetwork::CConnectionId(inSName, inSid, from));
}

// the callbacks wrapper
void	cbMsgProcessing(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	if (from->appId() != 0xFFFFFF)
	{
		CUnifiedNetwork									*inst = CUnifiedNetwork::getInstance();
		uint16											sid = (uint16)from->appId();
		CUnifiedNetwork::TMsgMappedCallback::iterator	itcb;

		itcb = inst->_Callbacks.find(msgin.getName());
		if (itcb == inst->_Callbacks.end())
		{
			// the callback doesn't exist
			nlwarning("L5: can't find callback %s called by service %d", msgin.getName().c_str(), sid);
		}
		else
		{
			// call the callback
			(*itcb).second (msgin, msgin.getName().c_str(), sid);
		}
	}
	else
	{
		// service not yet identified
		nlwarning("L5: received the message %s from a not yet identified service", msgin.getName().c_str());
	}
}


TCallbackItem	ServerCbArray[] =
{
	{ "UN_SIDENT", cbServiceIdentification }
};


//
//
//

void	CUnifiedNetwork::init(const CInetAddress *addr, CCallbackNetBase::TRecordingState rec,
							  const string &shortName, uint16 port, TServiceId &sid)
{
	_RecordingState = rec;
	_Name = shortName;
	_SId = sid;

	// if the address isn't null, uses the naming service
	if (addr != NULL)
	{
		// connect to the naming service (may generate a ESocketConnectionFailed exception)
		CNamingClient::connect(*addr, rec);

		// connect the callback to know when a new service comes in or goes down
		CNamingClient::setRegistrationBroadcastCallback(uNetRegistrationBroadcast);
		CNamingClient::setUnregistrationBroadcastCallback(uNetUnregistrationBroadcast);

		if (port == 0)
			port = CNamingClient::queryServicePort ();
	}

	// setup the server callback
	_ServerPort = port;
	_CbServer.init(port);
	_CbServer.addCallbackArray(ServerCbArray, 1);				// the service ident callback
	_CbServer.setDefaultCallback(cbMsgProcessing);				// the default callback wrapper
	_CbServer.setConnectionCallback(cbConnection, NULL);
	_CbServer.setDisconnectionCallback(cbDisconnection, NULL);
}

void	CUnifiedNetwork::connect()
{
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

		// get the services list
		const list<CNamingClient::CServiceEntry>	&services = CNamingClient::getRegisteredServices();

		// connects to the registered services
		list<CNamingClient::CServiceEntry>::const_iterator	its;
		TNameMappedCallback::iterator						itcb;

		// don't connect to itself
		for (its = services.begin(); its != services.end(); ++its)
			if (_SId != (*its).SId)
				// add service with name, address, ident, not external, service id, and autoretry (obsolete)
				addService((*its).Name, (*its).Addr, true, false, (*its).SId, true);
	}
}

void	CUnifiedNetwork::release()
{
	CRWSynchronized< std::vector<CUnifiedConnection> >::CWriteAccessor	idAccess(&_IdCnx);
	CRWSynchronized<TNameMappedConnection>::CWriteAccessor				nameAccess(&_NamedCnx);

	uint	i;

	// disconnect all clients
	_CbServer.disconnect(NULL);

	// disconnect all connections to servers
	for (i=0; i<idAccess.value().size(); ++i)
		if (idAccess.value()[i].EntryUsed && !idAccess.value()[i].IsServerConnection)
			idAccess.value()[i].Connection.CbClient->disconnect();

	// clear all other data
	idAccess.value().clear();
	nameAccess.value().clear();
	_UpCallbacks.clear();
	_DownCallbacks.clear();
	_Callbacks.clear();
	_UpUniCallback.first = NULL;
	_UpUniCallback.second = NULL;
	_DownUniCallback.first = NULL;
	_DownUniCallback.second = NULL;

}

void	CUnifiedNetwork::addService(const string &name, const CInetAddress &addr, bool sendId, bool external, uint16 sid, bool autoRetry)
{
	if (external)
		sid = _ExtSId++;

	// create a new connection with the service, setup callback and connect
	CCallbackClient	*cbc = new CCallbackClient();
	cbc->setDisconnectionCallback(cbDisconnection, NULL);
	cbc->setDefaultCallback(cbMsgProcessing);
	try
	{
		cbc->connect(addr);
		cbc->getSockId()->setAppId(sid);
	}
	catch (ESocketConnectionFailed &e)
	{
		nlinfo ("L5: can't connect to %s (sid %u) now (%s)", name.c_str(), sid, e.what ());
	}

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
		cnx.IsConnected = true;
		cnx.IsExternal = external;
		cnx.AutoRetry = autoRetry;
		cnx.ExtAddress = addr;
	}

	if (sendId)
	{
		// send identification to the service
		CMessage	msg("UN_SIDENT");
		uint16		ssid = _SId;
		msg.serial(_Name);
		msg.serial(ssid);	// serializes a 16 bits service id
		send(name, msg);
	}

	// call the connection callback associated to this service
	TNameMappedCallback::iterator	itcb = _UpCallbacks.find(name);
	if (itcb != _UpCallbacks.end() && (*itcb).second.first != NULL)
	{
		TUnifiedNetCallback	cb = (*itcb).second.first;
		cb(name, sid, (*itcb).second.second);
	}

	if (_UpUniCallback.first != NULL)
	{
		_UpUniCallback.first(name, sid, _UpUniCallback.second);
	}
}


//
//
//


void	CUnifiedNetwork::update(sint32 timeout)
{
	{
		// lock read access to the connections
		CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
		const vector<CUnifiedConnection>									&connections = idAccess.value();

		_ConnectionRetriesStack.clear();
		_ConnectionStack.clear();
		_DisconnectionStack.clear();

		//
		TTime	newTime = CTime::getLocalTime();
		bool	enableRetry;
		if ((enableRetry = (newTime-_LastRetry > 5000)))
			_LastRetry = newTime;

		// update all connections
		_CbServer.update(timeout);

		uint	i;
		for (i=0; i<connections.size(); ++i)
		{
			const CUnifiedConnection	&cnx = connections[i];
			if (cnx.EntryUsed && !cnx.IsServerConnection)
			{
				cnx.Connection.CbClient->update(timeout);
				if (enableRetry && !cnx.IsConnected && cnx.AutoRetry)
					_ConnectionRetriesStack.push_back(i);
			}
		}
	}

	updateConnectionTable();
}


void	CUnifiedNetwork::updateConnectionTable()
{
	CRWSynchronized<TNameMappedConnection>::CWriteAccessor				nameAccess(&_NamedCnx);
	CRWSynchronized< std::vector<CUnifiedConnection> >::CWriteAccessor	idAccess(&_IdCnx);
	uint	i;

	// try to reconnect
	for (i=0; i<_ConnectionRetriesStack.size(); ++i)
	{
		CUnifiedConnection	&cnx = idAccess.value()[_ConnectionRetriesStack[i]];
		nlassert(!cnx.IsServerConnection);

		try
		{
			cnx.Connection.CbClient->connect(cnx.ExtAddress);
			cnx.IsConnected = true;
		}
		catch (ESocketConnectionFailed &e)
		{
			nlinfo ("L5: can't connect to %s (sid %u) now (%s)", cnx.ServiceName.c_str(), cnx.ServiceId, e.what ());
		}
	}

	// remove all flaged connections
	for (i=0; i<_DisconnectionStack.size(); ++i)
	{
		CUnifiedConnection	&cnx = idAccess.value()[_DisconnectionStack[i]];

		nlassert(cnx.EntryUsed);

		// flag the connection as unavailable
		cnx.IsConnected = false;

		if (!cnx.IsExternal || !cnx.AutoRetry)
		{
			// if the cnx isn't extern or if no autoretry, delete it.
			cnx.EntryUsed = false;

			if (!cnx.IsServerConnection)
				delete cnx.Connection.CbClient;

			TNameMappedConnection::iterator	itnmc = nameAccess.value().find(cnx.ServiceName);
			nlassert(itnmc != nameAccess.value().end());
			nameAccess.value().erase(itnmc);
		}
	}

	// add new connections
	for (i=0; i<_ConnectionStack.size(); ++i)
	{
		if (_ConnectionStack[i].SId >= idAccess.value().size())
			idAccess.value().resize(_ConnectionStack[i].SId+1);

		CUnifiedConnection	&cnx = idAccess.value()[_ConnectionStack[i].SId];

		nlassert(!cnx.EntryUsed);

		cnx = CUnifiedConnection(_ConnectionStack[i].SName, _ConnectionStack[i].SId, _ConnectionStack[i].SHost);
		cnx.IsConnected = true;
		nameAccess.value().insert(make_pair(_ConnectionStack[i].SName, _ConnectionStack[i].SId));

		// get the connection callback
		CUnifiedNetwork::TNameMappedCallback::iterator	it = _UpCallbacks.find(cnx.ServiceName);

		if (it != _UpCallbacks.end())
		{
			// call it
			TUnifiedNetCallback	cb = (*it).second.first;
			cb(cnx.ServiceName, cnx.ServiceId, (*it).second.second);
		}

		if (_UpUniCallback.first != NULL)
		{
			_UpUniCallback.first(cnx.ServiceName, cnx.ServiceId, _UpUniCallback.second);
		}
	}
}

//
//
//

void	CUnifiedNetwork::send(const string &serviceName, const CMessage &msgout)
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
			if (!connection.IsConnected)
				continue;

			found = true;

			if (connection.IsServerConnection)
				_CbServer.send(msgout, connection.Connection.HostId);
			else
				connection.Connection.CbClient->send(msgout);
		}
	}

	// looks for waiting connections
	uint	i;
	for (i=0; i<_ConnectionStack.size(); ++i)
	{
		if (_ConnectionStack[i].SName == serviceName)
		{
			found = true;
			_CbServer.send(msgout, _ConnectionStack[i].SHost);
		}
	}

	if (!found)
		nlwarning("L5: can't find service %s to send message %s", serviceName.c_str(), msgout.getName().c_str());
}

void	CUnifiedNetwork::send(uint16 sid, const CMessage &msgout)
{
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);

	if (sid>=idAccess.value().size() || !idAccess.value()[sid].EntryUsed || !idAccess.value()[sid].EntryUsed)
	{
		uint	i;
		for (i=0; i<_ConnectionStack.size(); ++i)
		{
			if (_ConnectionStack[i].SId == sid)
			{
				_CbServer.send(msgout, _ConnectionStack[i].SHost);
				return;
			}
		}
		nlwarning("L5: incorrect service id %d to send message %s", sid, msgout.getName().c_str());
	}
	else
	{
		const CUnifiedConnection	&connection = idAccess.value()[sid];

		if (connection.IsServerConnection)
			_CbServer.send(msgout, connection.Connection.HostId);
		else
			connection.Connection.CbClient->send(msgout);
	}
}

void	CUnifiedNetwork::send(const CMessage &msgout)
{
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);
	const std::vector<CUnifiedConnection>								&connections = idAccess.value();

	uint	i;
	for (i=0; i<connections.size(); ++i)
		if (connections[i].EntryUsed && connections[i].IsConnected)
			if (connections[i].IsServerConnection)
				_CbServer.send(msgout, connections[i].Connection.HostId);
			else
				connections[i].Connection.CbClient->send(msgout);

	for (i=0; i<_ConnectionStack.size(); ++i)
		_CbServer.send(msgout, _ConnectionStack[i].SHost);
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


void	CUnifiedNetwork::setServiceUpCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg)
{
	if (serviceName == "*")
	{
		_UpUniCallback = make_pair(cb, arg);
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

void	CUnifiedNetwork::setServiceDownCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg)
{
	if (serviceName == "*")
	{
		_DownUniCallback = make_pair(cb, arg);
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
			sent += idAccess.value()[i].Connection.CbClient->getBytesSended();

	sent += _CbServer.getBytesSended();
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

	received += _CbServer.getBytesReceived();
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

	sent += _CbServer.getSendQueueSize();
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

	received += _CbServer.getReceiveQueueSize();
	return received;
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(const std::string &name, TSockId &host)
{
	CRWSynchronized<TNameMappedConnection>::CReadAccessor				nameAccess(&_NamedCnx);
	CRWSynchronized< std::vector<CUnifiedConnection> >::CReadAccessor	idAccess(&_IdCnx);

	sint	count = nameAccess.value().count(name);
	
	if (count <= 0)
	{
		nlwarning("L5: couldn't access the service %s", name.c_str());
		return NULL;
	}
	else if (count > 1)
	{
		nlwarning("L5: more than one service %s to get CCallbackNetBase", name.c_str());
	}

	TNameMappedConnection::const_iterator	itnmc = nameAccess.value().find(name);

	const CUnifiedConnection	&cnx = idAccess.value()[(*itnmc).second];
	if (cnx.IsServerConnection)
	{
		host = cnx.Connection.HostId;
		return &_CbServer;
	}
	else
	{
		return cnx.Connection.CbClient;
	}
}

//
//
//

CUnifiedNetwork	*CUnifiedNetwork::_Instance = NULL;

CUnifiedNetwork	*CUnifiedNetwork::getInstance ()
{
	if (_Instance == NULL)
	{
		_Instance = new CUnifiedNetwork();
	}

	return _Instance;
}

bool CUnifiedNetwork::isUsed ()
{
	return (_Instance != NULL);
}

} // NLNET

