/** \file fuzzyvar.cpp
 * Fuzzy controler class for the scripting language
 *
 * $Id: fuzzyvar.cpp,v 1.10 2002/08/21 13:58:34 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/ai/fuzzy/fuzzyvar.h"
#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/constraint.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/fuzzy/fuzzycond.h"

namespace NLAIFUZZY 
{	
	using namespace NLAIAGENT;

	CFuzzyVar::CFuzzyVar(const NLAIAGENT::IVarName &name, double min, double max, NLAIAGENT::IObjetOp *value) : IBaseVar(name)
	{
		if ( value )
			_Value = ((NLAIAGENT::INombre<double> *)value)->getValue();
		else
			_Value = 0;

		_Min = min;
		_Max = max;
	}

	CFuzzyVar::CFuzzyVar(const NLAIAGENT::IVarName &name, double min, double max, double value) : IBaseVar(name)
	{
		_Value = value;
		_Min = min;
		_Max = max;
	}

	CFuzzyVar::CFuzzyVar(const CFuzzyVar &cp) : IBaseVar( cp )
	{
		_Value = cp._Value;
		_Min = cp._Min;
		_Max = cp._Max;
	}

	CFuzzyVar::~CFuzzyVar()
	{
		
		std::vector<IFuzzySet *>::const_iterator it_fs = _sets.begin();
		while ( it_fs != _sets.end() )
		{
			( *it_fs )->release();
			it_fs++;
		}

		std::vector<CFuzzyFact *>::const_iterator it_f = _facts.begin();
		while ( it_f != _facts.end() )
		{
			( *it_f )->release();
			it_f++;
		}

		std::vector<NLAIAGENT::CStringVarName *>::const_iterator it_q = _qual.begin();
		while ( it_q != _qual.end() )
		{
			( *it_q )->release();
			it_q++;
		}
	}

	void CFuzzyVar::addSet(IFuzzySet *my_set, char *set_name)
	{
		_sets.push_back(my_set);
		my_set->incRef();
		NLAIAGENT::CStringVarName *qual = new NLAIAGENT::CStringVarName( set_name );
		_qual.push_back( qual );
	}

	std::list<CFuzzyFact *> *CFuzzyVar::fuzzify(double val)
	{
		std::list<CFuzzyFact *> *result = new std::list<CFuzzyFact *>;
		for ( sint32 i = 0; i < (sint32) _sets.size(); i++ )
		{
			if ( _sets[i]->isIn( val ) )
			{
				result->push_back( new CFuzzyFact( _sets[i], _sets[i]->membership(val) ) );
			}
		}
		return result;
	}

	double CFuzzyVar::unfuzify()
	{
		double total_membership = 0.0;
		double set_membership;
		double set_center;

		_Value = 0.0;
		for( sint32 i = 0; i < (sint32) _sets.size() ; i++ )
		{
			set_membership = _sets[i]->agregate();
			if ( set_membership )
			{
				set_center = _sets[i]->center();
				_Value = _Value + set_membership * set_center;
				total_membership = total_membership + set_membership;
			}
		}

		if ( total_membership > 0 )
			_Value = _Value / total_membership;
		else 
			_Value = 0;

		return _Value;
	}

	void CFuzzyVar::addFact(CFuzzyFact *f)
	{
		_facts.push_back(f);
	}

	IFuzzySet *CFuzzyVar::getSet(char *name)
	{
		std::vector<IFuzzySet *>::iterator it_set = _sets.begin();
		while ( it_set != _sets.end() )
		{
			if ( !strcmp( (*it_set) ->getName(), name) )
				return *it_set;
			it_set++;
		}
		return NULL;
	}

	void CFuzzyVar::addFact(char *set_name, double membership)
	{
		IFuzzySet *set = getSet( set_name );
		if ( set )
			_facts.push_back( new CFuzzyFact(set, membership) );
	}

	void CFuzzyVar::setValue(NLAIAGENT::IObjetOp *obj)
	{
		_Value = ((NLAIAGENT::INombre<double> *)obj)->getValue();
	}

	void CFuzzyVar::setValue(float value)
	{
		_Value = value;
	}

	NLAIAGENT::IObjetOp *CFuzzyVar::getValue() const
	{
		return new NLAIAGENT::DigitalType( (float) _Value );
	}

	const NLAIC::IBasicType *CFuzzyVar::clone() const
	{
		CFuzzyVar *clone = new CFuzzyVar(*this);
		return clone;
	}

	const NLAIC::IBasicType *CFuzzyVar::newInstance() const
	{      
	  NLAIAGENT::CStringVarName x("Inst");
	  CFuzzyVar *instance = new CFuzzyVar(x,0,0);
	  return instance;
	}

	void CFuzzyVar::save(NLMISC::IStream &os)
	{
		IObjectIA::save(os);
		os.serial( (double &) _Value );
		os.serial( (double &) _Min );
		os.serial( (double &) _Max );
		sint32 size = _sets.size();
		os.serial( size );
		for (sint32 i = 0; i < size; i++ )
		{
			_sets[i]->save(os);
		}
	}

	void CFuzzyVar::load(NLMISC::IStream &is)
	{
		IObjectIA::load( is );

		is.serial( _Value );
		is.serial( _Min );
		is.serial( _Max );

		sint32 nb_sets;
		is.serial( nb_sets );
		for (sint32 i = 0; i < nb_sets; i++ )
		{
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			IFuzzySet *tmp_val = (IFuzzySet *) id.allocClass();
			tmp_val->load( is );
			_sets.push_back( tmp_val );
		}
	}
 
	void CFuzzyVar::getDebugString(std::string &txt) const
	{
		txt += NLAIC::stringGetBuild("CFuzzyVar<%s> = %f\n", getName().getString(), _Value);
		for(sint32 i = 0; i < (sint32) _sets.size(); i++)
		{
			if ( _sets[i]->isIn( _Value ) )
			{
				std::string buf;				
				_sets[i]->getDebugString(buf);
				txt += NLAIC::stringGetBuild("  %s %f\n", buf.c_str(), _sets[i]->membership( _Value ) );				
			}
		}
	}

	bool CFuzzyVar::isEqual(const CFuzzyVar &a) const
	{
		return _Value == a._Value;
	}

	bool CFuzzyVar::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return _Value == ((CFuzzyVar &)a)._Value;
	}

	bool CFuzzyVar::isTrue() const
	{
		return false;
	}

	const NLAIC::CIdentType &CFuzzyVar::getType() const
	{
		return IdFuzzyVar;
	}

	bool CFuzzyVar::operator==(NLAILOGIC::IBaseVar *var)
	{
		return false;// _Value = var->getValue()
	}

	bool CFuzzyVar::unify(NLAILOGIC::IBaseVar *, bool assign)
	{
		return false;
	}

	bool CFuzzyVar::unify(NLAIAGENT::IObjetOp *, bool assign)
	{
		return false;
	}

	NLAIAGENT::IObjetOp *CFuzzyVar::operator == (NLAIAGENT::IObjetOp &a) const
	{
		return NULL;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CFuzzyVar::run()
	{
		return IObjectIA::ProcessRun;
	}

	bool CFuzzyVar::isIn(IFuzzySet *my_set)
	{
		return my_set->isIn(_Value);
	}

	float CFuzzyVar::membership(IFuzzySet *my_set)
	{
		return (float) my_set->membership( _Value);
	}

	CSimpleFuzzyCond *CFuzzyVar::getCond(char *name)
	{
		IFuzzySet *my_set = getSet( name );
		if ( my_set )
		{
			return new CSimpleFuzzyCond( this, my_set );
		}
		else
			return NULL;
	}

	sint32 CFuzzyVar::getMethodIndexSize() const
	{
		return IObjetOp::getMethodIndexSize() + 2;
	}

	// Executes a method from its index id and with its parameters
	IObjectIA::CProcessResult CFuzzyVar::runMethodeMember(sint32 id, IObjectIA *params)
	{
		if ( id <= IBaseVar::getMethodIndexSize() )
			return IBaseVar::runMethodeMember(id, params);

		IObjectIA::CProcessResult r;

		std::string buf;
		NLAIAGENT::IObjetOp *x= NULL;
		if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
		{
			( ((NLAIAGENT::IBaseGroupType *)params))->popFront();

#ifdef NL_DEBUG
			x->getDebugString(buf);
#endif
		}
#ifdef NL_DEBUG
		getDebugString(buf);
#endif

		sint32 base = IBaseVar::getMethodIndexSize();
		sint32 op_add_subset = base+1;
		sint32 op_unfuzify	= base+2;
		
		if ( id == op_add_subset )
		{
			addSet( (IFuzzySet *) x, ((IFuzzySet *)x)->getName());
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}


		if ( id == op_unfuzify )
		{
			unfuzify();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = new NLAIAGENT::DigitalType( (float) _Value);
		}

		// TODO: throw exception....
		return r;
	}

	IObjectIA::CProcessResult CFuzzyVar::runMethodeMember(sint32 inheritance, sint32 id, IObjectIA *params)
	{
		return runMethodeMember(id,params);
	}


	void CFuzzyVar::init(IObjectIA *p)
	{
		NLAIAGENT::IBaseGroupType *params = ((NLAIAGENT::IBaseGroupType *)p);

		if ( params->size() < 2 ) 
		{
		//	throw Exc::
		}

		// Nom
		IObjectIA * arg = (IObjectIA *) params->popFront();
		strcpy(_Name, ((NLAIAGENT::IVarName *) arg)->getString() );
		
		// Sous ensembles
		while (	 params->size() )
		{
			IFuzzySet *set = (IFuzzySet *) params->getFront()->clone();
			addSet( set, set->getName() );
			params->popFront();
		}
	}


	tQueue CFuzzyVar::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *name,const IObjectIA &param) const
	{
		tQueue result;

		result = IBaseVar::isMember(className, name, param);

		if ( result.size() )
			return result;

		if(className != NULL) 
		{
			if ( *name == CStringVarName( _ADDSUBSET_ ) )
			{
				IObjectIA *op_type = (IObjectIA *) new NLAISCRIPT::COperandVoid();
				result.push( NLAIAGENT::CIdMethod(1 + IObjetOp::getMethodIndexSize(), 0.0,NULL, op_type ) );
			}
			if ( *name == CStringVarName(_UNFUZIFY_) )
			{
				IObjectIA *op_type = (IObjectIA *) new NLAISCRIPT::COperandSimple( new NLAIC::CIdentType( DigitalType::IdDigitalType) );
				result.push( NLAIAGENT::CIdMethod(2 + IObjetOp::getMethodIndexSize(), 0.0, NULL, op_type ) );
			}
		}
		return result;
	}
}
