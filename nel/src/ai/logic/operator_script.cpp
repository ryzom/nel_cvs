#include "nel/ai/logic/operator_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/agent/gd_agent_script.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	COperatorScript::COperatorScript(const COperatorScript &a) : CAgentScript(a)
	{
	}

	COperatorScript::COperatorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::COperatorClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
	}	

	COperatorScript::COperatorScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
	}

	COperatorScript::~COperatorScript()
	{
	}

	const NLAIC::IBasicType *COperatorScript::clone() const
	{		
		COperatorScript *m = new COperatorScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *COperatorScript::newInstance() const
	{	
		COperatorScript *instance;
		if ( _AgentClass )
		{
			instance = (COperatorScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new COperatorScript(NULL);
		}
		return instance;
	}
	
	void COperatorScript::getDebugString(char *t) const
	{
		if ( _AgentClass )
		{
			char buf[1024 * 4];
			_AgentClass->getDebugString(buf);
			strcpy(t,buf);
		}
		else
			strcpy(t,"<COperatorScript>");
	}

	bool COperatorScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIC::CIdentType &COperatorScript::getType() const
	{		
		return IdOperatorScript;
	}

	void COperatorScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		// TODO
	}

	void COperatorScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		// TODO
	}

	sint32 COperatorScript::getMethodIndexSize() const
	{
		return CAgentScript::getBaseMethodCount();
	}

	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

/*		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
*/
		return CAgentScript::runMethodeMember(heritance,index,params);
	}



	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,IObjectIA *params)
	{	

		index = index - IAgent::getMethodIndexSize();
/*

		if ( index < getBaseMethodCount() )
			return CAgentScript::runMethodeMember(index, params);
*/
		IObjectIA::CProcessResult r;

		char buf[1024];
		getDebugString(buf);

		return r;
	}

	int COperatorScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}


	tQueue COperatorScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

		const char *txt = name->getString();

		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;

		if ( *name == CStringVarName("post") )
		{
/*			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + fid_activate, 0.0,NULL, r_type ) );*/
		}
		return result;
	}

	void COperatorScript::setParent(const IWordNumRef *parent)
	{
		// Gets the father's factbase
		const IRefrence *father = (const IRefrence *) *parent;
		NLAILOGIC::CFactBase &fact_base = ( (CAgentScript *)father )->getFactBase();
		
		// Adds the needed asserts to the factbase
		((NLAISCRIPT::COperatorClass *) _AgentClass)->initialiseFactBase( &fact_base );
				
		// Sets the parent
		IRefrence::setParent(parent);		
	}

/*	void COperatorScript::setFactBase(NLAILOGIC::CFactBase *fb)
	{
		_FactBase = fb;
	}
*/
	const IObjectIA::CProcessResult &COperatorScript::run()
	{
		NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
		context->Self = this;

		((NLAISCRIPT::COperatorClass *)_AgentClass)->isValidFonc( context );


		return CAgentScript::run();

//		return IObjectIA::ProcessRun;
/*		if ( _IsActivated )
		{
			return CAgentScript::run();
		}
		else
			return IObjectIA::ProcessRun;
			*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
}
