/** \file interpret_object.h
 * Sevral class for define a script class.
 *
 * $Id: interpret_object.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#include "agent/agent.h"
#include "script/interpret_methodes.h"

namespace NLIASCRIPT
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
		NLIAAGENT::IVarName *RegisterName;	
		///Name of the Component.
		NLIAAGENT::IVarName *ObjectName;

		/**
		Local define the distribution type.

		If it true then the component is all time local.
		If it false the component can be just distributed in the server. And we can't manipulate directly but by message. 
		*/
		bool Local;
		CComponent()
		{
			RegisterName = NULL;
			ObjectName = NULL;
			Local = false;
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
	class IClassInterpret: public NLIAAGENT::IObjectIA
	{
	private:
		///Name of the class
		NLIAAGENT::IVarName *_Name;
		///Type of the class.
		NLIAC::CIdentType *_IdType;

	protected:
		IClassInterpret(const NLIAAGENT::IVarName &,const IClassInterpret &);

	public:
		/// Returns a static component from its name in the class
		virtual CComponent *getComponent(const NLIAAGENT::IVarName &) const = 0;
		
		/// Returns a component from its index
		virtual CComponent *getComponent(sint32 i) const = 0;

	public:		

		IClassInterpret();
		IClassInterpret(NLIAC::CIdentType *idType);
		virtual ~IClassInterpret();
		const NLIAC::CIdentType &getType() const;
		void setType(NLIAC::CIdentType *idType);
		void setType(const NLIAAGENT::IVarName &, const IClassInterpret &);

		/// Instanciates an object of the class
		virtual NLIAAGENT::IObjectIA *buildNewInstance() const = 0;

		/// Registers a new static component
		virtual sint32 registerComponent(const NLIAAGENT::IVarName &) = 0;		
				
		/// Returns the number of static components in the class
		virtual sint32 getStaticMemberSize() const = 0;		

		/// \name code method.		
		//@{		
		virtual sint32 addBrancheCode(const NLIAAGENT::IVarName &,const CParam &) = 0;
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
		virtual const NLIAAGENT::IVarName *getClassName() const;
		virtual void setClassName(const NLIAAGENT::IVarName &);

		/// Defines the base class using its name in the registry
		virtual void setInheritanceName(const NLIAAGENT::IVarName &name) = 0;

		/// Returns the name of the base class in the registry
		virtual const NLIAAGENT::IVarName *getInheritanceName() const = 0;

		/// Return a pointer to the base IClassInterpret object
		virtual const IClassInterpret *getBaseClass() const = 0;
		///Return the name of the compoment given by an offset.
		virtual const char *getComponentName(sint32 = 0) const = 0;

		const NLIAAGENT::IVarName *getName() const
		{
			return _Name;
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
	class CClassInterpretFactory: public NLIAC::IClassCFactory
	{
	private:
		IClassInterpret *_inst;
	public:
		CClassInterpretFactory(IClassInterpret *a);

		/// \name IClassCFactory method.
		//@{
		void getDebugString(char *text) const;
		const NLIAC::IBasicType *clone() const;
		const NLIAC::IBasicType *newInstance() const;
		const NLIAC::CIdentType &getType() const;
		const NLIAC::IBasicInterface *createInstance() const;
		const NLIAC::IBasicInterface *getClass() const;
		//@}

		~CClassInterpretFactory();
	};


	/**
	* Class CExceptionHaveNoType.
	* 
	* This an no type exception it happen when an object have no NLIAC::CIdentType.
	* 	
	* \author Chafik sameh
	* \author Portier Perre.
	* \author Nevrax France
	* \date 2000
	*/	
	class CExceptionHaveNoType: public NLIAE::IException
	{
	private:
		char *_Text;
	public:
		
		CExceptionHaveNoType(const char *text)
		{
			_Text = new char [strlen(text) + 1];
			strcpy(_Text,text);
		}

		const char *what() const
		{
			return _Text;
		}

		virtual const NLIAE::IException *clone() const
		{
			return new CExceptionHaveNoType(_Text);
		}

		virtual ~CExceptionHaveNoType()
		{
			delete _Text;
		}
	};
}
#endif
