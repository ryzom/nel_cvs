#include "nel/ai/logic/operator_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/agent/gd_agent_script.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/agent/comp_handle.h"
#include "nel/ai/fuzzy/fuzzyset.h"

//#include "agent_service/creature/creature.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	COperatorScript::COperatorScript(const COperatorScript &a) : CActorScript(a)
	{
		_CurrentGoal = a._CurrentGoal;
	}

	COperatorScript::COperatorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::COperatorClass *actor_class )
	: CActorScript(manager, father, components, actor_class )
	{	
		_CurrentGoal = NULL;
	}	

	COperatorScript::COperatorScript(IAgentManager *manager, bool stay_alive) : CActorScript( manager )
	{
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
		/*
		if ( _AgentClass )
		{
			char buf[1024 * 4];
			_AgentClass->getDebugString(buf);
			strcpy(t,buf);
		}
		else
			strcpy(t,"<COperatorScript>");
		strcat(t,"<");
		if ( _IsActivated )
			strcat(t,"activated>");
		else
			strcat(t,"idle>");
		char pri_buf[1024];
		sprintf(pri_buf," <P %f>", priority() );
		strcat(t, pri_buf);
		*/
		CAgentScript::getDebugString(t);
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
/*
	sint32 COperatorScript::getMethodIndexSize() const
	{
		return CAgentScript::getBaseMethodCount();
	}

	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

		if ( index == 1 )
		{
			if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
			{
#ifdef _DEBUG
				const char *dbg_param_type = (const char *) params->getType();
				char dbg_param_string[1024 * 8];
				params->getDebugString(dbg_param_string);
#endif
				const IObjectIA *child = ( ((NLAIAGENT::IBaseGroupType *)params) )->getFront();
				( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
#ifdef _DEBUG
				const char *dbg_param_front_type = (const char *) child->getType();
#endif
				IAgent *new_child = (IAgent *) child->newInstance();
				_Launched.push_back( new_child );
				addChild(new_child);
			}
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
			return r;
		}
		return CAgentScript::runMethodBase(index, heritance, params);
	}
*/

/*
	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,IObjectIA *params)
	{	

#ifdef NL_DEBUG
		char buf[1024];
		getDebugString(buf);
#endif
		int i = index - IAgent::getMethodIndexSize();

		if ( i == 1 )
		{
			if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
			{
#ifdef _DEBUG
				const char *dbg_param_type = (const char *) params->getType();
				char dbg_param_string[1024 * 8];
				params->getDebugString(dbg_param_string);
#endif
				IAgent *new_child = (IAgent *) ((NLAIAGENT::IBaseGroupType *) params)->get();
				_Launched.push_back( new_child );
				addDynamicAgent( (IBaseGroupType *) params);
			}
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
			return r;
		}
		return CAgentScript::runMethodBase(index, params);
	}

	int COperatorScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}

*/
/*
	tQueue COperatorScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		
#ifdef NL_DEBUG
		const char *dbg_func_name = name->getString();
#endif

		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;

		// Processes succes and failure functions
		if ( *name == CStringVarName("Launch") )
		{
		*/
/*			double d;
			d = ((NLAISCRIPT::CParam &)*ParamSuccessMsg).eval((NLAISCRIPT::CParam &)param);
			
			if ( d >= 0.0 )
			{*/
/*			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + 1, 0.0,NULL, r_type ) );
			//}
		}
		return result;
	}
*/

	void COperatorScript::instanciateGoalArgs(NLAILOGIC::CGoal *goal)
	{
		const std::vector<IObjectIA *> &args = goal->getArgs();
		std::vector<IObjectIA *>::const_iterator it_arg = args.begin();
		while ( it_arg != args.end() )
		{
#ifdef NL_DEBUG
			char buffer[1024 * 4];
			(*it_arg)->getDebugString( buffer );
#endif
			it_arg++;
		}
	}

	const IObjectIA::CProcessResult &COperatorScript::run()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif

		bool is_activated = false;

		/*CAgentScript *father = (CAgentScript *) getParent();
		std::vector<NLAILOGIC::CGoal *> &goals = father->getGoalStack();
		std::vector<NLAILOGIC::CGoal *> activated_goals;

		if ( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() != NULL )
		{

			// Looks for the goal
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
		}
		else
		{
			NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
			context->Self = this;

			if ( !((NLAISCRIPT::COperatorClass *)_AgentClass)->isValidFonc( context ) )
				is_activated = false;
		}

		// Runs the operator if every precondition is validated	
		if ( is_activated )
		{
			if ( _IsActivated == false)
			{
				// Registers with the goal and gets the args
				NLAILOGIC::CGoal *current_goal = activated_goals.front();
				instanciateGoalArgs(current_goal);
				current_goal->addSuccessor( (IBasicAgent *) this );


				activate();
				if ( _OnActivateIndex != -1 )
				{
					if ( getAgentManager() != NULL )
					{
						NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
						context->Self = this;
						runMethodeMember( _OnActivateIndex ,context);
						_OnActivateIndex = -1;
					}
				}
			}
			return CAgentScript::run();
		}
		else
		{
			if ( _IsActivated == true )
				unActivate();			
			return IObjectIA::ProcessRun;
		}*/
		return IObjectIA::ProcessRun;
	}

	void COperatorScript::cancel()
	{
		CActorScript::cancel();
	}

	float COperatorScript::priority() const
	{
		int i;
		double pri = 1.0;
		for ( i = 0; i < (int) ( (NLAISCRIPT::COperatorClass *) _AgentClass)->getFuzzyVars().size(); i++)
		{
#ifdef NL_DEBUG
			const char *dbg_set_name = ( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getFuzzySets() )[i]->getString();
			const char *dbg_var_name = ( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getFuzzyVars() )[i]->getString();
#endif
			CComponentHandle var_handle( *(const IVarName *)( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getFuzzySets() )[i],(IAgent *)getParent(), true );
			CComponentHandle set_handle( *(const IVarName *)( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getFuzzyVars() )[i],(IAgent *)getParent(), true );

			NLAIFUZZY::IFuzzySet *set = (NLAIFUZZY::IFuzzySet *) set_handle.getValue();
			DigitalType *var = (DigitalType *) var_handle.getValue();

			// Min
			if ( set != NULL && var != NULL )
			{
				double value = var->getNumber();
				double membership = set->membership( value );
				if ( membership < pri )
					pri = membership;
			}
		}
		return (float) pri;
	}
}
