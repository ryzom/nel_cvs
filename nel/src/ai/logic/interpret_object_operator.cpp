#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/logic/operator_script.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/factbase.h"
#include "nel/ai/logic/varset.h"
#include "nel/ai/script/codage.h"

namespace NLAISCRIPT
{
	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		_Goal = NULL;
	}
	
	COperatorClass::COperatorClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		_Goal = NULL;
	}

	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		_Goal = NULL;
	}

	COperatorClass::COperatorClass(const COperatorClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
		if ( c._Goal != NULL)
			_Goal = (NLAILOGIC::CGoal *) c._Goal->clone();
		else
			_Goal = NULL;
	}	

	COperatorClass::COperatorClass()
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));
		_Goal = NULL;
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

	void COperatorClass::getDebugString(char *t) const
	{
		strcpy( t, "<COperatorClass>\n");
		int i;
		for ( i = 0; i < (int) _Vars.size(); i++ )
		{
			char buf[1024];
			_Vars[i]->getDebugString(buf);
			strcat(t,"   -");
			strcat(t, buf);
			strcat(t,"\n");
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

	COperatorClass::~COperatorClass()
	{
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

	/// Verifies if the preconditions are validated<
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
			char buffer[2054];
			res->front()->getDebugString( buffer );
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

					NLAIAGENT::IObjetOp *l_val = (*unified)[ l_pos ]; 
					NLAIAGENT::IObjetOp *r_val = (**it_f)[ ivar ];

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

					NLAIAGENT::IObjetOp *l_val = (*unified)[ l_pos ]; 
					NLAIAGENT::IObjetOp *r_val = (**it_f)[ ivar ];

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
			sint32 p = pl[i];
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
			char buffer[1024];
			tmp->getDebugString(buffer);
		}
		unified->release();
		return result;
	}

	std::list<NLAILOGIC::CFact *> *COperatorClass::forward(std::list<NLAILOGIC::CFact *> &facts)
	{
		NLAILOGIC::CValueSet *unified = unifyForward( facts );
		char buf[1024];
		unified->getDebugString( buf );
		std::list<NLAILOGIC::CFact *> *result = new std::list<NLAILOGIC::CFact *>;
		for (sint32 i = 0; i < (sint32) _Concs.size(); i++ )
		{
			NLAILOGIC::CFact *tmp = buildFromVars( _Concs[i], _PosVarsConc[i], unified );
			result->push_back( tmp );
			char buffer[1024];
			tmp->getDebugString(buffer);
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

		if ( result = fp->unify( vals, pos_vals ) )
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

/*	void COperatorClass::setGoal(NLAILOGIC::CGoal *g)
	{
		if ( _Goal != NULL )
		{
			_Goal->release();
		}

		_Goal = g;

		if ( _Goal )
			_Goal->incRef();
	}
	*/

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

		_CondAsserts.push_back( assert_name );
		std::list<const NLAIAGENT::IVarName *> *tmp_list = new std::list<const NLAIAGENT::IVarName *>;
		while ( !params_list.empty() )
		{
			const char *txt = params_list.front()->getString();
			tmp_list->push_back( params_list.front() );
			params_list.pop_front();
		}		
		_ClassCondVars.push_back( tmp_list );
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
		fact_base = new NLAILOGIC::CFactBase();
		int i;
		for ( i = 0; i < (int) _CondAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_CondAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = fact_base->addAssert( name, _ClassCondVars[i]->size() );
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassCondVars[i]->begin();
			while ( it_var != _ClassCondVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				const char *txt = var_name.getString();
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPrecondition( pattern );
		}

		for ( i = 0; i < (int) _ConcAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_ConcAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = fact_base->addAssert( name, _ClassConcVars[i]->size() );
			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassConcVars[i]->begin();
			while ( it_var != _ClassConcVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				const char *txt = var_name.getString();
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPostcondition( pattern );
		}
	}

	void COperatorClass::setGoal(NLAIAGENT::CStringVarName &g)
	{
		if ( _Goal != NULL )
		{
			_Goal->release();
		}

		_Goal = new NLAILOGIC::CGoal( *(NLAIAGENT::CStringVarName *) g.clone());
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
				int ip = (uint32)*context.Code;
				context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
				*context.Code = 0;


				r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);
				// If we are in Debug Mode
				/*if (context.ContextDebug.Active)
				{
					context.ContextDebug.callStackPop();
				}*/
				*context.Code = ip;
				context.Code = opTmp;		

				if ( r.Result != NULL)
					return false;
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
	
	void COperatorClass::initialiseFactBase(NLAILOGIC::CFactBase *inst_fact_base)
	{
		int i;
		for ( i = 0; i < (int) _CondAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_CondAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = inst_fact_base->addAssert( name, _ClassCondVars[i]->size() );
/*			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassCondVars[i]->begin();
			while ( it_var != _ClassCondVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				const char *txt = var_name.getString();
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPrecondition( pattern );
			*/
		}

		for ( i = 0; i < (int) _ConcAsserts.size() ; i++ )
		{
			NLAIAGENT::CStringVarName name = *(NLAIAGENT::CStringVarName *)_ConcAsserts[i]->clone();
			NLAILOGIC::IBaseAssert *assert = inst_fact_base->addAssert( name, _ClassConcVars[i]->size() );
/*			NLAILOGIC::CFactPattern *pattern = new NLAILOGIC::CFactPattern( assert );
			std::list<const NLAIAGENT::IVarName *>::iterator it_var = _ClassConcVars[i]->begin();
			while ( it_var != _ClassConcVars[i]->end() )
			{
				NLAIAGENT::CStringVarName var_name = *(NLAIAGENT::CStringVarName *)(*it_var);
				const char *txt = var_name.getString();
				pattern->addVar(  new NLAILOGIC::CVar( var_name ) );
				it_var++;
			}
			addPostcondition( pattern );
			*/
		}
		
	}
}
