/** \file message_script.cpp
 *
 * $Id: message_script.cpp,v 1.21 2002/07/23 15:37:47 chafik Exp $
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
#include "nel/ai/agent/message_script.h"
#include "nel/ai/script/interpret_object_message.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/type_def.h"
#include "nel/ai/script/object_unknown.h"
#include "nel/ai/agent/msg.h"
#include "nel/ai/e/ai_exception.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	CMessageScript::CMessageScript(const CMessageScript &m):IMessageBase(m)
	{
		_MessageClass = m._MessageClass;
		/*if ( _MessageClass )
			_MessageClass->incRef();*/
		//CVectorGroupType *x = new CVectorGroupType(m.size());
	}
	
	CMessageScript::CMessageScript(NLAISCRIPT::CMessageClass *b) : _MessageClass( b )
	{
		/*if ( _MessageClass )
			_MessageClass->incRef();*/
	}

	CMessageScript::CMessageScript( std::list<IObjectIA *> &components, NLAISCRIPT::CMessageClass *message_class ): _MessageClass( message_class )
	{	
		/*if ( _MessageClass )
			_MessageClass->incRef();*/

		// Creates the static components array
		CVectorGroupType *x = new CVectorGroupType(components.size());
		std::list<IObjectIA *>::iterator it_c = components.begin();		
		sint32 i = 0;
		while ( it_c != components.end() )
		{			
			x->setObject(i,*it_c);
			it_c++;					
			i ++;
		}
		setMessageGroup(x);
		setGroup(CMessageGroup::msgScriptingGroup);
	}	

	CMessageScript::~CMessageScript()
	{
/*#ifdef NL_DEBUG
		const char *name = (const char *)getType();
#endif*/
		/*if ( _MessageClass )
			_MessageClass->release();		*/
	}

	sint32 CMessageScript::getMethodIndexSize() const
	{
		return IMessageBase::getMethodIndexSize() + _MessageClass->getMethodIndexSize();
	}

	sint32 CMessageScript::getBaseMethodCount() const
	{
		return IMessageBase::getMethodIndexSize();
	}


	NLAISCRIPT::IOpCode *CMessageScript::getMethode(sint32 inheritance,sint32 index)
	{
#ifdef NL_DEBUG
		if ( index >= _MessageClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}

		if ( inheritance >= _MessageClass->sizeVTable())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLAISCRIPT::IOpCode *)_MessageClass->getBrancheCode(inheritance,index).getCode();
	}

	NLAISCRIPT::IOpCode *CMessageScript::getMethode(sint32 index)
	{
#ifdef NL_DEBUG
		if ( index >= _MessageClass->getMethodIndexSize())
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (NLAISCRIPT::IOpCode *)_MessageClass->getBrancheCode(index).getCode();
	}


	IObjectIA::CProcessResult CMessageScript::runMethodeMember(sint32 inheritance, sint32 index, IObjectIA *c)
	{
		if(c->getType() != NLAISCRIPT::CCodeContext::IdCodeContext)
		{
			return IMessageBase::runMethodeMember(inheritance,index, c);
		}

		NLAISCRIPT::IOpCode *opPtr = NULL;
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;

		sint32 i = index - getBaseMethodCount();
		if(i < 0)
		{
			/*if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
				listBidon.incRef();
			}*/

			IObjectIA::CProcessResult r = IMessageBase::runMethodeMember(index,(IObjectIA *)context.Param.back());
			if(r.Result != NULL)
			{
				context.Stack++;
				context.Stack[(int)context.Stack] = r.Result;
			}
			r.Result = NULL;
			return r;
		}
		else
		{
			opPtr = getMethode( inheritance, i);
		}
		
		IObjectIA::CProcessResult r;

		if(opPtr)
		{
			NLAISCRIPT::IOpCode &op = *opPtr;
			NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
			sint32 ip = (uint32)*context.Code;
			context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
			*context.Code = 0;

			/*TProcessStatement k = IObjectIA::ProcessIdle;

			while(k != IObjectIA::ProcessEnd)
			{
				k = op.run(context);	
			}*/
			
			r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);

			// If we are in Debug Mode
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.callStackPop();
			}
			
			*context.Code = ip;
			context.Code = opTmp;
		}
		else
		{
			throw NLAIE::CExceptionUnReference("CMessageScript::runMethodeMember(sint32 , sint32, IObjectIA *) because code is null");
		}
		/*IObjectIA::CProcessResult r;
		r.Result = NULL;
		r.ResultState = k;*/
		return r;
	}

	IObjectIA::CProcessResult CMessageScript::runMethodeMember(sint32 index,IObjectIA *c)
	{
		if(c->getType() != NLAISCRIPT::CCodeContext::IdCodeContext)
		{
			return IMessageBase::runMethodeMember(index, c);
		}

		NLAISCRIPT::IOpCode *opPtr = NULL;
		NLAISCRIPT::CCodeContext &context = (NLAISCRIPT::CCodeContext &)*c;

		sint32 i = index - getBaseMethodCount();
		if(i < 0)
		{
			/*if (context.ContextDebug.Active)
			{
				context.ContextDebug.Param.push_back(&listBidon);
				listBidon.incRef();
			}*/

			IObjectIA::CProcessResult r = IMessageBase::runMethodeMember(index,(IObjectIA *)context.Param.back());
			if(r.Result != NULL)
			{
				context.Stack++;
				context.Stack[(int)context.Stack] = r.Result;
			}
			r.Result = NULL;
			return r;
		}
		else
		{
			opPtr = getMethode(i);
		}

		IObjectIA::CProcessResult r;

		if(opPtr)
		{
			NLAISCRIPT::IOpCode &op = *opPtr;
			NLAISCRIPT::CCodeBrancheRun *opTmp = context.Code;
			sint32 ip = (uint32)*context.Code;
			context.Code = (NLAISCRIPT::CCodeBrancheRun *)&op;		
			*context.Code = 0;

			/*:TProcessStatement k = IObjectIA::ProcessIdle;

			while(k != IObjectIA::ProcessEnd)
			{
				k = op.run(context);	
			}*/		

			r = ((NLAISCRIPT::ICodeBranche *)opPtr)->run(context);

			// If we are in Debug Mode
			if (context.ContextDebug.Active)
			{
				context.ContextDebug.callStackPop();
			}

			*context.Code = ip;
			context.Code = opTmp;
		}
		else
		{
			throw NLAIE::CExceptionUnReference("CMessageScript::runMethodeMember(sint32, IObjectIA *) because code is null");
		}
		/*IObjectIA::CProcessResult r;
		r.Result = NULL;
		r.ResultState = k;*/
		return r;
	}	

	sint32 CMessageScript::isClassInheritedFrom(const IVarName &class_name) const
	{
		return _MessageClass->isClassInheritedFrom( class_name );
	}	

	bool CMessageScript::setStaticMember(sint32 index,IObjectIA *op)
	{
#ifdef NL_DEBUG
		if ( index >= size() )
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		CVectorGroupType *a = (CVectorGroupType *)getMessageGroup();
		IObjectIA *b = (IObjectIA *)(*a)[ index ];
		a->setObject(index, op);
		if(op != b) b->release();
		else return true;
		return false;
	}

	const IObjectIA *CMessageScript::getStaticMember(sint32 index) const
	{
#ifdef NL_DEBUG
		if ( index >= size() )
		{
			throw NLAIE::CExceptionIndexError();
		}
#endif
		return (*this)[ index ];		
	}

	sint32 CMessageScript::getStaticMemberIndex(const IVarName &name) const
	{
		return _MessageClass->getStaticMemberIndex(name);
	}

	sint32 CMessageScript::getStaticMemberSize() const
	{
		return 0;
	}

	tQueue CMessageScript::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{	

		tQueue result = IMessageBase::isMember(className, methodName, param);
		
		if ( result.size()  )
		{
			return result;
		}

		if( *methodName == CStringVarName("send") )
		{
			tQueue r;
			NLAISCRIPT::COperandVoid typeR;
			NLAISCRIPT::CObjectUnknown *t = new NLAISCRIPT::CObjectUnknown((NLAISCRIPT::IOpType *)typeR.clone());			
			r.push(CIdMethod(0,0.0,NULL,t));
			return r;
		}

		return result;
	}

	void CMessageScript::save(NLMISC::IStream &os)
	{
		IMessageBase::save(os);
	}

	void CMessageScript::load(NLMISC::IStream &is)
	{
		IMessageBase::load(is);
	}

	const NLAIC::IBasicType *CMessageScript::clone() const
	{		
		CMessageScript *cl = new CMessageScript( *this );

		return cl;
	}

	const NLAIC::IBasicType *CMessageScript::newInstance() const
	{
		CMessageScript *instance;
		if ( _MessageClass )
		{
			instance = (CMessageScript *) _MessageClass->buildNewInstance();
		}
		else 
		{
			instance = new CMessageScript();
		}
		return instance;
	}

	void CMessageScript::getDebugString(std::string &t) const
	{
		if ( _MessageClass )
			t += NLAIC::stringGetBuild("<%s> (scripted)\n   -StaticComponents:\n",(const char *)_MessageClass->getType());
		else
			t += NLAIC::stringGetBuild("<undefined_class> (scripted)\n   -StaticComponents:\n");
		
		for (sint32 i = 0; i < size(); i++ )
		{
			std::string buf;
			t += "     ";
			if ( _MessageClass->getComponent(i) )
			{
				t += _MessageClass->getComponentName(i);
			}
			else
				t += "<unnamed>";

			t += "     ";
			(*this)[i]->getDebugString(buf);
			t += buf;
			t += "\n";

		}
	}

	bool CMessageScript::isEqual(const IBasicObjectIA &a) const
	{
		return IMessageBase::isEqual(a);
	}

	const NLAIC::CIdentType &CMessageScript::getType() const
	{
		if ( _MessageClass ) 
			return _MessageClass->getType();
		else
			return IdMessageScript;
	}	

	const IObjectIA::CProcessResult &CMessageScript::run()
	{
		return IObjectIA::ProcessRun;
	}
}
