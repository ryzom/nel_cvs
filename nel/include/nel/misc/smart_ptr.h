/* smart_ptr.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: smart_ptr.h,v 1.2 2000/09/21 09:12:35 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_SMART_PTR_H
#define NL_SMART_PTR_H


#include <stdio.h>
#include "nel/misc/types_nl.h"


namespace NLMISC
{


// =====================================================================================
/**
 * To use CSmartPtr, derive from this class.
 * Your class doens't have to be virtual, or doesn't have to provide a virtual dtor.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRefCount 
{
public:
	// Can't put this to private since must be used by CSmartPtr (and friend doesn't work with template).
	// Provide incref()/decref() function doen't work since decref() can't do a delete this on a non virtual dtor.
	// So Ptr gestion can only be used via CSmartPtr.
    sint crefs;
	/// Default constructor init crefs to 0.
    CRefCount() { crefs = 0; }
	/// operator= must NOT copy crefs!!
	CRefCount &operator=(const CRefCount &) {return *this;}
};



// =====================================================================================
// For debug only.
#define	SMART_TRACE(_s)	((void)0)
//#define	SMART_TRACE(_s)	printf("%s: %d\n", _s, Ptr?Ptr->crefs:0)


/**
 * SmartPtr class. T Must derive from CRefCount.
 * The ref counter cannot be put in the smartptr if inheritance must be supported.
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
		getch();
		b0= new B;
		a0=b0;
	}
 *\endcode
 *
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
    CSmartPtr(const CSmartPtr &copy) { Ptr=copy.Ptr; if(Ptr) Ptr->crefs++; SMART_TRACE("ctor(Copy)"); }
	/// Release the SmartPtr.
    ~CSmartPtr();


	/// Cast operator.
    operator T*(void) { SMART_TRACE("castT*()"); return Ptr; }
	/// Cast operator. Doesn't check NULL.
    T* operator->(void) { SMART_TRACE("ope->()"); return Ptr; }
	/// Cast operator. Doesn't check NULL.
    T& operator*(void) { SMART_TRACE("ope*()"); return *Ptr; }

	/// operator=. Giving a NULL pointer is a valid operation.
    CSmartPtr& operator=(T* p);
	/// operator=. Giving a NULL pointer is a valid operation.
    CSmartPtr& operator=(CSmartPtr<T> &p);

	/// Compare two pointers.
    bool operator==(CSmartPtr<T> &p) {SMART_TRACE("ope==()"); return Ptr==p.Ptr;}
};




// =====================================================================================
// Implementation.


#ifdef NL_OS_WINDOWS
#define	SMART_INLINE __forceinline
#else
#define	SMART_INLINE inline 
#endif


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


#undef	SMART_INLINE
#undef	SMART_TRACE


}


#endif // NL_SMART_PTR_H

/* End of smart_ptr.h */
