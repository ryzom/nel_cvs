/** \file ps_attrib_maker.h
 * <File description>
 *
 * $Id: ps_attrib_maker.h,v 1.1 2001/05/02 11:49:50 vizerie Exp $
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

#ifndef NL_PS_ATTRIB_MAKER_H
#define NL_PS_ATTRIB_MAKER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib.h"
#include "nel/misc/stream.h"


/**
 * Here we define attribute maker, that is object that can produce an attribute following some rule.
 * This allow, for example, creation of a color gradient, or color flicker, size strectching and so on...
 */




namespace NL3D {


/**
 * This is the base struct for any attrib maker
 * It can be used to fill a vertex buffer, or a table
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
template <typename T> struct CPSAttribMaker : public NLMISC::IStreamable
{	
	/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes, and use it to get the
	 * The particle life as an input
	 */
	  virtual void make(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib) const = 0 ;

	/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
	 *  \see make()
	 */
	  virtual void make4(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib) const = 0 ;

	/** The same as make4, but with n replication instead of 4	 
	 *  \see make4
	 */
	  virtual void makeN(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const = 0 ;

	/// serialisation of the object
	virtual void serial(NLMISC::IStream &f) = 0 ;

	/// get the max value (meaningful for orderer set only)

	virtual T getMaxValue(void) const { return T() ; /* no mean by default */ }

	/// dtor
	  virtual ~CPSAttribMaker() {}
};


/** This template generate an attrib filler by defining the method of the CPSCAttribMaker class. You can derive your own class but it is a shortcut to do the job
 *  \param T : the type to produce
 *  \param F : a class that override the () operator, the input is time (CAnimationTime) , and the output is the same type as T.
 *             Inline is preferable, as it will be called a lot 
 *             It can stores info that indicate how to build it
 */

template <typename T, class F> struct CPSAttribMakerT : public CPSAttribMaker<T>
{
	/// the functor object 
	F _F ;	
	
	/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes, and use it to get the
	 * The particle life as an input
	 */
	  virtual void make(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib) const ;

	/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
	 *  \see make()
	 */
	  virtual void make4(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib) const ;

	/** The same as make4, but with n replication instead of 4	 
	 *  \see make4
	 */
	  virtual void makeN(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const ;


	/// serialisation of the object
	virtual void serial(NLMISC::IStream &f)
	{
	  f.serial(_F) ;
	}

	/// dtor
	  virtual ~CPSAttribMakerT() {}


	/// the type of the attribute to be produced
	  typedef T value_type ;
	/// the type of the functor object
	  typedef F functor_type ;
} ;

///////////////////////////////////////////////
// implementation of CPSAttribMakerT methods //
///////////////////////////////////////////////


template <typename T, class F> void CPSAttribMakerT<T, F>::make(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib) const
{
	uint8 *pt = (uint8 *) tab ;
	while (numAttrib --)
	{
		*(T *)pt = _F(*it) ; 
		pt += stride ;
		++it ;
	}
}

template <typename T, class F> void CPSAttribMakerT<T, F>::make4(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib) const
{
	uint8 *pt = (uint8 *) tab ;


	// first precompute the various strides (stride * 2, 3 and 4)
	const uint32 stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride2 << 1 ;
	while (numAttrib --)
	{		
		// fill 4 attrib with the same value at once 
		*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(*it) ;		
		pt += stride4 ; // advance of 4 
		++it ;
	}
}

template <typename T, class F> void CPSAttribMakerT<T, F>::makeN(TPSAttribTime::const_iterator it, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const
{
	nlassert(nbReplicate  != 0) ;
	uint8 *pt = (uint8 *) tab ;
	uint32 k ;
	T value ;
	
	while (numAttrib --)
	{		
		value = _F(*it) ;
		k = 0 ;
		++it ;
		do
		{
			*(T *)pt = value  ;
			pt += stride ;
		}
		while (--k) ;
	}
}




} // NL3D


#endif // NL_PS_ATTRIB_MAKER_H

/* End of ps_attrib_maker.h */
