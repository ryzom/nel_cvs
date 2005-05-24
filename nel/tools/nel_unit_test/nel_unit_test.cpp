
#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>

#include "src/cpptest.h"

using namespace std;
using namespace NLMISC;



enum OutputType
{
	Compiler,
	Html,
	TextTerse,
	TextVerbose
};

static void
usage()
{
	cout << "usage: mytest [MODE]\n"
		 << "where MODE may be one of:\n"
		 << "  --compiler\n"
		 << "  --html\n"
		 << "  --text-terse (default)\n"
		 << "  --text-verbose\n";
	exit(0);
}

static auto_ptr<Test::Output>
cmdline(int argc, char* argv[])
{
	if (argc > 2)
		usage(); // will not return
	
	Test::Output* output = 0;
	
	if (argc == 1)
		output = new Test::TextOutput(Test::TextOutput::Verbose);
	else
	{
		const char* arg = argv[1];
		if (strcmp(arg, "--compiler") == 0)
			output = new Test::CompilerOutput;
		else if (strcmp(arg, "--html") == 0)
			output =  new Test::HtmlOutput;
		else if (strcmp(arg, "--text-terse") == 0)
			output = new Test::TextOutput(Test::TextOutput::Terse);
		else if (strcmp(arg, "--text-verbose") == 0)
			output = new Test::TextOutput(Test::TextOutput::Verbose);
		else
		{
			cout << "invalid commandline argument: " << arg << endl;
			usage(); // will not return
		}
	}
	
	return auto_ptr<Test::Output>(output);
}


struct TLibraryInfo
{
	CLibrary	Library;
	string		WorkingPath;
};

// Main test program
//
int
main(int argc, char* argv[])
{
	std::vector<TLibraryInfo*>	testLibs;
	try
	{
		// Demonstrates the ability to use multiple test suites
		//
		Test::Suite ts;

		// load the configuration file
		CConfigFile cf;
		cf.load("test_suite.cfg");

		// insert the search path (not recursives)
		CConfigFile::CVar *pvar = cf.getVarPtr("TestDllPaths");
		if (pvar != NULL)
		{
			for (sint i=0; i<pvar->size(); ++i)
			{
				CLibrary::addLibPath(pvar->asString(i));
			}
		}

		// load the test dll
		pvar = cf.getVarPtr("TestDllList");
		if (pvar != NULL)
		{
			for (sint i=0; i<pvar->size(); i+=2)
			{
				string name = pvar->asString(i);
				string libName = CLibrary::makeLibName(name);

				string workingPath = pvar->asString(i+1);

				TLibraryInfo *pli = new TLibraryInfo;
				testLibs.push_back(pli);

				testLibs.back()->WorkingPath = workingPath;

				if (!testLibs.back()->Library.loadLibrary(libName, false, true, true))
				{
					nlwarning("Can't load test library '%s'", libName.c_str());
					delete testLibs.back();
					testLibs.pop_back();
				}
			}
		}


		// Register the test from the test libs
		for (uint i=0; i<testLibs.size(); ++i)
		{
			typedef auto_ptr<Test::Suite> (*TEntryPoint)(const std::string &workingPath);

			TEntryPoint entryPoint = NULL;
			entryPoint = *(TEntryPoint*)(testLibs[i]->Library.getSymbolAddress("registerTestSuite"));

			if (entryPoint != NULL)
			{
				// Get the test suite pointera and give it the working path
				auto_ptr<Test::Suite> ptr = entryPoint(testLibs[i]->WorkingPath);
				ts.add(ptr);
			}
		}

		// Store the current path
		string currentPath = CPath::getCurrentPath();
		// Run the tests
		//
		auto_ptr<Test::Output> output(cmdline(argc, argv));
		if (ts.run(*output, true))
		{
			// Restore the working path
			CPath::setCurrentPath(currentPath.c_str());

			Test::HtmlOutput* html = dynamic_cast<Test::HtmlOutput*>(output.get());
			if (html)
			{
				std::ofstream fout("result.html");
				html->generate(fout, true, "MyTest");
			}
		}
	}
	catch (...)
	{
		cout << "unexpected exception encountered\n";
	}
	return 0;
}

