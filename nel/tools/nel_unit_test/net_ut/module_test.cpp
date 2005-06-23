
#include "nel/misc/dynloadlib.h"
#include "nel/misc/command.h"
#include "nel/net/module_common.h"
#include "nel/net/module_manager.h"
#include "nel/net/module.h"
#include "nel/net/inet_address.h"
#include "nel/net/module_socket.h"


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

	void				onModuleUp(const TModuleProxyPtr &module)
	{
	}
	/** Called by a socket to inform this module that another
	 *	module has been deleted OR has been no more accessible (due to
	 *	some gateway disconnection).
	 */
	void				onModuleDown(const TModuleProxyPtr &module)
	{
	}

	void				onProcessModuleMessage(const TModuleProxyPtr &senderModule, const TModuleMessagePtr &message)
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
	}


	void moduleManagerCommands()
	{
		string cmd;
		// load a library
		cmd = "moduleManager.loadLibrary net_ut/net_module_lib_test/net_module_lib_test";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// dump the module state
		cmd = "moduleManager.dumpModuleManager ";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// create a module
		cmd = "moduleManager.createModule ModuleType1 AModuleName";
		TEST_ASSERT(CCommandRegistry::getInstance().execute(cmd, InfoLog()));

		// dump the module state
		cmd = "moduleManager.dumpModuleManager ";
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
		TModuleInitInfo	mif;

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
		TModuleInitInfo	mif;

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
		TModuleInitInfo	mif;

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
