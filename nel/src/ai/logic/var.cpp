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

#include "nel/ai/logic/var.h"
#include "nel/ai/logic/boolval.h"

#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/constraint.h"
#include "nel/ai/script/type_def.h"


namespace NLAILOGIC
{
	using namespace NLAIAGENT;
	

	IBaseVar::IBaseVar(const IVarName &n)
	{
		_Name = (IVarName *) n.clone();
	}

	IBaseVar::IBaseVar(const char *n)	
	{
		_Name = new CStringVarName(n);
	}

	IBaseVar::IBaseVar(const IBaseVar &cp)
	{
		_Name = (IVarName *) cp._Name->clone();
	}


	TQueue IBaseVar::isMember(const IVarName *className, const IVarName *name, const IObjectIA &param) const
	{
		TQueue result = IObjetOp::isMember( className, name, param);

		if ( result.size() )
			return result;

		if ( *name == CStringVarName("set") )
		{
			IObjectIA *op_type = (IObjectIA *) new NLAISCRIPT::COperandVoid();
			result.push( CIdMethod(IObjetOp::getMethodIndexSize() + 1,0.0,NULL, op_type ) );
		}
		return result;
	}

	sint32 IBaseVar::getMethodIndexSize() const
	{
		return IObjetOp::getMethodIndexSize() + 1;
	}

	// Executes a method from its index id and with its parameters
	IObjectIA::CProcessResult IBaseVar::runMethodeMember(sint32 id, IObjectIA *param)
	{

		if ( id <= IObjetOp::getMethodIndexSize() )
			return IObjetOp::runMethodeMember(id, param);

		IObjetOp *x = (IObjetOp *) ((IBaseGroupType *) param)->getFront();
		((IBaseGroupType *)param)->popFront();

#ifdef NL_DEBUG
		std::string buf;
		x->getDebugString(buf);
		getDebugString(buf);
#endif

		if ( id == ( IObjetOp::getMethodIndexSize() + 1 ) )
		{
			setValue( x );
			IObjectIA::CProcessResult r;
			r.ResultState =  processIdle;
			r.Result = NULL;
			return r;
		}

		// TODO: throw exception....
		return IObjectIA::CProcessResult();
	}

	void IBaseVar::setName(IVarName &name)
	{
		if ( _Name )
			_Name->release();

		_Name = (IVarName *) name.clone();
	}

	IVarName &IBaseVar::getName() const
	{
		return *_Name;
	}

	IBaseVar::~IBaseVar()
	{
		if ( _Name )
			_Name->release();
	}

	CVar::CVar(const IVarName &n, IObjetOp *value) : IBaseVar( n )
	{
		_Value = value;

		if ( _Value )
			_Value->incRef();
	}

	CVar::CVar(const char *n, IObjetOp *value) : IBaseVar( n )
	{
		_Value = value;

		if ( _Value )
			_Value->incRef();
	}


	CVar::CVar(const CVar &cp) : IBaseVar( cp )
	{
		_Value = cp._Value;
		if ( _Value ) 
			_Value->incRef();
	} 

	CVar::~CVar()
	{
		if ( _Value )
			_Value->release();
	}

	void CVar::setValue(IObjetOp *obj)
	{
		if ( _Value )
			_Value->release();

		_Value = obj;
		_Value->incRef();
	}

	IObjetOp *CVar::getValue() const
	{
		return _Value;
	}
		
	const NLAIC::IBasicType *CVar::clone() const
	{
		NLAIC::IBasicInterface *m = new CVar( *this );
		return m;
	}

	const NLAIC::IBasicType *CVar::newInstance() const
	{
		return clone();
	}

	void CVar::save(NLMISC::IStream &os)
	{					
		IObjetOp::save( os );
		_Name->save( os );
		if ( _Value ) 
		{
			bool val = true;
			os.serial( (bool &) val );
			os.serial( (NLAIC::CIdentType &) _Value->getType() );
			_Value->save(os);
		}
		else
		{
			bool val = false;
			os.serial( (bool &) val );
		}
	}

	void CVar::load(NLMISC::IStream &is)
	{
		if ( _Value )
			_Value->release();
		IObjetOp::load(is);
		_Name->load(is);	
		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		_Value = (CVar *)id.allocClass();
		_Value->load(is);	
	}

	void CVar::getDebugString(std::string &text) const
	{
		std::string buf;
		if ( _Value )
			_Value->getDebugString( buf );
		else
			buf = "<undefined>";
		text += NLAIC::stringGetBuild("CVar<%s> = %s <%8x>\n",_Name->getString(), buf.c_str() , this);
	}

	bool CVar::isEqual(const CVar &a) const
	{
		return ( (*(IObjectIA *)a._Value) == (*_Value) );
	}

	const IObjectIA::CProcessResult &CVar::run()
	{			
		return IObjectIA::ProcessRun;
	}

	bool CVar::isEqual(const IBasicObjectIA &a) const
	{
		CVar &tmp = (CVar &) a;

		if( !_Value || !tmp._Value )
			return false;

		return ( (*(IObjectIA *)tmp._Value) == (*_Value) );
	}

	const NLAIC::CIdentType &CVar::getType() const
	{
		return IdVar;
	}

	bool CVar::operator==(IBaseVar *var)
	{
		return ( (CVar *) var )->_Value == _Value ;
	}

	bool CVar::unify(IBaseVar *y, bool assign)
	{	
		IObjetOp *x_val = _Value;
		IObjetOp *y_val = y->getValue();
		
		if ( !x_val && !y_val )
			return true;

		if ( x_val && y_val )
			if ( (*x_val) == (*y_val) )
				return true;
			else
				return false;
		else
		{
			if ( y_val && assign )
			{
				_Value = y_val;
				_Value->incRef();
			}
			return true;
		}
	}

	bool CVar::unify(IObjetOp *y_val, bool assign) {
		IObjetOp *x_val = _Value;
		
		if ( !x_val && !y_val )
			return true;

		if ( x_val && y_val )
			if ( (*x_val) == (*y_val) )
				return true;
			else
				return false;
		else
		{
			if ( y_val && assign )
			{
				_Value = y_val;
				_Value->incRef();
			}
			return true;
		}
	}

	bool CVar::isTrue() const
	{
		return false;
	}

	IObjetOp *CVar::operator == (IObjetOp &a) const
	{
		return new CBoolType( ((CVar &)a)._Value == _Value  );
	}
}
