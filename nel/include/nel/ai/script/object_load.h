/** \file object_load.h
 * Sevral class for loading object from the context.
 *
 * $Id: object_load.h,v 1.6 2001/05/22 16:08:01 chafik Exp $
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

#ifndef NL_OBJECT_LOAD_H
#define NL_OBJECT_LOAD_H

namespace NLAISCRIPT
{
	/**
	* Class ILoadObject.
	* 
	* This class is an abstract class and it allow to gets an object from the compiler context with sever way.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class ILoadObject: public NLAIC::IBasicInterface
	{
	public:
		ILoadObject()
		{

		}

		/// \name Programmer have to define those abstract meyhod.
		//@{
		///Extract an object from the context.
		virtual const NLAIAGENT::IObjectIA *getObject(CCodeContext &context) const = 0;
		///Extract a method argument from context.
		virtual const NLAIAGENT::IBaseGroupType *getParam(CCodeContext &context) const = 0;
		//@}

		virtual void getDebugString(std::string &) const{}
		virtual ~ILoadObject()
		{
		}
	};

	/**
	* Class CLoadSelfObject.
	* 
	* This class is a concrete class allow to gets an compoment from the context.Self object.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CLoadSelfObject: public ILoadObject
	{		
	public:
		static const NLAIC::CIdentType IdLoadSelfObject;
	private:
		std::list<sint32> _I;
	public:
		CLoadSelfObject()
		{
		}

		CLoadSelfObject(const std::list<sint32> &i):_I(i)
		{
		}


		///Get an compoment from the context.Self object.
		virtual const NLAIAGENT::IObjectIA *getObject(CCodeContext &context) const;
		///Get the argument define in the stack.
		virtual const NLAIAGENT::IBaseGroupType *getParam(CCodeContext &context) const;


		/// \name NLAIC::IBasicInterface.
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdLoadSelfObject;
		}

		virtual const NLAIC::IBasicType *clone() const 
		{
			NLAIC::IBasicType *x= new CLoadSelfObject(_I);
			return x;
		}		

		virtual const NLAIC::IBasicType *newInstance() const 
		{
			NLAIC::IBasicType *x= new CLoadSelfObject();
			return x;
		}

		virtual void save(NLMISC::IStream &os)
		{
			std::list<sint32> &i = (std::list<sint32> &) _I;
			os.serialCont(i);			
		}
		virtual void load(NLMISC::IStream &is) 
		{
			is.serialCont(_I);			
		}
		//@}
	};

	class CLoadStackObject: public ILoadObject
	{		
	public:
		static const NLAIC::CIdentType IdLoadSelfObject;
	private:
		std::list<sint32> _I;

	public:
		CLoadStackObject(){}
		CLoadStackObject(const std::list<sint32> &i):_I(i)
		{
		}

		///Get an compoment from the stack object.
		virtual const NLAIAGENT::IObjectIA *getObject(CCodeContext &context) const;
		///Get the argument define in the stack for an object stored in the stack.
		virtual const NLAIAGENT::IBaseGroupType *getParam(CCodeContext &context) const;

		/// \name NLAIC::IBasicInterface.
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdLoadSelfObject;
		}

		virtual const NLAIC::IBasicType *clone() const 
		{
			NLAIC::IBasicType *x= new CLoadStackObject(_I);
			return x;
		}		

		virtual const NLAIC::IBasicType *newInstance() const 
		{
			NLAIC::IBasicType *x= new CLoadStackObject();
			return x;
		}

		virtual void save(NLMISC::IStream &os)
		{
			std::list<sint32> &i = (std::list<sint32> &) _I;
			os.serialCont(i);			
		}
		virtual void load(NLMISC::IStream &is) 
		{
			is.serialCont(_I);			
		}
		//@}
	};

	class CLoadHeapObject: public ILoadObject
	{
	public:
		static const NLAIC::CIdentType IdLoadHeapObject;
	private:
		std::list<sint32> _I;
		sint32 _N;
	public:
		CLoadHeapObject(){}
		CLoadHeapObject(const std::list<sint32> &i,int n):_I(i),_N(n)
		{
		}

		///Get an compoment from the heap object.
		virtual const NLAIAGENT::IObjectIA *getObject(CCodeContext &context) const;
		///Get the argument define in the stack for an object stored in the heap.
		virtual const NLAIAGENT::IBaseGroupType *getParam(CCodeContext &context) const;

		/// \name NLAIC::IBasicInterface.
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdLoadHeapObject;
		}

		virtual const NLAIC::IBasicType *clone() const 
		{
			NLAIC::IBasicType *x= new CLoadHeapObject(_I,_N);
			return x;
		}		

		virtual const NLAIC::IBasicType *newInstance() const 
		{
			NLAIC::IBasicType *x= new CLoadHeapObject();
			return x;
		}

		virtual void save(NLMISC::IStream &os)
		{
			std::list<sint32> &i = (std::list<sint32> &) _I;
			os.serialCont(i);
			sint32 n = _N;
			os.serial(n);
		}
		virtual void load(NLMISC::IStream &is) 
		{
			is.serialCont(_I);
			is.serial(_N);
		}
		//@}
	};
}

#endif
