/** \file agent_digital.h
 * Sevral class for nomber manipulation.
 *
 * $Id: agent_digital.h,v 1.7 2001/01/18 15:16:19 puzin Exp $
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
	class DigitalType : public INombre<float>
	{

	public:
		static DigitalType NullOperator;

		static const NLAIC::CIdentType IdDigitalType;

	public:
		///Construct with an initial value.
		DigitalType(float value):INombre<float>(value)
		{			
		}

		///Construct with an uninitialize value.
		DigitalType():INombre<float>()
		{			
		}
		
		DigitalType(const DigitalType &a):INombre<float>(a)
		{			
		}

		///Construct from a NEL stream.
		DigitalType(NLMISC::IStream &is):INombre<float>(is)
		{
		}
		
		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new DigitalType(*this);
			m->incRef();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(char *text) const
		{
			sprintf(text,"<float> %f",getValue());
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

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new BorneDigitalType(*this);
			m->incRef();
				return m;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;				

		virtual const CProcessResult &run();
		
		virtual void getDebugString(char *text) const
		{
			sprintf(text,"_Value<%f> [_inf<%f> _sup<%f>]",getValue(),getInf(),getSup());
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
	class DDigitalType : public INombre<double>
	{

	public:
		static const NLAIC::CIdentType IdDDigitalType;

	public:
		///Construct with an initial value.
		DDigitalType(double value):INombre<double>(value)
		{			
		}

		///Construct with an uninitialize value.
		DDigitalType():INombre<double>()
		{			
		}
		
		DDigitalType(const DDigitalType &a):INombre<double>(a)
		{			
		}

		///Construct from a NEL stream.
		DDigitalType(NLMISC::IStream &is):INombre<double>(is)
		{
		}
		
		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new DDigitalType(*this);
			m->incRef();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(char *text) const
		{
			sprintf(text,"%e",getValue());
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

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new BorneDDigitalType(*this);
			m->incRef();
				return m;
		}		

		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;				

		virtual const CProcessResult &run();
		
		virtual void getDebugString(char *text) const
		{
			sprintf(text,"_Value<%e> [_inf<%e> _sup<%e>]",getValue(),getInf(),getSup());
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
	class IntegerType: public INombre<sint32>
	{
	public:

		static const NLAIC::CIdentType IdIntegerType;

	public:
		IntegerType(sint32 value):INombre<sint32>(value)
		{			
		}

		IntegerType():INombre<sint32>()
		{			
		}

		IntegerType(const IntegerType &a):INombre<sint32>(a)
		{			
		}

		IntegerType(NLMISC::IStream &is):INombre<sint32>(is)
		{
		}
		

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new IntegerType(*this);
			m->incRef();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(char *text) const
		{
			sprintf(text,"%d",getValue());
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
	class ShortIntegerType: public INombre<sint16>
	{
	public:

		static const NLAIC::CIdentType IdShortIntegerType;

	public:
		ShortIntegerType(sint16 value):INombre<sint16>(value)
		{			
		}

		ShortIntegerType():INombre<sint16>()
		{			
		}

		ShortIntegerType(const ShortIntegerType &a):INombre<sint16>(a)
		{			
		}

		ShortIntegerType(NLMISC::IStream &is):INombre<sint16>(is)
		{
		}
		

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new ShortIntegerType(*this);
			m->incRef();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(char *text) const
		{
			sprintf(text,"%d",getValue());
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
	class CharType: public INombre<sint8>
	{
	public:

		static const NLAIC::CIdentType IdCharType;

	public:
		CharType(sint8 value):INombre<sint8>(value)
		{			
		}

		CharType():INombre<sint8>()
		{			
		}

		CharType(const CharType &a):INombre<sint8>(a)
		{			
		}

		CharType(NLMISC::IStream &is):INombre<sint8>(is)
		{
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CharType(*this);
			m->incRef();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		virtual const NLAIC::CIdentType &getType() const;		
		

		virtual void getDebugString(char *text) const
		{
			sprintf(text,"%c",getValue());
		}		

		virtual const CProcessResult &run();
	};
}


#endif
