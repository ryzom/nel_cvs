
/** \file varstack.h
 *
 * $Id: varstack.h,v 1.10 2002/03/12 15:55:21 chafik Exp $
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


namespace NLAISCRIPT
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
	class CVarPStack:public NLAIAGENT::IObjetOp
	{

	public:
		static NLAIC::CIdentType IdVarPStack;		
	private:		
		///Heap pointer index.
		sint32		_Val;	

	#ifdef NL_DEBUG
		NLAIAGENT::IVarName *_NameVar;
	#endif


	public:
		static NLAIAGENT::IObjectIA**	_LocalTableRef;

	public:
	#ifdef NL_DEBUG
		CVarPStack(sint32 val, const char *nameVar):
		  _Val(val),_NameVar(new NLAIAGENT::CIndexedVarName(nameVar))
		{			  
		}

		CVarPStack(const CVarPStack &v):NLAIAGENT::IObjetOp(v),_Val(v._Val),_NameVar((NLAIAGENT::IVarName *)v._NameVar->clone())
		{		
		}
	#else
		CVarPStack(sint32 val):
		  _Val(val)
		{
		}

		CVarPStack(const CVarPStack &v):NLAIAGENT::IObjetOp(v), _Val(v._Val)
		{
		}
	#endif

		///Gets the type of object stored in the heap.
		const NLAIC::CIdentType &getVarType(void) const
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
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CVarPStack(*this);
			return x;
		}
		
		const NLAIC::IBasicType *newInstance() const 
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

		const NLAIC::CIdentType &getType(void) const
		{
			return IdVarPStack;
		}		

		void getDebugString(std::string &text) const
		{		
			std::string valVar;
			_LocalTableRef[_Val]->getDebugString(valVar);
			text = NLAIC::stringGetBuild("CVarPStack<%d>: _vale<%d> pointe la variable %s",this,_Val,valVar.c_str());
		}

		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const
		{
			const CVarPStack &t = (const CVarPStack &)a;
			return t._Val == _Val;
		}

		const NLAIAGENT::IObjectIA::CProcessResult &run()
		{
			return NLAIAGENT::IObjectIA::ProcessRun;
		}

		virtual	NLAIAGENT::IObjetOp &neg()
		{
			((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val])->neg();
			return *this;
		}

		NLAIAGENT::IObjetOp &operator += (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) += a;
			return *this;
		}

		NLAIAGENT::IObjetOp &operator -= (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) -= a;
			return *this;
		}	

		NLAIAGENT::IObjetOp &operator *= (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) *= a;
			return *this;
		}
		NLAIAGENT::IObjetOp &operator /= (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) /= a;
			return *this;
		}	

		NLAIAGENT::IObjetOp *operator + (const NLAIAGENT::IObjetOp &a) 
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) + a;
			
		}

		NLAIAGENT::IObjetOp *operator - (const NLAIAGENT::IObjetOp &a)
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) - a;
		}

		NLAIAGENT::IObjetOp *operator * (const NLAIAGENT::IObjetOp &a)
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) * a;
		}

		NLAIAGENT::IObjetOp *operator / (const NLAIAGENT::IObjetOp &a) 
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) / a;
		}
		

		NLAIAGENT::IObjetOp *operator < (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) < a;
		}
		NLAIAGENT::IObjetOp *operator > (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) > a;
		}

		NLAIAGENT::IObjetOp *operator <= (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) <= a;
		}

		NLAIAGENT::IObjetOp *operator >= (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) >= a;
		}
		
		NLAIAGENT::IObjetOp *operator ! ()  const
		{
			return !*((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]);
		}

		NLAIAGENT::IObjetOp *operator != (NLAIAGENT::IObjetOp &a) const
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) != a;
		}		

		NLAIAGENT::IObjetOp *operator == (NLAIAGENT::IObjetOp &a) const
		{
			return *((NLAIAGENT::IObjetOp *)_LocalTableRef[_Val]) == a;
		}

		bool isTrue() const
		{
			return ((NLAIAGENT::IObjetOp*)_LocalTableRef[_Val])->isTrue();
		}
		//@}
		
		virtual ~CVarPStack()
		{
	#ifdef NL_DEBUG
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
	class CVarPStackParam:public NLAIAGENT::IObjetOp
	{

	public:
		static NLAIC::CIdentType IdVarPStackParam;
		static	sint32	_Shift;
	private:		
		sint32		_Val;		

#ifdef NL_DEBUG
		NLAIAGENT::IVarName *_NameVar;
#endif	

	public:
#ifdef NL_DEBUG
		CVarPStackParam(sint32 val, const char *nameVar): _Val(val),_NameVar(new NLAIAGENT::CIndexedVarName(nameVar))
		{			  
		}

		CVarPStackParam(const CVarPStackParam &v):NLAIAGENT::IObjetOp(v),_Val(v._Val),_NameVar((NLAIAGENT::IVarName *)v._NameVar->clone())
		{		
		}
#else
		CVarPStackParam( sint32 val): _Val(val)
		{
		}

		CVarPStackParam(const CVarPStackParam &v):NLAIAGENT::IObjetOp(v), _Val(v._Val)
		{
		}
#endif
				

		///Clone the object stored in the heap.
		const NLAIAGENT::IObjectIA *getObjectIAClone() const
		{
			return (NLAIAGENT::IObjectIA  *)CVarPStack::_LocalTableRef[_Val + _Shift]->clone();
		}

		///get the type of the object stored in the heap.
		const NLAIC::CIdentType &getVarType(void) const
		{			
			return CVarPStack::_LocalTableRef[_Val + _Shift]->getType();
		}

		///get the object stored in the heap.
		const NLAIAGENT::IObjectIA *getObject(void) const
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
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CVarPStackParam(*this);
			return x;
			//return (NLAIAGENT::IObjectIA  *)CVarPStack::_LocalTableRef[_Val + _Shift]->clone();
		}
		
		const NLAIC::IBasicType *newInstance() const 
		{
			NLAIC::IBasicType *x = new CVarPStackParam(*this);
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

		const NLAIC::CIdentType &getType(void) const
		{
			return IdVarPStackParam;
		}		

		void getDebugString(std::string &text) const
		{		
			std::string valVar;
			CVarPStack::_LocalTableRef[_Val + _Shift]->getDebugString(valVar);
#ifdef NL_DEBUG			
			text = NLAIC::stringGetBuild("Stack<%d,%s>: pointe la variable %s",_Val,_NameVar->getString(),valVar.c_str());
#else			
			text = NLAIC::stringGetBuild("Stack<%d>: pointe la variable %s",_Val,valVar.c_str());
#endif
		}

		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const
		{
			const CVarPStackParam &t = (const CVarPStackParam &)a;
			return t._Val == _Val;
		}

		const NLAIAGENT::IObjectIA::CProcessResult &run()
		{
			return NLAIAGENT::IObjectIA::ProcessRun;
		}

		virtual	NLAIAGENT::IObjetOp &neg()
		{
			((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift])->neg();
			return *this;
		}

		NLAIAGENT::IObjetOp &operator += (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) += a;
			return *this;
		}

		NLAIAGENT::IObjetOp &operator -= (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) -= a;
			return *this;
		}	

		NLAIAGENT::IObjetOp &operator *= (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) *= a;
			return *this;
		}
		NLAIAGENT::IObjetOp &operator /= (const NLAIAGENT::IObjetOp &a)
		{
			*((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) /= a;
			return *this;
		}	

		NLAIAGENT::IObjetOp *operator + (const NLAIAGENT::IObjetOp &a)
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) + a;
			
		}

		NLAIAGENT::IObjetOp *operator - (const NLAIAGENT::IObjetOp &a)
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) - a;
		}

		NLAIAGENT::IObjetOp *operator * (const NLAIAGENT::IObjetOp &a)
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) * a;
		}

		NLAIAGENT::IObjetOp *operator / (const NLAIAGENT::IObjetOp &a)
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) / a;
		}
		

		NLAIAGENT::IObjetOp *operator < (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) < a;
		}
		NLAIAGENT::IObjetOp *operator > (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) > a;
		}

		NLAIAGENT::IObjetOp *operator <= (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) <= a;
		}

		NLAIAGENT::IObjetOp *operator >= (NLAIAGENT::IObjetOp &a)  const
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) >= a;
		}
		
		NLAIAGENT::IObjetOp *operator ! ()  const
		{
			return !*((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]);
		}

		NLAIAGENT::IObjetOp *operator != (NLAIAGENT::IObjetOp &a) const
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) != a;
		}		

		NLAIAGENT::IObjetOp *operator == (NLAIAGENT::IObjetOp &a) const
		{
			return *((NLAIAGENT::IObjetOp *)CVarPStack::_LocalTableRef[_Val + _Shift]) == a;
		}

		bool isTrue() const
		{
			return ((NLAIAGENT::IObjetOp*)CVarPStack::_LocalTableRef[_Val + _Shift])->isTrue();
		}		
		//@}
		virtual ~CVarPStackParam()
		{
	#ifdef NL_DEBUG
			_NameVar->release();
	#endif
		}
	};
		
}
#endif
