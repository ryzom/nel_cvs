#include "nel/ai/logic/operator_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/agent/gd_agent_script.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/agent/comp_handle.h"
#include "nel/ai/fuzzy/fuzzyset.h"
#include "nel/ai/logic/valueset.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	COperatorScript::COperatorScript(const COperatorScript &a) : CActorScript(a)
	{
		_CurrentGoal = a._CurrentGoal;
		_CyclesBeforeUpdate = a._CyclesBeforeUpdate;
		_IsActivable = false;
	}

	COperatorScript::COperatorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::COperatorClass *actor_class )
	: CActorScript(manager, father, components, actor_class )
	{	
		_CurrentGoal = NULL;
		_CyclesBeforeUpdate = 0;
		_IsActivable = false;
	}	

	COperatorScript::COperatorScript(IAgentManager *manager, bool stay_alive) : CActorScript( manager )
	{
		_CurrentGoal = NULL;
		_CyclesBeforeUpdate = 0;
		_IsActivable = false;
	}

	COperatorScript::~COperatorScript()
	{
#ifdef NL_DEBUG
		const char *className = (const char *)getType();
#endif
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
	
	void COperatorScript::getDebugString(std::string &t) const
	{
		if ( _AgentClass )
			_AgentClass->getDebugString(t);
		else
			t += "<COperatorScript>";
		t += "<";

		if ( _IsActivated )
			t += "activated>";
		else
			t += "idle>";
		t += " <P ";
		char pri_buf[128];
		sprintf(pri_buf, "%f", priority() );
		t += pri_buf;

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

	const IObjectIA::CProcessResult &COperatorScript::run()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif

		if ( _CyclesBeforeUpdate == 0 )
		{
			_IsActivable = checkActivation();
			_CyclesBeforeUpdate = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getUpdateEvery();
		}
		else
			_CyclesBeforeUpdate--;


		// Runs the operator if every precondition is validated	
		if ( _IsActivable )
		{
			// If it wasn't activated before, initialises the current goal and runs the OnActivate() function
			if ( _IsActivated == false)			
			{
				// Registers to the goal and gets the args
				if ( _CurrentGoal == NULL && ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() != NULL )
				{
					_CurrentGoal = selectGoal();							// Select a goal among possible ones
					_CurrentGoal->addSuccessor( (IBasicAgent *) this );		// Adds the operator to the list of operators launched for this goal
					linkGoalArgs( _CurrentGoal );							// Instanciates the goal's args values into the operator's components
				}

				if ( ( _CurrentGoal != NULL && _CurrentGoal->isSelected() ) || ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() == NULL)
				{
					activate();

					// if found, runs the script's OnActivate() function declared in the operator class
					if ( _OnActivateIndex != -1 )
					{
						if ( getAgentManager() != NULL )
						{
							NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
							context->Self = this;
							runMethodeMember( _OnActivateIndex , context);
							_OnActivateIndex = -1;
						}
					}
					else
						onActivate();
				}
			}
			else
				if ( (_CurrentGoal != NULL) )
				{
					if ( (_IsPaused == false) && !_CurrentGoal->isSelected() )
						pause();
					if ( (_IsPaused == true)  && _CurrentGoal->isSelected() )
						restart();
				}

			if ( _CurrentGoal != NULL)
			{
				if(_CurrentGoal->isSelected()) 
					return CAgentScript::run();
				else
				{
					processMessages();
					return IObjectIA::ProcessRun;
				}
			}		
		}
		else
		{
			// If the operator is curently activated, unactivates it
			if ( _IsActivated == true )
			{
				unActivate();			
//				_IsPaused = false;
			}
			return IObjectIA::ProcessRun;
		}
		return IObjectIA::ProcessRun;
	}

	// This function selects a goals among the ones the operator can process
	// At this times returns the first of the list (can be overriden to implement more complex mechanisms)
	NLAILOGIC::CGoal *COperatorScript::selectGoal()
	{
		if ( !_ActivatedGoals.empty() )
			return _ActivatedGoals.front();
		else
			return NULL;
	}


	bool COperatorScript::checkActivation()
	{
		if ( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() != NULL)
		{
			lookForGoals();

			// If a goal is posted corresponding to this operator's one
			if ( _ActivatedGoals.empty() )
				return false;	// Checks the boolean funcs conditions
		}
	
		// If the operator doesn't have a defined goal, acts like a simple trigger (which means only check the preconditions)
		return checkPreconditions();
	}

	// Looks for the goals the operator could process in the father's goal stack
	void COperatorScript::lookForGoals()
	{
		_ActivatedGoals.clear();
		CAgentScript *father = (CAgentScript *) getParent();
		const std::vector<NLAILOGIC::CGoal *> *goals = father->getGoalStack();

		int i;
		for ( i = 0; i < (int) goals->size(); i++ )
		{
			NLAILOGIC::CGoal *av_goal = (*goals)[i];
			const NLAILOGIC::CGoal *op_goal = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal();

#ifdef NL_DEBUG
			std::string buf_g1;
			std::string buf_g2;
			av_goal->getDebugString(buf_g1);
			op_goal->getDebugString(buf_g2);
#endif
			if ( (*( (*goals)[i])) == *( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() )
				_ActivatedGoals.push_back( (*goals)[i] );
		}
	}

	// Checks the boolean scripted funcs conditions
	bool COperatorScript::checkPreconditions()
	{
		NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
		context->Self = this;
		return ((NLAISCRIPT::COperatorClass *)_AgentClass)->isValidFonc( context );
	}

	void COperatorScript::cancel()
	{
		CActorScript::cancel();
	}

	float COperatorScript::priority() const
	{
		if (! _IsActivable )
			return 0.0;

		int i;
		double pri = 1.0;
		for ( i = 0; i < (int) ( (NLAISCRIPT::COperatorClass *) _AgentClass)->getFuzzyVars().size(); i++)
		{
#ifdef NL_DEBUG
			const char *dbg_op_class = (const char *) ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getType();
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

#ifdef NL_DEBUG
		const char *dbg_class = (const char *) getType();
#endif

		float class_pri = ( (NLAISCRIPT::COperatorClass *) _AgentClass)->getPriority();
		return pri * class_pri;
	}

	NLAILOGIC::CFact *COperatorScript::buildFromVars(NLAILOGIC::IBaseAssert *assert, std::vector<sint32> &pl, NLAILOGIC::CValueSet *vars)
	{
		NLAILOGIC::CFact *result = new NLAILOGIC::CFact( assert);	// TODO:: pas besoin du nombre dans ce constructeur puisqu'on a l'assert
		for (sint32 i = 0; i < (sint32) pl.size() ; i++ )
		{
			sint32 p = pl[i];
			result->setValue( i, (*vars)[ pl[i] ] );
		}
		return result;
	}

	std::list<NLAILOGIC::CValueSet *> *COperatorScript::propagate(std::list<NLAILOGIC::CValueSet *> &liaisons, NLAILOGIC::CValueSet *fact, std::vector<sint32> &pos_vals) 
	{
		std::list<NLAILOGIC::CValueSet *> *conflits = new std::list<NLAILOGIC::CValueSet *>;
		std::list<NLAILOGIC::CValueSet *> buf_liaisons;
		// Pour chaque liaison...
		std::list< NLAILOGIC::CValueSet *>::iterator it_l = liaisons.begin();
		
		while ( it_l != liaisons.end() )
		{

			NLAILOGIC::CValueSet *l = *it_l;
#ifdef NL_DEBUG
			std::string buf;
			l->getDebugString( buf );
#endif

			NLAILOGIC::CValueSet *result = unifyLiaison( l, fact, pos_vals );
			if ( result )
			{
#ifdef NL_DEBUG
				std::string buf;
				result->getDebugString( buf );
#endif

				if ( result->undefined() == 0 )
				{
					conflits->push_back( result );
				}
				else 
					buf_liaisons.push_back( result );
			}
			it_l++;
		}

		while ( buf_liaisons.size() )
		{
			liaisons.push_back( buf_liaisons.front() );
			buf_liaisons.pop_front();
		}

		return conflits;
	}

	std::list<NLAILOGIC::CFact *> *COperatorScript::propagate(std::list<NLAILOGIC::CFact *> &facts)
	{
		std::list<NLAILOGIC::CFact *> *conflicts = new std::list<NLAILOGIC::CFact *>;
		std::list< NLAILOGIC::CValueSet *>	liaisons;
		NLAILOGIC::CValueSet *empty = new NLAILOGIC::CValueSet( ( (NLAISCRIPT::COperatorClass *)_AgentClass)->getVars().size() );
		liaisons.push_back( empty );
		
		NLAISCRIPT::COperatorClass *op_class = (NLAISCRIPT::COperatorClass *) _AgentClass;

		std::list<NLAILOGIC::CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_asserts;
			op_class->getAssertPos( (*it_f)->getAssert() , op_class->getConds(), pos_asserts);
			for (sint32 i = 0; i < (sint32) pos_asserts.size(); i++ )
			{
				std::list<NLAILOGIC::CValueSet *> *links = propagate( liaisons, *it_f, op_class->getPosVarsConds()[ pos_asserts[i] ] );
				if ( links )
				{
					while ( links->size() )
					{
						for (sint32 i = 0; i < (sint32) op_class->getConcs().size(); i++ )
						{
							NLAILOGIC::CFact *r = buildFromVars( op_class->getConcs()[i], op_class->getPosVarsConcs()[i], links->front() );
							std::string buf;
							r->getDebugString( buf );
							// Tests if the fact is already in the conflicts list
							bool found = false;
							std::list<NLAILOGIC::CFact *>::iterator it_c = conflicts->begin();
							while ( ! found && it_c != conflicts->end() )
							{
								found = (**it_c) == *r;
								it_c++;
							}
							if ( !found )
							{
								std::string buf;
								r->getDebugString( buf );
								conflicts->push_back( r );
							}
						}
						links->front()->release();
						links->pop_front();
					}
					delete links;
				}
			}
			it_f++;
		}

		while ( liaisons.size() )
		{
			liaisons.front()->release();
			liaisons.pop_front();
		}
		return conflicts;
	}

	NLAILOGIC::CValueSet *COperatorScript::unifyLiaison( const NLAILOGIC::CValueSet *fp, NLAILOGIC::CValueSet *vals, std::vector<sint32> &pos_vals)
	{
		NLAILOGIC::CValueSet *result;

		if ( result = fp->unify( vals, pos_vals ) )
			return result;
		else
		{
			delete result;
			return NULL;
		}
	}

	void COperatorScript::linkGoalArgs(NLAILOGIC::CGoal *g)
	{
		std::vector<NLAIAGENT::IObjectIA *>::const_iterator it_arg = g->getArgs().begin();
		std::vector<sint32>::iterator it_pos = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoalVarPos().begin();
		while ( it_arg != g->getArgs().end() )
		{
			setStaticMember( *it_pos, *it_arg );
			it_arg++;
			it_pos++;
		}
	}

	void COperatorScript::onSuccess( IObjectIA *)
	{
		// Tells the goal the operator succeded
		_CurrentGoal->operatorSuccess( this );
		_CurrentGoal = NULL;
		unActivate();
	}
	
	void COperatorScript::onFailure( IObjectIA *)
	{
		// Tells the goal the operator failed
		_CurrentGoal->operatorFailure( this );
		_CurrentGoal = NULL;
		unActivate();
	}
}
