/** \file libcode.h
 * Library class exemple for the script.
 *
 * $Id: libcode.h,v 1.8 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_LIB_CODE_H
#define NL_LIB_CODE_H

namespace NLAISCRIPT
{	

	/**
	* Class CCallPrint.
	* 
	* This class allows to print and formated string. In the script its define the print('string',objects, ...); expression
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CCallPrint: public NLAIAGENT::IObjectIA
	{

	public:
		static const NLAIC::CIdentType IdCallPrint;
		static NLAIC::IIO *inputOutput;

	public:	

		CCallPrint()
		{
		}

		
		/// \name NLAIAGENT::IObjectIA method.
		//@{
		virtual sint32 isClassInheritedFrom(const NLAIAGENT::IVarName &) const
		{
			return 0;
		}		
		virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *mathodName,const NLAIAGENT::IObjectIA &) const;		
		virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 heritance, sint32 index, NLAIAGENT::IObjectIA *);
		virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *);

		const NLAIAGENT::IObjectIA::CProcessResult &run(){return NLAIAGENT::IObjectIA::ProcessRun;}
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const{ return true;}	
		//@}
		
				
		///Returns the const NLAIAGENT::IBaseGroupType * as an formated string.
		NLAIAGENT::IObjectIA::CProcessResult format(const NLAIAGENT::IBaseGroupType *g);
		///Print the contents of the NLAIAGENT::IBaseGroupType.
		NLAIAGENT::IObjectIA::CProcessResult printList(const NLAIAGENT::IBaseGroupType *g);

		/// \name NLAIC::IBasicInterface method.
		//@{
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CCallPrint();
			return x;            
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdCallPrint;
		}

		virtual void getDebugString(std::string &t) const
		{
			t = "Print()";
		}

		void save(NLMISC::IStream &os)
		{
			
		}

		void load(NLMISC::IStream &is) 
		{				
		}
		//@}
		
		virtual ~CCallPrint()
		{
		}
	};
		
	void initExternalLib();
}
#endif
