/** \file agent_nombre.h
 * template class for nomber manipulation.
 *
 * $Id: agent_nombre.h,v 1.3 2001/01/08 14:39:59 valignat Exp $
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
#ifndef NL_AGENT_NOMBRE_H
#define NL_AGENT_NOMBRE_H

#include "nel/ai/logic/boolval.h"

namespace NLAIAGENT
{	

	/**
	This define encapsulate a nomber and manage its manipulation.

	Nomber manibulation is need for the script and agent fonctionality.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	template <class tNombre> 
	class INombre: public IObjetOp
	{
	protected:
		tNombre _Value;

	public:
		INombre(tNombre value):_Value(value)
		{
		}
		INombre(const INombre &value):IObjetOp(value),_Value((tNombre)value._Value)
		{
		}
		INombre()
		{			
		}
		INombre(NLMISC::IStream &is)
		{
			load(is);
		}

		virtual ~INombre()
		{
		}

		virtual void save(NLMISC::IStream &os)
		{	
			tNombre v = _Value;
			// TODO
			os.serial( v );
		}

		virtual void load(NLMISC::IStream &is)
		{				
			// TODO
			is.serial( _Value );			

		}		

		virtual bool isEqual(const IBasicObjectIA &a) const
		{			
			return !(*(IObjectIA *)this == (IObjetOp &)a);
		}

		virtual bool isTrue() const
		{
			return _Value != (tNombre)0;
		}

		tNombre getValue() const
		{
			return _Value;
		}
		void setValue(tNombre value) 
		{
			_Value = value;
		}		

		virtual IObjectIA &operator = (const IObjectIA &a)
		{
			tNombre v((tNombre)((const INombre &)a).getValue());
			_Value = v;
			chekBorn();
			return *this;
		}

		virtual IObjetOp &operator += (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			addValue(v);
			return *this;
		}

		virtual IObjetOp &operator -= (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			subValue(v);
			return *this;
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}

		virtual IObjetOp &operator *= (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			mulValue(v);
			return *this;
		}

		virtual IObjetOp &operator /= (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			divValue(v);
			return *this;
		}

		virtual IObjetOp *operator + (const IObjetOp &a)  const
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o += a;
			return o;
		}

		virtual IObjetOp *operator - (const IObjetOp &a)  const
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o -= a;
			return o;
		}

		virtual const IObjetOp *operator * (const IObjetOp &a)  const
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o *= a;
			return o;
		}

		virtual const IObjetOp *operator / (const IObjetOp &a)  const
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o /= a;
			return o;
		}


		virtual IObjetOp *operator < (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value < v);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator > (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value > v);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator <= (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value <= v);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator >= (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value >= v);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator ! () const
		{			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value == (tNombre)0);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator != (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value != v);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator == (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value == v);
			x->incRef();
			return x;
		}

		
	protected:

		virtual void chekBorn(){}
		
		void addValue(tNombre value)
		{
			_Value += value;
			chekBorn();
		}

		void subValue(tNombre value)
		{
			_Value -= value;
			chekBorn();
		}

		void mulValue(tNombre value)
		{
			_Value *= value;
			chekBorn();
		}

		void divValue(tNombre value)
		{
			_Value /= value;
			chekBorn();
		}
	};

	template <class tNombre> 
	class  IBornNombre: public INombre<tNombre>
	{
	private:
		tNombre _inf,_sup;
	public:
		IBornNombre(tNombre value,tNombre inf,tNombre sup):INombre<tNombre>(value),_inf((tNombre)inf),_sup((tNombre)sup)
		{
		}
		IBornNombre(const IBornNombre &value):INombre<tNombre>(value),_inf((tNombre)value._inf),_sup((tNombre)value._sup)
		{
		}
		IBornNombre(tNombre inf,tNombre sup):_inf((tNombre)inf),_sup((tNombre)sup)
		{			
		}
		IBornNombre(NLMISC::IStream &is):INombre<tNombre>(is)
		{			
		}
		virtual ~IBornNombre()
		{
		}

		virtual void save(NLMISC::IStream &)
		{
/*			INombre<tNombre>::save(os);
			os.serial( _sup );
			os.serial(_inf );*/
		}

		virtual void load(NLMISC::IStream &)
		{
			// TODO
/*			INombre<tNombre>::load(is);
			is.serial( _sup );
			is.serial( _inf );*/
		}

		tNombre getSup() const
		{
			return _sup;
		}
		tNombre getInf() const
		{
			return _inf;
		}
	protected:

		void chekBorn()
		{
			if(_Value > _sup) _Value = _sup;
			if(_Value < _inf) _Value = _inf;
		}					
	};
}

#endif
