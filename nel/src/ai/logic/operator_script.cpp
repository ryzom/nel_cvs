#include "nel/ai/logic/operator_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/agent/gd_agent_script.h"
#include "nel/ai/logic/interpret_object_operator.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	COperatorScript::COperatorScript(const COperatorScript &a) : CAgentScript(a)
	{
		_Activated = a._Activated;
		_CurrentGoal = a._CurrentGoal;
	}

	COperatorScript::COperatorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::COperatorClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
		_Activated = false;
		_CurrentGoal = NULL;
	}	

	COperatorScript::COperatorScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
		_Activated = false;
		_CurrentGoal = NULL;
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
		if ( getFactoryClass() != NULL ) 
			return getFactoryClass()->getType();
		else
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
		return CAgentScript::runMethodBase(index, heritance, params);
	}



	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,IObjectIA *params)
	{	

		int i = index - IAgent::getMethodIndexSize();


		if ( i < getBaseMethodCount() )
			return CAgentScript::runMethodBase(index, params);

		IObjectIA::CProcessResult r;

#ifdef NL_DEBUG
		char buf[1024];
		getDebugString(buf);
#endif

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
/*
		if ( *name == CStringVarName("post") )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + fid_activate, 0.0,NULL, r_type ) );
		}*/
		return result;
	}

	const IObjectIA::CProcessResult &COperatorScript::run()
	{

		setState(processBuzzy,NULL);

#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif

		bool is_activated = false;

		// Looks for the goal
		std::vector<NLAILOGIC::CGoal *> &goals = ( (CAgentScript *)getParent() )->getGoalStack();
		std::vector<NLAILOGIC::CGoal *> activated_goals;
		int i;
		for ( i = 0; i < (int) goals.size(); i++ )
		{
			NLAILOGIC::CGoal *av_goal = goals[i];
			const NLAILOGIC::CGoal *op_goal = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal();

#ifdef NL_DEBUG
			char buf_g1[1024 * 2];
			char buf_g2[1024 * 2];

			av_goal->getDebugString(buf_g1);
			op_goal->getDebugString(buf_g2);
#endif
			if ( (*(goals[i])) == *( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() )
				activated_goals.push_back( goals[i] );
		}

		// If a goal is posted corresponding to this operator's one
		if ( activated_goals.size() )
		{
			is_activated = true;

			// Checks the boolean funcs conditions
			NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
			context->Self = this;
			
			if ( !((NLAISCRIPT::COperatorClass *)_AgentClass)->isValidFonc( context ) )
				is_activated = false;

		}
		else
			is_activated = false;

		// Runs the operator if every precondition is validated	
		if ( is_activated )
		{
			if ( _Activated == false)
			{
				// Registers with the goal and gets the args
				NLAILOGIC::CGoal *current_goal = activated_goals.front();
				current_goal->addSuccessor( (IBasicAgent *) this );

				// Executes the OnActivate() function
				tQueue r = _AgentClass->isMember( NULL, &CStringVarName("OnActivate"), NLAISCRIPT::CParam() );
				if ( !r.empty() )
				{
					NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
					context->Self = this;
					runMethodeMember( r.top().Index ,context);
				}

				_Activated = true;
			}
			return CAgentScript::run();
		}
		else
		{
			if ( _Activated == true )
				_Activated = false;
			setState(processIdle,NULL);			
			return IObjectIA::ProcessRun;
		}
	}

	void COperatorScript::cancel()
	{
	}
}
