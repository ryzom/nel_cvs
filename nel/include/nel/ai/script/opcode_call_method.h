/** \file opcode_call_method.h
 * Sevral call op-code fonctionality.
 *
 * $Id: opcode_call_method.h,v 1.8 2003/01/31 14:59:43 chafik Exp $
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
#ifndef NL_OP_CODE_CALL_METHOD_H
#define NL_OP_CODE_CALL_METHOD_H

namespace NLAISCRIPT
{
	/**
	Abstract class that allow to manage context on method call.
	*/
	class IMethodContext
	{
	public:
		/**
		Allow to save the register, ip pointer .... before method call.
		*/
		virtual void saveContext(CCodeContext &context) = 0;

		/**
		Allow to reload old context before the return of method.
		*/
		virtual void loadContext(CCodeContext &context) = 0;

		/**
		Server method need this, in particular on the copy constructor where it is easy to clone param to init attribut.
		*/
		virtual const IMethodContext *clone() const = 0;
	};


	class CMethodContext: public IMethodContext
	{
	public:
		virtual void saveContext(CCodeContext &context);
		virtual void loadContext(CCodeContext &context);

		const IMethodContext *clone() const;
		
	};

	class CMethodContextDebug: public IMethodContext
	{
	public:
		virtual void saveContext(CCodeContext &context);
		virtual void loadContext(CCodeContext &context);

		const IMethodContext *clone() const;
		
	};

	/**
	That class allow to run script method class via the NLAIAGENT::TProcessStatement IOpCode::runOpCode(CCodeContext &context).
	*/
	class ICallMethod: public IOpRunCode
	{
	private:
		IMethodContext *_MethodContext;

	public:
		ICallMethod(const ICallMethod &c):_MethodContext((IMethodContext *)c._MethodContext->clone())
		{
		}

		ICallMethod(IMethodContext *c):_MethodContext(c)
		{
		}

		/**
		Saving context before method call.
		*/
		void saveContext(CCodeContext &context)
		{
			_MethodContext->saveContext(context);
		}

		/**
		reload context before the return of the method.
		*/
		void loadContext(CCodeContext &context)
		{
			_MethodContext->loadContext(context);
		}

		/**
		Delete context.
		*/
		virtual ~ICallMethod()
		{
			delete _MethodContext;
		}
	};

	/**
	*/
	class CCallMethod: public ICallMethod
	{
	public:
		static const NLAIC::CIdentType IdCallMethode;
		
	private:
		sint32 _Inheritance;
		sint32 _I;

	public:

		CCallMethod(const CCallMethod &c):ICallMethod(c),_Inheritance(c._Inheritance),_I(c._I)
		{			
		}

		CCallMethod(IMethodContext *c,sint32 inheritance,sint32 i):ICallMethod(c),_Inheritance(inheritance),_I(i)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(std::string &str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CCallMethod(*this);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdCallMethode;
		}

		

		void save(NLMISC::IStream &os)
		{		
			sint32 n = (sint32) _I;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_I = n;
			is.serial(n);
			_Inheritance = n;
		}
				

		virtual ~CCallMethod()
		{
		}

	};	
	
	class CCallHeapMethodi: public ICallMethod
	{
	public:
		static const NLAIC::CIdentType IdCallHeapMethodei;
		
	private:
		sint32 _Inheritance;
		sint32 _I;
		sint32 _HeapPos;
		std::list<sint32> _N;

	public:			
		CCallHeapMethodi(const CCallHeapMethodi &c):ICallMethod(c),_Inheritance(c._Inheritance),_I(c._I),_HeapPos(c._HeapPos),_N(c._N)
		{
		}
		CCallHeapMethodi(IMethodContext *c,sint32 inheritance,sint32 i,sint32 heapPos,const std::list<sint32> &l):
								ICallMethod(c),_Inheritance(inheritance),_I(i),_HeapPos(heapPos),_N(l)
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		void getDebugResult(std::string &str,CCodeContext &context) const;
				
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CCallHeapMethodi(*this);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdCallHeapMethodei;
		}		
		
		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _I;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
			n = (sint32) _HeapPos;
			os.serial(n);
			std::list<sint32> &i = (std::list<sint32> &)_N;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_I = n;
			is.serial(n);
			_Inheritance = n;
			is.serial(n);
			_HeapPos = n;			
			is.serialCont(_N);
		}
				
		virtual ~CCallHeapMethodi()
		{
		}

	};
	

	class CCallStackMethodi: public ICallMethod
	{
	public:
		static const NLAIC::CIdentType IdCallStackMethodei;

	protected:
		sint32 _Inheritance;
		sint32 _I;
		std::list<sint32> _N;
		
	public:			
		CCallStackMethodi(const CCallStackMethodi &c):ICallMethod(c),_Inheritance(c._Inheritance),_I(c._I),_N(c._N)
		{
		}
		CCallStackMethodi(IMethodContext *c,sint32 inheritance,sint32 i,const std::list<sint32> &l):ICallMethod(c),_Inheritance(inheritance),_I(i),_N(l)
		{			
		}

		virtual  NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(std::string &str,CCodeContext &context) const;		

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CCallStackMethodi(*this);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdCallStackMethodei;
		}
		

		void save(NLMISC::IStream &os)
		{		
			sint32 n = (sint32) _I;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
			std::list<sint32> &i = (std::list<sint32> &)_N;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_I = n;
			is.serial(n);
			_Inheritance = n;
			is.serialCont(_N);
		}
				
		virtual ~CCallStackMethodi()
		{
		}

	};

	class CCallStackNewMethodi: public CCallStackMethodi
	{
	public:
		static const NLAIC::CIdentType IdCallStackNewMethodei;
	
	public:
		CCallStackNewMethodi(const CCallStackNewMethodi &c):CCallStackMethodi(c)
		{
			
		}
		CCallStackNewMethodi(IMethodContext *c,sint32 inheritance,sint32 i,const std::list<sint32> &l):CCallStackMethodi(c,inheritance,i,l)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(std::string &str,CCodeContext &context) const
		{		
			NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)context.Stack[(int)context.Stack];
			std::list<sint32>::const_iterator it = _N.begin();

			while(it != _N.end())
			{
				obj = (NLAIAGENT::IObjectIA *)obj->getStaticMember(*it++);
			}

			str = NLAIC::stringGetBuild("CallMethod %d de la class '%s' dans le stack apres un new",_I, (const char *)obj->getType());
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CCallStackNewMethodi(*this);
			return x;
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdCallStackNewMethodei;
		}
				
		virtual ~CCallStackNewMethodi()
		{
		}

	};
	

	class CCallMethodi: public ICallMethod
	{
	public:
		static const NLAIC::CIdentType IdCallMethodei;
		
	private:
		sint32 _Inheritance;
		sint32 _I;
		std::list<sint32> _N;
		
	public:
		CCallMethodi(const CCallMethodi &c):ICallMethod(c),_Inheritance(c._Inheritance),_I(c._I),_N(c._N)
		{

		}
		
		CCallMethodi(IMethodContext *c,sint32 inheritance,sint32 i,const std::list<sint32> &l):ICallMethod(c),_Inheritance(inheritance),_I(i),_N(l)
		{			
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(std::string &str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CCallMethodi(*this);
			return x;
		}
		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdCallMethodei;
		}
		
		void save(NLMISC::IStream &os)
		{		
			sint32 n = (sint32) _I;
			os.serial(n);
			n = (sint32) _Inheritance;
			os.serial(n);
			std::list<sint32> &i = (std::list<sint32> &)_N;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_I = n;
			is.serial(n);
			_Inheritance = n;
			is.serialCont(_N);
		}

		virtual ~CCallMethodi()
		{
		}

	};		
}
#endif
