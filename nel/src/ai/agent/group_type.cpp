/** \file group_type.cpp
 *
 * $Id: group_type.cpp,v 1.6 2001/01/17 10:14:52 saffray Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/logic/boolval.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/interpret_methodes.h"
#include "nel/ai/agent/agent_method_def.h"

namespace NLAIAGENT
{
	const static sint32 _Const = 0;
	const static sint32 _Push = 1;
	const static sint32 _PushFront = 2;
	const static sint32 _Pop = 3;
	const static sint32 _PopFront = 4;
	const static sint32 _Back = 5;
	const static sint32 _Front = 6;
	const static sint32 _Get = 7;
	const static sint32 _Set = 8;
	const static sint32 _LastM = 9;

	IBaseGroupType::CMethodCall IBaseGroupType::_Method[] = 
	{
		IBaseGroupType::CMethodCall(_CONSTRUCTOR_,_Const),
		IBaseGroupType::CMethodCall("Push",_Push),
		IBaseGroupType::CMethodCall("PushFront",_PushFront),
		IBaseGroupType::CMethodCall("Pop",_Pop),
		IBaseGroupType::CMethodCall("PopFront",_PopFront),
		IBaseGroupType::CMethodCall("Back",_Back),
		IBaseGroupType::CMethodCall("Front",_Front),		
		IBaseGroupType::CMethodCall("Get",_Get),
		IBaseGroupType::CMethodCall("Set",_Set)		
	};
      
	IBaseGroupType::IBaseGroupType()
	{
	}

	IBaseGroupType::~IBaseGroupType()
	{
	}

	sint32 IBaseGroupType::getMethodIndexSize() const
	{
		return IObjetOp::getMethodIndexSize() + _LastM;
	}

	tQueue IBaseGroupType::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{
		tQueue a;
		NLAISCRIPT::CParam methodParam;
		NLAISCRIPT::CParam &param = (NLAISCRIPT::CParam &)p;
		
		if(className == NULL)
		{
			for(int i = 0; i < _LastM; i++)
			{
				if(*methodName == IBaseGroupType::_Method[i].MethodName)
				{					
					CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
					c->incRef();					
					a.push(CIdMethod(IBaseGroupType::_Method[i].Index + IObjetOp::getMethodIndexSize(),0.0,NULL,c));					
					break;
				}
			}
		}
		return a;
	}

	IObjectIA::CProcessResult IBaseGroupType::runMethodeMember(sint32, sint32, IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}
	IObjectIA::CProcessResult IBaseGroupType::runMethodeMember(sint32 index,IObjectIA *p)
	{
		IBaseGroupType *param = (IBaseGroupType *)p;

		switch(index - IObjetOp::getMethodIndexSize())
		{
		case _Const:
			return IObjectIA::CProcessResult();
		case _Push:	
			{
				CIteratorContener i = param->getIterator();
				while(!i.isInEnd())
				{
					IObjectIA *a = (IObjectIA *)i++;
					a->incRef();
					push(a);
				}	
			}
			return IObjectIA::CProcessResult();
		case _PushFront:	
			{
				CIteratorContener i = param->getIterator();
				while(!i.isInEnd())
				{
					IObjectIA *a = (IObjectIA *)i++;
					a->incRef();
					pushFront(a);
				}	
			}
			return IObjectIA::CProcessResult();
			
		
		case _Pop:			
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)pop();				
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _PopFront:	
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)popFront();				
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Back:
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)get();
				a->incRef();
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Front:	
			{
				IObjectIA::CProcessResult c;
				IObjectIA *a = (IObjectIA *)getFront();
				a->incRef();
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Get:
			{
				IObjectIA::CProcessResult c;
				const DigitalType *f = (const DigitalType *)param->get();
				IObjectIA *a = (IObjectIA *)(*this)[(sint32)f->getValue()];
				a->incRef();
				c.Result = a;
				c.ResultState = IObjectIA::ProcessIdle;
				return c;
			}

		case _Set:
			{
				IObjectIA::CProcessResult c;
				CIteratorContener i = param->getIterator();
				const DigitalType *f = (const DigitalType *)i ++;
				IObjectIA *n = (IObjectIA *)i++;				
				

				set((sint32)f->getValue(),n);
				n->incRef();							
				return IObjectIA::CProcessResult();
			}						
		}

		return IObjectIA::CProcessResult();
	}

	sint32 IBaseGroupType::isClassInheritedFrom(const IVarName &) const
	{
		return -1;
	}

	bool IBaseGroupType::isTrue() const
	{
		return IObjetOp::isTrue();
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

	IObjetOp &CGroupType::operator += (const IObjetOp &a)
	{
		cpy(a);
		return *this;
	}

	IObjetOp &CGroupType::operator -= (const IObjetOp &a)
	{
		erase(a);
		return *this;
	}

	IObjetOp *CGroupType::operator ! () const
	{		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(!size());
		x->incRef();
		return x;
	}

	IObjectIA &CGroupType::operator = (const IObjectIA &a)
	{		
		clear();
		if((uint)(((const NLAIC::CTypeOfObject &)a.getType()) & NLAIC::CTypeOfObject::tList))
		{
			const CGroupType &g= (const CGroupType &)a;
			tListType::const_iterator i = g._List.begin();
			while(i != g._List.end())
			{
				_List.push_back((const IObjectIA *)(*i++)->clone());
			}
		}	
		else _List.push_back((const IObjectIA *)a.clone());
		return *this;
	}

	const IObjectIA *CGroupType::operator[] (sint32 index) const
	{
		if ( index >= (sint32) _List.size() )
		{
			//TODO: throw Exc::CExceptionIndexError
		}

		tListType::const_iterator it_l = _List.begin();
		for (sint32 i = 0; i < index; i++)
			it_l++;

		return *it_l;
	}

	void CGroupType::set(int index,IObjectIA *o)
	{
		tListType::iterator it_l = _List.begin();
		for (sint32 i = 0; i < index; i++)
			it_l++;
		((IObjectIA *)(*it_l))->release();
		(*it_l) = o;
	}

	const NLAIC::CIdentType &CGroupType::getType() const		
	{		
		return IdGroupType;
	}

	const IObjectIA::CProcessResult &CGroupType::run()
	{
		tListType::iterator i = _List.begin();
		while(i != _List.end())
		{
			((IObjectIA *)(*i++))->run();
		}
		return IObjectIA::ProcessRun;  
	}

	CGroupType::tListType &CGroupType::getList() 
	{
		return _List;
	}

	CGroupType::CGroupType()
	{						
	}

	CGroupType::CGroupType(const CGroupType &g)
	{
		tListType::const_iterator i = g._List.begin();
		while(i != g._List.end())
		{
			_List.push_back((const IObjectIA *)(*i++)->clone());
		}
	}			

	void CGroupType::getDebugString(char *text) const
	{		
				
		sprintf(text,"[");
		sint32 k = 0;
		std::list<const IObjectIA *>::const_iterator i = _List.begin();
		while(i != _List.end())
		{
			char temp[1024*8];
			(*i++)->getDebugString(temp);				
			strcat(text,temp);
			if(i != _List.end()) strcat(text," ");
		}
		strcat(text,"]");
	}	

//	IObjetOp *CGroupType::operator ! () const;

	void CGroupType::push(const IObjectIA *o)
	{
		_List.push_back(o);
	}

	void CGroupType::pushFront(const IObjectIA *o)
	{
		_List.push_front(o);
	}

	void CGroupType::cpy(const IObjectIA &o) 
	{
		_List.push_back((const IObjectIA *)o.clone());
	}

	const IObjectIA *CGroupType::pop() 
	{
		const IObjectIA *Obj = _List.back();
		_List.pop_back();
		return Obj;
	}

	const IObjectIA *CGroupType::get() 
	{
		return _List.back();			
	}

	const IObjectIA *CGroupType::popFront() 
	{
		const IObjectIA *Obj = _List.front();
		_List.pop_front();
		return Obj;
	}

	const IObjectIA *CGroupType::getFront() 
	{
		return _List.front();			
	}	

	sint32 CGroupType::size() const
	{
		return _List.size();
	}

	CGroupType::tListType CGroupType::findList(const IObjectIA &obj) const
	{
		tListType l;			
		tListType::const_iterator i = _List.begin();
		while(i != _List.end())
		{
			const IObjectIA *o = *i++;
			if( *o == obj) l.push_back();
		}
		return l;

	}

	CGroupType::tListType::const_iterator CGroupType::getBegin() const
	{
		return _List.begin();
	}

	CGroupType::tListType::const_iterator CGroupType::getEnd() const
	{
		return _List.end();
	}		

	CGroupType::tListType::iterator CGroupType::getBegin()
	{
		return _List.begin();
	}

	CGroupType::tListType::iterator CGroupType::getEnd()
	{
		return _List.end();
	}		

	const IObjectIA *CGroupType::find(const IObjectIA &obj) const
	{			
		tListType::const_iterator i = _List.begin();
		while(i != _List.end())
		{
			const IObjectIA *o = *i++;
			if( *o == obj) return o;
		}
		return NULL;
	}

	/*void CGroupType::eraseFirst(const IObjectIA &obj) 
	{				
		tListType::iterator i = _List.begin();
		while(i != _List.end())
		{
			IObjectIA *o= (IObjectIA *)*i;
			if( *o == obj)
			{					
				_List.erase(i);
				o->release();
				return;
			}
			i++;
		}			
	}*/

	void CGroupType::eraseAll(const IObjectIA &obj) 
	{			
		tListType::iterator i = _List.begin();
		while(i != _List.end())
		{
			tListType::iterator j = i++;
			IObjectIA *o= (IObjectIA *)*j;
			if( *o == obj)
			{					
				_List.erase(j);
				o->release();
				return;
			}
		}
	}

	void CGroupType::erase(const IObjectIA *o) 
	{	
		tListType::iterator i = _List.begin();
		while(i != _List.end())
		{
			tListType::iterator j = i++;
			if( *j == o)
			{
				IObjectIA *o = (IObjectIA *)*j;
				_List.erase(j);
				o->release();
				return;
			}				
		}
	}

	void CGroupType::erase(const IObjectIA &obj) 
	{	
		tListType::iterator i = _List.begin();
		while(i != _List.end())
		{
			tListType::iterator j = i++;
			if( *(*j) == obj)
			{
				IObjectIA *o = (IObjectIA *)*j;
				_List.erase(j);
				o->release();
				return;
			}				
		}
	}

	void CGroupType::erase(std::list<const IObjectIA *> &l) 
	{			
		tListType::iterator i = l.begin();
		while(i != l.end())
		{
			erase(*i++);								
		}
	}		

	bool CGroupType::isEqual(const IBasicObjectIA &a) const
	{
		const CGroupType &b = (const CGroupType &)a;			
		if(size() != b.size()) return false;

		tListType::const_iterator i = _List.begin();
		tListType::const_iterator j = b._List.begin();
		
		while(i != _List.end())
		{
			const IObjectIA *a1 = *i;
			const IObjectIA *a2 = *j;
			if( a1 != a2)
				if(!(*a1 == *a2)) return false;
			i ++;
			j ++;
		}
		return true;
	}

	const NLAIC::IBasicType *CGroupType::clone() const
	{
		NLAIC::IBasicInterface *m = new CGroupType(*this);
		m->incRef();
		return m;
	}

	const NLAIC::IBasicType *CGroupType::newInstance() const
	{
		NLAIC::IBasicInterface *m = new CGroupType();
		m->incRef();
		return m;
	}

	const NLAIC::CIdentType &CGroupType::getType() const;		

	void CGroupType::save(NLMISC::IStream &os)
	{	
		sint32 size = _List.size();
		os.serial(size);
		std::list<const IObjectIA *>::const_iterator i = _List.begin();
		while(i != _List.end())
		{
			IObjectIA *o= (IObjectIA *)*i++;
			os.serial( (NLAIC::CIdentType &) o->getType() );
			o->save(os);
		}			
	}

	IObjetOp &CGroupType::neg()
	{
		tListType::iterator i = _List.begin();
		while(i != _List.end())
		{
			const NLAIC::CTypeOfOperator &op = (const NLAIC::CTypeOfOperator &)(*i)->getType();
			if((uint32)(op & NLAIC::CTypeOfOperator::opNeg)) ((IObjetOp *)(*i))->neg();
			i++;
		}
		return *this;	
	}

	void CGroupType::load(NLMISC::IStream &is)
	{			
		sint32 i;
		while(_List.size())
		{
			NLAIC::IBasicInterface * o = (NLAIC::IBasicInterface *)_List.front();
			o->release();
			_List.pop_front();
		}
		is.serial( i );
		NLAIC::CIdentTypeAlloc id;
		while(i--)
		{				
			is.serial( id );
			NLAIC::IBasicInterface *o = (NLAIC::IBasicInterface *)id.allocClass();				
			o->load(is);
			_List.push_back((const IObjectIA *)o);
				
		}			
	}

	void CGroupType::clear()
	{
		while(_List.size())
		{
			NLAIC::IBasicInterface * o = (NLAIC::IBasicInterface *)_List.front();
			o->release();
			_List.pop_front();
		}
	}

	CGroupType::~CGroupType()
	{			
		clear();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IObjetOp &CVectorGroupType::operator += (const IObjetOp &a)
	{
		cpy(a);
		return *this;
	}

	IObjetOp &CVectorGroupType::operator -= (const IObjetOp &a)
	{
		erase(a);
		return *this;
	}

	IObjetOp *CVectorGroupType::operator ! () const
	{		
		NLAILOGIC::CBoolType *x = new NLAILOGIC::CBoolType(!size());
		x->incRef();
		return x;
	}

	IObjectIA &CVectorGroupType::operator = (const IObjectIA &a)
	{		
		clear();
		if((uint)(((const NLAIC::CTypeOfObject &)a.getType()) & NLAIC::CTypeOfObject::tList))
		{
			const CVectorGroupType &g= (const CVectorGroupType &)a;
			tVectorType::const_iterator i = g._Vector.begin();
			while(i != g._Vector.end())
			{
				_Vector.push_back((const IObjectIA *)(*i++)->clone());
			}
		}	
		else 
			_Vector.push_back((const IObjectIA *)a.clone());
		return *this;
	}

	const NLAIC::CIdentType &CVectorGroupType::getType() const		
	{		
		return IdVectorGroupType;
	}

	const IObjectIA::CProcessResult &CVectorGroupType::run()
	{
		tVectorType::iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			((IObjectIA *)(*i++))->run();
		}
		return IObjectIA::ProcessRun; 
	}

	CVectorGroupType::tVectorType &CVectorGroupType::getVector() 
	{
		return _Vector;
	}


	CVectorGroupType::CVectorGroupType(sint32 i): _Vector(i)
	{
	}

	CVectorGroupType::CVectorGroupType()
	{						
	}

	CVectorGroupType::CVectorGroupType(const CVectorGroupType &g)
	{
		_Vector = g._Vector;
		tVectorType::const_iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			((IObjectIA *)(*i++))->incRef();
		}
	}			

	void CVectorGroupType::getDebugString(char *text) const
	{		
		
		if(_Vector.size())
		{			
			sprintf(text,"[",_Vector.size());
			sint32 k = 0;
			std::vector<const IObjectIA *>::const_iterator i = _Vector.begin();
			while(i != _Vector.end())
			{
				char temp[1024*8];
				(*i++)->getDebugString(temp);				
				strcat(text,temp);
				if(i != _Vector.end()) strcat(text," ");
			}
			strcat(text,"]");

		}
		else
		{
			sprintf(text,"CVectorGroupType<%04x>: <empty>",this);
		}		
	}	

//	IObjetOp *CVectorGroupType::operator ! () const;

	void CVectorGroupType::push(const IObjectIA *o)
	{
		_Vector.push_back(o);
	}

	void CVectorGroupType::pushFront(const IObjectIA *o)
	{
		_Vector.push_back(o);
		for(sint32 i = (sint32)_Vector.back() - 2; i == 0; i -- )
		{			
			_Vector[i + 1] = _Vector[i];
		}
		_Vector[0] = o;

	}

	void CVectorGroupType::cpy(const IObjectIA &o) 
	{
		_Vector.push_back((const IObjectIA *)o.clone());
	}

	const IObjectIA *CVectorGroupType::pop() 
	{
		const IObjectIA *Obj = _Vector.back();
		_Vector.erase( _Vector.end() );
		return Obj;
	}

	const IObjectIA *CVectorGroupType::get() 
	{
		return _Vector.back();			
	}

	const IObjectIA *CVectorGroupType::popFront() 
	{
		const IObjectIA *Obj = _Vector.front();
		_Vector.erase( _Vector.begin() );
		return Obj;
	}

	const IObjectIA *CVectorGroupType::getFront() 
	{
		return _Vector.front();			
	}	

	sint32 CVectorGroupType::size() const
	{
		return _Vector.size();
	}

	CVectorGroupType::tVectorType CVectorGroupType::findList(const IObjectIA &obj) const
	{
		tVectorType l;			
		tVectorType::const_iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			const IObjectIA *o = *i++;
			if( *o == obj) l.push_back();
		}
		return l;

	}

	const IObjectIA *CVectorGroupType::operator[] (sint32 index) const
	{
#ifdef _DEGUG
		if ( index >= (sint32) _Vector.size() )
		{
			//TODO: throw Exc::CExceptionIndexError
		}
#endif

		return _Vector[ index ];
	}

	void CVectorGroupType::set(int index,IObjectIA *o)
	{
		((IObjectIA *)_Vector[ index ])->release();
		_Vector[ index ] = o;
	}


	CVectorGroupType::tVectorType::const_iterator CVectorGroupType::getBegin() const
	{
		return _Vector.begin();
	}

	CVectorGroupType::tVectorType::const_iterator CVectorGroupType::getEnd() const
	{
		return _Vector.end();
	}		

	CVectorGroupType::tVectorType::iterator CVectorGroupType::getBegin()
	{
		return _Vector.begin();
	}

	CVectorGroupType::tVectorType::iterator CVectorGroupType::getEnd()
	{
		return _Vector.end();
	}		

	const IObjectIA *CVectorGroupType::find(const IObjectIA &obj) const
	{			
		tVectorType::const_iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			const IObjectIA *o = *i++;
			if( *o == obj) return o;
		}
		return NULL;
	}	

	void CVectorGroupType::eraseAll(const IObjectIA &obj) 
	{			
		tVectorType::iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			tVectorType::iterator j = i++;
			IObjectIA *o= (IObjectIA *)*j;
			if( *o == obj)
			{					
				_Vector.erase(j);
				o->release();
				return;
			}
		}
	}

	void CVectorGroupType::erase(const IObjectIA *o) 
	{	
		tVectorType::iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			tVectorType::iterator j = i++;
			if( *j == o)
			{
				IObjectIA *o = (IObjectIA *)*j;
				_Vector.erase(j);
				o->release();
				return;
			}				
		}
	}

	void CVectorGroupType::erase(const IObjectIA &obj) 
	{	
		tVectorType::iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			tVectorType::iterator j = i++;
			if ( *(*j) == obj )
			{
				IObjectIA *o = (IObjectIA *)*j;
				_Vector.erase(j);
				o->release();
				return;
			}				
		}
	}

	void CVectorGroupType::erase(std::list<const IObjectIA *> &l) 
	{			
		std::list<const IObjectIA *>::iterator i = l.begin();
		while(i != l.end())
		{
			erase(*i++);								
		}
	}		

	bool CVectorGroupType::isEqual(const IBasicObjectIA &a) const
	{
		const CVectorGroupType &b = (const CVectorGroupType &)a;			
		if(size() != b.size()) return false;

		tVectorType::const_iterator i = _Vector.begin();
		tVectorType::const_iterator j = _Vector.begin();
		
		while(i != _Vector.end())
		{
			if(!(*i++)->isEqual(*(*j++))) return false;
		}
		return true;
	}

	const NLAIC::IBasicType *CVectorGroupType::clone() const
	{
		NLAIC::IBasicInterface *m = new CVectorGroupType(*this);
		m->incRef();
		return m;
	}

	const NLAIC::IBasicType *CVectorGroupType::newInstance() const
	{
		NLAIC::IBasicInterface *m = new CVectorGroupType();
		m->incRef();
		return m;
	}

	const NLAIC::CIdentType &CVectorGroupType::getType() const;		

	void CVectorGroupType::save(NLMISC::IStream &os)
	{	
		sint32 size = _Vector.size();
		os.serial( size );
		std::vector<const IObjectIA *>::const_iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			IObjectIA *o= (IObjectIA *)*i++;
			os.serial( (NLAIC::CIdentType &) o->getType() );
			o->save(os);
		}			
	}

	IObjetOp &CVectorGroupType::neg()
	{
		tVectorType::iterator i = _Vector.begin();
		while(i != _Vector.end())
		{
			const NLAIC::CTypeOfOperator &op = (const NLAIC::CTypeOfOperator &)(*i)->getType();
			if((uint32)(op & NLAIC::CTypeOfOperator::opNeg)) ((IObjetOp *)(*i))->neg();
			i++;
		}
		return *this;	
	}

	void CVectorGroupType::load(NLMISC::IStream &is)
	{			
		sint32 i;
		while(_Vector.size())
		{
			NLAIC::IBasicInterface * o = (NLAIC::IBasicInterface *)_Vector.front();
			o->release();
			_Vector.erase( _Vector.begin() );
		}
		is.serial( i );
		NLAIC::CIdentTypeAlloc id;
		while(i--)
		{				
			is.serial( id );
			NLAIC::IBasicInterface *o = (NLAIC::IBasicInterface *)id.allocClass();				
			o->load(is);
			_Vector.push_back((const IObjectIA *)o);
				
		}			
	}

	void CVectorGroupType::clear()
	{
		while(_Vector.size())
		{
			NLAIC::IBasicInterface * o = (NLAIC::IBasicInterface *)_Vector.front();
			o->release();
			_Vector.erase( _Vector.begin() );
		}
	}

	tQueue CVectorGroupType::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{
		tQueue a;
		NLAISCRIPT::CParam methodParam;
		NLAISCRIPT::CParam &param = (NLAISCRIPT::CParam &)p;
		
		if(param.size() != 1) return IBaseGroupType::isMember(className,methodName,p);

		if(className == NULL)
		{			
			if(*methodName == IBaseGroupType::_Method[0].MethodName)
			{				
				
				CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
				c->incRef();					
				a.push(CIdMethod(_Const + IBaseGroupType::getMethodIndexSize(),0.0,NULL,c));				
			}
			else
			{
				return IBaseGroupType::isMember(className,methodName,p);
			}
			
		}
		return a;
	}

	sint32 CVectorGroupType::getMethodIndexSize() const
	{
		return IBaseGroupType::getMethodIndexSize() + 1;
	}

	IObjectIA::CProcessResult CVectorGroupType::runMethodeMember(sint32, sint32, IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}
	IObjectIA::CProcessResult CVectorGroupType::runMethodeMember(sint32 index,IObjectIA *p)
	{
		sint32 i= index - IBaseGroupType::getMethodIndexSize();
		if(i == _Const)
		{
			IBaseGroupType *param = (IBaseGroupType *)p;
			const DigitalType *f = (const DigitalType *)param->get();
			_Vector.reserve((int)f->getValue());
		}
		return IBaseGroupType::runMethodeMember(index,p);
	}	

	CVectorGroupType::~CVectorGroupType()
	{			
		clear();
	}

}
