/** \file codage.h
 * Sevral class for the interpreter fonctionality.
 *
 * $Id: codage.h,v 1.21 2002/08/20 15:22:11 chafik Exp $
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

#ifndef NL_CODAGE_H
#define NL_CODAGE_H
#ifdef NL_OS_WINDOWS
#pragma warning (disable: 4620)
#pragma warning (disable: 4621)
#endif


#include "nel/ai/script/virtual_op_code.h"
#include "nel/ai/script/context_debug.h"

namespace NLAISCRIPT
{

	// ======================================================================================================
	/**
	 * Class CIndexStackPointer.
	 * 
	 * Defined a basic fonctionality of an index pointer.
	 * 
	 *
	 * \author Chafik sameh
	 * \author Nevrax France
	 * \date 2000
	 */
	
	class CIndexStackPointer
	{
	protected:
		/// Index of the stack pointer.
		int		_Sp;

	private:
		///List of mark for the pop-push mark method.
		std::list<int> _Marque;

	public:
		CIndexStackPointer():_Sp(0)
		{
		}

		///To get the pointer.
		virtual operator int ()
		{
			return _Sp;
		}

		///To inc the pointer.
		virtual void operator ++ ( int )
		{		
			_Sp ++;
		}

		///To dec the pointer.
		virtual void operator -- ( int )
		{		
			_Sp --;
		}

		///To add by k range.
		virtual void operator += (int k)
		{
			_Sp += k;
		}

		///To dec by k range.
		virtual void operator -= (int k)
		{		
			_Sp -= k;
		}

		///Add a mark to store the pointer possition in a list.
		virtual void pushMark()
		{
			_Marque.push_back(_Sp);
		}

		///Get the last mark pushed in the list.
		virtual int popMark()
		{
			int i = 0;			
			i = _Marque.back();
			_Marque.pop_back();		
			return i;
		}

		virtual int markSize()
		{
			return _Marque.size();
		}

		///Get the last mark.
		virtual int mark()
		{
			return _Marque.back();		
		}

		///Reinitializ the stack manager
		void clear()
		{
			_Sp = 0;			
			_Marque.clear();
		}

		virtual ~CIndexStackPointer()	
		{
		}

	};

	// ======================================================================================================
	/**
	 * Class CStackPointer.
	 * 
	 * Defined a basic fonctionality of a stack index pointer.
	 * 
	 *
	 * \author Chafik sameh
	 * \author Nevrax France
	 * \date 2000
	 */

	class CStackPointer: public CIndexStackPointer
	{
	private:				
		///the count of allocation table.		
		int				_Count;
		///_Bp is a relative pointer to upright the stack point at the entry point of a methode.
		int				_Bp;
		
		///_Sh is a relative pointer to upright the stack after allocat the argument of a methode.
		int				_Sh;

		///list to storager the last shift release in _Bp
		std::list<int>	_Decalage;
		///list to storager the last shift release in _Sh
		std::list<int>	_Shift;

		///this IObjectIA table have _Count size.
		NLAIAGENT::IObjectIA	**_Stack;	

	public:
		CStackPointer(int count = 1024*8)
		{
			_Count = count;
			_Stack = new NLAIAGENT::IObjectIA *[ count ];
			for(int i = 0; i < _Count; i ++)
			{
				_Stack[i] = NULL;
			}			
			_Sp = 0;
			_Bp = 0;
			_Sh = 0;
		}

		///Get the pointer in the position i.
		NLAIAGENT::IObjectIA *&operator [] (int i)
		{
#ifdef NL_DEBUG
			if ( (_Sp + _Bp + _Sh) >= _Count )
			{
				throw NLAIE::CExceptionIndexError();
			}
#endif			
			return _Stack[i + _Bp + _Sh];
		}

		///Get the IObjectIA pointer in current position.
		operator NLAIAGENT::IObjectIA *()
		{
#ifdef NL_DEBUG
			if ( (_Sp + _Bp + _Sh) >= _Count )
			{
				throw NLAIE::CExceptionIndexError();
			}
#endif			
			return _Stack[_Sp + _Bp + _Sh];
		}

		///Dec the current position of the stack point by one.
		virtual void operator -- (int)
		{
#ifdef NL_DEBUG
			if ( (_Sp + _Bp + _Sh) >= _Count )
			{
				throw NLAIE::CExceptionIndexError();
			}
#endif						
			int i = _Sp + _Bp  + _Sh;
			if(_Stack[i])
			{			
				_Stack[i]->release(); 
				_Stack[i] = NULL;
			}
			_Sp --;
		}
		
		///Dec the current position of the stack point by k unite.
		virtual void operator -= (int k)
		{
#ifdef NL_DEBUG
			if ( (_Sp + _Bp + _Sh) >= _Count )
			{
				throw NLAIE::CExceptionIndexError();
			}
#endif						
			int n = _Sp + _Bp + _Sh;
			for(int i = n - k; i < n; i ++) 
			{
				if(_Stack[i])
				{
					_Stack[i]->release();
					_Stack[i] = NULL;
				}
			}
			_Sp -= k;
		}

		///set an bp shift to relative stack pointer _Bp.
		void addStack(int bp)
		{
			_Bp += bp;
			_Sp -= bp;
			_Decalage.push_back(bp);
		}

		///restore the last _Bp shift to the stack pointer.
		void restoreStack()
		{			
			int k = _Bp;
			_Bp -= _Decalage.back();
			_Decalage.pop_back();
			_Sp += k;
			
			*this -= k;				
		}

		///set an s shift to relative stack pointer _Sh.
		void setShift(int s)
		{
			_Sh += s;
			_Sp -= s;
			_Shift.push_back(s);
		}

		///restore the last _Sh shift to the stack pointer.
		void restoreShift()
		{			
			int s = _Shift.back();
			_Shift.pop_back();
			_Sp += s;
			_Sh -= s;
		}


		///if there is any _Bp shift.
		bool restoreStackState()
		{
			return _Decalage.size() != 0;
		}

		///if there is any _Sh shift.
		bool restoreShiftState()
		{
			return _Shift.size() != 0;
		}
		
		~CStackPointer()
		{		
			if(_Stack)
			{
			
				for(int i = _Bp; i < (_Sp + _Bp); i ++)
				{
					if(_Stack[i] != NULL)
					{
						_Stack[i]->release();
						_Stack[i] = NULL;
					}
				}
				delete []_Stack;
				_Stack = NULL;
			}			
		}
	};


	class CCodeBrancheRun;
	typedef	CStackPointer	tStackRef;
	typedef	CCodeBrancheRun	tCodeRef;

	// ======================================================================================================
	/**
	 * Class CCodeContext.
	 * 
	 * Define the contexte of the interpreter.
	 * 	 	  	 
	 * \author Chafik sameh
	 * \author Robert gabriel
	 * \author Nevrax France
	 * \date 2000
	 */


	class CCodeContext : public NLAIAGENT::IObjectIA
	{		
	public:

		/// Stack is the stack for computing operation same as '+', '/' ...
		tStackRef			&Stack;
		/// Heap is the stack for storage local variable
		tStackRef			&Heap;
		///Code is the code in run.
		tCodeRef			*Code;
		///Self define the this of the interpreter class objects
		const NLAIAGENT::IObjectIA	*Self;
		///InputOutput for the out put and in put request.
		NLAIC::IIO			*InputOutput;
		///Param shared the function attribut.
		std::list<NLAIAGENT::IObjectIA	*> Param;
		///If the debugger is in run ContextDebug define its context.
		CContextDebug		ContextDebug;

	public:
		///Referance of the object in the class factory.
		static const NLAIC::CIdentType IdCodeContext;

	public:

		CCodeContext(tStackRef &stackMem,tStackRef &heapMem,tCodeRef *codeMem,const NLAIAGENT::IObjectIA *thisClass,NLAIC::IIO *io):
		Stack(stackMem),
		Heap(heapMem),		
		Code(codeMem),
		Self(thisClass),
		InputOutput(io)		
		{
			if(InputOutput != NULL) InputOutput->incRef();
		}

		///For reinit the context after using.
		void init()
		{
			ContextDebug.init();
		}

		///set the input output class.
		void setIO(NLAIC::IIO *io)
		{
			if(InputOutput != NULL) InputOutput->release();
			InputOutput = io;			
		}

		/// \name IBasicInterface method.
		//@{
		const NLAIC::CIdentType &getType(void) const
		{
			return IdCodeContext;
		}
		const NLAIC::IBasicType *clone(void) const
		{
			CCodeContext *x = new CCodeContext(Stack,Heap,Code,Self,InputOutput);			
			return x;
		}
		const NLAIC::IBasicType *newInstance(void) const
		{
			return clone();
		}
		void getDebugString(std::string &t) const
		{
			t = NLAIC::stringGetBuild("CCodeContext");
		}
		void save(NLMISC::IStream &f)
		{			
		}
		void load(NLMISC::IStream &f)
		{
		}
		//@}

		/// \name IBasicObjectIA method.
		//@{
		bool isEqual(const class NLAIAGENT::IBasicObjectIA &) const
		{
			return true;
		}
		//@}

		/// \name IObjectIA method.
		//@{
		const NLAIAGENT::IObjectIA::CProcessResult &run(void)
		{
			return NLAIAGENT::IObjectIA::ProcessRun;;
		}
		//@}

		~CCodeContext()
		{
			if(InputOutput != NULL) InputOutput->release();
		}
	};

	class IConstraint;

	// ======================================================================================================
	/**
	 * Class CBagOfCode.
	 * 
	 * Define a container for the op-code produced from the parser.
	 * 
	 * A container of code is define by a op-code pointer and a list of constraine that needs the index range of the op-code in the CCodeBrancheRun class,
	 * where CCodeBrancheRun is the op-code container.
	 *
	 * \author Chafik sameh	 
	 * \author Nevrax France
	 * \date 2000
	 */
	class CBagOfCode 
	{
	private:
		IOpCode *_Opcode;
		std::list<IConstraint *> _Constraint;

	public:
		CBagOfCode(IOpCode *opcode):_Opcode(opcode)
		{
			
		}

		///That is the only way to have the op-code contened.
		operator IOpCode *() const
		{
			return _Opcode;
		}

		///That is the only way to set the op-code in container.
		void setCode(IOpCode *op)
		{
			_Opcode = op;	
		}

		///allow us to add a constraint in the bag.
		void addConstraint(IConstraint *c)
		{			
			_Constraint.push_back(c);
		}

		///allow us to add the all constraint in a bag.
		void addConstraint(CBagOfCode *b)
		{	
			std::list<IConstraint *>::iterator it = b->_Constraint.begin();
			while(it != b->_Constraint.end())
			{				
				_Constraint.push_back(*it++);
			}			
		}

		///setConstraintIndex method transmit to them constraint the index range of op-code and the base pointer of where they are.
		///Not that the base pointer is a CCodeBrancheRun and when it's built an index range is creat for each of op-code in the container at this moment setConstraintIndex we can call the copnstraints.
		void setConstraintIndex(int i,CCodeBrancheRun *c);

		~CBagOfCode(){}
		

	};

	typedef	std::list<CBagOfCode *> tListCode;
	typedef	std::list<CBagOfCode *>::iterator tListCodeIter;

	// ======================================================================================================
	/**
	 * Class CCodeBrancheRun.
	 * 
	 * Define a table for storage and run op-code.
	 * 	 
	 * \author Chafik sameh	 
	 * \author Nevrax France
	 * \date 2000
	 */	 
	class CCodeBrancheRun: public ICodeBranche
	{
	public:
		static NLAIC::IIO	*InputOutput;

	protected:
		///Table for storage the op code.
		IOpCode	**_TableCode;
		///The code pointer index of the curent op code in run.
		int		_Ip;
		///The count of the op code table.
		int		_Count;	 
		///The state of tje last run method called.
		NLAIAGENT::IObjectIA::CProcessResult _RunState;

	protected:

		/// del allow the class to del and reinitialize the class.
		void del()
		{
			if(_TableCode != NULL)
			{
				for(int i = 0 ;i < _Count; i ++)
				{
					if(_TableCode[i]) _TableCode[i]->release();
				}
				delete []_TableCode;
			}
		}

	public:
		static const NLAIC::CIdentType IdCodeBrancheRun;
	public:

		
		///copy constructor of the class.
		CCodeBrancheRun(const CCodeBrancheRun &l):_TableCode(NULL),_Ip(0),_Count(l._Count)
		{
			_TableCode = new IOpCode * [_Count];
			for(int i = 0 ;i < _Count; i ++) 
			{
				_TableCode[i] = (IOpCode *)l._TableCode[i];//->clone();				
				_TableCode[i]->incRef();
			}
		}

		///you can construct the class with a liste of op code.
		CCodeBrancheRun(const tListCode &l):
			_TableCode(NULL),_Ip(0),_Count(l.size())
		{
			initCode(l);	
		}	

		///the constructor built a class with N op code copy of the op parametre.
		CCodeBrancheRun(int N,const IOpCode &op);
		///the constructor built a class with N NUL op code.
		CCodeBrancheRun(int N);

		///the constructor built a class with a stream.
		CCodeBrancheRun(NLMISC::IStream &is):_TableCode(NULL),_Ip(0),_Count(0)
		{
			load(is);
		}

		virtual ~CCodeBrancheRun()
		{
			del();
		}

		
		void initCode(const tListCode &l)
		{
			del();
			_Count = l.size();

			tListCode::const_iterator i;		
			_TableCode = new IOpCode * [l.size()];
			i = l.begin();
			int k = 0;
			while(i != l.end())
			{
				_TableCode[k ++] = (IOpCode *)*i++;
			}		
		}

		void initCode(const CCodeBrancheRun &l)
		{
			del();

			_Ip = 0;
			_Count = l._Count;
			
			_TableCode = new IOpCode * [_Count];		
			for(int i = 0; i < _Count;i ++)
			{
				_TableCode[i ++] = (IOpCode *)l._TableCode[i]->clone();
			}
		}

		///allow us to acces to an element in the internal table of the class.
		IOpCode *&operator [] (int i)
		{
			return _TableCode[i];
		}

		/// set the code pointer to l range.		
		void operator = (uint32 l)
		{
			_Ip = l;
		}

		/// move the ip pointer at l element.
		void operator += (uint32 l)
		{
			_Ip += l;
		}

		/// get the ip pointer.
		operator uint32 ()
		{
			return _Ip;
		}

		void init()
		{
			_Ip = 0;
		}

		/// get the op code from the curent position.
		IOpCode &nextCode()
		{
			return *_TableCode[_Ip++];
		}					

		/// \name ICodeBranche method.
		//@{		
		virtual const NLAIAGENT::IObjectIA::CProcessResult &run(NLAIAGENT::IObjectIA &self);
		virtual const NLAIAGENT::IObjectIA::CProcessResult &run(CCodeContext &);

		NLAIAGENT::TProcessStatement runOpCode(CCodeContext &context);
		void getDebugResult(std::string &str,CCodeContext &context) const;		
		//@}

		/// \name IBasicInterface method.
		//@{
		void save(NLMISC::IStream &f);	
		void load(NLMISC::IStream &f);	
		void getDebugString(std::string &) const;
		const NLAIC::CIdentType &getType() const;		
		const NLAIC::IBasicType *clone() const 
		{
			NLAIC::IBasicType *x = new CCodeBrancheRun(*this);			
			return x;
		}
		const NLAIC::IBasicType *newInstance() const;
		
		//@}

		/// \name IBasicObjectIA method.
		//@{
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const{ return true;}	
		//@}

		/// \name IObjectIA method.
		//@{
		virtual const NLAIAGENT::IObjectIA::CProcessResult &run();		
		//@}
		
	};
}
#include "nel/ai/script/opcode.h"

#endif

