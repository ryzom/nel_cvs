
#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/debug.h"
#include "nel/misc/sstring.h"

#include "src/cpptest.h"

using namespace std;
using namespace NLMISC;

// Test suite for stream based classes
// ! not complete at all at time of writing !
class CStreamTS: public Test::Suite
{
public:
	CStreamTS ()
	{
		TEST_ADD(CStreamTS::memStreamSwap);

	}
	
	void memStreamSwap()
	{
		CMemStream ms2;
			
		string s;
		{
			CMemStream ms1;

			s = "foo1";
			ms1.serial(s);
			s = "foo2";
			ms1.serial(s);
			s = "";

			ms2.swap(ms1);

			// check that ms1 is empty now
			TEST_ASSERT(ms1.length() == 0);
		}

		TEST_ASSERT(!ms2.isReading());
		ms2.invert();
		ms2.serial(s);
		TEST_ASSERT(s == "foo1");
		ms2.serial(s);
		TEST_ASSERT(s == "foo2");
	}
};

Test::Suite *createCStreamTS()
{
	return new CStreamTS;
}

