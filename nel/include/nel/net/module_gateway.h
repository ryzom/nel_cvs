/** \file module_gateway.h
 * module gateway interface
 *
 * $Id: module_gateway.h,v 1.3 2005/08/09 19:06:25 boucher Exp $
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


#ifndef NL_FILE_MODULE_GATEWAY_H
#define NL_FILE_MODULE_GATEWAY_H

#include "nel/misc/twin_map.h"
#include "module_common.h"
#include "inet_address.h"
#include "message.h"
#include "module_message.h"

namespace NLNET
{
	class IGatewayTransport;
	class CGatewayRoute;

	/** Interface for gateway.
	 *	A gateway is the part of the module layer that interconnect
	 *	module locally and give access to foreign module hosted in
	 *	other process.
	 *	Gateway can interconnect local module with themselves, as well as
	 *	connect with another gateway in another process or host.
	 *
	 *	This interface is designed with a base implementation that
	 *	allow one to redefine some part of the gateway behavior
	 *	without rewriting a complete gateway from scratch.
	 *
	 *	Gateway can be client and/or server. Client forward 
	 *	any message for foreign module to the server, server
	 *	route message either locally or to client gateway that host 
	 *	the module addressee.
	 *
	 *	A client gateway can be connected to more than one client,
	 *	establishing a bridge between the two server.
	 *
	 *	In the case of an hybrid configured gateway (i.e. a gateway
	 *	acting as client AND server), the message are routed from the
	 *	server endpoint to the client part if needed.
	 *
	 *	Any message routing or module discovering can be intercepted 
	 *	by overloading a virtual method to change the standard behavior
	 *	of the gateway.
	 */
	class IModuleGateway : public NLMISC::CRefCount
	{
	public:
		// Exception classes ===================================

		/// The gateway is already open while trying to open it.
		class EGatewayAlreadyOpen : public NLMISC::Exception
		{
		};

		/// When trying to open the gateway server, the TCP port is already in use
		class EGatewayPortInUse : public NLMISC::Exception
		{
		};
		
		/// The gateway is not open while trying to close it.
		class EGatewayNotOpen : public NLMISC::Exception
		{
		};

		/// A gateway is not connected while trying to communicate with
		class EGatewayNotConnected : public NLMISC::Exception
		{
		};

		enum TModuleGatewayConstant
		{
			/// A gateway client connection is accepted
			mgc_accept_connection,
			/// A gateway client connection is rejected
			mgc_reject_connection,
			/// A client to server connection is a success
			mgc_connection_success,
			/// A client to server connection has failed
			mgc_connection_failed,
		};

		virtual ~IModuleGateway() {}

		//@{
		//@name Gateway general information
		/// Return the local name of the gateway
		virtual const std::string &getGatewayName() const =0;
		/// Return the Fully Qualified Gateway Name (FQGN)
		virtual const std::string &getFullyQualifiedGatewayName() const =0;
		/// Return the gateway proxy of this gateway
//		virtual TModuleGatewayProxyPtr &getGatewayProxy() =0;
		//@}

		//@{
		//@name Gateway transport management
		/// Return a pointer on the named transport interface, or NULL if the transport is unknown.
		virtual IGatewayTransport *getGatewayTransport(const std::string &transportName) const =0;

		/// Return the number of transport currently active on this gateway
		virtual uint32	getTransportCount() const =0;

		/// Return the number route available
		virtual uint32	getRouteCount() const =0;

		/// Return the number of ping received. This is incremented by special "GW_PING" message for unit testing
		virtual uint32 getReceivedPingCount() const =0;

		//@}

		//@{
		//@name Gateway transport callback
		/// A new route a added by a transport
		virtual void onRouteAdded(CGatewayRoute *route) =0;

		/// A route is removed by a transport
		virtual void onRouteRemoved(CGatewayRoute *route) =0;

		/// A transport have received a message
		virtual void onReceiveMessage(CGatewayRoute *from, CMessage &msgin) =0;
		//@}

		//@{
		//@name Module management
		/** Callback called when the gateway has received some new module
		 *	and eventually, need to disclose the module information to 
		 *	the connected gateway.
		 *	The default behavior is to disclose the module to all
		 *	connected gateway.
		 */
		virtual void onAddModuleProxy(IModuleProxy *addedModule) =0;
		/** Callback called when a module become unavailable, either
		 *	because it is unplugged from it's socket, or, the
		 *	gateway that disclosed it has been disconnected.
		 */
		virtual void onRemoveModuleProxy(IModuleProxy *removedModule) =0;

		/** Disclose module information to a connected gateway.
		 *	This can also be this gateway itself.
		 */
		virtual void discloseModule(IModuleProxy *moduleProxy) 
			throw (EGatewayNotConnected)
			=0;

		/** Retrieve the proxy for a locally plugged module.
		 *	Each local module plugged in a gateway has an associated
		 *	proxy. This method return this proxy or NULL if the 
		 *	module is not plugged here.
		 */
		virtual IModuleProxy *getPluggedModuleProxy(IModule *pluggedModule) =0;

		/// Return the number of proxies managed by this gateway
		virtual uint32	getProxyCount() const =0;

		/// Fill a vector with the list of proxies managed here. The module are filled in ascending proxy id order.
		virtual void	getModuleProxyList(std::vector<IModuleProxy*> &resultList) const =0;
		//@}

		//@{
		//@name Module messaging
		/** Callback called when a message arrive from a gateway and need
		 *	to be dispatched.
		 *	The default behavior is to route the message in any case to
		 *	the destination module.
		 *	You can override this callback to add some message filtering
		 *	or hacking feature.
		 */
//		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, TModuleMessagePtr &message) =0;

		/** Send a message to a module.
		 */
		virtual void sendModuleMessage(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, const NLNET::CMessage &message) =0;

		/** Send a message to the module plugged in this gateway.
		 *	You can override this method to change the dispatching, add filtering,
		 *	message hacking or interceptor.
		 */
		virtual void dispatchMessageModule(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, CMessage &message) =0;
		//@}
	};

	/** Interface class for gateway transport.
	 *	A gateway transport is an object associated to a standard gateway
	 *	at run time and that provide a mean to interconnect with
	 *	other gateway.
	 *	As each transport mode as it's own command requirement,
	 *	a generic command system is provided sending command message
	 *	to the transport implementation.
	 *	
	 *	Example of transport are layer 3 client and layer 3 server.
	 */
	class IGatewayTransport
	{
	protected:
		/// Back pointer to the gateway hosting this transport
		IModuleGateway			*_Gateway;
	public:
		/// Invalid command
		class EInvalidCommand : public NLMISC::Exception
		{
		public:
			EInvalidCommand(const char *err) : Exception(err) {}
		};

		/// Error in the transport
		class ETransportError : public NLMISC::Exception
		{
		public:
			ETransportError(const char *err) : Exception(err) {}
		};
		
		struct TCtorParam
		{
			IModuleGateway *Gateway;
		};
		/// Constructor, establish link with the associated gateway
		IGatewayTransport(const TCtorParam &param)
		{
			_Gateway = param.Gateway;
		}

		virtual ~IGatewayTransport() {}

		/// Return the class name from the transport factory
		virtual const std::string &getClassName() const =0;

		/// The gateway send a command message to the transport
		virtual void onCommand(const CMessage &command) throw (EInvalidCommand) = 0;
		/// The gateway send a textual command to the transport
		virtual bool onCommand(const TParsedCommandLine &command) throw (EInvalidCommand) = 0;

		/// The gateway update the transport regularly
		virtual void update() =0;

		/// Return the number of route currently open by the transport
		virtual uint32 getRouteCount() const =0;

		/// Dump debug information in the specified log stream
		virtual void dump(NLMISC::CLog &log) const =0;
	};

	/** Base class for gateway route.
	 *	Route are provided by transport.
	 *	Transport provide a mean to build route
	 *	between gateway.
	 *	Route show the list of foreign gateway that are 
	 *	reachable with it and are use to send
	 *	message to these gateways.
	 *
	 *	The route store proxy id translation table, i.e,
	 *	for each module proxy that come from this route
	 *	we store association of the local proxy ID with
	 *	the foreign proxy ID, that is the proxy that
	 *	represent the module at the outbound of the route.
	 *
	 *	Note that even if the route object is created
	 *	by the transport, the translation table is
	 *	feed and managed by the gateway implementation.
	 */
	class CGatewayRoute
	{
	public:
		/// The local foreign => local proxy id translation table
		typedef std::map<TModuleId, TModuleId>	TForeignToLocalIdx;
		TForeignToLocalIdx	ForeignToLocalIdx;

		/// The transport that manage this route
		IGatewayTransport	*_Transport;


		//@{
		/// @name Informations on the next module message to dispatch

		/// next message type, set to CModuleMessageHeaderCodec::mt_invalid when no module message are awaited
		CModuleMessageHeaderCodec::TMessageType	NextMessageType;
		/// Id of the sender proxy
		TModuleId		NextSenderProxyId;
		/// Id of the addressee proxy
		TModuleId		NextAddresseeProxyId;
		//@}

		/// constructor, must provide the transport
		CGatewayRoute(IGatewayTransport *transport)
			: _Transport(transport),
			NextMessageType(CModuleMessageHeaderCodec::mt_invalid)
		{
		}
		/// Return the transport that hold this route
		IGatewayTransport *getTransport() { return _Transport; };
		/// Send a message via the route
		virtual void sendMessage(const CMessage &message) const =0;
	};

//	const TModuleGatewayPtr	NullModuleGateway;

	/** This interface represent a foreign gateway
	 *	It is used to represent foreign gateway
	 *	known by a gateway, it is also stored
	 *	with the module proxy to remember the 
	 *	source of the module.
	 */
//	class IModuleGatewayProxy : public NLMISC::CRefCount
//	{
//	public:
//		/// The gateway is not collocated
//		class EGatewayNotCollocated : public NLMISC::Exception
//		{
//		};
//
//		virtual ~IModuleGatewayProxy() {}
//
//		/// Return the host name of the machine hosting this gateway.
//		virtual const std::string &getGatewayHost() =0;
//		/** If the gateway is collocated (i.e, lie in the current process), 
//		 *	then return the gateway pointer to make direct call on
//		 *	the gateway interface.
//		 */
//		virtual IModuleGateway *getLocalGateway() 
//			throw (EGatewayNotCollocated) 
//			=0;
//
//		/// Return true if the gateway lie in the current process
//		virtual bool isCollocated() =0;
//		/// Return true if the 
//		virtual bool isConnected() =0;
//
//	};
//
//	const TModuleGatewayProxyPtr	NullModuleGatewayProxy;

} // namespace NLNET


#endif // NL_FILE_MODULE_GATEWAY_H

