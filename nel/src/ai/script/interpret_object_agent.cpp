/** \file interpret_object_agent.cpp
 *
 * $Id: interpret_object_agent.cpp,v 1.17 2001/01/18 15:04:57 portier Exp $
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
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/c/registry_class.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/script/lexsupport.h"

namespace NLAISCRIPT
{	

	const NLAIAGENT::IObjectIA::CProcessResult &CAgentClass::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

	CAgentClass::CAgentClass(const NLAIAGENT::IVarName &name) : _Components(0),_Inheritance(NULL)
	{
		setType(name, *this);	
		_lastRef = -1;
		_RunIndex = -1;
		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));

		_MsgIndirectTable = NULL;
		_NbScriptedComponents = 0;
	}

	CAgentClass::CAgentClass(const NLAIAGENT::IVarName &name, const NLAIAGENT::IVarName &base_class_name) :
		_Components(0),
		_Inheritance( (NLAIAGENT::IVarName *)base_class_name.clone() )
	{
		setType(name, *this);
		_lastRef = -1;
		_RunIndex = -1;
		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));

		_MsgIndirectTable = NULL;
		_NbScriptedComponents = 0;
	}
	
	CAgentClass::CAgentClass( const CAgentClass &a):
						_Components(a._Components),_Inheritance(a._Inheritance == NULL ? NULL : (NLAIAGENT::IVarName *)a._Inheritance->clone())
	{		
		setType(new NLAIC::CIdentType(a.getType()));
		_lastRef = -1;
		_RunIndex = -1;
		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));

		_MsgIndirectTable = NULL;
		_NbScriptedComponents = 0;
	}
	
	CAgentClass::CAgentClass(const NLAIC::CIdentType &ident):_Components(0),_Inheritance(NULL)
	{
		setType(new NLAIC::CIdentType(ident));
		_lastRef = -1;
		_RunIndex = -1;

		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));

		_MsgIndirectTable = NULL;
		_NbScriptedComponents = 0;
	}

	CAgentClass::CAgentClass():_Components(0),_Inheritance(NULL)
	{		
		_lastRef = -1;
		_RunIndex = -1;
		_Methode.size();		

		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));

		_MsgIndirectTable = NULL;
		_NbScriptedComponents = 0;
	}

	CAgentClass::~CAgentClass()
	{
		
		for(sint32 i =  0; i < (sint32)_Components.size(); i++)
		{
			CComponent *c = _Components[i];
			if(c->ObjectName) c->ObjectName->release();
			if(c->RegisterName) c->RegisterName->release();
			delete c;
		}

		if(_Methode.size())
		{		
#ifdef NL_DEBUG
	char txtClass[2048*8];
	sprintf(txtClass,getClassName()->getString());
#endif	
			sint32 j;
			for(j =  0; j < (sint32)_Methode.size(); j++)
			{				
				CMethodeName *c = _Methode[j];
#ifdef NL_DEBUG
	char txt[2048*8];
	c->getDebugString(txt);
#endif					
				c->release();				
			}

			clearIndirectMsgTable();

		}

		if(_Inheritance != NULL) 
			_Inheritance->release();

	}

	void CAgentClass::clearIndirectMsgTable()
	{
		if ( _MsgIndirectTable != NULL )
		{
			for ( int i = 0; i < (int) _Methode.size(); i++ )
				if ( _MsgIndirectTable[ i ] != NULL )
					delete[] _MsgIndirectTable[ i ];

			delete[] _MsgIndirectTable;

			_MsgIndirectTable = NULL;
		}
	}
	
	bool CAgentClass::isMessageFunc(const CParam &param) const 
	{
		int s = param.size();
		if ( param.size() == 1 )
		{
			IOpType &msg_arg = *((IOpType *)param[0]);
			const NLAIC::CIdentType &msg_type = *msg_arg.getConstraintTypeOf();
			CAgentClass *child_class = (CAgentClass *) msg_type.getFactory()->getClass();
	
			if ( child_class->isClassInheritedFrom( NLAIAGENT::CStringVarName("Message") ) != -1 )
				return true;
		}
		return false;
	}

	/// Build the table that translates an agent's message processing function index into
	/// it's child equivalent message processing function index.
	void CAgentClass::buildChildsMessageMap()
	{
		
		sint32 i, child_index, father_index;

		std::vector< std::vector<sint32> > l_index;
#ifdef _NL_DEBUG_
		const char *dbg_this_class_name = getClassName()->getString();
#endif

		clearIndirectMsgTable();

		_MsgIndirectTable = new sint32 *[ _Methode.size() ];
		for (i = 0; i < (int) _Methode.size(); i++ )
		{
			_MsgIndirectTable[i] = 0;
			l_index.push_back( std::vector<sint32>() );
		}

		for (i =0; i < (int) _Components.size() ; i++ ) // ... for each of its components ...
		{
			NLAIC::CIdentType c_type( _Components[ i ]->RegisterName->getString() );
#ifdef _NL_DEBUG_
			const char *dbg_class_name = _Components[ i ]->RegisterName->getString();
#endif
				if( ((const NLAIC::CTypeOfObject &) c_type) & NLAIC::CTypeOfObject::tAgentInterpret ) // ...if it's a scripted agent...
				{
					_NbScriptedComponents ++;
					CAgentClass *child_class = (CAgentClass *) c_type.getFactory()->getClass();
					// ... for each of its methods...
#ifdef _NL_DEBUG_
				sint32 dbg_nb_funcs = child_class->getBrancheCodeSize();
#endif
				for (child_index =0; child_index < child_class->getBrancheCodeSize(); child_index++ )
				{
					CMethodeName &method = child_class->getBrancheCode( (int) child_index );
#ifdef _NL_DEBUG_
					const char *dbg_meth_name = method.getName().getString();
#endif

#ifdef _NL_DEBUG_
					int dbg_param_size = method.getParam().size();
					char dbg_param_name [1024*8];
					method.getParam().getDebugString(dbg_param_name);
					char dbg_real_name [1024*8];
					sprintf(dbg_real_name,"%s.%s %s",dbg_class_name,dbg_meth_name,dbg_param_name);
#endif
					if ( isMessageFunc( method.getParam() ) )	// ... if it's a message processing function...
					{
						// Looks if the father has a procecessing function for this message
						sint32 father_index = findMethod( method.getName(), method.getParam() );
						if ( father_index != -1 )
						{
							// The father processes this message.
							l_index[ father_index ].push_back( child_index );
						}
						else
						{

							// Looks if the father has a procecessing function for this message
							sint32 father_index = findMethod( method.getName(), method.getParam() );
							if ( father_index != -1 )
							{
								// The father processes this message.
								l_index[ father_index ].push_back( child_index );
							}
							else
							{
								// The father doesn't process this message so we've got to pick it up in its other message list.
							}
						}
					}
				}
			}
		}

		for ( father_index = 0; father_index < (int) l_index.size(); father_index++ )
		{
			if ( ! l_index[ father_index ].empty() )
			{
				sint32 *index = new sint32[ _NbScriptedComponents ];
				for ( child_index = 0; child_index < (int) l_index[father_index].size(); child_index++ )
					index[ (int) child_index ] = (l_index[ (int) father_index ])[ (int) child_index ];
				_MsgIndirectTable[ father_index ] = index;
			}

		}
	}

	sint32 CAgentClass::getChildMessageIndex(const NLAIAGENT::IMessageBase *msg, sint32 child_index )
	{
		return _MsgIndirectTable[ msg->getMethodIndex() - getBaseMethodCount() ][child_index];
	}

	
	/// Adds a static component to an agent
	sint32 CAgentClass::registerComponent(const NLAIAGENT::IVarName &type_name)
	{			
		CComponent *c = new CComponent();
		c->RegisterName = (NLAIAGENT::IVarName *)type_name.clone();
		c->ObjectName = NULL;
		_Components.push_back(c);
		return _Components.size() - 1;
	}
	
	/// Adds a static component to an agent
	sint32 CAgentClass::registerComponent(const NLAIAGENT::IVarName &type_name, NLAIAGENT::CStringVarName &field_name)
	{			
		CComponent *c = new CComponent();
		c->RegisterName = (NLAIAGENT::IVarName *)type_name.clone();
		c->ObjectName = (NLAIAGENT::IVarName *)field_name.clone();
		_Components.push_back(c);
		return _Components.size() - 1;
	}
	

	sint32 CAgentClass::getComponentIndex(const NLAIAGENT::IVarName &name) const
	{
		for(sint32 i = _Components.size() - 1; i >= 0; i --)
		{

#ifdef _DEBUG
			char buffer[1024 * 8];
			name.getDebugString( buffer );
			char buffer2[1024 * 8];
			_Components[i]->ObjectName->getDebugString( buffer2 );
#endif
			if (_Components[i]->ObjectName !=NULL && (*_Components[i]->ObjectName) == name) 
				return i;
		}
		return -1;
	}

	CComponent *CAgentClass::getComponent(const NLAIAGENT::IVarName &name) const
	{
		for(sint32 i = _Components.size() - 1; i >= 0; i --)
		{
			if (_Components[i]->ObjectName !=NULL && *_Components[i]->ObjectName == name) 
				return _Components[i];
		}
		return NULL;
	}


	sint32 CAgentClass::getStaticMemberIndex(const NLAIAGENT::IVarName &name) const
	{
		sint32 n = 0;
		const IClassInterpret *classType = getBaseClass();
		while(classType != NULL)
		{
			n += classType->getStaticMemberSize();
			classType = classType->getBaseClass();
		}
		classType = this;
		while(classType != NULL)
		{		
			for(sint32 i = classType->getStaticMemberSize() - 1; i >= 0; i --)
			{
				if (classType->getComponent(i)->ObjectName != NULL && *classType->getComponent(i)->ObjectName == name) 
				{										
					return i + n;
				}
			}			
			classType = classType->getBaseClass();
			if(classType != NULL) n -= classType->getStaticMemberSize();
		}
		
		return -1;
	}

	sint32 CAgentClass::getInheritedStaticMemberIndex(const NLAIAGENT::IVarName &name) const
	{

#ifdef _DEBUG
		const char *dbg_this_type = (const char *) getType();
		char buffer[1024 * 8];
		name.getDebugString(buffer);
#endif


		sint32 nb_components = 0;
		std::vector<const CAgentClass *>::const_iterator it_bc = _VTable.begin();
		sint32 index;
		while ( it_bc != _VTable.end() && (  ( index = (*it_bc)->getComponentIndex( name ) ) == -1 ) )
		{
			nb_components += (*it_bc)->getStaticMemberSize();
			it_bc++;
		}

		if ( it_bc != _VTable.end() && index != -1)
			return nb_components + index;
		else
			return -1;
	}
/*
	sint32 CAgentClass::getInheritedStaticMemberIndex(const NLAIAGENT::IVarName &name) const
	{
		sint32 n = 0;
		const IClassInterpret *classType = getBaseClass();
		while(classType != NULL)
		{
			n += classType->getStaticMemberSize();
			classType = classType->getBaseClass();
		}
		classType = this;
		while(classType != NULL)
		{		
			for(sint32 i = classType->getStaticMemberSize() - 1; i >= 0; i --)
			{
				if (classType->getComponent(i)->ObjectName != NULL && *classType->getComponent(i)->ObjectName == name) 
				{										
					return i + n;
				}
			}			
			classType = classType->getBaseClass();
			if(classType != NULL) n -= classType->getStaticMemberSize();

		}
		return -1;
	}
*/
	const NLAIAGENT::IObjectIA *CAgentClass::getStaticMember(sint32 index) const
	{					
		try
		{
			/*NLAIC::CIdentType id(_Components[i]->RegisterName->getString());
			const NLAIAGENT::IObjectIA *o = (const NLAIAGENT::IObjectIA *)id.getFactory()->getClass();
			return o;*/
			sint32 n = 0;
			const IClassInterpret *classType = getBaseClass();
			while(classType != NULL)
			{
				n += classType->getStaticMemberSize();
				classType = classType->getBaseClass();
			}
			classType = this;
			while(classType != NULL)
			{		
				for(sint32 i = classType->getStaticMemberSize() - 1; i >= 0; i --)
				{
					if(index == i + n)
					{
						NLAIC::CIdentType id(classType->getComponent(i)->RegisterName->getString());
						const NLAIAGENT::IObjectIA *o = (const NLAIAGENT::IObjectIA *)id.getFactory()->getClass();
						return o;

					}
					/*if (classType->getComponent(i)->ObjectName != NULL && *classType->getComponent(i)->ObjectName == name) 
					{										
						return i + n;
					}*/
				}			
				classType = classType->getBaseClass();
				if(classType != NULL) n -= classType->getStaticMemberSize();
			}
		}				
		catch(NLAIE::IException &e)
		{
			throw NLAIE::CExceptionContainer(e);
		}
				
		return NULL;
	}

	sint32 CAgentClass::getStaticMemberSize() const
	{		
		return _Components.size();
	}

	CComponent *CAgentClass::getComponent(sint32 i) const
	{
		/*sint32 n = 0;
		const IClassInterpret *classType = getBaseClass();
		while(classType != NULL)
		{
			n += classType->getStaticMemberSize();
			classType = classType->getBaseClass();
		}
		classType = this;
		while(classType != NULL)
		{		
			for(sint32 i = classType->getStaticMemberSize() - 1; i >= 0; i --)
			{
				if (classType->getComponent(i)->ObjectName != NULL && *classType->getComponent(i)->ObjectName == name) 
				{										
					return i + n;
				}
			}			
			classType = classType->getBaseClass();
			if(classType != NULL) n -= classType->getStaticMemberSize();
		}*/


		if ( i < (sint32)_Components.size() ) 
			return _Components[i];
		else 
			return NULL;
	}

	CMethodeName &CAgentClass::getBrancheCode(sint32 i) const
	{
		return 	*_Methode[i];
	}

	sint32 CAgentClass::getBrancheCodeSize() const
	{
		return _Methode.size();
	}

	CMethodeName &CAgentClass::getBrancheCode(sint32 no_base_class, sint32 no_methode) const
	{
		return _VTable[ no_base_class ]->getBrancheCode( no_methode );
	}

	CMethodeName &CAgentClass::getBrancheCode() const
	{
		if(_lastRef < 0) throw NLAIE::CExceptionUnReference("you try to access to an unrefrence index");
		return *_Methode[_lastRef];
	}
	
	sint32 CAgentClass::getMethodIndexSize() const
	{
		return (sint32)_Methode.size() + getBaseMethodCount();
	}

	sint32 CAgentClass::addBrancheCode(const NLAIAGENT::IVarName &name,const CParam &param)
	{	
#ifdef NL_DEBUG
	char txtClass[2048*8];
	char txt[2048*8];
	param.getDebugString(txtClass);
	sprintf(txt,"%s%s",name.getString(),txtClass);
	sprintf(txtClass,getClassName()->getString());			
#endif
		sint32 i = findMethod(name,param);
		CMethodeName *m = new CMethodeName(name);
		if(i >= 0) 
		{			
			CMethodeName *oldM = _Methode[i];
			oldM->release();
			_Methode[i] = m;
			_Methode[i]->setParam(param) ;
			_lastRef = i;
		}
		else
		{

			_Methode.push_back(m);
			_Methode.back()->setParam(param);
			_lastRef = _Methode.size() - 1;
		}
		return _lastRef;
	}

	NLAIAGENT::tQueue CAgentClass::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{
		NLAIAGENT::tQueue q;
		const IClassInterpret *classType = this;
		NLAIAGENT::CIdMethod k;

		if( className != NULL )
		{
			classType = NULL;
			for(sint32 i = 1; i < (sint32)_VTable.size(); i ++)
			{
				if(*_VTable[i]->getClassName() == *className)
				{
					classType = _VTable[i];
				}
			}
		}

		if( classType != NULL )
		{		
			for(sint32 i = 0; i < getMethodIndexSize() - getBaseMethodCount(); i ++)
			{
				CMethodeName &m = classType->getBrancheCode(i);
				if(m.getName() == *methodName )
				{
					k.Weight = m.getParam().eval((const CParam &)param);
					k.Index = i + getBaseMethodCount();
					k.Method = &m;					
					IOpType *t = (IOpType *)m.getTypeOfMethode();
					t->incRef();
					if(k.ReturnType != NULL)
					{
						k.ReturnType->release();
					}
					k.ReturnType = new CObjectUnknown(t);
					if(k.Weight >= 0.0)
					{
						q.push(k);
					}
				}
			}
		}

		if( !q.size() )
		{
			return getBaseObjectInstance()->isMember(className,methodName,param);
		}
		return q;
	}

	sint32 CAgentClass::findMethod(const NLAIAGENT::IVarName &name,const CParam &param) const
	{						
		for(sint32 i = 0 ; i < (sint32)_Methode.size(); i ++)
		{			
			CMethodeName *m = _Methode[i];
			const CParam &p = (const CParam &)m->getParam();
			if(m->getName() == name && p == param) return i;
		}
		return -1;
	}

	

	void CAgentClass::createBaseClassComponents( std::list<NLAIAGENT::IObjectIA *> &comps) const
	{
#ifdef NL_DEBUG
		const char *txt = NULL;
		if(getName() != NULL) txt = getName()->getString();		
#endif				
		if ( _Inheritance )
		{
			const CAgentClass *base_class = (const CAgentClass *) getBaseClass();
			base_class->createBaseClassComponents( comps );
		}		
		createComponents( comps );
#ifdef NL_DEBUG
		sint32 i = (sint32)comps.size();
#endif		
	}

	void CAgentClass::createComponents( std::list<NLAIAGENT::IObjectIA *> &comps) const
	{
		for (sint32 i = 0; i < (sint32) _Components.size(); i++)
		{
			CComponent *comp = _Components[i];
			sint32 class_index = NLAIC::getRegistry()->getNumIdent( comp->RegisterName->getString() );

			NLAIAGENT::IObjectIA *obj = (NLAIAGENT::IObjectIA *) NLAIC::getRegistry()->createInstance( class_index );
			comps.push_back( obj );
		}
	}

	void CAgentClass::buildVTable()
	{		
		_VTable.clear();
		getClassPath(_VTable);
					
		buildVMethode();
	}

	void CAgentClass::buildVMethode()
	{
#ifdef NL_DEBUG
	char txtClass[2048*8];
	sprintf(txtClass,getClassName()->getString());
#endif					
		if(sizeVTable() > 1)
		{	
			const IClassInterpret *t= _VTable[sizeVTable() - 2];
			if(t->getMethodIndexSize() - getBaseMethodCount()) _Methode.resize(t->getMethodIndexSize() - getBaseMethodCount());
			
			for(sint32 i = 0; i < t->getMethodIndexSize() - getBaseMethodCount(); i ++)
			{
				CMethodeName *m = &t->getBrancheCode(i);
#ifdef NL_DEBUG
	char txt[2048*8];
	m->getDebugString(txt);	
#endif
				m->incRef();
				_Methode[i] = m;
			}
		}		
	}	

	sint32 CAgentClass::isClassInheritedFrom(const NLAIAGENT::IVarName &className) const
	{		
		for(sint32 i = 0; i < (sint32)_VTable.size(); i ++)
		{
#ifdef NL_DEBUG
		const NLAIAGENT::IObjectIA *o = _VTable[i];
#endif
			const NLAIAGENT::IVarName *thisName = _VTable[i]->getClassName();			
			if(thisName == NULL) 
			{
				//thisName = 
				const NLAIC::CIdentType *id = &_VTable[i]->getType();
				if(id == NULL)
				{
					if(getClassName() != NULL)
					{
						if(*getClassName() == className) return i;
						else return -1;
					}
					else return -1;
				}
				else
				if(strcmp((const char *)*id , className.getString()) == 0)
				{
					return i;
				}
			}
			else
			{
				if(*(_VTable[i]->getClassName()) == className)
				{
					return i;
				}
			}
		}		
		return -1;
	}

	const IClassInterpret *CAgentClass::getInheritance(sint32 n) const
	{
		return _VTable[n];
	}

	sint32 CAgentClass::sizeVTable() const
	{
		return _VTable.size();
	}

	NLAIAGENT::IObjectIA *CAgentClass::buildNewInstance() const
	{
		// Création des composants statiques
		std::list<NLAIAGENT::IObjectIA *> components;

		// Composants des classes de base
		createBaseClassComponents( components );

		// Composants propres
		//createComponents( components );		

		// Création de l'agent
		NLAIAGENT::CAgentScript *instance = new NLAIAGENT::CAgentScript(NULL, NULL, components,  (CAgentClass *) this );

		return instance;
	}
	
	const NLAIC::IBasicType *CAgentClass::clone() const
	{
		NLAIC::IBasicType *x = new CAgentClass(*this);
		return x;
	}

	const NLAIC::IBasicType *CAgentClass::newInstance() const
	{
		NLAIC::IBasicType *x = new CAgentClass();
		return x;
	}

	void CAgentClass::getDebugString(char *t) const
	{
		sprintf(t,"<CAgentClass> %s\n", getClassName()->getString() );
	}

	void CAgentClass::save(NLMISC::IStream &os)
	{
		// Saves static components
		sint32 size = _Components.size();
		os.serial( size );
		sint32 i;
		for ( i = 0; i < (sint32) _Components.size() ; i++ )
		{
			_Components[i]->save( os );
		}
		
		// Saves class methods
		size = _Methode.size();
		os.serial( size );
		for ( i = 0; i < (sint32) _Methode.size(); i++)
		{
			os.serial( (NLAIC::CIdentType &)_Methode[i]->getType() );
			_Methode[i]->save( os );
		}
		os.serial( (NLAIC::CIdentType &) _Inheritance->getType() );
		_Inheritance->save( os );
	}

	void CAgentClass::load(NLMISC::IStream &is)
	{
		// Saves static components
		sint32 _NbComponents;
		is.serial( _NbComponents );
		sint32 i;
		for ( i = 0; i < (sint32) _NbComponents ; i++ )
		{
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			CComponent *comp = (CComponent *)id.allocClass();
			comp->load(is);
			_Components.push_back( comp );
		}

		for ( i = 0; i < (sint32) _Methode.size(); i++)
		{
			delete _Methode[i];
		}
		_Methode.clear();

		// Loads class methods
		sint32 nb_methods;
		is.serial( nb_methods );
		for ( i = 0; i < (sint32) nb_methods; i++)
		{
			NLAIC::CIdentTypeAlloc id;
			is.serial( id );
			CMethodeName *methode = (CMethodeName *)id.allocClass();
			methode->load(is);
			methode->incRef();
			_Methode.push_back( methode );
		}

		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		_Inheritance = (NLAIAGENT::IVarName *) id.allocClass();
		_Inheritance->load( is );
		_Inheritance->incRef();
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CAgentClass::run();
	
	bool CAgentClass::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		const CAgentClass &i = (const CAgentClass &)a;			
		return getClassName() == i.getClassName();
	}

	const NLAIAGENT::IVarName *CAgentClass::getInheritanceName() const
	{
		return _Inheritance;
	}
	
	void CAgentClass::setInheritanceName(const NLAIAGENT::IVarName &name)
	{
		if(_Inheritance != NULL)
		{
			_Inheritance->release();				
		}
		_Inheritance = (NLAIAGENT::IVarName *)name.clone();
		
	}

	const IClassInterpret *CAgentClass::getBaseClass() const
	{
		if ( _Inheritance )
		{
			const IClassInterpret *base_class = (const IClassInterpret *)( (CClassInterpretFactory *) NLAIC::getRegistry()->getFactory( _Inheritance->getString() ) )->getClass();
			return base_class;
		}
		else
			return NULL;
	}

	// Returns the highest class in the class hiérarchie (SuperClass)
	const CAgentClass *CAgentClass::getSuperClass() const
	{
		const CAgentClass *base_class = this;
		
		while ( base_class->getBaseClass() )
		{
			base_class = (CAgentClass *) base_class->getBaseClass();
		}
		return base_class;
	}

	/// Builds a vector with the path from the super class to this class
	const void CAgentClass::getClassPath(std::vector<const CAgentClass *> &path) const
	{
		const CAgentClass *base_class = (CAgentClass *) getBaseClass();
#ifdef NL_DEBUG
		const char *txt = NULL;
		if(getName() != NULL) txt = getName()->getString();
		else if(getName() != NULL) txt = base_class->getName()->getString();
#endif		
		if ( base_class /*&& !(base_class->getType() == IdAgentClass)*/)
		{
			base_class->getClassPath( path );
		}
		path.push_back( this );
	}

	/// Returns the number of base classes (the distance to the super class)
	sint32 CAgentClass::getNbBaseClass() const
	{
		sint32 dist = 0;
		const CAgentClass *base_class = this;
		while ( base_class->getBaseClass() )
		{
			base_class = (CAgentClass *) base_class->getBaseClass();
			dist++;
		}
		return dist;
	}

	const char *CAgentClass::getComponentName(sint32 i) const
	{
		sint32 nb_components = 0;
		std::vector<const CAgentClass *>::const_iterator it_bc = _VTable.begin();
		while ( it_bc != _VTable.end() && nb_components <= i )
		{
			nb_components = nb_components + (*it_bc)->getStaticMemberSize();
			it_bc++;
		}
		it_bc--;
		CComponent *component = (*it_bc)->getComponent( i - ( nb_components - (*it_bc)->getStaticMemberSize() ) );
		return component->ObjectName->getString();
	}

	sint32 CAgentClass::getRunMethod() const
	{
		return _RunIndex;
	}

	void CAgentClass::setRunMethod(sint32 index)
	{
		_RunIndex = index + getBaseMethodCount();
	}
}