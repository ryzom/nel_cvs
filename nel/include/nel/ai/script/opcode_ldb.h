/** \file opcode_ldb.h
 * Sevral op-code for loading object to the stack.
 *
 * $Id: opcode_ldb.h,v 1.9 2003/02/04 14:21:52 chafik Exp $
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
#ifndef NL_OPCODE_LDB_H
#define NL_OPCODE_LDB_H

namespace NLAISCRIPT
{			
	/**
	This class is an op-code, it inc the stack ref and add the content of a variable to the curent stack.
	*/
	class CLdbOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbOpCode;

	private:
		///Var that it add to the stack.
		NLAIAGENT::IObjectIA *_B;
	public:
		CLdbOpCode(const NLAIAGENT::IObjectIA &b);		

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(std::string &,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbOpCode(*_B);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbOpCode;
		}

		void getDebugString(std::string &) const{ }

		void save(NLMISC::IStream &os)
		{
			os.serial( (NLAIC::CIdentType &) _B->getType() );
			_B->save(os);		
		}

		void load(NLMISC::IStream &is) 
		{
			_B->release();
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			_B = (NLAIAGENT::IObjectIA *)id.allocClass();
			_B->load(is);
		}				
		
		virtual ~CLdbOpCode()
		{
			_B->release();
		}
	};

	/**
	This class is an op-code, it inc the stack ref and add the content of a member attribut to the curent stack. 
	The member is extrat by clone from the attibute Self of the structur CCodeContext.
	*/
	class CLdbMemberOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbMemberOpCode;
	private:
		int _B;
	public:
		CLdbMemberOpCode(int b):
		_B(b)
		{
		}
		

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(std::string &str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbMemberOpCode(_B);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbMemberOpCode;
		}		

		void getDebugString(std::string &) const{ }

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
		
		virtual ~CLdbMemberOpCode()
		{			
		}
	};
	/**
	This class is an op-code, it inc the stack ref and add the content of a sub member attribut to the curent stack. 
	The member is extrat by clone from the attibute Self of the structur CCodeContext. This op code is called when it is necessary
	to load a member as x1.x2...xn
	*/
	class CLdbMemberiOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbMemberiOpCode;
	private:
		std::list<sint32> _I;
	public:
		CLdbMemberiOpCode(std::list<sint32> b):
		_I(b)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(std::string &str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbMemberiOpCode(_I);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbMemberiOpCode;
		}

		void getDebugString(std::string &) const{ }

		void save(NLMISC::IStream &os)
		{			
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{	
			is.serialCont(_I);
		}
		
		virtual ~CLdbMemberiOpCode()
		{			
		}
	};

	/**
	This class is an op-code, it inc the stack ref and add the content of a sub member attribut to the curent stack. 
	The member is extrat by clone from the current position on the stack. This op code is called when it is necessary
	to load a member as x1.x2...xn
	*/
	class CLdbStackMemberiOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbStackMemberiOpCode;
	private:

		///Range of attibut.
		std::list<sint32> _I;
	public:
		CLdbStackMemberiOpCode(std::list<sint32> b):
		_I(b)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(std::string &str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbStackMemberiOpCode(_I);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbStackMemberiOpCode;
		}

		void getDebugString(std::string &) const{ }

		void save(NLMISC::IStream &os)
		{			
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			is.serialCont(_I);
		}
		
		virtual ~CLdbStackMemberiOpCode()
		{			
		}
	};


	/**
	This class is an op-code, it inc the stack ref and add the content of a sub member attribut to the curent stack. 
	The member is extrat by clone from an arbitrary position on the heap. This op code is called when it is necessary
	to load a member as x1.x2...xn
	*/
	class CLdbHeapMemberiOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbHeapMemberiOpCode;
	private:
		std::list<sint32> _I;
		sint32 _N;
	public:
		CLdbHeapMemberiOpCode(const std::list<sint32> &b,sint32 n):
		_I(b),_N(n)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(std::string &str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbHeapMemberiOpCode(_I,_N);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbHeapMemberiOpCode;
		}

		void getDebugString(std::string &) const{ }

		void save(NLMISC::IStream &os)
		{			
			std::list<sint32> &i = (std::list<sint32> &) _I;
			os.serialCont(i);
			sint32 n = _N;
			os.serial(n);
		}

		void load(NLMISC::IStream &is) 
		{				
			is.serialCont(_I);
			is.serial(_N);
		}
		
		virtual ~CLdbHeapMemberiOpCode()
		{			
		}
	};

	/**
	This class is an op-code, it inc the stack ref and add the content of a sub member attribut to the curent stack. 
	The member is extrat from an arbitrary position on the heap. This op code is called when it is necessary
	to load a member without making clone.
	*/
	class CLdbRefOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbRefOpCode;
	private:
		int _B;
	public:
		CLdbRefOpCode(int b):
		_B(b)
		{			
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(std::string &str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbRefOpCode(_B);
			return x;

		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbRefOpCode;
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

		virtual ~CLdbRefOpCode()
		{			
		}
		
	};
}
#endif
