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
#include "nel/ai/script/test_method.h"
#include "nel/ai/character/character.h"
#include "nel/ai/logic/fsm_script.h"
#include "nel/ai/agent/agent_timer.h"
//#include "static_init.h"

using namespace NLAIAGENT;

namespace NLAILINK 
{
	//using namespace NLAIAGENT;
	void initIALib()
	{
		staticInitAgent();
		registerLibClass();
		NLAIC::initRegistry();
		NLAISCRIPT::initExternalLib();		
		NLAIAGENT::CAgentScript::initAgentScript();
		NLAIAGENT::CFsmScript::initClass();
		NLAIAGENT::CProxyAgentMail::initClass();
		NLAISCRIPT::CLibTest::initClass();

		NLAICHARACTER::CCharacterNoeud::initClass();
		NLAICHARACTER::CCharacterChild::initClass();

		NLAIAGENT::CAgentManagerTimer::initClass();
		CAgentWatchTimer::initClass();
	}

	void releaseIALib()
	{		
		NLAIAGENT::CIndexedVarName::releaseClass();
		NLAIC::releaseRegistry();
		NLAIAGENT::CLocWordNumRef::clear();
		NLAIAGENT::CAgentScript::releaseAgentScript();
		NLAIAGENT::CFsmScript::releaseClass();
		if(NLAIAGENT::CProxyAgentMail::MainAgent != NULL) 
		{
			NLAIAGENT::CProxyAgentMail::MainAgent->release();
		}
		NLAIAGENT::CProxyAgentMail::releaseClass();
		NLAISCRIPT::CLibTest::releaseClass();
		NLAICHARACTER::CCharacterNoeud::releaseClass();
		NLAICHARACTER::CCharacterChild::releaseClass();

		NLAIAGENT::CAgentManagerTimer::releaseClass();
		CAgentWatchTimer::releaseClass();

		staticReleaseLibClass();
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
