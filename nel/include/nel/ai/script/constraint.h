/** \file constraint.h
 * Sevral class for the compiler fonctionality.
 *
 * $Id: constraint.h,v 1.10 2002/05/27 09:22:45 chafik Exp $
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

#ifndef NL_CONSTRAINT_H
#define NL_CONSTRAINT_H

#include "nel/ai/script/interpret_methodes.h"

namespace NLAISCRIPT
{
	class CCompilateur;
	class CCodeBrancheRun;	
	class CMethodeName;
	class IClassInterpret;
	class IOpCode;

	/**
	* Struct CFunctionTag.
	* 
	* This structer is used for passing method characteristic.
	* 
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	struct CFunctionTag
	{
		///Idee of the base class were the method is. Idee have the same functionality as a offset.
		sint32 Inheritance;
		///Offset of the method in the class.
		sint32 MethodNum;
		///Name and type of the method.
		CMethodeName *Method;
		///Pointer adress of the class that the metod is define, in general this pointer is the class strored in the class factory.
		const NLAIAGENT::IObjectIA *Object;
		///Method name.
		const NLAIAGENT::IVarName *MethodName;
		///Hierarchy access of the method.
		std::list<sint32>	Member;
		///Type of returned expression.
		NLAIAGENT::IObjectIA *ReturnType;

		///Default constructor.
		CFunctionTag()
		{
			Method = NULL;
			Object = NULL;
			ReturnType = NULL;
			MethodName = NULL;
		}

		///Copy constructor. Its needed because some method use a stack CFunctionTag manipulation.
		CFunctionTag(const CFunctionTag &c)
		{
			Inheritance = c.Inheritance;
			MethodNum = c.MethodNum;
			Method = c.Method;  
			Object = c.Object;
			MethodName = c.MethodName;
			Member = c.Member;
			ReturnType = c.ReturnType;
			if(ReturnType) ReturnType->incRef();
		}

		///Its needed because some method use a stack CFunctionTag manipulation.
		const CFunctionTag &operator = (const CFunctionTag &c)
		{
			Inheritance = c.Inheritance;
			MethodNum = c.MethodNum;
			Method = c.Method;  
			Object = c.Object;
			MethodName = c.MethodName;
			Member = c.Member;
			if(ReturnType) ReturnType->release();
			ReturnType = c.ReturnType;
			if(ReturnType) ReturnType->incRef();
			return *this;
		}

		
		///Set the type of return method expression, the type is a NLAIAGENT::IObjectIA where its used the getType member method.		
		void setTypeObject(NLAIAGENT::IObjectIA *o)
		{
			if(ReturnType) ReturnType->release();
			ReturnType = o;
			ReturnType->incRef();
		}

		virtual ~CFunctionTag()
		{
			if(ReturnType) ReturnType->release();
		}

	};

	/**
	* Class IConstraint.
	* 
	* This class is a abstract class used for all back prossing work needed for the compiler. Earlier than do multy passing parssing, we chose to 
	* push all expression needed to be clarify in a list of IConstraint where its can auto-call them self.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class IConstraint : public NLAIC::IPointerGestion
	{
	public:
		/**
		This enum define all IConstraint class type defined.
		That is'nt very clean because it is'nt very objects way to do jobs but that make easy to make job with out make heavy the hisarchy class.
		Because the best way is to derive class form NLAIC::IBasicInterface but we have to register class in class factory, define a save an load ..., in short
		make a lot of work for an internal work. If later peopole need this kind of class we'll clean it.
		*/
		enum ConstraintTypeEnum
		{
			///Class CConstraintMethode.
			constraintMethode,
			///Class CConstraintChkMethodeType.
			constraintChkMethodeType,
			///Class COperandSimple
			operandSimple,
			///class COperandSimpleListOr
			operandSimpleListOr,
			///Any class!!!!
			operandAnyObject,
			///Class COperandVoid
			operandVoid,
			///Class COperandUnknown
			operandUnknown,
			///Class COperationType
			operationType,
			///Class COperationTypeGD
			operationTypeGD,
			///Class COperandListType
			operandListType,
			///Class CConstraintStackComp
			constraintStackComp,
			///Class CConstraintDebug
			constraintDebug,
			///Class CConstraintFindRun
			constraintFindRun
		};

	private:
		///List of constraint dependency. When constraint is resolve it call all constraint contain in this list and run its.
		std::list<IConstraint *> _ConstraintList;

	public:
		IConstraint() {}

		///Run with a CCompilateur dependency.
		virtual void run(CCompilateur &) = 0;
		///Run with a IConstraint dependency.
		virtual void run(IConstraint *) = 0;
		///If the constraint is resolved.
		virtual bool satisfied() = 0;
		/**Add a op-code dependency.
		the integer n is the index of the opcode in in the opcodePtr pointer.
		*/
		virtual void addIndex(int n,CCodeBrancheRun *opcodePtr) = 0;
		///get the type of class. the type must be in the ConstraintTypeEnum enumeration.
		virtual ConstraintTypeEnum getTypeOfClass() const = 0;
		///get an error string when the constraint is'nt stisfied.
		virtual void getError(char *) const = 0;
		
		///get the line of the expression that it have to be resolve by constraint in the code source.
		virtual int getLine() const = 0;		
		///get the colone of the expression that it have to be resolve by constraint in the code source.
		virtual int getColone() const = 0;		
		
		virtual bool operator == (const IConstraint &) const = 0;

		///Type of the expression that it have to be resolve by constraint.
		virtual const NLAIC::CIdentType *getConstraintTypeOf() = 0;
		///Get a clone pointer memory.
		virtual const IConstraint *clone() const = 0;
		///Get a string info.
		virtual const char *getInfo() = 0;
		///If this constraint depond on other constraint.
		virtual bool dependOn(const IConstraint *) const = 0;
		
		///Add a constraint dependency. We add a constraint dependency when a given constraint want the result of an other constraint.
		void addConstraint(IConstraint *c)
		{
			_ConstraintList.push_back(c);
		}
		///pop the last constaint dependency.
		void popConstraint(IConstraint *c);	
		///When a constraint is resoved,we call all dependency.
		void runConnexcion()
		{
			while(_ConstraintList.size())
			{
				IConstraint *a = _ConstraintList.back();
				_ConstraintList.pop_back();
				a->run(this);
			}		
		}

		virtual ~IConstraint() 
		{
		}

	};	
	class IClassInterpret;
	

	/**
	* Class CConstraintMethode.
	* 
	* This class is allow to find method.
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class CConstraintMethode: public IConstraint
	{	

	public:
		/**
		This enum allow to define the way to call the method that the class have to find.
		Later we use this enum to build the correct opcode.
		*/
		enum TCallTypeOpCode{
			///The call is normal. Base class that constain method is the self class.
			normalCall,
			///Call mehod from the stack. Base class that constain method is stored in the stack.
			stackCall,
			///Call mehod from the heap. Base class that constain method is stored in the heap.
			heapCall,
			///Call constructor method.
			newCall,
			///We juste search method.
			searchCall
		};
	protected:
		///Type of search. 
		TCallTypeOpCode _CallType;
		///Type of base class that constain method. A NULL value mean that method is in the last class parsed.
		IConstraint *_BaseClass;
		///Hierarchy method name (a hierarchy name is some thing like classBaseName.className.methodName().)
		NLAIAGENT::IBaseGroupType *_MethodName;
		///Method argument.
		CParam *_Param;		
		bool _Satisfied;				
		int _Lin;
		int _Col;
		std::list<std::pair<int, CCodeBrancheRun *> > _Code;
		char *_Txt;
		IConstraint *_Type;
		char *_TxtInfo;		
		int _PosHeap;
		bool _DelCType;
		CFunctionTag _M;		
		
	public:
		CConstraintMethode(TCallTypeOpCode callType,int posHeap,IConstraint *baseClass,NLAIAGENT::IBaseGroupType *methodName,CParam *param,int lin,int col):
			_CallType(callType),_PosHeap(posHeap),_BaseClass(baseClass),_MethodName(methodName),_Param(param),_Satisfied(false),_Lin(lin),_Col(col)
		{						
			_Txt = NULL;
			_Type = NULL;
			std::string txt;
			std::string m;
			std::string p;
			_MethodName->getDebugString(m);
			_Param->getDebugString(p);
			txt = "constraint<CConstraintMethode> for";
			txt += m;
			txt += p;
			txt += NLAIC::stringGetBuild(" at line %d",_Lin);
			_TxtInfo = new char [strlen(txt.c_str()) + 1];
			strcpy(_TxtInfo,txt.c_str());			
			_DelCType = false;
		}
		/// \name IConstraint method.
		//@{

		const char *getInfo() 
		{
			return _TxtInfo;
		}		

		virtual void run(CCompilateur &);
		virtual void run(IConstraint *);

		bool satisfied()
		{
			return _Satisfied;
		}		

		bool dependOn(const IConstraint *c) const
		{
			if( c == _Type || c == _BaseClass || c == this) return true;
			return false;
		}

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass() && 
				((const CConstraintMethode &)c)._CallType == _CallType &&
				((const CConstraintMethode &)c)._BaseClass == _BaseClass &&
				((const CConstraintMethode &)c).isEqual(*_MethodName,*_Param)) 
			{				
				return true;
			}
			return false;
		}		

		void addIndex(int i,CCodeBrancheRun *c)
		{			
			_Code.push_back(std::pair<sint32, CCodeBrancheRun *>(i,c));
		}

		void getError(char *) const;

		const IConstraint *clone() const;					

		bool isEqual(const NLAIAGENT::IBaseGroupType &,const CParam &) const;

		virtual void setOpCode(CCompilateur &comp,IOpCode *x,IConstraint *cType,bool del);

		const NLAIC::CIdentType *getConstraintTypeOf()
		{							
			return _Type == NULL ? NULL : _Type->getConstraintTypeOf();
		}

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return constraintMethode;
		}

		const CFunctionTag &getfunctionTag() const
		{
			return _M;
		}

		int getLine() const
		{
			return _Lin;
		}
		int getColone() const
		{
			return _Col;
		}
		//@}

		virtual ~CConstraintMethode()
		{
			_MethodName->release();
			_Param->release();
			if(_BaseClass)_BaseClass->release();
			if(_Txt != NULL) delete _Txt;
			delete _TxtInfo;			
			if(_DelCType) _Type->release();
		}
		
	};	
}
#include "nel/ai/script/constraint_type.h"
#endif
