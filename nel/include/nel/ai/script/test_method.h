/** \file test_method.h
 * Library class exemple for the binary test.
 *
 * $Id: test_method.h,v 1.2 2001/03/05 13:48:34 portier Exp $
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
#ifndef NL_TEST_METHOD_H
#define NL_TEST_METHOD_H

namespace NLAISCRIPT
{
	/**
	* Class CLibTest.
	* 
	* This class allows to define several method to testing validate state of agent.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CLibTest: public NLAIAGENT::IObjectIA
	{
	public:
		///This enum define ident for hard coded method that we have to import its under the script.
		enum  TMethodNumDef {
			//TIsAgent, ///Test if an expression is an agent;
			TIsNULL, ///Test if an expression is NULL;
			TLastM ///The count of export method.
		};

		enum TTypeCheck{
			CheckAll,
			CheckCount,
			DoNotCheck
		};

		///Structure to define the name, id and argument type of hard coded mathod.
		struct CMethodCall
		{
			CMethodCall(const char *name, int i,const NLAIAGENT::IObjectIA *a,TTypeCheck checkArg,int argCount, NLAIAGENT::IObjectIA *r): 
					MethodName (name),ArgType(a),ReturnValue(r)
			{
				Index = i;
				CheckArgType = checkArg;
				ArgCount = argCount;
			}

			~CMethodCall()
			{
				ReturnValue->release();
			}
			///Name of the method.
			NLAIAGENT::CStringVarName MethodName;
			///Type of the method argument.
			const NLAIAGENT::IObjectIA *ArgType;
			///Return value type.
			NLAIAGENT::IObjectIA *ReturnValue;
			///CheckArg is for force the method argument test. If its true we test juste the name coherence.
			TTypeCheck CheckArgType;			
			///Count neaded when the CheckCount it set.
			sint ArgCount;
			///Index of the method in the class.
			sint32 Index;				
		};
		///This variable its used to store method import characteristic.
		static CMethodCall StaticMethod[];

	public:
		static const NLAIC::CIdentType IdLibTest;

	public:
		CLibTest()
		{
		}

		/// \name NLAIAGENT::IObjectIA method.
		//@{
		virtual sint32 isClassInheritedFrom(const NLAIAGENT::IVarName &) const;
		
		virtual sint32 getMethodIndexSize() const
		{
			return 0;
		}
		virtual NLAIAGENT::tQueue isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *mathodName,const NLAIAGENT::IObjectIA &) const;		
		virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 heritance, sint32 index, NLAIAGENT::IObjectIA *);
		virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *);

		const NLAIAGENT::IObjectIA::CProcessResult &run(){return NLAIAGENT::IObjectIA::ProcessRun;}
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const{ return true;}	
		//@}

		/// \name NLAIC::IBasicInterface method.
		//@{
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibTest();
			return x;            
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLibTest;
		}

		void getDebugString(char *t) const
		{
			sprintf(t,"lib for serveral testing state");
		}

		void save(NLMISC::IStream &os)
		{
			
		}

		void load(NLMISC::IStream &is) 
		{				
		}
		//@}
	};
}
#endif
