#include "nel/ai/nl_ai.h"
#include "nel/ai/static_init.h"
#include "nel/ai/script/compilateur.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/agent/msg_group.h"
#include "nel/ai/script/libcode.h"
#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/main_agent_script.h"
#include "nel/ai/agent/agent_proxy_mailer.h"

using namespace NLAIAGENT;

namespace NLAILINK 
{
	//using namespace NLAIAGENT;
	void initIALib()
	{
		registerLibClass();
		NLAIC::initRegistry();
		NLAISCRIPT::initExternalLib();		
		NLAIAGENT::CAgentScript::initAgentScript();
		NLAIAGENT::CProxyAgentMail::initClass();
		
	}

	void releaseIALib()
	{		
		NLAIAGENT::CIndexedVarName::releaseClass();
		NLAIC::releaseRegistry();
		NLAIAGENT::CLocWordNumRef::clear();
		NLAIAGENT::CAgentScript::releaseAgentScript();
		if(NLAIAGENT::CProxyAgentMail::MainAgent != NULL) NLAIAGENT::CProxyAgentMail::MainAgent->release();
		NLAIAGENT::CProxyAgentMail::releaseClass();
	}

	void setLocalServerID(uint8 u)
	{
		NLAIAGENT::CAgentNumber::ServerID = u;
		NLAIAGENT::CNumericIndex::_I.CreatorId = (uint64)u;
		NLAIAGENT::CNumericIndex::_I.DynamicId = (uint64)u;
	}

	void setMainManager(NLAIAGENT::IMainAgent *manager)
	{
		NLAIAGENT::CProxyAgentMail::MainAgent = manager;
		//NLAIAGENT::CProxyAgentMail::MainAgent->incRef();
	}	
}
