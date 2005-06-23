/** \file module_gateway.h
 * module gateway interface
 *
 * $Id: module_gateway.cpp,v 1.1 2005/06/23 16:38:14 boucher Exp $
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

using namespace std;
using namespace NLMISC;



namespace NLNET
{

	/** A simple gateway that interconnect module locally
	 *	For testing purpose and simple case.
	 */
	class CLocalGateway : 
		public CModuleBase,
		public IModuleGateway,
		public CModuleSocket
	{
		// the proxy that represent this gateway
		TModuleGatewayProxyPtr		_ThisProxy;

		typedef CTwinMap<TModuleProxyPtr, string>	TModuleProxies;
		// The modules proxies
		TModuleProxies		_ModuleProxies;

		
	public:

		CLocalGateway()
		{
			
		}

		~CLocalGateway()
		{
			// we need to unplug any plugged module
			while (!_PluggedModules.empty())
			{
				(*_PluggedModules.begin())->unplugModule(this);
			}
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
		virtual TModuleGatewayProxyPtr &getGatewayProxy()
		{
			nlassert(!_ThisProxy.isNull());
			return _ThisProxy;
		}

		virtual  bool isGatewayServerOpen()
		{
			return false;
		}

		virtual CInetAddress getGatewayServerAddress()
		{
			CInetAddress invalid;

			return invalid;
		}
		virtual void getGatewayClientList(std::vector<TModuleGatewayProxyPtr> gatewayList)
		{
			return;
		}
		virtual void openGatewayServer(CInetAddress listeningAddress)
			throw (EGatewayAlreadyOpen)
		{
			nlstop;
		}
		virtual void closeGatewayServer() 
			throw (EGatewayNotOpen)
		{
			nlstop;
		}
		virtual void shutdownGatewayServer()
		{
			nlstop;
		}
		virtual void onGatewayServerOpen()
		{
		}
		virtual void onGatewayServerClose()
		{
		}
		virtual TModuleGatewayConstant onClientGatewayConnect(TModuleGatewayProxyPtr &clientGateway)
		{
			return mgc_reject_connection;
		}
		virtual void onClientGatewayDisconnect(TModuleGatewayProxyPtr &clientGateway)
		{
		}
		virtual void getGatewayServerList(std::vector<TModuleGatewayProxyPtr> serverList)
		{
			return;
		}
		virtual bool isGatewayConnected()
		{
			return false;
		}
		virtual void connectGateway(CInetAddress serverAdress)
		{
			nlstop;
		}
		virtual void disconnectGateway(TModuleGatewayProxyPtr &serverGateway)
		{
			nlstop;
		}
		virtual void onGatewayConnection(const TModuleGatewayProxyPtr &serverGateway, TModuleGatewayConstant connectionResult)
		{
			nlstop;
		}
		virtual void onGatewayDisconnection(const TModuleGatewayProxyPtr &serverGateway)
		{
			nlstop;
		}
		virtual void onAddModuleProxy(TModuleProxyPtr &addedModule)
		{
			// always disclose module to local modules 
			discloseModule(addedModule, getGatewayProxy());
		}
		virtual void onRemoveModuleProxy(TModuleProxyPtr &removedModule)
		{
		}
		virtual void discloseModule(IModuleProxy *module, IModuleGatewayProxy *gateway)
			throw (EGatewayNotConnected)
		{
			// check that the gateway is this gateway
			nlassert(gateway == _ThisProxy);

			// check that the module is plugged here
			nlassert(_ModuleProxies.getB(module) != NULL);

			// warn any plugged module
			TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
			for (; first != last; ++first)
			{
				if((*first)->getModuleId() != module->getModuleId())
				{
					(*first)->onModuleUp(module);
				}
			}
		}
		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, const TModuleMessagePtr &message)
		{
		}
		virtual void sendModuleMessage(TModuleGatewayProxyPtr &destGateway, const TModuleMessagePtr &message)
		{
		}
		virtual void dispatchMessageModule(TModuleGatewayProxyPtr &senderGateway, const TModuleMessagePtr &message)
		{
		}
		/***********************************************************
		 ** Module methods 
		 ***********************************************************/
		void	initModule(const TModuleInitInfo &initInfo)
		{
			CModuleBase::initModule(initInfo);

			// in fact, this gateway is so simple, that it have no option !

			IModuleManager::getInstance().registerModuleSocket(this);
		}

		void				onServiceUp(const std::string &serviceName, uint16 serviceId)
		{
		}
		void				onServiceDown(const std::string &serviceName, uint16 serviceId)
		{
		}
		void				onModuleUpdate()
		{
		}
		void				onApplicationExit()
		{
		}
		void				onModuleUp(const TModuleProxyPtr &module)
		{
		}
		void				onModuleDown(const TModuleProxyPtr &module)
		{
		}
		void				onProcessModuleMessage(const TModuleProxyPtr &senderModule, const TModuleMessagePtr &message)
		{
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

		void _sendModuleMessage(IModule *senderModule, TModuleId destModuleId, TModuleMessagePtr &message ) 
			throw (EModuleNotReachable, EModuleNotPluggedHere)
		{
			nlstop;
		}
		virtual void _broadcastModuleMessage(IModule *senderModule, TModuleMessagePtr &message)
			throw (EModuleNotPluggedHere)
		{
			nlstop;
		}

		void onModulePlugged(const TModulePtr &pluggedModule)
		{
			// A module has just been plugged here, we need to disclose it the the
			// other module, and disclose other module to it.

			// create a proxy for this module
			IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
					this, 
					pluggedModule->getModuleClassName(), 
					getGatewayName()+"/"+pluggedModule->getModuleFullyQualifiedName(),
					_ThisProxy,
					pluggedModule->getModuleId());

			// and store it
			_ModuleProxies.add(modProx, modProx->getModuleName());

			// disclose the new module to other modules
			discloseModule(modProx, _ThisProxy);

			// second, disclose already plugged proxy to the new one
			{
				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
				for (; first != last; ++first)
				{
					if (first->first->getModuleName() != pluggedModule->getModuleFullyQualifiedName())
						pluggedModule->onModuleUp(first->first);
				}
			}


		}
		/// Called just after a module as been effectively unplugged from a socket
		void				onModuleUnplugged(const TModulePtr &module)
		{
			// remove the proxy info
			TModuleProxies::TBToAMap::const_iterator it(_ModuleProxies.getBToAMap().find(getGatewayName()+"/"+module->getModuleFullyQualifiedName()));
			nlassert(it != _ModuleProxies.getBToAMap().end());

			IModuleProxy *modProx = it->second;
			// warn all connected module that a module become unavailable
			{
				TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
				for (; first != last; ++first)
				{
					if ((*first)->getModuleFullyQualifiedName() != modProx->getModuleName())
						(*first)->onModuleDown(it->second);
				}
			}

			// warn the unplugged module that all plugged modules are become unavailable
			{
				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
				for (; first != last; ++first)
				{
					if (first->first->getModuleName() != module->getModuleFullyQualifiedName())
						module->onModuleDown(first->first);
				}
			}

			TModuleId localId = modProx->getModuleId();
			// remove reference to the proxy
			_ModuleProxies.removeWithA(modProx);

			// release the module proxy 
			IModuleManager::getInstance().releaseModuleProxy(localId);
			
		}

		void getModuleList(std::vector<IModuleProxy*> &resultList)
		{
			TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
			for (; first != last; ++first)
			{
				resultList.push_back(first->first);
			}
		}

	};


	// register the module factory
	NLNET_REGISTER_MODULE_FACTORY(CLocalGateway, "LocalGateway");

	void forceGatewayLink()
	{
	}

} // namespace NLNET

