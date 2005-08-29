
#include "nel/net/message.h"
#include "nel/misc/debug.h"

#include "src/cpptest.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// Test suite for stream based classes
// ! not complete at all at time of writing !
class CMessageTS: public Test::Suite
{
public:
	CMessageTS ()
	{
		TEST_ADD(CMessageTS::messageSwap);
		TEST_ADD(CMessageTS::lockSubMEssage);

	}

	void lockSubMEssage()
	{
		CMessage master("BIG");

		// serial some stuff
		for (uint8 i=0; i<10; ++i)
		{
			master.serial(i);
		}

		sint32 sizes[4];

		// serial 4 sub messages
		for (uint i=0; i<4; ++i)
		{
			CMessage sub(toString("SUB_%u", i));

			for (uint8 j=0; j<i*4; ++j)
			{
				sub.serial(j);
			}

			string s("A MESSAGE");
			sub.serial(s);

			sizes[i] = sub.length();

			master.serialMessage(sub);
		}

		// invert the message
		master.invert();

		// now, unpack and check

		// read the first master data
		for (uint8 i=0; i<10; ++i)
		{
			uint8 b;
			master.serial(b);

			TEST_ASSERT(b == i);
		}

		// unpack each sub message
		for (uint i=0; i<4; ++i)
		{
			uint32 subSize;
			master.serial(subSize);

			master.lockSubMessage(subSize);
			TEST_ASSERT(subSize == sizes[i]);

			TEST_ASSERT(master.getName() == toString("SUB_%u", i));
			TEST_ASSERT(master.length() == sizes[i]);

			for (uint8 j=0; j<i*4; ++j)
			{
				uint8 b;
				master.serial(b);
				TEST_ASSERT(b == j);
			}

			string s;
			master.serial(s);
			TEST_ASSERT(s == "A MESSAGE");

			TEST_ASSERT(master.getPos() == master.length());

			master.unlockSubMessage();
		}

	}
	
	void messageSwap()
	{
		CMessage msg2;
			
		string s;
		{
			CMessage msg1("NAME");

			s = "foo1";
			msg1.serial(s);
			s = "foo2";
			msg1.serial(s);
			s = "";

			msg2.swap(msg1);

			// check that ms1 is empty now
			TEST_ASSERT(msg1.length() == 0);
			TEST_ASSERT(!msg1.typeIsSet());
		}

		TEST_ASSERT(!msg2.isReading());
		msg2.invert();
		TEST_ASSERT(msg2.typeIsSet());
		TEST_ASSERT(msg2.getName() == "NAME");
		msg2.serial(s);
		TEST_ASSERT(s == "foo1");
		msg2.serial(s);
		TEST_ASSERT(s == "foo2");
	}
};

Test::Suite *createCMessageTS()
{
	return new CMessageTS;
}

