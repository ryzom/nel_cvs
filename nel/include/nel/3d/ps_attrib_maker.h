/** \file ps_attrib_maker.h
 * <File description>
 *
 * $Id: ps_attrib_maker.h,v 1.7 2001/06/06 08:24:06 vizerie Exp $
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
#include "nel/3d/ps_located.h"
#include "nel/misc/stream.h"






namespace NL3D {

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
	 */

	  virtual void make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const = 0 ;

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
	float getNbCycles(void) const { return nbCycles ; }


	/** construct the attrib maker specifying the number of cycles to do.
	 *  \see setNbCycles()
	 */	 
	CPSAttribMaker(float nbCycles) : _NbCycles(nbCycles)
	{
	
	}


	/// tells wether one may choose one attribute from a CPSLocated to use as an input. If false, the input(s) is fixed
	virtual bool hasCustomInput(void) { return false ; }
		

	/** set a new input type (if supported). The default does nothing
	 *  \see hasCustomInput()
	 */
	virtual void setInput(CPSLocated::AttributeType input) {}


	/** get the type of input (if supported). The default return attrDate
	 *  \see hasCustomInput()
	 */
	virtual CPSLocated::AttributeType getInput(void) const { return CPSLocated::attrDate ; }
	


	/** tells wether clamping is supported for the input (value can't go above MaxInputValue)
	 *  The default is false
	 */
	bool isClampingSupported(void) const { return false ; }


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




	/// dtor
	virtual ~CPSAttribMaker() {}

	protected:

		float _NbCycles ;



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
		  virtual void make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const ;

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
			CPSAttribMaker<T>::serial(f) ;
		    f.serial(_F) ;
			f.serialEnum(_InputType) ;
			f.serial(_Clamp) ;
		}

		/** construct the attrib maker specifying the number of cycles to do.
		 *  \see setNbCycles()
		 */	 
		CPSAttribMakerT(float nbCycles) : CPSAttribMaker<T>(nbCycles), _InputType(CPSLocated::attrDate)
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
		virtual void setInput(CPSLocated::AttributeType input) { _InputType = input ; }


		/** get the type of input (if supported). The default return attrDate
		 *  \see hasCustomInput()
		 */
		virtual CPSLocated::AttributeType getInput(void) const { return _InputType ; }


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
		CPSLocated::AttributeType _InputType ;

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
	
	const float time = _NbCycles * loc->getTime()[index] ;
	return _F(time - (uint32) time) ;
	
}

template <typename T, class F> 
void CPSAttribMakerT<T, F>::make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const
{
	nlassert(loc) ;

	switch (_InputType)
	{
		case CPSLocated::attrDate:	
		{
			TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, loc->getLastForever()) ;
		}
		break ;
		case CPSLocated::attrInvMass:	
		{
			TPSAttribFloat::const_iterator it = (loc->getInvMass().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;		
		case CPSLocated::attrSpeed:	
		{
			CVectNormIterator it = (loc->getSpeed().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;

		case CPSLocated::attrPosition:	
		{
			CVectNormIterator it = (loc->getPos().begin() ) + startIndex ;
			makeByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
	}

}


template <typename T, class F> 
void CPSAttribMakerT<T, F>::make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const
{
	nlassert(loc) ;
	switch (_InputType)
	{
		case CPSLocated::attrDate:	
		{
			TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, loc->getLastForever()) ;
		}
		break ;		
		case CPSLocated::attrSpeed:	
		{
			CVectNormIterator it = (loc->getSpeed().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;

		case CPSLocated::attrPosition:	
		{
			CVectNormIterator it = (loc->getPos().begin() ) + startIndex ;
			make4ByIterator(it, tab, stride, numAttrib, true) ;
		}
		break ;
		case CPSLocated::attrInvMass:	
		{
			TPSAttribFloat::const_iterator it = (loc->getInvMass().begin() ) + startIndex ;
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
		case CPSLocated::attrDate:	
		{
			TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, loc->getLastForever()) ;
		}
		break ;				
		case CPSLocated::attrSpeed:	
		{
			CVectNormIterator it = (loc->getSpeed().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate,  true) ;
		}
		break ;

		case CPSLocated::attrPosition:	
		{
			CVectNormIterator it = (loc->getPos().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true) ;
		}
		break ;
		case CPSLocated::attrInvMass:	
		{
			TPSAttribFloat::const_iterator it = (loc->getInvMass().begin() ) + startIndex ;
			makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true) ;
		}
		break ;
}
}


/*
template <typename T, class F> void CPSAttribMakerT<T, F>::makeN(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const
{
	nlassert(loc) ;
	nlassert(nbReplicate  != 0) ;

	TPSAttribTime::const_iterator it = (loc->getTime().begin() ) + startIndex ;
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
*/



} // NL3D


#endif // NL_PS_ATTRIB_MAKER_H

/* End of ps_attrib_maker.h */
