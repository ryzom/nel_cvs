/** \file agent_nombre.h
 * template class for nomber manipulation.
 *
 * $Id: agent_nombre.h,v 1.16 2002/06/06 09:26:56 portier Exp $
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

	/**
	This define the base of nomber manipulation.Nomber manibulation is need for the script and agent fonctionality.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class INombreDefine: public IObjetOp
	{

	public:

		///This enum define ident for hard coded method that we have to import its under the script.
		enum  TMethodNumDef {
			TConst, ///Index of the constructor method
			TLastM ///The count of import method.
		};

		///Structure to define the name ande the id of hard coded mathod.
		struct CMethodCall
		{
			CMethodCall(const char *name, int i): MethodName (name)
			{				
				Index = i;
			}
			///Name of the method.
			CStringVarName MethodName;
			///Index of the method in the class.
			sint32 Index;
		};
		///This variable its used to store method import characteristic.
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


		///All number can be read as a bouble, user ave to make the good cast to retreive the type of the number.
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

		virtual double getNumber() const = 0;		

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

		virtual IObjetOp &neg() = 0;		

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

		virtual IObjetOp *operator + (const IObjetOp &a)
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o += a;
			return o;
		}

		virtual IObjetOp *operator - (const IObjetOp &a)
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o -= a;
			return o;
		}

		virtual IObjetOp *operator * (const IObjetOp &a)
		{
			IObjetOp *o = (IObjetOp *)clone();			
			*o *= a;
			return o;
		}

		virtual IObjetOp *operator / (const IObjetOp &a)
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
			tNombre v = (tNombre)((const INombre &)a).getNumber();
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value <= v);			
			return x;
		}

		virtual IObjetOp *operator >= (IObjetOp &a) const
		{
			tNombre v = (tNombre)((const INombre &)a).getNumber();
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
	class IDigital : public INombre<tNombre>
	{

	public :
		IDigital(tNombre value):INombre<tNombre>(value)
		{
		}

		IDigital(const IDigital &value):INombre<tNombre>(value)
		{
		}
		
		IDigital()
		{			
		}
		
		IDigital(NLMISC::IStream &is):INombre<tNombre>(is)
		{
		}
		
		virtual ~IDigital()
		{
		}

		virtual double getNumber() const
		{
			return (double)_Value;
		}
	};

	template <class tNombre> 
	class  IBornNombre: public IDigital<tNombre>
	{
	private:
		tNombre _inf,_sup;
	public:
		IBornNombre(tNombre value,tNombre inf,tNombre sup):IDigital<tNombre>(value),_inf((tNombre)inf),_sup((tNombre)sup)
		{
		}
		IBornNombre(const IBornNombre &value):IDigital<tNombre>(value),_inf((tNombre)value._inf),_sup((tNombre)value._sup)
		{
		}
		IBornNombre(tNombre inf,tNombre sup):_inf((tNombre)inf),_sup((tNombre)sup)
		{			
		}
		IBornNombre(NLMISC::IStream &is):IDigital<tNombre>(is)
		{			
		}
		virtual ~IBornNombre()
		{
		}

		virtual void save(NLMISC::IStream &)
		{
/*			IDigital<tNombre>::save(os);
			os.serial( _sup );
			os.serial(_inf );*/
		}

		virtual void load(NLMISC::IStream &)
		{
			// TODO
/*			IDigital<tNombre>::load(is);
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
