/** \file interpret_fuzzy_controler.h
 *	Scripted class for a fuzzy controler
 *
 * $Id: interpret_fuzzy_controler.h,v 1.2 2001/01/08 10:47:05 chafik Exp $
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
		std::vector<NLAIAGENT::IObjectIA *> _Inputs;
		std::vector<NLAIAGENT::IVarName *> _InputNames;
		std::vector<NLAIAGENT::IObjectIA *> _Outputs;
		std::vector<NLAIAGENT::IVarName *> _OutputNames;
	public:
		static const NLAIC::CIdentType IdOperatorClass;
		
		CFuzzyControlerClass( std::list<NLAIAGENT::IObjectIA *> &, NLIASCRIPT::CFuzzyControlerClass *);

		CFuzzyControlerClass(const NLAIAGENT::IVarName &);
		CFuzzyControlerClass(const NLAIC::CIdentType &);
		CFuzzyControlerClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CFuzzyControlerClass(const CFuzzyControlerClass &);
		CFuzzyControlerClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;

		virtual ~CFuzzyControlerClass();

		virtual const std::vector<NLAIAGENT::IObjectIA *> &getInputs() const;
		virtual const std::vector<NLAIAGENT::IObjectIA *> &getOutputs() const;
		void addInput(NLAIAGENT::IObjectIA *);
		void addOutput(NLAIAGENT::IObjectIA *);
		const NLAIAGENT::IVarName *getInputName(sint32);
		const NLAIAGENT::IVarName *getOutputName(sint32);
	};
}
#endif
