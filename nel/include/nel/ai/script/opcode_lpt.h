/** \file opcode_lpt.h
 * Sevral op-code for loading referance object to the stack.
 *
 * $Id: opcode_lpt.h,v 1.9 2002/08/21 13:58:17 lecroart Exp $
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
#ifndef NL_OPCODE_LPT_H
#define NL_OPCODE_LPT_H

namespace NLAISCRIPT
{			
/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************
	class CLptMemberOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLptMemberOpCode;
	private:
		int _B;
	public:
		CLptMemberOpCode(int b):
		_B(b)
		{
		}
		

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(char *str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLptMemberOpCode(_B);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLptMemberOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{
			sint32 b = (sint32) _B;
			os.serial( b );
		}

		void load(NLMISC::IStream &is) 
		{			
			sint32 b;
			is.serial(b);			
			_B = (int) b;
		}				
		
		~CLptMemberOpCode()
		{			
		}
	};

	class CLptStackMemberiOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLptStackMemberiOpCode;
	private:		
		 std::list<sint32> _I;
	public:
		CLptStackMemberiOpCode(std::list<sint32> b):
		_I(b)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(char *str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLptStackMemberiOpCode(_I);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLptStackMemberiOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{			
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			is.serialCont(_I);
		}
		
		~CLptStackMemberiOpCode()
		{			
		}
	};

	class CLptHeapMemberiOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLptHeapMemberiOpCode;
	private:
		std::list<sint32> _I;
		sint32 _N;
	public:
		CLptHeapMemberiOpCode(std::list<sint32> b,sint32 n):
		_I(b),_N(n)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(char *str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLptHeapMemberiOpCode(_I,_N);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLptHeapMemberiOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{			
			std::list<sint32> &i = (std::list<sint32> &) _I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			is.serialCont(_I);
		}
		
		~CLptHeapMemberiOpCode()
		{			
		}
	};

	class CLptMemberiOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLptMemberiOpCode;
	private:
		std::list<sint32> _I;
	public:
		CLptMemberiOpCode(std::list<sint32> b):
		_I(b)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(char *str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLptMemberiOpCode(_I);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLptMemberiOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os) 
		{			
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{	
			is.serialCont(_I);
		}
		
		~CLptMemberiOpCode()
		{			
		}
	};

	class CLptRefOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLptRefOpCode;
	private:
		int _B;
	public:
		CLptRefOpCode(int b):
		_B(b)
		{			
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		void getDebugResult(char *str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLptRefOpCode(_B);
			return x;

		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLptRefOpCode;
		}		

		void save(NLMISC::IStream &os)
		{
			sint32 b = (sint32) _B;
			os.serial( b );
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 b;
			is.serial( b );
			_B = (int) b;
		}		

		~CLptRefOpCode()
		{			
		}
		
	};
**********************************************************/
}

#endif
