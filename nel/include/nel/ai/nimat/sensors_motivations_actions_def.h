/** \file sensors_motivations_actions_def.h
 * Sensors, motivations and actions list of enums.
 *
 * $Id: sensors_motivations_actions_def.h,v 1.3 2003/06/17 12:15:27 robert Exp $
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
	
	Action_FightContact,
	Action_HealContact,
	Action_MoveTo,
	Action_MoveAwayFrom,
	Action_MagicMissile,
	Action_HarvestSap,
	Action_SpellSlow,
	Action_SpellSpeed,
	Action_DispelMagic,
	Action_RunToOfDeath,
	Action_RunAwayOfDeath,
	Action_Patrol,
	Action_TakeWater,
	Action_UseWater,

	//////////////////////////////////////////////////////////////////////////
	// Then we have all the virtual action that are high level action
	// They must all be after Action_VIRTUAL_ACTIONS and they must start with "V_"
	Action_VIRTUAL_ACTIONS,
	Action_V_Fight,
	Action_V_Protect,
	Action_V_Flee,
	Action_V_MoveTo,
	
	Action_Unknown
};

static const NLMISC::CStringConversion<TAction>::CPair stringTableAction [] =
{ 
	{ "DoNothing",		Action_DoNothing },

	{ "FightContact",	Action_FightContact },
	{ "HealContact",	Action_HealContact },
	{ "MoveTo",			Action_MoveTo },
	{ "MoveAwayFrom",	Action_MoveAwayFrom },
	{ "MagicMissile",	Action_MagicMissile },
	{ "HarvestSap",		Action_HarvestSap },
	{ "SpellSlow",		Action_SpellSlow },
	{ "SpellSpeed",		Action_SpellSpeed },
	{ "DispelMagic",	Action_DispelMagic },
	{ "RunToOfDeath",	Action_RunToOfDeath },
	{ "RunAwayOfDeath",	Action_RunAwayOfDeath },
	{ "Patrol",			Action_Patrol },
	{ "TakeWater",		Action_TakeWater },
	{ "UseWater",		Action_UseWater },
	
	{ "V_Fight",		Action_V_Fight },
	{ "V_Protect",		Action_V_Protect },
	{ "V_Flee",			Action_V_Flee },
	{ "V_MoveTo",		Action_V_MoveTo }
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
	Motivation_GroupProtection,

	Motivation_HarvestSap,
	Motivation_BodyGard,
	Motivation_TerritoryProtection,
	Motivation_Cultivate,
	
	Motivation_Unknown
};

static const NLMISC::CStringConversion<TMotivation>::CPair stringTableMotivation [] =
{ 
	{ "Aggro",				Motivation_Aggro },
	{ "Fear",				Motivation_Fear },
	{ "GroupProtection",	Motivation_GroupProtection },
	{ "HarvestSap",			Motivation_HarvestSap },
	{ "BodyGard",			Motivation_BodyGard },
	{ "TerritoryProtection",Motivation_TerritoryProtection },
	{ "Cultivate",			Motivation_Cultivate }
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
	Sensor_MyLife = 0,				//(H)igh (M)edium (L)ow (D)ead
	Sensor_MyMana,					//(H)igh (M)edium (L)ow (E)xhausted
	Sensor_MyLastAction,			//(F)ight (H)eal (S)hoot (M)oveTo (L)eave h(A)rvestSap
	Sensor_ThreatOnMe,				//(H)igh (M)edium (L)ow
	
	//////////////////////////////////////////////////////////////////////////
	// Then we have all sensors that need a target to compute
	// They must all be after Sensors_WITHTARGET
	Sensors_WITHTARGET, // *** This enum is used as a delimiter between sensor with no target and sensors with target ***

	Sensor_TargetLife,				//(H)igh (M)edium (L)ow (D)ead
	Sensor_TargetMana,				//(H)igh (M)edium (L)ow (E)xhausted
	Sensor_TargetCurrentAction,		//(F)ight (H)eal (S)hoot (M)oveTo (L)eave h(A)rvestSap
	Sensor_ThreatOnTarget,			//(H)igh (M)edium (L)ow
	Sensor_IsA,						//(H)omin (P)lant
	Sensor_IsFighter,				//(T)rue (F)alse
	Sensor_IsHealer,				//(T)rue (F)alse
	Sensor_IsMago,					//(T)rue (F)alse
	
	// Self <-> target relation information
	Sensor_TargetIsMyCurrentTarget,	//(T)rue (F)alse
	Sensor_IAmMyTargetCurrentTarget,//(T)rue (F)alse
	Sensor_SpeedDifference,			//I'm (F)aster (E)gal (S)lower
	Sensor_TargetDistance,			//(C)ontact (L)ong (F)ar
	Sensor_LevelDifference,			//I'm (S)tronger (E)gal (W)eek
	Sensor_Relationship,			//(E)nnemy (N)eutral (F)riend
	
	Sensor_Unknown
};

// The conversion table
static const NLMISC::CStringConversion<TSensor>::CPair stringTableSensor [] =
{ 
	{ "MyLife",						Sensor_MyLife },
	{ "MyMana",						Sensor_MyMana },
	{ "MyLastAction",				Sensor_MyLastAction },
	{ "ThreatOnMe",					Sensor_ThreatOnMe },
	
	{ "TargetLife",					Sensor_TargetLife },
	{ "TargetMana",					Sensor_TargetMana },
	{ "TargetCurrentAction",		Sensor_TargetCurrentAction },
	{ "ThreatOnTarget",				Sensor_ThreatOnTarget },
	{ "IsA",						Sensor_IsA },
	{ "IsFighter",					Sensor_IsFighter },
	{ "IsHealer",					Sensor_IsHealer },
	{ "IsMago",						Sensor_IsMago },
	
	{ "TargetIsMyCurrentTarget",	Sensor_TargetIsMyCurrentTarget },
	{ "IAmMyTargetCurrentTarget",	Sensor_IAmMyTargetCurrentTarget },
	{ "SpeedDifference",			Sensor_SpeedDifference },
	{ "TargetDistance",				Sensor_TargetDistance },
	{ "LevelDifference",			Sensor_LevelDifference },
	{ "Relationship",				Sensor_Relationship }
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

