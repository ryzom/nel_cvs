/** \file unified_network.cpp
 * Network engine, layer 5 with no multithread support
 *
 * $Id: unified_network.cpp,v 1.97.4.1 2006/09/21 20:40:58 cado Exp $
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
#include "nel/misc/entity_id.h" // for createMessage()
#include "nel/misc/variable.h"

#include "nel/misc/thread.h"
#include "nel/misc/mutex.h"

#include "nel/net/module_common.h"

#ifdef NL_OS_UNIX
#include <sched.h>
#endif

using namespace std;
using namespace NLMISC;


namespace NLNET {

CLog test(CLog::LOG_INFO);
CFileDisplayer fd;

static uint ThreadCreator = 0;

static const uint64 AppIdDeadConnection = 0xDEAD;

static uint32 TotalCallbackCalled = 0;

#ifdef NL_OS_UNIX
/// Yield method (Unix only)
CVariable<uint32> UseYieldMethod("nel", "UseYieldMethod", "0=select 1=usleep 2=nanosleep 3=sched_yield 4=none", 0, 0, true );
#endif

/// Reduce sending lag
CVariable<bool> FlushSendsBeforeSleep("nel", "FlushSendsBeforeSleep", "If true, send buffers will be flushed before sleep, not in next update", true, 0, true );

/// Network congestion monitoring
CVariable<uint> L5TotalBytesInLowLevelSendQueues("nel", "L5TotalBytesInLowLevelSendQueues", "Number of bytes pending in send queues (postponed by non-blocking send()) for network congestion monitoring. N/A if FlushSendsBeforeSleep disabled)", 0, 0, true );

/// Receiving size limit
CVariablePtr<uint32> DefaultMaxExpectedBlockSize("nel", "DefaultMaxExpectedBlockSize", "If receiving more than this value in bytes, the connection will be dropped", &CBufNetBase::DefaultMaxExpectedBlockSize, true );

/// Sending size limit
CVariablePtr<uint32> DefaultMaxSentBlockSize("nel", "DefaultMaxSentBlockSize", "If sending more than this value in bytes, the program may be stopped", &CBufNetBase::DefaultMaxSentBlockSize, true );


#define AUTOCHECK_DISPLAY nlwarning
//#define AUTOCHECK_DISPLAY CUnifiedNetwork::getInstance()->displayInternalTables (), nlerror

// Small log to help debugging unified network connection events (TODO: remove)
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
		uni->_IdCnx[sid] = CUnifiedNetwork::CUnifiedConnection(name, sid, false);
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

	uni->callServiceDownCallback(uc->ServiceName, uc->ServiceId);

	/*
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
	}*/

	if(!uc->Connections.empty ())
	{
		// set all connection to dead, now, all messages received on this socket will be ignored and closed
		for (uint i = 0; i < uc->Connections.size (); ++i)
		{
			if (uc->Connections[i].valid())
				uc->Connections[i].setAppId (AppIdDeadConnection);
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
		test.displayNL ("-connect dead connection");
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

			if (uc->IsExternal)
			{
				if (!uc->AutoRetry)
				{
					// If it s a external service with no auto retry, remove the connection
					
					// call the user callback
					uni->callServiceDownCallback(uc->ServiceName, uc->ServiceId);
					
					/*CUnifiedNetwork::TNameMappedCallback::iterator	it2 = uni->_DownCallbacks.find(uc->ServiceName);

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
					}*/

					uni->removeNamedCnx (uc->ServiceName, uc->ServiceId);

					uni->_ConnectionToReset.push_back(uc->ServiceId);
//					uc->reset ();
				}
				else
				{
					// call the user callback
					uni->callServiceDownCallback(uc->ServiceName, uc->ServiceId);
				}
			}
			else
			{
				// reset the connection
				uint i;
				for (i = 0; i < uc->Connections.size (); i++)
				{
					if (uc->Connections[i].valid() && uc->Connections[i].CbNetBase->getSockId(uc->Connections[i].HostId) == from)
					{
						if (uc->Connections[i].IsServerConnection)
						{
							// we have to remove the stuffs now because HostId will not be accessible later
							uc->Connections[i].reset();
						}
						else
						{
							// if it s a client, we can't delete now because the callback client is currently in use
							// only disconnect
							if(uc->Connections[i].CbNetBase->connected ())
							{
								uc->Connections[i].CbNetBase->disconnect (uc->Connections[i].HostId);
							}
						}
						break;
					}
				}
				if (i == uc->Connections.size ())
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
	bool isExternal;
	msgin.serial (isExternal);

	nlinfo ("HNETL5: + connec ident '%s' %s-%hu pos %hu ext %d", from->asString().c_str(), inSName.c_str(), inSid, (uint16)pos, (uint8)isExternal);
	
	allstuffs += "+rconnect "+inSName+"-"+toString(inSid)+" pos "+toString((uint16)pos)+"\n";
	test.displayNL ("+rconnect %s-%hu pos %hu", inSName.c_str (), (uint16)inSid, (uint16)pos);

	if(isExternal)
	{
		nlassert (pos == 0);
	}

	if (inSid == 0)
	{
		if (isExternal)
		{
			inSid = CUnifiedNetwork::getInstance ()->_ExtSId++;
			nlwarning ("HNETL5: Received a connection from a service with a SId 0, we give him the SId %d", inSid);
		}
		else
		{
			nlwarning ("HNETL5: Received a connection from a service with a SId 0 and wasn't external, disconnecting it");
			netbase.disconnect();
			return;
		}
	}

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
		uni->_IdCnx[inSid] = CUnifiedNetwork::CUnifiedConnection(inSName, inSid, isExternal);
		uni->_UsedConnection.push_back (inSid);
		break;
	default:
		break;
	}

	if (uni->_IdCnx[inSid].IsExternal != isExternal)
	{
		AUTOCHECK_DISPLAY ("HNETL5: Receive a connection that is not totally external %d %d", uni->_IdCnx[inSid].IsExternal, isExternal);
		return;
	}

	bool FirstConnection = (uni->_IdCnx[inSid].Connections.size () == 0);

	// add the connection to the already inserted unified connection
	if (pos >= uni->_IdCnx[inSid].Connections.size ())
	{
		uni->_IdCnx[inSid].Connections.resize(pos+1);
	}
	uni->_IdCnx[inSid].Connections[pos] = CUnifiedNetwork::CUnifiedConnection::TConnection(&netbase, from);

	// If the connection is external, we'll never receive the ExtAddress by the naming service, so add it manually
	if (isExternal)
	{
		uni->_IdCnx[inSid].ExtAddress.push_back (netbase.hostAddress (from));
		uni->_IdCnx[inSid].setupNetworkAssociation (uni->_NetworkAssociations, uni->_DefaultNetwork);
	}


	// todo ace temp to savoir comment c est possible ce cas la
	if (uni->_IdCnx[inSid].Connections.size() == 3)
	{
		CUnifiedNetwork::CUnifiedConnection *uc = &uni->_IdCnx[inSid];
		nlstop;
		nlinfo ("HNETL5: ext addr %s", vectorCInetAddressToString (uc->ExtAddress).c_str ());
		for(uint i = 0; i < uc->Connections.size(); i++)
			nlinfo ("HNETL5: cnx %s", uc->Connections[i].HostId->asString ().c_str ());
		nlinfo ("HNETL5: %s", allstuffs.c_str ());
	}

	// send the callback to the user with the first connection
	if (FirstConnection)
	{
		// insert the name in the map to be able to send message with the name
		uni->addNamedCnx (inSName, inSid);

		uni->callServiceUpCallback (inSName, inSid);
/*
		// now we warn the user
		CUnifiedNetwork::TNameMappedCallback::iterator	it = uni->_UpCallbacks.find(inSName);
		if (it != uni->_UpCallbacks.end())
		{
			// call it
			for (list<TCallbackArgItem> it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
			{
				TUnifiedNetCallback	cb = (*it2).first;
				if (cb) cb(inSName, inSid, (*it2).second);
			}
		}

		for (uint c = 0; c < uni->_UpUniCallback.size (); c++)
		{
			if (uni->_UpUniCallback[c].first != NULL)
				uni->_UpUniCallback[c].first (inSName, inSid, uni->_UpUniCallback[c].second);
		}*/
	}
}

// the callbacks wrapper
void	uncbMsgProcessing(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	if (from->appId() == AppIdDeadConnection)
	{
		AUTOCHECK_DISPLAY ("HNETL5: Receive a message from a dead connection");
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
		if((*itcb).second == 0)
		{
			nlwarning ("HNETL5: Received message %s from a service %hu but the associated callback is NULL", msgin.getName ().c_str(), sid);
			return;
		}

		{
			static map<string, CHTimer> timers;
			map<string, CHTimer>::iterator it;
			
			{
				H_AUTO(L5UCHTimerOverhead);
				string callbackName = "USRCB_" + msgin.getName();
				it = timers.find(callbackName);
				if(it == timers.end())
				{
					it = timers.insert(make_pair(callbackName, CHTimer(NULL))).first;
					(*it).second.setName((*it).first.c_str());
				}
			}

			{
				H_AUTO(L5UserCallback);
				
				(*it).second.before();
				const std::string &cbName = itcb->first;
				(*itcb).second (msgin, uc->ServiceName, sid);
				(*it).second.after();
			}
		}

		uc->TotalCallbackCalled++;
		TotalCallbackCalled++;
	}
}


TCallbackItem	unServerCbArray[] =
{
	{ "UN_SIDENT", uncbServiceIdentification }
};



//
// Alive check thread
//

class CAliveCheck : public NLMISC::IRunnable
{
public:
	CAliveCheck() : ExitRequired(false)	{ }

	virtual void	run();
	virtual			~CAliveCheck()	{ }

	volatile bool	ExitRequired;

	static CAliveCheck*	Thread;

	struct CCheckAddress
	{
		CCheckAddress() : NeedCheck(false), AddressValid(false), ConnectionId(0xDEAD)	{ }
		CInetAddress	Address;
		std::string		ServiceName;
		uint16			ServiceId;
		uint			ConnectionId;
		uint			ConnectionIndex;
		volatile bool	NeedCheck;
		volatile bool	AddressValid;
	};

	CCheckAddress		CheckList[128];

	void			checkService(CInetAddress address, uint connectionId, uint connectionIndex, std::string service, uint16 id);

};

CAliveCheck*	CAliveCheck::Thread = NULL;
IThread*		AliveThread = NULL;


void	CAliveCheck::run()
{
	// setup thread
	Thread = this;

	TTime	t = CTime::getLocalTime();

	while (!ExitRequired)
	{
		if (CTime::getLocalTime() - t < 10000)
		{
			nlSleep(100);
			continue;
		}

		uint	i;
		for (i=0; i<sizeof(CheckList)/sizeof(CheckList[0]); ++i)
		{
			if (CheckList[i].NeedCheck && !CheckList[i].AddressValid)
			{
				try
				{
					CCallbackClient	cbc;
					cbc.connect(CheckList[i].Address);
					// success (no exception)
					CheckList[i].AddressValid = true;
					cbc.disconnect();
				}
				catch (ESocketConnectionFailed &e)
				{
#if FINAL_VERSION
					nlinfo ("HNETL5: can't connect to %s-%hu now (%s)", CheckList[i].ServiceName.c_str(), CheckList[i].ServiceId, e.what ());
#else
					nlwarning ("HNETL5: can't connect to %s-%hu now (%s)", CheckList[i].ServiceName.c_str(), CheckList[i].ServiceId, e.what ());
#endif
				}
			}
		}

		t = CTime::getLocalTime();
	}

	ExitRequired = false;
}

void	CAliveCheck::checkService(CInetAddress address, uint connectionId, uint connectionIndex, std::string service, uint16 id)
{
	uint	i;
	for (i=0; i<sizeof(CheckList)/sizeof(CheckList[0]); ++i)
	{
		if (CheckList[i].NeedCheck)
			continue;

		CheckList[i].Address = address;
		CheckList[i].ConnectionId = connectionId;
		CheckList[i].ConnectionIndex = connectionIndex;
		CheckList[i].ServiceName = service;
		CheckList[i].ServiceId = id;

		CheckList[i].AddressValid = false;
		CheckList[i].NeedCheck = true;

		return;
	}
}


//
//
//

bool	CUnifiedNetwork::init(const CInetAddress *addr, CCallbackNetBase::TRecordingState rec,
							  const string &shortName, uint16 port, TServiceId &sid)
{
	// the commands can now be invoked
	registerCommandsHandler();

	//DebugLog->addNegativeFilter ("HNETL5");

	if (_Initialised)
	{
		AUTOCHECK_DISPLAY ("HNETL5: Unified network layer already initialized");
		return true;
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

#ifdef NL_OS_UNIX
	/// Init the main pipe to select() on data available
	if ( ::pipe( _MainDataAvailablePipe ) != 0 )
		nlwarning( "Unable to create main D.A. pipe" );
	//nldebug( "Pipe: created" );
#endif

	// setup the server callback only if server port != 0, otherwise there's no server callback
	_ServerPort = port;

	if(_ServerPort != 0)
	{
		nlassert (_CbServer == 0);
		_CbServer = new CCallbackServer( CCallbackNetBase::Off, "", true, false ); // don't init one pipe per connection
#ifdef NL_OS_UNIX
		_CbServer->setExternalPipeForDataAvailable( _MainDataAvailablePipe ); // the main pipe is shared for all connections
		//nldebug( "Pipe: set (server %p)", _CbServer );
#endif
		bool retry = false;
		do 
		{
			retry = false;
			try
			{
				_CbServer->init(port);
			}
			catch (ESocket &)
			{
				nlwarning("Failed to init the listen socket on port %u, is the service already running ?", port);
				// wait a little before retrying
				nlSleep(5000);

				retry = true;
			}
		} while(retry);

		_CbServer->addCallbackArray(unServerCbArray, 1);				// the service ident callback
		_CbServer->setDefaultCallback(uncbMsgProcessing);				// the default callback wrapper
		_CbServer->setConnectionCallback(uncbConnection, NULL);
		_CbServer->setDisconnectionCallback(uncbDisconnection, NULL);
	}
	else
	{
		nlinfo ("HNETL5: ServerPort is 0 so I don't create a CCallbackServer");
	}

	if (CNamingClient::connected())
	{
		// register the service
		for (uint i = 0; i < laddr.size(); i++)
			laddr[i].setPort(_ServerPort);

		if (_SId == 0)
		{
			if ( ! CNamingClient::registerService(_Name, laddr, _SId) )
			{
				nlinfo ("HNETL5: Registration denied");
				return false;
			}
		}
		else
		{
			CNamingClient::registerServiceWithSId(_Name, laddr, _SId);
		}

		sid = _SId;

		nlinfo ("HNETL5: Server '%s' added, registered and listen to port %hu", _Name.c_str (), _ServerPort);
	}

	string fn = _Name+"_"+toString(_SId)+".log";
	fd.setParam (fn);
	test.addDisplayer (&fd);
	test.displayNL ("**************INIT***************");

	AliveThread = IThread::create(new CAliveCheck(), 1024*4);
	AliveThread->start();

	_Initialised = true;
	return true;
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

void	CUnifiedNetwork::release(bool mustFlushSendQueues, const std::vector<std::string>& namesOfOnlyServiceToFlushSending)
{
	if (!_Initialised)
		return;

	// the commands can't be invoked
	unregisterCommandsHandler();

	// terminate the auto reconnection thread
	if (CAliveCheck::Thread)
	{
		CAliveCheck::Thread->ExitRequired = true;
		AliveThread->wait();
//		while (CAliveCheck::Thread->ExitRequired)
//			nlSleep(100);
		delete CAliveCheck::Thread;
		delete AliveThread;
	}

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());


	// Ensure all outgoing data are sent before disconnecting, if requested
	if ( mustFlushSendQueues )
	{
		nlinfo( "HNETL5: Flushing sending queues..." );
		float totalBytes;
		uint bytesRemaining, i=0;
		while ( (bytesRemaining = tryFlushAllQueues( namesOfOnlyServiceToFlushSending )) != 0 )
		{
			if ( i == 0 )
				totalBytes = (float)bytesRemaining;
			if ( i % 20 == 0 )
				nldebug( "%.1f%% of %.3f KB flushed so far", // display without HNETL5 to bypass filters!
					((float)(bytesRemaining-totalBytes))/totalBytes, totalBytes / 1024.0f );
			++i;

			nlSleep( 100 );
		}
		nldebug( "HNETL5: Flush done in %u steps", i+1 );
	}

	// disconnect all clients
	if(_CbServer)
	{
		_CbServer->disconnect(InvalidSockId);
		delete _CbServer;
		_CbServer = 0;
	}

	// disconnect all connections to servers
	for (uint i = 0; i<_IdCnx.size(); ++i)
	{
		if (_IdCnx[i].State != CUnifiedNetwork::CUnifiedConnection::NotUsed)
		{
			for(uint j = 0 ; j < _IdCnx[i].Connections.size (); j++)
			{
				if (_IdCnx[i].Connections[j].valid() && !_IdCnx[i].Connections[j].IsServerConnection)
				{
					if (_IdCnx[i].Connections[j].CbNetBase->connected ())
						_IdCnx[i].Connections[j].CbNetBase->disconnect();
				
					delete _IdCnx[i].Connections[j].CbNetBase;
				}
			}
			_IdCnx[i].Connections.clear ();
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

#ifdef NL_OS_UNIX
	::close( _MainDataAvailablePipe[PipeRead] );
	::close( _MainDataAvailablePipe[PipeWrite] );
#endif
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
		*uc = CUnifiedConnection(name, sid, external);
		_UsedConnection.push_back (sid);
	}
	else
	{
		// If the entry already set, check that all is correct
		if (name != uc->ServiceName) AUTOCHECK_DISPLAY ("HNETL5: name are different in addService %s %s", name.c_str (), uc->ServiceName.c_str ());
		if (sid != uc->ServiceId) AUTOCHECK_DISPLAY ("HNETL5: sid are different in addService %hu %hu", sid, uc->ServiceId);
		if (addr != uc->ExtAddress) AUTOCHECK_DISPLAY ("HNETL5: external addr are different in addService '%s' '%s'", vectorCInetAddressToString(addr).c_str(), vectorCInetAddressToString(uc->ExtAddress).c_str ());
	}
	uc->AutoRetry = autoRetry;
	uc->SendId = sendId;
	uc->ExtAddress = addr;
	nlassert (!addr.empty());

	// associate nid with ext address
	uc->setupNetworkAssociation (_NetworkAssociations, _DefaultNetwork);

	// connect to all connection
	bool	connectSuccess;
	
	if (uc->Connections.size () < addr.size ())
	{
		uc->Connections.resize (addr.size ());
	}

	vector<CInetAddress> laddr = CInetAddress::localAddresses();

	for (uint i = 0; i < addr.size(); i++)
	{
		// first we have to look if we have a network that can established the connection

		uint j = 0;
		// it s 127.0.0.1, it s ok
		if (!addr[i].is127001 ())
		{
			for (j = 0; j < laddr.size (); j++)
			{
				if (laddr[j].internalNetAddress () == addr[i].internalNetAddress ())
				{
					// it's ok, we can try
					break;
				}
			}

			// If we don't found a valid network, we'll try with the first one.
			// It's happen, for example, when you try to connect to a service that is not in the network but use IP translation
			if (j == laddr.size ())
			{
				nlwarning ("HNETL5: I can't access '%s' because I haven't a net card on this network, we'll use the first network", addr[i].asString ().c_str ());
				j = 0;
			}
		}

		// Create a new connection with the service, setup callback and connect
		CCallbackClient	*cbc = new CCallbackClient( CCallbackNetBase::Off, "", true, false ); // don't init one pipe per connection
#ifdef NL_OS_UNIX
		cbc->setExternalPipeForDataAvailable( _MainDataAvailablePipe ); // the main pipe is shared for all connections
		//nldebug( "Pipe: set (client %p)", cbc );
#endif
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
			uc->Connections[i] = CUnifiedNetwork::CUnifiedConnection::TConnection(cbc);

//			nlinfo ("HNETL5: %s", allstuffs.c_str ());
		}

		if (connectSuccess && sendId)
		{
			// send identification to the service
			CMessage	msg("UN_SIDENT");
			msg.serial(_Name);
			uint16		ssid = _SId;
			if (uc->IsExternal)
			{
				// in the case that the service is external, we can't send our sid because the external service can
				// have other connectin with the same sid (for example, LS can have 2 WS with same sid => sid = 0 and leave
				// the other side to find a good number
				ssid = 0;
			}
			msg.serial(ssid);	// serializes a 16 bits service id
			uint8 pos = j;
			msg.serial(pos);	// send the position in the connection table
			msg.serial (uc->IsExternal);
			cbc->send (msg);
		}
	}

	if (addr.size () != uc->Connections.size())
	{
		nlwarning ("HNETL5: Can't connect to all connections to the service %d/%d", addr.size (), uc->Connections.size());
	}

	bool cntok = false;
	for (uint j = 0; j < uc->Connections.size(); j++)
	{
		if (uc->Connections[j].CbNetBase != NULL)
		{
			if (uc->Connections[j].CbNetBase->connected ())
			{
				cntok = true;
				break;
			}
		}
	}

	if (cntok)
	{
		// add the name only if at least one connection is ok
		addNamedCnx (name, sid);

		callServiceUpCallback (name, sid); // global callback ("*") will be called even for external service
		
/*		
		// call the connection callback associated to this service
		TNameMappedCallback::iterator	itcb = _UpCallbacks.find(name);
		if (itcb != _UpCallbacks.end())
		{
			for (list<TCallbackArgItem> it2 = (*itcb).second.begin(); it2 != (*itcb).second.end(); it2++)
			{				
				TUnifiedNetCallback	cb = (*it2).first;
				if (cb) cb(name, sid, (*it2).second);
			}
		}

		if (!external)
		{
			for (uint i = 0; i < _UpUniCallback.size (); i++)
			{
				if (_UpUniCallback[i].first != NULL)
					_UpUniCallback[i].first (name, sid, _UpUniCallback[i].second);
			}
		}
*/	}

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

/*
 *
 */
void	CUnifiedNetwork::update(TTime timeout)
{
	H_AUTO(CUnifiedNetworkUpdate);

	H_BEFORE(UNMisc1);
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
	TTime remainingTime = timeout;

	// check if we need to retry to connect to the client
	if ((enableRetry = (t0-_LastRetry > 5000)))
		_LastRetry = t0;

	H_AFTER(UNMisc1);

	H_BEFORE(UNNamingCheck);
	// Try to reconnect to the naming service if connection lost
	if (_NamingServiceAddr.isValid ())
	{
		if (CNamingClient::connected ())
		{
			CNamingClient::update ();
		}
		else if (enableRetry)
		{
			H_AUTO(L5NSReconnect);
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
	H_AFTER(UNNamingCheck);

	H_BEFORE(UNUpdateCnx);
	while (true)
	{
		H_AUTO(L5OneLoopUpdate);

		if (CAliveCheck::Thread)
		{
			uint	i;
			for (i=0; i<sizeof(CAliveCheck::Thread->CheckList)/sizeof(CAliveCheck::Thread->CheckList[0]); ++i)
			{
				CAliveCheck::CCheckAddress	&address = CAliveCheck::Thread->CheckList[i];
				if (address.NeedCheck && address.AddressValid)
				{
					CUnifiedConnection &uc = _IdCnx[address.ConnectionId];
					if (uc.ServiceName == address.ServiceName &&
						uc.ServiceId == address.ServiceId &&
						uc.ValidRequested)
					{
						uc.ValidRequested = false;
						uc.ValidExternal = true;
					}

					address.NeedCheck = false;
					address.AddressValid = false;
				}
			}
		}

		// update all server connections
		if (_CbServer)
		{
			_CbServer->update2((sint32)remainingTime, 0);
		}
		
		// update all client connections
		for (uint k = 0; k<_UsedConnection.size(); ++k)
		{
			H_AUTO(UNBrowseConnections);
			nlassert (_IdCnx[_UsedConnection[k]].State == CUnifiedNetwork::CUnifiedConnection::Ready);
			for (uint j = 0; j < _IdCnx[_UsedConnection[k]].Connections.size (); j++)
			{
				// WARNING : don't take a reference in the outside loop because
				//			_IdCnx can be resized by execution of a callback
				CUnifiedConnection &uc = _IdCnx[_UsedConnection[k]];
				nlassert(_IdCnx[_UsedConnection[k]].Connections.size() > j);
				CUnifiedConnection::TConnection &conn = _IdCnx[_UsedConnection[k]].Connections[j];
				H_AUTO(UNBrowseSubConnections);
				if (!conn.valid())
					continue;

				if (conn.IsServerConnection)
					continue;

				nlassert(uc.Connections.size() > j);
				if (conn.CbNetBase->connected ())
				{
				nlassert(uc.Connections.size() > j);
					conn.CbNetBase->update2((sint32)remainingTime, 0);
				}
				else if (enableRetry && uc.AutoRetry)
				{
					if (uc.ValidExternal)	
					{
						uc.ValidExternal = false;
						uc.ValidRequested = false;
						autoReconnect( uc, j );
					}
					else if (!uc.ValidRequested && CAliveCheck::Thread)
					{
						uc.ValidRequested = true;
						CAliveCheck::Thread->checkService(uc.ExtAddress[j], _UsedConnection[k], j, uc.ServiceName, uc.ServiceId);
					}
				}
			}
		}

		// reset closed client connection
		for (uint i=0; i<_ConnectionToReset.size(); ++i)
		{
			// remove the _UsedConnection
			bool found = false;
			for (vector<uint16>::iterator it = _UsedConnection.begin (); it != _UsedConnection.end(); it++)
			{
				if (*it == _IdCnx[_ConnectionToReset[i]].ServiceId)
				{
					found = true;
					_UsedConnection.erase (it);
					break;
				}
			}
			if (!found) AUTOCHECK_DISPLAY ("HNETL5: can't find the sid %hu in the _UsedConnection", _IdCnx[_ConnectionToReset[i]].ServiceId);

			_IdCnx[_ConnectionToReset[i]].reset();
		}
		_ConnectionToReset.clear();

		enableRetry = false;

		if ( FlushSendsBeforeSleep.get() )
		{
			// Flush all connections
			L5TotalBytesInLowLevelSendQueues = tryFlushAllQueues();
		}

		//      t0 --------- previousTime -------------------------- t0 + timeout
		//                                    prevRemainingTime
		//
		//      t0 -------------- currentTime ---------------------- t0 + timeout
		//                                        remainingTime
		TTime prevRemainingTime = remainingTime;
		TTime currentTime = CTime::getLocalTime();
		remainingTime = t0 + timeout - currentTime;

		// If it's the end (or if the Unix system time was changed forwards), don't sleep (currentTime > t0 + timeout)
		if ( remainingTime <= 0 )
			break;

		// If the Unix system time was changed backwards, don't wait more than requested and don't provide an erroneous time to the sleep function that would fail (currentTime < previousTime)
		if ( remainingTime > prevRemainingTime )
		{
			// Restart at previousTime
			nldebug( "Backward time sync detected (at least -%"NL_I64"d ms)", remainingTime - prevRemainingTime );
			remainingTime = prevRemainingTime;
			t0 = currentTime - (timeout - remainingTime);
		}

#ifdef NL_OS_UNIX
		// Sleep until the time expires or we receive a message
		H_BEFORE(L5UpdateSleep);
		switch ( UseYieldMethod.get() )
		{
		case 0: sleepUntilDataAvailable( remainingTime ); break; // accurate sleep with select()
		case 1: ::usleep(1000); break; // 20 ms
		case 2: nlSleep(1); break; // 10 ms (by nanosleep, but 20 ms measured on kernel 2.4.20)
		case 3:	::sched_yield(); break; // makes all slow (at least on kernel 2.4.20) !
		default: break; // don't sleep at all, makes all slow!
		}
		H_AFTER(L5UpdateSleep);
#else
		// Enable windows multithreading before rescanning all connections
		H_TIME(L5UpdateSleep, nlSleep(1);); // 0 (yield) would be too harmful to other applications
#endif
	}
	H_AFTER(UNUpdateCnx);

	H_TIME(UNAutoCheck, autoCheck(););
}


/*
 * Auto-reconnect
 */
void CUnifiedNetwork::autoReconnect( CUnifiedConnection &uc, uint connectionIndex )
{
	H_AUTO(L5AutoReconnect);
	try
	{
		CCallbackClient *cbc = (CCallbackClient *)uc.Connections[connectionIndex].CbNetBase;
		cbc->connect(uc.ExtAddress[connectionIndex]);
		uc.Connections[connectionIndex].CbNetBase->getSockId()->setAppId(uc.ServiceId);
		nlinfo ("HNETL5: reconnection to %s-%hu success", uc.ServiceName.c_str(), uc.ServiceId);

		// add the name only if at least one connection is ok
		if (!haveNamedCnx (uc.ServiceName, uc.ServiceId))
			addNamedCnx (uc.ServiceName, uc.ServiceId);

		// resend the identification is necessary
		if (uc.SendId)
		{
			// send identification to the service
			CMessage	msg("UN_SIDENT");
			msg.serial(_Name);

			uint16		ssid = _SId;
			if (uc.IsExternal)
			{
				// in the case that the service is external, we can't send our sid because the external service can
				// have other connection with the same sid (for example, LS can have 2 WS with same sid => sid = 0 and leave
				// the other side to find a good number
				ssid = 0;
			}
			msg.serial(ssid);	// serializes a 16 bits service id
			uint8 pos = connectionIndex;
			msg.serial(pos);	// send the position in the connection table
			msg.serial (uc.IsExternal);
			uc.Connections[connectionIndex].CbNetBase->send (msg, uc.Connections[connectionIndex].HostId);
		}

		// call the user callback
		callServiceUpCallback (uc.ServiceName, uc.ServiceId);
		/*
		CUnifiedNetwork::TNameMappedCallback::iterator	it = _UpCallbacks.find(uc.ServiceName);
		if (it != _UpCallbacks.end())
		{
			// call it
			for (list<TCallbackArgItem> it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
			{
				TUnifiedNetCallback	cb = (*it2).first;
				if (cb) cb(uc.ServiceName, uc.ServiceId, (*it2).second);
			}
		}

		for (uint c = 0; c < _UpUniCallback.size (); c++)
		{
			if (_UpUniCallback[c].first != NULL)
				_UpUniCallback[c].first (uc.ServiceName, uc.ServiceId, _UpUniCallback[c].second);
		}
		*/
	}
	catch (ESocketConnectionFailed &e)
	{
#if FINAL_VERSION
		nlinfo ("HNETL5: can't connect to %s-%hu now (%s)", uc.ServiceName.c_str(), uc.ServiceId, e.what ());
#else
		nlwarning ("HNETL5: can't connect to %s-%hu now (%s)", uc.ServiceName.c_str(), uc.ServiceId, e.what ());
#endif
	}
}

#ifdef NL_OS_UNIX
/*
 *
 */
void CUnifiedNetwork::sleepUntilDataAvailable( TTime msecMax )
{
	// Prevent looping infinitely if an erroneous time was provided
	if ( msecMax > 999 ) // limit not told in Linux man but here: http://docs.hp.com/en/B9106-90009/select.2.html
		msecMax = 999;

	// Prepare for select()
	//SOCKET descmax = 0;
	fd_set readers;
	FD_ZERO( &readers );
	/*
	// Code that would select on one pipe per CCallbackNetBase object. Not needed because we don't check
	// which socket state changed.
	if ( _CbServer )
	{
		// Include the main server queue in the check list
		int pipeDesc = _CbServer->dataAvailablePipeReadHandle();
		FD_SET(	pipeDesc, &readers );
		if ( pipeDesc > descmax )
			descmax = pipeDesc;
	}
	for ( uint k = 0; k!=_UsedConnection.size(); ++k )
	{
		// Include all client queues in the check list
		CUnifiedConnection &uc = _IdCnx[_UsedConnection[k]];
		nlassert (uc.State == CUnifiedNetwork::CUnifiedConnection::Ready);
		for (uint j = 0; j < uc.Connections.size (); j++)
		{
			if (!uc.Connections[j].valid())
				continue;

			if (uc.Connections[j].IsServerConnection)
				continue;

			if (uc.Connections[j].CbNetBase->connected ())
			{
				int pipeDesc = uc.Connections[j].CbNetBase->dataAvailablePipeReadHandle();
				FD_SET( pipeDesc, &readers );
				if ( pipeDesc > descmax )
					descmax = pipeDesc;
			}
		}
	}*/
	FD_SET( _MainDataAvailablePipe[PipeRead], &readers );
	SOCKET descmax = _MainDataAvailablePipe[PipeRead] + 1;

	// Select
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = msecMax * 1000;
	//nldebug( "Select %u ms", (uint)msecMax );
	//TTime before = CTime::getLocalTime();
	int res = ::select( descmax+1, &readers, NULL, NULL, &tv );
	if ( res == -1 )
		nlwarning( "HNETL5: Select failed in sleepUntilDataAvailable");
	//nldebug( "Slept %u ms", (uint)(CTime::getLocalTime()-before) );
}
#endif


//
//
//
uint8 CUnifiedNetwork::findConnectionId (uint16 sid, uint8 nid)
{
	if (_IdCnx[sid].Connections.size () == 0)
	{
		nlwarning ("HNETL5: Can't send message to %s because no connection are available", _IdCnx[sid].ServiceName.c_str ());
		return 0xFF;
	}

	// by default, connection id will be the default one
	uint8 connectionId = _IdCnx[sid].DefaultNetwork;

	if (nid == 0xFF)
	{
		// it s often happen because they didn't set a good network configuration, so it s in debug to disable it easily
		//nldebug ("HNETL5: nid %hu, will use the default connection %hu", (uint16)nid, (uint16)connectionId);
	}
	else if (nid >= _IdCnx[sid].NetworkConnectionAssociations.size())
	{
		nlwarning ("HNETL5: No net association for nid %hu, use the default connection %hu", (uint16)nid, (uint16)connectionId);
	}
	else
	{
		if (_IdCnx[sid].NetworkConnectionAssociations[nid] >= _IdCnx[sid].Connections.size ())
		{
			nlwarning ("HNETL5: Can't send message to %s because nid %d point on a bad connection (%d and only have %d cnx), use default connection", _IdCnx[sid].ServiceName.c_str (), nid, connectionId, _IdCnx[sid].Connections.size ());
		}
		else
		{
			connectionId = _IdCnx[sid].NetworkConnectionAssociations[nid];
		}
	}

	if (connectionId >= _IdCnx[sid].Connections.size() || !_IdCnx[sid].Connections[connectionId].valid() || !_IdCnx[sid].Connections[connectionId].CbNetBase->connected())
	{
		// there's a problem with the selected connectionID, so try to find a valid one
		nlwarning ("HNETL5: Can't find selected connection id %hu to send message to %s because connection is not valid or connected, find a valid connection id", (uint16)connectionId, _IdCnx[sid].ServiceName.c_str ());

		for (connectionId = 0; connectionId < _IdCnx[sid].Connections.size(); connectionId++)
		{
			if (_IdCnx[sid].Connections[connectionId].valid() && _IdCnx[sid].Connections[connectionId].CbNetBase->connected())
			{
				// we found one at last, use this one
				//nldebug ("HNETL5: Ok, we found a valid connectionid, use %hu",  (uint16)connectionId);
				break;
			}
		}

		if (connectionId == _IdCnx[sid].Connections.size())
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

uint	CUnifiedNetwork::send(const string &serviceName, const CMessage &msgout, bool warnIfNotFound, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(const string&, const CMessage&) whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	TNameMappedConnection::const_iterator								it;
	pair<TNameMappedConnection::const_iterator,TNameMappedConnection::const_iterator>	range;
	range = _NamedCnx.equal_range(serviceName);

	uint found = 0;
	if (range.first != _NamedCnx.end())
	{
		for (it=range.first; it!=range.second; ++it)
		{
			uint16	sid = (*it).second;
			if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
			{
				// It often happen when the service is down (connection broke and the naming not already say that it s down)
				// In this case, just warn
				nlwarning ("HNETL5: Can't send %s to the service '%s' because it was in the _NamedCnx but not in _IdCnx (means that the service is down)", msgout.getName().c_str(), serviceName.c_str ());
				return false;
			}

			++found;

			uint8 connectionId = findConnectionId (sid, nid);
			if (connectionId == 0xff)	// failed
			{
				nlwarning ("HNETL5: Can't send %s message to %hu because no connection available", msgout.getName().c_str(), sid);
				continue;
			}

			//nldebug ("HNETL5: send message to %s using nid %d cnx %d / %s", serviceName.c_str (), nid, connectionId, connectionId<_IdCnx[sid].ExtAddress.size ()?_IdCnx[sid].ExtAddress[connectionId].asString().c_str():"???");
			_IdCnx[sid].Connections[connectionId].CbNetBase->send (msgout, _IdCnx[sid].Connections[connectionId].HostId);
		}
	}

	if (!found && warnIfNotFound)
		nlwarning ("HNETL5: can't find service %s to send message %s", serviceName.c_str(), msgout.getName().c_str());

	return found;
}

bool	CUnifiedNetwork::send(uint16 sid, const CMessage &msgout, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(uint16, const CMessage&) whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
	{
		// Happens when trying to send a message to an unknown service id
		nlwarning ("HNETL5: Can't send %s to the service '%hu' because not in _IdCnx", msgout.getName().c_str(), sid);
		return false;
	}

	uint8 connectionId = findConnectionId (sid, nid);
	if (connectionId == 0xff)	// failed
	{
		nlwarning ("HNETL5: Can't send %s to the service '%hu' because no connection available", msgout.getName().c_str(), sid);
		return false;
	}

	_IdCnx[sid].Connections[connectionId].CbNetBase->send (msgout, _IdCnx[sid].Connections[connectionId].HostId);
	return true;
}

void	CUnifiedNetwork::sendAll(const CMessage &msgout, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::send(const CMessage&) whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	uint	i;
	for (i=0; i<_IdCnx.size(); ++i)
	{
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
		{
			uint8 connectionId = findConnectionId (i, nid);
			if (connectionId == 0xff)	// failed
			{
				nlwarning ("HNETL5: Can't send message to %u because no connection available", i);
				continue;
			}

			_IdCnx[i].Connections[connectionId].CbNetBase->send (msgout, _IdCnx[i].Connections[connectionId].HostId);
		}
	}
}


/* Flush all the sending queues, and report the number of bytes still pending.
 * To ensure all data are sent before stopping a service, you may want to repeat
 * calling this method evenly until it returns 0.
 * \param namesOfOnlyServiceToFlushSending If not empty, only the send queues to the
 * services specified (by short name) will be flushed.
 */
uint	CUnifiedNetwork::tryFlushAllQueues(const std::vector<std::string>& namesOfOnlyServiceToFlushSending)
{
	H_AUTO(L5FlushAll);
	uint bytesRemaining = 0;
	for (uint k = 0; k<_UsedConnection.size(); ++k) 
	{ 
		H_AUTO(UNFABrowseConnections); 
		CUnifiedConnection &uc = _IdCnx[_UsedConnection[k]]; 

		// Skip the connection if it is not found in the 'only' list (except if the list is empty)
		if ( (! namesOfOnlyServiceToFlushSending.empty()) &&
			 (std::find( namesOfOnlyServiceToFlushSending.begin(), namesOfOnlyServiceToFlushSending.end(), uc.ServiceName ) == namesOfOnlyServiceToFlushSending.end()) )
			 continue;

		nlassert (uc.State == CUnifiedNetwork::CUnifiedConnection::Ready); 
		for (uint j = 0; j < uc.Connections.size (); j++) 
		{ 
			H_AUTO(UNFABrowseSubConnections); 
			if (!uc.Connections[j].valid()) 
				continue; 
			
			if (uc.Connections[j].CbNetBase->connected ()) 
			{ 
				uint bytesRemainingLocal;
				uc.Connections[j].CbNetBase->flush(uc.Connections[j].HostId, &bytesRemainingLocal); 
				bytesRemaining += bytesRemainingLocal;
			} 
		} 
	}
	return bytesRemaining;
}


//
//
//

void	CUnifiedNetwork::addCallbackArray (const TUnifiedCallbackItem *callbackarray, sint arraysize)
{
	uint	i;

	for (i=0; i<(uint)arraysize; ++i)
		_Callbacks.insert(make_pair(string(callbackarray[i].Key),callbackarray[i].Callback));
}


void	CUnifiedNetwork::setServiceUpCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg, bool back)
{
	nlassert (cb != NULL);
	if (serviceName == "*")
	{
		if (back)
			_UpUniCallback.push_back (make_pair(cb, arg));
		else
			_UpUniCallback.insert (_UpUniCallback.begin(), make_pair(cb, arg));
	}
	else
	{
		if (back)
			_UpCallbacks[serviceName].push_back (make_pair(cb, arg));
		else
			_UpCallbacks[serviceName].insert (_UpCallbacks[serviceName].begin(), make_pair(cb, arg));
	}	
}

void	CUnifiedNetwork::removeServiceUpCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg)
{
	if (serviceName == "*")
	{
		uint i;
		for (i=0; i<_UpUniCallback.size(); ++i)
		{
			if (_UpUniCallback[i].first == cb && _UpUniCallback[i].second == arg)
			{
				// we found it
				_UpUniCallback.erase(_UpUniCallback.begin()+i);
				break;
			}
		}
		if (i == _UpUniCallback.size())
		{
			nlwarning("HNETL5 : can't remove service up callback, not found");
		}
	}
	else
	{
		if (_UpCallbacks.find(serviceName) != _UpCallbacks.end())
		{
			std::list<TCallbackArgItem> &list = _UpCallbacks[serviceName];
			std::list<TCallbackArgItem>::iterator first(list.begin()), last(list.end());
			for (; first != last; ++first)
			{
				if (first->first == cb && first->second == arg)
				{
					list.erase(first);
					break;
				}
			}

			if (first == last)
			{
				nlwarning("HNETL5 : can't remove service up callback, not found");
			}

			if (list.empty())
			{
				// no more callback for this service
				_UpCallbacks.erase(serviceName);
			}
		}
		else
		{
			nlwarning("HNETL5 : can't remove service up callback, not found");
		}
	}	
}

void	CUnifiedNetwork::setServiceDownCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg, bool back)
{
	nlassert (cb != NULL);
	if (serviceName == "*")
	{
		if (back)
			_DownUniCallback.push_back (make_pair(cb, arg));
		else
			_DownUniCallback.insert (_DownUniCallback.begin(), make_pair(cb, arg));
	}
	else
	{
		if (back)
			_DownCallbacks[serviceName].push_back (make_pair(cb, arg));
		else
			_DownCallbacks[serviceName].insert (_DownCallbacks[serviceName].begin(), make_pair(cb, arg));
	}
}

void	CUnifiedNetwork::removeServiceDownCallback (const string &serviceName, TUnifiedNetCallback cb, void *arg)
{
	if (serviceName == "*")
	{
		uint i;
		for (i=0; i<_DownUniCallback.size(); ++i)
		{
			if (_DownUniCallback[i].first == cb && _DownUniCallback[i].second == arg)
			{
				// we found it
				_DownUniCallback.erase(_DownUniCallback.begin()+i);
				break;
			}
		}
		if (i == _DownUniCallback.size())
		{
			nlwarning("HNETL5 : can't remove service down callback, not found");
		}
	}
	else
	{
		if (_DownCallbacks.find(serviceName) != _DownCallbacks.end())
		{
			std::list<TCallbackArgItem> &list = _DownCallbacks[serviceName];
			std::list<TCallbackArgItem>::iterator first(list.begin()), last(list.end());
			for (; first != last; ++first)
			{
				if (first->first == cb && first->second == arg)
				{
					list.erase(first);
					break;
				}
			}

			if (first == last)
			{
				nlwarning("HNETL5 : can't remove service down callback, not found");
			}

			if (list.empty())
			{
				// no more callback for this service
				_DownCallbacks.erase(serviceName);
			}
		}
		else
		{
			nlwarning("HNETL5 : can't remove service down callback, not found");
		}
	}	
}
//
//
//

uint64 CUnifiedNetwork::getBytesSent ()
{
	uint64	sent = 0;
	uint	j;

	for (vector<uint16>::iterator it = _UsedConnection.begin (); it != _UsedConnection.end(); it++)
	{
		if (_IdCnx[(*it)].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[(*it)].Connections.size (); ++j)
				if(_IdCnx[(*it)].Connections[j].valid () && !_IdCnx[(*it)].Connections[j].IsServerConnection)
					sent += _IdCnx[(*it)].Connections[j].CbNetBase->getBytesSent();
	}

/*	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connections.size (); ++j)
				if(_IdCnx[i].Connections[j].valid () && !_IdCnx[i].Connections[j].IsServerConnection)
					sent += _IdCnx[i].Connections[j].CbNetBase->getBytesSent();
*/
	if(_CbServer)
		sent += _CbServer->getBytesSent();
	return sent;
}

uint64 CUnifiedNetwork::getBytesReceived ()
{
	uint64	received = 0;
	uint	j;

	for (vector<uint16>::iterator it = _UsedConnection.begin (); it != _UsedConnection.end(); it++)
	{
		if (_IdCnx[(*it)].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[(*it)].Connections.size (); ++j)
				if(_IdCnx[(*it)].Connections[j].valid () && !_IdCnx[(*it)].Connections[j].IsServerConnection)
					received += _IdCnx[(*it)].Connections[j].CbNetBase->getBytesReceived();
	}
	
/*	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connections.size (); ++j)
				if(_IdCnx[i].Connections[j].valid () && !_IdCnx[i].Connections[j].IsServerConnection)
					received += _IdCnx[i].Connections[j].CbNetBase->getBytesReceived();
*/
	if (_CbServer)
		received += _CbServer->getBytesReceived();
	return received;
}

uint64 CUnifiedNetwork::getSendQueueSize ()
{
	uint64	sent = 0;
	uint	j;

	for (vector<uint16>::iterator it = _UsedConnection.begin (); it != _UsedConnection.end(); it++)
	{
		if (_IdCnx[(*it)].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[(*it)].Connections.size (); ++j)
				if(_IdCnx[(*it)].Connections[j].valid () && !_IdCnx[(*it)].Connections[j].IsServerConnection)
					sent += _IdCnx[(*it)].Connections[j].CbNetBase->getSendQueueSize();
	}

/*	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connections.size (); ++j)
				if(_IdCnx[i].Connections[j].valid () && !_IdCnx[i].Connections[j].IsServerConnection)
					sent += _IdCnx[i].Connections[j].CbNetBase->getSendQueueSize();
*/
	if (_CbServer)
		sent += _CbServer->getSendQueueSize();
	return sent;
}

uint64 CUnifiedNetwork::getReceiveQueueSize ()
{
	uint64	received = 0;
	uint	j;

	for (vector<uint16>::iterator it = _UsedConnection.begin (); it != _UsedConnection.end(); it++)
	{
		if (_IdCnx[(*it)].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[(*it)].Connections.size (); ++j)
				if(_IdCnx[(*it)].Connections[j].valid () && !_IdCnx[(*it)].Connections[j].IsServerConnection)
					received += _IdCnx[(*it)].Connections[j].CbNetBase->getReceiveQueueSize();
	}

/*	for (i=0; i<_IdCnx.size(); ++i)
		if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
			for (j=0; j<_IdCnx[i].Connections.size (); ++j)
				if(_IdCnx[i].Connections[j].valid () && !_IdCnx[i].Connections[j].IsServerConnection)
					received += _IdCnx[i].Connections[j].CbNetBase->getReceiveQueueSize();
*/
	if (_CbServer)
		received += _CbServer->getReceiveQueueSize();
	return received;
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(const std::string &name, TSockId &host, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::getNetBase() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	sint	count = _NamedCnx.count(name);

	if (count <= 0)
	{
		nlwarning ("HNETL5: couldn't access the service %s", name.c_str());
		host = InvalidSockId;
		return NULL;
	}
	else if (count > 1)
	{
		nlwarning ("HNETL5: %d services %s to getNetBase, returns the first valid", count, name.c_str());
	}

	TNameMappedConnection::const_iterator	itnmc = _NamedCnx.find(name);

	uint8 connectionId = findConnectionId ((*itnmc).second, nid);
	if (connectionId == 0xff)	// failed
	{
		nlwarning ("HNETL5: Can't getNetBase %s because no connection available", name.c_str());
		host = InvalidSockId;
		return NULL;
	}

	host = _IdCnx[(*itnmc).second].Connections[connectionId].HostId;
	return _IdCnx[(*itnmc).second].Connections[connectionId].CbNetBase;
}

CCallbackNetBase	*CUnifiedNetwork::getNetBase(uint16 sid, TSockId &host, uint8 nid)
{
	nlassertex(_Initialised == true, ("Try to CUnifiedNetwork::getNetBase() whereas it is not initialised yet"));

	if (ThreadCreator != NLMISC::getThreadId()) nlwarning ("HNETL5: Multithread access but this class is not thread safe thread creator = %u thread used = %u", ThreadCreator, NLMISC::getThreadId());

	if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
	{
		nlwarning ("HNETL5: Can't get net base to the service '%hu' because not in _IdCnx", sid);
		host = InvalidSockId;
		return NULL;
	}

	uint8 connectionId = findConnectionId (sid, nid);
	if (connectionId == 0xff)	// failed
	{
		nlwarning ("HNETL5: Can't getNetBase %hu because no connection available", sid);
		host = InvalidSockId;
		return NULL;
	}

	host = _IdCnx[sid].Connections[connectionId].HostId;
	return _IdCnx[sid].Connections[connectionId].CbNetBase;
}

TUnifiedMsgCallback CUnifiedNetwork::findCallback (const std::string &callbackName)
{
	TMsgMappedCallback::iterator	itcb = _Callbacks.find(callbackName);
	if (itcb == _Callbacks.end())
		return NULL;
	else
		return (*itcb).second;
}

bool CUnifiedNetwork::isServiceLocal (const std::string &serviceName)
{
	// it s me, of course we are local
	if (serviceName == _Name)
		return true;

	pair<TNameMappedConnection::const_iterator,TNameMappedConnection::const_iterator>	range;
	range = _NamedCnx.equal_range(serviceName);

	if (range.first != _NamedCnx.end())
	{
		uint16	sid = (*(range.first)).second;
		return isServiceLocal (sid);
	}

	return false;
}

bool CUnifiedNetwork::isServiceLocal (uint16 sid)
{
	// it s me, of course we are local
	if (sid == _SId)
		return true;

	if (sid >= _IdCnx.size () || _IdCnx[sid].State != CUnifiedNetwork::CUnifiedConnection::Ready)
	{
		return false;
	}

	vector<CInetAddress> laddr = CInetAddress::localAddresses();

	for (uint i = 0; i < laddr.size(); i++)
	{
		for (uint j = 0; j < _IdCnx[sid].ExtAddress.size(); j++)
		{
			if (_IdCnx[sid].ExtAddress[j].is127001 ())
				return true;

			if (_IdCnx[sid].ExtAddress[j].internalIPAddress () == laddr[i].internalIPAddress ())
				return true;
		}
	}
	return false;
}


/*
 * Return the name of the specified service, or "" if not found
 */
std::string			CUnifiedNetwork::getServiceName(uint16 sid)
{
	string s;
	CUnifiedConnection *c = getUnifiedConnection(sid, false);
	if (c)
		s = c->ServiceName;
	return s;
}


/*
 * Return a string identifying the service, using the format "NAME-sid" (or "sid" only if not found)
 */
std::string			CUnifiedNetwork::getServiceUnifiedName(uint16 sid)
{
	string s;
	CUnifiedConnection *c = getUnifiedConnection(sid, false);
	if (c)
		s = c->ServiceName + "-";
	s += toString(sid);
	return s;
}


//
//
//

//CUnifiedNetwork	*CUnifiedNetwork::_Instance = NULL;
NLMISC_SAFE_SINGLETON_IMPL(CUnifiedNetwork);


/*CUnifiedNetwork	*CUnifiedNetwork::getInstance ()
{
	if (_Instance == NULL)
		_Instance = new CUnifiedNetwork();

	return _Instance;
}
*/
bool CUnifiedNetwork::isUsed ()
{
	return (_Instance != NULL);
}

//
//
//

CUnifiedNetwork::CUnifiedConnection	*CUnifiedNetwork::getUnifiedConnection (uint16 sid, bool warn)
{
	if (sid < _IdCnx.size () && _IdCnx[sid].State == CUnifiedConnection::Ready)
	{
		if (sid != _IdCnx[sid].ServiceId)
		{
			AUTOCHECK_DISPLAY ("HNETL5: Sid index %hu is not the same that in the entry %hu", sid, _IdCnx[sid].ServiceId);
			return NULL;
		}
		return &_IdCnx[sid];
	}
	else
	{
		if ( warn )
			nlwarning ("HNETL5: Try to get a bad unified connection (sid %hu is not in the table)", sid);
		return NULL;
	}
}

void	CUnifiedNetwork::autoCheck()
{
	H_AUTO(L5UpdateAutoCheck);
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
		if ((*itn).first != _IdCnx[(*itn).second].ServiceName) 
			AUTOCHECK_DISPLAY ("HLNET5: problem with name synchro between _NameCnx '%s' and _IdCnx '%s' '%d'", 
									(*itn).first.c_str(), 
									_IdCnx[(*itn).second].ServiceName.c_str (), 
									uint32((*itn).second));
		if (_IdCnx[(*itn).second].AutoCheck == 0)  
			AUTOCHECK_DISPLAY ("HLNET5: problem with name synchro between _NameCnx '%s' and _IdCnx '%s' '%d'", 
									(*itn).first.c_str(), 
									_IdCnx[(*itn).second].ServiceName.c_str (), 
									uint32((*itn).second));
		if (_IdCnx[(*itn).second].AutoCheck > 1)  
			AUTOCHECK_DISPLAY ("HLNET5: problem with name synchro between _NameCnx '%s' and _IdCnx '%s' '%d' more than one entry is named with the same name", 
									(*itn).first.c_str(), 
									_IdCnx[(*itn).second].ServiceName.c_str (), 
									uint32((*itn).second));
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
			if (!_IdCnx[i].Connections.empty ()) AUTOCHECK_DISPLAY ("HLNET5: sid %d connection size should be 0 and is %d", i, _IdCnx[i].Connections.size ());
			if (!_IdCnx[i].ExtAddress.empty ()) AUTOCHECK_DISPLAY ("HLNET5: sid %d ext addr size should be 0 and is %d", i, _IdCnx[i].ExtAddress.size ());
			if (_IdCnx[i].AutoCheck != 0) AUTOCHECK_DISPLAY ("HLNET5: sid %d prob with syncro with _NamedCnx", i);
		}
		else if (_IdCnx[i].State == CUnifiedNetwork::CUnifiedConnection::Ready)
		{
			if (_IdCnx[i].ServiceId != i) AUTOCHECK_DISPLAY ("HNETL5: Bad syncro sid index sid entry for %d %d", i, _IdCnx[i].ServiceId);

			if (_IdCnx[i].ServiceName == "DEAD") AUTOCHECK_DISPLAY ("HLNET5: sid %d name should not be DEAD and is '%s'", i, _IdCnx[i].ServiceName.c_str ());
			if (_IdCnx[i].ServiceId == 0xDEAD) AUTOCHECK_DISPLAY ("HLNET5: sid %d sid should not be 0xDEAD and is 0x%X", i, _IdCnx[i].ServiceId);
			if (!_IdCnx[i].ExtAddress.empty () && _IdCnx[i].Connections.size () > _IdCnx[i].ExtAddress.size()) AUTOCHECK_DISPLAY ("HLNET5: sid %d ext addr size should not be 0 and is %d", i, _IdCnx[i].ExtAddress.size ());

			if (_IdCnx[i].AutoRetry == true && _IdCnx[i].Connections.size () > 1) AUTOCHECK_DISPLAY ("HLNET5: sid %d auto retry with more than one connection %d", i, _IdCnx[i].Connections.size ());
			if (_IdCnx[i].AutoRetry == true && _IdCnx[i].IsExternal == false) AUTOCHECK_DISPLAY ("HLNET5: sid %d auto retry with internal connection", i);
			if (_IdCnx[i].AutoRetry == true && _IdCnx[i].Connections[0].valid() == false) AUTOCHECK_DISPLAY ("HLNET5: sid %d auto retry with invalid connection", i);

			for (j = 0; j < _IdCnx[i].Connections.size (); j++)
			{
				if (_IdCnx[i].Connections[j].valid() && !_IdCnx[i].Connections[j].IsServerConnection && _IdCnx[i].Connections[j].CbNetBase->connected () && _IdCnx[i].Connections[j].getAppId() != i) AUTOCHECK_DISPLAY ("HLNET5: sid %d bad appid %"NL_I64"X", i, _IdCnx[i].Connections[j].getAppId());
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
/*			log->displayNL ("> %s-%hu %s %s %s (%d extaddr %d cnx) tcbc %d", _IdCnx[i].ServiceName.c_str (), _IdCnx[i].ServiceId, _IdCnx[i].IsExternal?"ext":"int", _IdCnx[i].AutoRetry?"autoretry":"noautoretry", _IdCnx[i].SendId?"sendid":"nosendid", _IdCnx[i].ExtAddress.size (), _IdCnx[i].Connections.size (), _IdCnx[i].TotalCallbackCalled);
			uint maxc = _IdCnx[i].Connections.size ();
			if(_IdCnx[i].Connections.size () <= _IdCnx[i].ExtAddress.size ())
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
				if(j < _IdCnx[i].Connections.size () && _IdCnx[i].Connections[j].valid())
				{
					if(_IdCnx[i].Connections[j].IsServerConnection)
					{
						ext += "server ";
					}
					else
					{
						ext += "client ";
					}
					ext += _IdCnx[i].Connections[j].CbNetBase->getSockId (_IdCnx[i].Connections[j].HostId)->asString ();
					ext += " appid:" + toString(_IdCnx[i].Connections[j].getAppId());
					if (_IdCnx[i].Connections[j].CbNetBase->connected ())
						ext += " connected";
					else
						ext += " notconnected";
				}
				else
				{
					ext += "notvalid";
				}

				log->displayNL ("     - %s %s", base.c_str (), ext.c_str ());
			}*/

			_IdCnx[i].display (false, log);
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

bool CUnifiedNetwork::haveNamedCnx (const std::string &name, uint16 sid)
{
	CUnifiedNetwork::TNameMappedConnection::iterator											it;
	pair<CUnifiedNetwork::TNameMappedConnection::iterator,CUnifiedNetwork::TNameMappedConnection::iterator>	range;
	range = _NamedCnx.equal_range(name);

	if (range.first != range.second)
	{
		for (it=range.first; it!=range.second && (*it).second!=sid; ++it)
			;
		
		return (it != range.second);
	}
	return false;
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
			AUTOCHECK_DISPLAY ("HNETL5: Try to add 2 times the same connection %s-%hu", name.c_str(), sid);
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
		AUTOCHECK_DISPLAY ("HNETL5: The unified connection %s-%hu wasn't on the _NamedCnx", name.c_str(), sid);
		return;
	}

	// select good service id
	for (it=range.first; it!=range.second && (*it).second!=sid; ++it)
		;

	// assume id exists
	if (it == range.second)
	{
		AUTOCHECK_DISPLAY ("HNETL5: The unified connection %s-%hu wasn't on the _NamedCnx", name.c_str(), sid);
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

void CUnifiedNetwork::callServiceUpCallback (const std::string &serviceName, uint16 sid, bool callGlobalCallback)
{
	// now we warn the user
	CUnifiedNetwork::TNameMappedCallback::iterator	it = _UpCallbacks.find(serviceName);
	if (it != _UpCallbacks.end())
	{
		// call it
		for (list<TCallbackArgItem>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
		{
			TUnifiedNetCallback	cb = (*it2).first;
			if (cb)
				cb(serviceName, sid, (*it2).second);
			else
				nlwarning ("HNETL5: User set an empty callback for '%s' service up", serviceName.c_str());
		}
	}
	
	if(callGlobalCallback)
	{
		for (uint c = 0; c < _UpUniCallback.size (); c++)
		{
			if (_UpUniCallback[c].first != NULL)
				_UpUniCallback[c].first (serviceName, sid, _UpUniCallback[c].second);
			else
				nlwarning ("HNETL5: User set an empty callback for '*' service up");
		}
	}
}

void CUnifiedNetwork::callServiceDownCallback (const std::string &serviceName, uint16 sid, bool callGlobalCallback)
{
	// now we warn the user
	CUnifiedNetwork::TNameMappedCallback::iterator	it = _DownCallbacks.find(serviceName);
	if (it != _DownCallbacks.end())
	{
		// call it
		for (list<TCallbackArgItem>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
		{
			TUnifiedNetCallback	cb = (*it2).first;
			if (cb)
				cb(serviceName, sid, (*it2).second);
			else
				nlwarning ("HNETL5: User set an empty callback for '%s' service down", serviceName.c_str());
		}
	}
	
	if(callGlobalCallback)
	{
		for (uint c = 0; c < _DownUniCallback.size (); c++)
		{
			if (_DownUniCallback[c].first != NULL)
				_DownUniCallback[c].first (serviceName, sid, _DownUniCallback[c].second);
			else
				nlwarning ("HNETL5: User set an empty callback for '*' service down");
		}
	}
}

void CUnifiedNetwork::CUnifiedConnection::display (bool full, CLog *log)
{
	log->displayNL ("> %s-%hu %s %s %s (%d ExtAddr %d Cnx) TotalCb %d", ServiceName.c_str (), ServiceId, IsExternal?"External":"NotExternal",
		AutoRetry?"AutoRetry":"NoAutoRetry", SendId?"SendId":"NoSendId", ExtAddress.size (), Connections.size (), TotalCallbackCalled);
	
	uint maxc = std::max (ExtAddress.size (), Connections.size ());
	
	for (uint j = 0; j < maxc; j++)
	{
		string base;
		if(j < ExtAddress.size ())
		{
			base += ExtAddress[j].asString ();
		}
		else
		{
			base += "NotValid";
		}
		
		string ext;
		if(j < Connections.size () && Connections[j].valid())
		{
			if(Connections[j].IsServerConnection)
			{
				ext += "Server ";
			}
			else
			{
				ext += "Client ";
			}
			ext += Connections[j].CbNetBase->getSockId (Connections[j].HostId)->asString ();
			ext += " AppId:" + toString(Connections[j].getAppId());
			if (Connections[j].CbNetBase->connected ())
				ext += " Connected";
			else
				ext += " NotConnected";
		}
		else
		{
			ext += "NotValid";
		}
		
		log->displayNL ("  - %s %s", base.c_str (), ext.c_str ());
		if(full)
		{
			log->displayNL ("     * ReceiveQueueStat");
			Connections[j].CbNetBase->displayReceiveQueueStat(log);
			log->displayNL ("     * SendQueueStat");
			Connections[j].CbNetBase->displaySendQueueStat(log, Connections[j].HostId);
			log->displayNL ("     * ThreadStat");
			Connections[j].CbNetBase->displayThreadStat(log);
		}
	}
}



//
//
//


//
// Commands
//

bool createMessage (CMessage &msgout, const vector<string> &args, CLog &log)
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
		else if (type == "e")			{ CEntityId e; e.fromString(value.c_str()); msgout.serial(e); }
		else { log.displayNL ("type '%s' is not a valid type", type.c_str()); return false; }
	}
	return true;
}


//
// Commands and Variables
//

NLMISC_CATEGORISED_VARIABLE(nel, uint32, TotalCallbackCalled, "Total callback called number on layer 5");

NLMISC_CATEGORISED_DYNVARIABLE(nel, uint64, SendQueueSize, "current size in bytes of all send queues")
{
	if (get)
	{
		if (!CUnifiedNetwork::isUsed ())
			*pointer = 0;
		else
			*pointer = CUnifiedNetwork::getInstance()->getSendQueueSize();
	}
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, uint64, ReceiveQueueSize, "current size in bytes of all receive queues")
{
	if (get)
	{
		if (!CUnifiedNetwork::isUsed ())
			*pointer = 0;
		else
			*pointer = CUnifiedNetwork::getInstance()->getReceiveQueueSize();
	}
}


NLMISC_CATEGORISED_DYNVARIABLE(nel, uint64, ReceivedBytes, "total of bytes received by this service")
{
	if (get)
	{
		if (!CUnifiedNetwork::isUsed ())
			*pointer = 0;
		else
			*pointer = CUnifiedNetwork::getInstance()->getBytesReceived ();
	}
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, uint64, SentBytes, "total of bytes sent by this service")
{
	if (get)
	{
		if (!CUnifiedNetwork::isUsed ())
			*pointer = 0;
		else
			*pointer = CUnifiedNetwork::getInstance()->getBytesSent ();
	}
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

NLMISC_CATEGORISED_COMMAND(nel, msgin, "Simulate an input message from another service (ex: msgin 128 REGISTER u32 10 b 1 f 1.5)", "<ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*")
{
	if(args.size() < 2) return false;
	
	if (!CUnifiedNetwork::isUsed ())
	{
		log.displayNL("Can't do that because the service doesn't use CUnifiedNetwork");
		return false;
	}

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
//	msg.clear ();

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

NLMISC_CATEGORISED_COMMAND(nel, msgout, "Send a message to a specified service (ex: msgout 128 REGISTER u32 10 b 1 f 1.5)", "<ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*")
{
	if(args.size() < 2) return false;

	if (!CUnifiedNetwork::isUsed ())
	{
		log.displayNL("Can't do that because the service doesn't use CUnifiedNetwork");
		return false;
	}

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
	
NLMISC_CATEGORISED_COMMAND(nel, l5QueuesStats, "Displays queues stats of network layer5", "")
{
	if(args.size() != 0) return false;
	
	if (!CUnifiedNetwork::isUsed ())
	{
		log.displayNL("Can't display internal table because layer5 is not used");
		return false;
	}
	
	log.displayNL ("%u Unified Connections:", CUnifiedNetwork::getInstance()->_IdCnx.size ());
	for (uint i = 0; i < CUnifiedNetwork::getInstance()->_IdCnx.size (); i++)
	{
		if(CUnifiedNetwork::getInstance()->_IdCnx[i].State != CUnifiedNetwork::CUnifiedConnection::NotUsed)
		{
			CUnifiedNetwork::getInstance()->_IdCnx[i].display (true, &log);
		}
	}
	
	return true;
}

	
NLMISC_CATEGORISED_COMMAND(nel, l5InternalTables, "Displays internal table of network layer5", "")
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

NLMISC_CATEGORISED_COMMAND(nel, l5Callback, "Displays all callback registered in layer5", "")
{
	if(args.size() != 0) return false;

	if (!CUnifiedNetwork::isUsed ())
	{
		log.displayNL("Can't display internal table because layer5 is not used");
		return false;
	}
	
	log.displayNL ("There're %d registered callbacks:", CUnifiedNetwork::getInstance()->_Callbacks.size());
	uint i = 0;
	for (CUnifiedNetwork::TMsgMappedCallback::iterator it = CUnifiedNetwork::getInstance()->_Callbacks.begin(); it != CUnifiedNetwork::getInstance()->_Callbacks.end(); it++)
	{
		log.displayNL (" %d '%s' %s", i++, (*it).first.c_str(), ((*it).second == NULL?"have a NULL address":""));
	}
	
	return true;
}

NLMISC_CATEGORISED_COMMAND(nel, isServiceLocal, "Says if a service is local or not compare with this service", "<sid>|<service name>")
{
	if(args.size() != 1) return false;

	if (!CUnifiedNetwork::isUsed ())
	{
		log.displayNL("Can't do that because the service doesn't use CUnifiedNetwork");
		return false;
	}

	uint16 sid = atoi (args[0].c_str ());
	if (sid > 0)
	{
		log.displayNL ("Service %s-%hu and sid %s are %son the same computer", CUnifiedNetwork::getInstance ()->_Name.c_str(), (uint16)CUnifiedNetwork::getInstance ()->_SId, args[0].c_str(), CUnifiedNetwork::getInstance ()->isServiceLocal (sid)?"":"not ");
	}
	else
	{
		log.displayNL ("Service %s-%hu and %s are %son the same computer", CUnifiedNetwork::getInstance ()->_Name.c_str(), (uint16)CUnifiedNetwork::getInstance ()->_SId, args[0].c_str(), CUnifiedNetwork::getInstance ()->isServiceLocal (args[0])?"":"not ");
	}

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CUnifiedNetwork, addService)
{
	TParsedCommandLine pcl;
	pcl.parseParamList(rawCommandString);

	if (pcl.SubParams.size() != 2)
		return false;

	// syntax is as follow : 
	// <serviceName> ( address=<address:port> [sid=<serviceId>] [sendId] [external] [autoRetry] )

	TParsedCommandLine * serviceInfo = pcl.SubParams[1];
	const TParsedCommandLine *address = serviceInfo->getParam("address");
	if (address == NULL)
	{
		log.displayNL("Can't find param 'address'");
		return false;
	}

	CInetAddress ia(address->ParamValue);;
	if (!ia.isValid())
	{
		log.displayNL("Can't parse internet address in '%s'", address->ParamValue.c_str());
		return false;
	}

	uint16 serviceId = 0;
	const TParsedCommandLine *sid = serviceInfo->getParam("sid");
	if (sid != NULL)
		serviceId = atoi(sid->ParamValue.c_str());

	bool sendId = serviceInfo->getParam("sendId") != NULL;
	bool external = serviceInfo->getParam("external") != NULL;
	bool autoRetry = serviceInfo->getParam("autoRetry") != NULL;

	log.displayNL("Adding service '%s' as sid %u with [sendId = %s], [external = %s], [autoRetry = %s]",
		serviceInfo->ParamName.c_str(),
		sid,
		sendId ? "YES" : "NO",
		external ? "YES" : "NO",
		autoRetry ? "YES" : "NO"
		);

	addService(serviceInfo->ParamName, 
				ia, 
				sendId, 
				external, 
				serviceId,
				autoRetry, 
				false);

	return true;

}


} // NLNET
