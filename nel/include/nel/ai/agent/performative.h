/** \file performative.h
 *	
 *
 * $Id: performative.h,v 1.1 2001/01/19 12:50:18 chafik Exp $
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
#ifndef NL_PERFORMATIVE_H
#define NL_PERFORMATIVE_H

#include "messagerie.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAIAGENT
{	

	/**		
		Abstract Base class for performative allowing.
		* \author Chafik sameh
		* \author Portier Pierre
		* \author Nevrax France
		* \date 2000
	*/
	
	class IPerformative: public IntegerType
	{
	public:
		IPerformative(IMessageBase::TPerformatif pType):IntegerType((sint)pType)
		{
		}
		
		virtual const char *getName() const = 0;		
	};	

	class CPExec: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPExec;
	public:
		CPExec():IPerformative(IMessageBase::PExec)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPExec();			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPExec;
		}
		//@}

		virtual const char *getName() const
		{
			return "Exec";
		}
		
	};

	class CPAchieve: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPAchieve;
	public:
		CPAchieve():IPerformative(IMessageBase::PAchieve)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPAchieve;
		}
		
		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPAchieve();			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Achieve";
		}
		
	};

	class CPAsk: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPAsk;
	public:
		CPAsk():IPerformative(IMessageBase::PAsk)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPAsk;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPAsk();			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Ask";
		}
		
	};

	class CPBreak: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPBreak;
	public:
		CPBreak():IPerformative(IMessageBase::PBreak)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPBreak;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPBreak();			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}

		virtual const char *getName() const
		{
			return "Break";
		}
		
	};

	class CPTell: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPTell;
	public:
		CPTell():IPerformative(IMessageBase::PTell)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPTell;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPTell();			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Tell";
		}
		
	};
	

	class CPKill: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPKill;
	public:
		CPKill():IPerformative(IMessageBase::PKill)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPKill;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPKill();			
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Kill";
		}
		
	};
}
#endif