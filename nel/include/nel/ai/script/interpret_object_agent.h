/** \file interpret_object.h
 * Class for define an agent script class.
 *
 * $Id: interpret_object_agent.h,v 1.2 2001/01/05 16:05:57 chafik Exp $
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
#ifndef NL_INTERPRET_OBJECT_AGENT_H
#define NL_INTERPRET_OBJECT_AGENT_H
#include "script/interpret_object.h"

namespace NLIAAGENT {
	class CAgentScript;
};

namespace NLIASCRIPT
{
	/**
	* Class IAgentMultiClass.
	* 
	* This class is an abstract class use to define some basic functionality.
	* 
	*
	* \author Chafik sameh
	* \author Nevrax France
	* \date 2000
	*/	
	class IAgentMultiClass: public IClassInterpret
	{
	private:
		///Number of hard coded method defined in the C++ base class that the agent dervived from for exemple if the base c++ class is an CAgentScript, an CMessageScript, ...
		sint32	_BaseMethodCount;
		///C++ Base class representative pointer. This pointer allow to get the offset of base method defined in it.
		NLIAAGENT::IObjectIA *_BaseObjectInstance;
	public:

		IAgentMultiClass()
		{
		}
		virtual~IAgentMultiClass()
		{
		}
		///Get the base method count.
		sint32 getBaseMethodCount() const
		{
			return _BaseMethodCount;
		}

		///Set the base method count.
		void setBaseMethodCount(sint32 n)
		{
			_BaseMethodCount = n;
		}

		///Get the C++ base class representative pointer.
		NLIAAGENT::IObjectIA *getBaseObjectInstance() const
		{
			return _BaseObjectInstance;
		}

		///Set the C++ base class representative pointer. We have to incRef() before.
		void setBaseObjectInstance(NLIAAGENT::IObjectIA *o)
		{
			_BaseObjectInstance = o;
		}
	};

	
	/**
	* Class CAgentClass.
	* 
	* This class is an concret class use to define an scripted agent.
	* 
	*
	* \author Chafik sameh
	* \author Portier Pierre.
	* \author Nevrax France
	* \date 2000
	*/	
	class CAgentClass: public IAgentMultiClass
	{
	public:
		static const NLIAC::CIdentType IdAgentClass;
	private:
		std::vector<CComponent *> _Components;

		typedef std::vector<CMethodeName *>	tVectorMethode;
		tVectorMethode						_Methode;
		std::vector<const CAgentClass  *>	_VTable;
		sint32 _lastRef;		
		sint32 _RunIndex;
	private:			
		NLIAAGENT::IVarName *_Inheritance;		

		
	public:

		CAgentClass(const NLIAAGENT::IVarName &);
		CAgentClass(const NLIAAGENT::IVarName &, const NLIAAGENT::IVarName &);
		CAgentClass(const CAgentClass &);
		CAgentClass(const NLIAC::CIdentType &);
		CAgentClass();
		~CAgentClass();

		virtual NLIAAGENT::IObjectIA *buildNewInstance() const;
		virtual const NLIAC::IBasicType *clone() const;
		virtual const NLIAC::IBasicType *newInstance() const;
		virtual void getDebugString(char *t) const;
		virtual void save(NLMISC::IStream &);
		virtual void load(NLMISC::IStream &);
		virtual const NLIAAGENT::IObjectIA::CProcessResult &run();
		virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;


		//// Static components
		virtual sint32 registerComponent(const NLIAAGENT::IVarName &type_name);
		virtual sint32 registerComponent(const NLIAAGENT::IVarName &type_name, NLIAAGENT::CStringVarName &field_name);
		/// Returns a CComponent struct describing a static component from its index in the class
		/// Only the class's own static members are considered, not the inherited ones.
		virtual CComponent *getComponent(sint32 i) const;
		/// Returns a CComponent struct describing a static component from its name in the class
		/// Only the class's own static members are considered, not the inherited ones.
		virtual CComponent *getComponent(const NLIAAGENT::IVarName &name) const;
		virtual sint32 getComponentIndex(const NLIAAGENT::IVarName &) const;
//		virtual sint32 getInheritedComponentIndex(const NLIAAGENT::IVarName &name) const;

		/// Returns a CComponent struct describing a static component from its name in the class
		/// Only the class's own static members are considered, not the inherited ones.
		virtual sint32 getStaticMemberIndex(const NLIAAGENT::IVarName &) const;		
		virtual const NLIAAGENT::IObjectIA *getStaticMember(sint32) const; //throw Exc::IException &
		/// returns the number of static members specific to the class (not the inherited ones)
		virtual sint32 getStaticMemberSize() const;
		sint32 getInheritedStaticMemberIndex(const NLIAAGENT::IVarName &) const;
		

		/// Build the class static components for a new instance, including inherited ones		
		virtual void createComponents(std::list<NLIAAGENT::IObjectIA *> &) const;
		/// Build the bases classes for a new instance, including inherited ones
		virtual void createBaseClassComponents(std::list<NLIAAGENT::IObjectIA *> &) const;
		///Gets the name of the components with an offset reference.
		virtual const char *getComponentName(sint32) const;
		
		/// \name Methods.
		//@{
		virtual sint32 addBrancheCode(const NLIAAGENT::IVarName &,const CParam &);
		virtual CMethodeName &getBrancheCode() const; ///throw Exc::CExceptionUnReference;
		virtual CMethodeName &getBrancheCode(sint32) const;
		virtual CMethodeName &getBrancheCode(sint32, sint32) const;
		virtual NLIAAGENT::tQueue isMember(const NLIAAGENT::IVarName *className,const NLIAAGENT::IVarName *methodName,const NLIAAGENT::IObjectIA &param) const;
		virtual sint32 getMethodIndexSize() const;		
		virtual sint32 getRunMethod() const;
		virtual void setRunMethod(sint32 index);
		//@}
				
		/// \name Builds the v_table.
		//@{
		virtual void buildVTable();
		virtual void buildVMethode();		
		virtual sint32 sizeVTable() const;
		//@}
		
		/// \name Classes.
		//@{
		virtual sint32 isClassInheritedFrom(const NLIAAGENT::IVarName &) const;
		const NLIAAGENT::IVarName *getInheritanceName() const;					/// Returns the name of the base class of this class
		void setInheritanceName(const NLIAAGENT::IVarName &name);				/// Defines the base class of this class
		virtual const IClassInterpret *getBaseClass() const;					/// Returns the base class of this class
		const CAgentClass *getSuperClass() const;								/// Returns the highest class from which this class derives
		const void getClassPath(std::vector<const CAgentClass *> &path) const;	/// Fills a vector with pointers to all base classes from superclass to this class
		sint32 getNbBaseClass() const;												/// Returns the number of base classes
		//@}

		/// Return a pointer represent the inheritance range n, the inheritance graph are represented by a vector.
		virtual const IClassInterpret *getInheritance(sint32 n) const;
		sint32 findMethod(const NLIAAGENT::IVarName &name,const CParam &CParam) const;
	};
}
#endif
