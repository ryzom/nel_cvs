/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX <MODULE_NAME>.
 * NEVRAX <MODULE_NAME> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX <MODULE_NAME> is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX <MODULE_NAME>; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "logic/valueset.h"
#include "logic/var.h"
#include "logic/boolval.h"
#include "logic/clause.h"
#include "logic/ia_assert.h"

namespace NLAILOGIC
{
	using namespace NLAIAGENT;

	const TProcessStatement CValueSet::state = processIdle;

	// Construit un CValueSet de taille <size> avec des valeurs initialisées à NULL
	CValueSet::CValueSet(sint32 size)
	{
		_NbValues = size;
		_Values = new IObjetOp *[ _NbValues ];

		for ( sint32 i = 0; i < _NbValues; i++ )
		{
			_Values[ i ] = NULL;
		}
	}

	// Constructeur de copie
	CValueSet::CValueSet(const CValueSet &cp)
	{
		_NbValues = cp._NbValues;

		_Values = new IObjetOp *[ _NbValues ];

		for ( sint32 i = 0; i < _NbValues; i++ )
		{
			_Values[i] = (IObjetOp *) cp._Values[i];

			if ( _Values[i] )
				_Values[i]->incRef();
		}
	}

	// Construit un CValueSet à partir des valeurs d'une liste de variables
	CValueSet::CValueSet(std::vector<IBaseVar *> &vars)
	{
		_NbValues = vars.size();

		_Values = new IObjetOp *[ _NbValues ];

		for (sint32 i = 0; i < _NbValues; i++ )
		{
			_Values[ i ] = 	vars[i]->getValue();

			if ( _Values[i] )
				_Values[i]->incRef();
		}
	}

	// Construit un valueSet à partir d'un autre en changeant la position des valeurs:
	// pos = positions dans le nouveau CValueSet des valeurs du premier.
	CValueSet::CValueSet(CValueSet *vals, std::vector<sint32> &pos)
	{
		_NbValues = pos.size();

		_Values = new IObjetOp *[ _NbValues ];

		for (sint32 i = 0; i < _NbValues; i++ )
		{
			_Values[ i ] = 	NULL;
		}

		std::vector<sint32>::iterator it_pos = pos.begin();
		sint32 index = 0;
		while ( it_pos != pos.end() )
		{
			_Values[ index ] = vals->_Values[ *it_pos ];
			it_pos++;
			index++;
		}
	}

	// Construit un CValueSet de taille <size> à partir d'une liste de valeurs et de leurs positions
	CValueSet::CValueSet(sint32 size, std::list<IObjetOp *> *vals, std::vector<sint32> &pos)
	{
		_NbValues = size;

		_Values = new IObjetOp *[ _NbValues ];

		for (sint32 i = 0; i < _NbValues; i++ )
		{
			_Values[ i ] = 	NULL;
		}

		std::list<IObjetOp *>::iterator it_val = vals->begin();
		std::vector<sint32>::iterator it_pos = pos.begin();

		while ( it_val != vals->end() && it_pos != pos.end() )
		{
			_Values[ *it_pos ] = *it_val;
			(*it_val)->incRef();
			it_val++;
			it_pos++;
		}
	}

	CValueSet::~CValueSet()
	{
		for ( sint32 i = 0; i < _NbValues; i++ )
			if ( _Values[i] )
				_Values[i]->release();
		delete[] _Values;
	}

	void CValueSet::setValue(sint32 pos, IObjetOp *obj)
	{
		_Values[pos] = obj;
		if ( obj )
			obj->incRef();
	}

	// Unification d'un CValueSet ave un autre.
	// Renvoie NULL si l'unification échoue, 
	// Un nouveau CValueSet corrrespondant à la l'unification si réussit
	CValueSet *CValueSet::unify(const CValueSet *un) const
	{
		CValueSet *result = new CValueSet( _NbValues );
		result->incRef();
		int i;
		for( i = 0; i < _NbValues; i++ )
		{
			result->_Values[i] = _Values[i];
			if ( _Values[i] )
				_Values[i]->incRef();

		}
		
		for (i = 0; i < _NbValues ; i++ )
		{
			IObjetOp *x_val = result->_Values[ i ];
			IObjetOp *y_val = un->_Values[ i ];
	
/*			if ( !x_val && !y_val )
				result->_Values[i] = NULL ;
*/
			if ( x_val && y_val )
			{
				IObjetOp *test;
				if ( !( test = (*x_val) != ( *y_val ) )->isTrue() )
				{
					// Nothing to do 
				}						
				else
				{
					result->release();
					return NULL;
				}
				test->release();
			}
			else
			{
				if ( y_val )
				{
					result->_Values[i] = y_val;
					result->_Values[i]->incRef();
				}
			}
		}
		return result;
	}

	// Unifie un CValueSet avec une liste de valeurs et leurs positions
	CValueSet *CValueSet::unify(std::list<IObjetOp *> *vals, std::vector<sint32> &pos_vals) const
	{
		CValueSet *result = new CValueSet( _NbValues );
		result->incRef();
		for (sint32 i = 0; i < _NbValues; i++ )
		{
			result->_Values[i] = _Values[i];
			if ( _Values[i] )
				_Values[i]->incRef();

		}

		std::list<IObjetOp *>::iterator it_val = vals->begin();
		std::vector<sint32>::iterator it_pos = pos_vals.begin();

		// -----------------

		char buf[512];
		this->getDebugString(buf);
		char buf2[512];
		CValueSet tmp(_NbValues, vals, pos_vals);
		tmp.getDebugString(buf2);
		//TRACE(" \nUNIFICATION:\n VS = %s\n VP = %s \n", buf, buf2);
		
		// -----------------

		while ( it_val != vals->end()  && it_pos != pos_vals.end() )
		{
			sint32 pos = *it_pos;
			IObjetOp *l_val = _Values[ pos ];
			IObjetOp *r_val = *it_val;
			IObjetOp *test = NULL;
			if (  !l_val || !( test = (*l_val) != ( **it_val ) )->isTrue() )
			{
				if ( test )
					test->release();
				result->_Values[ pos ] = *it_val;
				result->_Values[ pos ]->incRef();
			}
			else
			{
				if ( test )
					test->release();
				delete result;
				return NULL;
			}
			it_val++;
			it_pos++;

		}

		return result;
	}

	CValueSet *CValueSet::unify(CValueSet *vals, std::vector<sint32> &pos_vals) const
	{
		CValueSet *result = new CValueSet( _NbValues );
		result->incRef();
		int i;
		for ( i = 0; i < _NbValues; i++ )
		{
			result->_Values[i] = _Values[i];
			if ( _Values[i] )
				_Values[i]->incRef();

		}

		for (i = 0; i < (sint32) pos_vals.size(); i++ )
		{
			sint32 pos = pos_vals[i];
			IObjetOp *l_val = _Values[ pos ];
			IObjetOp *r_val = (*vals)[i];
			IObjetOp *test = NULL;
			if (  !l_val || !( test = (*l_val) != ( *r_val ) )->isTrue() )
			{
				if ( test )
					test->release();
				if ( !l_val )
				{
					result->_Values[ pos ] = r_val;
					if ( (*vals)[i] )
						result->_Values[ pos ]->incRef();
				}
			}
			else
			{
				if ( test )
					test->release();
				result->release();
				return NULL;
			}
		}
		return result;
	}


	bool CValueSet::operator==(CValueSet *cp)
	{
		if( cp->_NbValues != _NbValues )
			return false;

		for ( sint32 i = 0; i < _NbValues; i++ )
			if ( cp->_Values[i] != _Values[i] )
				return false;
		return true;
	}

	IObjetOp *CValueSet::operator[](sint32 pos)
	{
		if ( pos <= _NbValues )
			return _Values[pos];	
		else
			return NULL;	// TODO: exception!!!!!!!!!!!!
	}

	IObjetOp *CValueSet::getValue(sint32 pos)
	{
		if ( pos <= _NbValues )
			return _Values[pos];	
		else
			return NULL;	// TODO: exception!!!!!!!!!!!!
	}

	// Renvoie le nombre de valeurs à NULL ( considérées comme indéfinies)
	sint32 CValueSet::undefined() const
	{
		sint32 nb_undef = _NbValues;
		for ( sint32 i = 0; i < _NbValues; i++ )
			if ( _Values[i] != NULL )
				nb_undef--;
		return nb_undef;
	}

	sint32 CValueSet::size()
	{
		return _NbValues;
	}

	// Renvoie une liste des valeurs non nulles du CValueSet
	std::list<IObjetOp *> *CValueSet::getValues()
	{
		// Attenrtion envoie une liste à détruire et sans faire de incRef();
		std::list<IObjetOp *> *result = new std::list<IObjetOp *>;
		for (sint32 i = 0; i < _NbValues; i++ )
		{
			if ( _Values[i] )
				result->push_back( _Values[i] );
		}
		return result;
	}

	const NLAIC::IBasicType *CValueSet::clone() const
	{
		NLAIC::IBasicType *tmp = new CValueSet( *this );
		tmp->incRef();
		return tmp;
	}

	const NLAIC::IBasicType *CValueSet::newInstance() const
	{
		return clone();
	}

	const NLAIC::CIdentType &CValueSet::getType() const
	{
		return IdValueSet;
	}

	void CValueSet::save(NLMISC::IStream &os)
	{
		sint32 nb_Values = (sint32) _NbValues;
		os.serial( nb_Values );
		std::list<IObjetOp *> values;
		std::vector<sint32> pos;

		for ( sint32 i = 0; i < _NbValues; i++ )
		{
			if ( _Values[i] )
			{
				values.push_back( _Values[i] );
				pos.push_back( i );
			}
		}
		sint32 size = (sint32) values.size();
		os.serial( size );

		std::list<IObjetOp *>::iterator it_val = values.begin();
		std::vector<sint32>::iterator it_pos = pos.begin();

		while ( it_val != values.end() )
		{
			os.serial( (NLAIC::CIdentType &) ( *it_val )->getType() );
			( *it_val )->save( os );
			sint32 ip = (sint32) *it_pos;
			os.serial( ip );
		}
	}

	void CValueSet::load(NLMISC::IStream &is)
	{
		// Détruit l'ancien tableau
		int i;
		for ( i = 0; i < _NbValues; i++ )
			if ( _Values[i] )
				_Values[i]->release();
		delete[] _Values;
		
		// En créé un nouveau
		sint32 nb_Values;
		is.serial( nb_Values );
		_NbValues = (sint32) nb_Values;
		sint32 nbvals;
		is.serial( nbvals );
		
		_Values = new IObjetOp *[ _NbValues ];

		for ( i = 0; i < _NbValues; i++ )
		{
			_Values[ i ] = NULL;
		}

		std::list<IObjetOp *> vals;
		std::vector<sint32> pos;

		for ( i = 0; i < nbvals; i++ )
		{
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			IObjetOp *tmp_val = (IObjetOp *) id.allocClass();
			tmp_val->load( is );
			tmp_val->incRef();
			vals.push_back(  tmp_val );
		}
	}

	void CValueSet::getDebugString(char *txt) const
	{
		char buf[512];
		strcpy(txt," <CValueSet> ");
		for (sint32 i = 0; i < _NbValues; i++ )
		{
			strcat( txt, " , ");
			if ( _Values[i] )
			{
				_Values[i]->getDebugString( buf );

				strcat( txt, buf );
			}
			else strcat( txt, "NULL");
		}
	}

	IObjetOp *CValueSet::operator ! () const
	{
		CBoolType *result = new CBoolType ( !isTrue() );
		result->incRef();
		return result;
	}

	IObjetOp *CValueSet::operator != (IObjetOp &a) const
	{
		if ( _NbValues != ((CValueSet &)a)._NbValues )
		{
			CBoolType *result = new CBoolType ( false );
			result->incRef();
			return result;
		}


		for (sint32 i = _NbValues; i < _NbValues; i++ )
		{
			if ( _Values[i] != ((CValueSet &)a)._Values[i] )
			{
				CBoolType *result = new CBoolType ( false );
				result->incRef();
				return result;
			}

		}
		CBoolType *result = new CBoolType ( true );
		result->incRef();
		return result;
	}

	IObjetOp *CValueSet::operator == (IObjetOp &a) const
	{
		if ( _NbValues != ((CValueSet &)a)._NbValues )
		{
			CBoolType *result = new CBoolType ( false );
			result->incRef();
			return result;
		}

		for (sint32 i = 0; i < _NbValues; i++ )
		{
			IObjetOp *test = (*_Values[i]) != *((CValueSet &)a)._Values[i];
			bool test_result = test->isTrue();
			test->release();
			if ( test_result )
			{
				CBoolType *result = new CBoolType ( false );
				result->incRef();
				return result;
			}
		}
		CBoolType *result = new CBoolType ( true );
		result->incRef();
		return result;
	}

	bool CValueSet::isEqual(const IBasicObjectIA &cmp) const
	{
		if ( _NbValues != ((CValueSet &)cmp)._NbValues )
		{
			return false;
		}

		for (sint32 i = _NbValues; i < _NbValues; i++ )
		{
			if ( _Values[i] != ((CValueSet &)cmp)._Values[i] )
			{
				return false;
			}
		}
		return true;
	}

	bool CValueSet::isTrue() const
	{
		return ( undefined() == (sint32) 0 );
	}

	const IObjectIA::CProcessResult &CValueSet::run()
	{
		return IObjectIA::ProcessRun;
	}

	CValueSet *CValueSet::forward(CClause *clause,std::vector<sint32> &pos)
	{
		CValueSet *result = new CValueSet( clause->nbVars() );

		std::vector<sint32>::iterator it_pos = pos.begin();

		for (sint32 i = 0; i < _NbValues; i++ )
		{
			if ( _Values[i] )
			{
				result->_Values[ *it_pos ] = _Values[i];
				_Values[i]->incRef();
				it_pos++;
			}
		}
		return result;
	}
}
