/** \file comp_handle.h
 *	
 *	Scripted actors	
 *
 * $Id: comp_handle.h,v 1.7 2002/08/01 14:17:54 chafik Exp $
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

#ifndef NL_COMP_HANDLE_H_
#define NL_COMP_HANDLE_H_

#include "nel/ai/agent/agent.h"
//#include "nel/ai/agent/agent_script.h"
//#include "nel/ai/agent/agent_manager.h"
#include "nel/ai/logic/bool_cond.h"
#include "nel/ai/script/type_def.h"
//#include "nel/ai/script/interpret_actor.h"

namespace NLAIAGENT
{
	class CComponentHandle/* : public IObjectIA */{
		private:
			IVarName		*_CompName;
			const IObjectIA	*_Comp;
			IAgent			*_CompFather;

		public:
			CComponentHandle()
			{
				_Comp = NULL;
				_CompFather = NULL;
			}

			~CComponentHandle()
			{
				if(_CompName != NULL) _CompName->release();
//				if ( _Comp )
//					_Comp->release();
			}


			CComponentHandle(const IVarName &comp_name, IAgent *comp_father , bool get = false)
			{
				_CompName = (IVarName *) comp_name.clone();
				_CompFather = comp_father;
				if ( get )
					getComponent();
				else
					_Comp = NULL;
			}

			void getComponent()
			{
				if ( _CompFather != NULL )
				{

#ifdef _DEBUG
					const char *dbg_father_type = (const char *) _CompFather->getType();
					const char *dbg_comp_name = (const char *) _CompName->getType();
					std::string buffer;
					_CompName->getDebugString(buffer);
#endif

					// Looks in static components
					sint32 comp_id = _CompFather->getStaticMemberIndex( *_CompName );
					if ( comp_id >= 0)
						_Comp = _CompFather->getStaticMember( comp_id );
					else
						_Comp = NULL;

					if ( _Comp == NULL )
					{
						// Looks in dynamic component
						CGroupType *param = new CGroupType();
						param->push( (IObjectIA *) new CStringType(*_CompName) );
						IObjectIA::CProcessResult comp = ( (CAgentScript *) _CompFather)->getDynamicAgent(param);
						//param->pop();
						delete param;
						NLAIAGENT::IBaseGroupType *result = (NLAIAGENT::IBaseGroupType *) comp.Result;
						if ( result->size() >  0 )
						{
							_Comp = result->get();
							((IObjectIA *)_Comp)->incRef();							
						}
						else
							_Comp = NULL;

						if(result != NULL) 
									result->release();
					}
				}
			}

			const IObjectIA *getValue()
			{
				if ( _Comp )
					return _Comp;
				
				getComponent();
				return _Comp;
			}

			const IVarName *getCompName()
			{
				return _CompName;
			}
	};
} // NLAIAGENT

#endif // NL_COMP_HANDLE_H_
