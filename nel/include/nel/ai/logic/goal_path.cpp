/** \file opcode_lpt.cpp
 *
 * $Id: goal_path.cpp,v 1.1 2002/08/02 14:37:23 portier Exp $
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
#include "nel/ai/logic/goal_path.h"

/*
class CGoalPath : public CActorScript
{
	private:
		std::vector<CGoal *>		_Goals;
		std::vector<int>			_OnSucces;
		std::vector<int>			_OnFailure;

	public:
		CActorScript(IAgentManager *, bool activated = false);

		void setGoalStack(CGoalStack * );
		void addGoal(CGoal *, int on_succes = 0, int on_failure = 0);

		/// Own success and failure functions
		/// These function tell other operators and goals that might be waiting for 
		/// the execution of this one.
		virtual void processSuccess(NLAIAGENT::IObjectIA *);
		virtual void processFailure(NLAIAGENT::IObjectIA *);
		virtual void success();
		virtual void failure();
};
*/

namespace NLAILOGIC
{
	CGoalPath::CGoalPath(NLAIAGENT::IAgentManager *manager) : CActorScript( manager, true )
	{
	}

	void CGoalPath::addGoal( CGoal *goal, bool action, int on_success, int on_failure )
	{
		_Goals.push_back( goal );
		_JmpNext.push_back( action );
		_OnSucces.push_back( on_success );
		_OnFailure.push_back( on_failure );
	}

	void CGoalPath::success()
	{
		if ( _JmpNext[ _CurrentState ] == true )
		{
			if ( _CurrentState < ( _Goals.size() - 1 ) )
			{
				_CurrentState++;
				// Launch the new current goal
			}
			else
			{
				// Sends the launcher a succes, then dies
				onSuccess( NULL );
			}
		}
	}

	void CGoalPath::failure()
	{
		if ( _JmpNext[ _CurrentState ] == true )
		{
			if ( _CurrentState < ( _Goals.size() - 1 ) )
			{
				_CurrentState++;
				// Launch the new current goal
			}
			else
			{
				// Sends the launcher a failure, then dies
				onFailure( NULL );
			}
		}
	}





} // NLAILOGIC