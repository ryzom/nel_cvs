/** \file type_def.h
 * Sevral class for typing object.
 *
 * $Id: type_def.h,v 1.14 2002/08/21 13:58:17 lecroart Exp $
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

#ifndef NL_TYPE_DEF_H
#define NL_TYPE_DEF_H
#include "nel/ai/script/constraint.h"

namespace NLAISCRIPT
{	
	struct CFactorType;

	/**
	* Class IOpType.
	* 
	* It is an abstract class to define a type of an expression. Note that a type is a constraint where it can be computate when user want.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class IOpType: public IConstraint
	{		

	public:
		IOpType()
		{
		}

		///\name Redefinition of base class method
		//@{
		virtual void run(IConstraint *){}
		virtual void run(CCompilateur &){}
		virtual void addIndex(int i,CCodeBrancheRun *){}
		virtual void getError(char *) const{}
		virtual int getLine() const{ return 0;}
		virtual int getColone() const{return 0;}
		double evalParam(IOpType *);
		virtual double eval (IOpType *);
		//@}

		virtual ~IOpType();

	public:
		static const IOpType*loadIOpType(NLMISC::IStream &f);		
	};

	/**
	* Class COperandVoid.
	* 
	* This class define an void type, its use for the no return method type.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class COperandVoid: public IOpType
	{
	
	public:
		COperandVoid()
		{						
		}

		const char *getInfo()
		{
			return "constraint<COperandVoid>";
		}

		const NLAIC::CIdentType *getConstraintTypeOf()
		{
			return &NLAIC::CIdentType::VoidType;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if ( f.isReading() )
			{
				throw NLMISC::EStream();
			}
			else
			{
				sint32 n =  (sint32)getTypeOfClass();
				f.serial(n);
			}
		}

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operandVoid;
		}

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass())
			{
				return true;
			}
			return false;
		}

		bool dependOn(const IConstraint *) const
		{
			return false;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperandVoid();
			return x;
		}		

		bool satisfied()
		{
			return true;
		}		

		virtual ~COperandVoid()
		{						
		}
	};

	/**
	* Class COperandAnyObject.
	* 
	* This class define an void type, its use for the no return method type.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/	
	class COperandAnyObject: public IOpType
	{
	
	public:
		COperandAnyObject()
		{						
		}

		const char *getInfo()
		{
			return "constraint<COperandAnyObject>";
		}

		const NLAIC::CIdentType *getConstraintTypeOf()
		{
			return &NLAIC::CIdentType::VoidType;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if ( f.isReading() )
			{
				throw NLMISC::EStream();
			}
			else
			{
				sint32 n =  (sint32)getTypeOfClass();
				f.serial(n);
			}
		}

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operandAnyObject;
		}

		bool operator == (const IConstraint &c) const
		{			
			return true;
		}

		bool dependOn(const IConstraint *) const
		{
			return false;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperandAnyObject();
			return x;
		}		

		bool satisfied()
		{
			return true;
		}		

		virtual ~COperandAnyObject()
		{						
		}
	};

	/**
	* Class COperandSimple.
	* 
	* This class define an know type.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class COperandSimple: public IOpType
	{
	private:
		NLAIC::CIdentType *_Ident;
		std::string _TxtInfo;

	public:
		COperandSimple(NLMISC::IStream	&f)
		{
			_Ident = new NLAIC::CIdentType(f);			
		}
		COperandSimple(NLAIC::CIdentType *i) : _Ident(i)
		{			
		}

		const char *getInfo()
		{
			if(_TxtInfo.begin() == _TxtInfo.end())
			{
				_TxtInfo = NLAIC::stringGetBuild("constraint<COperandSimple> for %s",(const char *)*_Ident);				
			}
			return _TxtInfo.c_str();
		}

		const NLAIC::CIdentType *getConstraintTypeOf()
		{
			return _Ident;
		}

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operandSimple;
		}

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass() && *((const COperandSimple &)c)._Ident == *_Ident)
			{
				return true;
			}
			return false;
		}

		bool dependOn(const IConstraint *) const
		{
			return false;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperandSimple(new NLAIC::CIdentType(*_Ident));
			return x;
		}		

		bool satisfied()
		{
			return true;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if ( f.isReading() )
			{
				if(_Ident) _Ident->release();
				_Ident = new NLAIC::CIdentType(f);
				_TxtInfo.clear();
				
			}
			else
			{
				sint32 n =  (sint32)getTypeOfClass();
				f.serial(n);
				_Ident->serial(f);
			}
		}

		virtual ~COperandSimple()
		{
			_Ident->release();			
		}
	};


	/**	
	* 
	* This class define an know type under a list od type.
	* Type = Typ_1 | Typ_2 ... | Type_n
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class COperandSimpleListOr: public IOpType
	{
	private:
		std::list<NLAIC::CIdentType *> _TypeListe;
		//NLAIC::CIdentType *_Ident;
		char *_TxtInfo;

	public:
		COperandSimpleListOr(NLMISC::IStream &f)
		{
			serial(f);
		}		

		COperandSimpleListOr(const std::list<NLAIC::CIdentType *> &l)
		{
			char txt[1028*8];		
			sprintf(txt,"constraint<COperandSimpleListOr> for ...");
			_TxtInfo = new char [strlen(txt) + 1];
			strcpy(_TxtInfo,txt);

			std::list<NLAIC::CIdentType *>::const_iterator i = l.begin();
			while(i != l.end())
			{
				_TypeListe.push_back(new NLAIC::CIdentType(*(*i++)));
			}
		}


		COperandSimpleListOr(int count, ...);

		const std::list<NLAIC::CIdentType *> &getList() const
		{
			return _TypeListe;
		}

		const char *getInfo()
		{
			return _TxtInfo;
		}

		const NLAIC::CIdentType *getConstraintTypeOf()
		{
			return _TypeListe.back();
		}

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operandSimpleListOr;
		}

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass())
			{		
				const COperandSimpleListOr &l = (const COperandSimpleListOr &)c;
				if(l._TypeListe.size() != _TypeListe.size()) return false;

				std::list<NLAIC::CIdentType *>::const_iterator i = _TypeListe.begin();
				std::list<NLAIC::CIdentType *>::const_iterator j = l._TypeListe.begin();
				while(i != _TypeListe.end())
				{
					if(!((*i++) == (*j++))) return false;
				}
				return true;
			}
			return false;
		}

		bool dependOn(const IConstraint *) const
		{
			return false;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperandSimpleListOr(_TypeListe);
			return x;
		}		

		bool satisfied()
		{
			return true;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if ( f.isReading() )
			{
				while(_TypeListe.size())
				{
					delete _TypeListe.back();
					_TypeListe.pop_back();
				}
				
			
				sint32 i;
				f.serial(i);				
				while(i --)
				{
					NLAIC::CIdentType *ident = new NLAIC::CIdentType(f);
					_TypeListe.push_back(ident);
				}				
				delete _TxtInfo;
				char txt[1028*8];		
				sprintf(txt,"constraint<COperandSimpleListOr> for ...");
				_TxtInfo = new char [strlen(txt) + 1];
				strcpy(_TxtInfo,txt);
			}
			else
			{
				sint32 n =  (sint32)getTypeOfClass();
				f.serial(n);
				n = _TypeListe.size();
				f.serial(n);
				std::list<NLAIC::CIdentType *>::iterator i = _TypeListe.begin();
				while(i != _TypeListe.end())
				{
					(*i++)->serial(f);
				}								
			}
		}

		virtual double eval (IOpType *);

		virtual ~COperandSimpleListOr()
		{
			while(_TypeListe.size())
			{
				delete _TypeListe.back();
				_TypeListe.pop_back();
			}
			delete _TxtInfo;
		}
	};


	/**
	* Class COperandUnknown.
	* 
	* This class define an unknow type. type is an IConstraint that we have to resolve it.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class COperandUnknown: public IOpType
	{
	private:		
		 IConstraint *_Constraint;		 
		 NLAIC::CIdentType *_Id;
		 char *_TxtInfo;
		 bool _RunIsDone;
		
	public:
		COperandUnknown(IConstraint *constraint):_Constraint(constraint),_Id(NULL)
		{		
			char txt[1028*8];		
			sprintf(txt,"constraint<COperandUnknown> for %s",_Constraint->getInfo());
			_TxtInfo = new char [strlen(txt) + 1];
			strcpy(_TxtInfo,txt);
			_Constraint->addConstraint(this);
			_RunIsDone = false;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if(_Id != NULL)
			{
				COperandSimple n(new NLAIC::CIdentType (*_Id));
				n.serial(f);
			}
			else throw NLMISC::EStream();
		}

		const char *getInfo()
		{
			return _TxtInfo;
		}
		
		const NLAIC::CIdentType *getConstraintTypeOf() 
		{
			if(_Id == NULL)
			{
				_Id = (NLAIC::CIdentType *)_Constraint->getConstraintTypeOf();
			}
			return _Id;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperandUnknown((IConstraint *)_Constraint->clone());		
			return x;
		}		

		void run(CCompilateur &c)
		{
			if(_Id == NULL)
			{
				_Constraint->run(c);
				run(_Constraint);
			}
		}
		
		void run(IConstraint *c)
		{			
			if(c == _Constraint) 
			{
				_Id = (NLAIC::CIdentType *)_Constraint->getConstraintTypeOf();
				if(satisfied())
				{
					_RunIsDone = true;					
				}
			}
		}
		
		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass() && *((const COperandUnknown &)c)._Constraint == *_Constraint)
			{
				return true;
			}
			return false;
		}

		bool dependOn(const IConstraint *c) const
		{
			return _Constraint == c || _Constraint->dependOn(c);
		}
		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operandUnknown;
		}

		bool satisfied()
		{
			return getConstraintTypeOf()  != NULL;
		}
		
		virtual ~COperandUnknown()			
		{			
			if (_Constraint != NULL)
			{			
				if(!_RunIsDone) _Constraint->popConstraint(this);				
				
				_Constraint->release();
				_Constraint = NULL;

			}
			delete _TxtInfo;
		}		
	};

	/**
	* Class COperationType.
	* 
	* This class define an unknow opertation type. For exemple to know the type of the expression (-a).
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class COperationType: public IOpType
	{
	private:
		///Type of the operand.
		IOpType	*_Operand;
		///Type of operation
		NLAIC::CTypeOfOperator::TTypeOp _Op;
		char *_TxtInfo;
		NLAIC::CIdentType *_Id;
	public:
		COperationType(const IOpType &operand,NLAIC::CTypeOfOperator::TTypeOp op):
			_Operand((IOpType *)operand.clone()),_Op(op)
		{

			char txt[1028*8];		
			sprintf(txt,"constraint<COperationType> for %s",_Operand->getInfo());
			_TxtInfo = new char [strlen(txt) + 1];
			strcpy(_TxtInfo,txt);
			_Id = NULL;
		}


		COperationType():_Operand(NULL),_Op((NLAIC::CTypeOfOperator::TTypeOp)0)
		{
			_Id = NULL;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if(_Id != NULL)
			{
				COperandSimple n(new NLAIC::CIdentType (*_Id));
				n.serial(f);
			}
			else throw NLMISC::EStream();
		}

		const char *getInfo()
		{
			return _TxtInfo;
		}

		void setOp(NLAIC::CTypeOfOperator::TTypeOp op)
		{
			_Op = op;
		}

		void setOperand(IOpType *operand)
		{
			_Operand = operand;
			char txt[1028*8];		
			sprintf(txt,"constraint<COperationType> for %s",operand->getInfo());
			_TxtInfo = new char [strlen(txt) + 1];
			strcpy(_TxtInfo,txt);
		}

		bool dependOn(const IConstraint *c) const
		{
			return _Operand->dependOn(c);
		}

		virtual void run(CCompilateur &c)
		{
			_Operand->run(c);
			_Id = (NLAIC::CIdentType *)getConstraintTypeOf();			
		}
		

		void run(IConstraint *c)
		{
		}

		const NLAIC::CIdentType *getConstraintTypeOf() 
		{							
			if(_Id) return _Id;
			else return (_Id = (NLAIC::CIdentType *)_Operand->getConstraintTypeOf());
		}
		
		bool satisfied()
		{			
			return getConstraintTypeOf() != NULL;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperationType((const COperationType &)*_Operand,_Op);
			return x;      				
		}
		

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass() && *((const COperationType &)c)._Operand == *_Operand && ((const COperationType &)c)._Op == _Op)
			{
				return true;
			}
			return false;
		}		

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operationType;
		}

		virtual ~COperationType()
		{
			_Operand->release();
			delete _TxtInfo;
		}
	};

	/**
	* Class COperationTypeGD.
	* 
	* This class define an unknow opertation type. For exemple to know the type of the expression (a + b). 
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class COperationTypeGD: public IOpType
	{
	private:
		///Right oprand.
		IOpType	*_OpG;
		//Left operand.
		IOpType	*_OpD;
		///Operation type.
		NLAIC::CTypeOfOperator::TTypeOp _Op;
		char *_TxtInfo;
		NLAIC::CIdentType *_Id;
	public:
		COperationTypeGD(const COperationType &opG,const COperationType &opD,NLAIC::CTypeOfOperator::TTypeOp op):
				_OpG((IOpType	*)opG.clone()),_OpD((IOpType *)opD.clone()),_Op(op)
		{
			char txt[1028*8];		
			sprintf(txt,"constraint<COperationType> for <%s> <%s>",_OpG->getInfo(),_OpD->getInfo());
			_TxtInfo = new char [strlen(txt) + 1];
			strcpy(_TxtInfo,txt);
			_Id = NULL;
			_TxtInfo = NULL;
					
		}
		
		COperationTypeGD()
		{
			_OpG = _OpD = NULL;
			_Op = (NLAIC::CTypeOfOperator::TTypeOp)0;
			_Id = NULL;
			_TxtInfo = NULL;
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if(_Id != NULL)
			{
				COperandSimple n(new NLAIC::CIdentType (*_Id));
				n.serial(f);
			}
			else throw NLMISC::EStream();
		}

		const char *getInfo()
		{
			if(_TxtInfo == NULL)
			{
				char txt[1028*8];		
				sprintf(txt,"constraint<COperationTypeGD> for <%s> <%s>",_OpG->getInfo(),_OpD->getInfo());
				_TxtInfo = new char [strlen(txt) + 1];
				strcpy(_TxtInfo,txt);
			}
			return _TxtInfo;
		}

		bool dependOn(const IConstraint *c) const
		{
			return _OpG->dependOn(c) || c == _OpG || _OpD->dependOn(c) || c == _OpD;
		}
		
		void setOp(NLAIC::CTypeOfOperator::TTypeOp op)
		{
			_Op = op;
		}

		void setOperationG(IOpType *opG)
		{
			_OpG = opG;			
			
		}

		void setOperationD(IOpType *opD)
		{
			_OpD = opD;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperationTypeGD ((const COperationType &)*_OpG,(const COperationType &)*_OpD,_Op);
			return x;
		}

		virtual void run(CCompilateur &c)
		{
			_OpG->run(c);
			_OpD->run(c);
			
			_Id = (NLAIC::CIdentType *)getConstraintTypeOf();
		}
		

		void run(IConstraint *c)
		{
		}

		const NLAIC::CIdentType *getConstraintTypeOf()
		{			
			if(satisfied()) return (_Id = (NLAIC::CIdentType *)_OpG->getConstraintTypeOf());
			return NULL;
		}
		
		bool satisfied();
		

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass() && *((const COperationTypeGD &)c)._OpG == *_OpG && *((const COperationTypeGD &)c)._OpD == *_OpD)
			{
				return true;
			}
			return false;
		}
		

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operationTypeGD;
		}

		virtual ~COperationTypeGD()
		{
			if(_OpG) _OpG->release();
			if(_OpD) _OpD->release();
			delete _TxtInfo;
		}
	};

	/**
	* Class COperandListType.
	* 
	* This class define an unknow list operand type. Its used for for object how have more than one way to compute the type.
	* This class can be interpreted by: Type = Type_1 | Type_2 |...| Type_i |...|Type_n. Where type_i is an IOpType to eval.
	*
	* \author Chafik sameh	
	* \author Nevrax France
	* \date 2000
	*/
	class COperandListType: public IOpType
	{
	private:
		std::list<IOpType *> _List;
		NLAIC::CIdentType *_Id;
		bool _S;
		char *_TxtInfo;
	public:

		COperandListType(const COperandListType &c)			
		{
			std::list<IOpType *>::const_iterator i = c._List.begin();
			_S = c._S;
			while(i != c._List.end())
			{
				_List.push_back(*i);
				(*i)->incRef();
				i ++;
			}			
			_TxtInfo = new char [strlen(c._TxtInfo) + 1];
			strcpy(_TxtInfo,c._TxtInfo);
			if(c._Id) _Id = new NLAIC::CIdentType (*c._Id);
		}

		COperandListType()
		{
			_Id = NULL;
			_S = false;
			char txt[1028*8];		
			sprintf(txt,"constraint<COperandListType>");
			_TxtInfo = new char [strlen(txt) + 1];
			strcpy(_TxtInfo,txt);
		}

		virtual void serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
		{
			if(_Id != NULL)
			{
				COperandSimple n(new NLAIC::CIdentType (*_Id));
				n.serial(f);
			}
			else throw NLMISC::EStream();
		}

		virtual ConstraintTypeEnum getTypeOfClass() const
		{
			return operandListType;
		}

		const IConstraint *clone() const
		{
			IConstraint *x = new COperandListType (*this);
			return x;
		}

		const NLAIC::CIdentType *getConstraintTypeOf()
		{
			return _Id;
		}

		bool operator == (const IConstraint &c) const
		{
			if(getTypeOfClass() == c.getTypeOfClass())
			{
				const COperandListType &o = (const COperandListType &)c;

				if(o._List.size() != _List.size()) return false;

				std::list<IOpType *>::const_iterator i = o._List.begin();				
				std::list<IOpType *>::const_iterator j = _List.begin();				
				while(j != _List.end())
				{
					if(!(*(*i) == *(*j))) return false;
					i ++;
					j ++;
				}							
				return true;
			}
			else
			{
				std::list<IOpType *>::const_iterator j = _List.begin();
				while(j != _List.end())
				{
					if( c == *(*j)) return true;
					j ++;
				}							
				return false;
			}
			return false;
		}

		const char *getInfo()
		{
			return _TxtInfo;
		}

		bool satisfied()
		{
			return _S;
		}

		void run(CCompilateur &c)
		{
			std::list<IOpType *>::iterator i = _List.begin();
			_S = true;
			while(i != _List.end())
			{
				IOpType *o = *i++;
				o->run(c);
				if(o->satisfied())
				{
					if(_Id == NULL) 
					{
						_Id = new NLAIC::CIdentType(*o->getConstraintTypeOf());
					}
					else if(!(*_Id == *o->getConstraintTypeOf())) _S = false;
				}
			}
		}

		bool dependOn(const IConstraint *a) const
		{
			std::list<IOpType *>::const_iterator i = _List.begin();			
			while(i != _List.end())
			{
				const IConstraint *o = *i++;
				if(a == o || o->dependOn(a)) return true;
			}
			return false;
		}

		void add(IOpType *o)
		{
			_List.push_back(o);
		}

		virtual ~COperandListType()
		{
			while(_List.size())
			{
				_List.back()->release();
				_List.pop_back();
			}
			if(_Id) delete _Id;
			delete _TxtInfo;
		}


	};

	///Loader for IOpType. Not that we save and load an know type.
	inline const IOpType *IOpType::loadIOpType(NLMISC::IStream &f)
	{
		IConstraint::ConstraintTypeEnum k = constraintMethode;
		IOpType	*x;

		f.serial((sint32 &)k);
	
		switch(k)
		{
		case operandAnyObject:
			x = new  COperandAnyObject();
			return x;

		case operandVoid:
			x = new COperandVoid();
			return x;

		case operandSimple:
			x = new COperandSimple(f);
			return x;

		case operandSimpleListOr:
			x = new COperandSimpleListOr(f);
			return x;
		default: break;
		}
		return NULL;
	}
}
#endif
