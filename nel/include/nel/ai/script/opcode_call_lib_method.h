/** \file opcode_call_lib_method.h
 * Sevral op-code fonctionality.
 *
 * $Id: opcode_call_lib_method.h,v 1.5 2001/04/17 09:26:09 portier Exp $
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

#ifndef NL_OP_CODE_CALL_LIB_METHOD_H
#define NL_OP_CODE_CALL_LIB_METHOD_H

namespace NLAISCRIPT
{
	///\name Call method in an IObjectIA.
	//@{

	///Call the method in the IObjectIA define in the self object.
	class CLibMemberMethod: public IOpRunCode
	{		
	public:
		static const NLAIC::CIdentType IdLibMemberMethod;
	private:		
		sint32 _Id;		
	public:			
		CLibMemberMethod(sint32 id):_Id(id)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode %d",_Id);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibMemberMethod(_Id);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibMemberMethod;
		}

		

		void save(NLMISC::IStream &os)
		{		
			sint32 n = (sint32) _Id;
			os.serial(n);
		}

		void load(NLMISC::IStream &is) 
		{			
			sint32 n;
			is.serial(n);
			_Id = n;
		}
		
			
		virtual ~CLibMemberMethod()
		{			
		}

	};

	///Call method in a base class in from IObjectIA define in the self object.
	class CLibMemberInheritedMethod: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLibMemberInheritedMethod;
	private:		
		sint32 _Inheritance;
		sint32 _Id;		
	public:			
		CLibMemberInheritedMethod(sint32 inheritance,sint32 id):_Inheritance(inheritance),_Id(id)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode %d member of the %d derivation",_Id,_Inheritance);
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibMemberInheritedMethod(_Inheritance,_Id);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibMemberInheritedMethod;
		}		

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _Id;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_Id = n;
			is.serial(n);
			_Inheritance = n;
		}		

		virtual ~CLibMemberInheritedMethod()
		{			
		}

	};

	/**
	This op code is define when we want to call an ambiguios method. For exemple if we want to execut the expression: Object_1.Object_2....Object_i.method();
	*/
	class CLibMemberMethodi: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLibMemberMethodi;
	private:		
		sint32 _Inheritance;
		sint32 _Id;
		std::list<sint32> _I;		
	public:			
		CLibMemberMethodi(sint32 inheritance,sint32 id,std::list<sint32> i): _Inheritance(inheritance),_Id(id),_I(i)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode %d member of the %d derivation",_Id,_Inheritance);
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibMemberMethodi(_Inheritance,_Id,_I);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibMemberMethodi;
		}		

		void save(NLMISC::IStream &os)
		{		
			sint32 n = (sint32) _Id;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_Id = n;
			is.serial(n);
			_Inheritance = n;
			is.serialCont(_I);
		}		

		virtual ~CLibMemberMethodi()
		{			
		}

	};

	///Call a method from a defined IObjectIA object.
	class CLibCallMethod: public IOpRunCode
	{		
	public:
		static const NLAIC::CIdentType IdLibCallMethod;
	private:		
		sint32 _Id;
		NLAIAGENT::IObjectIA *_Lib;
	public:			
		CLibCallMethod(sint32 id,const NLAIAGENT::IObjectIA &lib):_Id(id),_Lib((NLAIAGENT::IObjectIA*)lib.clone())
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode %d",_Id);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibCallMethod(_Id,*_Lib);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibCallMethod;
		}

		

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _Id;
			os.serial(n);
			os.serial( (NLAIC::CIdentType &) _Lib->getType() );
			_Lib->save(os);
		}

		void load(NLMISC::IStream &is) 
		{
			sint32 n;
			is.serial(n);
			_Id = n;
			_Lib->release();
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			_Lib = (NLAIAGENT::IObjectIA *)id.allocClass();
			_Lib->load(is);
		}
		
			
		virtual ~CLibCallMethod()
		{
			_Lib->release();
		}

	};	

	///Call a method from a defined IObjectIA object.
	class CLibCallInheritedMethod: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLibCallInheritedMethod;
	private:		
		sint32 _Inheritance;
		sint32 _Id;
		NLAIAGENT::IObjectIA *_Lib;
	public:			
		CLibCallInheritedMethod(sint32 inheritance,sint32 id,const NLAIAGENT::IObjectIA &lib):_Inheritance(inheritance),_Id(id),_Lib((NLAIAGENT::IObjectIA*)lib.clone())
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode %d member of the %d derivation",_Id,_Inheritance);
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibCallInheritedMethod(_Inheritance,_Id,*_Lib);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibCallInheritedMethod;
		}		

		void save(NLMISC::IStream &os)
		{		
			sint32 n = (sint32) _Id;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
			os.serial( (NLAIC::CIdentType &) _Lib->getType() );
			_Lib->save(os);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_Id = n;
			is.serial(n);
			_Inheritance = n;

			_Lib->release();
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			_Lib =  (NLAIAGENT::IObjectIA *)id.allocClass();
			_Lib->load(is);

		}		

		virtual ~CLibCallInheritedMethod()
		{
			_Lib->release();
		}

	};	
	class CLibCallMethodi: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLibCallMethodi;
	private:		
		sint32 _Inheritance;
		sint32 _Id;
		std::list<sint32> _I;
		NLAIAGENT::IObjectIA *_Lib;
	public:			
		CLibCallMethodi(sint32 inheritance,sint32 id,std::list<sint32> i,const NLAIAGENT::IObjectIA &lib):
						_Inheritance(inheritance),_Id(id),_I(i),_Lib((NLAIAGENT::IObjectIA*)lib.clone())
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode %d member of the %d derivation",_Id,_Inheritance);
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibCallMethodi(_Inheritance,_Id,_I,*_Lib);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibCallMethodi;
		}		

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _Id;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
			os.serial( (NLAIC::CIdentType &) _Lib->getType() );
			_Lib->save(os);
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_Id = n;
			is.serial(n);
			_Inheritance = n;

			_Lib->release();
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			_Lib =  (NLAIAGENT::IObjectIA *)id.allocClass();
			_Lib->load(is);
			is.serialCont ( _I );
		}		

		virtual ~CLibCallMethodi()
		{
			_Lib->release();
		}

	};

	class CLibStackMemberMethod: public IOpRunCode
	{		
	public:
		static const NLAIC::CIdentType IdLibStackMemberMethod;
	protected:		
		sint32 _Id;
		sint32 _H;
		std::list<sint32> _I;
	public:
		CLibStackMemberMethod(const CLibStackMemberMethod &a):_Id(a._Id),_H(a._H),_I(a._I)
		{
		}
		CLibStackMemberMethod(sint32 h,sint32 id,std::list<sint32> i):_Id(id),_H(h),_I(i)
		{			
		}

		virtual NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode frome the stack");	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibStackMemberMethod(_Id,_H,_I);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibStackMemberMethod;
		}

		

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _Id;
			os.serial(n);
			n = (sint32) _H;
			os.serial(n);			
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{
			sint32 n;
			is.serial(n);
			_Id = n;
			is.serial(n);
			_H = n;						
			is.serialCont ( _I );
		}
		
			
		virtual ~CLibStackMemberMethod()
		{			
		}

	};	

	class CLibStackNewMemberMethod: public CLibStackMemberMethod
	{		
	public:
		static const NLAIC::CIdentType IdLibStackNewMemberMethod;
	public:
		CLibStackNewMemberMethod(const CLibStackNewMemberMethod &a):CLibStackMemberMethod(a)
		{

		}

		CLibStackNewMemberMethod(sint32 h,sint32 id,std::list<sint32> i):CLibStackMemberMethod(h,id,i)
		{
			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibStackNewMemberMethod(*this);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLibStackNewMemberMethod;
		}

		virtual ~CLibStackNewMemberMethod()
		{			
		}
	};


	class CLibHeapMemberMethod: public CLibStackMemberMethod
	{		
	public:
		static const NLAIC::CIdentType IdLibHeapMemberMethod;
	private:
		sint32 _Index;
	public:	
		CLibHeapMemberMethod(const CLibHeapMemberMethod &a):CLibStackMemberMethod(a),_Index(a._Index)
		{
		}

		CLibHeapMemberMethod(sint32 h,sint32 id,std::list<sint32> i, sint32 index):CLibStackMemberMethod(h,id,i) , _Index(index)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"call the methode frome the heap");	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibHeapMemberMethod(*this);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdLibHeapMemberMethod;
		}

		

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _Index;
			os.serial(n);
			CLibStackMemberMethod::save(os);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_Index = n;
			CLibStackMemberMethod::load(is);
		}
		
			
		virtual ~CLibHeapMemberMethod()
		{			
		}

	};
	//@}
}
#endif
