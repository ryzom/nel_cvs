/** \file sensors_motivations_actions_def.cpp
 * Sensors, motivations and actions list of enums.
 *
 * $Id: sensors_motivations_actions_def.cpp,v 1.5 2003/07/24 17:03:29 robert Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX Ryzom.
 * NEVRAX Ryzom is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX Ryzom is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX Ryzom; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/ai/nimat/sensors_motivations_actions_def.h"

namespace NLAINIMAT
{
	
CActionResources::CActionResources()
{
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Idle,  ActionResources_forwardMove));

	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_MoveAway,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_MoveAway,  ActionResources_yaw));

	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_ShootTo,  ActionResources_pitch));
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_ShootTo,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_ShootTo,  ActionResources_button_attack));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_ShootTo,  ActionResources_button_attack2));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_Approach,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Target_Approach,  ActionResources_yaw));
	
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Item_Approach,  ActionResources_forwardMove));
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Item_Approach,  ActionResources_yaw));
	
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Waypoint_MoveTo,  ActionResources_forwardMove));
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Waypoint_MoveTo,  ActionResources_yaw));
//	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_Waypoint_MoveTo,  ActionResources_button_jump));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlag,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlag,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlag,  ActionResources_button_jump));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlagStart,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlagStart,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlagStart,  ActionResources_button_jump));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlagGoal,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlagGoal,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToTargetFlagGoal,  ActionResources_button_jump));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToMyFlagStart,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToMyFlagStart,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToMyFlagStart,  ActionResources_button_jump));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToMyFlagGoal,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToMyFlagGoal,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_MoveToMyFlagGoal,  ActionResources_button_jump));
	
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_LookAround,  ActionResources_forwardMove));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_LookAround,  ActionResources_yaw));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_LookAround,  ActionResources_button_jump));

	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassScout,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassSniper,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassSoldier,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassDemoman,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassMedic,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassHwguy,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassPyro,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassSpy,  ActionResources_selectClass));
	_ActionsResources.insert(std::pair<TAction, TActionResources>(Action_SelectClassEngineer,  ActionResources_selectClass));
}

CActionResources::~CActionResources()
{
	;
}

/// Let in the map myActionsByPriority all actions with no more actions Resources (thoses actions shouldn't be executed).
void CActionResources::filterMyActions (std::multimap<double, std::pair<TTargetId,TAction> >& myActionsByPriority)
{
	// We build a set of used resources.
	std::set<TActionResources> usedActionsResources;

	// For each wanted action in from the highest priority to the lowest
	std::multimap<double, std::pair<TTargetId,TAction> >::reverse_iterator itMyActionsByPriority( myActionsByPriority.end());
	std::multimap<double, std::pair<TTargetId,TAction> >::reverse_iterator itLast(myActionsByPriority.begin());
	std::set<TActionResources>::iterator	itUsedActionsResources;
	std::multimap<TAction, TActionResources>::iterator	itActionsResources, itActionsResources_begin, itActionsResources_last;
	bool actionIsOK = false;
	std::multimap<double, std::pair<TTargetId,TAction> >::iterator itMyActionsByPriority2Remove;
	
	while (itMyActionsByPriority != itLast )
	{
		double priority = (*itMyActionsByPriority).first;
		TAction action =  (*itMyActionsByPriority).second.second;
		itMyActionsByPriority++;
		
		// if none of the resources are used, the action may be executed and resources are stored as used.
		if (actionIsOK)
		{
			for (itActionsResources = itActionsResources_begin;
			itActionsResources != itActionsResources_last;
			itActionsResources++)
			{
				TActionResources ar = (*itActionsResources).second;
				usedActionsResources.insert(ar);
			}
			myActionsByPriority.erase(itMyActionsByPriority2Remove);
		}

		// We look for all needed action resources.
		itActionsResources_begin = _ActionsResources.lower_bound(action);
		itActionsResources_last = _ActionsResources.upper_bound(action);
		actionIsOK = true;

		// For each of thoses resources we look if it's already used.
		for (itActionsResources = itActionsResources_begin;
			 itActionsResources != itActionsResources_last;
			 itActionsResources++)
		{
			TActionResources ar = (*itActionsResources).second;
			itUsedActionsResources = usedActionsResources.find(ar);
			if( itUsedActionsResources != usedActionsResources.end())
			{
				actionIsOK = false;
				break;
			}
		}

		 itMyActionsByPriority2Remove = itMyActionsByPriority.base();
	}

	if (actionIsOK)
	{
		for (itActionsResources = itActionsResources_begin;
		itActionsResources != itActionsResources_last;
		itActionsResources++)
		{
			TActionResources ar = (*itActionsResources).second;
			usedActionsResources.insert(ar);
		}
		myActionsByPriority.erase(itMyActionsByPriority2Remove);
	}
}

} // NLAINIMAT

