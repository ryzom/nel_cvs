/** \file module.h
 * Class op-code storage.
 *
 * $Id: module.h,v 1.11 2003/02/04 14:21:52 chafik Exp $
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
#ifndef NL_MODULE_H
#define NL_MODULE_H

namespace NLAISCRIPT
{
	///this typedef define an correct std::map for store local variable in the heap.
	typedef std::map<NLAISCRIPT::CStringType , NLAIAGENT::IObjectIA * > TDicoStr;

	/**
	* Class IBlock.
	* 
	* This class allows to store a block of op-code with eachs local variables.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	

	class IBlock: public NLAIC::IBasicInterface
	{
	public:		
		static const NLAIC::CIdentType IdBlock;

	protected:
		///this typedef define an correct std::map iterator for store local variable in the heap.
		typedef std::map<NLAISCRIPT::CStringType , NLAIAGENT::IObjectIA *>::iterator TDicoStrIter;
		
	private:
		///Op-code that define the block. This object is build at the end of parse.
		CCodeBrancheRun*			_Cbr;
		///This list containe the bag of code.
		TListCode					_ListCode;
		///Local variable is stored here.
		TDicoStr					*_DicoLocVar;
		///If user want a reference to the local variable dictionary then we don't have to delete it.
		bool						_HaveToDeleteDico;

		bool						_Debug;				/// Switch between a normal CCodeBrancheRun and a debug one.
		bool						_FirstOpCodeInLine;	/// True while no new opCode as been add since the laste \n.
		uint16						_CurrentLine;		/// Current source code line


	public:

		///Construct with an active dictionarry and debug mode.
		IBlock(bool debugMode,TDicoStr *dico)
		{
			_Debug = debugMode;
			_Cbr = NULL;
			_FirstOpCodeInLine = true;
			_CurrentLine = 0;
			_HaveToDeleteDico = false;
			_DicoLocVar = dico;
		}

		IBlock(bool debugMode)		
		{
			_Debug = debugMode;
			_Cbr = NULL;
			_FirstOpCodeInLine = true;
			_CurrentLine = 0;
			_HaveToDeleteDico = true;
			_DicoLocVar = new TDicoStr;
		}		


		virtual ~IBlock()
		{
			while(_ListCode.size() != 0)
			{
				((IOpCode *)*_ListCode.front())->release();
				delete _ListCode.front();
				_ListCode.pop_front();
			}
			
			if(_Cbr)  _Cbr->release();
			TDicoStrIter it = _DicoLocVar->begin();
			while(it != _DicoLocVar->end())
			{
#ifdef NL_DEBUG				
				const NLAIAGENT::IObjectIA *o = (*it).second;
#endif								
				(*it).second->release();
				it ++;
			}		
			if(_HaveToDeleteDico) delete _DicoLocVar;
			
		}

		///Gets a pointer to a locale variable.
		NLAIAGENT::IObjectIA *getVar(const char *Name)
		{
			TDicoStr::iterator Itr = _DicoLocVar->find(NLAISCRIPT::CStringType(Name));

			if(Itr != _DicoLocVar->end())
			{
				return (*Itr).second;
			}
			
			return NULL;
		}

		NLAIAGENT::IObjectIA *eraseVar(const char *Name)
		{
			TDicoStr::iterator Itr = _DicoLocVar->find(NLAISCRIPT::CStringType(Name));

			if(Itr != _DicoLocVar->end())
			{
				_DicoLocVar->erase(Itr);
			}
			
			return NULL;
		}
		

		///Locks the dictionnarry and get it.
		TDicoStr *lockDictionarry()
		{
			_HaveToDeleteDico = false;
			return _DicoLocVar;
		}

		///Allocs a new local var .
		bool allocLocVar(const char *name, NLAIAGENT::IObjectIA *var)
		{
			if(getVar(name) != NULL) return false;
			_DicoLocVar->insert(TDicoStr::value_type(NLAISCRIPT::CStringType(name),var));		
			return true;
		}

		///Builds the code, here we update all IConstarint reference.
		CCodeBrancheRun *getCode()
		{
			if(/*_Cbr == NULL && */_ListCode.size())
			{
				sint32 k = _ListCode.size();
				CCodeBrancheRun *cbr;
				cbr = new CCodeBrancheRun(k);							
				for(sint32 i = 0; i < k; i++)
				{
					CBagOfCode *x = _ListCode.front();
					(*cbr)[i] = (IOpCode *)*x;
					x->setConstraintIndex(i,cbr);
					delete x;
					_ListCode.pop_front();
				}
				return cbr;

			}		
			return NULL;
		}

		///Build a CCodeBrancheRunDebug
		CCodeBrancheRunDebug* getCodeDebug(IScriptDebugSource* sourceCode)
		{
			if(/*_Cbr == NULL && */_ListCode.size())
			{
				sint32 k = _ListCode.size();
				CCodeBrancheRunDebug *cbr;
				cbr = new CCodeBrancheRunDebug(k, sourceCode);

				for(sint32 i = 0; i < k; i++)
				{
					CBagOfCode *x = _ListCode.front();
					(*cbr)[i] = (IOpCode *)*x;
					x->setConstraintIndex(i,cbr);
					delete x;
					_ListCode.pop_front();
				}
				return cbr;

			}		
			return NULL;
		}

		///If the code is builded.
		sint32 isCodeMonted()
		{		
			return _Cbr != NULL;
		}

		/// \name Base class method.
		//@{
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new IBlock(_Debug);
			return x;
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return IdBlock;
		}

		void getDebugString(std::string &) const
		{
		}

		void save(NLMISC::IStream &os)
		{			
		}

		void load(NLMISC::IStream &is) 
		{			
		}
		const NLAIAGENT::IObjectIA::CProcessResult &run();
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const{ return true;}
		//@}

		///Get the list of code.
		TListCode &listCode()
		{
			return _ListCode;
		}

		///Add a op-code pointer, warnning the function increment the ref of the pointer by incRef()
		void addCode(IOpCode *op)
		{			
			//op->incRef();
			CBagOfCode *x = new CBagOfCode(op);
			_ListCode.push_back(x);

			// Debug part
			CConstraintDebug* pcdb;
			if (_Debug && _FirstOpCodeInLine)
			{
				pcdb = new CConstraintDebug(_CurrentLine, 1);
				x->addConstraint(pcdb);
				_FirstOpCodeInLine = false;
			}
		}	

		/// \name Op-code list manipulation.
		//@{
		void pushCode(IOpCode *op)
		{						
			_ListCode.push_back(new CBagOfCode(op));
		}	
		
		IOpCode *getBack()
		{			
			return (IOpCode *)*_ListCode.back();
		}	

		IOpCode *getFront()
		{			
			return (IOpCode *)*_ListCode.front();
		}	

		void pushBagCode(CBagOfCode *bOp)
		{
			_ListCode.push_back(bOp);
		}

		CBagOfCode *getBagOfCode()
		{			
			return _ListCode.back();
		}	
		//@}

		/// Set the current line number in the source code
		void setCurrentLine (uint16 line)
		{
			_CurrentLine = line;
		}

		/// Set the Flag _FirstOpCodeInLine
		void setFirstOpCodeInLine (bool b)
		{
			_FirstOpCodeInLine = b;
		}
	};
}
#endif
