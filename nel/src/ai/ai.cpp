#include "nel/ai/nl_ia.h"
#include "nel/ai/script/compilateur.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/agent/msg_group.h"
#include "nel/ai/script/libcode.h"
#include "nel/ai/c/registry_class.h"


namespace NLAILINK 
{
	using namespace NLAIAGENT;
	void initIALib()
	{
		NLAIC::initRegistry();
		NLAISCRIPT::initExternalLib();		
	}

	void releaseIALib()
	{
		CIndexedVarName::releaseClass();
		NLAIC::releaseRegistry();
		CLocWordNumRef::clear();		
	}
}
