/** \file smart_ptr.h
 * CSmartPtr and CRefPtr class.
 *
 * $Id: smart_ptr.h,v 1.16 2003/06/10 08:54:35 ledorze Exp $
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

#ifndef NL_SMART_PTR_H
#define NL_SMART_PTR_H


#include "nel/misc/types_nl.h"

#include <stdio.h>


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
		// For fu... dll problems, must use a flag to mark NullPtrInfo.
		bool	IsNullPtrInfo;

		CPtrInfo(void *p) {Ptr=p; RefCount=0; IsNullPtrInfo=false;}
		// Just for internal use, to mark our Null pointer.
		CPtrInfo(char ) {Ptr=NULL; RefCount=0x7FFFFFFF; IsNullPtrInfo=true;}
	};

	// OWN null for ref ptr. (Optimisations!!!)
	static	CPtrInfo	NullPtrInfo;
	friend struct		CPtrInfo;
	
	// for special case use only.
	sint	getRefCount()
	{
		return	crefs;
	}

public:
	// Can't put this to private since must be used by CSmartPtr (and friend doesn't work with template).
	// Provide incref()/decref() function doen't work since decref() can't do a delete this on a non virtual dtor.
	// So Ptr gestion can only be used via CSmartPtr.
    mutable	sint		crefs;	// The ref counter for SmartPtr use.
	mutable	CPtrInfo	*pinfo;	// The ref ptr for RefPtr use.
	
	/// Destructor which release pinfo if necessary.
	~CRefCount();
	/// Default constructor init crefs to 0.
    CRefCount() { crefs = 0; pinfo=&NullPtrInfo; }
	/// operator= must NOT copy crefs/pinfo!!
	CRefCount &operator=(const CRefCount &) {return *this;}
	/// copy cons must NOT copy crefs/pinfo!!
	CRefCount(const CRefCount &) {crefs = 0; pinfo=&NullPtrInfo;}
};



// ***************************************************************************
// For debug only.
#define	SMART_TRACE(_s)	((void)0)
#define	REF_TRACE(_s)	((void)0)
//#define	SMART_TRACE(_s)	printf("%s: %d \n", _s, Ptr?Ptr->crefs:0)
//#define	REF_TRACE(_s)	printf("%s: %d \n", _s, pinfo!=&CRefCount::NullPtrInfo?pinfo->RefCount:0)


/**
 * Standard SmartPtr class. T Must derive from CRefCount.
 * Once a normal ptr is assigned to a SmartPtr, the smartptr will own this pointer, and delete it when no other smartptr
 * reference the object (with a reference couting scheme). The following code works, since the object himself must herit 
 * from CRefCount, and so hold the refcount.
 * \code
	CSmartPtr<A>	a0, a1;
	A				*p0;
	a0= new A;	// Ok. RefCount==1.
	p0= a0;		// Ok, cast operator. object still owned by a0.
	a1= p0;		// Ok!! RefCount==2. Object owned by a0 and a1;
	// At destruction, a1 unref(), then a0 unref() and delete the object.
 \endcode
 *
 * The ref counter cannot be put directly in the smartptr since the preceding behavior must be supported and inheritance must be supported too.
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
 * SmartPtr are compatible with RefPtr. A ptr may be link to a CRefPtr and a CSmartPtr. As example, when the CSmartPtr
 * will destroy him, CRefPtr will be informed...
 * Sample:
 *\code
	void	foo()
	{
		A				*p;
		CSmartPtr<A>	sp;
		CRefPtr<A>		rp;

		p= new A;
		sp= p;		// OK. p is now owned by sp and will be deleted by sp.
		rp= p;		// OK. rp handle p.
		sp= NULL;	// Destruction. p deleted. rp automatically informed.
		p= rp;		// result: p==NULL.
	}
 \endcode
 *
 * \b PERFORMANCE \b WARNING! operator=() are about 10 times slower than normal pointers.
 * For local use, prefer cast the smartptr to a normal Ptr.
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
    CSmartPtr(const CSmartPtr &copy) { Ptr=copy.Ptr; if(Ptr) Ptr->crefs++; SMART_TRACE("ctor(Copy)"); }
	/// Release the SmartPtr.
    ~CSmartPtr();


	/// Cast operator.
    operator T*(void) const { SMART_TRACE("castT*()"); return Ptr; }
	/// Indirection operator. Doesn't check NULL.
    T& operator*(void) const { SMART_TRACE("ope*()"); return *Ptr; }
	/// Selection operator. Doesn't check NULL.
    T* operator->(void) const { SMART_TRACE("ope->()"); return Ptr; }
	/// returns if there's no object pointed by this SmartPtr.
	bool	isNull	() const { return Ptr==NULL; }

	/// operator=. Giving a NULL pointer is a valid operation.
    CSmartPtr& operator=(T* p);
	/// operator=. Giving a NULL pointer is a valid operation.
    CSmartPtr& operator=(const CSmartPtr &p);
	/// operator<. Compare the pointers.
    bool operator<(const CSmartPtr &p) const;

	sint getNbRef() { if(Ptr) return Ptr->crefs; else return 0; }
	// No need to do any operator==. Leave the work to cast  operator T*(void).
};



// ***************************************************************************
/**
 * CRefPtr: an handle on a ptr. T Must derive from CRefCount.
 * If you use CRefPtr, you can kill the object simply by calling delete (T*)RefPtr, or the kill() method. All other CRefPtr which 
 * point to it can know if it has been deleted. (but you must be sure that this ptr is not handle by a SmartPtr, of course...)
 *
 * SmartPtr are compatible with RefPtr. A ptr may be link to a CRefPtr and a CSmartPtr. As example, when the CSmartPtr
 * will destroy him, CRefPtr will be informed...
 * Sample:
 *\code
	void	foo()
	{
		A				*p;
		CSmartPtr<A>	sp;
		CRefPtr<A>		rp;

		p= new A;
		sp= p;		// OK. p is now owned by sp and will be deleted by sp.
		rp= p;		// OK. rp handle p.
		sp= NULL;	// Destruction. p deleted. rp automatically informed.
		if(rp==NULL)
			thisIsGood();	// rp==NULL.
	}
 \endcode
 *
 * \b PERFORMANCE \b WARNING! operator=() are about 10 times slower than normal pointers. So use them wisely.
 * For local use, prefer cast the refptr to a normal Ptr.
 * Also, an object used with a CRefPtr will allocate a small PtrInfo (one only per object, not per ptr).
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
	/// Indirection operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
    T& operator*(void)	const;
	/// Selection operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
    T* operator->(void)	const;


	/// operator=. Giving a NULL pointer is a valid operation.
	CRefPtr& operator=(T *v);
	/// operator=. Giving a NULL pointer is a valid operation.
    CRefPtr& operator=(const CRefPtr &copy);


	/**
	 * kill/delete the object pointed by the pointer, and inform the other RefPtr of this.
	 * "rp.kill()" and "delete (T*)rp" do the same thing, except that rp NULLity is updated with kill().
	 * RefPtr which point to the same object could know if the object is valid, by just testing it (
	 * by an implicit call to the cast operator to T*). But any calls to operator->() or operator*() will have 
	 * unpredictible effects (may crash... :) ).
	 */
	void	kill();


	// No need to do any operator==. Leave the work to cast  operator T*(void).
};



}


// ***************************************************************************
// ***************************************************************************
// Implementation.
// ***************************************************************************
// ***************************************************************************


#include "smart_ptr_inline.h"
#undef	SMART_TRACE
#undef	REF_TRACE



#endif // NL_SMART_PTR_H

/* End of smart_ptr.h */
