/** \file registry_type.h
 * Includes some class for typing objects.
 *
 * $Id: registry_type.h,v 1.18 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_REGISTER_TYPE_H
#define NL_REGISTER_TYPE_H

#include <iostream>
#include "nel/misc/types_nl.h"

namespace NLAIC
{		
	/**
	CBinaryType is a basic binnary nomber manipulation.

	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/

	class CBinaryType: public NLAIC::IBasicInterface
	{
	private:
		///Binary nomber.
		uint32 _Bites;

	public:
		///Construct with an initial nomber.
		CBinaryType(uint32 bites):_Bites(bites)
		{			
		}

		///Copy constroctor.
		CBinaryType(const CBinaryType &bites):_Bites(bites._Bites)
		{			
		}

		///Construct from stream.
		CBinaryType(NLMISC::IStream &is)
		{			
			load(is);
		}

		///defult constructor.
		CBinaryType():_Bites(0)
		{						
		}


		/// \name binary method.
		//@{
		CBinaryType &operator = (sint32 bites)
		{
			_Bites = bites;
			return *this;
		}
		//@}
		
		/// \name operator OR.
		//@{
		CBinaryType &operator |= (sint32 bites)
		{
			_Bites |= bites;
			return *this;	
		}
		CBinaryType &operator |= (const CBinaryType &bites)
		{
			_Bites |= bites._Bites;
			return *this;
		}
		CBinaryType operator | (sint32 bites)
		{			
			return CBinaryType(_Bites | bites);
		}		


		CBinaryType operator | (const CBinaryType &bites)
		{
			
			return CBinaryType(_Bites | bites._Bites);
		}
		//@}

		/// \name operator AND.
		//@{
		CBinaryType &operator &= (sint32 bites)
		{
			_Bites &= bites;
			return *this;
		}
		CBinaryType &operator &= (const CBinaryType &bites)
		{
			_Bites &= bites._Bites;
			return *this;
		}
		CBinaryType operator & (sint32 bites)
		{			
			return CBinaryType(_Bites & bites);
		}
		CBinaryType operator & (const CBinaryType &bites)
		{			
			return CBinaryType(_Bites & bites._Bites);
		}
		//@}
		
		/// \name operator XOR.
		//@{
		CBinaryType &operator ^= (sint32 bites)
		{
			_Bites ^= bites;
			return *this;
		}
		
		CBinaryType &operator ^= (const CBinaryType &bites)
		{
			_Bites ^= bites._Bites;
			return *this;
		}
		CBinaryType operator ^ (sint32 bites)
		{			
			return CBinaryType(_Bites ^ bites);
		}
		CBinaryType operator ^ (const CBinaryType &bites)
		{
			return CBinaryType(_Bites ^ bites._Bites);
		}
		//@}

		/// \name operator LSHIFT.
		//@{
		CBinaryType &operator <<= (sint32 bites)
		{
			_Bites <<= bites;
			return *this;
		}		
		CBinaryType operator << (sint32 bites)
		{
			return CBinaryType(_Bites << bites);
		}
		//@}

		/// \name operator RSHIFT.
		//@{
		CBinaryType &operator >>= (sint32 bites)
		{
			_Bites >>= bites;
			return *this;
		}
		CBinaryType operator >> (sint32 bites)
		{			
			return CBinaryType(_Bites >> bites);
		}

		///Not operator.
		bool operator !()
		{
			return !_Bites;
		}
		//@}

		///Get value of class.
		operator uint () const
		{
			return _Bites;
		}

		uint getValue() const
		{
			return _Bites;
		}

	public:
		/// \name IBasicInterface method.
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{

			throw NLAIE::CExceptionUnReference(std::string("const CIdentType & CBinaryType::getType()"));
			return NLAIC::CIdentType::VoidType;
		}

		virtual const NLAIC::IBasicType *clone() const 
		{
			return new CBinaryType(*this);
		}
		virtual const NLAIC::IBasicType *newInstance() const 
		{
			return new CBinaryType();
		}
		virtual void save(NLMISC::IStream &os)
		{
			uint32 bytes = (uint32) _Bites;
			os.serial( bytes );
		}

		virtual void getDebugString(std::string &t) const 
		{
			t = stringGetBuild("CBinaryType <%d>",_Bites);
		}

		virtual void load(NLMISC::IStream &is)
		{
			uint32 bytes;
			is.serial( bytes );
			_Bites = (uint32) bytes;
		}
		//@}

		void addType(sint32 b)
		{
			_Bites = _Bites | b;
		}
	};


	/**
	CTypeOfObject is an binnary type of objects.

	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class CTypeOfObject: public CBinaryType
	{
	public:
		static NLAIC::CIdentType IdTypeOfObject;
		
	public:
		///define the type of an objects.
		enum TTypeBite{
			///The object is a nomber.
			tNombre = 1,
			///The object is a string.
			TString = 2,
			///The object is a logic objects (fuzzy numerical or first ordrer).
			tLogic = 4,
			///The object is a list.
			tList = 8,			
			//tAdresse = 16,
			///The object is an agent.
			tAgent = 32,
			//tExec = 64,
			///The object is an agent defiend with the interpret. Agents class must derived from NLAIAGENT::CAgentScript.
			tInterpret = 256,
			///The object is an IBasicInterface object.
			tObject = 128,			
			///The object is an performative.
			tPerformative = 512,
			/// The object is an actor
			tActor = 1024,
			tMessage = 2048,
			///Last definition.
			tLast = tMessage,
			///That is an undef object.
			tUndef = 0xFFFFFFFF - tLast*2
			
		};

		CTypeOfObject()
		{
		}
	
		CTypeOfObject(uint32 bites):CBinaryType(bites)
		{
		}

		CTypeOfObject(const CTypeOfObject &bites):CBinaryType(bites)
		{			
		}

		CTypeOfObject(NLMISC::IStream &is):CBinaryType(is)
		{
		}		
		
		virtual ~CTypeOfObject()
		{			
		}
	public:
		/// \name IBasicInterface method.
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CTypeOfObject(*this);
			return x;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			NLAIC::IBasicType *x = new CTypeOfObject();			
			return x;
		}

		virtual const NLAIC::CIdentType &getType() const;
		virtual void getDebugString(char *txt) const;
		//@}
	};

	/**
	CTypeOfOperator is an binnary type of operation allowed by objects.

	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class CTypeOfOperator: public CBinaryType
	{
	public:
		static NLAIC::CIdentType IdTypeOfOperator;
		
	public:
		///define the type of an operation.
		enum TTypeOp{
			///None operation is define.
			opNone = 0,
			///Can add this objects with an ather.
			opAdd = 1,
			///Can substract this objects with an ather.
			opSub = 2,
			///Can multiply this objects with an ather.
			opMul = 4,
			///Can divide this objects with an ather.
			opDiv = 8,
			///Can test the equality this objects with an ather.
			opEq = 16,
			///Can test with the < operator.
			opInf = 32,
			///Can test with the > operator.
			opSup = 64,
			///Can test with the <= operator.
			opInfEq = 128,
			///Can test with the >= operator.
			opSupEq = 256,
			///Can test with the ! operator.
			opNot = 512,
			///Can test with the != operator.
			opDiff = 1024,
			///Can dot obj = -obj.
			opNeg = 2048,
			///Can affect the object.
			opAff = 4096,
			///That is an undef.
			tUndef = 0xFFFFFFFF - opAff*2 + 1
		};

		CTypeOfOperator()
		{
		}

		CTypeOfOperator(uint32 bites):CBinaryType(bites)
		{		
		}

		
		CTypeOfOperator(const CTypeOfOperator &bites):CBinaryType(bites)
		{			
		}
		
		CTypeOfOperator(NLMISC::IStream &is):CBinaryType(is)
		{
		}
		
		
		virtual ~CTypeOfOperator()
		{
		}
	public:
		/// \name IBasicInterface method.
		//@{

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CTypeOfOperator(*this);
			return x;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			NLAIC::IBasicType *x = new CTypeOfOperator();
			return x;
		}

		virtual const NLAIC::CIdentType &getType() const;
		virtual void getDebugString(char *txt) const;
		//@}
	};
}

#endif
