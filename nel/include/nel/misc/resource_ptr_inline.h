/** \file resource_ptr.h
 * CResourcePtr class inline definitions.
 *
 * $Id: resource_ptr_inline.h,v 1.1 2004/11/17 16:12:01 corvazier Exp $
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


#ifndef NL_RESOURCE_PTR_INLINE_H
#define NL_RESOURCE_PTR_INLINE_H

namespace	NLMISC
{



// ***************************************************************************
#ifdef NL_OS_WINDOWS
#define	SMART_INLINE __forceinline
#else
#define	SMART_INLINE inline 
#endif


// ***************************************************************************
// ***************************************************************************
// CResourcePtr.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> SMART_INLINE void	CResourcePtr<TPtr, TKey, TResourceFinder>::unRef() const
{
	pinfo->RefCount--;
	if(pinfo->RefCount==0)
	{
		// In CResourcePtr, Never delete the object.

		// We may be in the case that this==NullPtrInfo, and our NullPtrInfo has done a total round. Test it.
		if(pinfo->IsNullPtrInfo)
		{
			// This should not happens, but I'm not sure :) ...
			// Reset the NullPtrInfo to a middle round.
			pinfo->RefCount= 0x7FFFFFFF;
		}
		else
		{
			// If the CResourcePtr still point to a valid object.
			if(pinfo->Ptr)
			{
				// Inform the Object that no more CResourcePtr points on it.
				((TPtr*)(pinfo->Ptr))->pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);
			}
			// Then delete the pinfo.
			delete pinfo;
		}
	}
}


// ***************************************************************************
// Cons - dest.
template <class TPtr, class TKey, class TResourceFinder> inline CResourcePtr<TPtr, TKey, TResourceFinder>::CResourcePtr() 
{ 
	pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);
	Ptr= NULL;

	REF_TRACE("Smart()");
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline CResourcePtr<TPtr, TKey, TResourceFinder>::CResourcePtr(const TKey &key)
{
	Key = key;
	Ptr = (TPtr*)TResourceFinder::getResource(Key);
    if(Ptr)
	{
		// If no CResourcePtr handles v, create a pinfo ref...
		if(Ptr->pinfo->IsNullPtrInfo)
			Ptr->pinfo=new CRefCount::CPtrInfo(Ptr);
		pinfo=Ptr->pinfo;
		// v is now used by this.
		pinfo->RefCount++;
	}
	else
		pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);

	REF_TRACE("Smart(TPtr*)");
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline CResourcePtr<TPtr, TKey, TResourceFinder>::CResourcePtr(const CResourcePtr &copy)
{
	pinfo=copy.pinfo;
	pinfo->RefCount++;
	Ptr= (TPtr*)pinfo->Ptr;
	Key= copy.Key;

	REF_TRACE("SmartCopy()");
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline CResourcePtr<TPtr, TKey, TResourceFinder>::~CResourcePtr(void)
{
	REF_TRACE("~Smart()");

	unRef();
	pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);
	Ptr= NULL;
}

// ***************************************************************************
// Operators=.
template <class TPtr, class TKey, class TResourceFinder> CResourcePtr<TPtr, TKey, TResourceFinder> &CResourcePtr<TPtr, TKey, TResourceFinder>::operator=(const TKey &key)
{
	REF_TRACE("ope=(TPtr*)Start");

	Key = key;
	Ptr = (TPtr*)TResourceFinder::getResource(Key);
	if(Ptr)
	{
		// If no CResourcePtr handles v, create a pinfo ref...
		if(Ptr->pinfo->IsNullPtrInfo)
			Ptr->pinfo=new CRefCount::CPtrInfo(Ptr);
		// The auto equality test is implicitly done by upcounting first "Ptr", then downcounting "this".
		Ptr->pinfo->RefCount++;
		unRef();
		pinfo= Ptr->pinfo;
	}
	else
	{
		unRef();
		pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);
	}


	REF_TRACE("ope=(TPtr*)End");

	return *this;
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> CResourcePtr<TPtr, TKey, TResourceFinder> &CResourcePtr<TPtr, TKey, TResourceFinder>::operator=(const CResourcePtr &copy) 
{
	REF_TRACE("ope=(Smart)Start");

	// The auto equality test is implicitly done by upcounting first "copy", then downcounting "this".
	copy.pinfo->RefCount++;
	unRef();
	pinfo=copy.pinfo;
	// Must Refresh the ptr and the key.
	Ptr= (TPtr*)pinfo->Ptr;
	Key= copy.Key;

	REF_TRACE("ope=(Smart)End");
	return *this;
}

// ***************************************************************************
// Operations.
/* Not needed for the moment in CResourcePtr
template <class TPtr, class TKey, class TResourceFinder> void	CResourcePtr<T>::kill()
{
	REF_TRACE("SmartKill");

	T	*ptr= (T*)pinfo->Ptr;

	// First, release the refptr.
	unRef();
	pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);
	Ptr= NULL;

	// Then delete the pointer.
	if(ptr)
		delete ptr;
}*/


// ***************************************************************************
// Cast.
template <class TPtr, class TKey, class TResourceFinder> inline CResourcePtr<TPtr, TKey, TResourceFinder>::operator TPtr*()	const 
{
	REF_TRACE("SmartCast TPtr*()");

	// Refresh the Ptr.
	Ptr= (TPtr*)pinfo->Ptr;
	if (pinfo != static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo))
	{
		// Does the pointer has been deleted ?
		if (Ptr == NULL)
		{
			REF_TRACE("SmartCast TPtr*() has been deleted, get a new one");
			// Try to get it
			Ptr = (TPtr*)TResourceFinder::getResource(Key);
		}
		else
			REF_TRACE("SmartCast TPtr*() has not been deleted");

	}
	else
		REF_TRACE("SmartCast TPtr*() is NULL");

	return Ptr;
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline TPtr* CResourcePtr<TPtr, TKey, TResourceFinder>::getPtr() const
{
	return static_cast<TPtr*>(*this);
}

// ***************************************************************************
// Operators.
template <class TPtr, class TKey, class TResourceFinder> inline TPtr& CResourcePtr<TPtr, TKey, TResourceFinder>::operator*(void)  const
{ 
	REF_TRACE("Smart *()");
	return *Ptr; 
}
template <class TPtr, class TKey, class TResourceFinder> inline TPtr* CResourcePtr<TPtr, TKey, TResourceFinder>::operator->(void) const
{ 
	REF_TRACE("Smart ->()");
	return Ptr;  
}


// ***************************************************************************
// ***************************************************************************
// CStaticResourcePtr.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// Cons - dest.
template <class TPtr, class TKey, class TResourceFinder> inline CStaticResourcePtr<TPtr, TKey, TResourceFinder>::CStaticResourcePtr() 
{ 
	Ptr= NULL;

	REF_TRACE("Smart()");
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline CStaticResourcePtr<TPtr, TKey, TResourceFinder>::CStaticResourcePtr(const TKey &key)
{
	Ptr = (TPtr*)TResourceFinder::getResource(key);

	REF_TRACE("Smart(TPtr*)");
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline CStaticResourcePtr<TPtr, TKey, TResourceFinder>::CStaticResourcePtr(const CStaticResourcePtr &copy)
{
	Ptr= copy.Ptr;

	REF_TRACE("SmartCopy()");
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline CStaticResourcePtr<TPtr, TKey, TResourceFinder>::~CStaticResourcePtr(void)
{
	REF_TRACE("~Smart()");

	Ptr= NULL;
}

// ***************************************************************************
// Operators=.
template <class TPtr, class TKey, class TResourceFinder> CStaticResourcePtr<TPtr, TKey, TResourceFinder> &CStaticResourcePtr<TPtr, TKey, TResourceFinder>::operator=(const TKey &key)
{
	REF_TRACE("ope=(TPtr*)Start");

	Ptr = (TPtr*)TResourceFinder::getResource(key);

	REF_TRACE("ope=(TPtr*)End");

	return *this;
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> CStaticResourcePtr<TPtr, TKey, TResourceFinder> &CStaticResourcePtr<TPtr, TKey, TResourceFinder>::operator=(const CStaticResourcePtr &copy) 
{
	REF_TRACE("ope=(Smart)Start");

	Ptr= copy.Ptr;

	REF_TRACE("ope=(Smart)End");
	return *this;
}

// ***************************************************************************
// Operations.
/* Not needed for the moment in CStaticResourcePtr
template <class TPtr, class TKey, class TResourceFinder> void	CStaticResourcePtr<T>::kill()
{
	REF_TRACE("SmartKill");

	T	*ptr= (T*)pinfo->Ptr;

	// First, release the refptr.
	unRef();
	pinfo= static_cast<CRefCount::CPtrInfo*>(&CRefCount::NullPtrInfo);
	Ptr= NULL;

	// Then delete the pointer.
	if(ptr)
		delete ptr;
}*/


// ***************************************************************************
// Cast.
template <class TPtr, class TKey, class TResourceFinder> inline CStaticResourcePtr<TPtr, TKey, TResourceFinder>::operator TPtr*()	const 
{
	REF_TRACE("SmartCast TPtr*()");

	return Ptr;
}

// ***************************************************************************
template <class TPtr, class TKey, class TResourceFinder> inline TPtr* CStaticResourcePtr<TPtr, TKey, TResourceFinder>::getPtr() const
{
	return static_cast<TPtr*>(*this);
}

// ***************************************************************************
// Operators.
template <class TPtr, class TKey, class TResourceFinder> inline TPtr& CStaticResourcePtr<TPtr, TKey, TResourceFinder>::operator*(void)  const
{ 
	REF_TRACE("Smart *()");
	return *Ptr; 
}
template <class TPtr, class TKey, class TResourceFinder> inline TPtr* CStaticResourcePtr<TPtr, TKey, TResourceFinder>::operator->(void) const
{ 
	REF_TRACE("Smart ->()");
	return Ptr;  
}


// ***************************************************************************
#undef	SMART_INLINE



} // NLMISC


#endif // NL_RESOURCE_PTR_INLINE_H

