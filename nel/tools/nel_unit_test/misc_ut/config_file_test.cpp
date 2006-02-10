
#include "nel/misc/app_context.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "src/cpptest.h"

using namespace std;
using namespace NLMISC;

// Test suite for CConfigFile class
class CConfigFileTS : public Test::Suite
{
	string		_WorkingPath;
	string		_OldPath;
public:
	CConfigFileTS (const std::string &workingPath)
		: _WorkingPath(workingPath)
	{
		TEST_ADD(CConfigFileTS::configWithInclude);
	}

	void setup()
	{
		_OldPath = CPath::getCurrentPath();
		CPath::setCurrentPath(_WorkingPath.c_str());
	}

	void tear_down()
	{
		CPath::setCurrentPath(_OldPath.c_str());
	}
	
	void configWithInclude()
	{
		CConfigFile configFile;

		TEST_THROWS_NOTHING(configFile.load("cfg_with_include.cfg"));

		TEST_ASSERT(configFile.loaded());
		TEST_ASSERT(configFile.getVarPtr("CfgWithInclude") != NULL);
		TEST_ASSERT(configFile.getVar("CfgWithInclude").asString(0) == "ok");
		TEST_ASSERT(configFile.getVarPtr("IncludedCfg") != NULL);
		TEST_ASSERT(configFile.getVar("IncludedCfg").asString(0) == "ok");
	}
};

Test::Suite *createCConfigFileTS(const std::string &workingPath)
{
	return new CConfigFileTS(workingPath);
}
