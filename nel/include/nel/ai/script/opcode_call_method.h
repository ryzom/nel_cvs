/** \file opcode_call_method.h
 * Sevral call op-code fonctionality.
 *
 * $Id: opcode_call_method.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

namespace NLIASCRIPT
{
	class IMethodContext
	{
	public:
		virtual void saveConstext(CCodeContext &context) = 0;
		virtual void loadConstext(CCodeContext &context) = 0;

		virtual const IMethodContext *clone() const = 0;
	};


	class CMethodContext: public IMethodContext
	{
	public:
		virtual void saveConstext(CCodeContext &context);
		virtual void loadConstext(CCodeContext &context);

		const IMethodContext *clone() const;
		
	};

	class CMethodContextDebug: public IMethodContext
	{
	public:
		virtual void saveConstext(CCodeContext &context);
		virtual void loadConstext(CCodeContext &context);

		const IMethodContext *clone() const;
		
	};

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

		void saveConstext(CCodeContext &context)
		{
			_MethodContext->saveConstext(context);
		}

		void loadConstext(CCodeContext &context)
		{
			_MethodContext->loadConstext(context);
		}

		virtual ~ICallMethod()
		{
			delete _MethodContext;
		}
	};

	class CCallMethod: public ICallMethod
	{
	public:
		static const NLIAC::CIdentType IdCallMethode;
		
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

		NLIAAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"CallMethod %d de la class '%s'",_I, (const char *)(context.Self)->getType());
		}

		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CCallMethod(*this);
			x->incRef();         
			return x;
		}
		const NLIAC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLIAC::CIdentType &getType() const
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
				

		~CCallMethod()
		{
		}

	};	
	
	class CCallHeapMethodi: public ICallMethod
	{
	public:
		static const NLIAC::CIdentType IdCallHeapMethodei;
		
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

		NLIAAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Heap[(int)_HeapPos];
			std::list<sint32>::const_iterator it = _N.begin();			
			while(it != _N.end())
			{
				obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
			}
			sprintf(str,"CallMethod %d de la class '%s' dans le Heap",_I, (const char *)obj->getType());			
		}

		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CCallHeapMethodi(*this);
			x->incRef();
			return x;
		}
		const NLIAC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLIAC::CIdentType &getType() const
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
				
		~CCallHeapMethodi()
		{
		}

	};
	

	class CCallStackMethodi: public ICallMethod
	{
	public:
		static const NLIAC::CIdentType IdCallStackMethodei;

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

		virtual  NLIAAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Stack[(int)context.Stack - 1];
			std::list<sint32>::const_iterator it = _N.begin();			
			while(it != _N.end())
			{
				obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
			}
			sprintf(str,"CallMethod %d de la class '%s' dans le stack",_I, (const char *)obj->getType());
		}

		virtual const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CCallStackMethodi(*this);
			x->incRef();         
			return x;
		}
		const NLIAC::IBasicType *newInstance() const
		{
			return clone();
		}
		virtual const NLIAC::CIdentType &getType() const
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
				
		~CCallStackMethodi()
		{
		}

	};

	class CCallStackNewMethodi: public CCallStackMethodi
	{
	public:
		static const NLIAC::CIdentType IdCallStackNewMethodei;
	
	public:
		CCallStackNewMethodi(const CCallStackNewMethodi &c):CCallStackMethodi(c)
		{
			
		}
		CCallStackNewMethodi(IMethodContext *c,sint32 inheritance,sint32 i,const std::list<sint32> &l):CCallStackMethodi(c,inheritance,i,l)
		{			
		}

		NLIAAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(char *str,CCodeContext &context) const
		{		
			NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Stack[(int)context.Stack];
			std::list<sint32>::const_iterator it = _N.begin();

			while(it != _N.end())
			{
				obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
			}

			sprintf(str,"CallMethod %d de la class '%s' dans le stack apres un new",_I, (const char *)obj->getType());
		}

		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CCallStackNewMethodi(*this);
			x->incRef();         
			return x;
		}

		const NLIAC::CIdentType &getType() const
		{
			return IdCallStackNewMethodei;
		}
				
		~CCallStackNewMethodi()
		{
		}

	};
	

	class CCallMethodi: public ICallMethod
	{
	public:
		static const NLIAC::CIdentType IdCallMethodei;
		
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

		NLIAAGENT::TProcessStatement runOpCode(CCodeContext &context);		
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			NLIAAGENT::IObjectIA *obj = (NLIAAGENT::IObjectIA *)context.Self;
			std::list<sint32>::const_iterator it = _N.begin();

			while(it != _N.end())
			{
				obj = (NLIAAGENT::IObjectIA *)obj->getStaticMember(*it++);
			}

			sprintf(str,"CallMethodi %d de la class '%s'",_I, (const char *)obj->getType());
		}

		const NLIAC::IBasicType *clone() const
		{
			NLIAC::IBasicType *x = new CCallMethodi(*this);
			x->incRef();         
			return x;
		}
		const NLIAC::IBasicType *newInstance() const
		{
			return clone();
		}
		const NLIAC::CIdentType &getType() const
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
				
		~CCallMethodi()
		{
		}

	};		
}
#endif
