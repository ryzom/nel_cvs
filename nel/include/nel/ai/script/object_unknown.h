/** \file object_unknown.h
 * Class for store an IOpType.
 *
 * $Id: object_unknown.h,v 1.3 2001/01/08 10:50:46 chafik Exp $
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

#ifndef NL_OBJECT_UNKNOWN_H
#define NL_OBJECT_UNKNOWN_H


namespace NLIASCRIPT
{
	/**
	* Class CObjectUnknown.
	* 
	* this class define an IObjectIA as an type. This type is divide in two part, the real type of the objects and a functional type.
	* This last is use when we have to have an script compoment member, method member or any internal computation. The real type is use for the comparaison between type.
	* This is very useful for ambiguous object same as an agent where he can be the agent or a mail box. Or a mail box can be an agent. A correct syntax is
	* to say i have a type of object as the type of an other object.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CObjectUnknown: public NLAIAGENT::IObjectIA	
	{
	
	private:
		///Type of the object.
		IOpType *_opType;
		///Functional type.
		IOpType *_opBaseType;

	public:
		///Construct the object with one type _opType = _opBaseType;
		CObjectUnknown(IOpType *opType)
		{
			_opType = opType;
			_opBaseType = _opType;
			_opBaseType->incRef();
		}

		///Construct the object with an opType and an opBaseType;
		CObjectUnknown(IOpType *opType,IOpType *opBaseType)
		{
			_opType = opType;
			_opBaseType = opBaseType;
		}

		/// \name Base class method.
		//@{
		const NLAIC::CIdentType &getType() const throw (NLAIE::CExceptionUnReference)
		{
			const NLAIC::CIdentType *id = _opType->getConstraintTypeOf();
			if(id == NULL) throw NLAIE::CExceptionUnReference("object type is unreference");
			return *id;
		}

		const NLAIC::IBasicType *clone() const
		{
			_opType->incRef();
			_opBaseType->incRef();
			return new CObjectUnknown(_opType,_opBaseType);
		}

		const NLAIC::IBasicType *newInstance() const
		{
			return clone();			
		}

		void getDebugString(char *txt) const
		{
			const NLAIC::CIdentType *id = _opType->getConstraintTypeOf();
			if(id)
			{
				sprintf(txt,"CObjectUnknown< _opType < %s > >", (const char *)*id);
			}
			else 
				sprintf(txt,"CObjectUnknown< _opType < NULL > >");
		}

		void save(NLMISC::IStream &os)
		{
			
		}

		void load(NLMISC::IStream &is)
		{
		}

		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const 
		{
			return true;
		}		

		const NLAIAGENT::IObjectIA::CProcessResult &run()
		{
			return NLAIAGENT::IObjectIA::ProcessRun;;
		}
		//@}
		

		/// \name Type manipulation.
		//@{
		const IOpType *getBaseType() const
		{
			return _opBaseType;
		}

		const IOpType *getClassType() const
		{
			return _opType;

		}

		void setClassType(IOpType *o)
		{
			_opType->release();
			_opType = o;
		}

		void setBaseType(IOpType *o)
		{
			_opBaseType->release();
			_opBaseType = o;
		}
		//@

		~CObjectUnknown()
		{
			_opType->release();
			_opBaseType->release();
		}
	};
}


#endif
