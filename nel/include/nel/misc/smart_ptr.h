/* smart_ptr.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: smart_ptr.h,v 1.5 2000/10/02 16:58:46 berenguier Exp $
 *
 * CSmartPtr and CRefPtr class.
 */

#ifndef NL_SMART_PTR_H
#define NL_SMART_PTR_H


#include <stdio.h>
#include "nel/misc/types_nl.h"


namespace NLMISC
{


// ***************************************************************************
/**
 * To use CSmartPtr or CRefPtr, derive from this class.
 * Your class doens't have to be virtual, or doesn't have to provide a virtual dtor.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRefCount 
{
public:
	// The instance handle.
	// Can't put those to private since must be used by CRefPtr (and friend doesn't work with template).
	struct	CPtrInfo
	{
		void	*Ptr;			// to know if the instance is valid.
		sint	RefCount;		// RefCount of ptrinfo (!= instance)
		CPtrInfo(void *p) {Ptr=p; RefCount=0;}
		// Just for internal use, to mark our Null pointer.
		CPtrInfo(char ) {Ptr=NULL; RefCount=0x7FFFFFFF;}
	};

	// OWN null for ref ptr. (Optimisations!!!)
	static	CPtrInfo	NullPtrInfo;
	friend struct		CPtrInfo;
		
public:
	// Can't put this to private since must be used by CSmartPtr (and friend doesn't work with template).
	// Provide incref()/decref() function doen't work since decref() can't do a delete this on a non virtual dtor.
	// So Ptr gestion can only be used via CSmartPtr.
    mutable	sint		crefs;	// The ref counter for SmartPtr use.
	mutable	CPtrInfo	*pinfo;	// The ref ptr for RefPtr use.
	
	/// Default constructor init crefs to 0.
    CRefCount() { crefs = 0; pinfo=&NullPtrInfo; }
	/// operator= must NOT copy crefs/pinfo!!
	CRefCount &operator=(const CRefCount &) {return *this;}
};



// ***************************************************************************
// For debug only.
//#define	SMART_TRACE(_s)	((void)0)
//#define	REF_TRACE(_s)	((void)0)
#define	SMART_TRACE(_s)	printf("%s: %d \n", _s, Ptr?Ptr->crefs:0)
#define	REF_TRACE(_s)	printf("%s: %d \n", _s, pinfo!=&CRefCount::NullPtrInfo?pinfo->RefCount:0)


/**
 * Standard SmartPtr class. T Must derive from CRefCount.
 * The ref counter cannot be put directly in the smartptr since inheritance must be supported.
 * Here, if A is a base class of B, Pa and Pb are smartptr of a and b respectively, then \c Pa=Pb; is a valid operation.
 * But, doing this, you may ensure that you have a virtual dtor(), since dtor() Pa may call ~A() (or you may ensure that Pa
 * won't destruct A, which it sound much more as a normal pointer :) ).
 *
 * Sample:
 *\code
	class A : public CRefCount
	{
	public:
		A() {puts("A()");}
		virtual ~A() {puts("~A()");}
	};


	class B : public A
	{
	public:
		B() {puts("B()");}
		~B() {puts("~B()");}
	};


	void	testPtr()
	{
		CSmartPtr<A>	a0,a1,a2;
		CSmartPtr<B>	b0;

		a0= new A;
		a1= a0;
		a1= new A;
		a2= a1;
		a1=NULL;
		b0= new B;
		a0=b0;

		printf("%d\n", (A*)NULL==a0);
		printf("%d\n", b0!=a0);
		printf("%d\n", (A*)NULL==a1);
		printf("%d\n", a2!=a0);
	}
 *\endcode
 *
 * SmartPtr are NOT compatible with RefPtr. A ptr may be link to a CRefPtr OR a CSmartPtr, but not two at the same time.
 * Sample:
 *\code
	void	foo()
	{
		A				*p;
		CSmartPtr<A>	sp;
		CRefPtr<A>		rp;

		p= new A;
		sp= p;		// OK. p is now owned by sp and will be deleted by sp.
		rp= p;		// Error!!! (compile but will lead too funny crashs).
	}
 \endcode
 *
 * \sa CRefPtr
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
template <class T>
class CSmartPtr 
{
    T* Ptr;
public:

	/// Init a NULL Ptr.
    CSmartPtr() { Ptr=NULL; SMART_TRACE("ctor()"); }
	/// Attach a ptr to a SmartPtr.
    CSmartPtr(T* p) { Ptr=p; if(Ptr) Ptr->crefs++; SMART_TRACE("ctor(T*)"); }
	/// Copy constructor.
    CSmartPtr(CSmartPtr &copy) { Ptr=copy.Ptr; if(Ptr) Ptr->crefs++; SMART_TRACE("ctor(Copy)"); }
	/// Release the SmartPtr.
    ~CSmartPtr();


	/// Cast operator.
    operator T*(void) const { SMART_TRACE("castT*()"); return Ptr; }
	/// Cast operator. Doesn't check NULL.
    T* operator->(void) const { SMART_TRACE("ope->()"); return Ptr; }
	/// Cast operator. Doesn't check NULL.
    T& operator*(void) const { SMART_TRACE("ope*()"); return *Ptr; }

	/// operator=. Giving a NULL pointer is a valid operation.
    CSmartPtr& operator=(T* p);
	/// operator=. Giving a NULL pointer is a valid operation.
    CSmartPtr& operator=(CSmartPtr<T> &p);

	// No need to do any operator==. Leave the work to cast  operator T*(void).
};



// ***************************************************************************
/**
 * CRefPtr: an Advanced SmartPtr class. T Must derive from CRefCount.
 * CRefPtr works like a CSmartPtr and provide the same behavior, except for the kill() method.
 * If you use CRefPtr, you can use the kill() method do delete the object. All other CRefPtr which point 
 * to it can know if it has been deleted.
 *
 * SmartPtr are NOT compatible with RefPtr. A ptr may be link to a CRefPtr OR a CSmartPtr, but not two at the same time.
 * Sample:
 *\code
	void	foo()
	{
		A				*p;
		CSmartPtr<A>	sp;
		CRefPtr<A>		rp;

		p= new A;
		sp= p;		// OK. p is now owned by sp and will be deleted by sp.
		rp= p;		// Error!!! (compile but will lead too funny crashs).
	}
 \endcode
 * \sa CSmartPtr
 */
template <class T> 
class CRefPtr
{
private:
	CRefCount::CPtrInfo		*pinfo;		// A ptr to the handle of the object.
    mutable T				*Ptr;		// A cache for pinfo->Ptr. UseFull to speed up  ope->()  and  ope*()

	void	unRef()  const;				// Just release the handle pinfo, but do not update pinfo/Ptr, if deleted.

public:

	/// Init a NULL Ptr.
    CRefPtr();
	/// Attach a ptr to a RefPtr.
    CRefPtr(T *v);
	/// Copy constructor.
    CRefPtr(const CRefPtr &copy);
	/// Release the RefPtr.
    ~CRefPtr(void);


	/// Cast operator. Check if the object has been deleted somewhere, and return NULL if this is the case.
	operator T*()	const;
	/// Cast operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
    T& operator*(void)	const;
	/// Cast operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
    T* operator->(void)	const;


	/// operator=. Giving a NULL pointer is a valid operation.
	CRefPtr& operator=(T *v);
	/// operator=. Giving a NULL pointer is a valid operation.
    CRefPtr& operator=(const CRefPtr &copy);


	/**
	 * kill/delete the object pointed by the pointer, and inform the other RefPtr of this.
	 * RefPtr which point to the same object could know if the object is valid, by just testing it (
	 * by an implicit call to the cast operator to T*). But any calls to operator->() or operator*() will have 
	 * unpredictible effects (may crash... :) ).
	 */
	void	kill();

	// No need to do any operator==. Leave the work to cast  operator T*(void).
};





// ***************************************************************************
// ***************************************************************************
// Implementation.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
#ifdef NL_OS_WINDOWS
#define	SMART_INLINE __forceinline
#else
#define	SMART_INLINE inline 
#endif


// ***************************************************************************
template<class T>
inline CSmartPtr<T>::~CSmartPtr(void) 
{ 
	SMART_TRACE("dtor()");

    if(Ptr)
	{
		if (--(Ptr->crefs) == 0)
			delete Ptr;
		Ptr=NULL;
	}
}
template<class T>    
SMART_INLINE CSmartPtr<T>& CSmartPtr<T>::operator=(T* p)
{
	SMART_TRACE("ope=(T*)Start");

	// Implicit manage auto-assignation.
    if(p)
		p->crefs++;
    if(Ptr)
	{
		if (--(Ptr->crefs) == 0)
			delete Ptr;
	}
	Ptr = p;

	SMART_TRACE("ope=(T*)End");

	return *this;
}
template<class T>    
SMART_INLINE CSmartPtr<T>& CSmartPtr<T>::operator=(CSmartPtr<T> &p)
{
	return operator=(p.Ptr);
}



// ***************************************************************************


//===========================================================
template<class T>    
SMART_INLINE void	CRefPtr<T>::unRef() const
{
	pinfo->RefCount--;
	if(pinfo->RefCount==0)
	{
		if(pinfo->Ptr)
			delete (T*)pinfo->Ptr;

		// We may be in the case that this==NullPtrInfo, and our NullPtrInfo has done a total round. Test it.
		if(pinfo!=&CRefCount::NullPtrInfo)
			delete pinfo;
		else
		{
			// Reset it to a middle round.
			pinfo->RefCount= 0x7FFFFFFF;
		}
	}
}


//===========================================================
// Cons - dest.
template <class T> inline CRefPtr<T>::CRefPtr() 
{ 
	pinfo= &CRefCount::NullPtrInfo;
	Ptr= NULL;

	REF_TRACE("Smart()");
}
template <class T> inline CRefPtr<T>::CRefPtr(T *v)
{
	Ptr= v;
    if(v)
	{
		if(v->pinfo==&CRefCount::NullPtrInfo) v->pinfo=new CRefCount::CPtrInfo(v);
		pinfo=v->pinfo;
		// v is now used by this.
		pinfo->RefCount++;
	}
	else
		pinfo= &CRefCount::NullPtrInfo;

	REF_TRACE("Smart(T*)");
}
template <class T> inline CRefPtr<T>::CRefPtr(const CRefPtr &copy)
{
	pinfo=copy.pinfo;
	pinfo->RefCount++;
	Ptr= (T*)pinfo->Ptr;

	REF_TRACE("SmartCopy()");
}
template <class T> inline CRefPtr<T>::~CRefPtr(void)
{
	REF_TRACE("~Smart()");

	unRef();
	pinfo= &CRefCount::NullPtrInfo;
	Ptr= NULL;
}

//===========================================================
// Operators=.
template <class T> CRefPtr<T> &CRefPtr<T>::operator=(T *v) 
{
	REF_TRACE("ope=(T*)Start");


	Ptr= v;
	if(v)
	{
		// First, ensure that we work with a valid Ptr (ie, create his pinfo, if never done).
		if(v->pinfo==&CRefCount::NullPtrInfo) v->pinfo=new CRefCount::CPtrInfo(v);
		// The auto equality test is implicitly done by upcounting first "v", then downcounting "this".
		v->pinfo->RefCount++;
		unRef();
		pinfo= v->pinfo;
	}
	else
	{
		unRef();
		pinfo= &CRefCount::NullPtrInfo;
	}


	REF_TRACE("ope=(T*)End");

	return *this;
}
template <class T> CRefPtr<T> &CRefPtr<T>::operator=(const CRefPtr &copy) 
{
	REF_TRACE("ope=(Smart)Start");

	// The auto equality test is implicitly done by upcounting first "copy", then downcounting "this".
	copy.pinfo->RefCount++;
	unRef();
	pinfo=copy.pinfo;
	Ptr= (T*)pinfo->Ptr;

	REF_TRACE("ope=(Smart)End");
	return *this;
}


//===========================================================
// Operations.
template <class T> void	CRefPtr<T>::kill()
{
	REF_TRACE("SmartKill");

	// If pinfo is not NullPtrInfo, or if p exist yet, delete him.
	if(pinfo->Ptr)
	{
		delete (T*)pinfo->Ptr;
		pinfo->Ptr=NULL;
	}

	unRef();
	pinfo= &CRefCount::NullPtrInfo;
	Ptr= NULL;
}


//===========================================================
// Cast.
template <class T> inline CRefPtr<T>::operator T*()	const 
{
	REF_TRACE("SmartCast T*()");

	// Refresh Ptr.
	Ptr= (T*)pinfo->Ptr;
	return Ptr;
}


//===========================================================
// Operators.
template <class T> inline T& CRefPtr<T>::operator*(void)  const
{ 
	REF_TRACE("Smart *()");
	return *Ptr; 
}
template <class T> inline T* CRefPtr<T>::operator->(void) const
{ 
	REF_TRACE("Smart ->()");
	return Ptr;  
}



// ***************************************************************************
#undef	SMART_INLINE
#undef	SMART_TRACE
#undef	REF_TRACE



}


#endif // NL_SMART_PTR_H

/* End of smart_ptr.h */
