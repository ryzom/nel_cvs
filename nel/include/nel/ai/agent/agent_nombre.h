/** \file agent_nombre.h
 * template class for nomber manipulation.
 *
 * $Id: agent_nombre.h,v 1.6 2001/01/17 16:53:18 chafik Exp $
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

//#include "nel/ai/agent/agent_object.h"
#include "nel/ai/logic/boolval.h"
#include "nel/misc/vector.h"

namespace NLAIAGENT
{	


	class INombreDefine: public IObjetOp
	{

	public:

		enum  TMethodNumDef {
			TConst,
			TLastM
		};

		struct CMethodCall
		{
			CMethodCall(const char *name, int i): MethodName (name)
			{				
				Index = i;
			}
			CStringVarName MethodName;
			sint32 Index;
		};
		static CMethodCall StaticMethod[];

	public:
		INombreDefine()
		{
		}

		INombreDefine(const INombreDefine &value):IObjetOp(value)
		{
		}

		///\name Some IObjectIA method definition.
		//@{
		virtual sint32 getMethodIndexSize() const;
		virtual sint32 isClassInheritedFrom(const IVarName &) const;
		virtual tQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;		
		//@}


		virtual double getNumber() const = 0;
		virtual ~INombreDefine()
		{
		}

	};

	/**
	This define encapsulate a nomber and manage its manipulation.

	Nomber manibulation is need for the script and agent fonctionality.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	template <class tNombre> 
	class INombre: public INombreDefine
	{	

	protected:
		tNombre _Value;

	public:
		INombre(tNombre value):_Value(value)
		{
		}
		INombre(const INombre &value):INombreDefine(value),_Value((tNombre)value._Value)
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

		virtual double getNumber() const
		{
			return (double)_Value;
		}

		///\name Some IObjectIA method definition.
		//@{		
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *)
		{
			return IObjectIA::CProcessResult();
		}
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *p)
		{
			IBaseGroupType *param = (IBaseGroupType *)p;

			switch(index - IObjetOp::getMethodIndexSize())
			{
			case INombreDefine::TConst:
				{					
					INombre *o = (INombre *)param->get();
					_Value = (tNombre )((INombreDefine *)param->get())->getNumber();
					return IObjectIA::CProcessResult();		
				}
			}

			return IObjectIA::CProcessResult();
		}
		//@}

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
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			_Value = v;
			chekBorn();
			return *this;
		}

		virtual IObjetOp &operator += (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			addValue(v);
			return *this;
		}

		virtual IObjetOp &operator -= (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
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
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			mulValue(v);
			return *this;
		}

		virtual IObjetOp &operator /= (const IObjetOp &a)
		{
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
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
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value < v);			
			return x;
		}

		virtual IObjetOp *operator > (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value > v);			
			return x;
		}

		virtual IObjetOp *operator <= (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value <= v);			
			return x;
		}

		virtual IObjetOp *operator >= (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getValue();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value >= v);			
			return x;
		}

		virtual IObjetOp *operator ! () const
		{			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value == (tNombre)0);			
			return x;
		}

		virtual IObjetOp *operator != (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value != v);
			return x;
		}

		virtual IObjetOp *operator == (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombreDefine &)a).getNumber();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value == v);
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

	using NLMISC::CVector;
	/**
	This define encapsulate a vector and manage its manipulation.

	Vector manibulation is need for the script and agent fonctionality.
	* \author Chafik sameh	 	
	* \author Saffray alain
	* \author Nevrax France
	* \date 2001
	*/
	class IVector: public IObjetOp
	{
	protected:
		CVector _Value;

	public:
		IVector(float x, float y, float z):_Value(x, y, z)
		{
		}
		IVector(const IVector &value):IObjetOp(value), _Value((CVector)value._Value)
		{
		}
		IVector()
		{			
		}
		IVector(NLMISC::IStream &is)
		{
			load(is);
		}

		virtual ~IVector()
		{
		}

		virtual void save(NLMISC::IStream &os)
		{	
			os.serial( _Value );
		}

		virtual void load(NLMISC::IStream &is)
		{				
			is.serial( _Value );
		}		

		virtual bool isEqual(const IBasicObjectIA &a) const
		{			
			return !(*(IObjectIA *)this == (IObjetOp &)a);
		}

		virtual bool isTrue() const
		{
			return true;
		}

		CVector getValue() const
		{
			return _Value;
		}

		void setValue(CVector value) 
		{
			_Value = value;
		}		

		float x() const
		{
			return _Value.x;
		}

		float y() const
		{
			return _Value.y;
		}

		float z() const
		{
			return _Value.z;
		}

		void x(const float u)
		{
			_Value.x = u;
		}

		void y(const float v)
		{
			_Value.y = v;
		}

		void z(const float w)
		{
			_Value.z = w;
		}

		float squareLength()
		{
			return _Value.x * _Value.x + _Value.y * _Value.y + _Value.z * _Value.z;
		}

		float length()
		{
			return (float) sqrt(squareLength());
		}

		void unit()
		{
			_Value.normalize();
		}

		virtual IObjectIA &operator = (const IObjectIA &a)
		{
			CVector v((CVector)((const IVector &)a).getValue());
			_Value = v;
			return *this;
		}

		virtual IObjetOp &operator += (const IObjetOp &a)
		{
			CVector v = (CVector)((const IVector &)a).getValue();
			_Value += _Value;
			return *this;
		}

		virtual IObjetOp &operator -= (const IObjetOp &a)
		{
			CVector v = (CVector)((const IVector &)a).getValue();
			_Value -= _Value;
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

		virtual IObjetOp *operator != (IObjetOp &a) const
		{
			CVector v = (CVector)((const IVector &)a).getValue();			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value != v);
			x->incRef();
			return x;
		}

		virtual IObjetOp *operator == (IObjetOp &a) const
		{
			CVector v = (CVector)((const IVector &)a).getValue();			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value == v);
			x->incRef();
			return x;
		}

		virtual sint32 getMethodIndexSize() const;
		virtual tQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);
		virtual sint32 isClassInheritedFrom(const IVarName &) const;
	protected:

	};
}

#endif
