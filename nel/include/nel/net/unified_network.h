/** \file unified_network.h
 * Network engine, layer 5 with no multithread support
 *
 * $Id: unified_network.h,v 1.48.2.1 2005/04/22 09:56:22 legros Exp $
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

#ifndef NL_UNIFIED_NETWORD_H
#define NL_UNIFIED_NETWORD_H

#include "nel/misc/types_nl.h"

#include <hash_map>
#include <vector>

#include "nel/misc/time_nl.h"
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"
#include "nel/net/naming_client.h"

namespace NLNET {

/** Callback function type for service up/down processing
 * \param serviceName name of the service that is un/registered to the naming service
 * \param arg a pointer initialized by the user
 */
typedef void (*TUnifiedNetCallback) (const std::string &serviceName, uint16 sid, void *arg);

/** Callback function type for message processing
 * \param msgin message received
 * \param serviceName name of the service that sent the message
 * \param sid id of the service that sent the message
 */
typedef void (*TUnifiedMsgCallback) (CMessage &msgin, const std::string &serviceName, uint16 sid);

/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
struct TUnifiedCallbackItem
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	char				*Key;
	/// The callback function
	TUnifiedMsgCallback	Callback;

};

/**
 * Layer 5
 *
 * When calling send(), a message is stored in a queue. It will be effectively sent
 * when a flush is done. By default, the variable FlushSendsBeforeSleep is on so that
 * the message is sent in the same update cycle as the send() call. If FlushSendsBeforeSleep
 * is set to off, more messages will be sent together, but with a greater delay.
 *
 * Handling network congestion:
 * When a destination service is not fast enough to process the incoming messages,
 * the uploading stream can get saturated. As a result, a single flush will not manage to send the
 * entire data at a time. The NeL Network Layer 5 will silently continue streaming up the remaining
 * parts of the unsent data, as long as update() is called evenly.
 * More in depth:
 * - Either let the layer 5 handle network congestions. If the destination service is slow or
 * stuck and the source service still has data to send, the delivery may be delayed for a long
 * time. If your source service is shut down (calling release()) while some data is still pending,
 * a call to release(true) may take a while or even not exit while the destination service is stuck.
 * - Or handle yourself network congestions by calling tryFlushAllQueues() multiple times. Then you can display
 * the progression of sending, abort it if too long, etc.
 *
 * \author Vianney Lecroart, Benjamin Legros, Olivier Cado
 * \author Nevrax France
 * \date 2002-2004
 */
class CUnifiedNetwork
{
public:

	/** Returns the singleton instance of the CUnifiedNetwork class.
	 */
	static CUnifiedNetwork *getInstance ();

	/** Returns true if the application called getInstance(). This function is used to know if the user is using layer 4 or layer 5
	 */
	static bool isUsed ();

	/** Creates the connection to the Naming Service.
	 * If the connection failed, ESocketConnectionFailed exception is generated.
	 * This function is called automatically called by the service class at the beginning.
	 *
	 * \param addr address of the naming service (0 is you don't want to use the naming service)
	 * \param rec recording state to know if we have to record or replay messages
	 * \return false if the instance startup was denied by the naming service
	 */
	bool	init (const CInetAddress *addr, CCallbackNetBase::TRecordingState rec, const std::string &shortName, uint16 port, TServiceId &sid );

	/** Registers to the Naming Service, and connects to the present services
	 */
	void	connect();

	/** Closes the connection to the naming service, every other connection and free.
	 * \param mustFlushSendQueues If true, all send queues or only queues in namesOfOnlyServiceToFlushSending
	 * will be really sent before disconnecting. In some cases disconnect(true) can take a while.
	 * \param namesOfOnlyServiceToFlushSending When mustFlushSendQueues is true, this param can hold
	 * a list of service short names. Only the send queues to the services specified
	 * will be flushed (and waited) at exit. If the list is empty (and mustFlushSendQueues is true),
	 * all the queues will be flushed (and waited).
	 * See also "Handling network congestion" in CUnifiedNetwork above comments, and tryFlushAllQueues().
	 */
	void	release (bool mustFlushSendQueues=true, const std::vector<std::string>& namesOfOnlyServiceToFlushSending=std::vector<std::string>() );

	/** Adds a specific service to the list of connected services.
	 */
	void	addService(const std::string &name, const CInetAddress &addr, bool sendId = true, bool external = true, uint16 sid=0, bool autoRetry = true, bool shouldBeAlreayInserted = false);
	void	addService(const std::string &name, const std::vector<CInetAddress> &addr, bool sendId = true, bool external = true, uint16 sid=0, bool autoRetry = true, bool shouldBeAlreayInserted = false);

	/** Adds a callback array in the system. You can add callback only *after* adding the server, the client or the group.
	 */
	void	addCallbackArray (const TUnifiedCallbackItem *callbackarray, sint arraysize);

	/** Call it evenly. the parameter select the timeout value in seconds for each update. You are absolutely certain that this
	 * function will not be returns before this amount of time you set.
	 * If you set the update timeout value higher than 0, all messages in queues will be process until the time is greater than the timeout user update().
	 * If you set the update timeout value to 0, all messages in queues will be process one time before calling the user update(). In this case, we don't nlSleep(1).
	 */
	void	update (NLMISC::TTime timeout = 0);

	/** Sends a message to a specific serviceName. If there's more than one service with this name, all services of this name will receive the message.
	 * \param serviceName name of the service you want to send the message (may not be unique.)
	 * \param msg the message you want to send.
	 * \param nid Network Id specify to which network the message must pass throw (0xFF mean the default network)
	 * \return the number of service instances found (may be counter even if the sending failed)
	 */
	uint	send (const std::string &serviceName, const CMessage &msg, bool warnIfNotFound=true, uint8 nid=0xFF);

	/** Sends a message to a specific serviceId.
	 * \param serviceId Id of the service you want to send the message.
	 * \param msg the message you want to send.
	 * \param nid Network Id specify to which network the message must pass throw (0xFF mean the default network)
	 * \return true if the service was found (may return true even if the sending failed)
	 */
	bool	send (uint16 serviceId, const CMessage &msg, uint8 nid=0xFF);

	/** Broadcasts a message to all connected services.
	 * \param msg the message you want to send.
	 * \param nid Network Id specify to which network the message must pass throw (0xFF mean the default network)
	 */
	void	sendAll (const CMessage &msg, uint8 nid=0xFF);

	/** Flush all or part of the sending queues, and report the number of bytes still pending.
	 * To ensure manually all data are sent before stopping a service, you may want
	 * to repeat calling this method evenly until it returns 0. The default release(false) of
	 * CUnifiedNetwork only flushes each connection once, but if the network is
	 * congested (when there are big streams to send) the first flush may not
	 * succeed to send entire buffers.
	 * \param namesOfOnlyServiceToFlushSending If not empty, only the send queues to the
	 * services specified (by short name) will be flushed.
	 * See also "Handling network congestion" in CUnifiedNetwork above comments.
	 */
	uint	tryFlushAllQueues(const std::vector<std::string>& namesOfOnlyServiceToFlushSending=std::vector<std::string>());

	/** Sets callback for incoming connections.
	 * On a client, the callback will be call when the connection to the server is established (the first connection or after the server shutdown and started)
	 * On a server, the callback is called each time a new client is connected to him
	 * 
	 * You can set more than one callback, each one will be called one after one.
	 * If the serviceName is "*", the callback will be call for any services
	 * If you set the same callback for a specific service S and for "*", the callback might be
	 * call twice (in case the service S is up)
	 *
	 * \param back if true, put the callback at the end of the callback array, otherwise but on the beginning. You should always use true
	 */
	void	setServiceUpCallback (const std::string &serviceName, TUnifiedNetCallback cb, void *arg = 0, bool back=true);

	/** Sets callback for disconnections.
	 * On a client, the callback will be call each time the connection to the server is lost.
	 * On a server, the callback is called each time a client is disconnected.
	 * 
	 * You can set more than one callback, each one will be called one after one.
	 * If the serviceName is "*", the callback will be call for any services
	 * If you set the same callback for a specific service S and for "*", the callback might be
	 * call twice (in case the service S is down)
	 *
	 * \param back if true, put the callback at the end of the callback array, otherwise but on the beginning. You should always use true
	 */
	void	setServiceDownCallback (const std::string &serviceName, TUnifiedNetCallback cb, void *arg = 0, bool back=true);

	/** Associate a string with a network id
	 * If the send don't set a specific nid, it ll use the 0, so be sure that the nid 0 is set to a network.
	 * You must call this function before the connect() function.
	 * \param networkName must be in a xxx.xxx.xxx.xxx format. The low value will be ignore depending of the network class.
	 * \param nid a number (used as an index in a vector for constant access so numbers should be contiguous) that will be use to send to this network.
	 */
	void	addNetworkAssociation (const std::string &networkName, uint8 nid);

	/** Clear all network association */
	void	clearNetworkAssociation () { _NetworkAssociations.clear (); }

	/** This array says to wich network we need to send the message for the default nid.
	 * For example you can says that message for AES will use the network 0 and message for LS will use the network 1.
	 * To do that, just call the function with string "AES0" and "LS1" the number is the nid (look at addNetworkAssociation())
	 * addNetworkAssociation("192.168.0.0", 0); addNetworkAssociation("192.168.1.0", 1);
	 * In this case if you send a message to AES with default nid, it'll be send to 192.168.0.x
	 * In this case if you send a message to LS with default nid, it'll be send to 192.168.1.y
	 */
	void	addDefaultNetwork (const std::string &defnet) { nlinfo ("HNETL5: Add default network '%s'", defnet.c_str()); _DefaultNetwork.push_back (defnet); }

	/// Clear all default network
	void	clearDefaultNetwork () { _DefaultNetwork.clear (); }

	/// Returns true if the sid service is on the same computer than this service
	bool	isServiceLocal (uint16 sid);

	/// Returns true if the serviceName service is on the same computer than this service
	bool	isServiceLocal (const std::string &serviceName);

	/// Return the name of the specified service, or "" if not found
	std::string			getServiceName(uint16 sid);

	/// Return a string identifying the service, using the format "NAME-sid" (or "sid" only if not found)
	std::string			getServiceUnifiedName(uint16 sid);

	
	/// \warning You should not use getNetBase functions because it could have more than one connection to a service and in this case
	///          it ll return the first connection

	/// Gets the CCallbackNetBase of the service
	CCallbackNetBase	*getNetBase(const std::string &name, TSockId &host, uint8 nid=0xFF);

	/// Gets the CCallbackNetBase of the service
	CCallbackNetBase	*getNetBase(uint16 sid, TSockId &host, uint8 nid=0xFF);

	/// Gets the total number of bytes sent
	uint64				getBytesSent ();

	/// Gets the total number of bytes received
	uint64				getBytesReceived ();

	/// Gets the total number of bytes queued for sending
	uint64				getSendQueueSize ();

	/// Gets the total number of bytes queued after receiving
	uint64				getReceiveQueueSize ();

	/// Find a callback in the array
	TUnifiedMsgCallback findCallback (const std::string &callbackName);

	/// Return the service ids of the active connections
	const std::vector<uint16>&	getConnectionList() const { return _UsedConnection; }

	void				displayInternalTables (NLMISC::CLog *log = NLMISC::InfoLog);

	void				displayUnifiedConnection (uint16 sid, NLMISC::CLog *log = NLMISC::InfoLog)
	{
		getUnifiedConnection(sid)->display(false, log);
	}

private:

	/// A map of service ids, referred by a service name
	struct TNameMappedConnection : public std::hash_multimap<std::string, uint16> {};

	/// A map of callbacks, refered by message name
	typedef std::map<std::string, TUnifiedMsgCallback>			TMsgMappedCallback;

	/// A callback and its user data
	typedef std::pair<TUnifiedNetCallback, void *>				TCallbackArgItem;

	/// A map of service up/down callbacks with their user data.
	typedef std::hash_map<std::string, std::list<TCallbackArgItem> >	TNameMappedCallback;



	/// This may contains a CCallbackClient or a TSockId, depending on which type of connection it is.
	class CUnifiedConnection
	{
	public:
		/// NotUsed    = the unified connection is empty, not used
		/// Ready      = we can use the unified connection
		enum TState { NotUsed, Ready };

		/// The connection structure
		struct TConnection
		{
			/// If the current service is connect to the other service as a server or a client
			bool				 IsServerConnection;
			/// If it s a client connection, it's the client connection otherwise it s the server connection
			CCallbackNetBase	*CbNetBase;
			/// If it s a server connection, it's the host id, it s InvalidId if it s a client
			TSockId				 HostId;

			TConnection() : IsServerConnection(false), CbNetBase(NULL), HostId(InvalidSockId) { }
			TConnection(CCallbackClient *cbc) : IsServerConnection(false), CbNetBase(cbc), HostId(InvalidSockId) { }
			TConnection(CCallbackNetBase *cbnb, TSockId hi) : IsServerConnection(true), CbNetBase(cbnb), HostId(hi) { }

			void setAppId (uint64 appid) { CbNetBase->getSockId (HostId)->setAppId (appid); }
			uint64 getAppId () { return CbNetBase->getSockId (HostId)->appId (); }
			
			bool valid ()
			{
				if(IsServerConnection)
					return CbNetBase != 0 && HostId != InvalidSockId;
				else
					return CbNetBase != 0;
			}

			void reset ()
			{
				if (CbNetBase != 0)
				{
					if (IsServerConnection)
					{
						if (HostId != InvalidSockId)
							CbNetBase->disconnect (HostId);
					}
					else
					{
						CbNetBase->disconnect ();
						delete CbNetBase;
					}
				}
				CbNetBase = 0;
				IsServerConnection = false;
				HostId = InvalidSockId;
			}
		};


		/// The name of the service (may not be unique)
		std::string					ServiceName;
		/// The id of the service (is unique)
		uint16						ServiceId;
		/// If the service entry is used
		TState						State;
		/// If the connection is extern to the naming service
		bool						IsExternal;
		/// Auto-retry mode
		bool						AutoRetry;
		/// Valid External
		bool						ValidExternal;
		/// Validation Requested
		bool						ValidRequested;
		/// Auto identify at connection
		bool						SendId;
		/// Used for debug purpose
		uint						AutoCheck;
		/// The external connection address
		std::vector<CInetAddress>	ExtAddress;
		/// Connection to the service (me be > 1)
		std::vector<TConnection>	Connection;
		/// This is used to associate a nid (look addNetworkAssociation) with a TConnection.
		std::vector<uint8>			NetworkConnectionAssociations;
		/// This contains the connection id that will be used for default network, it's a connection id used for Connection index
		uint8						DefaultNetwork;

		uint32						TotalCallbackCalled;

		CUnifiedConnection() { reset(); }

		CUnifiedConnection(const std::string &name, uint16 id, bool isExternal) 
		{
			reset ();
			ServiceName = name;
			ServiceId = id;
			State = Ready;
			IsExternal = isExternal;
		}

		CUnifiedConnection(const std::string &name, uint16 id, CCallbackClient *cbc)
		{
			reset ();
			ServiceName = name;
			ServiceId = id;
			State = Ready;
			Connection.push_back(TConnection (cbc));
		}

		void display (bool full, NLMISC::CLog *log = NLMISC::InfoLog);

		void reset()
		{
			ServiceName = "DEAD";
			ServiceId = 0xDEAD;
			State = NotUsed;
			IsExternal = false;
			ValidExternal = false;
			ValidRequested = false;
			AutoRetry = false;
			SendId = false;
			AutoCheck = false;
			ExtAddress.clear ();
			for (uint i = 0; i < Connection.size (); i++)
				Connection[i].reset();
			Connection.clear ();
			DefaultNetwork = 0xDD;
			NetworkConnectionAssociations.clear();
			TotalCallbackCalled = 0;
		}

		// this function wrap the globa default network and network asssociation with this specific connection because they can have
		// different index
		void setupNetworkAssociation (const std::vector<uint32> &networkAssociations, const std::vector<std::string> &defaultNetwork)
		{
			for (uint8 i = 0; i < networkAssociations.size (); i++)
			{
				uint8 j;
				for (j = 0; j < ExtAddress.size (); j++)
				{
					if (ExtAddress[j].internalNetAddress() == networkAssociations[i])
					{
						// we found an association, add it
						if (i >= NetworkConnectionAssociations.size ())
							NetworkConnectionAssociations.resize (i+1);

						NetworkConnectionAssociations[i] = j;
						nlinfo ("HNETL5: nid %hu will be use connection %hu", (uint16)i, (uint16)j);
						break;
					}
				}
				if (j == ExtAddress.size ())
				{
					nlinfo ("HNETL5: nid %hu is not found", (uint16)i);
				}
			}
			// find the default network
			uint j;
			for (j = 0; j < defaultNetwork.size (); j++)
			{
				uint32 pos = defaultNetwork[j].find(ServiceName);
				if (pos != std::string::npos && pos == 0 && ServiceName.size() == defaultNetwork[j].size ()-1)
				{
					uint8 nid = defaultNetwork[j][defaultNetwork[j].size ()-1] - '0';
					DefaultNetwork = NetworkConnectionAssociations[nid];
					nlinfo ("HNETL5: default network for '%s' will be nid %hu and connection id %hu", ServiceName.c_str(), (uint16)nid, (uint16)DefaultNetwork);
					break;
				}
			}
			if (j == defaultNetwork.size ())
			{
				if (NetworkConnectionAssociations.size ()>0)
					DefaultNetwork = NetworkConnectionAssociations[0];
				else
					DefaultNetwork = 0;

				if (defaultNetwork.size () > 0)
					nlwarning ("HNETL5: default network not found in the array, will use connection id %hu", (uint16)DefaultNetwork);
			}
		}
	};

protected:

	/// Auto-reconnect
	void				autoReconnect( CUnifiedConnection &uc, uint connectionIndex );

#ifdef NL_OS_UNIX
	/// Sleep (implemented by select())
	void				sleepUntilDataAvailable( NLMISC::TTime msecMax );
#endif

private:

	/// Vector of connections by service id (sid is the entry in this array, it means that there s some hole)
	std::vector<CUnifiedConnection>				_IdCnx;

	/// This vector contains only an index to the unified connection. It is used to have quick access on the available connections
	std::vector<uint16>							_UsedConnection;

	/// Map of connections by service name
	TNameMappedConnection						_NamedCnx;

	/// The callback server
	CCallbackServer								*_CbServer;

	/// Map of the up/down service callbacks
	TNameMappedCallback							_UpCallbacks;
	std::vector<TCallbackArgItem>				_UpUniCallback;
	TNameMappedCallback							_DownCallbacks;
	std::vector<TCallbackArgItem>				_DownUniCallback;

	/// Recording state
	CCallbackNetBase::TRecordingState			_RecordingState;

	/// Service name
	std::string									_Name;

	/// Map of callbacks
	TMsgMappedCallback							_Callbacks;

	/// The server port
	uint16										_ServerPort;

	/// Used for external service
	uint16										_ExtSId;

	/// Last time of retry
	NLMISC::TTime								_LastRetry;

	/// Time of the theorical next update
	NLMISC::TTime								_NextUpdateTime;

	/// The main instance
	static CUnifiedNetwork						*_Instance;

	/// Naming service
	NLNET::CInetAddress							_NamingServiceAddr;

	/// for each nid, which network address
	std::vector<uint32>							_NetworkAssociations;

	/// for each services, which network to take
	std::vector<std::string>					_DefaultNetwork;

#ifdef NL_OS_UNIX
	/// Pipe to select() on data available (shared among all connections)
	int											_MainDataAvailablePipe [2];
#endif

	/// Service id of the running service
	TServiceId									_SId;

	/// true if initialisation function called
	bool										_Initialised;

	//
	CUnifiedNetwork() : _CbServer(0), _ExtSId(256), _LastRetry(0), _NextUpdateTime(0), _Initialised(false)
	{
	}

	~CUnifiedNetwork() { }
	
	//
	void	autoCheck();

	// Return the unified connection if available or NULL.
	// Don't keep the pointer because it can be invalid if the table is resized.
	CUnifiedConnection	*getUnifiedConnection (uint16 sid, bool warn=true);

	bool haveNamedCnx (const std::string &name, uint16 sid);
	void addNamedCnx (const std::string &name, uint16 sid);
	void removeNamedCnx (const std::string &name, uint16 sid);

	// with a sid and a nid, find a good connection to send a message
	uint8 findConnectionId (uint16 sid, uint8 nid);

	void callServiceUpCallback (const std::string &serviceName, uint16 sid, bool callGlobalCallback = true);
	void callServiceDownCallback (const std::string &serviceName, uint16 sid, bool callGlobalCallback = true);
	
	friend void	uncbConnection(TSockId from, void *arg);
	friend void	uncbDisconnection(TSockId from, void *arg);
	friend void	uncbServiceIdentification(CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
	friend void	uncbMsgProcessing(CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
	friend void	uNetRegistrationBroadcast(const std::string &name, TServiceId sid, const std::vector<CInetAddress> &addr);
	friend void	uNetUnregistrationBroadcast(const std::string &name, TServiceId sid, const std::vector<CInetAddress> &addr);
	friend struct nel_isServiceLocalClass;
	friend struct nel_l5CallbackClass;
	friend struct nel_l5QueuesStatsClass;
};


} // NLNET


#endif // NL_UNIFIED_NETWORK_H

/* End of unified_network.h */
