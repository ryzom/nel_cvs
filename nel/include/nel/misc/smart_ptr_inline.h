/** \file smart_ptr_inline.h
 * SmartPtr class inline definitions.
 *
 * $Id: smart_ptr_inline.h,v 1.7 2004/09/07 19:12:27 boucher Exp $
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


#ifndef NL_SMARTPTR_INLINE_H
#define NL_SMARTPTR_INLINE_H

namespace	NLMISC
{



// ***************************************************************************
#ifdef NL_OS_WINDOWS
#define	SMART_INLINE __forceinline
#else
#define	SMART_INLINE inline 
#endif


// ***************************************************************************
inline CRefCount::~CRefCount()
{
	// This is the destruction of the objet.
#ifdef NL_DEBUG
	nlassert(crefs==0);
#endif

	// If a CRefPtr still points on me...
	if(!pinfo->IsNullPtrInfo)
	{
		// inform them of my destruction.
		pinfo->Ptr= NULL;
	}
}


// ***************************************************************************
// ***************************************************************************
// CSmartPtr.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
template<class T>
inline CSmartPtr<T>::~CSmartPtr(void) 
{ 
	SMART_TRACE("dtor()");

    if(Ptr)
	{
#ifdef NL_DEBUG
		nlassert(Ptr->crefs>=0);
#endif
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
SMART_INLINE CSmartPtr<T>& CSmartPtr<T>::operator=(const CSmartPtr &p)
{
	return operator=(p.Ptr);
}
template<class T>    
SMART_INLINE bool CSmartPtr<T>::operator<(const CSmartPtr &p) const
{
	return Ptr<p.Ptr;
}



// ***************************************************************************
// ***************************************************************************
// CRefPtr.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
template<class T>    
SMART_INLINE void	CRefPtr<T>::unRef() const
{
	pinfo->RefCount--;
	if(pinfo->RefCount==0)
	{
		// In CRefPtr, Never delete the object.

		// We may be in the case that this==NullPtrInfo, and our NullPtrInfo has done a total round. Test it.
		if(pinfo->IsNullPtrInfo)
		{
			// This should not happens, but I'm not sure :) ...
			// Reset the NullPtrInfo to a middle round.
			pinfo->RefCount= 0x7FFFFFFF;
		}
		else
		{
			// If the CRefPtr still point to a valid object.
			if(pinfo->Ptr)
			{
				// Inform the Object that no more CRefPtr points on it.
				((T*)(pinfo->Ptr))->pinfo= &CRefCount::NullPtrInfo;
			}
			// Then delete the pinfo.
			delete pinfo;
		}

	}
}


// ***************************************************************************
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
		// If no CRefPtr handles v, create a pinfo ref...
		if(v->pinfo->IsNullPtrInfo)
			v->pinfo=new CRefCount::CPtrInfo(v);
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

// ***************************************************************************
// Operators=.
template <class T> CRefPtr<T> &CRefPtr<T>::operator=(T *v) 
{
	REF_TRACE("ope=(T*)Start");


	Ptr= v;
	if(v)
	{
		// If no CRefPtr handles v, create a pinfo ref...
		if(v->pinfo->IsNullPtrInfo)
			v->pinfo=new CRefCount::CPtrInfo(v);
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
	// Must Refresh the ptr.
	Ptr= (T*)pinfo->Ptr;

	REF_TRACE("ope=(Smart)End");
	return *this;
}


// ***************************************************************************
// Operations.
template <class T> void	CRefPtr<T>::kill()
{
	REF_TRACE("SmartKill");

	T	*ptr= (T*)pinfo->Ptr;

	// First, release the refptr.
	unRef();
	pinfo= &CRefCount::NullPtrInfo;
	Ptr= NULL;

	// Then delete the pointer.
	if(ptr)
		delete ptr;
}


// ***************************************************************************
// Cast.
template <class T> inline CRefPtr<T>::operator T*()	const 
{
	REF_TRACE("SmartCast T*()");

	// Refresh Ptr.
	Ptr= (T*)pinfo->Ptr;
	return Ptr;
}


// ***************************************************************************
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



} // NLMISC


#endif // NL_SMARTPTR_INLINE_H

