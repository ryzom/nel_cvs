
/** \file varstack.h
 *
 * $Id: varstack.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_VAR_STACK_H
#define NL_VAR_STACK_H


namespace NLIASCRIPT
{
	/**
	* Class CVarPStack.
	* 
	* This class define an object manipulate by an heap pointer index. In fact local variable is translate in an index value,this index is a pointer heap index.
	* 
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class CVarPStack:public NLIAAGENT::IObjetOp
	{

	public:
		static NLIAC::CIdentType IdVarPStack;		
	private:		
		///Heap pointer index.
		sint32		_Val;	

	#ifdef _DEBUG
		NLIAAGENT::IVarName *_NameVar;
	#endif


	public:
		static NLIAAGENT::IObjectIA**	_LocalTableRef;

	public:
	#ifdef _DEBUG
		CVarPStack(sint32 val, const char *nameVar):
		  _Val(val),_NameVar(new NLIAAGENT::CIndexedVarName(nameVar))
		{			  
			_NameVar->incRef();
		}

		CVarPStack(const CVarPStack &v):NLIAAGENT::IObjetOp(v),_Val(v._Val),_NameVar((NLIAAGENT::IVarName *)v._NameVar->clone())
		{		
		}
	#else
		CVarPStack(sint32 val):
		  _Val(val)
		{
		}

		CVarPStack(const CVarPStack &v):NLIAAGENT::IObjetOp(v), _Val(v._Val)
		{
		}
	#endif

		///Gets the type of object stored in the heap.
		const NLIAC::CIdentType &getVarType(void) const
		{			
			return _LocalTableRef[_Val]->getType();
		}

		///Gets trhe index value.
		sint32 getIndex() const
		{
			return _Val;
		}		
				
		///\name Base class method.
		//@{
		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CVarPStack(*this);
			x->incRef();
			return x;
		}
		
		const NLIAC::IBasicType *newInstance() const 
		{
			return clone();
		}

		void save(NLMISC::IStream &os)
		{
			sint32 val = _Val;
			os.serial(val);
		}

		void load (NLMISC::IStream &is)
		{
			sint32 val;
			is.serial(val);
			val = _Val;
		}

		const NLIAC::CIdentType &getType(void) const
		{
			return IdVarPStack;
		}		

		void getDebugString(char *text) const
		{		
			char valVar[200];
			_LocalTableRef[_Val]->getDebugString(valVar);
			sprintf(text,"CVarPStack<%d>: _vale<%d> pointe la variable %s",this,_Val,valVar);
		}

		bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const
		{
			const CVarPStack &t = (const CVarPStack &)a;
			return t._Val == _Val;
		}

		const NLIAAGENT::IObjectIA::CProcessResult &run()
		{
			return NLIAAGENT::IObjectIA::ProcessRun;
		}

		virtual	NLIAAGENT::IObjetOp &neg()
		{
			((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val])->neg();
			return *this;
		}

		NLIAAGENT::IObjetOp &operator += (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) += a;
			return *this;
		}

		NLIAAGENT::IObjetOp &operator -= (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) -= a;
			return *this;
		}	

		NLIAAGENT::IObjetOp &operator *= (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) *= a;
			return *this;
		}
		NLIAAGENT::IObjetOp &operator /= (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) /= a;
			return *this;
		}	

		NLIAAGENT::IObjetOp *operator + (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) + a;
			
		}

		NLIAAGENT::IObjetOp *operator - (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) - a;
		}

		const NLIAAGENT::IObjetOp *operator * (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) * a;
		}

		const NLIAAGENT::IObjetOp *operator / (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) / a;
		}
		

		NLIAAGENT::IObjetOp *operator < (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) < a;
		}
		NLIAAGENT::IObjetOp *operator > (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) > a;
		}

		NLIAAGENT::IObjetOp *operator <= (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) <= a;
		}

		NLIAAGENT::IObjetOp *operator >= (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) >= a;
		}
		
		NLIAAGENT::IObjetOp *operator ! ()  const
		{
			return !*((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]);
		}

		NLIAAGENT::IObjetOp *operator != (NLIAAGENT::IObjetOp &a) const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) != a;
		}		

		NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const
		{
			return *((NLIAAGENT::IObjetOp *)_LocalTableRef[_Val]) == a;
		}

		bool isTrue() const
		{
			return ((NLIAAGENT::IObjetOp*)_LocalTableRef[_Val])->isTrue();
		}
		//@}
		
		virtual ~CVarPStack()
		{
	#ifdef _DEBUG
			_NameVar->release();
	#endif
		}
	};


	/**
	* Class CVarPStackParam.
	* 
	* This class define the same this as the CVarPStack but here we can define an shift on the heap and work with a relative index.
	* That is very useful when we have to declare method argument because if we want to call the script method MyMethod(Digital x,y);
	* Then we'll find x at the position -2 on the heap an y at the position -1.	
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class CVarPStackParam:public NLIAAGENT::IObjetOp
	{

	public:
		static NLIAC::CIdentType IdVarPStackParam;
		static	sint32	_Shift;
	private:		
		sint32		_Val;		

#ifdef _DEBUG
		NLIAAGENT::IVarName *_NameVar;
#endif	

	public:
#ifdef _DEBUG
		CVarPStackParam(sint32 val, const char *nameVar): _Val(val),_NameVar(new NLIAAGENT::CIndexedVarName(nameVar))
		{			  
			_NameVar->incRef();
		}

		CVarPStackParam(const CVarPStackParam &v):NLIAAGENT::IObjetOp(v),_Val(v._Val),_NameVar((NLIAAGENT::IVarName *)v._NameVar->clone())
		{		
		}
#else
		CVarPStackParam( sint32 val): _Val(val)
		{
		}

		CVarPStackParam(const CVarPStackParam &v):NLIAAGENT::IObjetOp(v), _Val(v._Val)
		{
		}
#endif
				

		///Clone the object stored in the heap.
		const NLIAAGENT::IObjectIA *getObjectIAClone() const
		{
			return (NLIAAGENT::IObjectIA  *)CVarPStack::_LocalTableRef[_Val + _Shift]->clone();
		}

		///get the type of the object stored in the heap.
		const NLIAC::CIdentType &getVarType(void) const
		{			
			return CVarPStack::_LocalTableRef[_Val + _Shift]->getType();
		}

		///get the object stored in the heap.
		const NLIAAGENT::IObjectIA *getObject(void) const
		{			
			return CVarPStack::_LocalTableRef[_Val + _Shift];
		}

		///get index of the object.
		sint32 getIndex() const
		{
			return _Val;
		}

		///\name Base class method.
		//@{
		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CVarPStackParam(*this);
			x->incRef();
			return x;
			//return (NLIAAGENT::IObjectIA  *)CVarPStack::_LocalTableRef[_Val + _Shift]->clone();
		}
		
		const NLIAC::IBasicType *newInstance() const 
		{
			NLIAC::IBasicType *x = new CVarPStackParam(*this);
			x->incRef();
			return x;
		}

		void save(NLMISC::IStream &os)
		{
			sint32 val = _Val;
			os.serial( val );
			sint32 shift = _Shift;
			os.serial( shift );
		}

		void load(NLMISC::IStream &is)
		{
			sint32 val;
			is.serial( val );
			_Val = val;
			sint32 shift;
			is.serial( shift );
			_Shift = shift;
		}

		const NLIAC::CIdentType &getType(void) const
		{
			return IdVarPStackParam;
		}		

		void getDebugString(char *text) const
		{		
			char valVar[200];
			CVarPStack::_LocalTableRef[_Val + _Shift]->getDebugString(valVar);
#if _DEBUG			
			sprintf(text,"Stack<%d,%s>: pointe la variable %s",_Val,_NameVar->getString(),valVar);
#else			
			sprintf(text,"Stack<%d>: pointe la variable %s",_Val,valVar);
#endif
		}

		bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const
		{
			const CVarPStackParam &t = (const CVarPStackParam &)a;
			return t._Val == _Val;
		}

		const NLIAAGENT::IObjectIA::CProcessResult &run()
		{
			return NLIAAGENT::IObjectIA::ProcessRun;
		}

		virtual	NLIAAGENT::IObjetOp &neg()
		{
			((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift])->neg();
			return *this;
		}

		NLIAAGENT::IObjetOp &operator += (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) += a;
			return *this;
		}

		NLIAAGENT::IObjetOp &operator -= (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) -= a;
			return *this;
		}	

		NLIAAGENT::IObjetOp &operator *= (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) *= a;
			return *this;
		}
		NLIAAGENT::IObjetOp &operator /= (const NLIAAGENT::IObjetOp &a)
		{
			*((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) /= a;
			return *this;
		}	

		NLIAAGENT::IObjetOp *operator + (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) + a;
			
		}

		NLIAAGENT::IObjetOp *operator - (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) - a;
		}

		const NLIAAGENT::IObjetOp *operator * (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) * a;
		}

		const NLIAAGENT::IObjetOp *operator / (const NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) / a;
		}
		

		NLIAAGENT::IObjetOp *operator < (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) < a;
		}
		NLIAAGENT::IObjetOp *operator > (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) > a;
		}

		NLIAAGENT::IObjetOp *operator <= (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) <= a;
		}

		NLIAAGENT::IObjetOp *operator >= (NLIAAGENT::IObjetOp &a)  const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) >= a;
		}
		
		NLIAAGENT::IObjetOp *operator ! ()  const
		{
			return !*((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]);
		}

		NLIAAGENT::IObjetOp *operator != (NLIAAGENT::IObjetOp &a) const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) != a;
		}		

		NLIAAGENT::IObjetOp *operator == (NLIAAGENT::IObjetOp &a) const
		{
			return *((NLIAAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) == a;
		}

		bool isTrue() const
		{
			return ((NLIAAGENT::IObjetOp*)CVarPStack::_LocalTableRef[_Val + _Shift])->isTrue();
		}		
		//@}
		virtual ~CVarPStackParam()
		{
	#ifdef _DEBUG
			_NameVar->release();
	#endif
		}
	};
		
}
#endif
