#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/logic/operator_script.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/script/codage.h"

	
#ifdef NL_DEBUG 
#ifdef NL_OS_WINDOWS
#include "windows.h"
#endif
#endif

namespace NLAISCRIPT
{
#ifdef NL_DEBUG 
	extern bool NL_AI_DEBUG_SERVER;
#endif
	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		_Goal = NULL;
		_Comment = NULL;
		_FactBase = new NLAILOGIC::CFactBase();
		_UpdateCycles = 0;
		_Priority = 1;
	}
	
	COperatorClass::COperatorClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		_Goal = NULL; 
		_Comment = NULL;
		_FactBase = new NLAILOGIC::CFactBase();
		_UpdateCycles = 0;
		_Priority = 1;
	}

	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		_Goal = NULL;
		_Comment = NULL;
		_FactBase = new NLAILOGIC::CFactBase();
		_UpdateCycles = 0;
		_Priority = 1;
	}

	COperatorClass::COperatorClass(const COperatorClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		if ( c._Goal != NULL)
			_Goal = (NLAILOGIC::CGoal *) c._Goal->clone();
		else
			_Goal = NULL;

		if ( c._Comment != NULL )
		{
			_Comment = new char[ strlen( c._Comment ) ];
			strcpy( _Comment, c._Comment );
		}
		else
			_Comment = NULL;

		if ( c._FactBase != NULL)
				_FactBase = (NLAILOGIC::CFactBase *) c._FactBase->clone();
			else
				_FactBase = new NLAILOGIC::CFactBase();

		_UpdateCycles = c._UpdateCycles;
		_Priority = c._Priority;

	}	

	COperatorClass::COperatorClass()
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));
		_Goal = NULL;
		_Comment = NULL;
		_FactBase = new NLAILOGIC::CFactBase();
		_UpdateCycles = 0;
		_Priority = 1;
	}

	COperatorClass::~COperatorClass()
	{
		if ( _Comment != NULL )
			delete[] _Comment;

		if ( _FactBase != NULL )
			_FactBase->release();

		if ( _Goal != NULL )
			_Goal->release();

	
		std::vector<NLAIAGENT::IVarName *>::iterator it_fvar = _FuzzyVars.begin();
		while ( it_fvar != _FuzzyVars.end() )
		{
			(*it_fvar)->release();
			it_fvar++;
		}

		it_fvar = _FuzzySets.begin();
		while ( it_fvar != _FuzzySets.end() )
		{
			(*it_fvar)->release();
			it_fvar++;
		}

		std::vector< IOpCode *>::iterator it_code =  _CondCode.begin();
		while ( it_code != _CondCode.end() )
		{
			(*it_code)->release();
			it_code++;
		}

		it_code = _ConcCode.begin();
		while ( it_code != _ConcCode.end() )
		{
			(*it_code)->release();
			it_code++;
		}
/*
		int i;
		for ( i = 0; i < (int) _CondCode.size(); i++ )
		{
			( (NLAIAGENT::IVarName *)_CondCode[i] )->release();
		}

		for ( i = 0; i < (int) _CondCode.size(); i++ )
		{
			( (NLAIAGENT::IVarName *)_ConcCode[i] )->release();
		}

		for ( i = 0; i < (int) _CondAsserts.size(); i++ )
		{
			( (NLAIAGENT::IVarName *) _CondAsserts[i] )->release();
		}

		for ( i = 0; i < (int) _ConcAsserts.size(); i++ )
		{
			( (NLAIAGENT::IVarName *) _ConcAsserts[i] )->release();
		}

		std::list<const NLAIAGENT::IVarName *>::iterator it_n = _BooleanConds.begin();
		while ( _BooleanConds.size() )
		{
			( (NLAIAGENT::IVarName *) _BooleanConds.front() )->release();
			_BooleanConds.pop_front();
		}

		it_n = _BooleanConcs.begin();
		while ( _BooleanConcs.size() )
		{
			( (NLAIAGENT::IVarName *) _BooleanConcs.front() )->release();
			_BooleanConcs.pop_front();
		}
		*/
	}


	const NLAIC::IBasicType *COperatorClass::clone() const
	{
		NLAIC::IBasicType *clone = new COperatorClass(*this);
		return clone;
	}

	const NLAIC::IBasicType *COperatorClass::newInstance() const
	{
		NLAIC::IBasicType *instance = new COperatorClass();
		return instance;
	}

	void COperatorClass::getDebugString(std::string &t) const
	{
		t += "<COperatorClass>";
		int i;
		for ( i = 0; i < (int) _Vars.size(); i++ )
		{
			std::string buf;
			_Vars[i]->getDebugString(buf);
			t += "   -";
			t += buf;
		}
	}

	NLAIAGENT::IObjectIA *COperatorClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;
		createBaseClassComponents( components );

		// Création du message
		NLAIAGENT::COperatorScript *instance = new NLAIAGENT::COperatorScript( NULL, NULL ,components,  (COperatorClass *) this );

		return instance;
	}

	/// Verifies if the preconditions are validated
	bool COperatorClass::isValid(NLAILOGIC::CFactBase *fb)
	{		
		std::list<NLAILOGIC::CFact *> *facts = new std::list<NLAILOGIC::CFact *>;
		for (sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			std::list<NLAILOGIC::CFact *> *fa = fb->getAssertFacts( _Conds[i] );
			while ( fa->size() )
			{
				facts->push_back( fa->front() );
				fa->pop_front();
			}
			delete fa;
		}
		std::list<NLAILOGIC::CFact *> *res = propagate( *facts );
		bool is_valid = !res->empty();
		while ( res->size() )
		{
#ifdef NL_DEBUG
						
#endif
			res->front()->release();
			res->pop_front();
		}
		delete res;

		while ( facts->size() )
		{
			facts->front()->release();
			facts->pop_front();	
		}
		delete facts;

		return is_valid;
	}

	NLAILOGIC::CValueSet *COperatorClass::unifyBackward(std::list<NLAILOGIC::CFact *> &facts)
	{
		NLAILOGIC::CValueSet *unified = new NLAILOGIC::CValueSet( _Vars.size() );
		std::list<NLAILOGIC::CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_assert;
			getAssertPos( (*it_f)->getAssert(), _Concs, pos_assert );
			for (sint32 pos = 0; pos < (sint32) pos_assert.size(); pos++)
			{
				for ( sint32 ivar = 0; ivar < (sint32) _PosVarsConc[ pos_assert[pos] ].size(); ivar++ )
				{
					sint32 l_pos = _PosVarsConc[ pos_assert[pos] ][ivar];

					NLAIAGENT::IObjectIA *l_val = (*unified)[ l_pos ]; 
					NLAIAGENT::IObjectIA *r_val = (**it_f)[ ivar ];

					if ( !l_val )
					{
						if ( r_val )
						{
							unified->setValue( l_pos, r_val );
						}
					}
					else
					{
						if ( r_val && ( l_val != r_val ) )
						{
							unified->release();
							return NULL;
						}
					}
				}
			}
			it_f++;
		}
		return unified;
	}

	NLAILOGIC::CValueSet *COperatorClass::unifyForward(std::list<NLAILOGIC::CFact *> &facts)
	{
		NLAILOGIC::CValueSet *unified = new NLAILOGIC::CValueSet( _Vars.size() );
		std::list<NLAILOGIC::CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_assert;
			getAssertPos( (*it_f)->getAssert(), _Conds, pos_assert );
			for (sint32 pos = 0; pos < (sint32) pos_assert.size(); pos++)
			{
				for ( sint32 ivar = 0; ivar < (sint32) _PosVarsCond[ pos_assert[pos] ].size(); ivar++ )
				{
					sint32 l_pos = _PosVarsCond[ pos_assert[pos] ][ivar];

					NLAIAGENT::IObjectIA *l_val = (*unified)[ l_pos ]; 
					NLAIAGENT::IObjectIA *r_val = (**it_f)[ ivar ];

					if ( !l_val )
					{
						if ( r_val )
						{
							unified->setValue( l_pos, r_val );
						}
					}
					else
					{
						if ( r_val && ( l_val != r_val ) )
						{
							unified->release();
							return NULL;
						}
					}
				}
			}
			it_f++;
		}
		return unified;
	}

	NLAILOGIC::CFact *COperatorClass::buildFromVars(NLAILOGIC::IBaseAssert *assert, std::vector<sint32> &pl, NLAILOGIC::CValueSet *vars)
	{
		NLAILOGIC::CFact *result = new NLAILOGIC::CFact( assert);	// TODO:: pas besoin du nombre dans ce constructeur puisqu'on a l'assert
		for (sint32 i = 0; i < (sint32) pl.size() ; i++ )
		{
			result->setValue( i, (*vars)[ pl[i] ] );
		}
		return result;
	}

	std::list<NLAILOGIC::CFact *> *COperatorClass::backward(std::list<NLAILOGIC::CFact *> &facts)
	{
		NLAILOGIC::CValueSet *unified = unifyBackward( facts );
		std::list<NLAILOGIC::CFact *> *result = new std::list<NLAILOGIC::CFact *>;
		for (sint32 i = 0; i < (sint32) _Conds.size(); i++ )
		{
			NLAILOGIC::CFact *tmp = buildFromVars( _Conds[i], _PosVarsCond[i], unified );
			result->push_back( tmp );
#ifdef NL_DEBUG
						
#endif
		}
		unified->release();
		return result;
	}

	std::list<NLAILOGIC::CFact *> *COperatorClass::forward(std::list<NLAILOGIC::CFact *> &facts)
	{
		NLAILOGIC::CValueSet *unified = unifyForward( facts );
#ifdef NL_DEBUG
		
#endif
		std::list<NLAILOGIC::CFact *> *result = new std::list<NLAILOGIC::CFact *>;
		for (sint32 i = 0; i < (sint32) _Concs.size(); i++ )
		{
			NLAILOGIC::CFact *tmp = buildFromVars( _Concs[i], _PosVarsConc[i], unified );
			result->push_back( tmp );
#ifdef NL_DEBUG
			
#endif
		}
		unified->release();
		return result;
	}


	std::list<NLAILOGIC::CFact *> *COperatorClass::propagate(std::list<NLAILOGIC::CFact *> &facts)
	{
		std::list<NLAILOGIC::CFact *> *conflicts = new std::list<NLAILOGIC::CFact *>;
		std::list< NLAILOGIC::CValueSet *>	liaisons;
		NLAILOGIC::CValueSet *empty = new NLAILOGIC::CValueSet( _Vars.size() );
		liaisons.push_back( empty );

		std::list<NLAILOGIC::CFact *>::iterator it_f = facts.begin();
		while ( it_f != facts.end() )
		{
			std::vector<sint32> pos_asserts;
			getAssertPos( (*it_f)->getAssert() , _Conds, pos_asserts);
			for (sint32 i = 0; i < (sint32) pos_asserts.size(); i++ )
			{
/*				std::list<NLAILOGIC::CValueSet *> *links = propagate( liaisons, *it_f, _PosVarsCond[ pos_asserts[i] ] );
				if ( links )
				{
					while ( links->size() )
					{
						for (sint32 i = 0; i < (sint32) _Concs.size(); i++ )
						{
							NLAILOGIC::CFact *r = buildFromVars( _Concs[i], _PosVarsConc[i], links->front() );
							char buf[1024];
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
								char buf[1024];
								r->getDebugString( buf );
								conflicts->push_back( r );
							}
						}
						links->front()->release();
						links->pop_front();
					}
					delete links;
				}
				*/
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

	NLAILOGIC::CValueSet *COperatorClass::unifyLiaison( const NLAILOGIC::CValueSet *fp, NLAILOGIC::CValueSet *vals, std::vector<sint32> &pos_vals)
	{
		NLAILOGIC::CValueSet *result;

		if ( (result = fp->unify( vals, pos_vals )) )
			return result;
		else
		{
			delete result;
			return NULL;
		}
	}

	void COperatorClass::getAssertPos(NLAILOGIC::IBaseAssert *a, std::vector<NLAILOGIC::IBaseAssert *> &l, std::vector<sint32> &pos)
	{
		for (sint32 i = 0; i < (sint32) l.size() ; i++ )
		{
			if ( (*(l[i])) == a )
				pos.push_back(i);
		}
	}

	float COperatorClass::priority() const
	{
		return 0.0;
	}

	void COperatorClass::success()
	{
	}

	void COperatorClass::failure()
	{
	}

	void COperatorClass::success(NLAILOGIC::IBaseOperator *)
	{
	}

	void COperatorClass::failure(NLAILOGIC::IBaseOperator *)
	{
	}

	const NLAILOGIC::CGoal *COperatorClass::getGoal()
	{
		return _Goal;
	}

	void COperatorClass::addPrecondition(NLAILOGIC::CFactPattern *pattern)
	{
		if ( pattern->getAssert() )
		{
			std::vector<sint32> pos_Vars;
			compileFactPattern( pattern, _Conds, pos_Vars);

//			pattern->getAssert()->addClause( this, pos_Vars );
			_Conds.push_back( pattern->getAssert() );
			_PosVarsCond.push_back( pos_Vars );

		}
	}

	void COperatorClass::addPostcondition(NLAILOGIC::CFactPattern *pattern)
	{
		if ( pattern->getAssert() )
		{
			std::vector<sint32> pos_Vars;
			compileFactPattern( pattern, _Conds, pos_Vars);

//			pattern->getAssert()->addInput( this );
			_Concs.push_back( pattern->getAssert() );
			_PosVarsConc.push_back( pos_Vars );
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	void COperatorClass::compileFactPattern(NLAILOGIC::CFactPattern *fp, std::vector<NLAILOGIC::IBaseAssert *>&patterns, std::vector<sint32> &pos_Vars)
	{
		// Recherche si variables à ajouter
		std::vector<NLAILOGIC::IBaseVar *> *vars_pattern = fp->getVars();
		if ( vars_pattern )
		{
			std::vector<NLAILOGIC::IBaseVar *>::iterator it_cond = vars_pattern->begin();
			while ( it_cond != vars_pattern->end() )
			{
				sint32 id_var = getVarPos( *it_cond );
				if ( id_var != -1 )
				{
					pos_Vars.push_back( id_var );
				}
				else
				{
					_Vars.push_back( (NLAILOGIC::IBaseVar *)(*it_cond)->clone() );
					pos_Vars.push_back( _Vars.size() - 1);
				}
				it_cond++;
			}
		}

		for ( sint32 i = 0; i < (sint32) vars_pattern->size(); i++ )
		{
			(*vars_pattern)[i]->release();
		}
		delete vars_pattern;
	}
	*/

	void COperatorClass::compileFactPattern(NLAILOGIC::CFactPattern *fp, std::vector<NLAILOGIC::IBaseAssert *>&patterns, std::vector<sint32> &pos_Vars)
	{
		// Recherche si variables à ajouter
		std::vector<NLAILOGIC::IBaseVar *> *vars_pattern = fp->getVars();
		if ( vars_pattern )
		{
			std::vector<NLAILOGIC::IBaseVar *>::iterator it_cond = vars_pattern->begin();
			while ( it_cond != vars_pattern->end() )
			{
				// Looks in the class components if the var already exists
				sint32 id_var = getComponentIndex( (*it_cond)->getName() );

				if ( id_var != -1 )
				{
					// If it exists, stores its index
					pos_Vars.push_back( id_var );
				}
				else
				{
					// If it doesn't exist, registers the var as a component of the class
					NLAIAGENT::CStringVarName var_name("Var");
					registerComponent( var_name , (const NLAIAGENT::CStringVarName &) (*it_cond)->getName() );

					// TODO: contrôle de type

					_Vars.push_back( (NLAILOGIC::IBaseVar *)(*it_cond)->clone() );
					pos_Vars.push_back( _Vars.size() - 1);
				}
				it_cond++;
			}
		}

		for ( sint32 i = 0; i < (sint32) vars_pattern->size(); i++ )
		{
			(*vars_pattern)[i]->release();
		}
		delete vars_pattern;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////

	/// Looks for a variable in the operator's variables vector and returns its position
	sint32 COperatorClass::getVarPos(NLAILOGIC::IBaseVar *var)
	{
		if ( _Vars.size() )
		{
			for (sint32 i = 0; i < (sint32)_Vars.size() ; i++ ) 
			{
				if ( var->getName() == _Vars[ i ]->getName() )
				{
					return i;
				}
			}
		}
		return -1;
	}

	/// Add first order patterns as preconditions or postconditions
	void COperatorClass::addFirstOrderCond(const NLAIAGENT::IVarName *assert_name, std::list<const NLAIAGENT::IVarName *> &params_list)
	{
	/*	_CondAsserts.push_back( assert_name );
		std::list<const NLAIAGENT::IVarName *> *tmp_list = new std::list<const NLAIAGENT::IVarName *>;
		while ( !params_list.empty() )
		{
			const char *txt = params_list.front()->getString();
			tmp_list->push_back( params_list.front() );
			params_list.pop_front();
		}		
		_ClassCondVars.push_back( tmp_list );
		*/

			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)assert_name->clone();
			NLAILOGIC::IBaseAssert *assert = _FactBase->addAssert( name, params_list.size() ); 
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = params_list.begin();
			while ( it_var != params_list.end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPrecondition( pattern );
	}

	void  COperatorClass::addFirstOrderConc(const NLAIAGENT::IVarName *assert_name, std::list<const NLAIAGENT::IVarName *> &params_list)
	{
		_ConcAsserts.push_back( assert_name );
		std::list<const NLAIAGENT::IVarName *> *tmp_list = new std::list<const NLAIAGENT::IVarName *>;
		while ( !params_list.empty() )
		{
			tmp_list->push_back( params_list.front() );
			params_list.pop_front();
		}
		_ClassConcVars.push_back( tmp_list );
	}

	/// Add first order patterns as preconditions or postconditions
	void COperatorClass::addBoolCond(const NLAIAGENT::IVarName *cond_name)
	{
		_BooleanConds.push_back( cond_name );
	}

	void COperatorClass::addBoolConc(const NLAIAGENT::IVarName *conc_name)
	{
		_BooleanConcs.push_back( conc_name );
	}
	
	/// Add first order patterns as preconditions or postconditions
	/// PreConditions code must be any piece of code that returns an object that is true or false using the isTrue() function.
	void COperatorClass::addCodeCond(IOpCode *code)
	{
		_CondCode.push_back( code );
	}

	/// PostConditions code is code that will be executed upon completion of the execution of the operator
	void COperatorClass::addCodeConc(IOpCode *code)
	{
		_ConcCode.push_back( code );
	}

	/// Compiles the conds and concs internaly
	void COperatorClass::buildLogicTables()
	{
//		_FactBase = new NLAILOGIC::CFactBase();
		int i;
		for ( i = 0; i < (int) _CondAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_CondAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = _FactBase->addAssert( name, _ClassCondVars[i]->size() );
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassCondVars[i]->begin();
			while ( it_var != _ClassCondVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPrecondition( pattern );
		}

		for ( i = 0; i < (int) _ConcAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_ConcAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = _FactBase->addAssert( name, _ClassConcVars[i]->size() );
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassConcVars[i]->begin();
			while ( it_var != _ClassConcVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPostcondition( pattern );
		}		
	}

	void COperatorClass::setGoal(const NLAIAGENT::CStringVarName &g)
	{

		_Goal = new NLAILOGIC::CGoal( *(NLAIAGENT::CStringVarName *) g.clone());
	}

	void COperatorClass::setGoal(const NLAIAGENT::IVarName *assert, std::list<const NLAIAGENT::IVarName *> &args)
	{
		_GoalAssert = (const NLAIAGENT::IVarName *) assert->clone();
		_GoalVars = args;

		if ( _Goal != NULL )
		{
			_Goal->release();
		}
	
		_Goal = new NLAILOGIC::CGoal( *(NLAIAGENT::CStringVarName *) assert->clone() );

		std::list<const NLAIAGENT::IVarName *>::iterator it_var = args.begin();
		while ( it_var != args.end() )
		{
			// Looks in the class components if the var already exists
			sint32 id_var = getComponentIndex( **it_var );

			if ( id_var != -1 )
			{
				// If it exists, stores its index
				_GoalPosVar.push_back( id_var );
			}
			else
			{
				// If it doesn't exist, registers the var as a component of the class
				NLAIAGENT::CStringVarName var_name("Var");
				registerComponent( var_name , (const NLAIAGENT::CStringVarName &) **it_var );
//				_Vars.push_back( (NLAILOGIC::IBaseVar *)(*it_cond)->clone() );
				_GoalPosVar.push_back( getComponentIndex(**it_var) );
			}
			it_var++;
		}

		std::list<NLAIAGENT::IObjectIA *> arg_list;
		while ( !args.empty() )
		{
			arg_list.push_back( (NLAIAGENT::IObjectIA *) args.front()->clone() );
			( (NLAIAGENT::IObjectIA *)args.front() )->release();
			args.pop_front();
		}

		_Goal->setArgs( arg_list );
	}

	bool COperatorClass::isValidFonc(NLAIAGENT::IObjectIA *c)
	{
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;
		
		for ( int i = 0; i < (int) _CondCode.size(); i++ )
		{
			NLAISCRIPT::IOpCode *opPtr = _CondCode[ i ];

			IObjectIA::CProcessResult r;
			if(opPtr != NULL)
			{
				NLAISCRIPT::IOpCode &op = *opPtr;
				NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
				int ip;
				if(context.Code != NULL) ip = (uint32)*context.Code;
				else ip =0;
				context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
				*context.Code = 0;

#ifdef NL_DEBUG
				sint sp = context.Stack.CIndexStackPointer::operator int ();
#endif
				r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);

				*context.Code = ip;
				context.Code = opTmp;		

				NLAIAGENT::IObjetOp *result = (NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack];
				result->incRef();
				context.Stack--;

#ifdef NL_DEBUG
				sint u = context.Stack.CIndexStackPointer::operator int ();
				if(sp != u)
				{
					
					throw;
				}
#endif

				if ( result != NULL)
				{
#ifdef NL_DEBUG
					const char *dbg_return_type = (const char *) result->getType();
					
#endif
					bool br = !result->isTrue();
					result->release();
		
					if ( br )
						return false;
				}

			}
		}
		return true;
	}

	void COperatorClass::activatePostConditions(NLAIAGENT::IObjectIA *c)
	{
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;
		
		for ( int i = 0; i < (int) _ConcCode.size(); i++ )
		{
			NLAISCRIPT::IOpCode *opPtr = _ConcCode[ i ];

			IObjectIA::CProcessResult r;
			if(opPtr != NULL)
			{
				NLAISCRIPT::IOpCode &op = *opPtr;
				NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
				int ip = (uint32)*context.Code;
				context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
				*context.Code = 0;

				r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);
				// If we are in Debug Mode
				if (context.ContextDebug.Active)
				{
					context.ContextDebug.callStackPop();
				}
				*context.Code = ip;
				context.Code = opTmp;		
			}
		}
	}
	
	void COperatorClass::initialiseFactBase(NLAILOGIC::CFactBase *inst__FactBase)
	{
		int i;
		for ( i = 0; i < (int) _CondAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_CondAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = inst__FactBase->addAssert( name, _ClassCondVars[i]->size() ); 
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassCondVars[i]->begin();
			while ( it_var != _ClassCondVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPrecondition( pattern );
		}

		for ( i = 0; i < (int) _ConcAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_ConcAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = inst__FactBase->addAssert( name, _ClassConcVars[i]->size() ); 
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassConcVars[i]->begin();
			while ( it_var != _ClassConcVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPostcondition( pattern );
		}
	}

	/// Sets the comment for the operator
	void COperatorClass::setComment(char *c)
	{
		if ( _Comment != NULL )
		{
			delete[] _Comment;
		}
		_Comment = new char[ strlen(c) + 1];
		strcpy(_Comment, c);
	}

	void COperatorClass::addFuzzyCond(NLAIAGENT::IVarName *var_name, NLAIAGENT::IVarName *fset)
	{
		_FuzzyVars.push_back( var_name );
		_FuzzySets.push_back( fset );
	}

	void COperatorClass::RegisterMessage(NLAIAGENT::IMessageBase::TPerformatif perf, const std::string &msg_class, const std::string &msg_varname)
	{
		try
		{

			// Checks if a trigger with the same message or var doesn't already exist
			std::vector<std::string>::iterator it_s = _TrigMsgVarname.begin();
			while ( it_s != _TrigMsgVarname.end() )
			{
				if ( msg_varname == *it_s )
				{
					std::string debugString;
					std::string text;
					getDebugString(debugString);
					text += NLAIC::stringGetBuild("MessageCond(%s) defined twice in operator class '%s'",msg_varname.c_str(), debugString.c_str());
					throw NLAIE::CExceptionNotImplemented(text.c_str()); 
				}
				it_s++;
			}

			NLAIC::CIdentType id_class( msg_class.c_str() );
			_TrigMsgClass.push_back( id_class );
			_TrigMsgPerf.push_back( perf );
			_TrigMsgVarname.push_back( msg_varname );

			// Looks in the class components if the var already exists
			sint32 id_var = getInheritedStaticMemberIndex( NLAIAGENT::CStringVarName((const char *) msg_varname.c_str()) );

			if ( id_var != -1 )
			{
				// If it exists, stores its index
				_TrigMsgPos.push_back( id_var );
			}
			else
			{
				// If it doesn't exist, registers the var as a component of the class
				NLAIAGENT::CStringVarName class_name( (const char *) msg_class.c_str() );
				NLAIAGENT::CStringVarName var_name( (const char *) msg_varname.c_str() );
				registerComponent( class_name, var_name );
				_TrigMsgPos.push_back( getInheritedStaticMemberIndex( NLAIAGENT::CStringVarName((const char *) msg_varname.c_str()) ) );
			}
		}
		catch (NLAIE::IException &err)
		{				
			throw CExceptionHaveNoType(err.what());
		}
	}

	// Checks if a message is in the PreCondition messages list
	sint32 COperatorClass::checkTriggerMsg(const NLAIAGENT::IMessageBase *msg)
	{
		sint32 n = _TrigMsgPerf.size();

		while ( n-- )
		{
			if ( msg->getPerformatif() == _TrigMsgPerf[n] )
			{
#ifdef NL_DEBUG
				const char *msg_dbg = (const char *) msg->getType();				
#endif
				if ( msg->getType() == _TrigMsgClass[n] )
					return _TrigMsgPos[n];
			}
		}
		return -1;
	}
}
