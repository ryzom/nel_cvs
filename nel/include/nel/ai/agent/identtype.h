/** \file ident.h
 * Sevral class for identification an objects fonctionality.
 *
 * $Id: identtype.h,v 1.1 2001/03/21 14:59:34 chafik Exp $
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


#ifndef NL_IDENTTYPE_H
#define NL_IDENTTYPE_H
#include <stdlib.h>
#include "nel/misc/file.h"
#include "nel/ai/e/ai_exception.h" 
#include "nel/ai/agent/agent_string.h"

namespace NLAIAGENT
{	

	/**
	Complete reference of an agent. 
	Name of the class of the type of object.
	Ident of the agent.
	*/
	struct CIdent: public CAgentNumber
	{
		std::string TypeName;
		CIdent()
		{			
		}		

		CIdent(const std::string &typeName,uint64 id,uint64 creator,uint64 dyn):CAgentNumber(id,creator,dyn),TypeName(typeName)
		{			
		}

		CIdent(const CIdent &a):CAgentNumber(a),TypeName(a.TypeName)
		{			
		}

		///fill from read stream.
		CIdent(NLMISC::IStream &is):CAgentNumber(is)
		{			
			std::string s;
			is.serial(s);
			TypeName = s;
		}

		///saving the nomber in an output stream.
		virtual void save(NLMISC::IStream &os)
		{	
			CAgentNumber::save(os);
			os.serial(TypeName);
		}

		///loading the nomber from an input stream.
		virtual void load(NLMISC::IStream &is)
		{
			CAgentNumber::load(is);
			std::string s;
			is.serial(s);
			TypeName = s;
		}

		const std::string &getTypeName() const
		{
			return TypeName;
		}
	};
}
#endif
