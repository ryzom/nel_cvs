#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/actor_script.h"
#include "nel/ai/script/lexsupport.h"
#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/logic/operator_script.h"
#include "nel/ai/logic/fact.h"
#include "nel/ai/logic/factbase.h"

namespace NLAISCRIPT
{
	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n) : CAgentClass(n)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());		
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}
	
	COperatorClass::COperatorClass(const NLAIC::CIdentType &id): CAgentClass(id)
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const NLAIAGENT::IVarName &n, const NLAIAGENT::IVarName &inheritance) : CAgentClass( inheritance )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}

	COperatorClass::COperatorClass(const COperatorClass &c) : CAgentClass( c )
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));		
	}	

	COperatorClass::COperatorClass()
	{
		setBaseMethodCount(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::COperatorScript *)(NLAIAGENT::COperatorScript::IdOperatorScript.getFactory()->getClass())));
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

/*	std::list<NLAILOGIC::CValueSet *> *COperatorClass::propagate(std::list<NLAILOGIC::CValueSet *> &liaisons, NLAILOGIC::CValueSet *fact, std::vector<sint32> &pos_vals) 
	{
		std::list<NLAILOGIC::CValueSet *> *conflits = new std::list<NLAILOGIC::CValueSet *>;
		std::list<NLAILOGIC::CValueSet *> buf_liaisons;
		// Pour chaque liaison...
		std::list< NLAILOGIC::CValueSet *>::iterator it_l = liaisons.begin();
		
		while ( it_l != liaisons.end() )
		{

			NLAILOGIC::CValueSet *l = *it_l;
			char buf[512];
			l->getDebugString( buf );

			NLAILOGIC::CValueSet *result = unifyLiaison( l, fact, pos_vals );
			if ( result )
			{
				char buf[512];
				result->getDebugString( buf );

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

  */
/*	std::list<NLAILOGIC::CFact *> *COperatorClass::test(std::list<NLAILOGIC::CFact *> &facts)
	{
		std::list<NLAILOGIC::CFact *> *preconds = backward( facts );

		std::list<NLAILOGIC::CFact *>::iterator it_f = preconds->begin();
		while ( it_f != preconds->end() )
		{
			( (NLAILOGIC::CFirstOrderAssert *)(*it_f)->getAssert() )->backward( *it_f );
			it_f++;
		}
		return NULL;
	}
	*/

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

	void COperatorClass::setGoal(NLAILOGIC::CGoal *g)
	{
		if ( _Goal != NULL )
		{
			_Goal->release();
		}

		_Goal = g;

		if ( _Goal )
			_Goal->incRef();
	}

	const NLAILOGIC::CGoal *COperatorClass::getGoal()
	{
		return _Goal;
	}
}
