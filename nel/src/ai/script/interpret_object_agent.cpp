/** \file interpret_object_agent.cpp
 *
 * $Id: interpret_object_agent.cpp,v 1.52 2002/09/18 09:44:05 portier Exp $
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
#include "nel/ai/script/libcode.h"

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
		_ConstructorIndex = -1;
		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));
		_Base_class = NULL;
		_ConstructorIndex = -1;
	}

	CAgentClass::CAgentClass(const NLAIAGENT::IVarName &name, const NLAIAGENT::IVarName &base_class_name) :
		_Components(0),
		_Inheritance( (NLAIAGENT::IVarName *)base_class_name.clone() )
	{
		setType(name, *this);
		_lastRef = -1;
		_RunIndex = -1;
		_ConstructorIndex = -1;
		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));
		_Base_class = NULL;
		_ConstructorIndex = -1;
	}
	
	CAgentClass::CAgentClass( const CAgentClass &a):
						_Components(a._Components),_Inheritance(a._Inheritance == NULL ? NULL : (NLAIAGENT::IVarName *)a._Inheritance->clone())
	{		
		setType(new NLAIC::CIdentType(a.getType()));
		_lastRef = -1;
		_RunIndex = -1;
		_ConstructorIndex = -1;
		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));
		_Base_class = a._Base_class;
		_ConstructorIndex = a._ConstructorIndex;
	}
	
	CAgentClass::CAgentClass(const NLAIC::CIdentType &ident):_Components(0),_Inheritance(NULL)
	{
		setType(new NLAIC::CIdentType(ident));
		_lastRef = -1;
		_RunIndex = -1;
		_ConstructorIndex = -1;

		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));
		_Base_class = NULL;
		_ConstructorIndex = -1;
	}

	CAgentClass::CAgentClass():_Components(0),_Inheritance(NULL)
	{		
		_lastRef = -1;
		_RunIndex = -1;
		_ConstructorIndex = -1;

		_Methode.size();		

		setBaseMethodCount(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass()))->getBaseMethodCount());
		setBaseObjectInstance(((NLAIAGENT::CAgentScript *)(NLAIAGENT::CAgentScript::IdAgentScript.getFactory()->getClass())));
		_Base_class = NULL;
		_ConstructorIndex = -1;
	}

	CAgentClass::~CAgentClass()
	{
		sint32 i;
		for(i =  0; i < (sint32)_Components.size(); i++)
		{
			CComponent *c = _Components[i];
			if(c->ObjectName) c->ObjectName->release();
			if(c->RegisterName) c->RegisterName->release();
			delete c;
		}

		for(i = 0; i < (sint32) _StaticComponents.size(); i++ )
			_StaticComponents[i]->release();

		clearIndirectMsgTable();

		if(_Inheritance != NULL) 
			_Inheritance->release();

	}

	void CAgentClass::clearIndirectMsgTable()
	{
		for (int i = 0; i < (int) _MsgIndirectTable.size(); i++ )
			if ( _MsgIndirectTable[i] != NULL )
				delete[] _MsgIndirectTable[i];
		_MsgIndirectTable.clear();
	}
	
	bool CAgentClass::isMessageFunc(const CParam &param) const 
	{
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
		
		sint32 i,j;
		sint32 child_index;
		sint32 nb_scripted_components = 0;

#ifdef NL_DEBUG
		const char *dbg_this_class_name = getClassName()->getString();
#endif
		clearIndirectMsgTable();

		/// Builds components included inherited ones
		std::vector<CComponent *> components;
		if ( _VTable.size() )
		{
			for ( i = 0; i < (int) _VTable.size(); i++ )
			{
				for ( j = 0; j < (int) _VTable[i]->_Components.size(); j++ )
				{
					components.push_back( _VTable[i]->_Components[j] );
				}
			}
		}


		/// Counts the number of scripted components
		for (i =0; i < (int) components.size() ; i++ ) // ... for each of its components ...
		{
			NLAIC::CIdentType c_type( components[ i ]->RegisterName->getString() );
			if( ((const NLAIC::CTypeOfObject &) c_type) & NLAIC::CTypeOfObject::tInterpret ) // ...if it's a scripted agent...
				nb_scripted_components ++;
		}

		// For each message processing function of the father, 
		// allocates the table with by default -1, which means the child doesn't process the
		// message.
		for ( i = 0; i < (int) _Methode.size(); i++ )
		{
			CMethodeName &method = getBrancheCode( (int) i );
			if ( isMessageFunc( method.getParam() ) )
			{
				_MsgIndirectTable.push_back( new sint32[nb_scripted_components ] );
				for ( child_index = 0; child_index < nb_scripted_components; child_index++ )
					_MsgIndirectTable[i][child_index] = -1;
			}
			else
				_MsgIndirectTable.push_back( NULL );
		}
		
		sint32 index_component = 0;
			
		for (i =0; i < (int) components.size() ; i++ ) // ... for each of its components ...
		{
			NLAIC::CIdentType c_type( components[ i ]->RegisterName->getString() );
#ifdef NL_DEBUG
			const char *dbg_class_name = components[ i ]->RegisterName->getString();
#endif
			if( ((const NLAIC::CTypeOfObject &) c_type) & NLAIC::CTypeOfObject::tInterpret ) // ...if it's a scripted agent...
			{
				CAgentClass *child_class = (CAgentClass *) c_type.getFactory()->getClass();
#ifdef NL_DEBUG
				sint32 dbg_nb_funcs = child_class->getBrancheCodeSize();
#endif
				for (child_index =0; child_index < child_class->getBrancheCodeSize(); child_index++ ) // ... for each of its methods...
				{
					CMethodeName &method = child_class->getBrancheCode( (int) child_index );
#ifdef NL_DEBUG
					const char *dbg_meth_name = method.getName().getString();
#endif
					if ( isMessageFunc( method.getParam() ) )	// ... if it's a message processing function...
					{
						// Looks if the father has a procecessing function for this message
						sint32 father_index = findMethod( method.getName(), method.getParam() );
						if ( father_index != -1 )
						{
							// The father processes this message. Puts the index for the child in the table.
							_MsgIndirectTable[ father_index ][ index_component ] = child_index;
						}
						else
						{
							// Ajoute la méthode chez le père
							father_index = addBrancheCode( method.getName(), method.getParam() );
							_Methode[ father_index ].Method->setCode((IOpCode *)NULL);
							_Methode[ father_index ].Method->setTypeOfMethode( new NLAISCRIPT::COperandVoid() );

						
							// Créée le tableau
							if ( father_index >= (int) _MsgIndirectTable.size() )
							{
								_MsgIndirectTable.push_back( new sint32[ nb_scripted_components ] );
								sint32 x;
								for ( x =0; x < nb_scripted_components; x++) 
									_MsgIndirectTable[ father_index ][x] = -1;
							}
							_MsgIndirectTable[ father_index ] [ index_component ] = child_index;
						}
					}
				}
				index_component++;
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
/*#ifdef NL_DEBUG
		std::string dbugS;
		type_name.getDebugString(dbugS);
		NLAIC::Out("registerComponent<%s>\n", dbugS.c_str());
#endif*/
		CComponent *c = new CComponent();
		c->RegisterName = (NLAIAGENT::IVarName *)type_name.clone();
		c->ObjectName = NULL;
		_Components.push_back(c);
		return _Components.size() - 1;
	}
	 
	/// Adds a static component to an agent
	sint32 CAgentClass::registerComponent(const NLAIAGENT::IVarName &type_name, const NLAIAGENT::CStringVarName &field_name)
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

#ifdef NL_DEBUG
			std::string buffer;
			name.getDebugString( buffer );
			std::string buffer2;
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

#ifdef NL_DEBUG
		const char *dbg_this_type = (const char *) getType();
		std::string buffer;
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
			throw NLAIE::CExceptionContainer(e.what());
		}
				
		return NULL;
	}

	sint32 CAgentClass::getStaticMemberSize() const
	{		
		return _Components.size();
	}

	CComponent *CAgentClass::getComponent(sint32 i) const
	{
		if ( i < (sint32) _Components.size() ) 
			return _Components[ i ];
		else 
			return NULL;
	}

	CMethodeName &CAgentClass::getBrancheCode(sint32 i) const
	{
#ifdef NL_DEBUG
		sint kkk = _Methode.size();
#endif
		CMethodeName *a = _Methode[i].Method;
		return 	*a;
	}

	sint32 CAgentClass::getBrancheCodeSize() const
	{
		return _Methode.size();
	}

	CMethodeName &CAgentClass::getBrancheCode(sint32 no_base_class, sint32 no_methode) const
	{
#ifdef NL_DEBUG
		const NLAIAGENT::IObjectIA *o = _VTable[ no_base_class ];
#endif
		
		return _VTable[ no_base_class ]->getBrancheCode( no_methode );
	}

	CMethodeName &CAgentClass::getBrancheCode() const
	{
		if(_lastRef < 0) throw NLAIE::CExceptionUnReference("you try to access to an unrefrence index");
		return *_Methode[_lastRef].Method;
	}
	
	sint32 CAgentClass::getMethodIndexSize() const
	{
		return (sint32)_Methode.size() + getBaseMethodCount();
	}

	sint32 CAgentClass::addBrancheCode(const NLAIAGENT::IVarName &name,const CParam &param)
	{	
#ifdef NL_DEBUG
	std::string txtClass;
	std::string txt;
	param.getDebugString(txtClass);
	txt = name.getString() + txtClass;
	txtClass = getClassName()->getString();
#endif
		sint32 i = findMethod(name,param);		
		if(i >= 0) 
		{			
			CMethodeName *oldM = _Methode[ i ].Method;
			if(_Methode[ i ].isBasedOnBaseClass())
			{
				CMethodeName *m = new CMethodeName(name);
				_Methode[ i ].setMethodBasedOnBaseClassState(false);
				oldM->release();
				_Methode[i] = m;
				m->setParam( param ) ;
				_lastRef = i;
			}
			else
			{				

				std::string txtP;
				std::string txt;
				param.getDebugString(txtP);
				txt = NLAIC::stringGetBuild("%s%s is all ready defined in '%s'",name.getString(),txtP.c_str(),getClassName()->getString());				
				throw NLAIE::CExceptionAllReadyExist((char *)txt.c_str());
			}
		}
		else
		{
			CMethodeName *m = new CMethodeName(name);
			_Methode.push_back( CMethodType( m ));
			m->setParam( param );
			_lastRef = _Methode.size() - 1;
			_Methode.back().setMethodBasedOnBaseClassState(false);
		}

		static NLAIAGENT::CStringVarName constructor_name("Constructor");
		if ( name == constructor_name )
			_ConstructorIndex = _lastRef;

		return _lastRef;
	}

	NLAIAGENT::tQueue CAgentClass::getPrivateMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{
		NLAIAGENT::tQueue q;
		const IClassInterpret *classType = this;
		NLAIAGENT::CIdMethod k;

		for(sint32 i = 0; i < getMethodIndexSize() - getBaseMethodCount(); i ++)
		{
			CMethodeName &m = classType->getBrancheCode(i);
#ifdef NL_DEBUG
			const char *dbg_this_name = m.getName().getString();
			const char *dbg_func_name = methodName->getString();
#endif
			if(m.getName() == *methodName )
			{
				k.Weight = m.getParam().eval((const CParam &)param);
				if(k.Weight < 0.0) continue;
				k.Index = i + getBaseMethodCount();
				k.Method = &m;					
				IOpType *t = (IOpType *)m.getTypeOfMethode();
				t->incRef();

				if(k.ReturnType != NULL)
				{
					k.ReturnType->release();
				}

				k.ReturnType = new CObjectUnknown(t);					
				q.push(k);					
			}
		}
		return q;
	}

	NLAIAGENT::tQueue CAgentClass::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{

#ifdef NL_DEBUG
		if ( className != NULL )
			const char *dbg_class_name = className->getString();
		const char *dbg_method_name = methodName->getString();
#endif

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
			q= getPrivateMember(className,methodName,param);			
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
#ifdef NL_DEBUG
			const char *dbg_method_name = _Methode[i].Method->getName().getString();	
#endif²
			CMethodeName *m = _Methode[i].Method;
			const CParam &p = (const CParam &)m->getParam();
			if( m->getName() == name && p == param ) 
				return i;
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
		NLAIAGENT::IObjectIA *obj;
		for (sint32 i = 0; i < (sint32) _Components.size(); i++)
		{
			CComponent *comp = _Components[i];
			if ( !comp->Static )
			{
				//sint32 class_index = NLAIC::getRegistry()->getNumIdent( comp->RegisterName->getString() );
				NLAIC::CIdentType id(comp->RegisterName->getString());
				sint class_index = id.getIndex();
				obj = (NLAIAGENT::IObjectIA *) NLAIC::getRegistry()->createInstance( class_index );
			}
			else
			{
#ifdef NL_DEBUG
				std::string comp_name;
				comp->RegisterName->getDebugString( comp_name );

				std::string comp_type;
				comp->ObjectName->getDebugString( comp_type );

				std::string buf;
				comp->StaticValue->getDebugString(buf);
#endif
				obj = comp->StaticValue;
				comp->StaticValue->incRef();
			}
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
	strcpy(txtClass,getClassName()->getString());
#endif					
		if(sizeVTable() > 1 )
		{	
			const IClassInterpret *t= _VTable[sizeVTable() - 2];
			
			if(t->getMethodIndexSize() > getBaseMethodCount()) 
			{
				_Methode.resize(t->getMethodIndexSize() - getBaseMethodCount());			
			
				int mmax = t->getMethodIndexSize() - getBaseMethodCount();
				for(sint32 i = 0; i < mmax; i ++)
				{
					CMethodeName *m = &t->getBrancheCode(i);
#ifdef NL_DEBUG
	std::string txt;
	m->getDebugString(txt);	
#endif
					m->incRef();
					_Methode[i] = m;
				}
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

		// Création de l'agent
		NLAIAGENT::CAgentScript *instance = new NLAIAGENT::CAgentScript(NULL, NULL, components,  (CAgentClass *) this );


		// TODO: add constructor call here!!!!!

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

	void CAgentClass::getDebugString(std::string &t) const
	{
		t += NLAIC::stringGetBuild("<CAgentClass> %s\n", getClassName()->getString() );
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
			os.serial( (NLAIC::CIdentType &)_Methode[i].Method->getType() );
			_Methode[i].Method->save( os );
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
			_Methode[i].Method->release();
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
			_Methode.push_back( CMethodType(methode));
		}

		NLAIC::CIdentTypeAlloc id;
		is.serial( id );
		_Inheritance = (NLAIAGENT::IVarName *) id.allocClass();
		_Inheritance->load( is );
		_Inheritance->incRef();
	}
	
	
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

	const IClassInterpret *CAgentClass::getComputeBaseClass()
	{
		if ( _Inheritance )
		{
			if(_Base_class == NULL)
			{
				_Base_class = (IClassInterpret *)( (CClassInterpretFactory *) NLAIC::getRegistry()->getFactory( _Inheritance->getString() ) )->getClass();				
				return _Base_class;
			}
			else
			{
				return _Base_class;
			}
		}
		else
		{			
			return NULL;
		}
	}

	const IClassInterpret *CAgentClass::getBaseClass() const
	{
		if ( _Inheritance )
		{
			if(_Base_class == NULL)
			{
				return (const IClassInterpret *)( (CClassInterpretFactory *) NLAIC::getRegistry()->getFactory( _Inheritance->getString() ) )->getClass();				
			}
			else
			{
				return _Base_class;
			}
		}
		else
			return NULL;
	}

	// Returns the highest class in the class hiérarchy (SuperClass)
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

	sint32 CAgentClass::getConstroctorMethod() const
	{
		return _ConstructorIndex;
	}

	void CAgentClass::setConstroctorMethod(sint32 index)
	{
		_ConstructorIndex = index + getBaseMethodCount();
	}

	void CAgentClass::initStatics()
	{	
		NLAIAGENT::CStringVarName staticinit_func_name("StaticInit");
		sint32 id_func = findMethod( staticinit_func_name, NLAISCRIPT::CParam() );
		if ( id_func != -1 )
		{	
			NLAISCRIPT::CStackPointer stack;
			NLAISCRIPT::CStackPointer heap;
			NLAISCRIPT::CCodeContext codeContext(stack,heap,NULL,this, NLAISCRIPT::CCallPrint::inputOutput);
			codeContext.Self = this;
			NLAISCRIPT::CCodeBrancheRun *o = (NLAISCRIPT::CCodeBrancheRun *)getBrancheCode( id_func ).getCode();
			codeContext.Code = o;
			o->run(codeContext);
		}
	}

	bool CAgentClass::setStaticMember(sint32 index, NLAIAGENT::IObjectIA *obj)
	{
#ifdef NL_DEBUG
		std::string buf;
		obj->getDebugString(buf);
#endif
		sint32 nb_components = 0;
		std::vector<const CAgentClass *>::const_iterator it_bc = _VTable.begin();
		while ( it_bc != _VTable.end() && nb_components <= index )
		{
			nb_components = nb_components + (*it_bc)->getStaticMemberSize();
			it_bc++;
		}
		it_bc--;
		CComponent *component = (*it_bc)->getComponent( index - ( nb_components - (*it_bc)->getStaticMemberSize() ) );
#ifdef NL_DEBUG
		std::string buf2, buf3;
		component->RegisterName->getDebugString(buf2);
		component->ObjectName->getDebugString(buf3);
#endif

		if(component->StaticValue != obj ) component->StaticValue = obj;
			return false;
		return true;
	}

	void CAgentClass::updateStaticMember(sint32 index, NLAIAGENT::IObjectIA *obj)
	{
#ifdef NL_DEBUG
		std::string buf;
		obj->getDebugString(buf);
#endif
		sint32 nb_components = 0;
		std::vector<const CAgentClass *>::const_iterator it_bc = _VTable.begin();
		while ( it_bc != _VTable.end() && nb_components <= index )
		{
			nb_components = nb_components + (*it_bc)->getStaticMemberSize();
			it_bc++;
		}
		it_bc--;
		CComponent *component = (*it_bc)->getComponent( index - ( nb_components - (*it_bc)->getStaticMemberSize() ) );
#ifdef NL_DEBUG
		std::string buf2, buf3;
		component->RegisterName->getDebugString(buf2);
		component->ObjectName->getDebugString(buf3);
#endif

		(*component->StaticValue) = *obj;
	}

	NLAIAGENT::IObjectIA *CAgentClass::getStaticComponentValue(std::string &c_name)
	{
		CComponent *component = getComponent( NLAIAGENT::CStringVarName( c_name.c_str() ) );
		return component->StaticValue;
	}
}
