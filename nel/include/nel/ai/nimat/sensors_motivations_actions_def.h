/** \file sensors_motivations_actions_def.h
 * Sensors, motivations and actions list of enums.
 *
 * $Id: sensors_motivations_actions_def.h,v 1.4 2003/06/19 17:14:34 robert Exp $
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

#ifndef RYSENSORS_MOTIVATIONS_ACTIONS_DEF_H
#define RYSENSORS_MOTIVATIONS_ACTIONS_DEF_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"

namespace NLAINIMAT
{

///////////////////////////
// All the Enums
///////////////////////////

/// This type give all the actions and virtual actions (high level action) of an Agent.
enum TAction
{
	//////////////////////////////////////////////////////////////////////////
	// First we have all the actions that may be executed
	Action_DoNothing = 0,	// This action must always be in 0. It's used this way in classifier.cpp
	
	Action_Idle,
	Action_Target_MoveAway,
	Action_Target_ShootTo,
	Action_Target_Approach,
	Action_Item_Approach,
	Action_Waypoint_MoveTo,
	Action_MoveToTargetFlag,
	Action_MoveToTargetFlagStart,
	Action_MoveToTargetFlagGoal,
	
	//////////////////////////////////////////////////////////////////////////
	// Then we have all the virtual action that are high level action
	// They must all be after Action_VIRTUAL_ACTIONS and they must start with "V_"
	Action_VIRTUAL_ACTIONS,
	
	Action_Unknown
};

static const NLMISC::CStringConversion<TAction>::CPair stringTableAction [] =
{ 
	{ "DoNothing",				Action_DoNothing },

	{ "Idle",					Action_Idle },
	{ "Target_MoveAway",		Action_Target_MoveAway },
	{ "Target_ShootTo",			Action_Target_ShootTo },
	{ "Target_Approach",		Action_Target_Approach },
	{ "Item_Approach",			Action_Item_Approach },
	{ "Waypoint_MoveTo",		Action_Waypoint_MoveTo },
	{ "MoveToTargetFlag",		Action_MoveToTargetFlag },
	{ "MoveToTargetFlagStart",	Action_MoveToTargetFlagStart },
	{ "MoveToTargetFlagGoal",	Action_MoveToTargetFlagGoal }
};

static NLMISC::CStringConversion<TAction> conversionAction
(
	stringTableAction,
	sizeof(stringTableAction) / sizeof(stringTableAction[0]),
	Action_Unknown
);

/// This type give all the motivations of an Agent.
enum TMotivation
{
	Motivation_Aggro = 0,
	Motivation_Fear,
	Motivation_GroupScore,
	Motivation_IndividualScore,
	
	Motivation_Unknown
};

static const NLMISC::CStringConversion<TMotivation>::CPair stringTableMotivation [] =
{ 
	{ "Aggro",			Motivation_Aggro },
	{ "Fear",			Motivation_Fear },
	{ "GroupScore",		Motivation_GroupScore },
	{ "IndividualScore",	Motivation_IndividualScore }
};

static NLMISC::CStringConversion<TMotivation> conversionMotivation
(
	stringTableMotivation,
	sizeof(stringTableMotivation) / sizeof(stringTableMotivation[0]),
	Motivation_Unknown
);

/// This type give all the sensors of an Agent.
enum TSensor
{
	//////////////////////////////////////////////////////////////////////////
	// First we have the locals sensors that don't depend of a target
	Sensor_BotLife = 0,			//(H)igh (M)edium (L)ow (D)ead
	Sensor_BotHasFlag,			//(T)rue (F)alse
	Sensor_BotHasTarget,		//(T)rue (F)alse
	Sensor_GroupHasFlag,		//(T)rue (F)alse
	Sensor_EnnemyGroupHasFlag,	//(T)rue (F)alse

	//////////////////////////////////////////////////////////////////////////
	// Then we have all sensors that need a target to compute
	// They must all be after Sensors_WITHTARGET
	Sensors_WITHTARGET, // *** This enum is used as a delimiter between sensor with no target and sensors with target ***

	Sensor_TARGET_SENSORS,	
	Sensor_TargetLife,			//(H)igh (M)edium (L)ow (D)ead
	Sensor_TargetHasFlag,		//(T)rue (F)alse
	Sensor_TargetIsMyFriend,	//(T)rue (F)alse
	Sensor_TargetDistance,		//(C)ontact (L)ong (F)ar
	Sensor_TargetIsMyCurrentTarget,	//(T)rue (F)alse
	Sensor_ITEM_SENSORS,	
	Sensor_ItemDistance,		//(C)ontact (L)ong (F)ar
	Sensor_ItemType,	
	Sensor_ItemIsBotFlag,		//(T)rue (F)alse
	Sensor_ItemIsEnnemyFlag,	//(T)rue (F)alse
	Sensor_WAYPOINT_SENSORS,	
	Sensor_wpFlag,				//(T)rue (F)alse
	Sensor_wpFlagGoal,			//(T)rue (F)alse
	
	Sensor_Unknown
};

// The conversion table
static const NLMISC::CStringConversion<TSensor>::CPair stringTableSensor [] =
{ 
	{"BotLife",					Sensor_BotLife },
	{"BotHasFlag",				Sensor_BotHasFlag },
	{"BotHasTarget",			Sensor_BotHasTarget },
	{"GroupHasFlag",			Sensor_GroupHasFlag },
	{"EnnemyGroupHasFlag",		Sensor_EnnemyGroupHasFlag },

	{"TARGET_SENSORS",			Sensor_TARGET_SENSORS },
	{"TargetLife",				Sensor_TargetLife },
	{"TargetHasFlag",			Sensor_TargetHasFlag },
	{"TargetIsMyFriend",		Sensor_TargetIsMyFriend },
	{"TargetDistance",			Sensor_TargetDistance },
	{"TargetIsMyCurrentTarget",	Sensor_TargetIsMyCurrentTarget },

	{"ITEM_SENSORS",			Sensor_ITEM_SENSORS },
	{"ItemDistance",			Sensor_ItemDistance },
	{"ItemType",				Sensor_ItemType },
	{"ItemIsBotFlag",			Sensor_ItemIsBotFlag },
	{"ItemIsEnnemyFlag",		Sensor_ItemIsEnnemyFlag },

	{"WAYPOINT_SENSORS",		Sensor_WAYPOINT_SENSORS },
	{"wpFlag",					Sensor_wpFlag },
	{"wpFlagGoal",				Sensor_wpFlagGoal },
};
static NLMISC::CStringConversion<TSensor> conversionSensor
(
	stringTableSensor,
	sizeof(stringTableSensor) / sizeof(stringTableSensor[0]),
	Sensor_Unknown
);

} // NLAINIMAT


#endif // RYSENSORS_MOTIVATIONS_ACTIONS_DEF_H

/* End of sensors_motivations_actions_def.h */

