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

#include "logic/operator.h"

#include <list>
#include <vector>
#include "logic/ia_assert.h"
#include "logic/varset.h"
#include "logic/valueset.h"
#include "logic/fact.h"

namespace NLIALOGIC
{
	using namespace NLIAAGENT;

	IBaseOperator::IBaseOperator()
	{
		_Comment = NULL;
	}

	IBaseOperator::IBaseOperator(const char *c)
	{
		_Comment = new char[ strlen(c) ];
		strcpy( _Comment , c);
	}

	IBaseOperator::IBaseOperator(const IBaseOperator &c)
	{
		if ( c._Comment )
		{
			_Comment = new char[strlen(c._Comment)];
			strcpy(_Comment,c._Comment);
		}

		sint32 i;
		for ( i = 0; i < (sint32) c._Conds.size(); i++ )
			_Conds.push_back( c._Conds[i] );

		for ( i = 0; i < (sint32) c._Concs.size(); i++ )
			_Concs.push_back( c._Concs[i] );
	}

	IBaseOperator::~IBaseOperator()
	{
		if ( _Comment )
			delete[] _Comment;
	}

	/// Sets teh comment for the operator
	void IBaseOperator::setComment(char *c)
	{
		if ( _Comment )
			delete[] _Comment;

		if ( c ) 
		{
			_Comment = new char[ strlen( c ) ];
			strcpy( _Comment, c );
		}
		else
			_Comment = NULL;
	}

	const std::vector<IBaseAssert *> &IBaseOperator::getPrecondAsserts() const
	{
		return _Conds;
	}

	const std::vector<IBaseAssert *> &IBaseOperator::getPostCondAsserts() const
	{
		return _Concs;
	}

	void IBaseOperator::save(NLMISC::IStream &os)
	{
		sint32 i;
		sint32 size = (sint32) _Conds.size();
		os.serial( size );
		for ( i = 0; i < (sint32) _Conds.size(); i++ )
		{
			os.serial( (NLIAC::CIdentType &) _Conds[i]->getType() );
			_Conds[i]->save(os);
		}

		size = (sint32) _Concs.size();
		os.serial( size );
		for ( i = 0; i < (sint32) _Concs.size(); i++ )
		{
			os.serial( (NLIAC::CIdentType &) _Concs[i]->getType() );
			_Concs[i]->save(os);
		}
	}

	void IBaseOperator::load(NLMISC::IStream &is)
	{
		sint32 i;
		sint32 nb_vals;

		is.serial( nb_vals );
		for ( i = 0; i < nb_vals; i++ )
		{
			NLIAC::CIdentTypeAlloc id;
			is.serial( id );
			IBaseAssert *tmp_val = (IBaseAssert *) id.allocClass();
			tmp_val->load( is );
			tmp_val->incRef();
			_Conds.push_back( tmp_val );
		}

		is.serial( nb_vals );
		for ( i = 0; i < nb_vals; i++ )
		{
			NLIAC::CIdentTypeAlloc id;
			is.serial( id );
			IBaseAssert *tmp_val = (IBaseAssert *) id.allocClass();
			tmp_val->load( is );
			tmp_val->incRef();
			_Concs.push_back( tmp_val );
		}
	}
}