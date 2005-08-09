/** \file module_gateway.h
 * module gateway interface
 *
 * $Id: module_gateway_transport.cpp,v 1.1 2005/08/09 19:06:45 boucher Exp $
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
#include "nel/net/module_gateway.h"
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_message.h"
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"

using namespace std;
using namespace NLMISC;



namespace NLNET
{
	/** the specialized route for server transport */
	class CL3ServerRoute : public CGatewayRoute
	{
	public:
		/// The id of the socket in the server
		TSockId			SockId;


		CL3ServerRoute(IGatewayTransport *transport)
			: CGatewayRoute(transport)
		{
		}

		void sendMessage(const CMessage &message) const;
	};

#define LAYER3_SERVER_CLASS_NAME "L3Server"

	/** Gateway transport using layer 3 server */
	class CGatewayL3ServerTransport : public IGatewayTransport
	{
		friend class CL3ServerRoute;
	public:
		/// Invalid command
		class EInvalidCommand : public NLMISC::Exception
		{
		};

		/// The callback server that receive connection and dispatch message
		auto_ptr<CCallbackServer>			_CallbackServer;

		/// A static mapper to retrieve transport from the CCallbackServer pointer
		typedef map<CCallbackNetBase*, CGatewayL3ServerTransport*>	TDispatcherIndex;
		static TDispatcherIndex				_DispatcherIndex;

		/// The table that keep track of all routes
		typedef std::map<TSockId, CL3ServerRoute*>	TRouteMap;
		TRouteMap	_Routes;

		
		/// Constructor
		CGatewayL3ServerTransport(const IGatewayTransport::TCtorParam &param) 
			: IGatewayTransport(param)
		{
		}

		~CGatewayL3ServerTransport()
		{
			if (_CallbackServer.get() != NULL)
			{
				// the transport is still open, close it before destruction
				closeServer();
			}
		}

		const std::string &getClassName() const
		{
			static string className(LAYER3_SERVER_CLASS_NAME);
			return className;
		}

		virtual void update()
		{
			// update the callback server
			if (_CallbackServer.get() != NULL)
				_CallbackServer->update();
		}

		virtual uint32 getRouteCount() const
		{
			return _Routes.size();
		}

		void dump(NLMISC::CLog &log) const
		{
			IModuleManager &mm = IModuleManager::getInstance();
			log.displayNL("  NeL Net layer 3 transport, SERVER mode");
			if (_CallbackServer.get() == NULL)
			{
				log.displayNL("  The server is currently closed.");
			}
			else
			{
				log.displayNL("  The server is open on '%s' and support %u routes :",
					_CallbackServer->listenAddress().asString().c_str(),
					_Routes.size());
				TRouteMap::const_iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					TSockId	sockId = first->first;
					CL3ServerRoute *route = first->second;
					log.displayNL("    + route to '%s', %u entries in the proxy translation table :",
						sockId->getTcpSock()->remoteAddr().asString().c_str(),
						route->ForeignToLocalIdx.size());

					{
						CGatewayRoute::TForeignToLocalIdx::const_iterator first(route->ForeignToLocalIdx.begin()), last(route->ForeignToLocalIdx.end());
						for (; first != last; ++first)
						{
							IModuleProxy *modProx = mm.getModuleProxy(first->second);

							log.displayNL("      + Proxy '%s' : local proxy id %u => foreign module id %u",
								modProx != NULL ? modProx->getModuleName().c_str() : "ERROR, invalid module",
								first->second,
								first->first);
						}
					}
				}
			}
		}

		void onCommand(const CMessage &command) throw (EInvalidCommand) 
		{
			// nothing done for now
			throw EInvalidCommand();
		}
		/// The gateway send a textual command to the transport
		bool onCommand(const TParsedCommandLine &command) throw (EInvalidCommand) 
		{
			if (command.SubParams.size() < 1)
				throw  EInvalidCommand();

			const std::string &commandName = command.SubParams[0].ParamName;
			if (commandName == "open")
			{
				const TParsedCommandLine *portParam = command.getParam("port");
				if (portParam == NULL)
					throw EInvalidCommand();

				uint16 port = atoi(portParam->ParamValue.c_str());

				openServer(port);
			}
			else if (commandName == "close")
			{
				closeServer();
			}
			else
				return false;

			return true;
		}

		/// Open the server by starting listing for incomming connection on the specified port
		void openServer(uint16 port) throw (ETransportError)
		{
			if (_CallbackServer.get() != NULL)
				throw ETransportError("openServer : The server is already open");

			// create a new callback server
			auto_ptr<CCallbackServer> cbs = auto_ptr<CCallbackServer> (new CCallbackServer());

			// register the callbacks
			cbs->setConnectionCallback(cbConnection, static_cast<IGatewayTransport*>(this));
			cbs->setDisconnectionCallback(cbDisconnection, static_cast<IGatewayTransport*>(this));
			cbs->setDefaultCallback(cbDispatchMessage);

			// open the server
			cbs->init(port);
			
			_CallbackServer = cbs;

			// register it in the dispatcher
			_DispatcherIndex.insert(make_pair(_CallbackServer.get(), this));
		}

		/// Close the server, this will close the listing socket and any active connection
		void closeServer()
		{
			if (_CallbackServer.get() == NULL)
				throw ETransportError("closeServer : The server is not open");

			// close all client connections
			while (!_Routes.empty())
			{
				CL3ServerRoute *route = _Routes.begin()->second;

				// close the connection
				_CallbackServer->disconnect(route->SockId);
				// callback the gateway
				_Gateway->onRouteRemoved(route);

				// delete route and cleanup
				_Routes.erase(_Routes.begin());
				delete route;
			}

			// Remove the dispatcher info
			_DispatcherIndex.erase(_CallbackServer.get());

			// release the callback server
			delete _CallbackServer.release();
		}


		/***************************************************/
		/** Event management                              **/
		/***************************************************/

		// handle the connection of a new client on the server
		void onConnection ( TSockId from)
		{
			nlassert(_Routes.find(from) == _Routes.end());

			// Create a new route for this connection
			CL3ServerRoute* route = new CL3ServerRoute(this);
			route->SockId = from;

			// store the route information
			_Routes.insert(make_pair(from, route));

			// callback the gateway
			_Gateway->onRouteAdded(route);
		}

		// handle the deconnection of a new client on the server
		void onDisconnection ( TSockId from)
		{
			TRouteMap::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			// callback the gateway that this route is no more
			_Gateway->onRouteRemoved(it->second);

			// delete the route
			CL3ServerRoute *route = it->second;
			_Routes.erase(it);
			delete route;
		}

		// Called to dispatch an incoming message to the gateway
		void onDispatchMessage(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
		{
			TRouteMap::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			_Gateway->onReceiveMessage(it->second, msgin);
		}


		/***************************************************/
		/** static callback forwarder                     **/
		/***************************************************/
		// Forwarder to the real method
		static void cbConnection ( TSockId from, void *arg )
		{
			nlassert(arg != NULL);
			CGatewayL3ServerTransport *transport = dynamic_cast<CGatewayL3ServerTransport *>(static_cast<IGatewayTransport*>(arg));
			nlassert(transport != NULL);

			transport->onConnection(from);
		}

		// Forwarder to the real method
		static void cbDisconnection ( TSockId from, void *arg )
		{
			nlassert(arg != NULL);
			CGatewayL3ServerTransport *transport = dynamic_cast<CGatewayL3ServerTransport *>(static_cast<IGatewayTransport*>(arg));
			nlassert(transport != NULL);

			transport->onDisconnection(from);
		}

		// Forward to the real method, do the dispatching to the correct CGatewayL3ServerTransport instance
		static void cbDispatchMessage (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
		{
			// retrieve the transport instance
			TDispatcherIndex::iterator it(_DispatcherIndex.find(&netbase));
			nlassert(it != _DispatcherIndex.end());

			// forward the call
			it->second->onDispatchMessage(msgin, from, netbase);
		}
	
	};
	
	CGatewayL3ServerTransport::TDispatcherIndex	CGatewayL3ServerTransport::_DispatcherIndex;

	// register this class in the transport factory
	NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayL3ServerTransport, std::string, string(LAYER3_SERVER_CLASS_NAME));

	void CL3ServerRoute::sendMessage(const CMessage &message) const
	{
		CGatewayL3ServerTransport *trpt = static_cast<CGatewayL3ServerTransport*>(_Transport);

		// send the message
		trpt->_CallbackServer->send(message, SockId);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/// Layer 3 client transport
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	class CL3ClientRoute : public CGatewayRoute
	{
	public:
		/// The Client callback
		mutable CCallbackClient		CallbackClient;

		CL3ClientRoute(IGatewayTransport *transport)
			: CGatewayRoute(transport)
		{
		}

		void sendMessage(const CMessage &message) const
		{
			CallbackClient.send(message);
		}
	};

#define LAYER3_CLIENT_CLASS_NAME "L3Client"

	/** Gateway transport using layer 3 client */
	class CGatewayL3ClientTransport : public IGatewayTransport
	{
		friend class CL3ClientRoute;
	public:
		/// Invalid command
		class EInvalidCommand : public NLMISC::Exception
		{
		};

		/// A static mapper to retrieve transport from the CCallbackServer pointer
		typedef map<CCallbackNetBase*, CGatewayL3ClientTransport*>	TDispatcherIndex;
		static TDispatcherIndex				_DispatcherIndex;

		/// Storage for active connection
		typedef map<TSockId, CL3ClientRoute*>	TClientRoutes;
		TClientRoutes			_Routes;

		/// Indexed storage of active connection (used for stable connId)
		/// a NULL TSockeId mean a free connection slot.
		typedef vector<TSockId>		TClientRouteIds;
		TClientRouteIds			_RouteIds;
		/// A list of free slot ready for use
		typedef vector<TClientRouteIds::difference_type>	TFreeRouteIds;
		TFreeRouteIds			_FreeRoutesIds;
		
		/// Constructor
		CGatewayL3ClientTransport(const IGatewayTransport::TCtorParam &param) 
			: IGatewayTransport(param)
		{
		}

		~CGatewayL3ClientTransport()
		{
			// close all open connection
			for (uint i=0; i<_RouteIds.size(); ++i)
			{
				if (_RouteIds[i] != NULL)
				{
					// close this open connection
					close(i);
				}
			}
		}

		const std::string &getClassName() const
		{
			static string className(LAYER3_CLIENT_CLASS_NAME);
			return className;
		}

		virtual void update()
		{
			// update the client connection
			TClientRoutes::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CL3ClientRoute *route = first->second;

				route->CallbackClient.update();
			}
		}

		virtual uint32 getRouteCount() const
		{
			return _Routes.size();
		}

		void dump(NLMISC::CLog &log) const
		{
			IModuleManager &mm = IModuleManager::getInstance();
			log.displayNL("  NeL Net layer 3 transport, CLIENT mode");

			log.displayNL("  There are actually %u active route :", _Routes.size());

			TClientRoutes::const_iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				TSockId	sockId = first->first;
				CL3ClientRoute *route = first->second;
				log.displayNL("    + route to '%s', %u entries in the proxy translation table :",
					sockId->getTcpSock()->remoteAddr().asString().c_str(),
					route->ForeignToLocalIdx.size());

				{
					CGatewayRoute::TForeignToLocalIdx::const_iterator first(route->ForeignToLocalIdx.begin()), last(route->ForeignToLocalIdx.end());
					for (; first != last; ++first)
					{
						IModuleProxy *modProx = mm.getModuleProxy(first->second);

						log.displayNL("      + Proxy '%s' : local proxy id %u => foreign module id %u",
							modProx != NULL ? modProx->getModuleName().c_str() : "ERROR, invalid module",
							first->second,
							first->first);
					}
				}
			}
		}

		void onCommand(const CMessage &command) throw (EInvalidCommand) 
		{
			// nothing done for now
			throw EInvalidCommand();
		}
		/// The gateway send a textual command to the transport
		bool onCommand(const TParsedCommandLine &command) throw (EInvalidCommand) 
		{
			if (command.SubParams.size() < 1)
				throw  EInvalidCommand();

			const std::string &commandName = command.SubParams[0].ParamName;
			if (commandName == "connect")
			{
				const TParsedCommandLine *addrParam = command.getParam("addr");
				if (addrParam == NULL)
					throw EInvalidCommand();

				CInetAddress addr(addrParam->ParamValue);

				connect(addr);
			}
			else if (commandName == "close")
			{
				const TParsedCommandLine *conIdParam= command.getParam("connId");
				if (conIdParam == NULL)
					throw EInvalidCommand();

				uint32	connId = atoi(conIdParam->ParamValue.c_str());

				close(connId);
			}
			else
				return false;

			return true;
			
		}

		/// connect to a server
		void connect(CInetAddress &addr)
		{
			auto_ptr<CL3ClientRoute> route = auto_ptr<CL3ClientRoute>(new CL3ClientRoute(this));

			// set the callbacks
			route->CallbackClient.setDisconnectionCallback(cbDisconnection, static_cast<IGatewayTransport*>(this));
			route->CallbackClient.setDefaultCallback(cbDispatchMessage);

			// connect to the server
			route->CallbackClient.connect(addr);

			// store the route
			_Routes.insert(make_pair(route->CallbackClient.getSockId(), route.get()));
			if (_FreeRoutesIds.empty())
				_RouteIds.push_back(route->CallbackClient.getSockId());
			else
			{
				nlassert(_FreeRoutesIds.back() == NULL);
				_RouteIds[_FreeRoutesIds.back()] = route->CallbackClient.getSockId();
				_FreeRoutesIds.pop_back();
			}

			// register it in the dispatcher
			_DispatcherIndex.insert(make_pair(&route->CallbackClient, this));

			// release the auto ptr
			CL3ClientRoute *rt = route.release();

			// callback the gateway about the new route
			_Gateway->onRouteAdded(rt);
		}


		// handle the connection of a new client on the server
		void close ( uint32 connId)
		{
			// some basic checks on connId
			if (connId >= _RouteIds.size())
			{
				nlwarning("Invalid connectionId %u, max is %u", connId, _RouteIds.size()-1);
				return;
			}

			if (_RouteIds[connId] == NULL)
			{
				nlwarning("Invalid connectionId %u, the connection is unused now.", connId);
				return;		
			}

			// retrieve the connection to close
			TClientRoutes::iterator it(_Routes.find(_RouteIds[connId]));
			nlassert(it != _Routes.end());

			CL3ClientRoute *route = it->second;

			// callback gateway
			_Gateway->onRouteRemoved(route);

			// close the connection
			route->CallbackClient.disconnect();

			// cleanup memory, index ...
			_DispatcherIndex.erase(&(route->CallbackClient));
			_Routes.erase(it);
			delete route;
			_RouteIds[connId] = NULL;
			_FreeRoutesIds.push_back(connId);
		}

		/***************************************************/
		/** Event management                              **/
		/***************************************************/

		// handle the deconnection of a the client from the server
		void onDisconnection ( TSockId from)
		{
			TClientRoutes::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			// callback the gateway that this route is no more
			_Gateway->onRouteRemoved(it->second);

			// delete the route
			CL3ClientRoute *route = it->second;
			_Routes.erase(it);
			delete route;
		}

		// Called to dispatch an incoming message to the gateway
		void onDispatchMessage(CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
		{
			TClientRoutes::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			_Gateway->onReceiveMessage(it->second, msgin);
		}


		/***************************************************/
		/** static callback forwarder                     **/
		/***************************************************/

		// Forwarder to the real method
		static void cbDisconnection ( TSockId from, void *arg )
		{
			nlassert(arg != NULL);
			CGatewayL3ServerTransport *transport = dynamic_cast<CGatewayL3ServerTransport *>(static_cast<IGatewayTransport*>(arg));
			nlassert(transport != NULL);

			transport->onDisconnection(from);
		}

		// Forward to the real method, do the dispatching to the correct CGatewayL3ServerTransport instance
		static void cbDispatchMessage (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
		{
			// retrieve the transport instance
			TDispatcherIndex::iterator it(_DispatcherIndex.find(&netbase));
			nlassert(it != _DispatcherIndex.end());

			// forward the call
			it->second->onDispatchMessage(msgin, from, netbase);
		}
	
	};
	
	CGatewayL3ClientTransport::TDispatcherIndex	CGatewayL3ClientTransport::_DispatcherIndex;

	// register this class in the transport factory
	NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayL3ClientTransport, std::string, string(LAYER3_CLIENT_CLASS_NAME));


	void forceGatewayTransportLink()
	{
	}

} // namespace NLNET