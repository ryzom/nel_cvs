/** \file object_ident.h
 * class for identification an objects fonctionality.
 *
 * $Id: object_ident.h,v 1.6 2003/01/27 16:54:43 chafik Exp $
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


#ifndef NL_OBJECT_IDENT_H
#define NL_OBJECT_IDENT_H
#include <stdlib.h>
#include "nel/misc/file.h"
#include "nel/ai/e/ai_exception.h" 
#include "nel/ai/agent/ident.h"
#include "nel/ai/agent/agent_object.h"

namespace NLAIAGENT
{
	/**
	This class store an CNumericIndex, it allow user to use the ident CNumericIndex as an agnet object.
	*/
	class CObjectIdent : public IObjectIA
	{
	public:
		static const NLAIC::CIdentType *IdObjectIdent;
	private:
		CNumericIndex _Id;
	public:
		CObjectIdent(const CObjectIdent &o) : _Id(o._Id)
		{
		}

		CObjectIdent(const CNumericIndex &id):_Id(id)
		{
		}
	
		CObjectIdent(const char *id):_Id(id)
		{
		}

		CObjectIdent(const CStringType &id):_Id(id.getStr().getString())
		{
		}

		CObjectIdent(NLMISC::IStream &is):_Id(is)
		{
		}

		virtual ~CObjectIdent()
		{
		}

		/**
		Get the CNumericIndex ident of an agent.
		*/
		const CNumericIndex &getId() const
		{
			return _Id;
		}
		
		void getDebugString(std::string &text) const
		{
			_Id.getDebugString(text);
		}

		const NLAIC::IBasicType *clone() const
		{			
			return new CObjectIdent(*this);
		}		

		const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		
		const NLAIC::CIdentType &getType() const
		{
			return *IdObjectIdent;
		}

		void save(NLMISC::IStream &os)
		{						
			_Id.save(os);
		}

		void load(NLMISC::IStream &is)
		{						
			_Id.load(is);			
		}
		
		bool isEqual(const IBasicObjectIA &a) const
		{
			const CObjectIdent &t = (const CObjectIdent &)a;
			return t._Id == _Id;
		}
		
		const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

	};
}
#endif
