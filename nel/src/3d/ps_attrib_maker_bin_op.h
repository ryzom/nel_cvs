/** \file ps_attrib_maker_bin_op.h
 * <File description>
 *
 * $Id: ps_attrib_maker_bin_op.h,v 1.1 2001/07/12 15:53:42 vizerie Exp $
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

#ifndef NL_PS_ATTRIB_MAKER_BIN_OP_H
#define NL_PS_ATTRIB_MAKER_BIN_OP_H

#include "nel/misc/types_nl.h"
#include "ps_attrib_maker.h"
#include "ps_plane_basis.h"
#include "nel/misc/rgba.h"


namespace NL3D {


/// this struct has an enumeration of various binary operators available with CPSAttribMakerBinOp
struct CPSBinOp
{
	enum BinOp
	{
		  selectArg1 = 0
		, selectArg2
		, modulate
		, add
		, subtract
		, last
	} ;
} ;

/// the size of the buffer use for intermediate operations
const uint PSBinOpBufSize = 1024 ;



/** some template functions and some specialization for binary operations
  * We don't override the usual operators, because we may want behaviour such as saturation, and this may be misleading
  */

template <class T>
inline T PSBinOpModulate(T arg1, T arg2) { return arg1 * arg2 ; }
template <class T>
inline T PSBinOpAdd(T arg1, T arg2) { return arg1 + arg2 ; }
template <class T>
inline T PSBinOpSubtract(T arg1, T arg2) { return arg1 - arg2 ; }

template <>
inline CPlaneBasis PSBinOpModulate(CPlaneBasis p1, CPlaneBasis p2)
{
	nlassert(0) ; // not allowed for now
	return CPlaneBasis(NLMISC::CVector::Null) ;
}
template <>
inline CPlaneBasis PSBinOpAdd(CPlaneBasis p1, CPlaneBasis p2)
{
	nlassert(0) ; // not allowed for now
	return CPlaneBasis(NLMISC::CVector::Null) ;
}
template <>
inline CPlaneBasis PSBinOpSubtract(CPlaneBasis p1, CPlaneBasis p2)
{
	nlassert(0) ; // not allowed for now
	return CPlaneBasis(NLMISC::CVector::Null) ;
}



 
template <>
inline NLMISC::CRGBA PSBinOpModulate(NLMISC::CRGBA t1, NLMISC::CRGBA t2)
{
	NLMISC::CRGBA result ;
	result.modulateFromColor(t1, t2) ;
	return result ;
}
template <>
inline NLMISC::CRGBA PSBinOpAdd(NLMISC::CRGBA t1, NLMISC::CRGBA t2)
{
	NLMISC::CRGBA r ;
	uint S = t1.R + t2.R ; if (S > 255) S = 255 ; r.R = (uint8) S ;
	S = t1.G + t2.G ; if (S > 255) S = 255 ; r.G = (uint8) S ;
	S = t1.B + t2.B ; if (S > 255) S = 255 ; r.B = (uint8) S ;
	return r ;
}
template <>
inline NLMISC::CRGBA PSBinOpSubtract(NLMISC::CRGBA t1, NLMISC::CRGBA t2)
{
	NLMISC::CRGBA r ;
	sint S = t1.R - t2.R ; if (S < 0) S = 0 ; r.R = (uint8) S ;
	S = t1.G - t2.G ; if (S < 0) S = 0 ; r.G = (uint8) S ;
	S = t1.B - t2.B ; if (S < 0) S = 0 ; r.B = (uint8) S ;
	return r ;
}




/**
 * An attribute maker that compute an attribute in a particle system.
 * It takes 2 other attributes makers and perform a binary operation on them to get the result
 * This allow to have more complex behaviour with  particles : random initial size that change with time and so on ...
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
template <class T> class CPSAttribMakerBinOp : public CPSAttribMaker<T>
{
public:
	/**  default ctor
	  *  It construct an selectArg1 operator. The 2 argument are set to NULL,
	  *  Which mean that an assertion will happen if get, make ... are called before setArg is called
	  */
	CPSAttribMakerBinOp() ;
	/// inherited from CPSAttribMaker
	virtual T		get			  (CPSLocated *loc, uint32 index) ; 
	virtual void   *make		  (CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool allowNoCopy = false) const ;	
	virtual void    make4		  (CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const ;	
	virtual void	makeN		  (CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const ;	
	virtual void    serial		  (NLMISC::IStream &f) throw(NLMISC::EStream) ;			
	virtual void    deleteElement (uint32 index) ;	
	virtual void    newElement	  (CPSLocated *emitterLocated, uint32 emitterIndex) ;	
	virtual void	resize		  (uint32 capacity, uint32 nbPresentElements) ;

	/** set an argument for the operator
	  * \param argNb must be 0 or 1 for the first and second argument
	  * \param arg the argument. Must have been allocated by new, and is then owned by this object
	  */
	void setArg(uint argNb, CPSAttribMaker<T> *arg)
	{
		nlassert(argNb < 2) ;
		delete _Arg[argNb] ;
		_Arg[argNb] = arg ;
		if (arg->hasMemory())
		{
			arg->resize(_MaxSize, _Size) ;
		}		
	}

	/** get an argument
	  * \see setArg
	  */
	CPSAttribMaker<T> *getArg(uint argNb)
	{
		nlassert(argNb < 2) ;
		return _Arg[argNb] ;
	}

	/** get an argument, const version
	  * \see setArg
	  */
	const CPSAttribMaker<T> *getArg(uint argNb) const
	{
		nlassert(argNb < 2) ;
		return _Arg[argNb] ;
	}

	/** Set the operator to use
	  * An assertion is thrown when no available
	  */
	void setOp(CPSBinOp::BinOp op)
	{
		nlassert(supportOp(op)) ;
		_Op = op ;
	}

	/// return true if an operation is supporte. The default support all ops
	bool supportOp(CPSBinOp::BinOp op) { return true ; }
	


	/// get the current operator
	CPSBinOp::BinOp getOp(void) const { return _Op ; }


	/** Some attributes maker may have father (binary operator => its 2 argument). If hasMemory value is true
	  * you must call the setFather method. Otherwise you must not.
	  */
	

	/// dtor
	virtual ~CPSAttribMakerBinOp() ;
protected:
	CPSBinOp::BinOp   _Op ; // the operator being used
	CPSAttribMaker<T> *_Arg[2] ; // the arguments for the binary operator	
	// the buffers used for intermediate computations
	static T _Buf1[PSBinOpBufSize], _Buf2[PSBinOpBufSize] ;
	void clean(void) ;	
	uint32 _Size, _MaxSize ;
};



////////////////////////////////////////
// CPSAttribMakerBinOp implementation //
////////////////////////////////////////
template <class T>
CPSAttribMakerBinOp<T>::CPSAttribMakerBinOp() : _Op(CPSBinOp::selectArg1), _Size(0), _MaxSize(0)
{
	_Arg[0] = _Arg[1] = NULL ;
	_HasMemory  = true ; 
}

template <class T>
void CPSAttribMakerBinOp<T>::clean(void) 
{
	delete _Arg[0] ;
	delete _Arg[1] ;
}

template <class T>
CPSAttribMakerBinOp<T>::~CPSAttribMakerBinOp() 
{
	clean() ;
}

/// cplane basis template specialization for supportOp
bool CPSAttribMakerBinOp<CPlaneBasis>::supportOp(CPSBinOp::BinOp op) 
{ 
	return  (op == CPSBinOp::selectArg1 || op == CPSBinOp::selectArg1) ;
}



template <class T>
T		CPSAttribMakerBinOp<T>::get			  (CPSLocated *loc, uint32 index)
{
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			return _Arg[0]->get(loc, index) ;
		break ;
		case CPSBinOp::selectArg2:
			return _Arg[1]->get(loc, index)  ;
		break ;
		case CPSBinOp::modulate:
			return PSBinOpModulate(_Arg[0]->get(loc, index), _Arg[1]->get(loc, index)) ;
		break ;
		case CPSBinOp::add:
			return PSBinOpAdd(_Arg[0]->get(loc, index), _Arg[1]->get(loc, index)) ;
		break ;
		case CPSBinOp::subtract:
			return PSBinOpSubtract(_Arg[0]->get(loc, index), _Arg[1]->get(loc, index)) ;
		break ;
	}

	nlstop ;
	return T() ;
}


template <class T>
void   *CPSAttribMakerBinOp<T>::make		  (CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool allowNoCopy /* = false */) const
{
	uint8 *dest = (uint8 *) tab ;
	uint leftToDo = numAttrib, toProcess  ;
	nlassert(_Arg[0] && _Arg[1]) ;	
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			return _Arg[0]->make(loc, startIndex, tab, stride, numAttrib, allowNoCopy) ;
		break ;
		case CPSBinOp::selectArg2:
			return _Arg[1]->make(loc, startIndex, tab, stride, numAttrib, allowNoCopy) ;
		break ;		
	}

	while (leftToDo)
	{
		toProcess = leftToDo > PSBinOpBufSize ? PSBinOpBufSize : leftToDo ;
		T *src1 = (T *) _Arg[0]->make(loc, startIndex + (numAttrib - leftToDo), &_Buf1[0], sizeof(T), toProcess, true) ;
		T *src2 = (T *) _Arg[1]->make(loc, startIndex + (numAttrib - leftToDo), &_Buf2[0], sizeof(T), toProcess, true) ;

		
		uint8 *destEnd = dest + (stride * toProcess) ;

		switch (_Op)
		{
			case CPSBinOp::modulate:
			{
				while (dest != destEnd)
				{
					* (T *) dest = PSBinOpModulate(*src1 ++, *src2 ++) ;
					dest += stride ;
				}
			}
			break ;
			case CPSBinOp::add:
			{
				while (dest != destEnd)
				{
					* (T *) dest = PSBinOpAdd(*src1 ++, *src2 ++) ;
					dest += stride ;
				}
			}
			break ;
			case CPSBinOp::subtract:
			while (dest != destEnd)
			{
				* (T *) dest = PSBinOpSubtract(*src1 ++, *src2 ++) ;
				dest += stride ;
			}
			break ;
		}

		leftToDo -= toProcess ;
	}

	return tab ;
}

template <class T>
void    CPSAttribMakerBinOp<T>::make4		  (CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const
{
	const uint stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride2 << 1 ; 
	uint8 *dest = (uint8 *) tab ;
	uint leftToDo = numAttrib, toProcess  ;
	nlassert(_Arg[0] && _Arg[1]) ;	
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			_Arg[0]->make4(loc, startIndex, tab, stride, numAttrib) ;
			return ;
		break ;
		case CPSBinOp::selectArg2:
			_Arg[1]->make4(loc, startIndex, tab, stride, numAttrib) ;
			return ;
		break ;		
	}

	while (leftToDo)
	{
		toProcess = leftToDo > PSBinOpBufSize ? PSBinOpBufSize : leftToDo ;
		T *src1 = (T *) _Arg[0]->make(loc, startIndex + (numAttrib - leftToDo), &_Buf1[0], sizeof(T), toProcess, true) ;
		T *src2 = (T *) _Arg[1]->make(loc, startIndex + (numAttrib - leftToDo), &_Buf2[0], sizeof(T), toProcess, true) ;
		
		uint8 *destEnd = dest + ((stride<<2) * toProcess) ;


		switch (_Op)
		{
			case CPSBinOp::modulate:
			{
				while (dest != destEnd)
				{
					// compute one value, and duplicate if 4 times
					* (T *) dest = PSBinOpModulate(*src1 ++, *src2 ++) ;
					* (T *) (dest + stride) = * (T *) dest ;
					* (T *) (dest + stride2) = * (T *) dest ;
					* (T *) (dest + stride3) = * (T *) dest ;
					dest += stride4 ;
				}
			}
			break ;
			case CPSBinOp::add:
			{
				while (dest != destEnd)
				{
					// compute one value, and duplicate if 4 times
					* (T *) dest = PSBinOpAdd(*src1 ++, *src2 ++) ;
					* (T *) (dest + stride) = * (T *) dest ;
					* (T *) (dest + stride2) = * (T *) dest ;
					* (T *) (dest + stride3) = * (T *) dest ;
					dest += stride4 ;
				}
			}
			break ;
			case CPSBinOp::subtract:
				while (dest != destEnd)
				{
					// compute one value, and duplicate if 4 times
					* (T *) dest = PSBinOpSubtract(*src1 ++, *src2 ++) ;
					* (T *) (dest + stride) = * (T *) dest ;
					* (T *) (dest + stride2) = * (T *) dest ;
					* (T *) (dest + stride3) = * (T *) dest ;
					dest += stride4 ;
				}
			break ;
		}

		leftToDo -= toProcess ;
	}
}

template <class T>
void	CPSAttribMakerBinOp<T>::makeN		  (CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const
{
		const uint stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride2 << 1 ; 
	uint8 *dest = (uint8 *) tab ;
	uint leftToDo = numAttrib, toProcess  ;
	uint k ;
	nlassert(_Arg[0] && _Arg[1]) ;	
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			_Arg[0]->makeN(loc, startIndex, tab, stride, numAttrib, nbReplicate) ;
			return ;
		break ;
		case CPSBinOp::selectArg2:
			_Arg[1]->makeN(loc, startIndex, tab, stride, numAttrib, nbReplicate) ;
			return ;
		break ;		
	}

	while (leftToDo)
	{
		toProcess = leftToDo > PSBinOpBufSize ? PSBinOpBufSize : leftToDo ;
		T *src1 = (T *) _Arg[0]->make(loc, startIndex + (numAttrib - leftToDo), &_Buf1[0], sizeof(T), toProcess, true) ;
		T *src2 = (T *) _Arg[1]->make(loc, startIndex + (numAttrib - leftToDo), &_Buf2[0], sizeof(T), toProcess, true) ;
		
		uint8 *destEnd = dest + ((stride * nbReplicate) * toProcess) ;


		switch (_Op)
		{
			case CPSBinOp::modulate:
			{
				while (dest != destEnd)
				{
					* (T *) dest = PSBinOpModulate(*src1 ++, *src2 ++) ;					
					k = (nbReplicate - 1) ;
					do
					{
						* (T *) (dest + stride) = *(T *) dest ;
						dest += stride ;
					}
					while (--k) ;	
				}
			}
			break ;
			case CPSBinOp::add:
			{
				while (dest != destEnd)
				{					
					* (T *) dest = PSBinOpAdd(*src1 ++, *src2 ++) ;					
					k = (nbReplicate - 1) ;
					do
					{
						* (T *) (dest + stride) = *(T *) dest ;
						dest += stride ;
					}
					while (--k) ;					
				}
			}
			break ;
			case CPSBinOp::subtract:
				while (dest != destEnd)
				{
					* (T *) dest = PSBinOpSubtract(*src1 ++, *src2 ++) ;					
					k = (nbReplicate - 1) ;
					do
					{
						* (T *) (dest + stride) = *(T *) dest ;
						dest += stride ;
					}
					while (--k) ;
				}
			break ;
		}

		leftToDo -= toProcess ;
	}

}

template <class T>
void    CPSAttribMakerBinOp<T>::serial		  (NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading())
	{
		clean() ;
	}
	f.serialVersion(1) ;
	f.serialEnum(_Op) ;
	f.serialPolyPtr(_Arg[0]) ;
	f.serialPolyPtr(_Arg[1]) ;
	f.serial(_Size, _MaxSize) ;
	
}

template <class T>
void    CPSAttribMakerBinOp<T>::deleteElement (uint32 index)
{
	if (_Arg[0]->hasMemory())	_Arg[0]->deleteElement(index) ;
	if (_Arg[1]->hasMemory())	_Arg[1]->deleteElement(index) ;
	nlassert(_Size != 0) ;
	--_Size ;
}

template <class T>
void    CPSAttribMakerBinOp<T>::newElement	  (CPSLocated *emitterLocated, uint32 emitterIndex)
{
	if (_Arg[0]->hasMemory())	_Arg[0]->newElement(emitterLocated, emitterIndex) ;
	if (_Arg[1]->hasMemory())	_Arg[1]->newElement(emitterLocated, emitterIndex) ;
	if (_Size != _MaxSize)
	{
		++_Size ;
	}
}

template <class T>
void	CPSAttribMakerBinOp<T>::resize		  (uint32 capacity, uint32 nbPresentElements)
{
	_MaxSize = capacity ;
	_Size = nbPresentElements ;
	if (_Arg[0]->hasMemory())	_Arg[0]->resize(capacity, nbPresentElements) ;
	if (_Arg[1]->hasMemory())	_Arg[1]->resize(capacity, nbPresentElements) ;
}


} // NL3D


#endif // NL_PS_ATTRIB_MAKER_BIN_OP_H

/* End of ps_attrib_maker_bin_op.h */
