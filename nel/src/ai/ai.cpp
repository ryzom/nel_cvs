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
#include "nel/ai/agent/volatil_memory.h"
#include "nel/ai/agent/operation.h"


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
		NLAIAGENT::CActorScript::initClass();
		NLAIAGENT::CProxyAgentMail::initClass();
		NLAISCRIPT::CLibTest::initClass();

		NLAICHARACTER::CCharacterNoeud::initClass();
		NLAICHARACTER::CCharacterChild::initClass();

		NLAIAGENT::CAgentManagerTimer::initClass();
		CLibTimerManager::initClass();
		CAgentWatchTimer::initClass();
		CAgentClockTimer::initClass();
		CVolatilMemmory::initClass();
		CHashTimerManager::initClass();		
		CAgentOperation::initClass();
	}

	void releaseIALib()
	{	
		NLAIAGENT::CAgentManagerTimer::releaseClass();
		CLibTimerManager::releaseClass();
		CAgentClockTimer::releaseClass();
		CAgentWatchTimer::releaseClass();

		NLAIAGENT::CIndexedVarName::releaseClass();
		NLAIC::releaseRegistry();
		//NLAIAGENT::CLocWordNumRef::clear();
		NLAIAGENT::CAgentScript::releaseAgentScript();
		NLAIAGENT::CFsmScript::releaseClass();
		
		NLAIAGENT::CProxyAgentMail::releaseClass();
		NLAISCRIPT::CLibTest::releaseClass();
		NLAICHARACTER::CCharacterNoeud::releaseClass();
		NLAICHARACTER::CCharacterChild::releaseClass();
		CVolatilMemmory::releaseClass();
		CHashTimerManager::releaseClass();
		CAgentOperation::releaseClass();

		staticReleaseLibClass();

	}

	void setLocalServerID(uint8 u)
	{
		NLAIAGENT::CAgentNumber::ServerId = u;
		NLAIAGENT::CNumericIndex::_I.setCreatorId( (uint64)u );
		NLAIAGENT::CNumericIndex::_I.setDynamicId( (uint64)u );
	}

	uint8 getLocalServerID()
	{
		return NLAIAGENT::CAgentNumber::ServerId;
	}

	void setMainManager(NLAIAGENT::IMainAgent *manager)
	{
		NLAIAGENT::CProxyAgentMail::MainAgent = manager;
		//NLAIAGENT::CProxyAgentMail::MainAgent->incRef();
	}

	void releaseMainManager()
	{
		if(NLAIAGENT::CProxyAgentMail::MainAgent != NULL) 
		{
			NLAIAGENT::CProxyAgentMail::MainAgent->release();
		}
	}

	static char LaseErrorCodeOrdreInterprete[32*1024];

	class IOTrace : public NLAIC::IIO
	{		
	public:

		IOTrace()
		{

		}
		
		virtual void Echo(char *str, ...) const	
		{
			char	temp[32*1024];		
			va_list argument;
			va_start (argument, str);	
			
			vsprintf(temp, str, argument);		
			strcpy(LaseErrorCodeOrdreInterprete,temp);			
			NLAIC::Out("%s",LaseErrorCodeOrdreInterprete);			
				
		}
		virtual const std::string InPut() const
		{
			return std::string();
		}
		virtual void save(NLMISC::IStream &os)
		{
		}
		void getDebugString(std::string &t) const
		{
			t += "this is a IOConsolInterface";
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new IOTrace();
			m->incRef();
			return m;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual void load(NLMISC::IStream &is)
		{
		}

		virtual const NLAIC::CIdentType &getType() const
		{
			static const NLAIC::CIdentType id("IDIOTEMP",	NLAIC::CSelfClassFactory(*this),
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),NLAIC::CTypeOfOperator(0));
			return id;
		}
		

		virtual ~IOTrace()
		{
		}
	};

	void buildScript(const std::string &scriptSrc, const std::string &name)
	{
		NLAISCRIPT::IScriptDebugSource *sourceCode = new NLAISCRIPT::CScriptDebugSourceFile(name.c_str());
		IOTrace Interface;
		NLAISCRIPT::CCompilateur *comp = new NLAISCRIPT::CCompilateur(Interface, scriptSrc.c_str(), scriptSrc.size(), sourceCode);
		NLAIAGENT::IObjectIA::CProcessResult r = comp->Compile();		
#ifdef NL_DEBUG
		if(r.ResultState == NLAIAGENT::processError)
				throw;
#endif
		sourceCode->release();
		delete comp;
	}
}
