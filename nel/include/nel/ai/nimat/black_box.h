/** \file black_box.h
 * An interface between an agent and a C++ component.
 *
 * $Id: black_box.h,v 1.1 2001/01/05 10:50:23 chafik Exp $
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

#ifndef NL_IA_BLACK_BOX_
#define NL_IA_BLACK_BOX_

namespace NLIANIMAT
{
	typedef std::string tString;
	typedef std::list<tString> tCommandList;
	typedef std::map<tString,CIdentType > tParameterList;
	/** An interface between an agent and a C++ component.
	 *	An IBlackBox is an interface for any component which can be use in the built of an agent.
	 *	It's like an electronique component which can be plug in an agent.
	 *	There can be input and output parameter and input and output command signal.
	 *  \author Robert Gabriel
	 *  \author Nevrax France
	 *  \date 2000
	 */
	class IBlackBox : public IObjectIA
	{
	public :

		IBlackBox();

		/** @name Description functions
		 *  Give the interface (Inputs, Outputs, ParamaeterIn and ParameterOut) of the IBlackBox.
		 */
		//@{
		/**
		 *	Return the list of Inputs for command signal.
		 *	It's a list of the differents pins which can be triggered when receiving an incoming event.
		 */
		virtual tCommandList getInputList() const = 0;

		/**
		 *	Return the list of Outputs for command signal.
		 *	It's a list of the differents pins which allows to send an event, to triggered anoter IBlackBox.
		 */
		virtual tCommandList getOutputList() const = 0;

		/**
		 *	Return the list of parameters used by the IBlackBox.
		 *	It's the name and type of each parameters.
		 */
		virtual tParameterList getParamInList() const = 0;

		/**
		 *	Return the list of parameters returned by the IBlackBox.
		 *	It's the name and type of each parameters.
		 */
		virtual tParameterList getParamOutList() const = 0;
		//@}


		/** @name Running functions
		 *  Functions used during the run time of the IBlackBox.
		 */
		//@{
		/// Fix the value of an input signal.
		virtual void setInput(tString inputName, bool) = 0;

		/// Return the value of an output signal.
		virtual bool getOutput(tString outputName) = 0;

		/// Set the value of an input parameter.
		virtual void setParamIn(tString parameterName, IObjectIA val) = 0;

		/// Get the value of an output parameter.
		virtual const IObjectIA* getParamOut(tString parameterName) = 0;

		/**
		 *	Return a list of the parameters which need to be updated.
		 */
		virtual tParameterList getNeededParamIn () const = 0;
		//@}
	};
}

#endif //NL_IA_BLACK_BOX_