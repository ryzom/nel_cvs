/** \file agent_digital.h
 * Sevral class for nomber manipulation.
 *
 * $Id: agent_digital.h,v 1.3 2001/01/08 10:47:05 chafik Exp $
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

#include "agent/agent.h"
#include "agent/agent_object.h"
#include "agent/agent_nombre.h"

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
			sprintf(text,"%f",getValue());
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

		virtual ~BorneDigitalType()
		{
		}

	protected:		
	};

	/*class IntegerType: public INombre<sint32>
	{
	public:

		static const NLAIC::CIdentType IdIntegerType;

	public:
		IntegerType(float value):INombre<sint32>((sint32)value)
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
			sprintf(text,"%f",getValue());
		}		

		virtual const CProcessResult &run();
	};*/
}

#endif
