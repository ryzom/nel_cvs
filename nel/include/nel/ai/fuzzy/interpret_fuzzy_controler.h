/** \file interpret_fuzzy_controler.h
 *	Scripted class for a fuzzy controler
 *
 * $Id: interpret_fuzzy_controler.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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
#ifndef NL_FUZZYCONTROLER_CLASS_H
#define NL_FUZZYCONTROLER_CLASS_H

#include "script/interpret_object_agent.h"

namespace NLIASCRIPT
{
	
	class CFuzzyControlerClass : public CAgentClass
	{
	private:
		std::vector<NLIAAGENT::IObjectIA *> _Inputs;
		std::vector<NLIAAGENT::IVarName *> _InputNames;
		std::vector<NLIAAGENT::IObjectIA *> _Outputs;
		std::vector<NLIAAGENT::IVarName *> _OutputNames;
	public:
		static const NLIAC::CIdentType IdOperatorClass;
		
		CFuzzyControlerClass( std::list<NLIAAGENT::IObjectIA *> &, NLIASCRIPT::CFuzzyControlerClass *);

		CFuzzyControlerClass(const NLIAAGENT::IVarName &);
		CFuzzyControlerClass(const NLIAC::CIdentType &);
		CFuzzyControlerClass(const NLIAAGENT::IVarName &, const NLIAAGENT::IVarName &);
		CFuzzyControlerClass(const CFuzzyControlerClass &);
		CFuzzyControlerClass();

		const NLIAC::IBasicType *clone() const;
		const NLIAC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLIAAGENT::IObjectIA *buildNewInstance() const;

		virtual ~CFuzzyControlerClass();

		virtual const std::vector<NLIAAGENT::IObjectIA *> &getInputs() const;
		virtual const std::vector<NLIAAGENT::IObjectIA *> &getOutputs() const;
		void addInput(NLIAAGENT::IObjectIA *);
		void addOutput(NLIAAGENT::IObjectIA *);
		const NLIAAGENT::IVarName *getInputName(sint32);
		const NLIAAGENT::IVarName *getOutputName(sint32);
	};
}
#endif
