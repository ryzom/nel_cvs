/** \file ps_attrib_maker_template.h
 * <File description>
 *
 * $Id: ps_attrib_maker_template.h,v 1.1 2001/05/08 13:37:09 vizerie Exp $
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

#ifndef NL_PS_ATTRIB_MAKER_TEMPLATE_H
#define NL_PS_ATTRIB_MAKER_TEMPLATE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib_maker.h"


namespace NL3D {

/*
 *	In this file, we define several template that helps to create attributes maker such as gradient (of float, int, vector etc) 
 * attributes maker are used in the particle system to generate values, such as size, color etc. see ps_attrib_maker.h
 * for more informations
 */



/**
 * This temlate functor blend between 2 value
 * It is used by CPSValueBlend
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
template <typename T> class CPSValueBlendFunc
{
public:
	/// this produce Values
	T operator()(CAnimationTime time) const
	{

		return (T) (time * _ValueRatio + _CstValue) ;	// a cast to T is necessary, because 
														// the specialization couls be done with integer
	}

	/// restrieve the start and end Value

	void getValues(T &startValue, T &endValue) const
	{
		startValue = (*this)(0) ;
		endValue = (*this)(1) ;
	}	

	/// set the Values

	void setValues(T startValue, T endValue)
	{
		_CstValue = startValue ;
		_ValueRatio = endValue - startValue ;
	}

	/// ctor
	CPSValueBlendFunc() {}

	/// serialization
	void serial(NLMISC::IStream &f)
	{
		f.serial(_CstValue, _ValueRatio) ;
	}

	T getMaxValue(void) const
	{
		return std::max((*this)(0), (*this)(1)) ;
	}

protected:
	T _CstValue ;
	T _ValueRatio ;
};



/** This is a Value blender class
 *  To use this, just derive a class, create a ctor, and declare it to the class registry 
 *  in the ctor, you should call _F.setValue to init the functor object.
 */

template <typename T> class CPSValueBlender : public CPSAttribMakerT<T, CPSValueBlendFunc<T> >
{
public:	
	/** ctor
	 *  With nbCycles, you can set the pattern frequency. It is usually one. See ps_attrib_maker.h
	 *  For further details
	 */
	CPSValueBlender(float nbCycles) : CPSAttribMakerT<T, CPSValueBlendFunc<T> >(nbCycles)
	{	
	}

	virtual T getMaxValue(void) const { return _F.getMaxValue() ; }

	// serialization is done by CPSAttribMakerT


} ;


/**
 * This functor blend between several Value
 * It is used by CPSValueGradient, that you can use to have gradients with your own types
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
template <typename T> class CPSValueGradientFunc
{
public:
	/// this produce Values
	T operator()(CAnimationTime time) const
	{
		nlassert(_Tab) ;
		return _Tab[(uint32) (time * _NbValues)] ;
	}

	/// copie the keys values in the specified table
	void getValues(T *tab) const
	{
		nlassert(tab) ;
		T *pt = tab ;
		T *src = _Tab ;
		for (uint32 k = 0 ; k < _NbValues ; ++k)
		{
			*pt++ = *src ;
			src += _NbStages ; // jump the interpolated values
		}
	}

	uint32 getNumValues(void) const { return ((_NbValues - 1) / _NbStages) + 1 ; }

	/** set the colors
	 *  \param numValue number of Values, must be >= 2
	 *  \ValueTab a table containing the Values. Value will be blended, so you must only provide keyframe Values	 
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 *  WARNING : for integer types, some specilization exist that ensure correct interpolation. see below
	 */

	inline void setValues(const T *ValueTab, uint32 numValues, uint32 nbStages) ;
	

	/// serialization
	virtual void serial(NLMISC::IStream &f) ;


	T getMaxValue(void) const
	{
		return _MaxValue ;
	}

	/// ctor
	CPSValueGradientFunc() : _Tab(NULL), _NbValues(0), _NbStages(0)
	{
	}

	/// dtor
	~CPSValueGradientFunc()
	{
		delete[] _Tab ;
	}
	
protected:
	// a table of Values that interpolate the values given
	T *_Tab ;


	// number of interpolated value between each 'key'
	uint32 _NbStages ;

	// total number of value in the tab
	uint32 _NbValues ;


	// the max value
	T _MaxValue ;
};




/** This is a Values gradient class
 *  To use this, just derive a class from a specialization of this template , create a ctor, and declare it to the class registry
 *  in the ctor, you should call _F.setValue to init the functor object
 */

template <typename T> class CPSValueGradient : public CPSAttribMakerT<T, CPSValueGradientFunc<T> >
{
public:

	/** ctor
	 *  With nbCycles, you can set the pattern frequency. It is usually one. See ps_attrib_maker.h
	 *  For further details
	 */

	CPSValueGradient(float nbCycles) : CPSAttribMakerT<T, CPSValueGradientFunc<T> >(nbCycles)
	{
	}

	virtual T getMaxValue(void) const { return _F.getMaxValue() ; }
	

} ;




////////////////////////////
// methods implementations //
////////////////////////////



	
template <typename T> 
inline void CPSValueGradientFunc<T>::setValues(const T *valueTab, uint32 numValues, uint32 nbStages)
{
	nlassert(numValues > 1) ;
	nlassert(nbStages > 0) ;

	_NbStages = nbStages ;

	_MaxValue = valueTab[0] ;

	if (_Tab)
	{
		delete[] _Tab ;		
	}

	_NbValues = 1 + (numValues - 1) * nbStages ;

	_Tab = new T[_NbValues] ;


	float invNbStages = 1.0f / float(nbStages) ;
	T currVal ;
	T step ; // the step between each interpolated value

	T *dest = _Tab ;

	// copy the tab performing linear interpolation between values given in parameter
	for (uint32 k = 0 ; k  < (numValues - 1) ; ++k)
	{
		currVal = valueTab[k] ;
		step = (valueTab[k + 1] - currVal) * invNbStages ;
		
		if (currVal > _MaxValue)
		{
			_MaxValue = currVal ;
		}

		for(uint32 l = 0 ; l < nbStages ; ++l)
		{
			
			*dest++ = currVal ;
			currVal += step ;
		}
	}
	*dest++ = valueTab[numValues - 1] ;
}


// sint32 specialization
// this is needed to get a correct step between values (wrong interpolation if an int is used)
// implementation in ps_attrib_maker_template.cpp ...

template <> 
inline void CPSValueGradientFunc<sint32>::setValues(const sint32 *valueTab, uint32 numValues, uint32 nbStages)
{
	nlassert(numValues > 1) ;
	nlassert(nbStages > 0) ;

	_NbStages = nbStages ;

	_MaxValue = valueTab[0] ;

	if (_Tab)
	{
		delete[] _Tab ;		
	}

	_NbValues = 1 + (numValues - 1) * nbStages ;

	_Tab = new sint32[_NbValues] ;


	float invNbStages = 1.0f / float(nbStages) ;
	float currVal ;
	float step ; // the step between each interpolated value

	sint32 *dest = _Tab ;

	// copy the tab performing linear interpolation between values given in parameter
	for (uint32 k = 0 ; k  < (numValues - 1) ; ++k)
	{
		currVal = (float) valueTab[k] ;
		step = (valueTab[k + 1] - currVal) * invNbStages ;
		
		if (currVal > _MaxValue)
		{
			_MaxValue = (sint32) currVal ;
		}

		for(uint32 l = 0 ; l < nbStages ; ++l)
		{
			
			*dest++ = (sint32) currVal ;
			currVal += step ;
		}
	}
	*dest++ = valueTab[numValues - 1] ;
}


	
template <typename T> 
void CPSValueGradientFunc<T>::serial(NLMISC::IStream &f)
{
	f.serialCheck((uint32) 'GVF_') ;
	f.serial(_NbStages) ;
	if (f.isReading())
	{
	
		// reload the number of keys 

		uint32 numVal ;
		f.serial(numVal) ;		
		_NbValues = 1 + (numVal - 1) * _NbStages ;

		// create the table on the stack for small gradient
		if (numVal < 256)
		{
			T tab[256] ;
			for (uint32 k = 0 ; k < numVal ; ++k)
			{
				f.serial(tab[k]) ;
			}
			setValues(tab, numVal, _NbStages) ;
		}	
		else
		{
			T *tab = new T[numVal] ;
			for (uint32 k = 0 ; k < numVal ; ++k)
			{
				f.serial(tab[k]) ;
			}
			setValues(tab, numVal, _NbStages) ;
			delete[] tab ;
		}
	}
	else
	{
		// saves the number of keys
		uint32 numKeyValues = getNumValues() ;
		f.serial(numKeyValues) ;


		// save each key
		T *src = _Tab ;
		for (uint32 k = 0 ; k < numKeyValues  ; ++k)
		{
			f.serial(*src) ;
			src += _NbStages ;
		}
	}
}

} // NL3D


#endif // NL_PS_ATTRIB_MAKER_TEMPLATE_H

/* End of ps_attrib_maker_template.h */
