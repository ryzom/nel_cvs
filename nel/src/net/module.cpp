/** \file module.cpp
 * module base implementation
 *
 * $Id: module.cpp,v 1.3 2005/06/24 19:40:28 boucher Exp $
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
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/inet_address.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/net/module_socket.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace NLNET
{
	//////////////////////////////////////
	// Module factory implementation
	//////////////////////////////////////

	IModuleFactory::IModuleFactory(const std::string &moduleClassName)
		: _ModuleClassName(moduleClassName)
	{
	}

	IModuleFactory::~IModuleFactory()
	{
		// Delete any module that still exist
		while (!_ModuleInstances.empty())
		{
			CRefPtr<IModule> sanityCheck(*(_ModuleInstances.begin()));

			IModuleManager::getInstance().deleteModule(sanityCheck);

			// container is cleared by deleteModule (see below)
			// make sure the module is effectively destroyed
			nlassert(sanityCheck == NULL);
		}

		// if the context is still active
		if (INelContext::isContextInitialised())
			// This factory is no longer available
			IModuleManager::getInstance().unregisterModuleFactory(this);
	}

	const std::string &IModuleFactory::getModuleClassName() const
	{
		return _ModuleClassName;
	}


	void IModuleFactory::deleteModule(IModule *module)
	{
		set<TModulePtr>::iterator it(_ModuleInstances.find(module));
		nlassert(it != _ModuleInstances.end());

		CRefPtr<IModule> sanityCheck(module);

		// removing this smart ptr must release the module
		_ModuleInstances.erase(it);

		nlassert(sanityCheck == NULL);
	}
	
	void IModuleFactory::registerModuleInFactory(TModulePtr module)
	{
		nlassert(module != NULL);

		nlassert(_ModuleInstances.find(module) == _ModuleInstances.end());

		// keep track of the module
		_ModuleInstances.insert(module);

		module->setFactory(this);
	}

	//////////////////////////////////////
	// Module base implementation
	//////////////////////////////////////

	CModuleBase::CModuleBase()
		: _ModuleId(INVALID_MODULE_ID),
		_ModuleFactory(NULL)
	{
	}
	CModuleBase::~CModuleBase()
	{
		// unregister the module in the module manager
//		IModuleManager::getInstance().onModuleDeleted(this);
	}

	TModuleId			CModuleBase::getModuleId() const
	{
		return _ModuleId;
	}
	const std::string	&CModuleBase::getModuleName() const
	{
		return _ModuleName;
	}

	const std::string	&CModuleBase::getModuleClassName() const
	{
		return _ModuleFactory->getModuleClassName();
	}

	const std::string	&CModuleBase::getModuleFullyQualifiedName() const
	{
		if (_FullyQualifedModuleName.empty())
		{
			// build the name
			string hostName = ::NLNET::CInetAddress::localHost().hostName();
			int pid = ::getpid();

			_FullyQualifedModuleName = hostName+":"+toString(pid)+":"+_ModuleName;
		}

		return _FullyQualifedModuleName;
	}


	// Init base module, init module name
	void				CModuleBase::initModule(const TModuleInitInfo &initInfo)
	{
		// read module init param for base module .

		// register this module in the command executor
		registerCommandsHandler();
	}

	const std::string &CModuleBase::getCommandHandlerName() const
	{
		return getModuleName();
	}


	void CModuleBase::plugModule(IModuleSocket *moduleSocket) throw (EModuleAlreadyPluggedHere)
	{
		CModuleSocket *sock = dynamic_cast<CModuleSocket*>(moduleSocket);
		nlassert(sock != NULL);

		TModuleSockets::iterator it(_ModuleSockets.find(moduleSocket));
		if (it != _ModuleSockets.end())
			throw EModuleAlreadyPluggedHere();


		// ok, we can plug the module

		sock->_onModulePlugged(this);

		// all fine, store the socket pointer.
		_ModuleSockets.insert(moduleSocket);
	}

	void CModuleBase::unplugModule(IModuleSocket *moduleSocket)  throw (EModuleNotPluggedHere)
	{
		CModuleSocket *sock = dynamic_cast<CModuleSocket*>(moduleSocket);
		nlassert(sock != NULL);

		TModuleSockets::iterator it(_ModuleSockets.find(moduleSocket));
		if (it == _ModuleSockets.end())
			throw EModuleNotPluggedHere();
		
		sock->_onModuleUnplugged(TModulePtr(this));

		_ModuleSockets.erase(it);
	}

	void CModuleBase::getPluggedSocketList(std::vector<IModuleSocket*> &resultList)
	{
		TModuleSockets::iterator first(_ModuleSockets.begin()), last(_ModuleSockets.end());
		for (; first != last; ++first)
		{
			resultList.push_back(*first);
		}
	}


	void CModuleBase::setFactory(IModuleFactory *factory)
	{
		nlassert(_ModuleFactory == NULL);

		_ModuleFactory = factory;
	}

	IModuleFactory *CModuleBase::getFactory()
	{
		return _ModuleFactory;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, plug)
	{
		if (args.size() != 1)
			return false;

		IModuleSocket *socket = IModuleManager::getInstance().getModuleSocket(args[0]);

		if (socket == NULL)
		{
			log.displayNL("Unknown socket named '%s'", args[0].c_str());
			return true;
		}

		plugModule(socket);

		if (_ModuleSockets.find(socket) == _ModuleSockets.end())
		{
			log.displayNL("Failed to plug the module '%s' into the socket '%s'", 
				getModuleName().c_str(),
				socket->getSocketName().c_str());
		}
		else
			log.displayNL("Module '%s' plugged into the socket '%s'", 
				getModuleName().c_str(),
				socket->getSocketName().c_str());

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, unplug)
	{
		if (args.size() != 1)
			return false;

		IModuleSocket *socket = IModuleManager::getInstance().getModuleSocket(args[0]);

		if (socket == NULL)
		{
			log.displayNL("Unknown socket named '%s'", args[0].c_str());
			return true;
		}

		if (_ModuleSockets.find(socket) == _ModuleSockets.end())
		{
			log.displayNL("The module '%s' is not plugged in the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());
			return true;
		}

		unplugModule(socket);

		if (_ModuleSockets.find(socket) != _ModuleSockets.end())
			log.displayNL("Failed to unplug the module '%s' from the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());
		else
			log.displayNL("Module '%s' unplugged out of the socket '%s'",
				getModuleName().c_str(),
				socket->getSocketName().c_str());

		return true;
	}

	NLMISC_CLASS_COMMAND_IMPL(CModuleBase, dump)
	{
		if (args.size() != 0)
			return false;

		log.displayNL("Dumping base module state :");
		log.displayNL("  Module class : '%s'", _ModuleFactory->getModuleClassName().c_str());
		log.displayNL("  Module ID : %u", _ModuleId);
		log.displayNL("  The module is plugged into %u sockets :", _ModuleSockets.size());
		{
			TModuleSockets::iterator first(_ModuleSockets.begin()), last(_ModuleSockets.end());
			for (; first != last; ++first)
			{
				IModuleSocket *ps = *first;
				vector<IModuleProxy*>	proxies;
				ps->getModuleList(proxies);

				log.displayNL("    Socket '%s', %u modules reachable :", ps->getSocketName().c_str(), proxies.size()-1);

				for (uint i=0; i<proxies.size(); ++i)
				{
					string name = proxies[i]->getModuleName();
					if (name.find('/') != string::npos)
						name = name.substr(name.find('/')+1);
					if (name != getModuleFullyQualifiedName())
					{
						log.displayNL("      Module '%s' (Module ID : %u, class : '%s')", 
							proxies[i]->getModuleName().c_str(),
							proxies[i]->getModuleId(),
							proxies[i]->getModuleClassName().c_str());
					}
				}
			}
		}

		return true;
	}


	/************************************************************************
	 * CModuleProxy impl
	 ************************************************************************/

	CModuleProxy::CModuleProxy(TModuleId localModuleId, const std::string &moduleClassName, const std::string &fullyQualifiedModuleName)
		: _LocalModuleId(localModuleId),
		_ModuleClassName(moduleClassName),
		_FullyQualifiedModuleName(fullyQualifiedModuleName),
		_ForeignModuleId(INVALID_MODULE_ID)
	{
	}

	TModuleId	CModuleProxy::getModuleId() const
	{
		return _LocalModuleId;
	}
	const std::string &CModuleProxy::getModuleName() const
	{
		return _FullyQualifiedModuleName;
	}
	const std::string &CModuleProxy::getModuleClassName() const
	{
		return _ModuleClassName;
	}

	IModuleGateway *CModuleProxy::getModuleGateway() const
	{
		return _Gateway;
	}

	void		CModuleProxy::sendModuleMessage(IModule *senderModule, const TModuleMessagePtr &message)
		throw (EModuleNotReachable)
	{
		if (_Gateway == NULL || _ForeignGateway == NULL || !_ForeignGateway->isConnected())	
			throw EModuleNotReachable();

		// fill message routing information
		message->_MessageType = CModuleMessage::mt_oneway;
		message->_SenderModuleId = senderModule->getModuleId();
		message->_AddresseeModuleId = _LocalModuleId;

		_Gateway->sendModuleMessage(_ForeignGateway, message);
	}

} // namespace NLNET
