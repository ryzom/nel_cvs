/** \file module.h
 * module interface
 *
 * $Id: module.h,v 1.10.4.2 2005/11/22 21:00:24 distrib Exp $
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


#ifndef NL_FILE_MODULE_H
#define NL_FILE_MODULE_H

#include "nel/misc/factory.h"
#include "nel/misc/command.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/co_task.h"
#include "nel/net/message.h"
#include "module_common.h"

namespace NLNET
{

	class CGatewayRoute;

	/** This is the interface for the a module.
	 *	It describe interaction with the module itself,
	 *	with the module manager, and the module socket.
	 */
	class IModule : public NLMISC::CRefCount
	{
		friend class IModuleFactory;

		virtual void setFactory(IModuleFactory *factory) =0;

	public:
		/// The module is already plugged in the specified pluging
		class EModuleAlreadyPluggedHere : public NLMISC::Exception
		{
		};

		/// An operation invocation has failed (mostly because of lost server module)
		class EInvokeFailed : public NLMISC::Exception
		{
		};

		// Module management =====================

		virtual ~IModule() {}

		/** Module initialization.
		 */
		virtual void				initModule(const TParsedCommandLine &initInfo) =0;

		//@name Basic module information
		//@{
		/** Return the module ID. Each module has a local unique ID assigned 
		 *	by the manager during module creation.
		 *	This ID is local because it is only valid inside a given process.
		 *	When module are declared in another process, they receive a 
		 *	local ID that is different than the ID in their host process.
		 */
		virtual TModuleId			getModuleId() const =0;
		/** Return the module name. Each module instance must have a unique 
		 *	name in the host process.
		 *	If no mane is given during module creation, the module manager
		 *	build a unique name from the module class and a number.
		 */
		virtual const std::string	&getModuleName() const =0;
		/// Return the module class.
		virtual const std::string	&getModuleClassName() const =0;
		/** Return the module fully qualified name.
		 *	the MFQN is the identifier that is used across process to identify
		 *	each module. 
		 *	The MDQNis composed from the computer host name, the process ID and
		 *	the module name.
		 *	Format : <hostname>:<pid>:<modulename>
		 *	This name is guarantied to be unique (at least, if the host name
		 *	is unique !)
		 */
		virtual const std::string	&getModuleFullyQualifiedName() const =0;

		/** Return the manifest of the module.
		 *	The manifest is a simple string of undefined format.
		 *	The manifest is used by a module to expose some intention
		 *	or affinity (or whaterver else yuou could imagine) of the
		 *	module.
		 *	The manifest if transmited along with the module proxy, allowing
		 *	any module seeing the proxy to read the manifest.
		 *	You should not use manifest to put big string because it is
		 *	transmited with proxy information.
		 *	Likewise, you should never use manifest to transmit critical data
		 *	(such as password) because any module can read it.
		 */
		virtual const std::string	&getModuleManifest() const =0;

		/** Tell if the module implementation support immediate dispatching.
		 *	Immediate dispatching is when a message is send between 
		 *	collocated module (i.e module on the same gateway). In that case,
		 *	the gateway forward the module immediately to the addressee module.
		 *	In some case, this is not the expected behavior because it give
		 *	different result depending if the communicating modules are 
		 *	collocated or not.
		 *	It you module didn't support collocation optimisation, you must
		 *	override this method and return false.
		 *	In this case, message are stored in a queue and dispatching occur
		 *	in the next gateway update, just before the network update.
		 */
		virtual bool isImmediateDispatchingSupported() const { return true; }
		//@}
			
		//@name Callback from the module manager
		//@{
		/// A Nel layer 5 service has started.
		virtual void				onServiceUp(const std::string &serviceName, uint16 serviceId) =0;
		/// A Nel layer 5 service has stopped.
		virtual void				onServiceDown(const std::string &serviceName, uint16 serviceId) = 0;
		/** Regular update from application.
		 *	If the application is a service, then it call CModuleManager::updateModules at each 
		 *	service loop.
		 *	If the application is a regular application, then you have to call manually
		 *	CModuleManager::updateModules at regular intervals.
		 */
		virtual void				onModuleUpdate() =0;
		/** The service main loop is terminating it job', all module while be
		 *	disconnected and removed after this callback.
		 */
		virtual void				onApplicationExit() = 0;
		//@}

		// socket management =====================

		//@name module sockets operation
		//@{
		/** Plug this module in the specified socket.
		 *	Note that a module can be plugged in several socket at the same
		 *	time, but not twice in the same socket.
		 */
		virtual void				plugModule(IModuleSocket *moduleSocket)
			throw (EModuleAlreadyPluggedHere)
			=0;
		/** Unplug this module from the specified socket.
		 *	Note that a module can be plugged in several socket at the same
		 *	time, but not twice in the same socket.
		 *	Throw an exception if the socket is not currently plug into
		 *	the specified socket.
		 */
		virtual void				unplugModule(IModuleSocket *moduleSocket) 
			throw (EModuleNotPluggedHere)
			=0;
		/** Fill resultList vector with the list of socket into
		 *	witch this module is currently plugged.
		 *	This method don't clear the result vector before filling it.
		 */
		virtual void				getPluggedSocketList(std::vector<IModuleSocket*> &resultList) =0;
		//@}


		//@name Callback from module sockets
		//@{
		/** Called by a socket to inform this module that another
		 *	module has been created OR has been made accessible (due to
		 *	a gateway connection).
		 */
		virtual void				onModuleUp(IModuleProxy *moduleProxy) = 0;
		/** Called by a socket to inform this module that another
		 *	module has been deleted OR has been no more accessible (due to
		 *	some gateway disconnection).
		 */
		virtual void				onModuleDown(IModuleProxy *moduleProxy) =0;
		/** Called by a socket to receive a message in the module context.
		 *	Basic implementation either forward directly to onProcessModuleMessage
		 *	or queue the message in the coroutine message queue (when a synchronous
		 *	messaging coroutine is started) for later dispatching.
		 */
		virtual void				onReceiveModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message) =0;
		/** Called internally by basic module imp to process the message by
		 *	application code.
		 */
		virtual void				onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message) =0;
		/** Called by a socket to inform this module that the security
		 *	data attached to a proxy have changed.
		 */
		virtual void				onModuleSecurityChange(IModuleProxy *moduleProxy) =0;

		/** Do a module operation invocation.
		 *	Caller MUST be in a module task to call this method.
		 *	The call is blocking until receptions of the operation
		 *	result message (or a module down)
		 */
		virtual void		invokeModuleOperation(IModuleProxy *destModule, const NLNET::CMessage &opMsg, NLNET::CMessage &resultMsg) 
			throw (EInvokeFailed)
			=0;

		//@}

		//@name Callback from module sockets management
		//@{
		enum TModuleSocketEvent
		{
			mse_plugged,
			mse_before_unplug,
			mse_unplugged,
		};
		virtual void	onModuleSocketEvent(IModuleSocket *moduleSocket, TModuleSocketEvent eventType) =0;
		/// Called just after a module as been effectively plugged into a socket
//		virtual void				onModulePlugged(IModuleSocket *moduleSocket) =0;
//		/** Called just before the module is unplug from a socket.
//		 *	The module can still send module message, but the unpluging
//		 *	will be done just after.
//		 */
//		virtual void				onBeforeModuleUnplugged(IModuleSocket *moduleSocket) =0;
//		/// Called just after a module as been effectively unplugged from a socket
//		virtual void				onModuleUnpluged() =0;
		//@}

		//@{
		//@name internal method, should not be used by client code

		virtual void _onModuleDown(IModuleProxy *removedProxy) =0;

	};

	const TModulePtr		NullModule;


	/** Base class for module identification data
	 *	Application writer should derive from this
	 *	class to create there own security information.
	 *	Security information are bound to proxy data
	 *	by a secured gateway.
	 */
	struct TSecurityData
	{
		// for factory system
		struct TCtorParam
		{
			uint8	DataTag;

			TCtorParam(uint8 dataTag)
				:	DataTag(dataTag)
			{
			}
		};
		/// An application defined identifier
		uint8			DataTag;
	
		/// Pointer to next security data item (or NULL)
		TSecurityData	*NextItem;

		TSecurityData(const TCtorParam &params)
			: DataTag(params.DataTag),
			NextItem(NULL)
			
		{
		}

		virtual ~TSecurityData()
		{
			if (NextItem != NULL)
				delete NextItem;
		}

		virtual void serial(NLMISC::CMemStream &s) =0;

	};

	struct TUnknownSecurityData : public TSecurityData
	{
		uint8				RealDataTag;
		std::vector<uint8>	Data;

		TUnknownSecurityData(uint8 realDataTag, uint32 size) 
			: TSecurityData(TSecurityData::TCtorParam(0xff)),
			RealDataTag(realDataTag),
			Data(size)
		{
		}
		
		void serial(NLMISC::CMemStream &s)
		{
			for (uint i=0; i<Data.size(); ++i)
			{
				s.serial(Data[i]);
			}
		}
	};

//	NLMISC_REGISTER_OBJECT(TSecurityData, TUnknownSecurityData, uint8, 0xff);

	/** This interface is implemented by the system 
	 *	and it give convenient access to distant module information
	 *	like module name or id,
	 *	it also provide a helper to send module message
	 *	without knowing the gateway.
	 *	Note that even collocated module (i.e. module created in the 
	 *	same process) must be accessed by module proxy or message 
	 *	send by through the socket interface.
	 */
	class IModuleProxy : public NLMISC::CRefCount
	{
	public:
		virtual ~IModuleProxy() {}

		/** Return the module ID. Each module has a local unique ID assigned 
		 *	by the manager during module creation.
		 *	This ID is local because it is only valid inside a given process.
		 *	When module are declared in another process, they receive a 
		 *	local ID that is different than the ID in their host process.
		 */
		virtual TModuleId		getModuleProxyId() const =0;

		/** Return the foreign id of the proxy.
		 *	This is the proxy id of this module in the
		 *	context at the outbound of the route.
		 *	Note that if the module distance id 0, then
		 *	this id is the module id and if the	distance
		 *	is 1, then this is the id of the local
		 *	proxy for the module in his host process.
		 *
		 *	Maybe you don't exactly understand the meaning of this
		 *	id, what is sure is that you should never use it !
		 */
		virtual TModuleId		getForeignModuleId() const =0;

		/**	Return the distance of this module in number of
		 *	gateway to cross.
		 *	Note that when a module is reachable via more than
		 *	one route, the gateway always use the shortest path
		 *	to communicate with the module.
		 */
		virtual uint32				getModuleDistance() const =0;

		/** Return a pointer to the route used to communicate
		 *	with the module.
		 *	For local module proxy, the return value is NULL
		 */
		virtual CGatewayRoute		*getGatewayRoute() const = 0;

		/** Return the module name. Each module instance must have a unique 
		 *	name in the host process.
		 *	If no mane is given during module creation, the module manager
		 *	build a unique name from the module class and a number.
		 *	Distant module name are always the FQMN, ie, it is the same as 
		 *	getModuleFullyQualifiedName()
		 */
		virtual const std::string &getModuleName() const =0;
		/** Return the module class.
		 *	For module proxy, this is always the fully qualified module name
		 */
		virtual const std::string &getModuleClassName() const =0;
		/** Return the manifest of the module.
		 *	The manifest is a simple string of undefined format.
		 *	The manifest is used by a module to expose some intention
		 *	or affinity (or whaterver else yuou could imagine) of the
		 *	module.
		 */
		virtual const std::string &getModuleManifest() const =0;

		/** Return the gateways interface by witch this module is accessible
		 *	In some case, more than one gateway can be returned when there
		 *	is multiple route to the same module.
		 */
		virtual IModuleGateway *getModuleGateway() const =0;

		/** Send a message to the module.
		 *	This method do the job of finding a valid socket to effectively send
		 *	the message.
		 */
		virtual void		sendModuleMessage(IModule *senderModule, const NLNET::CMessage &message)
			throw (EModuleNotReachable)
			=0;

		/** Return the first item of the security item list
		 *	If no security data are available, the method
		 *	return NULL.
		 */
		virtual const TSecurityData *getFirstSecurityData() const = 0;

		/** Look in the security data list for a block
		 *	matching the specified tag.
		 *	The first block having the tag is returned.
		 *	If no block have the requested tag, NULL is returned.
		 */
		virtual const TSecurityData *findSecurityData(uint8 dataTag) const =0;
	};

	const TModuleProxyPtr	NullModuleProxy;

	/// Base class for module task (aka module coroutine)
	class CModuleTask : public NLMISC::CCoTask
	{
		bool	_FailInvoke;
	protected:
		///only derived class can use it
		CModuleTask ()
			: _FailInvoke(false)
		{
		}

		void initMessageQueue(class CModuleBase *module);

		void flushMessageQueue(class CModuleBase *module);

	public:
		// for task blocked in 'invoke' to fail with an exception
		void failInvoke()
		{
			_FailInvoke = true;
		}

		bool mustFailInvoke()
		{
			return _FailInvoke;
		}
	};

	/// Template module task
	template <class T>
	class TModuleTask : public CModuleTask
	{
	public:
		typedef void (T::*TMethodPtr)();

	private:
		T			*_Module;
		TMethodPtr	_TaskMethod;

		// override CTask::run
		void run()
		{
			initMessageQueue(_Module);

			// run the module task command control to module task method
			(_Module->*_TaskMethod)();

			// finish the dispatch
			flushMessageQueue(_Module);
		}
	public:

		TModuleTask(T *module,  void (T::*taskMethod)())
			: _Module(module),
			_TaskMethod(taskMethod)
		{
		}
	};

	// a special macro for easy module task startup
#define	NLNET_START_MODULE_TASK(className, methodName) \
	{ \
		TModuleTask<className> *task = new TModuleTask<className>(this, className::methodName); \
		queueModuleTask(task); \
	} \

	/** Interface for module factory.
	 *	Each module MUST provide a factory and 
	 *	register an instance of the factory in
	 *	the module manager.
	 */
	class IModuleFactory : public NLMISC::CRefCount
	{
	protected:
		/// The class name of the factored module
		std::string				_ModuleClassName;
		/// The list of instantiated modules
		std::set<TModulePtr>	_ModuleInstances;
	public:


		/// Constructor, initialize the module class name
		IModuleFactory(const std::string &moduleClassName);

		/** Return the class name of the factored module */
		virtual const std::string &getModuleClassName() const;
		/** Return the initialization string helper */
		virtual const std::string &getInitStringHelp() const =0;

		/** Pretty simple method. Module initialization
		 *	is done after construction, so there are
		 *	no parameter at construction.
		 */
		virtual IModule *createModule() =0;

		/** The module manager call this to delete a module instance.*/
		virtual void	deleteModule(IModule *module);

		/** Virtual destructor.
		 *	The destructor while unregister the module factory from the
		 *	factory registry and ALL module factored
		 *	will also be deleted.
		 */	
		virtual ~IModuleFactory();

		/** Called by concrete factory to initialise the factored object */
		void registerModuleInFactory(TModulePtr module);
	};

//	const TModuleFactoryPtr	NullModuleFactory;

	template <class moduleClass>
	class CModuleFactory : public IModuleFactory
	{
	public:
		CModuleFactory(const std::string &moduleClassName)
			: IModuleFactory(moduleClassName)
		{}

		virtual IModule *createModule()
		{
			IModule *module = new moduleClass;
			registerModuleInFactory(module);
			return module;
		}

		virtual const std::string &getInitStringHelp() const
		{
			return moduleClass::getInitStringHelp();
		}

	};

#define NLNET_REGISTER_MODULE_FACTORY(moduleClassName, registrationName) \
	class moduleClassName##Factory : public NLNET::CModuleFactory<moduleClassName> \
	{ \
	public: \
		static const std::string &theModuleClassName() \
		{ \
			static const std::string name(registrationName); \
			return name; \
		} \
		\
		moduleClassName##Factory() \
			: NLNET::CModuleFactory<moduleClassName>(theModuleClassName()) \
		{} \
	};\
	NLMISC_REGISTER_OBJECT_INDIRECT(NLNET::IModuleFactory, moduleClassName##Factory, std::string, registrationName)


//#define NLNET_MAKE_MODULE_FACTORY_TYPENAME(moduleClassName) moduleClassName##Factory

#define NLNET_GET_MODULE_FACTORY(moduleClassName)	moduleClassName##Factory

	class CModuleSocket;

	/** Basic module implementation.
	 *	Module implementor should derive from this class
	 *	rather than rebuild a complete module from
	 *	scratch (from IModule in fact).
	 *	This class provide name and module registration,
	 *	message dispatching to message handler,
	 *	module socket interaction.
	 */
	class CModuleBase : public IModule, public NLMISC::ICommandsHandler
	{
		// Module manager is our friend coz it need to feed some field here
		friend class CModuleManager;
		friend class CModuleTask;

		typedef std::set<IModuleSocket *> 	TModuleSockets;
		/// This is the sockets where the module is plugged in
		TModuleSockets			_ModuleSockets;

		//@{
		//@name Synchronous messaging
		typedef std::list<std::pair<IModuleProxy *, CMessage> >	TMessageList;
		/// dynamically allocated list of synchronous message to process
		TMessageList			_SyncMessages;
		typedef std::list<CModuleTask*> TModuleTasks;
		/// list of coroutine to run for synchronous messaging (the first in the list is running)
		TModuleTasks			_ModuleTasks;

		typedef std::vector<IModuleProxy*>	TInvokeStack;
		/// stack of server module with pending invocation response
		TInvokeStack			_InvokeStack;

		/// The current message to process sender
		IModuleProxy			*_CurrentSender;
		/// The current message to process
		const NLNET::CMessage	*_CurrentMessage;
		/// True if the current message processing have generated an exception
		bool					_CurrentMessageFailed;

		/// task for message dispatching
		CModuleTask				*_MessageDispatchTask;
		//@}

		virtual void setFactory(IModuleFactory *factory);
		virtual IModuleFactory *getFactory();
	protected:
		/// Keep track of the module factory
		IModuleFactory		*_ModuleFactory;
		/// This is the local unique ID assigned to this module.
		TModuleId			_ModuleId;
		/// This is the module name.
		std::string			_ModuleName;
		/// This is the fully qualified module name
		mutable std::string	_FullyQualifedModuleName;


		CModuleBase();
		~CModuleBase();

		const std::string	&getCommandHandlerName() const;
		TModuleId			getModuleId() const;
		const std::string	&getModuleName() const;

		const std::string	&getModuleClassName() const;

		const std::string	&getModuleFullyQualifiedName() const;

		const std::string	&getModuleManifest() const;

		/** Called by a socket to receive a message in the module context.
		 *	Basic implementation either forward directly to onProcessModuleMessage
		 *	or queue the message in the coroutine message queue (when a synchronous
		 *	messaging coroutine is started) for later dispatching.
		 */
		virtual void		onReceiveModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message);

		/// The message dispatching task
		void _receiveModuleMessageTask();

		void queueModuleTask(CModuleTask *task);

	public:
		// return the default init string (empty)
		static const std::string &CModuleBase::getInitStringHelp();
	protected:
		// Init base module, init module name
		void				initModule(const TParsedCommandLine &initInfo);
		
		void				plugModule(IModuleSocket *moduleSocket) throw (EModuleAlreadyPluggedHere);
		void				unplugModule(IModuleSocket *moduleSocket)  throw (EModuleNotPluggedHere);
		void				getPluggedSocketList(std::vector<IModuleSocket*> &resultList);
		void				invokeModuleOperation(IModuleProxy *destModule, const NLNET::CMessage &opMsg, NLNET::CMessage &resultMsg) throw (EInvokeFailed);
		void				_onModuleDown(IModuleProxy *removedProxy);



		/// base module command table
		NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CModuleBase, dump, "display informations about module instance status", "no args")
			NLMISC_COMMAND_HANDLER_ADD(CModuleBase, plug, "plug the module in a module socket", "<socket_name>")
			NLMISC_COMMAND_HANDLER_ADD(CModuleBase, unplug, "unplug the module out of a module socket", "<socket_name>")
			NLMISC_COMMAND_HANDLER_ADD(CModuleBase, sendPing, "send a ping message to another module using the first available route", "<addresseeModuleName>")
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(dump);
		NLMISC_CLASS_COMMAND_DECL(plug);
		NLMISC_CLASS_COMMAND_DECL(unplug);
		NLMISC_CLASS_COMMAND_DECL(sendPing);
	};

	class CGatewayRoute;

	class CModuleProxy : public IModuleProxy
	{
		friend class CModuleManager;
		friend class CStandardGateway;

		/// The gateway that received the module informations
		IModuleGateway		*_Gateway;
		/// The route to use for reaching the module
		CGatewayRoute		*_Route;
		/// The module distance (in term of gateway to cross)
		uint32				_Distance;
		/// The module local ID
		TModuleId			_ModuleProxyId;
		/// The module foreign ID, this is the module ID in case of a local proxy
		TModuleId			_ForeignModuleId;

		/// The module class name;
		NLMISC::TStringId	_ModuleClassName;
		/// The  fully qualified module name.
		NLMISC::TStringId	_FullyQualifiedModuleName;
		/// The module manifest
		std::string			_Manifest;
		/// the list of security data
		TSecurityData		*_SecurityData;

		/// Private constructor, only manager instantiate module proxies
		CModuleProxy(TModuleId localModuleId, const std::string &moduleClassName, const std::string &fullyQualifiedModuleName, const std::string &moduleManifest);
	public:

		const CGatewayRoute * const getRoute() const
		{
			return _Route;
		}

		/** Return the module ID. Each module has a local unique ID assigned 
		 *	by the manager during module creation.
		 *	This ID is local because it is only valid inside a given process.
		 *	When module are declared in another process, they receive a 
		 *	local ID that is different than the ID in their host process.
		 */
		virtual TModuleId	getModuleProxyId() const;

		virtual TModuleId	getForeignModuleId() const;


		uint32				getModuleDistance() const;
		
		CGatewayRoute		*getGatewayRoute() const;

		/** Return the module name. Each module instance must have a unique 
		 *	name in the host process.
		 *	If no mane is given during module creation, the module manager
		 *	build a unique name from the module class and a number.
		 *	Distant module name are always the FQMN, ie, it is the same as 
		 *	getModuleFullyQualifiedName()
		 */
		virtual const std::string &getModuleName() const;
		/// Return the module class.
		virtual const std::string &getModuleClassName() const;
		/// return the module manifest
		virtual const std::string &getModuleManifest() const;

		/** Return the gateways interface by witch this module is accessible.
		 */
		virtual IModuleGateway *getModuleGateway() const;

		/** Send a message to the module.
		 */
		virtual void		sendModuleMessage(IModule *senderModule, const NLNET::CMessage &message)
			throw (EModuleNotReachable);
		
		virtual const TSecurityData *getFirstSecurityData() const 
		{
			return _SecurityData;
		}

		virtual const TSecurityData *findSecurityData(uint8 dataTag) const;


	};



	/** Utility class to do broadcast with a container of proxy pointer
	 */
	template <class PtrContainer>
	class TBroadcastModuleMessage
	{

		void sendMessage(IModuleProxy *senderProxy, const PtrContainer &addresseeProxies, NLNET::CMessage &message)
		{
			typename PtrContainer::iterator first(addresseeProxies.begin()), last(addresseeProxies.end());
			for (; first != last; ++first)
			{
				CModuleProxy *proxy = static_cast<CModuleProxy*>(*first);
				
				// copy the message before each send
				CMessage msgCopy(message);
				
				proxy->sendModuleMessage(senderProxy, msgCopy);
			}
		}
	};


} // namespace NLNET

#endif // NL_FILE_MODULE_H
