/** \file fuzzy_script.h
 * Interpret class for the fuzzy controler
 *
 * $Id: fuzzy_script.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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


#ifndef NL_FUZZY_SCRIPT_H
#define NL_FUZZY_SCRIPT_H

#include "agent/agent.h"

namespace NLIASCRIPT 
{
	class CMessageClass;
	class CFuzzyControlerClass;
	class IOpCode;
}

namespace NLIAFUZZY
{
	class CFuzzyControlerScript : public NLIAAGENT::IMessageBase {
		private:
			NLIASCRIPT::CMessageClass *_MessageClass;
 
		public:

			virtual sint32 getMethodIndexSize() const;
			virtual NLIAAGENT::tQueue isMember(const NLIAAGENT::IVarName *,const NLIAAGENT::IVarName *,const NLIAAGENT::IObjectIA &) const;
			virtual sint32 isClassInheritedFrom(const NLIAAGENT::IVarName &) const;		
			virtual	IObjectIA::CProcessResult runMethodeMember(sint32, sint32, NLIAAGENT::IObjectIA *);
			virtual	IObjectIA::CProcessResult runMethodeMember(sint32 index, NLIAAGENT::IObjectIA *);

			virtual void setStaticMember(sint32,NLIAAGENT::IObjectIA *);
			virtual const NLIAAGENT::IObjectIA *getStaticMember(sint32) const;
			virtual sint32 getStaticMemberSize() const;
			virtual sint32 getStaticMemberIndex(const NLIAAGENT::IVarName &) const;

			sint32 getBaseMethodCount() const;

		public:
			static const NLIAC::CIdentType IdFuzzyControlerScript;

			CFuzzyControlerScript(const CFuzzyControlerScript &);
			CFuzzyControlerScript();
			CFuzzyControlerScript( std::list<NLIAAGENT::IObjectIA *> &, NLIASCRIPT::CFuzzyControlerClass *);

			virtual ~CFuzzyControlerScript();
			virtual void load(NLMISC::IStream &is);
			virtual void save(NLMISC::IStream &os);
	
			const CProcessResult &run();

			NLIASCRIPT::IOpCode &getMethode(sint32 inheritance,sint32 index); 
			NLIASCRIPT::IOpCode &getMethode(sint32 index);

			virtual const NLIAC::IBasicType *clone() const;
			virtual const NLIAC::IBasicType *newInstance() const;
			virtual void getDebugString(char *t) const;
			virtual bool isEqual(const NLIAAGENT::IBasicObjectIA &a) const;
			static const NLIAC::CIdentType IdMessageScript;			
			const NLIAC::CIdentType &getType() const;
	};
}

#endif
