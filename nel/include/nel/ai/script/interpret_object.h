/** \file interpret_object.h
 * Sevral class for define a script class.
 *
 * $Id: interpret_object.h,v 1.20 2002/11/05 14:07:28 chafik Exp $
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

#ifndef NL_INTERPRET_OBJECT_H
#define NL_INTERPRET_OBJECT_H

#include "nel/ai/agent/agent.h"
#include "nel/ai/script/interpret_methodes.h"

namespace NLAISCRIPT
{
	/**
	* Struct CComponent.
	* 
	* This structer is used for define a component characteristic.
	* 
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	struct CComponent
	{
		///Component type name.
		NLAIAGENT::IVarName *RegisterName;	
		///Name of the Component.
		NLAIAGENT::IVarName *ObjectName;
		/// Static value if the component is a static one
		NLAIAGENT::IObjectIA *StaticValue;

		/**
		Local define the distribution type.

		If it true then the component is all time local.
		If it false the component can be just distributed in the server. And we can't manipulate directly but by message. 
		*/
		bool Local;
		bool Static;
		CComponent()
		{
			RegisterName = NULL;			// name of the type of component in the registry
			ObjectName = NULL;				// name of the member component in the class
			Local = false;					// true if the object is not accessible from a remote machine
			Static = false;					// true if the component is static (defined and stored in the class)
			StaticValue = NULL;				// static value of the component, NULL if non static
		}
		/**
		Save component characteristic.
		*/
		virtual void save(NLMISC::IStream &);
		/**
		Load component characteristic.
		*/
		virtual void load(NLMISC::IStream &);
	};

	/**
	* Class IClassInterpret.
	* 
	* This class define a class interpreted.
	* 
	*
	* \author Chafik sameh
	* \author Portier Perre.
	* \author Nevrax France
	* \date 2000
	*/	
	class IClassInterpret: public NLAIAGENT::IObjectIA
	{
	private:
		///Name of the class
		NLAIAGENT::IVarName *_Name;

	protected:
		///Type of the class.
		NLAIC::CIdentType *_IdType;

		IClassInterpret(const NLAIAGENT::IVarName &,const IClassInterpret &);

	public:
		/// Returns a static component from its name in the class
		virtual CComponent *getComponent(const NLAIAGENT::IVarName &) const = 0;
		
		/// Returns a component from its index
		virtual CComponent *getComponent(sint32 i) const = 0;

	public:		

		IClassInterpret();
		IClassInterpret(NLAIC::CIdentType *idType);
		virtual ~IClassInterpret();
		virtual const NLAIC::CIdentType &getType() const;
		void setType(NLAIC::CIdentType *idType);
		void setType(const NLAIAGENT::IVarName &, const IClassInterpret &);

		/// Instanciates an object of the class
		virtual NLAIAGENT::IObjectIA *buildNewInstance() const = 0;

		/// Registers a new static component
		virtual sint32 registerComponent(const NLAIAGENT::IVarName &) = 0;
		/// Registers a new static component
		virtual void classIsMounted() = 0;
				
		/// Returns the number of static components in the class
		virtual sint32 getStaticMemberSize() const = 0;		

		/// \name code method.		
		//@{		
		virtual sint32 addBrancheCode(const NLAIAGENT::IVarName &,const CParam &) = 0;
		virtual CMethodeName &getBrancheCode() const = 0;
		///get method in the class.
		virtual CMethodeName &getBrancheCode(sint32) const= 0;
		///get method in the base class. where h is the base class offset m is the method offset.
		virtual CMethodeName &getBrancheCode(sint32 h, sint32 m) const= 0;
		//@}

		///return the absolut index of the run() method of this class.
		virtual sint32 getRunMethod() const = 0;
		///set the absolut index of the run() method of this class.
		virtual void setRunMethod(sint32 index) = 0;

		/// Builds the v_table
		virtual void buildVTable() = 0;
		/// Return a pointer represent the inheritance range n, the inheritance graph are represented by a vector.
		virtual const IClassInterpret *getInheritance(sint32 n) const = 0;
		/// Get the size of the v_table
		virtual sint32 sizeVTable() const= 0;

		/// Returns the name of the class in the registry
		virtual const NLAIAGENT::IVarName *getClassName() const;
		virtual void setClassName(const NLAIAGENT::IVarName &);

		/// Defines the base class using its name in the registry
		virtual void setInheritanceName(const NLAIAGENT::IVarName &name) = 0;

		/// Returns the name of the base class in the registry
		virtual const NLAIAGENT::IVarName *getInheritanceName() const = 0;

		/// Return a pointer to the base IClassInterpret object
		virtual const IClassInterpret *getBaseClass() const = 0;
		virtual const IClassInterpret *getComputeBaseClass()  = 0;
		///Return the name of the compoment given by an offset.
		virtual const char *getComponentName(sint32 = 0) const = 0;

		virtual NLAIAGENT::tQueue getPrivateMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &param) const = 0;

		const NLAIAGENT::IVarName *getName() const
		{
			return _Name;
		}

		virtual NLAIC::CTypeOfObject getTypeClass() const
		{
			return NLAIC::CTypeOfObject::tInterpret;
		}

		virtual void initStatics()
		{
		}
	};	

	
	
	/**
	* Class CClassInterpretFactory.
	* 
	* This class define a class class factory for object derived from IClassInterpret.
	* 
	*
	* \author Chafik sameh
	* \author Portier Perre.
	* \author Nevrax France
	* \date 2000
	*/	
	class CClassInterpretFactory: public NLAIC::IClassFactory
	{
	private:
		IClassInterpret *_Inst;
	public:
		CClassInterpretFactory(IClassInterpret *a);
		CClassInterpretFactory(const IClassInterpret &a);

		/// \name IClassCFactory method.
		//@{
		void setClass(const NLAIC::IBasicInterface &);
		virtual void getDebugString(std::string &) const;
		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;
		const NLAIC::IBasicInterface *createInstance() const;
		const NLAIC::IBasicInterface *getClass() const;
		//@}

		virtual ~CClassInterpretFactory();
	};


	/**
	* Class CExceptionHaveNoType.
	* 
	* This an no type exception it happen when an object have no NLAIC::CIdentType.
	* 	
	* \author Chafik sameh
	* \author Portier Perre.
	* \author Nevrax France
	* \date 2000
	*/	
	class CExceptionHaveNoType: public NLAIE::IException
	{	
	public:

		CExceptionHaveNoType(const CExceptionHaveNoType &e):NLAIE::IException(e)
		{			
		}		
		
		CExceptionHaveNoType(const std::string &text):NLAIE::IException(text)
		{			
		}		

		virtual const NLAIE::IException *clone() const
		{
			return new CExceptionHaveNoType(*this);
		}

		virtual ~CExceptionHaveNoType() throw()
		{			
		}
	};
}
#endif
