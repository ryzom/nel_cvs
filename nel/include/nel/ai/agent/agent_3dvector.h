/** \file agent_3dvector.h
 * <File description>
 *
 * $Id: agent_3dvector.h,v 1.13 2003/02/05 16:05:53 chafik Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_AGENT_3DVECTOR_H
#define NL_AGENT_3DVECTOR_H

#include "nel/misc/types_nl.h"
#include "nel/ai/logic/boolval.h"
#include "nel/misc/vectord.h"


//using NLMISC::CVectorD;


namespace NLAIAGENT
{	
	/**
	 * This define an vector and manage its manipulation.
	 * Vector manibulation is need for the script and agent fonctionality.
	 * \author Alain SAFFRAY
	 * \author Guillaume PUZIN
	 * \author Nevrax France
	 * \date 2000
	 */
	class IVector: public IObjetOp
	{
	protected:
		NLMISC::CVectorD _Value;

	public:

		/// Constructor
		IVector(double x, double y, double z):_Value(x, y, z)
		{
		}

		IVector(const NLMISC::CVectorD &v):_Value(v)
		{			
		}

		IVector(const IVector &value):IObjetOp(value), _Value((NLMISC::CVectorD)value._Value)
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

		const NLMISC::CVectorD &getValueRef() const
		{
			return _Value;
		}

		NLMISC::CVectorD getValue() const
		{
			return _Value;
		}

		void setValue(NLMISC::CVectorD value) 
		{
			_Value = value;
		}		

		double x() const
		{
			return _Value.x;
		}

		double y() const
		{
			return _Value.y;
		}

		double z() const
		{
			return _Value.z;
		}

		void x(const double u)
		{
			_Value.x = u;
		}

		void y(const double v)
		{
			_Value.y = v;
		}

		void z(const double w)
		{
			_Value.z = w;
		}

		double squareLength()
		{
			return _Value.x * _Value.x + _Value.y * _Value.y + _Value.z * _Value.z;
		}

		double length()
		{
			return (double) sqrt(squareLength());
		}

		void unit()
		{
			_Value.normalize();
		}

		virtual IObjectIA &operator = (const IObjectIA &a)
		{
 			NLMISC::CVectorD v((NLMISC::CVectorD)((const IVector &)a).getValue());
			_Value = v;
			return *this;
		}

		virtual IObjetOp &operator += (const IObjetOp &a)
		{
			NLMISC::CVectorD v = (NLMISC::CVectorD)((const IVector &)a).getValue();
			_Value += v;
			return *this;
		}

		virtual IObjetOp &operator -= (const IObjetOp &a)
		{
			NLMISC::CVectorD v = (NLMISC::CVectorD)((const IVector &)a).getValue();
			_Value -= v;
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

		virtual IObjetOp *operator != (IObjetOp &a) const
		{
			NLMISC::CVectorD v = (NLMISC::CVectorD)((const IVector &)a).getValue();			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value != v);			
			return x;
		}

		virtual IObjetOp *operator == (IObjetOp &a) const
		{
			NLMISC::CVectorD v = (NLMISC::CVectorD)((const IVector &)a).getValue();			
			NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(_Value == v);			
			return x;
		}

		virtual sint32 getMethodIndexSize() const;
		virtual TQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);
		virtual sint32 isClassInheritedFrom(const IVarName &) const;
	};

	/**	
	This class store a vector<double,double,double> in an IObjectIA.

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Saffray Alain
	* \author Nevrax France
	* \date 2001
	*/
	class VectorType: public IVector
	{
	public:

		static const NLAIC::CIdentType IdVectorType;

	public:
		VectorType(double x, double y, double z):IVector(x, y, z)
		{			
		}

		VectorType(const NLMISC::CVectorD &v):IVector(v)
		{			
		}

		VectorType():IVector()
		{			
		}

		VectorType(const VectorType &a):IVector(a)
		{			
		}

		VectorType(NLMISC::IStream &is):IVector(is)
		{
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new VectorType(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text = NLAIC::stringGetBuild("Vector <%f %f %f>",getValue().x, getValue().y, getValue().z);
		}		

		virtual const CProcessResult &run();
	};
}


#endif // NL_AGENT_3DVECTOR_H

/* End of agent_3dvector.h */
