/** \file module_gateway.h
 * module gateway interface
 *
 * $Id: module_gateway.h,v 1.2 2005/06/23 17:39:57 boucher Exp $
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

#include "module_common.h"
#include "inet_address.h"

namespace NLNET
{
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
		/// The gateway is not open while trying to close it.
		class EGatewayNotOpen : public NLMISC::Exception
		{
		};

		/// A gateway is not connected while trying to commnicate with
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
		virtual TModuleGatewayProxyPtr &getGatewayProxy() =0;
		//@}

		//@{
		//@name Gateway server management
		/** Ask the gateway implementation to see if it is listening
		 *	for client gateway connection.
		 */
		virtual bool isGatewayServerOpen() =0;
		/** Return the gateway listening address (ie. ip+port).
		 *	If it has not been set, the method should return 0.0.0.0 as ip
		 *	and 0 as port.
		 */
		virtual CInetAddress  getGatewayServerAddress() =0;
		/** Fill the vector with the list of gateway currently connected on
		 *	this gateway server.
		 *	Note that the vector is not cleared before filling.
		 */
		virtual void getGatewayClientList(std::vector<TModuleGatewayProxyPtr> gatewayList) =0;
		/** Open the gateway server to listen and accept client gateway connection.
		 */
		virtual void openGatewayServer(CInetAddress listeningAddress)
			throw (EGatewayAlreadyOpen)
			=0;
		/** Close the gateway server. No more client connection will be accepted.
		 *	The gateway client that are currently connected are not
		 *	removed. You call shutdownGatewayServer to close 
		 *	all established connection.
		 */
		virtual void closeGatewayServer() 
			throw (EGatewayNotOpen)
			=0;
		/** Close all established client gateway connection and close the server
		 *	if it is open.
		 *	Unlike closeGatewayServer, this method don't throw EGatewayNotOpen.
		 */
		virtual void shutdownGatewayServer() =0;

		/** Callback called when the server is openned */
		virtual void onGatewayServerOpen() =0;
		/** Callback called when the server is closed */
		virtual void onGatewayServerClose() =0;

		/** Callback called when a client gateway connect on the server.
		 *	Default behavior is to return 'mgc_accept_connection'.
		 *	You can overide this method and return 'mgc_reject_connection' if 
		 *	you don't accept the gateway connection.
		 */
		virtual TModuleGatewayConstant onClientGatewayConnect(TModuleGatewayProxyPtr &clientGateway) =0;
		/** Callback called when a client gateway connection close.
		 *	When called, it is too late to send any message to the
		 *	disconnected gateway.
		 */
		virtual void onClientGatewayDisconnect(TModuleGatewayProxyPtr &clientGateway) =0;
		//@}

		//@{
		//@name Gateway client management

		/** Fill the vector with the list of server at witch this
		 *	gateway is connected.
		 *	Note that the vector is not cleared before being filled.
		 */
		virtual void getGatewayServerList(std::vector<TModuleGatewayProxyPtr> serverList) =0;
		/// Return true if the gateway is connected to at least one server.
		virtual bool isGatewayConnected() =0;
		/** Connect the gateway to a gateway server.
		 *	The connection is made asynchronously.
		 *	When the connection result is available (either
		 *	a successful or a failed connection), the 
		 *	method onGatewayConnection callback is called.
		 */
		virtual void connectGateway(CInetAddress serverAdress) =0;
		/** Close the connection with a gateway server.
		 *	The closure is done immediately.
		 */
		virtual void disconnectGateway(TModuleGatewayProxyPtr &serverGateway) =0;

		/** Callback called when the result of a connection attempt is known.
		 *	The result can be either 'mgc_connection_success' or 
		 *	'mgc_connection_failed'. In the later case, the serverGateway object 
		 *	is null.
		 */
		virtual void onGatewayConnection(const TModuleGatewayProxyPtr &serverGateway, TModuleGatewayConstant connectionResult) =0;

		/** Callback called when a server connection has been closed.
		 */
		virtual void onGatewayDisconnection(const TModuleGatewayProxyPtr &serverGateway) =0;
		//@}

		//@{
		//@name Module discovering
		/** Callback called when the gateway has received some new module
		 *	and eventually, need to disclose the module information to 
		 *	the connected gateway.
		 *	The default behavior is to disclose the module to all
		 *	connected gateway.
		 */
		virtual void onAddModuleProxy(TModuleProxyPtr &addedModule) =0;
		/** Callback called when a module become unavailable, either
		 *	because it is unplugged from it's socket, or, the
		 *	gateway that disclosed has been disconnected.
		 */
		virtual void onRemoveModuleProxy(TModuleProxyPtr &removedModule) =0;

		/** Disclose module information to a connected gateway.
		 *	This can also be this gateway itself.
		 */
		virtual void discloseModule(IModuleProxy *module, IModuleGatewayProxy *gateway) 
			throw (EGatewayNotConnected)
			=0;
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
		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, const TModuleMessagePtr &message) =0;

		/** Send a message to another gateway.
		 */
		virtual void sendModuleMessage(TModuleGatewayProxyPtr &destGateway, const TModuleMessagePtr &message) =0;

		/** Send a message to the module plugged in this gateway.
		 *	You can override this method to change the dispatching, add filtering,
		 *	message hacking or interceptor.
		 */
		virtual void dispatchMessageModule(TModuleGatewayProxyPtr &senderGateway, const TModuleMessagePtr &message) =0;
		//@}
	};

//	const TModuleGatewayPtr	NullModuleGateway;

	/** This interface represent a foreign gateway
	 *	It is used to represent foreign gateway
	 *	known by a gateway, it is also stored
	 *	with the module proxy to remember the 
	 *	source of the module.
	 */
	class IModuleGatewayProxy : public NLMISC::CRefCount
	{
	public:
		virtual ~IModuleGatewayProxy() {}
		virtual void getGatewayHost() =0;
		virtual void getLocalGateway() =0;
		virtual bool isCollocated() =0;
		virtual bool isConnected() =0;

	};

	const TModuleGatewayProxyPtr	NullModuleGatewayProxy;

} // namespace NLNET


#endif // NL_FILE_MODULE_GATEWAY_H

