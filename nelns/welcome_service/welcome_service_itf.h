
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef WELCOME_SERVICE_ITF
#define WELCOME_SERVICE_ITF
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/misc/string_conversion.h"

#include "nel/net/login_cookie.h"
	
#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
namespace WS
{
	

	struct TUserRole
	{
		enum TValues
		{
			ur_player,
			ur_editor,
			ur_animator,
			
			invalid
		};

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_player)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_editor)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_animator)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TUserRole()
			: _Value(invalid)
		{
		}
		TUserRole(TValues value)
			: _Value(value)
		{
		}

		TUserRole(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TUserRole &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TUserRole &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TUserRole &other) const
		{
			return _Value < other._Value;
		}

		const std::string &toString()
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue()
		{
			return _Value;
		}
	};
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceSkel
	{
	protected:
		CWelcomeServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CWelcomeServiceSkel()
		{
		}



	private:
		typedef void (CWelcomeServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const
		{
			static TMessageHandlerMap handlers;
			static bool init = false;

			if (!init)
			{
				std::pair < TMessageHandlerMap::iterator, bool > res;
				
				res = handlers.insert(std::make_pair(std::string("WU"), &CWelcomeServiceSkel::welcomeUser_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("DU"), &CWelcomeServiceSkel::disconnectUser_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				init = true;
			}

			return handlers;			
		}

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	private:
		
		void welcomeUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
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
			welcomeUser(sender, charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);
		}

		void disconnectUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			uint32	userId;
			nlRead(__message, serial, userId);
			disconnectUser(sender, userId);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// ask the welcome service to welcome a user
		virtual void welcomeUser(NLNET::IModuleProxy *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId) =0;
		// ask the welcome service to disconnect a user
		virtual void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CWelcomeServiceSkel	*_LocalModuleSkel;


	public:
		CWelcomeServiceProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "WelcomeService");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CWelcomeServiceSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CWelcomeServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// ask the welcome service to welcome a user
		void welcomeUser(NLNET::IModule *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
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
		void disconnectUser(NLNET::IModule *sender, uint32 userId)
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
		static const NLNET::CMessage &buildMessageFor_welcomeUser(NLNET::CMessage &__message, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
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
		static const NLNET::CMessage &buildMessageFor_disconnectUser(NLNET::CMessage &__message, uint32 userId)
		{
			__message.setType("DU");
			nlWrite(__message, serial, userId);


			return __message;
		}




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceClientSkel
	{
	protected:
		CWelcomeServiceClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CWelcomeServiceClientSkel()
		{
		}



	private:
		typedef void (CWelcomeServiceClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const
		{
			static TMessageHandlerMap handlers;
			static bool init = false;

			if (!init)
			{
				std::pair < TMessageHandlerMap::iterator, bool > res;
				
				res = handlers.insert(std::make_pair(std::string("RWS"), &CWelcomeServiceClientSkel::registerWS_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("WUR"), &CWelcomeServiceClientSkel::welcomeUserResult_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("UCP"), &CWelcomeServiceClientSkel::updateConnectedPlayerCount_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				init = true;
			}

			return handlers;			
		}

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	private:
		
		void registerWS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			uint32	shardId;
			nlRead(__message, serial, shardId);
			uint32	fixedSessionId;
			nlRead(__message, serial, fixedSessionId);
			registerWS(sender, shardId, fixedSessionId);
		}

		void welcomeUserResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			uint32	userId;
			nlRead(__message, serial, userId);
			bool	ok;
			nlRead(__message, serial, ok);
			std::string	shardAddr;
			nlRead(__message, serial, shardAddr);
			std::string	errorMsg;
			nlRead(__message, serial, errorMsg);
			welcomeUserResult(sender, userId, ok, shardAddr, errorMsg);
		}

		void updateConnectedPlayerCount_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			uint32	nbOnlinePlayers;
			nlRead(__message, serial, nbOnlinePlayers);
			uint32	nbPendingPlayers;
			nlRead(__message, serial, nbPendingPlayers);
			updateConnectedPlayerCount(sender, nbOnlinePlayers, nbPendingPlayers);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// Register the welcome service in the ring session manager
		// The provided sessionId will be non-zero only for a shard with a fixed sessionId
		virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId, uint32 fixedSessionId) =0;
		// return for welcome user
		virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg) =0;
		// transmits the current player counts
		virtual void updateConnectedPlayerCount(NLNET::IModuleProxy *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CWelcomeServiceClientSkel	*_LocalModuleSkel;


	public:
		CWelcomeServiceClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CWelcomeServiceClientSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CWelcomeServiceClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// Register the welcome service in the ring session manager
		// The provided sessionId will be non-zero only for a shard with a fixed sessionId
		void registerWS(NLNET::IModule *sender, uint32 shardId, uint32 fixedSessionId)
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
		void welcomeUserResult(NLNET::IModule *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
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
		void updateConnectedPlayerCount(NLNET::IModule *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
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
		static const NLNET::CMessage &buildMessageFor_registerWS(NLNET::CMessage &__message, uint32 shardId, uint32 fixedSessionId)
		{
			__message.setType("RWS");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serial, fixedSessionId);


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_welcomeUserResult(NLNET::CMessage &__message, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
		{
			__message.setType("WUR");
			nlWrite(__message, serial, userId);
			nlWrite(__message, serial, ok);
			nlWrite(__message, serial, const_cast < std::string& > (shardAddr));
			nlWrite(__message, serial, const_cast < std::string& > (errorMsg));


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateConnectedPlayerCount(NLNET::CMessage &__message, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
		{
			__message.setType("UCP");
			nlWrite(__message, serial, nbOnlinePlayers);
			nlWrite(__message, serial, nbPendingPlayers);


			return __message;
		}




	};

}
	
#endif
