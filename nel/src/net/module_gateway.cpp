/** \file module_gateway.h
 * module gateway interface
 *
 * $Id: module_gateway.cpp,v 1.3 2005/08/09 19:06:45 boucher Exp $
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
	/// Sub message for module description
	struct TModuleDescMsg
	{
		TModuleId	ModuleProxyId;
		uint32		ModuleDistance;
		string		ModuleFullName;
		string		ModuleClass;

		void serial(NLMISC::IStream &s)
		{
			s.serial(ModuleProxyId);
			s.serial(ModuleDistance);
			s.serial(ModuleFullName);
			s.serial(ModuleClass);

		}
	};
	/// message for adding module
	struct TModuleAddMsg
	{
		vector<TModuleDescMsg>	Modules;

		void serial(NLMISC::IStream &s)
		{
			s.serialCont(Modules);
		}
	};
	/// message for module distance update
	struct TModuleDistanceChangeMsg
	{
		TModuleId	ModuleId;
		uint32		NewDistance;

		void serial(NLMISC::IStream &s)
		{
			s.serial(ModuleId);
			s.serial(NewDistance);
		}
	};
	/// Message for module removing
	struct TModuleRemMsg
	{
		vector<TModuleId>	RemovedModules;

		void serial(NLMISC::IStream &s)
		{
			s.serialCont(RemovedModules);
		}
	};
	/// Message for module operation
	struct TModuleOperationMsg
	{
		TModuleId	ModuleId;
		string		OperationName;

		CMessage	MessageBody;

		void serial(NLMISC::IStream &s)
		{
			s.serial(ModuleId);
			s.serial(OperationName);
			s.serial(MessageBody);
		}
	};

	/** The standard gateway that interconnect module 
	 *	across process.
	 */
	class CStandardGateway : 
		public CModuleBase,
		public IModuleGateway,
		public CModuleSocket
	{
		typedef uint64	TGatewayId;
		
		// the proxy that represent this gateway
//		TModuleGatewayProxyPtr		_ThisProxy;

//		typedef CTwinMap<TModuleProxyPtr, TStringId>	TModuleProxiesIdx;
		// The modules proxies indexed by name and by proxy address
//		TModuleProxiesIdx		_ModuleProxies;

		typedef map<TModuleId, TModuleProxyPtr>			TModuleProxies;
		/// Module proxies managed by this gateway. The map key is the module proxy id
		TModuleProxies		_ModuleProxies;

		typedef map<TModuleId, TModuleId>				TLocalModuleIndex;
		/// Translation table to find module proxies for locally plugged module
		/// The map key is the local module id, the data is the associated proxy id
		TLocalModuleIndex		_LocalModuleIndex;


		typedef map<std::string, IGatewayTransport*>	TTransportList;
		/// the list of active transport
		TTransportList	_Transports;

		typedef set<CGatewayRoute*>		TRouteList;
		// the list of available routes
		TRouteList		_Routes;

		/// Ping counter for debug purpose
		uint32			_PingCounter;

	public:

		CStandardGateway()
			: _PingCounter(0)
		{
		}

		~CStandardGateway()
		{
			// we need to unplug any plugged module
			while (!_PluggedModules.getAToBMap().empty())
			{
				_PluggedModules.getAToBMap().begin()->second->unplugModule(this);
			}

			// delete all transport
			while (!_Transports.empty())
			{
				IGatewayTransport *transport = _Transports.begin()->second;

				delete transport;
				_Transports.erase(_Transports.begin());
			}

			// must be done before the other destructors are called
			unregisterSocket();
		}

		/***********************************************************
		 ** Gateway methods 
		 ***********************************************************/
		virtual const std::string &getGatewayName() const
		{
			return getModuleName();
		}
		virtual const std::string &getFullyQualifiedGatewayName() const
		{
			return getModuleFullyQualifiedName();
		}
		/// Return the gateway proxy of this gateway
//		virtual TModuleGatewayProxyPtr &getGatewayProxy()
//		{
//			nlassert(!_ThisProxy.isNull());
//			return _ThisProxy;
//		}

		virtual IGatewayTransport *getGatewayTransport(const std::string &transportName) const
		{
			TTransportList::const_iterator it(_Transports.find(transportName));

			if (it == _Transports.end())
				return NULL;
			else
				return it->second;
		}

		virtual uint32	getTransportCount() const
		{
			return _Transports.size();
		}

		virtual uint32	getRouteCount() const
		{
			return _Routes.size();
		}

		virtual uint32 getReceivedPingCount() const 
		{
			return _PingCounter;
		}
		
		virtual void onRouteAdded(CGatewayRoute *route)
		{
			// Remember the new route
			nlassert(_Routes.find(route) == _Routes.end());
			_Routes.insert(route);

			// a new route is available, send it the complete module list
			TModuleAddMsg message;
			message.Modules.reserve(_ModuleProxies.size());

			// first, fill the module proxy list
			{
				TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
				for (; first != last; ++first)
				{
					IModuleProxy *modProx = first->second;

					// only transmit module desc coming from other routes
					if (modProx->getGatewayRoute() != route)
					{
						message.Modules.resize(message.Modules.size()+1);
						TModuleDescMsg &modDesc = message.Modules.back();

						modDesc.ModuleProxyId = modProx->getModuleProxyId();
						modDesc.ModuleClass = modProx->getModuleClassName();
						modDesc.ModuleFullName = modProx->getModuleName();
						modDesc.ModuleDistance = modProx->getModuleDistance()+1;
					}
				}
			}

			// now, send the message
			CMessage buffer("MOD_ADD");
			buffer.serial(message);
			route->sendMessage(buffer);
		}

		/// A route is removed by a transport
		virtual void onRouteRemoved(CGatewayRoute *route)
		{
			nlassert(_Routes.find(route) != _Routes.end());
			// we need to remove all the proxy that come from this route
			CGatewayRoute::TForeignToLocalIdx::iterator first(route->ForeignToLocalIdx.begin()), last(route->ForeignToLocalIdx.end());
			for (; first != last; ++first)
			{
				TModuleId localProxyId = first->second;
				TModuleProxies::iterator it(_ModuleProxies.find(localProxyId));
				nlassert(it != _ModuleProxies.end());

				IModuleProxy *modProx = it->second;
					
				// trigger an event in the gateway
				onRemoveModuleProxy(modProx);

				// remove proxy record from the proxy list
				_ModuleProxies.erase(it);

				// Release the proxy object
				IModuleManager::getInstance().releaseModuleProxy(modProx->getModuleProxyId());
			}
			// cleanup the translation table
			route->ForeignToLocalIdx.clear();

			// clear the route tracker
			_Routes.erase(route);
		}

		/// A transport have received a message
		virtual void onReceiveMessage(CGatewayRoute *from, CMessage &msgin)
		{
			// dispatch the message
			if (from->NextMessageType != CModuleMessageHeaderCodec::mt_invalid)
			{
				// this message must be dispatched to a module
				onReceiveModuleMessage(from, msgin);
			}
			// Not a module message, dispatch the gateway message
			else if (msgin.getName() == "MOD_OP")
			{
				onReceiveModuleMessageHeader(from, msgin);
			}
			else if (msgin.getName() == "MOD_ADD")
			{
				onReceiveModuleAdd(from, msgin);
			}
			else if (msgin.getName() == "MOD_REM")
			{
				onReceiveModuleRemove(from, msgin);
			}
			else if (msgin.getName() == "MOD_DST_UPD")
			{
				onReceiveModuleDistanceUpdate(from, msgin);
			}
		}

		/***********************************/
		/* Inter gateway message reception */
		/***********************************/

		/** A gateway receive module operation */
		void onReceiveModuleMessage(CGatewayRoute *from, CMessage &msgin)
		{

			// clean the message type now, any return path will be safe
			CModuleMessageHeaderCodec::TMessageType msgType = from->NextMessageType;
			from->NextMessageType = CModuleMessageHeaderCodec::mt_invalid;

			// Retrieve sender and destination proxy and recall gateway send method
			IModuleProxy *senderProxy;
			IModuleProxy *addresseeProxy;

			TModuleProxies::iterator it;

			// sender proxy
			it = _ModuleProxies.find(from->NextSenderProxyId);
			if (it == _ModuleProxies.end())
			{
				nlwarning("Can't dispatch the module message, sender proxy %u is not in this gateway", from->NextSenderProxyId);
				return;
			}
			senderProxy = it->second;
			// addressee proxy
			it = _ModuleProxies.find(from->NextAddresseeProxyId);
			if (it == _ModuleProxies.end())
			{
				nlwarning("Can't dispatch the module message, sender proxy %u is not in this gateway", from->NextAddresseeProxyId);
				return;
			}
			addresseeProxy = it->second;

			// give the message to the gateway (either for local dispatch or for forwarding)
			sendModuleMessage(senderProxy, addresseeProxy, msgin);
		}

		// A gateway receive a module message header
		void onReceiveModuleMessageHeader(CGatewayRoute *from, CMessage &msgin)
		{
			if (from->NextMessageType != CModuleMessageHeaderCodec::mt_invalid)
			{
				// juste warn (but that is VERY BAD)
				nlwarning("Receiving a new module message header without having received the previous module message !");
			}

			// store the message information in the route
			CModuleMessageHeaderCodec::decode(
				msgin, 
				from->NextMessageType, 
				from->NextSenderProxyId,
				from->NextAddresseeProxyId);

			// translate sender id
			CGatewayRoute::TForeignToLocalIdx::iterator it;
			// translate sender id
			it = from->ForeignToLocalIdx.find(from->NextSenderProxyId);
			if (it == from->ForeignToLocalIdx.end())
			{
				nlwarning("The sender proxy %u is unknown in the translation table, can't dispatch the message !", from->NextSenderProxyId);
				from->NextMessageType = CModuleMessageHeaderCodec::mt_invalid;
				return;
			}
			from->NextSenderProxyId = it->second;
			// now, wait the message body
		}

		/** A gateway send new modules information */
		void onReceiveModuleAdd(CGatewayRoute *from, CMessage &msgin)
		{
			TModuleAddMsg message;
			msgin.serial(message);

			// for each received module info
			for (uint i=0; i<message.Modules.size(); ++i)
			{
				TModuleDescMsg &modDesc = message.Modules[i];

				// create a module proxy
				IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
					this,
					from,
					modDesc.ModuleDistance,
					modDesc.ModuleClass,
					modDesc.ModuleFullName,
					modDesc.ModuleProxyId);

				// store the proxy in the proxy list
				_ModuleProxies.insert(make_pair(modProx->getModuleProxyId(), modProx));

				// Fill the proxy id translation table for this route
				from->ForeignToLocalIdx.insert(make_pair(modDesc.ModuleProxyId, modProx->getModuleProxyId()));

				// trigger an event in the gateway
				onAddModuleProxy(modProx);
			}
		}

		void onReceiveModuleRemove(CGatewayRoute *from, CMessage &msgin)
		{
			TModuleRemMsg message;
			msgin.serial(message);

			// for each removed module
			for (uint i=0; i<message.RemovedModules.size(); ++i)
			{
				// translate the module id
				CGatewayRoute::TForeignToLocalIdx::iterator it(from->ForeignToLocalIdx.find(message.RemovedModules[i]));
				if (it == from->ForeignToLocalIdx.end())
				{
					// oups !
					nlwarning("onReceiveModuleRemove : unknown foreign module id %u", message.RemovedModules[i]);
					continue;
				}

				TModuleId proxyId = it->second;
				
				// retrieve the module proxy
				TModuleProxies::iterator it2(_ModuleProxies.find(proxyId));
				if (it2 == _ModuleProxies.end())
				{
					// oups !
					nlwarning("onReceiveModuleRemove : can't find proxy for id %u coming from foreign id %u", proxyId, message.RemovedModules[i]);
					continue;
				}
				IModuleProxy *modProx = it2->second;
				
				// trigger an event in the gateway
				onRemoveModuleProxy(modProx);

				// clean the translation table
				from->ForeignToLocalIdx.erase(it);

				// remove from the proxy list
				_ModuleProxies.erase(it2);
				// release the proxy
				IModuleManager::getInstance().releaseModuleProxy(proxyId);
			}
		}

		void onReceiveModuleDistanceUpdate(CGatewayRoute *from, CMessage &msgin)
		{
			nlstop;
		}


		
		virtual void onAddModuleProxy(IModuleProxy *addedModule)
		{
			// disclose module to local modules 
			discloseModule(addedModule);

//			CModuleProxy *modProx = dynamic_cast<CModuleProxy *>(addedModule);
//			nlassert(modProx != NULL);
			// and send module info to any route

			// for each route
			TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CGatewayRoute *route = *first;
				// only send info to other routes
				if (route != addedModule->getGatewayRoute())
				{
					// TODO : optimize by buffering and sending only one message for multiple module descriptor
					TModuleAddMsg message;
					message.Modules.resize(1);
					TModuleDescMsg &modDesc = message.Modules[0];

					modDesc.ModuleProxyId = addedModule->getModuleProxyId();
					modDesc.ModuleClass = addedModule->getModuleClassName();
					modDesc.ModuleFullName = addedModule->getModuleName();
					modDesc.ModuleDistance = addedModule->getModuleDistance()+1;

					CMessage buffer("MOD_ADD");
					buffer.serial(message);

					route->sendMessage(buffer);
				}
			}
		}

		virtual void onRemoveModuleProxy(IModuleProxy *removedModule)
		{
			// for each route
			{
				TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					CGatewayRoute *route = *first;
					// only send info to other routes
					if (route != removedModule->getGatewayRoute())
					{
						// TODO : optimize by buffering and sending only one message for multiple module descriptor
						TModuleRemMsg message;
						message.RemovedModules.push_back(removedModule->getModuleProxyId());

						CMessage buffer("MOD_REM");
						buffer.serial(message);

						route->sendMessage(buffer);
					}
				}
			}

			// warn any plugged module
			{
				TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;
					if (removedModule->getGatewayRoute() != NULL
						|| module->getModuleId() != removedModule->getForeignModuleId())
					{
						module->onModuleDown(removedModule);
					}
				}
			}
		}
		
		virtual void discloseModule(IModuleProxy *moduleProxy)
			throw (EGatewayNotConnected)
		{
//			CModuleProxy *modProx = dynamic_cast<CModuleProxy *>(moduleProxy);
//			nlassert(modProx != NULL);
			nlassert(moduleProxy->getModuleGateway() == this);

			// warn any plugged module
			TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
			for (; first != last; ++first)
			{
				IModule *module = first->second;
				if (moduleProxy->getGatewayRoute() != NULL
					|| module->getModuleId() != moduleProxy->getForeignModuleId())
				{
					module->onModuleUp(moduleProxy);
				}
			}
		}

		virtual IModuleProxy *getPluggedModuleProxy(IModule *pluggedModule)
		{
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(pluggedModule->getModuleId()));

			if (it == _LocalModuleIndex.end())
				return NULL;
			else
			{
				TModuleProxies::iterator it2(_ModuleProxies.find(it->second));
				nlassert(it2 != _ModuleProxies.end());
				return it2->second;
			}
		}

		virtual uint32	getProxyCount() const
		{
			return _ModuleProxies.size();
		}

		/// Fill a vector with the list of proxies managed here. The module are filled in ascending proxy id order.
		virtual void	getModuleProxyList(std::vector<IModuleProxy*> &resultList) const
		{
			TModuleProxies::const_iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
			for (; first != last; ++first)
			{
				resultList.push_back(first->second);
			}
		}


//		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, TModuleMessagePtr &message)
//		{
//		}
		
		virtual void sendModuleMessage(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, const NLNET::CMessage &message)
		{
//			CModuleProxy *modProx = dynamic_cast<CModuleProxy *>(destModule);
//			nlassert(modProx != NULL);
			
			if (addresseeProxy->getGatewayRoute() == NULL)
			{
				// the module is local, just forward the call to the dispatcher
				nlassert(senderProxy != NULL);
				nlassert(_ModuleProxies.find(senderProxy->getModuleProxyId()) != _ModuleProxies.end());

				// invert the message for immediate dispatching if needed
				if (!message.isReading())
					const_cast<CMessage&>(message).invert();

				dispatchMessageModule(senderProxy, addresseeProxy, const_cast<CMessage&>(message));
			}
			else
			{
				// the module is distant, send the message via the route
				// create a message for sending
				CMessage msgHeader("MOD_OP");
				CModuleMessageHeaderCodec::encode(
					msgHeader, 
					CModuleMessageHeaderCodec::mt_oneway, 
					senderProxy->getModuleProxyId(),
					addresseeProxy->getForeignModuleId());
				// send the header
				addresseeProxy->getGatewayRoute()->sendMessage(msgHeader);
				// send the message
				addresseeProxy->getGatewayRoute()->sendMessage(message);
			}
		}
		virtual void dispatchMessageModule(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, CMessage &message)
		{
			// retrieve the address module from the proxy
			nlassert(addresseeProxy->getGatewayRoute() == NULL);
			// As the addressee is local, the foreign proxy id is the local module id (a bit triky...)
			TModuleId addresseeModId = addresseeProxy->getForeignModuleId();
			
			const TModulePtr *adrcp = _PluggedModules.getB(addresseeModId);
			if (adrcp == NULL)
			{
				nlwarning("dispatchMessageModule : dispatching a message to module %u that is not plugged here !", addresseeModId);
				return;
			}

			IModule *addreseeMod = *adrcp;

			// finally, transmit the message to the module
			addreseeMod->onProcessModuleMessage(senderProxy, message);
		}
		/***********************************************************
		 ** Module methods 
		 ***********************************************************/
		void	initModule(const TParsedCommandLine &initInfo)
		{
			CModuleBase::initModule(initInfo);

			// no options for now

			registerSocket();
		}

		void				onServiceUp(const std::string &serviceName, uint16 serviceId)
		{
		}
		void				onServiceDown(const std::string &serviceName, uint16 serviceId)
		{
		}
		void				onModuleUpdate()
		{
			// update the transports
			TTransportList::iterator first(_Transports.begin()), last(_Transports.end());
			for (; first != last; ++first)
			{
				IGatewayTransport *transport = first->second;

				transport->update();
			}
		}
		void				onApplicationExit()
		{
		}
		void				onModuleUp(IModuleProxy *moduleProxy)
		{
		}
		void				onModuleDown(IModuleProxy *moduleProxy)
		{
		}
		void				onProcessModuleMessage(IModuleProxy *senderModuleProxy, CMessage &message)
		{
			// simple message for debug and unit testing
			if (message.getName() == "GW_PING")
			{
				_PingCounter++;
			}
		}
		
		void	onModuleSocketEvent(IModuleSocket *moduleSocket, TModuleSocketEvent eventType)
		{
		}

		/***********************************************************
		 ** Socket methods 
		 ***********************************************************/

		const std::string &getSocketName()
		{
			return getModuleName();
		}

		void _sendModuleMessage(IModule *senderModule, TModuleId destModuleProxyId, const NLNET::CMessage &message ) 
			throw (EModuleNotReachable, EModuleNotPluggedHere)
		{
			// the socket implementation already checked that the module is plugged here
			// just check that the destination module effectively from here
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(senderModule->getModuleId()));
			nlassert(it != _LocalModuleIndex.end());

			// get the sender proxy
			TModuleProxies::iterator it2(_ModuleProxies.find(it->second));
			nlassert(it2 != _ModuleProxies.end());

			IModuleProxy *senderProx = it2->second;

			// get the addressee proxy
			it2 = _ModuleProxies.find(destModuleProxyId);
			nlassert(it2 != _ModuleProxies.end());

			IModuleProxy *destProx = it2->second;


			sendModuleMessage(senderProx, destProx, message);
		}
		
		virtual void _broadcastModuleMessage(IModule *senderModule, const NLNET::CMessage &message)
			throw (EModuleNotPluggedHere)
		{
			nlstop;
		}

		void onModulePlugged(IModule *pluggedModule)
		{
			// A module has just been plugged here, we need to disclose it the the
			// other module, and disclose other module to it.

			// create a proxy for this module
			IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
					this, 
					NULL,	// the module is local, so there is no route
					0,		// the module is local, distance is 0
					pluggedModule->getModuleClassName(), 
//					getGatewayName()+"/"+pluggedModule->getModuleFullyQualifiedName(),
					pluggedModule->getModuleFullyQualifiedName(),
					pluggedModule->getModuleId()	// the module is local, foreign id is the module id
					);

			// and store it in the proxies container
			_ModuleProxies.insert(make_pair(modProx->getModuleProxyId(), modProx));
			// and also in the local module index
			_LocalModuleIndex.insert(make_pair(pluggedModule->getModuleId(), modProx->getModuleProxyId()));


			// trigger the new module proxy event
			onAddModuleProxy(modProx);
//			// disclose the new module to other modules
//			discloseModule(modProx);
//
			// second, disclose already knwon proxies in the gateway to the plugged module
			{
				TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
				for (; first != last; ++first)
				{
					IModuleProxy *modProx = first->second;

					// do not send a moduleUp on the module himself !
					// either the gateway is non null (distant module), or the
					// foreign module id is different of the local module (for local proxy,
					// the foreign module id store the local module id).
					if (modProx->getGatewayRoute() != NULL || modProx->getForeignModuleId() != pluggedModule->getModuleId())
					{
						pluggedModule->onModuleUp(modProx);
					}
				}
			}


		}
		/// Called just after a module as been effectively unplugged from a socket
		void				onModuleUnplugged(IModule *unpluggedModule)
		{
			// remove the proxy info
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(unpluggedModule->getModuleId()));
			nlassert(it != _LocalModuleIndex.end());
			TModuleProxies::iterator it2(_ModuleProxies.find(it->second));
			nlassert(it2 != _ModuleProxies.end());

			IModuleProxy *modProx = it2->second;
//			// warn all connected module that a module become unavailable
//			{
//				TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
//				for (; first != last; ++first)
//				{
//					IModule *otherModule = *first;
//					if (otherModule != unpluggedModule)
//						otherModule->onModuleDown(modProx);
//				}
//			}

			// warn the unplugged module that all proxies in this gateway become unavailable
			{
				TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
				for (; first != last; ++first)
				{
					IModuleProxy *modProx = first->second;

					if (modProx->getGatewayRoute() != NULL 
						|| modProx->getForeignModuleId() != unpluggedModule->getModuleId())
						unpluggedModule->onModuleDown(modProx);
				}
			}

			/// the gateway do the rest of the job
			onRemoveModuleProxy(modProx);

			TModuleId localProxyId = modProx->getModuleProxyId();
			// remove reference to the proxy
			_ModuleProxies.erase(localProxyId);

			// release the module proxy 
			IModuleManager::getInstance().releaseModuleProxy(localProxyId);
			
		}

		void getModuleList(std::vector<IModuleProxy*> &resultList)
		{
			TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
			for (; first != last; ++first)
			{
				resultList.push_back(first->second);
			}
		}


		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CStandardGateway, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, dump, "dump various information about the gateway statue", "")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportAdd, "add a new transport to this gateway", "<transportClass> <instanceName>")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportCmd, "send a command to a transport", "[<transportName> ( <cmd specific to transport> )]*")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportRemove, "remove an existing transport instance", "<transportName>")
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(transportRemove)
		{
			if (args.size() != 1)
				return false;

			TTransportList::iterator it(_Transports.find(args[0]));
			if (it == _Transports.end())
			{
				log.displayNL("Unknown transport named '%s', ignoring command", args[0].c_str());
				return false;
			}

			// delete the transport
			IGatewayTransport *transport = it->second;
			delete transport;
			_Transports.erase(it);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportCmd)
		{
			TParsedCommandLine pcl;
			if (!pcl.parseParamList(rawCommandString))
			{
				log.displayNL("Invalid parameter string, parse error");
				return false;
			}

			for (uint i=1; i<pcl.SubParams.size(); ++i)
			{
				TParsedCommandLine &subParam = pcl.SubParams[i];

				string transportName = subParam.ParamName;
				TTransportList::iterator it(_Transports.find(transportName));
				if (it == _Transports.end())
				{
					log.displayNL("Unknown transport named '%s', ignoring command.", transportName.c_str());
				}
				else if (subParam.SubParams.empty())
				{
					log.displayNL("Can't find sub param list for transport '%s' command", transportName.c_str());
				}
				else
				{
					// ok, we have a valid transport, send the command
					IGatewayTransport *transport = it->second;
					if (!transport->onCommand(subParam))
						return false;
				}
			}

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportAdd)
		{
			if (args.size() != 2)
				return false;

			if (_Transports.find(args[1]) != _Transports.end())
			{
				log.displayNL("A transport with that name already exist !");
				return true;
			}

			IGatewayTransport::TCtorParam param;
			param.Gateway = this;
			IGatewayTransport *transport = NLMISC_GET_FACTORY(IGatewayTransport, std::string).createObject(args[0], param);

			if (transport == NULL)
			{
				log.displayNL("Failed to create a transport with the class '%s'", args[0].c_str());
				return true;
			}

			// Store the transport
			_Transports.insert(make_pair(args[1], transport));

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			if (!args.empty())
				return false;
			
			// recall the dump for the module class
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);
			
			log.displayNL("------------------------------");
			log.displayNL("Dumping gateway informations :");
			log.displayNL("------------------------------");

			log.displayNL("The gateway has %u locally plugged module :", _PluggedModules.getAToBMap().size());
			{
				TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;
					log.displayNL("    ID:%5u : \tName = '%s' \tclass = '%s'", 
						module->getModuleId(),
						module->getModuleName().c_str(),
						module->getModuleClassName().c_str());
				}
			}


			log.displayNL("The gateway as %u transport activated :", _Transports.size());
			{
				TTransportList::iterator first(_Transports.begin()), last(_Transports.end());
				for (; first != last; ++first)
				{
					const string &name = first->first;
					IGatewayTransport *transport = first->second;

					log.displayNL("Transport '%s' (transport class is '%s') :", 
						name.c_str(),
						transport->getClassName().c_str());
					transport->dump(log);
				}
			}

			log.displayNL("------------------------------");
			log.displayNL("------- End of dump ----------");
			log.displayNL("------------------------------");
			return true;
		}

	};


//// 	CStandardGateway::TCallbackToGateway		CStandardGateway::_CallBackToGetwayIdx;

	// register the module factory
	NLNET_REGISTER_MODULE_FACTORY(CStandardGateway, "StandardGateway");



//	/** A simple gateway that interconnect module locally
//	 *	For testing purpose and simple case.
//	 */
//	class CLocalGateway : 
//		public CModuleBase,
//		public IModuleGateway,
//		public CModuleSocket
//	{
//		// the proxy that represent this gateway
//		TModuleGatewayProxyPtr		_ThisProxy;
//
//		typedef CTwinMap<TModuleProxyPtr, string>	TModuleProxies;
//		// The modules proxies
//		TModuleProxies		_ModuleProxies;
//
//		
//	public:
//
//		CLocalGateway()
//		{
//			
//		}
//
//		~CLocalGateway()
//		{
//			// we need to unplug any plugged module
//			while (!_PluggedModules.empty())
//			{
//				(*_PluggedModules.begin())->unplugModule(this);
//			}
//		}
//
//		/***********************************************************
//		 ** Gateway methods 
//		 ***********************************************************/
//		virtual const std::string &getGatewayName() const
//		{
//			return getModuleName();
//		}
//		virtual const std::string &getFullyQualifiedGatewayName() const
//		{
//			return getModuleFullyQualifiedName();
//		}
//		/// Return the gateway proxy of this gateway
//		virtual TModuleGatewayProxyPtr &getGatewayProxy()
//		{
//			nlassert(!_ThisProxy.isNull());
//			return _ThisProxy;
//		}
//
//		virtual  bool isGatewayServerOpen()
//		{
//			return false;
//		}
//
//		virtual CInetAddress getGatewayServerAddress()
//		{
//			CInetAddress invalid;
//
//			return invalid;
//		}
//		virtual void getGatewayClientList(std::vector<TModuleGatewayProxyPtr> gatewayList)
//		{
//			return;
//		}
//		virtual void openGatewayServer(uint16 listeningPort)
//			throw (EGatewayAlreadyOpen)
//		{
//			nlstop;
//		}
//		virtual void closeGatewayServer() 
//			throw (EGatewayNotOpen)
//		{
//			nlstop;
//		}
//		virtual void shutdownGatewayServer()
//		{
//			nlstop;
//		}
//		virtual void onGatewayServerOpen()
//		{
//		}
//		virtual void onGatewayServerClose()
//		{
//		}
//		virtual TModuleGatewayConstant onClientGatewayConnect(TModuleGatewayProxyPtr &clientGateway)
//		{
//			return mgc_reject_connection;
//		}
//		virtual void onClientGatewayDisconnect(TModuleGatewayProxyPtr &clientGateway)
//		{
//		}
//		virtual void getGatewayServerList(std::vector<TModuleGatewayProxyPtr> serverList)
//		{
//			return;
//		}
//		virtual bool isGatewayConnected()
//		{
//			return false;
//		}
//		virtual void connectGateway(CInetAddress serverAdress)
//		{
//			nlstop;
//		}
//		virtual void disconnectGateway(TModuleGatewayProxyPtr &serverGateway)
//		{
//			nlstop;
//		}
//		virtual void onGatewayConnection(const TModuleGatewayProxyPtr &serverGateway, TModuleGatewayConstant connectionResult)
//		{
//			nlstop;
//		}
//		virtual void onGatewayDisconnection(const TModuleGatewayProxyPtr &serverGateway)
//		{
//			nlstop;
//		}
//		virtual void onAddModuleProxy(TModuleProxyPtr &addedModule)
//		{
//			// always disclose module to local modules 
//			discloseModule(addedModule, getGatewayProxy());
//		}
//		virtual void onRemoveModuleProxy(TModuleProxyPtr &removedModule)
//		{
//		}
//		virtual void discloseModule(IModuleProxy *module, IModuleGatewayProxy *gateway)
//			throw (EGatewayNotConnected)
//		{
//			// check that the gateway is this gateway
//			nlassert(gateway == _ThisProxy);
//
//			// check that the module is plugged here
//			nlassert(_ModuleProxies.getB(module) != NULL);
//
//			// warn any plugged module
//			TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
//			for (; first != last; ++first)
//			{
//				if((*first)->getModuleId() != module->getModuleId())
//				{
//					(*first)->onModuleUp(module);
//				}
//			}
//		}
//		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, const TModuleMessagePtr &message)
//		{
//		}
//		virtual void sendModuleMessage(TModuleGatewayProxyPtr &destGateway, const TModuleMessagePtr &message)
//		{
//		}
//		virtual void dispatchMessageModule(TModuleGatewayProxyPtr &senderGateway, const TModuleMessagePtr &message)
//		{
//
//			TModuleId sourceId = message->getSenderModuleId();
//			TModuleProxies::TAToBMap::const_iterator firstSource(_ModuleProxies.getAToBMap().begin()), lastSource(_ModuleProxies.getAToBMap().end());
//			for (; firstSource != lastSource && firstSource->first->getForeignModuleId() != sourceId; ++firstSource) {}
//			nlassert(  firstSource != lastSource );
//
//			TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
//			TModuleId destId = message->getAddresseeModuleId();
//			for (; first != last && (*first)->getModuleId() != destId; ++first) {}
//			if (first != last)
//			{
//				(*first)->onProcessModuleMessage(firstSource->first, message);
//			}
//			
//
//		}
//		/***********************************************************
//		 ** Module methods 
//		 ***********************************************************/
//		void	initModule(const TParsedCommandLine &initInfo)
//		{
//			CModuleBase::initModule(initInfo);
//
//			// in fact, this gateway is so simple, that it have no option !
//
//			IModuleManager::getInstance().registerModuleSocket(this);
//		}
//
//		void				onServiceUp(const std::string &serviceName, uint16 serviceId)
//		{
//		}
//		void				onServiceDown(const std::string &serviceName, uint16 serviceId)
//		{
//		}
//		void				onModuleUpdate()
//		{
//		}
//		void				onApplicationExit()
//		{
//		}
//		void				onModuleUp(const TModuleProxyPtr &module)
//		{
//		}
//		void				onModuleDown(const TModuleProxyPtr &module)
//		{
//		}
//		void				onProcessModuleMessage(const TModuleProxyPtr &senderModule, const TModuleMessagePtr &message)
//		{
//		}
//		void	onModuleSocketEvent(IModuleSocket *moduleSocket, TModuleSocketEvent eventType)
//		{
//		}
//
//		/***********************************************************
//		 ** Socket methods 
//		 ***********************************************************/
//
//		const std::string &getSocketName()
//		{
//			return getModuleName();
//		}
//
//		void _sendModuleMessage(IModule *senderModule, TModuleId destModuleId, TModuleMessagePtr &message ) 
//			throw (EModuleNotReachable, EModuleNotPluggedHere)
//		{
//			TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
//			for (; first != last && first->first->getModuleId() != destModuleId; ++first) {}
//			if (first != last) {  first->first->sendModuleMessage(senderModule, message); return;}
//			throw EModuleNotReachable();
//
//			nlstop;
//		}
//		virtual void _broadcastModuleMessage(IModule *senderModule, TModuleMessagePtr &message)
//			throw (EModuleNotPluggedHere)
//		{
//			nlstop;
//		}
//
//		void onModulePlugged(const TModulePtr &pluggedModule)
//		{
//			// A module has just been plugged here, we need to disclose it the the
//			// other module, and disclose other module to it.
//
//			// create a proxy for this module
//			IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
//					this, 
//					pluggedModule->getModuleClassName(), 
//					getGatewayName()+"/"+pluggedModule->getModuleFullyQualifiedName(),
//					_ThisProxy,
//					pluggedModule->getModuleId());
//
//			// and store it
//			_ModuleProxies.add(modProx, modProx->getModuleName());
//
//			// disclose the new module to other modules
//			discloseModule(modProx, _ThisProxy);
//
//			// second, disclose already plugged proxy to the new one
//			{
//				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
//				for (; first != last; ++first)
//				{
//					if (first->first->getModuleName() != pluggedModule->getModuleFullyQualifiedName())
//						pluggedModule->onModuleUp(first->first);
//				}
//			}
//
//
//		}
//		/// Called just after a module as been effectively unplugged from a socket
//		void				onModuleUnplugged(const TModulePtr &module)
//		{
//			// remove the proxy info
//			TModuleProxies::TBToAMap::const_iterator it(_ModuleProxies.getBToAMap().find(getGatewayName()+"/"+module->getModuleFullyQualifiedName()));
//			nlassert(it != _ModuleProxies.getBToAMap().end());
//
//			IModuleProxy *modProx = it->second;
//			// warn all connected module that a module become unavailable
//			{
//				TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
//				for (; first != last; ++first)
//				{
//					if ((*first)->getModuleFullyQualifiedName() != modProx->getModuleName())
//						(*first)->onModuleDown(it->second);
//				}
//			}
//
//			// warn the unplugged module that all plugged modules are become unavailable
//			{
//				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
//				for (; first != last; ++first)
//				{
//					if (first->first->getModuleName() != module->getModuleFullyQualifiedName())
//						module->onModuleDown(first->first);
//				}
//			}
//
//			TModuleId localId = modProx->getModuleId();
//			// remove reference to the proxy
//			_ModuleProxies.removeWithA(modProx);
//
//			// release the module proxy 
//			IModuleManager::getInstance().releaseModuleProxy(localId);
//			
//		}
//
//		void getModuleList(std::vector<IModuleProxy*> &resultList)
//		{
//			TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
//			for (; first != last; ++first)
//			{
//				resultList.push_back(first->first);
//			}
//		}
//
//	};
//
//
//	// register the module factory
//	NLNET_REGISTER_MODULE_FACTORY(CLocalGateway, "LocalGateway");

	void forceGatewayLink()
	{
	}

} // namespace NLNET

