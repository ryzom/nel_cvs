
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "welcome_service_itf.h"

namespace WS
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CWelcomeServiceSkel::TInterceptor::TMessageHandlerMap &CWelcomeServiceSkel::TInterceptor::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("WU"), &TInterceptor::welcomeUser_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("DU"), &TInterceptor::disconnectUser_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CWelcomeServiceSkel::TInterceptor::onProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}

	
	void CWelcomeServiceSkel::TInterceptor::welcomeUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		uint32	charId;
			nlRead(__message, serial, charId);
		std::string	userName;
			nlRead(__message, serial, userName);
		NLNET::CLoginCookie	cookie;
			nlRead(__message, serial, cookie);
		std::string	priviledge;
			nlRead(__message, serial, priviledge);
		std::string	exPriviledge;
			nlRead(__message, serial, exPriviledge);
		WS::TUserRole	mode;
			nlRead(__message, serial, mode);
		uint32	instanceId;
			nlRead(__message, serial, instanceId);
		Skeleton->welcomeUser(sender, charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);
	}

	void CWelcomeServiceSkel::TInterceptor::disconnectUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		uint32	userId;
			nlRead(__message, serial, userId);
		Skeleton->disconnectUser(sender, userId);
	}
		// ask the welcome service to welcome a character
	void CWelcomeServiceProxy::welcomeUser(NLNET::IModule *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->welcomeUser(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_welcomeUser(__message, charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// ask the welcome service to disconnect a user
	void CWelcomeServiceProxy::disconnectUser(NLNET::IModule *sender, uint32 userId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->disconnectUser(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_disconnectUser(__message, userId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceProxy::buildMessageFor_welcomeUser(NLNET::CMessage &__message, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
	{
		__message.setType("WU");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < std::string& > (userName));
			nlWrite(__message, serial, const_cast < NLNET::CLoginCookie& > (cookie));
			nlWrite(__message, serial, const_cast < std::string& > (priviledge));
			nlWrite(__message, serial, const_cast < std::string& > (exPriviledge));
			nlWrite(__message, serial, mode);
			nlWrite(__message, serial, instanceId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceProxy::buildMessageFor_disconnectUser(NLNET::CMessage &__message, uint32 userId)
	{
		__message.setType("DU");
			nlWrite(__message, serial, userId);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CWelcomeServiceClientSkel::TInterceptor::TMessageHandlerMap &CWelcomeServiceClientSkel::TInterceptor::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("RWS"), &TInterceptor::registerWS_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("WUR"), &TInterceptor::welcomeUserResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UCP"), &TInterceptor::updateConnectedPlayerCount_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CWelcomeServiceClientSkel::TInterceptor::onProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}

	
	void CWelcomeServiceClientSkel::TInterceptor::registerWS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		uint32	shardId;
			nlRead(__message, serial, shardId);
		uint32	fixedSessionId;
			nlRead(__message, serial, fixedSessionId);
		Skeleton->registerWS(sender, shardId, fixedSessionId);
	}

	void CWelcomeServiceClientSkel::TInterceptor::welcomeUserResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		uint32	userId;
			nlRead(__message, serial, userId);
		bool	ok;
			nlRead(__message, serial, ok);
		std::string	shardAddr;
			nlRead(__message, serial, shardAddr);
		std::string	errorMsg;
			nlRead(__message, serial, errorMsg);
		Skeleton->welcomeUserResult(sender, userId, ok, shardAddr, errorMsg);
	}

	void CWelcomeServiceClientSkel::TInterceptor::updateConnectedPlayerCount_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		uint32	nbOnlinePlayers;
			nlRead(__message, serial, nbOnlinePlayers);
		uint32	nbPendingPlayers;
			nlRead(__message, serial, nbPendingPlayers);
		Skeleton->updateConnectedPlayerCount(sender, nbOnlinePlayers, nbPendingPlayers);
	}
		// Register the welcome service in the ring session manager
		// The provided sessionId will be non-zero only for a shard with a fixed sessionId
	void CWelcomeServiceClientProxy::registerWS(NLNET::IModule *sender, uint32 shardId, uint32 fixedSessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerWS(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId, fixedSessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_registerWS(__message, shardId, fixedSessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// return for welcome user
	void CWelcomeServiceClientProxy::welcomeUserResult(NLNET::IModule *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->welcomeUserResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId, ok, shardAddr, errorMsg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_welcomeUserResult(__message, userId, ok, shardAddr, errorMsg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// transmits the current player counts
	void CWelcomeServiceClientProxy::updateConnectedPlayerCount(NLNET::IModule *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateConnectedPlayerCount(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), nbOnlinePlayers, nbPendingPlayers);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_updateConnectedPlayerCount(__message, nbOnlinePlayers, nbPendingPlayers);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_registerWS(NLNET::CMessage &__message, uint32 shardId, uint32 fixedSessionId)
	{
		__message.setType("RWS");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serial, fixedSessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_welcomeUserResult(NLNET::CMessage &__message, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
	{
		__message.setType("WUR");
			nlWrite(__message, serial, userId);
			nlWrite(__message, serial, ok);
			nlWrite(__message, serial, const_cast < std::string& > (shardAddr));
			nlWrite(__message, serial, const_cast < std::string& > (errorMsg));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_updateConnectedPlayerCount(NLNET::CMessage &__message, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
	{
		__message.setType("UCP");
			nlWrite(__message, serial, nbOnlinePlayers);
			nlWrite(__message, serial, nbPendingPlayers);


		return __message;
	}

}
