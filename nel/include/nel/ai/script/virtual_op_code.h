/** \file virtual_op_code.h
 * Basic definition for op-codes maniputation.
 * $Id: virtual_op_code.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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
#ifndef NL_VIRTUAL_OP_CODE_H
#define NL_VIRTUAL_OP_CODE_H

namespace NLIASCRIPT
{	
	class CCodeContext;	


	/**
	* Class IOpCode.
	* 
	* This class define an object basic op-code, an op-code is an NLIAAGENT::IObjectIA hows have to run with an CCodeContext.
	* 
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class IOpCode: public NLIAAGENT::IObjectIA
	{
	public:
		IOpCode()
		{
		}
		
		///Computation of the op-code with a given context.
		virtual	NLIAAGENT::TProcessStatement runOpCode(CCodeContext &context) = 0;
		///gets debug string from the op-code.
		virtual void getDebugResult(char *str,CCodeContext &context) const = 0;
		virtual ~IOpCode()
		{
		}
	};
	
	/**
	* Class IOpCode.
	* 
	* The class allow to define a branch of code sam as a closure in the fuctional language.
	* 	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class ICodeBranche: public IOpCode
	{
		public:
			ICodeBranche()
			{
			}

			///Get the nex op-code to run.
			virtual IOpCode &nextCode() = 0;
			///Run for a given self class. The method create a new context.
			virtual const NLIAAGENT::IObjectIA::CProcessResult &run(NLIAAGENT::IObjectIA &self) = 0;
			///Run with a given context
			virtual const NLIAAGENT::IObjectIA::CProcessResult &run(CCodeContext &) = 0;

			virtual ~ICodeBranche()
			{
			}
	};	
	

	
	/**
	* Class IOpRunCode.
	* 
	* This class allow to define a op-code whis a predefined run() getDebugString(char *) and isquel() ... method's. 
	* These method's have the same bihavious for all derivation.
	* 	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class IOpRunCode:public IOpCode
	{		
	public:

		IOpRunCode()
		{
		}

		const NLIAAGENT::IObjectIA::CProcessResult &run(){return NLIAAGENT::IObjectIA::ProcessRun;}
		void getDebugString(char *) const{ }
		bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const{ return true;}	

		virtual ~IOpRunCode()
		{
		}
		
	};

}
#endif
