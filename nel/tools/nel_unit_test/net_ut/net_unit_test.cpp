
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/dynloadlib.h"
#include "nel/net/sock.h"
#include "nel/net/module_manager.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "src/cpptest.h"

#include "nel/misc/path.h"

using namespace std;

Test::Suite *createModuleTS();
Test::Suite *createCMessageTS();
Test::Suite *createServiceAndModuleTS();

// global test for any misc feature
class CNetTS : public Test::Suite
{
public:
	CNetTS(const std::string &workingPath)
	{
		// init the network API
		NLNET::CSock::initNetwork();

		add(auto_ptr<Test::Suite>(createCMessageTS()));
		add(auto_ptr<Test::Suite>(createModuleTS()));
		add(auto_ptr<Test::Suite>(createServiceAndModuleTS()));
		
		// initialise the application context
		new NLMISC::CApplicationContext;
		NLNET::IModuleManager::getInstance();

		NLMISC::createDebug();
	}

	~CNetTS()
	{
		// close the network API
		NLNET::CSock::releaseNetwork();
	}
	
private:
};


auto_ptr<Test::Suite> intRegisterTestSuite(const std::string &workingPath)
{
	return static_cast<Test::Suite*>(new CNetTS(workingPath));
}

NL_LIB_EXPORT_SYMBOL(registerTestSuite, void, intRegisterTestSuite);
