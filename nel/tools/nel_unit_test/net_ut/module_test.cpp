
#include "nel/misc/dynloadlib.h"
#include "nel/misc/command.h"
#include "nel/net/module_common.h"
#include "nel/net/module_manager.h"
#include "nel/net/module.h"
#include "nel/net/inet_address.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_gateway.h"


#include "src/cpptest.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

class CModuleType0 : public CModuleBase
{
public:

	void				onServiceUp(const std::string &serviceName, uint16 serviceId)
	{
	}
	/// A nel layer 5 service has stopped.
	void				onServiceDown(const std::string &serviceName, uint16 serviceId) 
	{
	}
	void				onModuleUpdate()
	{
	}
	/** The service main loop is terminating it job', all module while be
	 *	disconnected and removed after this callback.
	 */
	void				onApplicationExit() 
	{
	}

	void				onModuleUp(IModuleProxy *moduleProxy)
	{
	}
	/** Called by a socket to inform this module that another
	 *	module has been deleted OR has been no more accessible (due to
	 *	some gateway disconnection).
	 */
	void				onModuleDown(IModuleProxy *moduleProxy)
	{
	}

	void				onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
	{
	}

	void	onModuleSocketEvent(IModuleSocket *moduleSocket, IModule::TModuleSocketEvent eventType)
	{
	}
};

NLNET_REGISTER_MODULE_FACTORY(CModuleType0, "ModuleType0");

//NLNET_DECLARE_MODULE_FACTORY(CModuleType0);
//typedef class CModuleType0Factory	TFactoryType;
//NLMISC_REGISTER_OBJECT_INDIRECT(IModuleFactory, TFactoryType, std::string, std::string("ModuleType0"));

// Test suite for Modules class
class CModuleTS : public Test::Suite
{
public:
	CModuleTS ()
	{
		TEST_ADD(CModuleTS::testModuleInitInfoParsing);
		TEST_ADD(CModuleTS::testModuleInitInfoQuering);
		TEST_ADD(CModuleTS::testModuleInitInfoBadParsing);
		TEST_ADD(CModuleTS::localModuleFactory);
		TEST_ADD(CModuleTS::loadModuleLib);
		TEST_ADD(CModuleTS::createModule);
		TEST_ADD(CModuleTS::deleteModule);
		TEST_ADD(CModuleTS::unloadModuleLib);
		TEST_ADD(CModuleTS::createLocalGateway);
		TEST_ADD(CModuleTS::plugLocalGateway);
		TEST_ADD(CModuleTS::moduleManagerCommands);
		TEST_ADD(CModuleTS::gatewayTransportManagement);
		TEST_ADD(CModuleTS::connectGateways);
		TEST_ADD(CModuleTS::moduleDisclosure);
		TEST_ADD(CModuleTS::moduleMessaging);
	}

	void moduleMessaging()
	{
		IModuleManager &mm = IModuleManager::getInstance();
		CCommandRegistry &cr = CCommandRegistry::getInstance();

		// create two gateway an connect them, plug the gateway on themselves and send a message
		IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		IModuleGateway *gws = dynamic_cast<IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);

		// plug the module in itself before opening connection
		IModuleSocket *socketGws = mm.getModuleSocket("gws");
		TEST_ASSERT(socketGws != NULL);
		mods->plugModule(socketGws);

		// add transport for server mode
		string cmd = "gws.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));
		cmd = "gws.transportCmd l3s(open port=6185)";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));

		IModule *modc = mm.createModule("StandardGateway", "gwc", "");
		TEST_ASSERT(modc != NULL);
		IModuleGateway *gwc = dynamic_cast<IModuleGateway*>(modc);
		TEST_ASSERT(gwc != NULL);
		// add transport for client mode
		cmd = "gwc.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));
		cmd = "gwc.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));

		// plug the module in itself before opening connection
		IModuleSocket *socketGwc = mm.getModuleSocket("gwc");
		TEST_ASSERT(socketGwc != NULL);
		modc->plugModule(socketGwc);

		// update the gateways...
		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}

		// send a message from gws to gwc using the proxy
		// First, get the proxy for the client (must be the second one)
		vector<IModuleProxy*>	proxiesS;
		gws->getModuleProxyList(proxiesS);
		TEST_ASSERT(proxiesS.size() == 2 && proxiesS[1]->getModuleName().find("gwc") != string::npos);
		CMessage aMessage("GW_PING");
		proxiesS[1]->sendModuleMessage(mods, aMessage);

		// update the gateways...
		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}

		// check that the ping has been received
		TEST_ASSERT(gwc->getReceivedPingCount() == 1);

		// send two crossing message simultaneously
		vector<IModuleProxy*>	proxiesC;
		gwc->getModuleProxyList(proxiesC);
		TEST_ASSERT(proxiesC.size() == 2 && proxiesC[1]->getModuleName().find("gws") != string::npos);
		proxiesS[1]->sendModuleMessage(mods, aMessage);
		proxiesC[1]->sendModuleMessage(modc, aMessage);

		// update the gateways...
		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}
		// check that the ping has been received
		TEST_ASSERT(gwc->getReceivedPingCount() == 2);
		TEST_ASSERT(gws->getReceivedPingCount() == 1);
		

		// send with ISocket
		socketGws->sendModuleMessage(mods, proxiesS[1]->getModuleProxyId(), aMessage);
		// update the gateways...
		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}
		// check that the ping has been received
		TEST_ASSERT(gwc->getReceivedPingCount() == 3);
		TEST_ASSERT(gws->getReceivedPingCount() == 1);

		// cleanup modules
		mm.deleteModule(mods);
		TEST_ASSERT(mm.getLocalModule("gws") == NULL);
		mm.deleteModule(modc);
		TEST_ASSERT(mm.getLocalModule("gwc") == NULL);
	}

	void moduleDisclosure()
	{
		IModuleManager &mm = IModuleManager::getInstance();
		CCommandRegistry &cr = CCommandRegistry::getInstance();

		IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		IModuleGateway *gws = dynamic_cast<IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);

		TEST_ASSERT(gws->getProxyCount() == 0);

		// plug the module in itself before opening connection
		IModuleSocket *socketGws = mm.getModuleSocket("gws");
		TEST_ASSERT(socketGws != NULL);
		mods->plugModule(socketGws);

		// now, there must be one proxy in the gateway
		TEST_ASSERT(gws->getProxyCount() == 1);
		vector<IModuleProxy*>	proxies;
		gws->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[0]->getForeignModuleId() == mods->getModuleId());

		// add transport for server mode
		string cmd = "gws.transportAdd L3Server l3s";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));
		cmd = "gws.transportCmd l3s(open port=6185)";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));

		IModule *modc = mm.createModule("StandardGateway", "gwc", "");
		TEST_ASSERT(modc != NULL);
		IModuleGateway *gwc = dynamic_cast<IModuleGateway*>(modc);
		TEST_ASSERT(gwc != NULL);
		// add transport for client mode
		cmd = "gwc.transportAdd L3Client l3c";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));
		cmd = "gwc.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));

		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}

		// The server must have not changed
		TEST_ASSERT(gws->getProxyCount() == 1);

		// The client must have one proxy
		TEST_ASSERT(gwc->getProxyCount() == 1);
		proxies.clear();
		gwc->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[0]->getModuleName().find("gws") == proxies[0]->getModuleName().size() - 3);

		// plug the client module in itself after opening connection
		IModuleSocket *socketGwc = mm.getModuleSocket("gwc");
		TEST_ASSERT(socketGwc != NULL);
		modc->plugModule(socketGwc);


		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}

		// The server must have now the two modules
		TEST_ASSERT(gws->getProxyCount() == 2);
		proxies.clear();
		gws->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 2);
		TEST_ASSERT(proxies[0]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[0]->getForeignModuleId() == mods->getModuleId());
		TEST_ASSERT(proxies[1]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[1]->getModuleName().find("gwc") == proxies[1]->getModuleName().size() - 3);

		// The client must have two module also
		TEST_ASSERT(gwc->getProxyCount() == 2);
		proxies.clear();
		gwc->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 2);
		TEST_ASSERT(proxies[0]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[0]->getModuleName().find("gws") == proxies[1]->getModuleName().size() - 3);
		TEST_ASSERT(proxies[1]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[1]->getForeignModuleId() == modc->getModuleId());


		// unplug the client module in itself after opening connection
		mods->unplugModule(socketGws);

		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}

		// The server must have one module left
		TEST_ASSERT(gws->getProxyCount() == 1);
		proxies.clear();
		gws->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() != NULL);
		TEST_ASSERT(proxies[0]->getModuleName().find("gwc") == proxies[0]->getModuleName().size() - 3);

		// The client must have one module left
		TEST_ASSERT(gwc->getProxyCount() == 1);
		proxies.clear();
		gwc->getModuleProxyList(proxies);
		TEST_ASSERT(proxies.size() == 1);
		TEST_ASSERT(proxies[0]->getGatewayRoute() == NULL);
		TEST_ASSERT(proxies[0]->getForeignModuleId() == modc->getModuleId());
		
		// Dump the module state
		cmd = "gws.dump";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));
		cmd = "gwc.dump";
		TEST_ASSERT(cr.execute(cmd, InfoLog()));

		// cleanup modules
		mm.deleteModule(mods);
		TEST_ASSERT(mm.getLocalModule("gws") == NULL);
		mm.deleteModule(modc);
		TEST_ASSERT(mm.getLocalModule("gwc") == NULL);
	}

	void connectGateways()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		IModule *mods = mm.createModule("StandardGateway", "gws", "");
		TEST_ASSERT(mods != NULL);
		IModuleGateway *gws = dynamic_cast<IModuleGateway*>(mods);
		TEST_ASSERT(gws != NULL);
		// add transport for server mode
		string cmd = "gws.transportAdd L3Server l3s";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		cmd = "gws.transportCmd l3s(open port=6185)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
			
		IModule *modc1 = mm.createModule("StandardGateway", "gwc1", "");
		TEST_ASSERT(modc1 != NULL);
		IModuleGateway *gwc1 = dynamic_cast<IModuleGateway*>(modc1);
		TEST_ASSERT(gwc1 != NULL);
		// add transport for client mode
		cmd = "gwc1.transportAdd L3Client l3c";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		cmd = "gwc1.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		for (uint i=0; i<3; ++i)
		{
			nlSleep(100);
			mm.updateModules();
			nlSleep(100);
		}

		TEST_ASSERT(gws->getRouteCount() == 1);
		TEST_ASSERT(gwc1->getRouteCount() == 1);
			
		// do a second connect to the server for stress
		// add transport for client mode
		cmd = "gwc1.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// create third gateway
		IModule *modc2 = mm.createModule("StandardGateway", "gwc2", "");
		TEST_ASSERT(modc2 != NULL);
		IModuleGateway *gwc2 = dynamic_cast<IModuleGateway*>(modc2);
		TEST_ASSERT(gwc2 != NULL);
		// add transport for client mode
		cmd = "gwc2.transportAdd L3Client l3c";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		cmd = "gwc2.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// update the module to update the network callback client and server
		for (uint i=0; i<10; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			nlSleep(100);
			mm.updateModules();
		}

		TEST_ASSERT(gws->getRouteCount() == 3);
		TEST_ASSERT(gwc1->getRouteCount() == 2);
		TEST_ASSERT(gwc2->getRouteCount() == 1);

		// dump the gateways state
		cmd = "gws.dump";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		cmd = "gwc1.dump";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		cmd = "gwc2.dump";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// cleanup the modules
		mm.deleteModule(mods);
		TEST_ASSERT(mm.getLocalModule("gws") == NULL);
		mm.deleteModule(modc1);
		TEST_ASSERT(mm.getLocalModule("gwc1") == NULL);
		mm.deleteModule(modc2);
		TEST_ASSERT(mm.getLocalModule("gwc2") == NULL);
	}

	void gatewayTransportManagement()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		// create a gateway module
		IModule *mod = mm.createModule("StandardGateway", "gw", "");
		TEST_ASSERT(mod != NULL);
		IModuleGateway *gw = dynamic_cast<IModuleGateway*>(mod);
		TEST_ASSERT(gw != NULL);

		// Create a layer 3 server transport
		// send a transport creation command
		string cmd = "gw.transportAdd L3Server l3s";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		IGatewayTransport *transportL3s = gw->getGatewayTransport("l3s");
		TEST_ASSERT(transportL3s != NULL);

		// send a transport command
		cmd = "gw.transportCmd l3s(open port=6185)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// Create a layer 3 client transport
		// send a transport creation command
		cmd = "gw.transportAdd L3Client l3c";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		IGatewayTransport *transportL3c = gw->getGatewayTransport("l3c");
		TEST_ASSERT(transportL3c != NULL);

		// send a transport command
		cmd = "gw.transportCmd l3c(connect addr=localhost:6185)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// update the module to update the network callback client and server
		for (uint i=0; i<3; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			nlSleep(100);
			mm.updateModules();
		}

		TEST_ASSERT(transportL3s->getRouteCount() == 1);	
		TEST_ASSERT(transportL3c->getRouteCount() == 1);
		TEST_ASSERT(gw->getRouteCount() == 2);
		
		// dump the gateways state
		cmd = "gw.dump";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		
		// close all connections
		cmd = "gw.transportCmd l3s(close)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		
		cmd = "gw.transportCmd l3c(close connId=0)";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// update the module to update the network callback client and server
		for (uint i=0; i<3; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			nlSleep(100);
			mm.updateModules();
		}

		TEST_ASSERT(transportL3s->getRouteCount() == 0);
		TEST_ASSERT(transportL3c->getRouteCount() == 0);
		TEST_ASSERT(gw->getRouteCount() == 0);

		// Remove transports
		cmd = "gw.transportRemove l3s";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
		cmd = "gw.transportRemove l3c";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		TEST_ASSERT(gw->getGatewayTransport("l3c") == NULL);
		TEST_ASSERT(gw->getGatewayTransport("l3s") == NULL);
		TEST_ASSERT(gw->getTransportCount() == 0);

		// update the module to update the network callback client and server
		for (uint i=0; i<3; ++i)
		{
			// give some time to the listen and receiver thread to do there jobs
			nlSleep(100);
			mm.updateModules();
		}

		// cleanup the modules
		mm.deleteModule(mod);
		TEST_ASSERT(mm.getLocalModule("gw") == NULL);
	}

	void moduleManagerCommands()
	{
		string cmd;
		// load a library
		cmd = "moduleManager.loadLibrary net_ut/net_module_lib_test/net_module_lib_test";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// dump the module state
		cmd = "moduleManager.dump";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// create a module
		cmd = "moduleManager.createModule ModuleType1 AModuleName";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// dump the module state
		cmd = "moduleManager.dump";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// delete the module
		cmd = "moduleManager.deleteModule AModuleName";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));
	}

	void plugLocalGateway()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		IModule *gateway1 = mm.createModule("LocalGateway", "g1", "");
		TEST_ASSERT(gateway1 != NULL);
		IModule *gateway2 = mm.createModule("LocalGateway", "g2", "");
		TEST_ASSERT(gateway2 != NULL);

		IModuleSocket *socket1 = mm.getModuleSocket("g1");
		TEST_ASSERT(socket1 != NULL);
		IModuleSocket *socket2 = mm.getModuleSocket("g2");
		TEST_ASSERT(socket2 != NULL);
		gateway1->plugModule(socket1);
		gateway1->plugModule(socket2);
		gateway2->plugModule(socket1);
		gateway2->plugModule(socket2);

		mm.deleteModule(gateway1);
		TEST_ASSERT(mm.getLocalModule("g1") == NULL);
		mm.deleteModule(gateway2);
		TEST_ASSERT(mm.getLocalModule("g2") == NULL);
	}

	void createLocalGateway()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		IModule *gateway = mm.createModule("LocalGateway", "localGateway", "");
		TEST_ASSERT(gateway != NULL);

		IModule *mod1 = mm.createModule("ModuleType0", "plugged1", "");
		TEST_ASSERT(mod1 != NULL);
		IModule *mod2 = mm.createModule("ModuleType0", "plugged2", "");
		TEST_ASSERT(mod2 != NULL);

		IModuleSocket *socket = mm.getModuleSocket("localGateway");
		TEST_ASSERT(socket != NULL);
		mod1->plugModule(socket);
		mod2->plugModule(socket);

		mm.deleteModule(mod1);
		TEST_ASSERT(mm.getLocalModule("plugged1") == NULL);
		mm.deleteModule(mod2);
		TEST_ASSERT(mm.getLocalModule("plugged2") == NULL);

		mm.deleteModule(gateway);
		TEST_ASSERT(mm.getLocalModule("localGateway") == NULL);
	}

	void unloadModuleLib()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		CRefPtr<IModule> module1 = mm.createModule("ModuleType1", "TheModule2", "the args");
		TEST_ASSERT(module1 != NULL);

		TEST_ASSERT(mm.unloadModuleLibrary("net_module_lib_test"));

		// the module must have been deleted
		TEST_ASSERT(module1 == NULL);

		TModulePtr module2 = mm.createModule("ModuleType1", "TheModuleThatCantBeCreated", "the args");
		TEST_ASSERT(module2 == NULL);
	}

	void deleteModule()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		IModule *module = mm.createModule("ModuleType1", "TheModuleToDelete", "the args");
		TEST_ASSERT(module != NULL);

		CRefPtr<IModule> checkPtr(module);

		mm.deleteModule(module);
		TEST_ASSERT(checkPtr == NULL);
	}

	void createModule()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		TModulePtr module = mm.createModule("ModuleType1", "TheModule", "the args");
		TEST_ASSERT(module != NULL);

		TEST_ASSERT(module->getModuleClassName() == "ModuleType1");
		TEST_ASSERT(module->getModuleName() == "TheModule");

		string lh = NLNET::CInetAddress::localHost().hostName();
		string fqmn = lh+":"+toString(getpid())+":TheModule";

		TEST_ASSERT(module->getModuleFullyQualifiedName() == fqmn);
	}

	void loadModuleLib()
	{
		string moduleLibName = "net_ut/net_module_lib_test/net_module_lib_test";

		IModuleManager &mm = IModuleManager::getInstance();
		TEST_ASSERT(mm.loadModuleLibrary(moduleLibName));

		vector<string>	moduleList;
		mm.getAvailableModuleList(moduleList);

		TEST_ASSERT(moduleList.size() == 4);
		TEST_ASSERT(moduleList[0] == "LocalGateway");
		TEST_ASSERT(moduleList[1] == "ModuleType0");
		TEST_ASSERT(moduleList[2] == "ModuleType1");
		TEST_ASSERT(moduleList[3] == "ModuleType2");
	}

	void localModuleFactory()
	{
		IModuleManager &mm = IModuleManager::getInstance();

		vector<string>	moduleList;
		mm.getAvailableModuleList(moduleList);

		TEST_ASSERT(moduleList.size() == 2);
		TEST_ASSERT(moduleList[0] == "LocalGateway");
		TEST_ASSERT(moduleList[1] == "ModuleType0");
	}

	void testModuleInitInfoBadParsing()
	{
		TParsedCommandLine	mif;

		string	paramString = " a=1   b=2   ( b=1) "; 
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = " lswkd ,fpqoj(( çruq fzemfwijf ujr wmozejifp_zujf woijpç_u ' "; 
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a ( b=2"; 
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a  b=2)"; 
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a  b=2\"toto\"";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "=a";
		TEST_ASSERT(!mif.parseParamList(paramString));

		paramString = "a(=b)";
		TEST_ASSERT(!mif.parseParamList(paramString));
	}

	void testModuleInitInfoQuering()
	{
		TParsedCommandLine	mif;

		string	paramString = " a=1   b=2   sub   ( y=22 zzzz=12 subsub (g=\"bean in box\" z=2) ) "; 

		TEST_ASSERT(mif.parseParamList(paramString));

		TEST_ASSERT(mif.getParam("a") != NULL);
		TEST_ASSERT(mif.getParam("a") == &mif.SubParams[0]);

		TEST_ASSERT(mif.getParam("sub") != NULL);
		TEST_ASSERT(mif.getParam("sub") == &mif.SubParams[2]);

		TEST_ASSERT(mif.getParam("foo") == NULL);

		TEST_ASSERT(mif.getParam("sub.subsub.g") != NULL);
		TEST_ASSERT(mif.getParam("sub.subsub.g") == &mif.SubParams[2].SubParams[2].SubParams[0]);
	}

	void testModuleInitInfoParsing()
	{
		TParsedCommandLine	mif;

		string	paramString = "a"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		paramString = "a=1"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		paramString = "a(b=1)"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		paramString = "a a a a"; 
		TEST_ASSERT(mif.parseParamList(paramString));
		TEST_ASSERT(mif.SubParams.size() == 4);
		paramString = " a ( b=1 )"; 
		TEST_ASSERT(mif.parseParamList(paramString));

		paramString = " a=1   b=2   sub   ( y=22 zzzz=12 subsub (g=\"bean in box\" z=2) ) "; 

		TEST_ASSERT(mif.parseParamList(paramString));

		TEST_ASSERT(mif.SubParams.size() == 3);

		TEST_ASSERT(mif.SubParams[0].SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[0].ParamName == "a");
		TEST_ASSERT(mif.SubParams[0].ParamValue == "1");

		TEST_ASSERT(mif.SubParams[1].SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[1].ParamName == "b");
		TEST_ASSERT(mif.SubParams[1].ParamValue == "2");

		TEST_ASSERT(mif.SubParams[2].SubParams.size() == 3);
		TEST_ASSERT(mif.SubParams[2].ParamName == "sub");
		TEST_ASSERT(mif.SubParams[2].ParamValue.empty());

		TEST_ASSERT(mif.SubParams[2].SubParams[0].SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2].SubParams[0].ParamName == "y");
		TEST_ASSERT(mif.SubParams[2].SubParams[0].ParamValue == "22");
	
		TEST_ASSERT(mif.SubParams[2].SubParams[1].SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2].SubParams[1].ParamName == "zzzz");
		TEST_ASSERT(mif.SubParams[2].SubParams[1].ParamValue == "12");
	
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams.size() == 2);
		TEST_ASSERT(mif.SubParams[2].SubParams[2].ParamName == "subsub");
		TEST_ASSERT(mif.SubParams[2].SubParams[2].ParamValue.empty());
	
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams[0].SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams[0].ParamName == "g");
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams[0].ParamValue == "bean in box");
	
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams[1].SubParams.size() == 0);
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams[1].ParamName == "z");
		TEST_ASSERT(mif.SubParams[2].SubParams[2].SubParams[1].ParamValue == "2");
	}
};


Test::Suite *createModuleTS()
{
	return new CModuleTS;
}
