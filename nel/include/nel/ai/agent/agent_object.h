/** \file agent_object.h
 * Sevral class for objects manipulation.
 *
 * $Id: agent_object.h,v 1.23 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_AGENT_OBJECT_DEF_H
#define NL_AGENT_OBJECT_DEF_H


#include "nel/ai/agent/agent_string.h"
#include "nel/ai/agent/agent_operator.h"


namespace NLAIAGENT
{			

	/**	
	  This class is an IObjetOp string type.
		  
	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class CStringType: public IObjetOp
	{
	private:		
		IVarName *_Str;					

	public:
		static const NLAIC::CIdentType IdStringType;
	public:
		CStringType(const std::string &str): _Str(new CStringVarName(str.c_str()))
		{
		}

		CStringType(const IVarName &str): _Str((IVarName *)str.clone())
		{			
		}

		CStringType(const CStringType &a): _Str((IVarName *)a._Str->clone())
		{			
		}		

		CStringType(NLMISC::IStream &is)
		{
			load(is);
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CStringType(*this);
			return m;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		
		virtual const NLAIC::CIdentType &getType() const;		

		virtual void save(NLMISC::IStream &os)
		{			
			os.serial( (NLAIC::CIdentType &) ( _Str->getType() ) );
			_Str->save(os);
		}

		virtual void load(NLMISC::IStream &is)
		{			
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			_Str->release();
			_Str = (IVarName *)id.allocClass();			
			_Str->load(is);			
		}


		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%s",_Str->getString());
		}

		const IVarName & getStr() const
		{
			return *_Str;
		}

		///\name IObjetOp method definition.
		//@{
		virtual IObjetOp &operator += (const IObjetOp &a);		
		virtual IObjetOp &operator -= (const IObjetOp &a);		
		virtual IObjetOp *operator < (IObjetOp &a) const;
		virtual IObjetOp *operator > (IObjetOp &a) const;
		virtual IObjetOp *operator <= (IObjetOp &a) const;	
		virtual IObjetOp *operator >= (IObjetOp &a) const;	
		virtual IObjetOp *operator != (IObjetOp &a) const;
		virtual IObjetOp *operator == (IObjetOp &a) const;
		//@}


		virtual bool operator < (const CStringType &a) const;


		virtual bool isTrue() const
		{
			return IObjetOp::isTrue();
		}
		
		virtual ~CStringType()
		{	
			_Str->release();
		}	
		
		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			const CStringType &t = (const CStringType &)a;
			return *t._Str == *_Str;
		}
			

		virtual const CProcessResult &run();		
	};

		

	/**	
	  This class is a IObjetOp pair type.		  
	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/	
	class CPairType: public IObjetOp
	{
	public:
		struct CMethodCall
		{
			CMethodCall(const char *name, int i): MethodName (name)
			{				
				Index = i;
			}
			CStringVarName MethodName;
			sint32 Index;
		};
		static CMethodCall _Method[];

	private:		
		IObjectIA *_First;
		IObjectIA *_Second;

	public:
		static const NLAIC::CIdentType IdPaireType;
		
	public:
		CPairType(IObjectIA *first,IObjectIA *second): 
		  _First(first),_Second(second)
		{			
		}

		CPairType(const CPairType &a): _First(a._First),_Second(a._Second)
		{
			_First->incRef();
			_Second->incRef();
		}				

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPairType(*this);
			return m;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPaireType;
		}
		

		virtual void save(NLMISC::IStream &os)
		{												
		}

		virtual void load(NLMISC::IStream &is)
		{									
		}

		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

		virtual void getDebugString(std::string &text) const
		{
			std::string f,s;

			_First->getDebugString(f);
			_Second->getDebugString(s);
			text += "First:<";
			text += f;
			text += ">";
			text += "Second:<";
			text += s;
			text += ">";
		}

		virtual bool isTrue() const
		{
			return IObjetOp::isTrue();
		}
		
		IObjectIA *first() const
		{
			return _First;
		}

		IObjectIA *second() const
		{
			return _Second;
		}
		
		virtual ~CPairType()
		{			
			_First->release();
			_Second->release();
		}	

		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			const CPairType &t = (const CPairType &)a;
			if((t._First) == (_First)) return t._Second == _Second;
			return false;
		}

		virtual sint32 getMethodIndexSize() const;
		virtual TQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);
	};	

	/**	
	  This class is an iterator for list access.
	
	  We have two list type a std::vector list and a std::list type we define is class to standardize iterator list access.

	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/

	class IConstBasicIterator
	{
	public:
		IConstBasicIterator() {}
		virtual const IObjetOp* operator ++ (int) = 0;
		virtual const IObjetOp* operator -- (int) = 0;
		virtual operator const IObjetOp*() const = 0;		
		virtual bool isInEnd() const = 0;
		virtual bool isInBegin() const = 0;
		virtual ~IConstBasicIterator() {}
	};

	class IBasicIterator: public IConstBasicIterator
	{
	public:
		IBasicIterator() {}		
		virtual void erase() = 0;		
		virtual ~IBasicIterator() {}
	};

	/**	
	  Template version of IBasicIterator implementation.

	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	template<class typeClass>
	class CTemplateIterator: public IBasicIterator
	{
	private:
		typename typeClass::iterator _I;
		typeClass	&_ListType;
		
	public:
		CTemplateIterator(typeClass &l):_I(l.begin()),_ListType(l)
		{
		}
		virtual ~CTemplateIterator()
		{
		}

		virtual const IObjetOp* operator ++ (int) 
		{
			return (const IObjetOp*)*_I++;
		}
		virtual const IObjetOp* operator -- (int)
		{
			return (const IObjetOp*)*_I--;
		}

		virtual operator const IObjetOp *() const
		{
			return (const IObjetOp *)*_I;
		}
		
		virtual void erase()
		{
			typename typeClass::iterator temp = _I;
			_I++;
			_ListType.erase(temp);
		}

		virtual bool isInEnd() const
		{			
			return _I == _ListType.end();		
		}

		virtual bool isInBegin() const
		{
			return _I == _ListType.begin();
		}
	};

	template<class typeClass>
	class CConstTemplateIterator: public IConstBasicIterator
	{
	private:
		typename typeClass::const_iterator _I;
		const typeClass	&_ListType;
		
	public:
		CConstTemplateIterator(const typeClass &l):_I(l.begin()),_ListType(l)
		{
		}
		virtual ~CConstTemplateIterator()
		{
		}

		virtual const IObjetOp* operator ++ (int) 
		{
			return (const IObjetOp*)*_I++;
		}
		virtual const IObjetOp* operator -- (int)
		{
			return (const IObjetOp*)*_I--;
		}

		virtual operator const IObjetOp *() const
		{
			return (const IObjetOp *)*_I;
		}
				
		virtual bool isInEnd() const
		{			
			return _I == _ListType.end();		
		}

		virtual bool isInBegin() const
		{
			return _I == _ListType.begin();
		}
	};


	/**
	  std::list version of IBasicIterator implementation.

	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	
	class CListIterator: public CTemplateIterator<std::list<const IObjectIA *> >
	{
	public:
		CListIterator(std::list<const IObjectIA *> &l): CTemplateIterator<std::list<const IObjectIA *> >(l)
		  {
		  }

		virtual ~CListIterator()
		{
		}

	};

	class CConstListIterator: public CConstTemplateIterator<std::list<const IObjectIA *> >
	{
	public:
		CConstListIterator(const std::list<const IObjectIA *> &l): CConstTemplateIterator<std::list<const IObjectIA *> >(l)
		{
		}

		virtual ~CConstListIterator()
		{
		}

	};
	
	/**
	  std::vector version of IBasicIterator implementation.

	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class CVectorIterator: public CTemplateIterator<std::vector<const IObjectIA *> >
	{
	public:
		CVectorIterator(std::vector<const IObjectIA *>&l): CTemplateIterator<std::vector<const IObjectIA *> >(l)
		  {
		  }

		virtual ~CVectorIterator()
		{
		}

	};

	class CConstVectorIterator: public CConstTemplateIterator<std::vector<const IObjectIA *> >
	{
	public:
		CConstVectorIterator(const std::vector<const IObjectIA *>&l): CConstTemplateIterator<std::vector<const IObjectIA *> >(l)
		{
		}

		virtual ~CConstVectorIterator()
		{
		}

	};

	/**
	  This class is very use in all algorithme. 
	  The advatange is that programmer don't have to delete an abstract iterator IBasicIterator, and he can manipulate this class withaout memory leek.

	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class CConstIteratorContener: public IConstBasicIterator
	{
	protected:
		IBasicIterator *_I;
	public:
		CConstIteratorContener(const CConstIteratorContener &i):_I(i._I)
		{
		}

		CConstIteratorContener(IBasicIterator *i):_I(i)
		{
		}

		virtual ~CConstIteratorContener()
		{
			delete _I;
		}

		virtual const IObjetOp*operator ++ (int) 
		{
			const IObjetOp *a = (*_I)++;			
			return a;
		}
		virtual const IObjetOp*operator -- (int)
		{
			const IObjetOp *a = (*_I) --;			
			return a;
		}		
		virtual operator const IObjetOp*() const
		{
			return (const IObjetOp *)*_I;
		}		
		
		virtual bool isInEnd() const 
		{			
			return _I->isInEnd();
		}
		virtual bool isInBegin() const
		{
			return _I->isInBegin();
		}
	};

	class CIteratorContener: public CConstIteratorContener
	{	
	public:
		CIteratorContener(const CIteratorContener &i):CConstIteratorContener(i._I)
		{
		}

		CIteratorContener(const CConstIteratorContener &i):CConstIteratorContener(i)
		{
		}

		CIteratorContener(IBasicIterator *i):CConstIteratorContener(i)
		{
		}

		virtual ~CIteratorContener()
		{		
		}

		virtual void erase()
		{
			_I->erase();
		}

	};
	
	/**	
	  Base class for a IObjetOp list object.

	  * \author Portier Pierre	
	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class IBaseGroupType: public IObjetOp
	{

	public:
		struct CMethodCall
		{
			CMethodCall(const char *name, int i): MethodName (name)
			{				
				Index = i;
			}
			CStringVarName MethodName;
			sint32 Index;
		};
		static CMethodCall _Method[]; 	

	public:		

		IBaseGroupType(const IBaseGroupType &c):IObjetOp(c){}		
		IBaseGroupType();
		virtual ~IBaseGroupType();
		
		///\name Some IObjetOp method definition.
		//@{
		virtual IObjetOp &operator += (IObjetOp *a);
		virtual IObjetOp &operator -= (IObjetOp *a);
		virtual IObjetOp &operator += (const IObjetOp &a);
		virtual IObjetOp &operator -= (const IObjetOp &a);

		virtual IObjetOp *operator ! () const = 0;
		virtual bool isTrue() const;
		//@}

		///\name Some IObjectIA method definition.
		//@{
		virtual sint32 getMethodIndexSize() const;
		virtual sint32 isClassInheritedFrom(const IVarName &) const;
		virtual TQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);
		//@}
		
		///\name List method manipulation.
		//@{		
		virtual IObjectIA &operator = (const IObjectIA &a) = 0;
		virtual const IObjectIA *operator[] (sint32) const= 0;
		
		///Push an IObjectIA at the position indicate by the int.
		virtual void set(int,IObjectIA *) = 0;		
		
		///Push an IObjectIA back.
		virtual void push(const IObjectIA *o) = 0;
		///Push an IObjectIA front.
		virtual void pushFront(const IObjectIA *o) = 0;
		///Push an IObjectIA back using the clone method.
		virtual void cpy(const IObjectIA &o) = 0;
		///Get an iterator to parse the list.
		virtual CIteratorContener getIterator() = 0;
		virtual CConstIteratorContener getConstIterator() const = 0;

		///Pop the back IObjectIA and return it.
		virtual const IObjectIA *pop() = 0;
		///Pop the front IObjectIA and return it.
		virtual const IObjectIA *popFront() = 0;
		///return the back IObjectIA.
		virtual const IObjectIA *get() const = 0;		
		///return the front IObjectIA.
		virtual const IObjectIA *getFront() const = 0;
		///get the size of the list.
		virtual sint32 size() const = 0;

		///Find an element on the list the operator == is use.
		virtual const IObjectIA *find(const IObjectIA &obj) const = 0;
		
		//virtual void eraseFirst(const IObjectIA &) = 0;
		///Erase all element find equal at the const IObjectIA & on argument. 
		virtual void eraseAll(const IObjectIA &) = 0; 
		///Erase the element how have the same pointer memory as the argument const IObjectIA *.
		virtual void erase(const IObjectIA *) = 0; 
		///Erase the first element find equal at the const IObjectIA& on argument. 
		virtual void erase(const IObjectIA &) = 0;
		///Erase all element contain in the list.
		virtual void erase(std::list<const IObjectIA *> &) = 0;
		///Clear the list.
		virtual void clear() = 0;
		//@}
	};
	/**	
	  concret IBaseGroupType with a std::list container.

	  * \author Portier Pierre	
	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class CGroupType: public IBaseGroupType
	{

	public:
		static const NLAIC::CIdentType IdGroupType;
		
	public:
		//typedef of the std::list constainer
		typedef std::list<const IObjectIA *> TListType;
	public:				
		///std::list for store IObjectIA object.
		TListType _List;

	protected:

		TListType &getList();

	public:
		CGroupType();
		CGroupType(const CGroupType &g);

		virtual void getDebugString(std::string &text) const;

		virtual const CProcessResult &run();

		virtual IObjectIA &operator = (const IObjectIA &a);
		virtual IObjetOp *operator + (const IObjetOp &a);
		virtual IObjetOp *operator - (const IObjetOp &a);

		virtual IObjetOp *operator + (IObjetOp *a);
		virtual IObjetOp *operator - (IObjetOp *a);		

		virtual const IObjectIA *operator[] (sint32) const;

		virtual void set(int,IObjectIA *);

		virtual CIteratorContener getIterator()
		{
			return CIteratorContener(new CListIterator(_List));
		}

		virtual CConstIteratorContener getConstIterator() const
		{
			return CConstIteratorContener((IBasicIterator *)(new CConstListIterator(_List)));
		}

//		virtual bool isTrue() const;

		IObjetOp *CGroupType::operator ! () const;
		void push(const IObjectIA *o);
		void pushFront(const IObjectIA *o);
		void cpy(const IObjectIA &o);
		const IObjectIA *pop();
		const IObjectIA *get() const;
		const IObjectIA *popFront();
		const IObjectIA *getFront() const;
		sint32 size() const;
		TListType findList(const IObjectIA &obj) const;
		TListType::const_iterator getBegin() const;
		TListType::const_iterator getEnd() const;
		TListType::iterator getBegin();
		TListType::iterator getEnd();
		const IObjectIA *find(const IObjectIA &obj) const;		
		void eraseAll(const IObjectIA &obj); 
		void erase(const IObjectIA *o); 
		void erase(const IObjectIA &obj);
		void erase(std::list<const IObjectIA *> &l);
		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;		
		virtual void save(NLMISC::IStream &os);
		virtual IObjetOp &neg();
		virtual void load(NLMISC::IStream &is);
		void clear();

		virtual ~CGroupType();
	};

	/**	
	  concret IBaseGroupType with a std::vector container.

	  * \author Portier Pierre	
	  * \author Chafik sameh
	  * \author Nevrax France
	  * \date 2000
	*/
	class CVectorGroupType: public IBaseGroupType
	{

	public:
		static const NLAIC::CIdentType IdVectorGroupType;
		
	public:
		//typedef of the std::vector constainer
		typedef std::vector<const IObjectIA *> TVectorType;
	private:				
		///std::vector for store IObjectIA object.
		TVectorType _Vector;		

	protected:
		TVectorType &getVector();

	public:
		///Construct a vector with n uninitialize momory unite. Programmer must initialize the vector list after. 
		CVectorGroupType(sint32 );
		CVectorGroupType();
		CVectorGroupType(const CVectorGroupType &g);

		virtual void getDebugString(std::string &text) const;

		virtual const CProcessResult &run();
		
		virtual IObjetOp *operator + (const IObjetOp &a) ;
		virtual IObjetOp *operator - (const IObjetOp &a) ;

		virtual IObjetOp *operator + (IObjetOp *a);
		virtual IObjetOp *operator - (IObjetOp *a);		

		virtual IObjectIA &operator = (const IObjectIA &a);

		virtual const IObjectIA *operator[] (sint32) const;

		virtual void set(int,IObjectIA *);

		// virtual bool isTrue() const;

		IObjetOp *CVectorGroupType::operator ! () const;
		void push(const IObjectIA *o);
		void pushFront(const IObjectIA *o);
		virtual CIteratorContener getIterator()
		{
			return CIteratorContener(new CVectorIterator(_Vector));
		}

		virtual CConstIteratorContener getConstIterator() const
		{
			return CConstIteratorContener((IBasicIterator *)(new CConstVectorIterator(_Vector)));
		}

		void setObject(sint32 i,IObjectIA *a)
		{
			_Vector[i] = a;
		}

		void cpy(const IObjectIA &o);
		const IObjectIA *pop();
		const IObjectIA *get() const;
		const IObjectIA *popFront();
		const IObjectIA *getFront() const;
		sint32 size() const;
		TVectorType findList(const IObjectIA &obj) const;
		TVectorType::const_iterator getBegin() const;
		TVectorType::const_iterator getEnd() const;
		TVectorType::iterator getBegin();
		TVectorType::iterator getEnd();
		const IObjectIA *find(const IObjectIA &obj) const;		
		void eraseAll(const IObjectIA &obj); 
		void erase(const IObjectIA *o); 
		void erase(const IObjectIA &obj);
		void erase(std::list<const IObjectIA *> &l);
		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;		
		virtual void save(NLMISC::IStream &os);
		virtual IObjetOp &neg();
		virtual void load(NLMISC::IStream &is);
		void clear();

		virtual sint32 getMethodIndexSize() const;
		virtual TQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);

		virtual ~CVectorGroupType();
	};
/*
	class CMapGroupType: public IBaseGroupType
	{

	public:
		static const NLAIC::CIdentType IdVectorGroupType;
		
	public:
		typedef std::vector<const IObjectIA *> TVectorType;
	private:				
		///std::vector for store IObjectIA object.
		std::map<IObjectIA *, IObjectIA *> _Map;

	protected:
		TVectorType &getVector();

	public:
		///Construct a vector with n uninitialize momory unite. Programmer must initialize the vector list after. 
		CMapGroupType(sint32 );
		CMapGroupType();
		CMapGroupType(const CMapGroupType &g);

		virtual void getDebugString(std::string &text) const;

		virtual const CProcessResult &run();

		virtual IObjetOp &operator += (const IObjetOp &a);
		virtual IObjetOp &operator -= (const IObjetOp &a);
		virtual IObjectIA &operator = (const IObjectIA &a);

		virtual const IObjectIA *operator[] (sint32) const;

		virtual void set(int,IObjectIA *);

		// virtual bool isTrue() const;

		IObjetOp *CMapGroupType::operator ! () const;
		void push(const IObjectIA *o);
		void pushFront(const IObjectIA *o);
		virtual CIteratorContener getIterator()
		{
			return CIteratorContener(new CMapIterator(_Map));
		}


		virtual CConstIteratorContener getConstIterator() const
		{
			return CConstIteratorContener((IBasicIterator *)(new CConstMapIterator(_Map)));
		}

		void setObject(sint32 i,IObjectIA *a)
		{
			_Map[i] = a;
		}

		void cpy(const IObjectIA &o);
		const IObjectIA *pop();
		const IObjectIA *get() const;
		const IObjectIA *popFront();
		const IObjectIA *getFront() const;
		sint32 size() const;
		TVectorType findList(const IObjectIA &obj) const;
		TVectorType::const_iterator getBegin() const;
		TVectorType::const_iterator getEnd() const;
		TVectorType::iterator getBegin();
		TVectorType::iterator getEnd();
		const IObjectIA *find(const IObjectIA &obj) const;		
		void eraseAll(const IObjectIA &obj); 
		void erase(const IObjectIA *o); 
		void erase(const IObjectIA &obj);
		void erase(std::list<const IObjectIA *> &l);
		virtual bool isEqual(const IBasicObjectIA &a) const;
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;		
		virtual void save(NLMISC::IStream &os);
		virtual IObjetOp &neg();
		virtual void load(NLMISC::IStream &is);
		void clear();

		virtual sint32 getMethodIndexSize() const;
		virtual TQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);

		virtual ~CMapGroupType();
	};
*/
}
#endif
