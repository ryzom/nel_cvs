/** \file agent_digital.h
 * Sevral class for nomber manipulation.
 *
 * $Id: agent_digital.h,v 1.17 2002/09/10 08:26:11 chafik Exp $
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

#ifndef NL_AGENT_DIGITAL_H
#define NL_AGENT_DIGITAL_H

#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/agent_nombre.h"

namespace NLAIAGENT
{	
	
	/**
	
	This class store a float in an IObjectIA. 

	* \author Chafik sameh	 
	* \author Portier Pierre                   
	* \author Nevrax France 
	* \date 2000	
	*/
	class DigitalType : public IDigital<float>
	{

	public:
		static DigitalType NullOperator;

		static const NLAIC::CIdentType IdDigitalType;

	public:
		///Construct with an initial value.
		DigitalType(float value):IDigital<float>(value)
		{			
		}

		///Construct with an uninitialize value.
		DigitalType():IDigital<float>()
		{			
		}
		
		DigitalType(const DigitalType &a):IDigital<float>(a)
		{			
		}

		///Construct from a NEL stream.
		DigitalType(NLMISC::IStream &is):IDigital<float>(is)
		{
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}
		
		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new DigitalType(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
			
		virtual void getDebugString(std::string &text) const
		{			
			text += NLAIC::stringGetBuild("%f",getValue());
		}		

		virtual const CProcessResult &run();
		//@}				
	};


	/**	
	This class store a float in an IObjectIA. The float have a borne still as [min, max].

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000	
	*/
	class BorneDigitalType: public IBornNombre<float>
	{			

	public:
		static const NLAIC::CIdentType IdBorneDigitalType;

	public:

		/**
		Construct with a [inf, sup] born and a value.
		*/
		BorneDigitalType(float sup,float inf,float value): IBornNombre<float>(value,sup,inf)
		{
		}
		/**
		Construct with a [inf, sup] born and an uninitialize value.
		*/
		BorneDigitalType(float sup,float inf):IBornNombre<float>(sup,inf)
		{
		}		

		BorneDigitalType(const BorneDigitalType &a): IBornNombre<float>(a)
		{
		}				

		///Construct from a NEL stream.
		BorneDigitalType(NLMISC::IStream &is):IBornNombre<float>(is)
		{
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new BorneDigitalType(*this);			
			return m;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;				

		virtual const CProcessResult &run();
		
		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%f [_inf<%f> _sup<%f>]",getValue(),getInf(),getSup());
		}
		//@}

	protected:		
	};

	/**
	
	This class store a double in an IObjectIA. 

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Saffray Alain
	* \author Nevrax France
	* \date 2001
	*/
	class DDigitalType : public IDigital<double>
	{

	public:
		static const NLAIC::CIdentType IdDDigitalType;

	public:
		///Construct with an initial value.
		DDigitalType(double value):IDigital<double>(value)
		{			
		}

		///Construct with an uninitialize value.
		DDigitalType():IDigital<double>()
		{			
		}
		
		DDigitalType(const DDigitalType &a):IDigital<double>(a)
		{			
		}

		///Construct from a NEL stream.
		DDigitalType(NLMISC::IStream &is):IDigital<double>(is)
		{
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}
		
		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new DDigitalType(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%.5f",getValue());
		}		

		virtual const CProcessResult &run();
		//@}				
	};

	/**	
	This class store a double in an IObjectIA. The double have a borne still as [min, max].

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Saffray Alain
	* \author Nevrax France
	* \date 2001
	*/
	class BorneDDigitalType: public IBornNombre<double>
	{			

	public:
		static const NLAIC::CIdentType IdBorneDDigitalType;

	public:

		/**
		Construct with a [inf, sup] born and a value.
		*/
		BorneDDigitalType(double sup,double inf,double value): IBornNombre<double>(value,sup,inf)
		{
		}
		/**
		Construct with a [inf, sup] born and an uninitialize value.
		*/
		BorneDDigitalType(double sup,double inf):IBornNombre<double>(sup,inf)
		{
		}		

		BorneDDigitalType(const BorneDDigitalType &a): IBornNombre<double>(a)
		{
		}		

		///Construct from a NEL stream.
		BorneDDigitalType(NLMISC::IStream &is):IBornNombre<double>(is)
		{
		}


		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new BorneDDigitalType(*this);			
			return m;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;				

		virtual const CProcessResult &run();
		
		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%e [_inf<%e> _sup<%e>]",getValue(),getInf(),getSup());
		}
		//@}

	protected:		
	};


	/**	
	This class store a sint32 integer in an IObjectIA.

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Saffray Alain
	* \author Nevrax France
	* \date 2001
	*/
	class IntegerType: public IDigital<sint32>
	{
	public:

		static const NLAIC::CIdentType IdIntegerType;

	public:
		IntegerType(sint32 value):IDigital<sint32>(value)
		{			
		}

		IntegerType():IDigital<sint32>()
		{			
		}

		IntegerType(const IntegerType &a):IDigital<sint32>(a)
		{			
		}

		IntegerType(NLMISC::IStream &is):IDigital<sint32>(is)
		{
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}
		

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new IntegerType(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{

			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();
	};

	/**	
	This class store a sint16 short integer in an IObjectIA.

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Saffray Alain
	* \author Nevrax France
	* \date 2001
	*/
	class ShortIntegerType: public IDigital<sint16>
	{
	public:

		static const NLAIC::CIdentType IdShortIntegerType;

	public:
		ShortIntegerType(sint16 value):IDigital<sint16>(value)
		{			
		}

		ShortIntegerType():IDigital<sint16>()
		{			
		}

		ShortIntegerType(const ShortIntegerType &a):IDigital<sint16>(a)
		{			
		}

		ShortIntegerType(NLMISC::IStream &is):IDigital<sint16>(is)
		{
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}
		

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new ShortIntegerType(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();
	};

	/**	
	This class store a sint8 char in an IObjectIA.

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Saffray Alain
	* \author Nevrax France
	* \date 2001
	*/
	class CharType: public IDigital<sint8>
	{
	public:

		static const NLAIC::CIdentType IdCharType;

	public:
		CharType(sint8 value):IDigital<sint8>(value)
		{			
		}

		CharType():IDigital<sint8>()
		{			
		}

		CharType(const CharType &a):IDigital<sint8>(a)
		{			
		}

		CharType(NLMISC::IStream &is):IDigital<sint8>(is)
		{
		}

		virtual IObjetOp &neg()
		{
			_Value = -_Value;
			chekBorn();
			return *this;
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CharType(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();
	};

	class UInt8Type: public IDigital<uint8>
	{
	public:

		static const NLAIC::CIdentType IdUInt8Type;

	public:
		UInt8Type(sint8 value):IDigital<uint8>(value)
		{			
		}

		UInt8Type():IDigital<uint8>()
		{			
		}

		UInt8Type(const UInt8Type &a):IDigital<uint8>(a)
		{			
		}

		UInt8Type(NLMISC::IStream &is):IDigital<uint8>(is)
		{
		}

		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new UInt8Type(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();

		virtual IObjetOp &neg()
		{
			chekBorn();
			return *this;
		}
	};

	class UInt16Type: public IDigital<uint16>
	{
	public:

		static const NLAIC::CIdentType IdUInt16Type;

	public:
		UInt16Type(uint16 value):IDigital<uint16>(value)
		{			
		}

		UInt16Type():IDigital<uint16>()
		{			
		}

		UInt16Type(const UInt16Type &a):IDigital<uint16>(a)
		{			
		}

		UInt16Type(NLMISC::IStream &is):IDigital<uint16>(is)
		{
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new UInt16Type(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();

		virtual IObjetOp &neg()
		{
			chekBorn();
			return *this;
		}

	};

	class UInt32Type: public IDigital<uint32>
	{
	public:

		static const NLAIC::CIdentType IdUInt32Type;

	public:
		UInt32Type(uint32 value):IDigital<uint32>(value)
		{			
		}

		UInt32Type():IDigital<uint32>()
		{			
		}

		UInt32Type(const UInt32Type &a):IDigital<uint32>(a)
		{			
		}

		UInt32Type(NLMISC::IStream &is):IDigital<uint32>(is)
		{
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new UInt32Type(*this);			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();

		virtual IObjetOp &neg()
		{
			chekBorn();
			return *this;
		}

	};

	class UInt64Type: public INombre<uint64>
	{
	public:

		static const NLAIC::CIdentType IdUInt64Type;

	public:
		UInt64Type(uint64 value):INombre<uint64>(value)
		{			
		}

		UInt64Type():INombre<uint64>()
		{			
		}

		UInt64Type(const UInt64Type &a):INombre<uint64>(a)
		{			
		}

		UInt64Type(NLMISC::IStream &is):INombre<uint64>(is)
		{
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new UInt64Type(*this);
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(std::string &text) const
		{
			text += NLAIC::stringGetBuild("%d",getValue());
		}		

		virtual const CProcessResult &run();

		virtual double getNumber() const;		

		virtual IObjetOp &neg()
		{
			chekBorn();
			return *this;
		}

	};

}
#endif
