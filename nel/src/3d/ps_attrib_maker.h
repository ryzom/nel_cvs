/** \file ps_attrib_maker.h
 * <File description>
 *
 * $Id: ps_attrib_maker.h,v 1.5 2001/07/04 16:04:57 vizerie Exp $
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
#include "3d/ps_attrib.h"
#include "3d/ps_located.h"
#include "nel/misc/stream.h"






namespace NL3D {


// this struct only contains an enum that tell chat the input of an atribute maker is
struct TPSInputType
{
	/// input types
	enum TInputType
	{
		attrDate = 0,
		attrPosition = 1,
		attrInverseMass = 2,
		attrSpeed = 3,
		attrUniformRandom = 4,
		attrLast 
	} ;
} ;


/**
 * Here we define attribute maker, that is object that can produce an attribute following some rule.
 * This allow, for example, creation of a color gradient, or color flicker, size strectching and so on...
 * These attributes apply to particles. see paticle_system.h and ps_located.h
 */



// the max value for inputs
const float MaxInputValue = 0.9999f ;


/**
 * This is the base class for any attrib maker
 * It can be used to fill a vertex buffer, or a table
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */


template <typename T> class CPSAttribMaker : public NLMISC::IStreamable
{	
public:


	/// compute one value of the attribute for the given index
	virtual T get(CPSLocated *loc, uint32 index) = 0 ;


	/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes.
	 *  \param loc the 'located' that hold the 'located bindable' that need an attribute to be filled
	 *  \param startIndex usually 0, it gives the index of the first element in the located
	 *  \param tab where the data will be written
	 *  \param stride the stride, in byte, between each value to write
	 *  \param numAttrib the number of attributes to compute
     *  \param allowNoCopy data may be already present in memory, and may not need computation. When set to true, this allow no computation to be made
	 *         the return parameter is then le location of the datas. this may be tab (if recomputation where needed), or another value 
	 *         for this to work, the stride must most of the time be sizeof(T). This is intended to be used with derivers of CPSAttribMaker
	 *         that store values that do not depend on the input. The make method then just copy the data, we is sometime useless
	 *  \return where the data have been copied, this is always tab, unless allowNoCopy is set to true, in which case this may be different
	 *                                         
	 */

	  virtual void *make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool allowNoCopy = false) const = 0 ;

	/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
	 *  \see make()
	 */
	  virtual void make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const = 0 ;

	/** The same as make4, but with n replication instead of 4	 
	 *  \see make4
	 */
	 virtual void makeN(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const = 0 ;

	/// serialisation of the object. Derivers MUST call this, (if they use the attribute of this class at least)
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		f.serial(_NbCycles) ;	
	}

	/// get the max value (meaningful for orderer set only)

	virtual T getMaxValue(void) const { return T() ; /* no mean by default */ }


	/** Set the number of cycles that must be done during the life of a particle,
	 * or the number of cycle per second for a particle that has no life limit
	 * It must be >= 0
	 */
	void setNbCycles(float nbCycles) 
	{ 
		nlassert(nbCycles >= 0) ;
		_NbCycles = nbCycles ; 
	}

	/** Retrieve the number of cycles
	 *  \see setNbCycles()
	 */
	float getNbCycles(void) const { return _NbCycles ; }


	/** construct the attrib maker specifying the number of cycles to do.
	 *  \see setNbCycles()
	 */	 
	CPSAttribMaker(float nbCycles) : _NbCycles(nbCycles), _HasMemory(false)
	{
	
	}


	/// tells wether one may choose one attribute from a CPSLocated to use as an input. If false, the input(s) is fixed
	virtual bool hasCustomInput(void) { return false ; }
		

	/** set a new input type (if supported). The default does nothing
	 *  \see hasCustomInput()
	 */
	virtual void setInput(TPSInputType::TInputType input) {}


	/** get the type of input (if supported). The default return attrDate
	 *  \see hasCustomInput()
	 */
	virtual TPSInputType::TInputType getInput(void) const { return TPSInputType::attrDate ; }
	


	/** tells wether clamping is supported for the input (value can't go above MaxInputValue)
	 *  The default is false
	 */
	virtual bool isClampingSupported(void) const { return false ; }


	/** Enable, disable the clamping of input values.
	 *  The default does nothing (clamping unsupported)
	 *  \see isClampingSupported()
	 */
	virtual void setClamping(bool enable = true) {} ;


	/** Test if the clamping is enabled.
	 *  The default is false (clamping unsupported)
	 *  \see isClampingSupported()
	 */
	virtual bool getClamping(void) const  { return false  ; } ;


	/// Some attribute makers may hold memory. this return true if when this is the case
	bool hasMemory(void) const { return _HasMemory ; }

	/// delete an element, given its index. this must be called  only if memory management is used.
	virtual void deleteElement(uint32 index) { nlassert(false) ; }

	/** create a new element, and provides the emitter, 
	  *	this must be called only if this attribute maker has its own memory
	  */
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) { nlassert(false) ; }

	/** set a new capacity for the memorized attribute, and a number of used element. This usually is 0
	  * , but during edition, this may not be ... so new element are created.
	  * this must be called only if this attribute maker has its own memory
	  */
	virtual void resize(uint32 capacity, uint32 nbPresentElements) { nlassert(false) ; }



	/// dtor
	virtual ~CPSAttribMaker() {}


	

protected:

	float _NbCycles ;

	// set to true if the attribute maker owns its own memory for each particle attribute
	bool _HasMemory ;
};





/** This template generate an attrib maker by defining the methods of the CPSCAttribMaker class. You can derive your own class 
 * but it is a shortcut to do the job
 *  \param T : the type to produce
 *  \param F : a class that override the () operator, the input is the age of the particle (CAnimationTime) 
 *             , and the output is the same type as T.
 *             Inline is preferable, as it will be called a lot 
 *             It can stores info that indicate how to build it
 */

template <typename T, class F> class CPSAttribMakerT : public CPSAttribMaker<T>
{
	public:
		/// the functor object 
		F _F ;	

		/// compute one value of the attribute for the given index
		virtual T get(CPSLocated *loc, uint32 index) ;
		
		/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes, and use it to get the
		 * The particle life as an input
		 */
		  virtual void *make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool allowNoCopy = false) const ;

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \see make()
		 */
		  virtual void make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const ;

		/** The same as make4, but with n replication instead of 4	 
		 *  \see make4
		 */
		 virtual void makeN(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const ;


		/// serialisation of the object
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialVersion(1) ;
			CPSAttribMaker<T>::serial(f) ;
		    f.serial(_F) ;
			f.serialEnum(_InputType) ;
			f.serial(_Clamp) ;
		}

		/** construct the attrib maker specifying the number of cycles to do.
		 *  \see setNbCycles()
		 */	 
		CPSAttribMakerT(float nbCycles) : CPSAttribMaker<T>(nbCycles), _InputType(TPSInputType::attrDate)
										  , _Clamp(false)
		{}

		/// dtor
		  virtual ~CPSAttribMakerT() {}


		/** tells wether one may choose one attribute from a CPSLocated to use as an input. If false, the input(s) is fixed
		 *  For this class, it is supported
		 */
		virtual bool hasCustomInput(void) { return true ; } 
		

		/** set a new input type 		
		 */
		virtual void setInput(TPSInputType::TInputType input) { _InputType = input ; }


		/** get the type of input (if supported). The default return attrDate
		 *  \see hasCustomInput()
		 */
		virtual TPSInputType::TInputType getInput(void) const { return _InputType ; }


		/** tells wether clamping is supported for the input (value can't go above MaxInputValue)
		 */
		bool isClampingSupported(void) const { return true ; }


		/** Enable, disable the clamping of input values.		
		 *  \see isClampingSupported()
		 */
		virtual void setClamping(bool enable = true) { _Clamp = enable ; } ;


		/** Test if the clamping is enabled.		
		 *  \see isClampingSupported()
		 */
		virtual bool getClamping(void) const  { return _Clamp  ; } ;


		/// the type of the attribute to be produced
		  typedef T value_type ;

		/// the type of the functor object
		  typedef F functor_type ;

	private:		

		// type of the input
		TPSInputType::TInputType _InputType ;

		// clamping on/ off
		bool _Clamp ;

		/** This special iterator on a vector attributes enables to convert the speed to its norm		 
		 *  It is for private use only, and it has not all the functionnalities of an iterator.
		 *  The src datas can't be modified as we return the norm, and not a reference on the value
		 */
		struct CVectNormIterator
		{
			
			TPSAttribVector::const_iterator Iter ;

			CVectNormIterator() ;

			CVectNormIterator(TPSAttribVector::const_iterator it) : Iter(it)
			{
			}

			float operator*() const { return Iter->norm() ; }
			

			// post increment
			CVectNormIterator &operator++(int)
			{
				CVectNormIterator tmp = *this ;
				++Iter ;
				return tmp ;
			}


			// pre-increment
			CVectNormIterator &operator++()
			{
				++Iter ;
				return *this ;
			}

			// post decrement
			CVectNormIterator &operator--(int)
			{
				CVectNormIterator tmp = *this ;
				--Iter ;
				return tmp ;
			}


			// pre-decrement
			CVectNormIterator &operator--()
			{
				--Iter ;
				return *this ;
			}

		} ;

		/** This special iterator return random values every time it is read
		 *  It is for private use only, and it has not all the functionnalities of an iterator.		 
		 */

		struct CRandomIterator
		{
			float operator*() const { return rand() * (1.f / RAND_MAX) ; }

			
			// dummy post increment
			CRandomIterator &operator++(int)
			{			
				return *this ;
			}
			// dummy pre-increment
			CRandomIterator &operator++()
			{				
				return *this ;
			}
			// dummy post decrement
			CRandomIterator &operator--(int)
			{			
			}
			// dummy pre-decrement
			CRandomIterator &operator--()
			{				
				return this ;
			}

		} ;


		 /** generate an attribute by using the given iterator. this allow to chose the input of
  		  *  \param canOverlapOne must be true if the entry iterator can give values above 1
		  *  the attribute maker with no speed penalty
		  */

		 template <class It> void makeByIterator(It it, void *tab, uint32 stride
												, uint32 numAttrib, bool canOverlapOne) const
		 {						
			uint8 *pt = (uint8 *) tab ;

			if (_NbCycles > 1 || canOverlapOne)
			{
				// the value could cycle, so we need to clamp it to 0.0f 1.0f

				if (!_Clamp)
				{
					if (_NbCycles == 1)
					{
						while (numAttrib --)
						{	
							*(T *)pt = _F((*it) - (uint32) (*it)) ; 
							pt += stride ;
							++it ;
						}
					}
					else
					{
						while (numAttrib --)
						{
							const float time =  _NbCycles * (*it) ;
							*(T *)pt = _F(time - (uint32) time) ; 
							pt += stride ;
							++it ;
						}	
					}
				}
				else
				{
					// clamping is on

					float value ;

					if (_NbCycles == 1)
					{
						while (numAttrib --)
						{	
							value = (*it) ;
							if (value > MaxInputValue)
							{
								value = MaxInputValue ;
							}
							*(T *)pt = _F(value) ; 
							pt += stride ;
							++it ;
						}
					}
					else
					{
						while (numAttrib --)
						{
							float value =  _NbCycles * (*it) ;
							if (value > MaxInputValue)
							{
								value = MaxInputValue ;
							}														
							*(T *)pt = _F(value) ; 
							pt += stride ;
							++it ;
						}	
					}
				}
			}
			else
			{
				// the fastest case : it match the particle's life perfeclty

				if (_NbCycles == 1)
				{
					while (numAttrib --)
					{
						*(T *)pt = _F(*it) ; 
						pt += stride ;
						++it ;
					}
				}
				else
				{
					// the particle won't cover the whole pattern durin his life
					while (numAttrib --)
					{
						*(T *)pt = _F(_NbCycles  * (*it)) ; 
						pt += stride ;
						++it ;
					}
				}
			}
		}

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \param canOverlapOne must be true if the entry iterator can give values above 1
		 *  \see makeByIterator()
		 */
		 template <class It> void make4ByIterator(It it, void *tab, uint32 stride
												  , uint32 numAttrib, bool canOverlapOne) const
		 {
			

			uint8 *pt = (uint8 *) tab ;


			// first precompute the various strides (stride * 2, 3 and 4)
			const uint32 stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride2 << 1 ;

			if (_NbCycles > 1 || canOverlapOne)
			{
				
				if (!_Clamp)
				{
					if (_NbCycles == 1)
					{			
						while (numAttrib --)
						{		
							// fill 4 attrib with the same value at once 
							*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F((*it) - (uint32) (*it)) ;		
							pt += stride4 ; // advance of 4 
							++it ;
						}
					}
					else
					{			
						while (numAttrib --)
						{		
							const float time =  _NbCycles * (*it) ;
							// fill 4 attrib with the same value at once 
							*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(time - (uint32) time) ;		
							pt += stride4 ; // advance of 4 
							++it ;
						}
					}
				}
				else
				{
					float value ;

					if (_NbCycles == 1)
					{			
						while (numAttrib --)
						{		
							value = *it ;
							if (value > MaxInputValue)
							{
								value = MaxInputValue ;
							}
							// fill 4 attrib with the same value at once 
							*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(value) ;		
							pt += stride4 ; // advance of 4 
							++it ;
						}
					}
					else
					{			
						while (numAttrib --)
						{		
							value =   _NbCycles * (*it) ;
							if (value > MaxInputValue)
							{
								value = MaxInputValue ;
							}
							// fill 4 attrib with the same value at once 
							*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(value) ;		
							pt += stride4 ; // advance of 4 
							++it ;
						}
					}					
				}
			}
			else
			{
				// the fastest case : it match the particle's life perfeclty

				if (_NbCycles == 1)
				{
					while (numAttrib --)
					{		
						// fill 4 attrib with the same value at once 
						*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(*it) ;		
						pt += stride4 ; // advance of 4 
						++it ;
					}
				}
				else
				{
					// the particle won't cover the whole pattern durin his life

					while (numAttrib --)
					{		
						// fill 4 attrib with the same value at once 
						*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(_NbCycles * *it) ;		
						pt += stride4 ; // advance of 4 
						++it ;
					}
				}
			}	
		 }


		/** The same as make4, but with n replication instead of 4	 
		 *  \param canOverlapOne must be true if the entry iterator can give values above 1
		 *  \see make4ByIterator
		 */
		 template <class It> void makeNByIterator(It it, void *tab, uint32 stride, uint32 numAttrib
												  , uint32 nbReplicate, bool canOverlapOne) const
		 {
				
				nlassert(nbReplicate > 1) ; 				

				uint8 *pt = (uint8 *) tab ;

				// loop counter
				uint k ;


				if (_NbCycles > 1 || canOverlapOne)
				{
					
					if (!_Clamp)
					{
						if (_NbCycles == 1)
						{			
							while (numAttrib --)
							{		
								// fill 4 attrib with the same value at once 
								*(T *)pt = _F((*it) - (uint32) (*it)) ;						
								k = nbReplicate - 1;
								do 
								{
									*(T *) (pt + stride) = *(T *) pt ;
									pt += stride ;
								}
								while (--k) ;
								
								++it ;
							}
						}
						else
						{			
							while (numAttrib --)
							{		
								const float time =  _NbCycles * (*it) ;
								// fill 4 attrib with the same value at once 
								*(T *)pt = _F(time - (uint32) time) ;					
								k = nbReplicate - 1;
								do 
								{
									*(T *) (pt + stride) = *(T *) pt ;
									pt += stride ;
								}
								while (--k) ;
								++it ;
							}
						}
					}
					else
					{
						float value ;
						// clamping is on
						if (_NbCycles == 1)
						{			
							while (numAttrib --)
							{		
								// fill 4 attrib with the same value at once 
								value = *it ;
								if (value > MaxInputValue)
								{
									value = MaxInputValue ;
								}
								*(T *)pt = _F(value) ;						
								k = nbReplicate - 1;
								do 
								{
									*(T *) (pt + stride) = *(T *) pt ;
									pt += stride ;
								}
								while (--k) ;
								
								++it ;
							}
						}
						else
						{			
							while (numAttrib --)
							{		
								value =  _NbCycles * (*it) ;
								if (value > MaxInputValue)
								{
									value = MaxInputValue ;
								}
								// fill 4 attrib with the same value at once 
								*(T *)pt = _F(value) ;					
								k = nbReplicate - 1;
								do 
								{
									*(T *) (pt + stride) = *(T *) pt ;
									pt += stride ;
								}
								while (--k) ;
								++it ;
							}
						}
					}
				}
				else
				{
					// the fastest case : it match the particle's life perfeclty

					if (_NbCycles == 1)
					{
						while (numAttrib --)
						{		
							// fill 4 attrib with the same value at once 
							*(T *)pt = _F(*it) ;						
							k = nbReplicate - 1;
							do 
							{
								*(T *) (pt + stride) = *(T *) pt ;
								pt += stride ;
							}
							while (--k) ;
							++it ;
						}
					}
					else
					{
						// the particle won't cover the whole pattern durin his life

						while (numAttrib --)
						{		
							// fill 4 attrib with the same value at once 
							*(T *)pt =  _F(_NbCycles * *it) ;					
							k = nbReplicate - 1;
							do 
							{
								*(T *) (pt + stride) = *(T *) pt ;
								pt += stride ;
							}
							while (--k) ;
							++it ;
						}
					}
				}	
			}



} ;

///////////////////////////////////////////////
// implementation of CPSAttribMakerT methods //
///////////////////////////////////////////////


template <typename T, class F> 
T  CPSAttribMakerT<T, F>::get(CPSLocated *loc, uint32 index)
{	
	switch (_InputType)
	{
		case TPSInputType::attrDate:	
		{
			float v = _NbCycles * loc->getTime()[index] ;
			if (_Clamp)
			{
				if (v > MaxInputValue) v = MaxInputValue ;
			}

			return _F(v - (uint32) v) ;
		}
		break ;
		case TPSInputType::attrInverseMass:	
		{
			float v = _NbCycles * loc->getInvMass()[index] ;
			if (_Clamp)
			{
				if (v > MaxInputValue) v = MaxInputValue ;
			}
			return _F(v - (uint32) v) ;
		}
		break ;		
		case TPSInputType::attrSpeed:	
		{
			float v = _NbCycles * loc->getSpeed()[index].norm() ;
			if (_Clamp)
			{
				if (v > MaxInputValue) v = MaxInputValue ;
			}
			return _F(v - (uint32) v) ;
		}
		break ;

		case TPSInputType::attrPosition:	
		{
			float v = _NbCycles * loc->getPos()[index].norm() ;
			if (_Clamp)
			{
				if (v > MaxInputValue) v = MaxInputValue ;
			}
			return _F(v - (uint32) v) ;
		}
		break ;
		case TPSInputType::attrUniformRandom:	
		{
			return _F(rand() * (1.f / RAND_MAX)) ;
		}
		break ;
	}		

	return T() ;
}

template <typename T, class F> 
void *CPSAttribMakerT<T, F>::make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool allowNoCopy /* = false */) const
{
	nlassert(loc) ;

	switch (_InputType)
	{
		case TPSInputType::attrDate:	
		{
			TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, loc->getLastForever()) ;
		}
		break ;
		case TPSInputType::attrInverseMass:	
		{
			TPSAttribFloat::const_iterator it = (loc->getInvMass().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;		
		case TPSInputType::attrSpeed:	
		{
			CVectNormIterator it = (loc->getSpeed().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;

		case TPSInputType::attrPosition:	
		{
			CVectNormIterator it = (loc->getPos().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
		case TPSInputType::attrUniformRandom:	
		{
			CRandomIterator it ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
	}

	// we must alway copy the data there ...
	return tab ;
}


template <typename T, class F> 
void CPSAttribMakerT<T, F>::make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const
{
	nlassert(loc) ;
	switch (_InputType)
	{
		case TPSInputType::attrDate:	
		{
			TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, loc->getLastForever()) ;
		}
		break ;		
		case TPSInputType::attrSpeed:	
		{
			CVectNormIterator it = (loc->getSpeed().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;

		case TPSInputType::attrPosition:	
		{
			CVectNormIterator it = (loc->getPos().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
		case TPSInputType::attrInverseMass:	
		{
			TPSAttribFloat::const_iterator it = (loc->getInvMass().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
		case TPSInputType::attrUniformRandom:	
		{
			CRandomIterator it ;
			make4ByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
	}
}


template <typename T, class F> 
void CPSAttribMakerT<T, F>::makeN(CPSLocated *loc, uint32 startIndex, void *tab
								  , uint32 stride, uint32 numAttrib, uint32 nbReplicate) const
{
	nlassert(loc) ;
	nlassert(nbReplicate >= 1) ; 

switch (_InputType)
{
		case TPSInputType::attrDate:	
		{
			TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, loc->getLastForever()) ;
		}
		break ;				
		case TPSInputType::attrSpeed:	
		{
			CVectNormIterator it = (loc->getSpeed().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate,  true) ;
		}
		break ;

		case TPSInputType::attrPosition:	
		{
			CVectNormIterator it = (loc->getPos().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true) ;
		}
		break ;
		case TPSInputType::attrInverseMass:	
		{
			TPSAttribFloat::const_iterator it = (loc->getInvMass().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true) ;
		}
		break ;
		case TPSInputType::attrUniformRandom:	
		{
			CRandomIterator it ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true) ;
		}
		break ;
}
}

/** this functor 
  *
  */


/**  This class is an attribute maker that has memory, all what is does is to duplicate its mem when 'make' is called
  *  It own an attribute maker that tells how to produce the attribute from its emiter date, speed and so on ...
  */
template <typename T> class CPSAttribMakerMemory : public CPSAttribMaker<T>
{
public:	

	///\TODO : create a base class for CPSAttribMaker, that don't have the attributes not needed for this class
	/// ctor (note : we don't use the nbCycle field ...)
	CPSAttribMakerMemory() : CPSAttribMaker<T>(1.f), _Scheme(NULL)
	{
		_HasMemory = true ;
	}

	/** set a default value for initialisation, otherwise it will be garbage.
	  * This is needed when new element are generated, but not from an emitter
	  * for example, when you set this scheme to a LocatedBindable that does have a least one instance in it
	  *
	  *  example :
	  *      CPSDot *d = new CPSDot ;
	  *      CPSAttribMakerMemory<RGBA> *genAttribMaker = new CPSAttribMakerMemory<RGBA> ;
	  *      genAttribMaker->setScheme(CPSColorBlender(CRGBA::White, CRGBA::Black)
	  *      Now, if an emitter emit these particle, it'll start to emit white ones, and then black ones
      *      d->setColorScheme(  genAttribMaker) ;
	  *      now, suppose that there were several dot instanciated before the setScheme is performed :
	  *          d->newElement() ;
	  *      no color has been memorized for this element, so when setScheme is performed, it has to generate one
	  *      There are no emitter that provides it, so its taken from the default value
	  *      Note : this should only be useful in an editor, that allow the user to change the scheme with a running system ...
	  *        
	  */

    void setDefaultValue(T defaultValue) { _DefaultValue = defaultValue ;}

	/// get the default value :
	T getDefaultValue(void) const { return _DefaultValue ; }



	/** set the scheme used to store attribute. this MUST be called, otherwise an assertion will be thrown later
	  * It must have been allocated by new, and it will be deleted by this object
	  */
	void setScheme(CPSAttribMaker<T> *scheme)
	{
		nlassert(scheme) ;
		if (_Scheme) delete _Scheme ;
		_Scheme = scheme ;
	}

	/// get the scheme used
	CPSAttribMaker<T> *getScheme(void) { return _Scheme ; }
	/// get the scheme used (const version)
	const CPSAttribMaker<T> *getScheme(void) const { Return _Scheme ; }



	/// dtor
	~CPSAttribMakerMemory()
	{
		if (_Scheme)
		{
			delete _Scheme ;
		}
	}

	/// inherited from CPSAttribMaker
	virtual T get(CPSLocated *loc, uint32 index) 
	{ 
		return _T[index] ; 
	}

	/// inherited from CPSAttribMaker
	virtual void *make(CPSLocated *loc, uint32 startIndex, void *output, uint32 stride, uint32 numAttrib, bool allowNoCopy = false) const
	{
		void *tab = output ;
		if (!allowNoCopy || sizeof(T) != stride)
		{
			// we just copy what we have memorized
			CPSAttrib<T>::const_iterator it = _T.begin() + startIndex, endIt = _T.begin() + startIndex + numAttrib ;
			while (it != endIt)
			{
				*(T *) tab = *it ;
				++it ;
				tab = (uint8 *) tab + stride ;
			}
			return output ;
		}
		else
		{
			// the caller will read data directly in the vector ...
			return (void *) &(*(_T.begin() + startIndex)) ;
		}
	}	

	/// inherited from CPSAttribMaker
	virtual void make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const
	{
		// we just copy what we have memorized
		CPSAttrib<T>::const_iterator it = _T.begin() + startIndex, endIt = _T.begin() + startIndex + numAttrib ;
		while (it != endIt)
		{
			*(T *) tab = *it ;
			tab = (uint8 *) tab + stride ;
			*(T *) tab = *it ;
			tab = (uint8 *) tab + stride ;
			*(T *) tab = *it ;
			tab = (uint8 *) tab + stride ;
			*(T *) tab = *it ;
			tab = (uint8 *) tab + stride ;
			++it ;			
		}
	}

	/// inherited from CPSAttribMaker
	virtual void makeN(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const
	{
		// we just copy what we have memorized
		uint k ;
		CPSAttrib<T>::const_iterator it = _T.begin() + startIndex, endIt = _T.begin() + startIndex + numAttrib ;
		while (it != endIt)
		{
			for (k = 0 ; k < nbReplicate ; ++k)
			{
				*(T *) tab = *it ;
				tab = (uint8 *) tab + stride ;
			}			
			++it ;			
		}
	}

	/// serialisation of the object. Derivers MUST call this, (if they use the attribute of this class at least)
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		
		f.serialVersion(1) ;
		CPSAttribMaker<T>::serial(f) ;
		if (f.isReading())
		{
			if (_Scheme) delete _Scheme ;
		}
		f.serialPolyPtr(_Scheme) ;
		f.serial(_T) ;
		f.serial(_DefaultValue) ;
	}
	
	/// inherited from CPSAttribMaker
	virtual void deleteElement(uint32 index) 
	{ 
		nlassert(_Scheme) ; // you should have called setScheme !
		_T.remove(index) ; 
	}
	/// inherited from CPSAttribMaker
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) 
	{ 
		nlassert(_Scheme) ; // you should have called setScheme !
		if (emitterLocated)
		{
			_T.insert(_Scheme->get(emitterLocated, emitterIndex)) ;
		}
		else
		{			
			_T.insert(_DefaultValue) ;
		}
	}
	virtual void resize(uint32 capacity, uint32 nbPresentElements)
	{
		nlassert(_Scheme) ;
		_T.resize(capacity) ;
		if (nbPresentElements > _T.getSize())
		{
			while (_T.getSize() != nbPresentElements)
			{
				_T.insert(_DefaultValue) ;
			}
		}
		else if (nbPresentElements < _T.getSize())
		{
			while (_T.getSize() != nbPresentElements)
			{
				_T.remove(_T.getSize() - 1) ;
			}
		}
	}

protected:
	// the attribute w memorize
	CPSAttrib<T> _T ;

	// the default value for generation (when no emitter can be used)
	T _DefaultValue ;

	/** this attribute maker tells us how to produce arguments from an emitter. as an example, we may want to have a gradient
	  * of color : the emitter emit green then blue particles, following a gradient. the color is produced by _Scheme and 
	  * _T stores it
	  */
	CPSAttribMaker<T> *_Scheme ;

} ;



} // NL3D



#endif // NL_PS_ATTRIB_MAKER_H

/* End of ps_attrib_maker.h */
