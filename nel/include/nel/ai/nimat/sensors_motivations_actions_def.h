/** \file sensors_motivations_actions_def.h
 * Sensors, motivations and actions list of enums.
 *
 * $Id: sensors_motivations_actions_def.h,v 1.7 2003/07/24 17:03:15 robert Exp $
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
#include "set"
#include "map"

namespace NLAINIMAT
{

typedef uint32							TTargetId;
	
///////////////////////////
// All the Enums
///////////////////////////

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
	Sensor_DamageTaken,			//(T)rue (F)alse
	Sensor_BotClass,			// s(C)out s(N)iper (S)oldier (D)emoman (M)edic (H)eavy_weapon_guy (P)yro sp(Y) (E)ngineer (O)ther
	
	Sensor_ScoutInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_SniperInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_SoldierInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_DemomanInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_MedicInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_HWGuyInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_PyroInMyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_SpyInMyTeam,			// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_EngineerInMyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	
	Sensor_ScoutInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_SniperInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_SoldierInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_DemomanInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_MedicInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_HWGuyInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_PyroInEnemyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_SpyInEnemyTeam,		// (T)true (F)alse. True if I had seen one seen my last respawn.
	Sensor_EngineerInEnemyTeam,	// (T)true (F)alse. True if I had seen one seen my last respawn.
	
	//////////////////////////////////////////////////////////////////////////
	// Then we have all sensors that need a target to compute
	// They must all be after Sensors_WITHTARGET
	Sensors_WITHTARGET, // *** This enum is used as a delimiter between sensor with no target and sensors with target ***

	Sensor_TARGET_SENSORS,	
	Sensor_TargetLife,			//(H)igh (M)edium (L)ow (D)ead (U)nknown
	Sensor_TargetHasFlag,		//(T)rue (F)alse
	Sensor_TargetIsMyFriend,	//(T)rue (F)alse
	Sensor_TargetDistance,		//(C)ontact (L)ong (F)ar
	Sensor_TargetIsMyCurrentTarget,	//(T)rue (F)alse
	Sensor_TargetClass,			// s(C)out s(N)iper (S)oldier (D)emoman (M)edic (H)eavy_weapon_guy (P)yro sp(Y) (E)ngineer (O)ther

//	Sensor_ITEM_SENSORS,	
//	Sensor_ItemDistance,		//(C)ontact (L)ong (F)ar
//	Sensor_ItemType,	
//	Sensor_ItemIsBotFlag,		//(T)rue (F)alse
//	Sensor_ItemIsEnnemyFlag,	//(T)rue (F)alse
//	Sensor_WAYPOINT_SENSORS,	
//	Sensor_wpFlag,				//(T)rue (F)alse
//	Sensor_wpFlagGoal,			//(T)rue (F)alse
	
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
	{"DamageTaken",				Sensor_DamageTaken	},
	{"BotClass",				Sensor_BotClass	},
	
	{"ScoutInMyTeam",			Sensor_ScoutInMyTeam },		
	{"SniperInMyTeam",			Sensor_SniperInMyTeam },		
	{"SoldierInMyTeam",			Sensor_SoldierInMyTeam },		
	{"DemomanInMyTeam",			Sensor_DemomanInMyTeam },		
	{"MedicInMyTeam",			Sensor_MedicInMyTeam },		
	{"HWGuyInMyTeam",			Sensor_HWGuyInMyTeam },		
	{"PyroInMyTeam",			Sensor_PyroInMyTeam },		
	{"SpyInMyTeam",				Sensor_SpyInMyTeam },			
	{"EngineerInMyTeam",		Sensor_EngineerInMyTeam },	
	
	{"ScoutInEnemyTeam",		Sensor_ScoutInEnemyTeam },	
	{"SniperInEnemyTeam",		Sensor_SniperInEnemyTeam },	
	{"SoldierInEnemyTeam",		Sensor_SoldierInEnemyTeam },	
	{"DemomanInEnemyTeam",		Sensor_DemomanInEnemyTeam },	
	{"MedicInEnemyTeam",		Sensor_MedicInEnemyTeam },	
	{"HWGuyInEnemyTeam",		Sensor_HWGuyInEnemyTeam },	
	{"PyroInEnemyTeam",			Sensor_PyroInEnemyTeam },		
	{"SpyInEnemyTeam",			Sensor_SpyInEnemyTeam },		
	{"EngineerInEnemyTeam",		Sensor_EngineerInEnemyTeam }	,
	
	{"TARGET_SENSORS",			Sensor_TARGET_SENSORS },
	{"TargetLife",				Sensor_TargetLife },
	{"TargetHasFlag",			Sensor_TargetHasFlag },
	{"TargetIsMyFriend",		Sensor_TargetIsMyFriend },
	{"TargetDistance",			Sensor_TargetDistance },
	{"TargetIsMyCurrentTarget",	Sensor_TargetIsMyCurrentTarget },
	{"TargetClass",				Sensor_TargetClass }

//	{"ITEM_SENSORS",			Sensor_ITEM_SENSORS },
//	{"ItemDistance",			Sensor_ItemDistance },
//	{"ItemType",				Sensor_ItemType },
//	{"ItemIsBotFlag",			Sensor_ItemIsBotFlag },
//	{"ItemIsEnnemyFlag",		Sensor_ItemIsEnnemyFlag },
//
//	{"WAYPOINT_SENSORS",		Sensor_WAYPOINT_SENSORS },
//	{"wpFlag",					Sensor_wpFlag },
//	{"wpFlagGoal",				Sensor_wpFlagGoal },
};
static NLMISC::CStringConversion<TSensor> conversionSensor
(
	stringTableSensor,
	sizeof(stringTableSensor) / sizeof(stringTableSensor[0]),
	Sensor_Unknown
);

/// This type give all the actions and virtual actions (high level action) of an Agent.
enum TAction
{
	//////////////////////////////////////////////////////////////////////////
	// First we have all the actions that may be executed
	Action_DoNothing = 0,	// This action must always be in 0. It's used this way in classifier.cpp
		
//	Action_Idle,
	Action_Target_MoveAway,
	Action_Target_ShootTo,
	Action_Target_Approach,
//	Action_Item_Approach,
//	Action_Waypoint_MoveTo,
	Action_MoveToTargetFlag,
	Action_MoveToTargetFlagStart, // The one I must catch
	Action_MoveToTargetFlagGoal,
	Action_MoveToMyFlagStart,	// The one my enemy must catch
	Action_MoveToMyFlagGoal,
	Action_LookAround,
	
	Action_SelectClassScout,
	Action_SelectClassSniper,
	Action_SelectClassSoldier,
	Action_SelectClassDemoman,
	Action_SelectClassMedic,
	Action_SelectClassHwguy,
	Action_SelectClassPyro,
	Action_SelectClassSpy,
	Action_SelectClassEngineer,
	
	//////////////////////////////////////////////////////////////////////////
	// Then we have all the virtual action that are high level action
	// They must all be after Action_VIRTUAL_ACTIONS and they must start with "V_"
	Action_VIRTUAL_ACTIONS,
	
	Action_Unknown
};

static const NLMISC::CStringConversion<TAction>::CPair stringTableAction [] =
{ 
	{ "DoNothing",				Action_DoNothing },
		
//	{ "Idle",					Action_Idle },
	{ "Target_MoveAway",		Action_Target_MoveAway },
	{ "Target_ShootTo",			Action_Target_ShootTo },
	{ "Target_Approach",		Action_Target_Approach },
//	{ "Item_Approach",			Action_Item_Approach },
//	{ "Waypoint_MoveTo",		Action_Waypoint_MoveTo },
	{ "MoveToTargetFlag",		Action_MoveToTargetFlag },
	{ "MoveToTargetFlagStart",	Action_MoveToTargetFlagStart },
	{ "MoveToTargetFlagGoal",	Action_MoveToTargetFlagGoal },
	{ "MoveToMyFlagStart",		Action_MoveToMyFlagStart },
	{ "MoveToMyFlagGoal",		Action_MoveToMyFlagGoal },
	{ "LookAround",				Action_LookAround },
	{ "SelectClassScout",		Action_SelectClassScout },
	{ "SelectClassSniper",		Action_SelectClassSniper },
	{ "SelectClassSoldier",		Action_SelectClassSoldier },
	{ "SelectClassDemoman",		Action_SelectClassDemoman },
	{ "SelectClassMedic",		Action_SelectClassMedic },
	{ "SelectClassHwguy",		Action_SelectClassHwguy },
	{ "SelectClassPyro",		Action_SelectClassPyro },
	{ "SelectClassSpy",			Action_SelectClassSpy },
	{ "SelectClassEngineer",	Action_SelectClassEngineer },
};

static NLMISC::CStringConversion<TAction> conversionAction
(
	stringTableAction,
	sizeof(stringTableAction) / sizeof(stringTableAction[0]),
	Action_Unknown
);

/// This type give all the action resources of an Agent.
enum TActionResources
{
	ActionResources_pitch = 0,
	ActionResources_yaw,
	ActionResources_forwardMove,
	ActionResources_sideMove,
	ActionResources_button_attack,
	ActionResources_button_attack2,
	ActionResources_button_jump,
	ActionResources_button_duck,
	ActionResources_button_forward, // Used for ladders
	ActionResources_button_use,
	ActionResources_button_reload,
	ActionResources_currentWeapon,
	ActionResources_callForHealth,
	ActionResources_grenade1,
	ActionResources_grenade2,
	ActionResources_selectClass,

	ActionResources_Unknown
};

/*
 *	Définition : Je dois pouvoir associer à une action un ensemble de ressources d'actions.
 *	Utilisation : Je donne un certain nombre d'actions avec des priorités, il me répond par une liste des actions activables.
 *	L'algo : Je parcour ma liste d'action de la plus forte à la plus faible. 
 *	Si les ressources d'action sont libre, je les notes comme occupées, puis je note l'action comme activable.
 */

class CActionResources
{
public:
	CActionResources();
	virtual ~CActionResources();

	/// Let in the map myActionsByPriority all actions with no more actions Resources (thoses actions shouldn't be executed)
	void filterMyActions (std::multimap<double, std::pair<TTargetId,TAction> >& myActionsByPriority);

private:
	std::multimap<TAction, TActionResources>	_ActionsResources;
};

} // NLAINIMAT


#endif // RYSENSORS_MOTIVATIONS_ACTIONS_DEF_H

/* End of sensors_motivations_actions_def.h */

