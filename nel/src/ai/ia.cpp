#include "link/nl_ia.h"
#include "script/compilateur.h"
#include "agent/agent.h"
#include "agent/agent_digital.h"
#include "agent/msg_group.h"
#include "script/libcode.h"
#include "c/registry_class.h"


namespace NLIALINK 
{
	using namespace NLAIAGENT;
	void initIALib()
	{
		NLAIC::initRegistry();
		NLIASCRIPT::initExternalLib();		
	}

	void releaseIALib()
	{
		CIndexedVarName::releaseClass();
		NLAIC::releaseRegistry();
		CLocWordNumRef::clear();		
	}
}