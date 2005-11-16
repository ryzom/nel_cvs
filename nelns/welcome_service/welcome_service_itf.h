
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef WELCOME_SERVICE_ITF
#define WELCOME_SERVICE_ITF
#include "nel/net/module.h"
#include "nel/misc/string_conversion.h"

#include "nel/net/login_cookie.h"
	
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
	class CWelcomeServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

	public:
		CWelcomeServiceProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "WelcomeService");
			_ModuleProxy = proxy;
		}
		virtual ~CWelcomeServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

			// ask the welcome service to welcome a user
		void welcomeUser(NLNET::IModule *sender, uint32 userId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
		{
			NLNET::CMessage message("WU");
			message.serial(userId);
			message.serial(const_cast < std::string& > (userName));
			message.serial(const_cast < NLNET::CLoginCookie& > (cookie));
			message.serial(const_cast < std::string& > (priviledge));
			message.serial(const_cast < std::string& > (exPriviledge));
			message.serial(mode);
			message.serial(instanceId);

			_ModuleProxy->sendModuleMessage(sender, message);
		}
		// ask the welcome service to disconnect a user
		void disconnectUser(NLNET::IModule *sender, uint32 userId)
		{
			NLNET::CMessage message("DU");
			message.serial(userId);

			_ModuleProxy->sendModuleMessage(sender, message);
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
		typedef void (CWelcomeServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, NLNET::CMessage &message);
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
		bool onDispatchMessage(NLNET::IModuleProxy *sender, NLNET::CMessage &message)
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
		
		void welcomeUser_skel(NLNET::IModuleProxy *sender, NLNET::CMessage &message)
		{
			uint32	userId;
			message.serial(userId);
			std::string	userName;
			message.serial(userName);
			NLNET::CLoginCookie	cookie;
			message.serial(cookie);
			std::string	priviledge;
			message.serial(priviledge);
			std::string	exPriviledge;
			message.serial(exPriviledge);
			WS::TUserRole	mode;
			message.serial(mode);
			uint32	instanceId;
			message.serial(instanceId);
			welcomeUser(sender, userId, userName, cookie, priviledge, exPriviledge, mode, instanceId);
		}

		void disconnectUser_skel(NLNET::IModuleProxy *sender, NLNET::CMessage &message)
		{
			uint32	userId;
			message.serial(userId);
			disconnectUser(sender, userId);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// ask the welcome service to welcome a user
		virtual void welcomeUser(NLNET::IModuleProxy *sender, uint32 userId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId) =0;
		// ask the welcome service to disconnect a user
		virtual void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId) =0;


	};
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

	public:
		CWelcomeServiceClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;
		}
		virtual ~CWelcomeServiceClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

			// Register the welcome service in the ring session manager
		void registerWS(NLNET::IModule *sender, uint32 shardId)
		{
			NLNET::CMessage message("RWS");
			message.serial(shardId);

			_ModuleProxy->sendModuleMessage(sender, message);
		}
		// return for wecome user
		void welcomeUserResult(NLNET::IModule *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
		{
			NLNET::CMessage message("WUR");
			message.serial(userId);
			message.serial(ok);
			message.serial(const_cast < std::string& > (shardAddr));
			message.serial(const_cast < std::string& > (errorMsg));

			_ModuleProxy->sendModuleMessage(sender, message);
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
		typedef void (CWelcomeServiceClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, NLNET::CMessage &message);
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
				
				init = true;
			}

			return handlers;			
		}

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, NLNET::CMessage &message)
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
		
		void registerWS_skel(NLNET::IModuleProxy *sender, NLNET::CMessage &message)
		{
			uint32	shardId;
			message.serial(shardId);
			registerWS(sender, shardId);
		}

		void welcomeUserResult_skel(NLNET::IModuleProxy *sender, NLNET::CMessage &message)
		{
			uint32	userId;
			message.serial(userId);
			bool	ok;
			message.serial(ok);
			std::string	shardAddr;
			message.serial(shardAddr);
			std::string	errorMsg;
			message.serial(errorMsg);
			welcomeUserResult(sender, userId, ok, shardAddr, errorMsg);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// Register the welcome service in the ring session manager
		virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId) =0;
		// return for wecome user
		virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg) =0;


	};
	
}
	
#endif
