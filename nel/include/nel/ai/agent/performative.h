/** \file performative.h
 *	
 *
 * $Id: performative.h,v 1.8 2003/01/31 14:59:43 chafik Exp $
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

#include "nel/ai/agent/msg.h"
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

	/**
	When an agent want to exec a script.
	*/
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

	/**
	When an agent want to achieve logic job to an other agent such as goal for exempel.
	*/
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

	/**
	When an agent want to ask an ather agent, this ather agent send a tell to tel her the result ogf ask.
	*/
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

	/**
	When an agent want to tell some thing to an ather. A Tell is used after an Ask performatif.
	*/
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
	

	/**
	When an agent want to break an ather.
	*/
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

	/**
	When an agent want to kill an ather.
	*/
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

	/**
	When an agent have to communicate an error result to an ather.
	*/
	class CPError: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdPError;
	public:
		CPError():IPerformative(IMessageBase::PError)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdPError;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPError();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Error";
		}
		
	};

	class CPEven: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdEven;
	public:
		CPEven():IPerformative(IMessageBase::PEven)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdEven;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPEven();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Even";
		}		
	};

	class CPService: public IPerformative
	{
	public:
		static const NLAIC::CIdentType IdService;
	public:
		CPService():IPerformative(IMessageBase::PService)
		{
		}

		/// \name Base class member method.			
		//@{
		virtual const NLAIC::CIdentType &getType() const
		{
			return IdService;
		}

		virtual const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicInterface *m = new CPService();
			return m;
		}
		
		virtual const NLAIC::IBasicType *newInstance() const
		{
			return clone();
		}
		//@}
		
		virtual const char *getName() const
		{
			return "Even";
		}	
	};
}
#endif
