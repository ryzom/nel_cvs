/** \file ps_attrib.h
 * <File description>
 *
 * $Id: ps_attrib.h,v 1.2 2001/06/25 13:58:25 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_PS_ATTRIB_H
#define NL_PS_ATTRIB_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/3d/animation_time.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"


namespace NL3D {


/**
 * This class is intended to store an attribute list in a located or in a located bindable
 * such as speed, color and so on. It is important to remember that a located holds all instance of object of
 * one type (force, emitter, particles or both...). 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
	
template <typename T> class CPSAttrib
{
protected:
	// the container type is likely to change depending on memory requirement
	typedef std::vector<T> TContType ;
	typedef T type ;
public:

	/// Constructor
	CPSAttrib(uint32 maxNbInstances = DefaultMaxLocatedInstance);

	/// resize the attributes tab. This tells what is the mx number of element in this tab, but don't add elements
	void resize(uint32 nbInstances) ;

	/// resize the attribute tab, but fill the whole tab of with a given value
	void resizeNFill(uint32 nbInstances) ;
	 
	/// get a const reference on an attribute instance
	const T &operator[](uint32 index) const { nlassert(index < _Size) ; return _Tab[index] ; }
	/// get a reference on an attribute instance
	T &operator[](uint32 index) { nlassert(index < _Size) ; return _Tab[index] ; }

	/// insert 


	/// an iterator on the datas
	typedef TContType::iterator iterator ;
	/// a const iterator on the datas
	typedef TContType::const_iterator const_iterator ;



	/// get an iterator at the beginning of the container
	iterator begin(void) { return _Tab.begin() ; }

	/// get an iterator at the end of the container
	iterator end(void) { return _Tab.end() ; }	

	/// get an iterator at the beginning of the container
	const_iterator begin(void) const { return _Tab.begin() ; }

	/// get an iterator at the end of the container
	const_iterator end(void) const { return _Tab.end() ; }	
	

	/**
	 * create a new object in the tab 
	 * \return the index if there were enough room for it or -1 else
	 */
	sint32 insert(const T &t = T() ) ;


	/// return the number of instance in the container
	uint32 getSize(void) const { return _Size ; }

	/// return the max number of instance in the container
	uint32 getMaxSize(void) const { return _MaxSize ; }


	/// remove an object from the tab
	void remove(uint32 index) ; 

	/// Serialization method
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	/// clear the container
	void clear(void)
	{
		_Tab.clear() ;
		_Tab.reserve(_MaxSize) ;
		_Size = 0 ;
	}

protected:		
	TContType _Tab ; 
	uint32 _Size, _MaxSize ;
} ;



/////////////////////////////////////////////////////////////////////////
//					IMPLEMENTATION									   //
/////////////////////////////////////////////////////////////////////////

template <typename T> 
CPSAttrib<T>::CPSAttrib(uint32 maxNbInstances) : _MaxSize(maxNbInstances), _Size(0)
{
}

template <typename T> 
void CPSAttrib<T>::resizeNFill(uint32 nbInstances)
{
	nlassert(_Size == _Tab.size()) ;
	_Tab.reserve(nbInstances) ;
	if (_Size > nbInstances)
	{
		_Tab.resize(nbInstances) ;
		_Size = nbInstances ;
		return ;
	}
	sint32 leftToFill = nbInstances - _Size ;

	if (leftToFill > 0)
	{
		do
		{
			_Tab.push_back(T()) ;
		}
		while (--leftToFill) ;
	}

	_Size = _MaxSize = nbInstances ;
	
	nlassert(_Size == _Tab.size()) ;
}


template <typename T> 
void CPSAttrib<T>::resize(uint32 nbInstances)
{
	nlassert(_Size == _Tab.size()) ;
	_Tab.reserve(nbInstances) ;
	_MaxSize = nbInstances ;
	if (_Size > _MaxSize)
	{
		_Size = _MaxSize ;
	}
	nlassert(_Size == _Tab.size()) ;
}


template <typename T> 
sint32 CPSAttrib<T>::insert(const T &t)
{
	nlassert(_Size == _Tab.size()) ;
	if (_Size == _MaxSize) 
	{
		return -1 ;
	}
	++ _Size ;
	_Tab.push_back(t) ;
	return _Size - 1 ;

	nlassert(_Size == _Tab.size()) ;
}


template <typename T> 
void CPSAttrib<T>::remove(uint32 index)
{
	nlassert(_Size == _Tab.size()) ;
	nlassert(index < _Size) ;
	// we copy the last element in place of this one
	if (index != _Size - 1)
	{
		_Tab[index] = _Tab[_Size - 1] ;
	}
	_Tab.pop_back() ;

	-- _Size ;

	nlassert(_Size == _Tab.size()) ;
}

template <typename T> 
void CPSAttrib<T>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	f.serialCheck((uint32)'PASA') ;
	f.serialVersion(1) ;
	f.serial(_Size) ;
	f.serial(_MaxSize) ;
	f.serialCont(_Tab) ;
}




// here we give some definition for common types

typedef CPSAttrib<NLMISC::CVector> TPSAttribVector ;
typedef CPSAttrib<NLMISC::CRGBA>   TPSAttribRGBA   ;
typedef CPSAttrib<float>		   TPSAttribFloat  ;	
typedef CPSAttrib<uint32>		   TPSAttribUInt  ;	
typedef CPSAttrib<CAnimationTime>  TPSAttribTime   ;





} // NL3D


#endif // NL_PS_ATTRIB_H

/* End of ps_attrib.h */
