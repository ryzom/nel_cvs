#include "nel/ai/logic/operator_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/agent/comp_handle.h"
#include "nel/ai/fuzzy/fuzzyset.h"
#include "nel/ai/logic/valueset.h"
#include "nel/ai/logic/factbase.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	COperatorScript::COperatorScript(const COperatorScript &a) : CActorScript(a)
	{
		_CurrentGoal = a._CurrentGoal;		
		/*if ( _CurrentGoal != NULL )
			_CurrentGoal->incRef();*/
		_FactBase = a._FactBase;
		/*if ( _FactBase != NULL )
			_FactBase->incRef();*/

		_CyclesBeforeUpdate = a._CyclesBeforeUpdate;
		_IsActivable = a._IsActivable;
		_Maintain = a._Maintain;
		_Priority = 0;
		_Exclusive = a._Exclusive;
		_FactBase = NULL;
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
		_Maintain = false;
		_Priority = 0;
		_Exclusive = true;
		_FactBase = NULL;
	}	

	COperatorScript::COperatorScript(IAgentManager *manager, bool stay_alive) : CActorScript( manager )
	{
		_CurrentGoal = NULL;
		_CyclesBeforeUpdate = 0;
		_IsActivable = false;
		_Maintain = false;
		_Priority = 0;
		_Exclusive = true;
		_FactBase = NULL;
	}

	COperatorScript::~COperatorScript()
	{
#ifdef NL_DEBUG
		//const char *className = (const char *)getType();
#endif

		/*if ( _CurrentGoal != NULL )
			_CurrentGoal->release();

		if ( _FactBase != NULL )
			_FactBase->release();*/

		std::vector<NLAIAGENT::IObjectIA *>::iterator it_val = _VarValues.begin();
		while ( it_val != _VarValues.end() )
		{
			(*it_val)->release();
			it_val++;
		}
	}

	void COperatorScript::onKill(IConnectIA *a)
	{
		CActorScript::onKill(a);
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
		std::string text;
		text = NLAIC::stringGetBuild("%f", priority() );
		t += text;
		t += "\n\t";

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

	// This is where the activation of the operator and its children is controled...
	const IObjectIA::CProcessResult &COperatorScript::run()
	{

#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
		std::string dbg_goal;		
#endif

		// Checks the goal and preconditions after a certain number of cycles (defined in the script using the UdpateEvery keyword)
		if ( _CyclesBeforeUpdate == 0 )
		{
			_IsActivable = checkActivation();
			_CyclesBeforeUpdate = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getUpdateEvery();
			calcPriority();
		}
		else
			_CyclesBeforeUpdate--;

		if ( _IsActivable ) // If the goal exists and the preconditions are validated
		{
			if ( ! _IsActivated )		// If the operator wasn't activated before, initialises the current goal and runs the OnActivate() function
			{
				// Registers to the goal and gets the args
				if ( _CurrentGoal == NULL && ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() != NULL )
				{
					_CurrentGoal = selectGoal();							// Select a goal among possible ones
					//_CurrentGoal->incRef();
					_CurrentGoal->addSuccessor( (IBasicAgent *) this );		// Adds the operator to the list of operators launched for this goal
					linkGoalArgs( _CurrentGoal );							// Instanciates the goal's args values into the operator's components
				}

				if ( ( _CurrentGoal != NULL && _CurrentGoal->isSelected() ) || ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() == NULL)
				{
					activate();
					execOnActivate();	// Execs onActivate() function in the script if defined, otherwise the C++ one inherited from CActorScript.
				}
			}
			else
				checkPause();			// Check if the agent must be paused / restarted

			// If the operator is activated and not paused, runs it (including its children), 
			// otherwise just process messages
			if ( _IsActivated )
			{
				if( !_IsPaused ) 
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
			if ( _IsActivated == true )		// If the operator is curently activated, unactivates it
			{
				unActivate();
			}
			processMessages();
			return IObjectIA::ProcessRun;
		}
		return IObjectIA::ProcessRun;
	}


	void COperatorScript::execOnActivate()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif
		if ( _OnActivateIndex != -1 ) 					// if found, runs the script's OnActivate() function declared in the operator class
		{
			if ( getAgentManager() != NULL )
			{
				NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
#ifdef NL_DEBUG
				sint sp = context->Stack.CIndexStackPointer::operator int ();
#endif
				context->Self = this;
				IObjectIA::CProcessResult r = runMethodeMember( _OnActivateIndex , context);
				if(r.Result != NULL)
				{
					throw;
				}

#ifdef NL_DEBUG
				if ( sp != context->Stack.CIndexStackPointer::operator int () )
				{
					throw;
				}
#endif
				_OnActivateIndex = -1;
			}
		}
		else
			onActivate();								// Otherwise runs the C++ virtual one (inherited from CActorScript)
	}

	void COperatorScript::checkPause()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif
		if ( (_CurrentGoal != NULL) )
		{
			if ( !_IsPaused && !_CurrentGoal->isSelected() )
				pause();
			if ( _IsPaused && _CurrentGoal->isSelected() )
				restart();
		}
	}

	// This function selects a goals among the ones the operator can process
	// Actualy returns the first of the list (can be overriden to implement more complex mechanisms)
	NLAILOGIC::CGoal *COperatorScript::selectGoal()
	{
		if ( !_ActivatedGoals.empty() )
			return _ActivatedGoals.front();
		else
			return NULL;
	}

	// Check if the owner of the operator has a goal the operator could process and if the preconditions are validated
	bool COperatorScript::checkActivation()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif

		// If not in maintain mode and the operator has already been activated, keeps it activated.
		if ( _IsActivated && _CurrentGoal != NULL && !_Maintain)
			return true;

		if ( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() != NULL)
		{
//			if ( _CurrentGoal != NULL )
				lookForGoals();

			// If no goal is posted corresponding to this operator's one, returns false
			if ( _ActivatedGoals.empty() )
				return false;	
		}
		// Checks the boolean funcs conditions
		return checkPreconditions();
	}

	// Looks for the goals the operator could process in the father's goal stack
	void COperatorScript::lookForGoals()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif
		_ActivatedGoals.clear();
		CAgentScript *father = (CAgentScript *) getParent();
		const std::vector<NLAILOGIC::CGoal *> *goals = father->getGoalStack();

		int i;
		for ( i = 0; i < (int) goals->size(); i++ )
		{
			NLAILOGIC::CGoal *av_goal = (*goals)[i];
			const NLAILOGIC::CGoal *op_goal = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal();

			if ( (*( (*goals)[i])) == *( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoal() )
				_ActivatedGoals.push_back( (*goals)[i] );
		}
	}

	// Checks the boolean scripted funcs conditions
	bool COperatorScript::checkPreconditions()
	{
#ifdef NL_DEBUG
		const char *dbg_class_name = (const char *) getType();
#endif
		// Checks for message triggers
		if ( ( (NLAISCRIPT::COperatorClass *) _AgentClass )->NbMsgTrigger() != 0 && !checkTrigMsg() )
			return false;

		NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
		context->Self = this;
		return ((NLAISCRIPT::COperatorClass *)_AgentClass)->isValidFonc( context );
	}

	// Called by the goal when canceled: removes all childs and unactivates the operator.
	void COperatorScript::cancel()
	{
		CActorScript::cancel();
	}

	// The priority of an operator is calculated as follow:
	//		
	//		- If the operator is not activable (no corresponding goal and / or preconditions are not validated), return 0
	//		- If fuzzy conds exist,  returns their truth value multiplied by the operator's class priority (defined in the script using the Priority keyword)
	//		- If no fuzzy conds, return 1 * the operator's class priority (defined in the script using the Priority keyword)
	void COperatorScript::calcPriority()
	{
		int i;
		double pri = 1.0;
		for ( i = 0; i < (int) ( (NLAISCRIPT::COperatorClass *) _AgentClass)->getFuzzyVars().size(); i++)
		{
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

		if (! _IsActivable )
			_Priority = 0.0;
		else
			_Priority = (float)pri * class_pri;
	}

	float COperatorScript::priority() const
	{
		return _Priority;
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

	// Tries to unify a new first order logic fact with current instanciations of the operator's vars
	std::list<NLAILOGIC::CValueSet *> *COperatorScript::propagate(std::list<NLAILOGIC::CValueSet *> &liaisons, NLAILOGIC::CValueSet *fact, std::vector<sint32> &pos_vals) 
	{
		std::list<NLAILOGIC::CValueSet *> *conflits = new std::list<NLAILOGIC::CValueSet *>;
		std::list<NLAILOGIC::CValueSet *> buf_liaisons;
		// Pour chaque liaison...
		std::list< NLAILOGIC::CValueSet *>::iterator it_l = liaisons.begin();
		
		while ( it_l != liaisons.end() )
		{

			NLAILOGIC::CValueSet *l = *it_l;

			NLAILOGIC::CValueSet *result = unifyLiaison( l, fact, pos_vals );
			if ( result )
			{

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


	// Tries to unify a list of new first order logic facts with current instanciations of the operator's vars
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


	// Simple unifcation between two first order logic patterns
	NLAILOGIC::CValueSet *COperatorScript::unifyLiaison( const NLAILOGIC::CValueSet *fp, NLAILOGIC::CValueSet *vals, std::vector<sint32> &pos_vals)
	{
		NLAILOGIC::CValueSet *result;

		if ( result = fp->unify( vals, pos_vals ) )
			return result;
		else
		{
			return NULL;
		}
	}

	// Instanciates a goals's args as static components of the operator
	void COperatorScript::linkGoalArgs(NLAILOGIC::CGoal *g)
	{
		std::vector<NLAIAGENT::IObjectIA *>::const_iterator it_arg = g->getArgs().begin();
		std::vector<sint32>::iterator it_pos = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->getGoalVarPos().begin();
		while ( it_arg != g->getArgs().end() )
		{
			if(!setStaticMember( *it_pos, *it_arg ))
										(*it_arg)->incRef();
			it_arg++;
			it_pos++;
		}
	}

	// Function called when a launched actor succeded
	void COperatorScript::onSuccess( IObjectIA *)
	{
#ifdef NL_DEBUG
		const char *dbg_class = (const char *) getType();
#endif

		if(_CurrentGoal == NULL) 
			return;
		_CurrentGoal->operatorSuccess( this );
		_CurrentGoal = NULL;
		unActivate();
	}
	
	// Function called when a lauched actor failed
	void COperatorScript::onFailure( IObjectIA *)
	{

#ifdef NL_DEBUG
		const char *dbg_class = (const char *) getType();
#endif

		if(_CurrentGoal == NULL) 
			return;
		// Tells the goal the operator failed
		_CurrentGoal->operatorFailure( this );
		_CurrentGoal = NULL;
		unActivate();
	}

	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

		switch ( index )
		{
			case fid_modeachieve:
				_Maintain = false;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_modemaintain:
				_Maintain = true;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_isActivable:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAILOGIC::CBoolType( _IsActivable );
				return r;

			case fid_isPaused:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAILOGIC::CBoolType( _IsPaused );
				return r;

			case fid_getPriority:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( priority() );
				return r;

			case fid_background:
				_Exclusive = false;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_exclusive:
				_Exclusive = true;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_achieve:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
		}
		return r;
	}



	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,IObjectIA *params)
	{	
		int i = index - CActorScript::getMethodIndexSize();


		IObjectIA::CProcessResult r;
		std::vector<CStringType *> handles;


		switch( i )
		{
			case fid_modemaintain:
				_Maintain = true;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_modeachieve:
				_Maintain = false;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_isActivable:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAILOGIC::CBoolType( _IsActivable );
				return r;

			case fid_isPaused:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAILOGIC::CBoolType( _IsPaused );
				return r;

			case fid_getPriority:
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = new NLAIAGENT::DigitalType( priority() );
				return r;

			case fid_background:
				_Exclusive = false;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

			case fid_exclusive:
				_Exclusive = true;
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;

		}
		return CActorScript::runMethodBase(index, params);
	}

	int COperatorScript::getBaseMethodCount() const
	{
		return CActorScript::getBaseMethodCount() + fid_last;
	}

	tQueue COperatorScript::getPrivateMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

#ifdef NL_DEBUG
		const char *dbg_func_name = name->getString();		
#endif

		tQueue result; 

		static NLAIAGENT::CStringVarName modeachieve_name("SetModeAchieve");
		static NLAIAGENT::CStringVarName modemaintain_name("SetModeMaintain");
		static NLAIAGENT::CStringVarName ispaused_name("IsPaused");
		static NLAIAGENT::CStringVarName isactivable_name("IsActivable");
		static NLAIAGENT::CStringVarName priority_name("GetPriority");
		static NLAIAGENT::CStringVarName exclusive_name("SetExclusive");
		static NLAIAGENT::CStringVarName background_name("SetBackground");


		if ( *name == modeachieve_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  CActorScript::getMethodIndexSize() + fid_modeachieve, 0.0,NULL, r_type ) );
		}

		if ( *name == modemaintain_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CActorScript::getMethodIndexSize() + fid_modemaintain , 0.0,NULL, r_type ) );
		}

		if ( *name == ispaused_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CActorScript::getMethodIndexSize() + fid_isPaused , 0.0,NULL, r_type ) );
		}


		if ( *name == isactivable_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CActorScript::getMethodIndexSize() + fid_isActivable , 0.0,NULL, r_type ) );
		}

		if ( *name == priority_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CActorScript::getMethodIndexSize() + fid_getPriority , 0.0,NULL, r_type ) );
		}

		if ( *name == exclusive_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CActorScript::getMethodIndexSize() + fid_exclusive , 0.0,NULL, r_type ) );
		}

		if ( *name == background_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CActorScript::getMethodIndexSize() + fid_background , 0.0,NULL, r_type ) );
		}

		if ( result.empty() )
			return CActorScript::getPrivateMember(className, name, param);
		else
			return result;
	}

	sint32 COperatorScript::getMethodIndexSize() const
	{
		return CActorScript::getMethodIndexSize() + fid_last;
	}

	bool COperatorScript::isExclusive()
	{
#ifdef NL_DEBUG
		const char *dbg_exclusive = (const char *) getType();
#endif
		return _Exclusive;
	}

	bool COperatorScript::checkTrigMsg()
	{
		IBasicAgent *father = (IBasicAgent *) getParent();
		if ( father != NULL )
		{
			IMailBox *mailbox = father->getMail();
			if ( mailbox != NULL )
			{
				std::list<const IMessageBase *>::const_iterator it_msg;
				const std::list<const IMessageBase *> &msg_list = mailbox->getMesseageListe();

				it_msg = msg_list.begin();
				while ( it_msg != msg_list.end() )
				{
					IMessageBase *msg = (NLAIAGENT::IMessageBase *)*it_msg;
#ifdef NL_DEBUG
					const char *id = (const char *) msg->getType();					
#endif

					sint32 msg_comp_pos = ( (NLAISCRIPT::COperatorClass *) _AgentClass )->checkTriggerMsg( msg );
					if ( msg_comp_pos == -1)
						return false;
					else
					{
						if(!setStaticMember( msg_comp_pos, (NLAIAGENT::IObjectIA *) msg ))
																				msg->incRef();						
					}
					it_msg++;
				}	
			}
		}
		return false;
	}
}
