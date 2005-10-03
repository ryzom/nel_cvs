
#include "nel/misc/types_nl.h"
#include "nel/misc/co_task.h"
#include "nel/misc/common.h"

#include "src/cpptest.h"
#include <vector>

using namespace std;
using namespace NLMISC;

char *referenceResult[] =
{
	"Task1 : 0",
	"Task2 : 0",
	"Main : 0",

	"Task1 : 1",
	"Task2 : 1",
	"Main : 1",

	"Task1 : 2",
	"Task2 : 2",

	"Task1 : 3",
	"Task2 : 3",

	"Task1 : 4",
	"Task2 : 4",

	"Task2 : 5",

	"Task2 : 6",
};

vector<string>	result;

class CTask1 : public CCoTask
{
	void run()
	{
		for (uint i=0; i<5; ++i)
		{
			string s = toString("Task1 : %u", i);
			result.push_back(s);
			yield();
		}
	}
};

class CTask2 : public CCoTask
{
	void run()
	{
		for (uint i=0; i<7; ++i)
		{
			string s = NLMISC::toString("Task2 : %u", i);
			result.push_back(s);
			yield();
		}
	}
};

// Test suite for coroutine task
class CCoTaskTS: public Test::Suite
{
public:
	CCoTaskTS ()
	{
		TEST_ADD(CCoTaskTS::runTasks);

	}
	
	void runTasks()
	{
		result.clear();

		CTask1 t1;
		CTask2 t2;

		t1.resume();
		t2.resume();

		for (uint i=0; i<2; ++i)
		{
			string s = toString("Main : %u", i);
			result.push_back(s);
			t1.resume();
			t2.resume();
		}

		while (!t1.isFinished())
		{
			t1.resume();
			t2.resume();
		}

		t1.wait();
		t2.wait();

		TEST_ASSERT(sizeofarray(referenceResult) == result.size());
		for (uint i=0; i<sizeofarray(referenceResult); ++i)
		{
			string &s1 = result[i];
			char *s2 = referenceResult[i];
			TEST_ASSERT(referenceResult[i] == result[i]);
		}
	}
};

Test::Suite *createCCoTaskTS()
{
	return new CCoTaskTS;
}

