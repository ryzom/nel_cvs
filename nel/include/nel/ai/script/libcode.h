/** \file libcode.h
 * Library class exemple for the script.
 *
 * $Id: libcode.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

namespace NLIASCRIPT
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
	class CCallPrint: public NLIAAGENT::IObjectIA
	{

	public:
		static const NLIAC::CIdentType IdCallPrint;
		static NLIAC::IIO *inputOutput;

	public:	

		CCallPrint()
		{
		}

		
		/// \name NLIAAGENT::IObjectIA method.
		//@{
		virtual sint32 isClassInheritedFrom(const NLIAAGENT::IVarName &) const
		{
			return -1;
		}		
		virtual NLIAAGENT::tQueue isMember(const NLIAAGENT::IVarName *className,const NLIAAGENT::IVarName *mathodName,const NLIAAGENT::IObjectIA &) const;		
		virtual	NLIAAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 heritance, sint32 index, NLIAAGENT::IObjectIA *);
		virtual	NLIAAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index,NLIAAGENT::IObjectIA *);

		const NLIAAGENT::IObjectIA::CProcessResult &run(){return NLIAAGENT::IObjectIA::ProcessRun;}
		bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const{ return true;}	
		//@}
		
				
		///Returns the const NLIAAGENT::IBaseGroupType * as an formated string.
		NLIAAGENT::IObjectIA::CProcessResult format(const NLIAAGENT::IBaseGroupType *g);
		///Print the contents of the NLIAAGENT::IBaseGroupType.
		NLIAAGENT::IObjectIA::CProcessResult printList(const NLIAAGENT::IBaseGroupType *g);

		/// \name NLIAC::IBasicInterface method.
		//@{
		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CCallPrint();
			x->incRef();         
			return x;            
		}

		const NLIAC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLIAC::CIdentType &getType() const
		{
			return IdCallPrint;
		}

		void getDebugString(char *t) const
		{
			sprintf(t,"Print()");
		}

		void save(NLMISC::IStream &os)
		{
			
		}

		void load(NLMISC::IStream &is) 
		{				
		}
		//@}
		
		~CCallPrint()
		{
		}
	};
		
	void initExternalLib();
}
#endif
