/** \file volatil_memory.h
 *
 * $Id: volatil_memory.h,v 1.1 2001/05/29 15:19:20 chafik Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_object.h"

namespace NLAIAGENT
{		
	class CVolatilMemmory: public IAgent
	{
	public:
		static const NLAIC::CIdentType IdVolatilMemmory;

	private:
		IBaseGroupType *_List;

	public:
		CVolatilMemmory(IBaseGroupType *l = new CGroupType);
		CVolatilMemmory(const CVolatilMemmory &);
		virtual ~CVolatilMemmory();		


		IBaseGroupType *getList() const
		{
			return _List;
		}

		/// \name IBasicInterface method.
		//@{
		virtual void save(NLMISC::IStream &os);		
		virtual void load(NLMISC::IStream &is);
		
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdVolatilMemmory;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{			
			NLAIC::IBasicType *x = new CVolatilMemmory();		
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{			
			NLAIC::IBasicType *x = new CVolatilMemmory(*this);			
			return x;
		}		

		virtual void getDebugString(std::string &t) const;		
		//@}

		/// \name IObjectIA method.
		//@{
		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}
		//@}
	};
}
