/** \file interpret_methodes.h
 * Sevral class for the compiler fonctionality.
 *
 * $Id: interpret_methodes.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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
#ifndef NL_INTERPRET_METHODE_H
#define NL_INTERPRET_METHODE_H

#include "script/virtual_op_code.h"
#include "agent/agent.h"

namespace NLIASCRIPT
{
	class IOpType;

	/**
	* Class CParam.
	* 
	* This class is an type container who's strored the arguments type of a given method.
	* 
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/
	class CParam : public NLIAAGENT::IObjectIA
	{
	private:		
		typedef std::vector<IOpType *> tParam;
	private:
		///Vector that is contain a IOpType argument type.
		tParam			_Param;

	public:
		static const NLIAC::CIdentType IdParam;		

		CParam();
		CParam(const CParam &);
		~CParam();
		///Need to know if a CParam is equal as an ather.
		bool operator == (const CParam &p) const;
		/**
		Return the distance between to arguments.
		An infinite result is defined by a negative value and represent an unequal argument.
		0.0 value mean that the two arguments is equal.
		A positive value mean that some of parameter is unequal but have the same based class, in this configuration the return value represent 
		how far the two class was between the base class. More the value is more far the class is.
		*/
		double eval(const CParam &p) const;
		///Make equality.
		void operator = (const CParam &p);
		///get a parameter in the vector.
		const IOpType *operator[] (sint32 i) const;
		///Push new parameter type define in a list define by an NLIAAGENT::IBaseGroupType object.
		void push(const NLIAAGENT::IBaseGroupType &);
		///Push new parameter type. Here we clone the arguments.
		void push(const IOpType &);
		///Push new parameter type. User have to make an incRef().
		void push(IOpType *i);

		///get the last parameter added.
		const IOpType &get() const;
		///pop the last parameter added.
		void pop();
		///Clear the vector containt.
		void clear();
		///get vector size.
		sint32 size() const;

		/// \name NLIAC::IBasicInterface and NLIAAGENT::IObjectIA method.
		//@{
		virtual const NLIAC::CIdentType &getType() const;
		virtual const NLIAC::IBasicType *clone() const;
		virtual const NLIAC::IBasicType *newInstance() const;
		virtual void getDebugString(char *) const;
		virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &) const;
		void load(NLMISC::IStream &);
		void save(NLMISC::IStream &);		
		virtual const NLIAAGENT::IObjectIA::CProcessResult &run();
		//@}

		void getString(char *txt) const;		
	};

	/**
	* Class CMethodeName.
	* 
	* This class define what a method is. A methid have a name, argument, a branche code ant a return type.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/
	class CMethodeName : public NLIAAGENT::IObjectIA
	{	
	private:
		///Name of the method.
		NLIAAGENT::IVarName	*_MethodeName;
		///op code pointer for run method.
		IOpCode			*_Code;
		///_Param is the rgument of the method.
		CParam			_Param;
		///Type of return expression.
		IOpType			*_TypeOfMethode;
		
	public:
		static const NLIAC::CIdentType IdMethodeName;

		CMethodeName(const NLIAAGENT::IVarName &name);
		CMethodeName(const CMethodeName &);
		CMethodeName();
		~CMethodeName();
		void setName(const NLIAAGENT::IVarName &name);
		const NLIAAGENT::IVarName &getName() const;		
		void setParam(const CParam &p);
		void setTypeOfMethode(IOpType *);
		CParam &getParam();
		void setCode(IOpCode *c);
		IOpCode &getCode();
		const IOpType *getTypeOfMethode() const;

		/// \name NLIAC::IBasicInterface and NLIAAGENT::IObjectIA method.
		//@{
		void load(NLMISC::IStream &);
		void save(NLMISC::IStream &);
		virtual const NLIAC::CIdentType &getType() const;
		virtual const NLIAC::IBasicType *clone() const;
		const NLIAC::IBasicType *newInstance() const;
		virtual void getDebugString(char *) const;
		virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &) const;
		virtual const NLIAAGENT::IObjectIA::CProcessResult &run();
		//@}
	};
}
#endif
