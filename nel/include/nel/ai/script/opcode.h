/** \file opcode.h
 * Sevral op-code fonctionality.
 *
 * $Id: opcode.h,v 1.4 2001/01/08 11:16:29 chafik Exp $
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

#ifndef NL_OP_CODE_H
#define NL_OP_CODE_H

#include <math.h>
#include "logic/varset.h"
#include "script/opcode_ldb.h"
#include "script/opcode_lpt.h"
#include "script/interpret_methodes.h"
#include "script/constraint.h"

namespace NLAISCRIPT
{
	/**
	* Class CLdbNewOpCode.
	* 
	* Load a new object in the stack.
	*
	* \author Portier Pierre
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CLdbNewOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLdbNewOpCode;
	private:
		NLAIAGENT::IObjectIA *_B;
	public:
		CLdbNewOpCode(const NLAIAGENT::IObjectIA &b): _B( (NLAIAGENT::IObjectIA *) b.clone() )
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{
			NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *)_B->clone();
			NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)context.Stack[(int)context.Stack];
			obj->init(param);
			param->release();
			//context.Stack ++;
			context.Stack[(int)context.Stack] = obj;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8];
			_B->getDebugString(X);
			sprintf(str,"ldb %s",X);		
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLdbNewOpCode(*_B);
			x->incRef();
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLdbNewOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{
			os.serial( (NLAIC::CIdentType &) (_B->getType()) );
			_B->save(os);		
		}

		void load(NLMISC::IStream &is) 
		{
			_B->release();
			NLAIC::CIdentTypeAlloc id;
			is >> id;
			_B = (NLAIAGENT::IObjectIA *)id.allocClass();
			_B->load(is);
		}
		const NLAIAGENT::IObjectIA::CProcessResult &run(){return NLAIAGENT::IObjectIA::ProcessRun;;}
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const{ return true;}	
		
		~CLdbNewOpCode()
		{
			_B->release();
		}
	};

	///\name Operator op code
	/** 
	Objects must correctly define the good CTypeOfOperator by binnary combination.
	For exemple if you have to do A = C + B; then type of C an B object must declare the CTypeOfOperator::opAdd define in 
	the CTypeOfOperator::TTypeOp when its was registred in the class factory.
	*/
	//@{

	/**
	* Class CNegOpCode.
	* 
	* It is the same thing that *stack = -(*stack).
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	

	class CNegOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdNegOpCode;
	private:
		
	public:
				
		CNegOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack])->neg();
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}	

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8];
			((NLAIAGENT::IObjectIA *)context.Stack)->getDebugString(X);
			sprintf(str,"-%s",X);		
		}
		const NLAIC::IBasicType *clone() const 
		{			
			NLAIC::IBasicType *x = new CNegOpCode();
			x->incRef();
			return x;
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdNegOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}
				
		void load(NLMISC::IStream &is) 
		{		
		}
		
		~CNegOpCode()
		{
		}
	};
	
	/**
	* Class CAddOpCode.
	* 
	* It is the same thing that *stack[i - 1] += *stack[i].
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CAddOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdAddOpCode;
	private:
		
	public:
		
		CAddOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			*((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) += *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const 
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s + %s",X,Y);		
		}	

		const NLAIC::IBasicType *clone() const 
		{			
			NLAIC::IBasicType *x = new CAddOpCode();
			x->incRef();
			return x;
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdAddOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}				

		void load(NLMISC::IStream &is) 
		{		
		}
		
		~CAddOpCode()
		{
		}
	};

	/**
	* Class CSubOpCode.
	* 
	* It is the same thing that *stack[i - 1] -= *stack[i].
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CSubOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdSubOpCode;
	private:
		
	public:

		CSubOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			*((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) -= *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}
		
		void getDebugResult(char *str,CCodeContext &context) const 
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s - %s",X,Y);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CSubOpCode();
			x->incRef();
			return x;
		}
		
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdSubOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}				

		~CSubOpCode()
		{
		}
	};

	/**
	* Class CDivOpCode.
	* 
	* It is the same thing that *stack[i - 1] /= *stack[i].
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CDivOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdDivOpCode;
	private:
		
	public:

		CDivOpCode()
		{		
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			*((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) /= *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s / %s",X,Y);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CDivOpCode();
			x->incRef();
			return x;
		}
		
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdDivOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}
				
		~CDivOpCode()
		{
		}
	};

	/**
	* Class CMulOpCode.
	* 
	* It is the same thing that *stack[i - 1] *= *stack[i].
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CMulOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdMulOpCode;
		
	private:
		
	public:

		CMulOpCode()
		{
		}	
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			*((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) *= *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s * %s",X,Y);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CMulOpCode();
			x->incRef();
			return x;
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdMulOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}				
		
		~CMulOpCode()
		{
		}
	};


	/**
	* Class CSupOpCode.
	* 
	* It is the same thing that (*stack[i - 1] > *stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CSupOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdSupOpCode;
	private:
		
	public:	

		CSupOpCode()
		{		
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			NLAIAGENT::IObjetOp *op = *((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) > *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack - 1] != NULL) context.Stack[(int)context.Stack - 1]->release();
			context.Stack[(int)context.Stack - 1] = op;
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s > %s",X,Y);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CSupOpCode();
			x->incRef();
			return x;
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdSupOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}				
		
		~CSupOpCode()
		{
		}
	};

	/**
	* Class CInfOpCode.
	* 
	* It is the same thing that (*stack[i - 1] < *stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CInfOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdInfOpCode;

	private:
		
	public:	
		
		CInfOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			NLAIAGENT::IObjetOp *op = *((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) < *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack - 1] != NULL) context.Stack[(int)context.Stack - 1]->release();
			context.Stack[(int)context.Stack - 1] = op;
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s < %s",X,Y);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CInfOpCode();
			x->incRef();
			return x;
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdInfOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}				

		~CInfOpCode()
		{
		}
	};

	/**
	* Class CEqOpCode.
	* 
	* It is the same thing that (*stack[i - 1] == *stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CEqOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdEqOpCode;
	private:
		
	public:	

		CEqOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			NLAIAGENT::IObjetOp *op = *((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) == *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack - 1] != NULL) context.Stack[(int)context.Stack - 1]->release();
			context.Stack[(int)context.Stack - 1] = op;
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s = %s",X,Y);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CEqOpCode();
			x->incRef();         
			return x;            
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdEqOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}
				
		~CEqOpCode()
		{
		}
	};

	/**
	* Class CSupEqOpCode.
	* 
	* It is the same thing that (*stack[i - 1] >= *stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CSupEqOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdSupEqOpCode;
	private:
		
	public:	

		CSupEqOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			NLAIAGENT::IObjetOp *op = *((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) >= *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack - 1] != NULL) context.Stack[(int)context.Stack - 1]->release();
			context.Stack[(int)context.Stack - 1] = op;
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}
		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s >= %s",X,Y);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CSupEqOpCode();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdSupEqOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}
				
		
		~CSupEqOpCode()
		{
		}
	};

	/**
	* Class CInfEqOpCode.
	* 
	* It is the same thing that (*stack[i - 1] <= *stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CInfEqOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdInfEqOpCode;
	private:
		
	public:	
		CInfEqOpCode()
		{		
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			NLAIAGENT::IObjetOp *op = *((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) <= *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack - 1] != NULL) context.Stack[(int)context.Stack - 1]->release();
			context.Stack[(int)context.Stack - 1] = op;
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s <= %s",X,Y);	
		}
		
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CInfEqOpCode();
			x->incRef();         
			return x;            
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdInfEqOpCode;
		}
		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}
				

		~CInfEqOpCode()
		{
		}
	};

	/**
	* Class CDiffOpCode.
	* 
	* It is the same thing that (*stack[i - 1] != *stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CDiffOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdDiffOpCode;
	private:
		
	public:	
		CDiffOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			NLAIAGENT::IObjetOp *op = *((NLAIAGENT::IObjetOp *)context.Stack[(int)context.Stack - 1]) != *((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack - 1] != NULL) context.Stack[(int)context.Stack - 1]->release();
			context.Stack[(int)context.Stack - 1] = op;
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char X[1024*8],Y[1024*8];
			context.Stack[(int)context.Stack - 1]->getDebugString(X);
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"%s != %s",X,Y);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CDiffOpCode();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdDiffOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}				
		
		~CDiffOpCode()
		{
		}
	};

	/**
	* Class CNotOpCode.
	* 
	* It is the same thing that !(*stack[i]).
	* Where i is the current stack position.
	*	
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CNotOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdNotOpCode;
		
	private:
		
	public:	
		CNotOpCode()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			
			NLAIAGENT::IObjetOp *op = !*((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(context.Stack[(int)context.Stack] != NULL) context.Stack[(int)context.Stack]->release();
			context.Stack[(int)context.Stack] = op;
			//context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}
		void getDebugResult(char *str,CCodeContext &context) const
		{
			char Y[1024*8];
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"!%s",Y);
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CNotOpCode();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdNotOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{		
		}
				
		
		~CNotOpCode()
		{
		}
	};

	//@}

	/**
	* Class CJFalseOpCode.
	* 
	* This class define a jump if the !(*stack[i]) is true.
	* Where i is the current stack position.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CJFalseOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdJFalseOpCode;
	private:
		int _N;	
	public:	
		CJFalseOpCode(int l):
		_N(l)
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			NLAIAGENT::IObjetOp *op = !*((NLAIAGENT::IObjetOp *)((NLAIAGENT::IObjectIA *)context.Stack));
			if(op->isTrue()) *context.Code += _N;
			op->release();
			context.Stack--;
			return NLAIAGENT::IObjectIA::ProcessIdle;
		}

		void getDebugResult(char *str,CCodeContext &context) const 
		{
			char X[1024*8];		
			context.Stack[(int)context.Stack]->getDebugString(X);
			sprintf(str,"if !(%s) is false then goto %d",X,_N);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CJFalseOpCode(_N);
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdJFalseOpCode;
		}		

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _N;
			os.serial( n );
		}

		void load(NLMISC::IStream &is) 
		{		
			sint32 n;
			is.serial( n );
			_N = (int) n;
		}
		
		~CJFalseOpCode()
		{
		}
	};

	/**
	* Class CJmpOpCode.
	* 
	* This class define a jump.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CJmpOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdJmpOpCode;
	private:
		int _N;	
	public:	
		CJmpOpCode(int l):
		_N(l)
		{
		}
		
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			*context.Code += _N;	
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"goto %d",_N);	
		}
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CJmpOpCode(_N);
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdJmpOpCode;
		}

		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _N;
			os.serial( n );
		}

		void load(NLMISC::IStream &is) 
		{		
			sint32 n;
			is.serial( n );
			_N = (int) n;
		}

		~CJmpOpCode()
		{
		}
	};


	/// \name Affectation 
	//@{ 

	/**
	* Class CAffOpCode.
	* 
	* This class define: *heap[h] = *stack[s].
	* Where s is the current stack position.
	* Where h define a heap position.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CAffOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdAffOpCode;
		
	private:
		///Heap position.
		int _I;
	public:	
		CAffOpCode(int i):
		  _I(i)
		{
			
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			NLAIAGENT::IObjectIA *i = (NLAIAGENT::IObjectIA *)context.Stack;
			i->incRef();
			NLAIAGENT::IObjectIA *tmp = context.Heap[(int)_I];
			context.Heap[_I] = i;
			if(tmp) tmp->release();			
			context.Stack --;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char Y[1024*8];					
			context.Stack[(int)context.Stack]->getDebugString(Y);
			sprintf(str,"Affectation<%d> a %s",_I,Y);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CAffOpCode(_I);
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdAffOpCode;
		}

		void getDebugString(char *) const{ }

		void save(NLMISC::IStream &os)
		{
			sint32 i = (sint32) _I;
			os.serial( i );
		}

		void load(NLMISC::IStream &is) 
		{		
			sint32 i;
			is.serial( i );
			_I = (int) i;
		}
				

		~CAffOpCode()
		{
		}
	};

	/// Same as AddOpCode, but help to keep a trace of the varname for debug.
	class CAffOpCodeDebug : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdAffOpCodeDebug;
		
	private:
		int _I;
		char * _VarName;

	public:	
		CAffOpCodeDebug(int i, const char* varName);
		
		~CAffOpCodeDebug();

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(char *str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const;

		const NLAIC::IBasicType *newInstance() const;

		const NLAIC::CIdentType &getType() const;

		void getDebugString(char *) const;

		void save(NLMISC::IStream &os);

		void load(NLMISC::IStream &is);
	};

	/**
	* Class CAffHeapMemberiOpCode.
	* 
	* This class define an expression like: *heap[h].member_1.member_2...member_n = *stack[s].
	* Where s is the current stack position.
	* Where h define a heap position.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CAffHeapMemberiOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdAffHeapMemberiOpCode;
	private:
		std::list<sint32> _I;
		sint32 _N;
	public:	
		CAffHeapMemberiOpCode(std::list<sint32> i,sint32 n):
		  _I(i),_N(n)
		{
			
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(char *str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CAffHeapMemberiOpCode(_I,_N);
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdAffHeapMemberiOpCode;
		}


		void save(NLMISC::IStream &os)
		{
			sint32 n = (sint32) _N;
			os.serial( n );
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{				
			sint32 n;
			is.serial(n);
			_N = n;
			is.serialCont(_I);
		}

		~CAffHeapMemberiOpCode()
		{
		}
	};

	/**
	* Class CAffMemberiOpCode.
	* 
	* This class define an expression like: *self.member_1.member_2...member_i = *stack[s].
	* Where s is the current stack position.	
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CAffMemberiOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdAffMemberiOpCode;
	private:
		std::list<sint32> _I;
	public:	
		CAffMemberiOpCode(std::list<sint32> i):
		  _I(i)
		{
			
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(char *str,CCodeContext &context) const
		{
			
			sprintf(str,"Affectation du membre");	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CAffMemberiOpCode(_I);
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdAffMemberiOpCode;
		}


		void save(NLMISC::IStream &os)
		{
			std::list<sint32> &i = (std::list<sint32> &)_I;
			os.serialCont(i);
		}

		void load(NLMISC::IStream &is) 
		{	
			is.serialCont( _I );
		}
				

		~CAffMemberiOpCode()
		{
		}
	};

	/**
	* Class CAffMemberOpCode.
	* 
	* This class define an expression like: *self.member = *stack[s].
	* Where s is the current stack position.	
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class CAffMemberOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdAffMemberOpCode;
		
	private:
		int _I;
	public:	
		CAffMemberOpCode(int i):
		  _I(i)
		{
			
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(char *str,CCodeContext &context) const
		{
			
			sprintf(str,"Affectation du membre %d",_I);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CAffMemberOpCode(_I);
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdAffMemberOpCode;
		}


		void save(NLMISC::IStream &os)
		{
			sint32 i = (sint32) _I;
			os.serial( i );
		}

		void load(NLMISC::IStream &is) 
		{		
			sint32 i;
			is.serial( i );
			_I = (int) i;
		}
				

		~CAffMemberOpCode()
		{
		}
	};

	//@}

	///Inc by one the heap pointer. The result is an new allocation in the heap.
	class CLocAlloc: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLocAlloc;
	public:	
	
		CLocAlloc()
		{
			
		}		
	
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			/*NLAIAGENT::IObjectIA *i = (NLAIAGENT::IObjectIA *)NLAIC::getRegistry()->createInstance(_B);
			context.Heap[(int)context.Heap] = NULL;*/
			context.Heap ++;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{					
			sprintf(str,"Reservation d'un espace memoir locale pour une variable Ó l'emplacement %d",(int)context.Heap);				
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLocAlloc();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdLocAlloc;
		}		

		void save(NLMISC::IStream &os)
		{			
		}

		void load(NLMISC::IStream &is) 
		{				
		}
				
		
		~CLocAlloc()
		{			
		}
	};

	/// Same as LocAlloc, but help to keep a trace of the varname for debug.
	class CLocAllocDebug: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdLocAllocDebug;

	public:	
	
		CLocAllocDebug();
		
		~CLocAllocDebug();

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		void getDebugResult(char *str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const;

		const NLAIC::IBasicType *newInstance() const ;

		const NLAIC::CIdentType &getType() const;

		void save(NLMISC::IStream &os);

		void load(NLMISC::IStream &is);
		
	};

	///Make a mark on the last heap allocation
	class CMarkAlloc: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdMarkAlloc;
	public:
		CMarkAlloc()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			context.Heap.pushMark();	
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"Marque le bloc de memoire Ó %d",(int)context.Heap);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CMarkAlloc();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdMarkAlloc;
		}


		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{				
		}				

		~CMarkAlloc()
		{
		}
	};

	///Free the heap between the last mark made an the curent position oif the heap pointer.
	class CFreeAlloc: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdFreeAlloc;
	public:

		CFreeAlloc()
		{
		}
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{				
			int i = (int)context.Heap;	
			i -= context.Heap.popMark();
			context.Heap -= i;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"libere le bloc de memoire de %d Ó %d",(int)context.Heap.mark(),(int)context.Heap);	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CFreeAlloc();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdFreeAlloc;
		}

		
		
		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{				
		}				
		
		~CFreeAlloc()
		{
		}
	};

	/// Same as FreeAlloc, but help to keep a trace of the varname for debug.
	class CFreeAllocDebug: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdFreeAllocDebug;
	public:

		CFreeAllocDebug();
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		~CFreeAllocDebug();

		void getDebugResult(char *str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const;

		const NLAIC::IBasicType *newInstance() const;

		const NLAIC::CIdentType &getType() const;
		
		void save(NLMISC::IStream &os);

		void load(NLMISC::IStream &is);
	};

	///Exit code.
	class CHaltOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdHaltOpCode;
	public:	
		CHaltOpCode()
		{
		}		
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			return NLAIAGENT::IObjectIA::ProcessEnd;
		}
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"Stop");	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CHaltOpCode();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdHaltOpCode;
		}

		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{				
		}
				

		~CHaltOpCode()
		{
		}
	};		


	///Do not do anything.
	class CNopOpCode: public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdNopOpCode;
	public:	
		CNopOpCode()
		{
		}		
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{		
			return NLAIAGENT::IObjectIA::ProcessIdle;
		}
		
		void getDebugResult(char *str,CCodeContext &context) const
		{		
			sprintf(str,"No operation");	
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CNopOpCode();
			x->incRef();         
			return x;            
		}
		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}
		const NLAIC::CIdentType &getType() const
		{
			return IdNopOpCode;
		}

		

		void save(NLMISC::IStream &os)
		{		
		}

		void load(NLMISC::IStream &is) 
		{				
		}
				

		~CNopOpCode()
		{
		}
	};		

	class CTellOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdTellOpCode;
	public:
		CTellOpCode()
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{
			char f[1024*8];
			context.Stack[(int)context.Stack]->getDebugString(f);
			( (NLAILOGIC::CFactPattern  *) context.Stack[(int)context.Stack] )->propagate();
			return NLAIAGENT::IObjectIA::ProcessIdle;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char f[1024*8];
			context.Stack[(int)context.Stack]->getDebugString(f);
			sprintf(str,"tell %s",f);
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CTellOpCode();
			x->incRef();
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdTellOpCode;
		}

		void getDebugString(char *str) const
		{
			sprintf(str,"tell");
		}

		void save(NLMISC::IStream &os)
		{			
		}

		void load(NLMISC::IStream &is) 
		{
		}				
		
		~CTellOpCode()
		{
		}
	};

	/// Pops the object on top of the stack, puts it in a list, and pushes the list on the stack.
	class CMakeArgOpCode : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdMakeArgOpCode;
	public:

		CMakeArgOpCode()
		{
		}

		~CMakeArgOpCode()
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context)
		{
			NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
			NLAIAGENT::IBaseGroupType *arglist = new NLAIAGENT::CGroupType;
			arglist->incRef();
			arglist->cpy(*param);
			param->release();
			context.Stack[(int)context.Stack] = arglist;
			return NLAIAGENT::IObjectIA::ProcessIdle;;
		}

		void getDebugResult(char *str,CCodeContext &context) const
		{
			char buf[1024*8];
			NLAIAGENT::IObjectIA *param = context.Stack[(int)context.Stack];
			param->getDebugString(buf);
			sprintf(str,"ldb %s -> [%s]", buf, buf);		
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *clone = new CMakeArgOpCode();
			clone->incRef();
			return clone;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdMakeArgOpCode;
		}

		void getDebugString(char *txt) const
		{
			strcpy(txt, "CMakeArgOpCode");
		}

		void save(NLMISC::IStream &os)
		{
		}

		void load(NLMISC::IStream &is) 
		{
		}

		const NLAIAGENT::IObjectIA::CProcessResult &run()
		{
			return NLAIAGENT::IObjectIA::ProcessRun;
		}

		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const
		{ 
			return true;
		}	
		
	};

	class CMarkMsg : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdMarkMsg;
		
	private:
		int _I;
		int _Heritance;

	public:

		CMarkMsg(int h,int i):_Heritance(h),_I(i)
		{			
		}

		~CMarkMsg()
		{
		}

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);		

		void getDebugResult(char *str,CCodeContext &context) const
		{						
			sprintf(str,"MarkMsg for method %d in heritance %d", _I, _Heritance);					
		}

		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *clone = new CMarkMsg(_Heritance,_I);
			clone->incRef();
			return clone;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdMarkMsg;
		}

		void getDebugString(char *txt) const
		{
			strcpy(txt, "CMakeArgOpCode");
		}

		void save(NLMISC::IStream &os)
		{
			sint32 i = (sint32) _I;
			os.serial( i );
			i = (sint32) _Heritance;
			os.serial( i );
		}

		void load(NLMISC::IStream &is) 
		{
			sint32 i;
			is.serial( i );
			_I = i;
			is.serial( i );
			_Heritance = (int) i;
		}		

		const NLAIAGENT::IObjectIA::CProcessResult &run()
		{
			return NLAIAGENT::IObjectIA::ProcessRun;
		}

		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const
		{ 
			return true;
		}	
		
	};

	class ILoadObject;
	
	class CFindRunMsg : public IOpRunCode
	{
	public:
		static const NLAIC::CIdentType IdFindRunMsg;
		
	private:
		CParam *_Param;
		NLAIAGENT::IBaseGroupType *_MethodName;
		IOpType *_BaseClass;
		ILoadObject *_ObjectLoad;

	public:

		CFindRunMsg(NLAIAGENT::IBaseGroupType *methodName,CParam *param,IOpType *baseClass,ILoadObject *objectLoad);

		CFindRunMsg(const NLAIAGENT::IBaseGroupType &methodName,const CParam &param,const IOpType &baseClass,const ILoadObject &objectLoad);

		~CFindRunMsg();
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);
		void getDebugResult(char *str,CCodeContext &context) const;		

		const NLAIC::IBasicType *clone() const;		

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdFindRunMsg;
		}

		void getDebugString(char *txt) const
		{			
		}

		void save(NLMISC::IStream &os);

		void load(NLMISC::IStream &is);		

		const NLAIAGENT::IObjectIA::CProcessResult &run()
		{
			return NLAIAGENT::IObjectIA::ProcessRun;
		}

		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const
		{ 
			return true;
		}	
		
	};

	/// Build a name liste that match the actual function parameters values.
	class CAddParamNameDebug: public IOpRunCode
	{
	private :
		NLAIAGENT::IBaseGroupType* _DebugAttrib;

	public:
		static const NLAIC::CIdentType IdAddParamNameDebug;
	public:

		CAddParamNameDebug(const NLAIAGENT::IBaseGroupType &debugAttrib);
		
		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);

		~CAddParamNameDebug();

		void getDebugResult(char *str,CCodeContext &context) const;

		const NLAIC::IBasicType *clone() const;

		const NLAIC::IBasicType *newInstance() const;

		const NLAIC::CIdentType &getType() const;
		
		void save(NLMISC::IStream &os);

		void load(NLMISC::IStream &is);
	};
}

#include "script/opcode_call_method.h"
#include "script/opcode_call_lib_method.h"
#endif

