/** \file message_script.h
 * class for script message.
 *
 * $Id: message_script.h,v 1.12 2003/01/21 11:24:25 chafik Exp $
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
#ifndef NL_MESSAGE_SCRIPT_H
#define NL_MESSAGE_SCRIPT_H

#include "nel/ai/agent/agent.h"
#include "nel/ai/script/virtual_op_code.h"
#include "nel/ai/agent/agent_manager.h"

namespace NLAISCRIPT 
{
	class CMessageClass;
}

namespace NLAIAGENT
{
	/**
		Class for managing message agent script. Note that it is write with the same model of CAgentScript but don't have the same base class, because
		message is not an agent and message must be very light. Remember agent have connection list, child list and a mail boxe, this is incompatible 
		with spirit of message.

		* \author Chafik sameh
		* \author Nevrax France
		* \date 2000
	*/
	class CMessageScript : public IMessageBase 
	{
		public:
			static const NLAIC::CIdentType IdMessageScript;
		protected:			
			NLAISCRIPT::CMessageClass *_MessageClass;

		public:
			/// \name IObjectIA member class. 
			//@{
			virtual sint32 getMethodIndexSize() const;
			virtual TQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
			virtual sint32 isClassInheritedFrom(const IVarName &) const;		
			virtual	IObjectIA::CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);
			virtual	IObjectIA::CProcessResult runMethodeMember(sint32 index,IObjectIA *);

			virtual bool setStaticMember(sint32,IObjectIA *);
			virtual const IObjectIA *getStaticMember(sint32) const;
			virtual sint32 getStaticMemberSize() const;
			virtual sint32 getStaticMemberIndex(const IVarName &) const;
			virtual const CProcessResult &run();
			//@}

			/// \name IBasicObjectIA member class. 
			//@{
			virtual bool isEqual(const IBasicObjectIA &a) const;
			//@}

			/// \name NLAIC::IBasicInterface member class. 
			//@{
			virtual void load(NLMISC::IStream &is);
			virtual void save(NLMISC::IStream &os);
			virtual const NLAIC::IBasicType *clone() const;
			virtual const NLAIC::IBasicType *newInstance() const;
			virtual void getDebugString(std::string &t) const;			
			virtual const NLAIC::CIdentType &getType() const;
			//@}			

		public:
			CMessageScript(const CMessageScript &);
			CMessageScript(NLAISCRIPT::CMessageClass *b = NULL);
			CMessageScript( std::list<IObjectIA *> &, NLAISCRIPT::CMessageClass *);

			virtual ~CMessageScript();			
				

			///get the closure correspondent of the method indexed by index in the base class inheritance.
			NLAISCRIPT::IOpCode *getMethode(sint32 inheritance,sint32 index); 
			///get the closure correspondent of the method indexed by index.
			NLAISCRIPT::IOpCode *getMethode(sint32 index);

			/// Return the nomber of internal C++ hard coded method that the class can process.
			virtual sint32 getBaseMethodCount() const;

			const NLAISCRIPT::CMessageClass *getCreatorClass() const
			{
				return _MessageClass;
			}

			virtual void setAgentManager(IAgentManager *)
			{
			}
	};
}
#endif
