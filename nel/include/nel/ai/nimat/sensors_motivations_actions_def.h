/** \file sensors_motivations_actions_def.h
 * Sensors, motivations and actions list of enums.
 *
 * $Id: sensors_motivations_actions_def.h,v 1.2 2003/03/14 14:27:45 robert Exp $
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
	Action_DoNothing = 0, // This action must always be in 0. It's used this way in classifier.cpp
	
	Action_CloseCombat,
	Action_DistanceCombat,
	Action_Eat,
	Action_V_Fight,

	Action_Wander,
	Action_Graze,
	Action_Rest,
	Action_Migrate,
	Action_Fight,
	Action_Flee,
	Action_MoveToTarget,
	Action_MoveToRestPlace,
	Action_SelfHeal,
	Action_Unknown
};

static const NLMISC::CStringConversion<TAction>::CPair stringTableAction [] =
{ 
	{ "DoNothing",		Action_DoNothing },

	{ "CloseCombat",	Action_CloseCombat },
	{ "DistanceCombat",	Action_DistanceCombat },
	{ "Eat",			Action_Eat },
	{ "V_Fight",		Action_V_Fight },

	{ "Wander",			Action_Wander },
	{ "Graze",			Action_Graze },
	{ "Rest",			Action_Rest },
	{ "Migrate",		Action_Migrate },
	{ "Fight",			Action_Fight },
	{ "Flee",			Action_Flee },
	{ "MoveToTarget",	Action_MoveToTarget },
	{ "MoveToRestPlace",Action_MoveToRestPlace },
	{ "SelfHeal",		Action_SelfHeal }
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
	Motivation_Wander = 0,
	Motivation_Hunger,
	Motivation_Fatigue,
	Motivation_Flock,
	Motivation_Aggro,
	Motivation_Fear,
	
	Motivation_Unknown
};

static const NLMISC::CStringConversion<TMotivation>::CPair stringTableMotivation [] =
{ 
	{ "Wander",		Motivation_Wander },
	{ "Hunger",		Motivation_Hunger },
	{ "Fatigue",	Motivation_Fatigue },
	{ "Flock",		Motivation_Flock },
	{ "Aggro",		Motivation_Aggro },
	{ "Fear",		Motivation_Fear }
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

	Sensor_Test_MunitionsAmount = 0,

	// Self inner state
	Sensor_IAmAlive,		// 'T'rue,		'F'alse

	// Self description
	Sensor_IAmOnNest,		// 'T'rue,		'F'alse

	// Self action
	Sensor_IAmEngaged,		// 'T'arget,	'O'ther target,	'N'ot engaged
	Sensor_IAmFleeing,		// 'T'arget,	'O'ther target,	'N'ot fleeing

	
	//////////////////////////////////////////////////////////////////////////
	// Then we have all sensors that need a target to compute
	// They must all be after Sensors_WITHTARGET
	Sensors_WITHTARGET, // *** This enum is used as a delimiter between sensor with no target and sensors with target ***

	// Target inner state
	Sensor_ItIsAlive,		// 'T'rue,		'F'alse

	// Target description
	Sensor_ItIsA,			// 'B'idoche,	'P'redator,		'H'erbivore,	'S'cavenger,	'O'min,	'A'nimat

	// Target Action
	Sensor_ItIsEngagedOn,	// 'F'riend,	'E'nnemy,		'U'ndefined,	'M'e,		'N'ot engaged

	// Self <-> target relation information
	Sensor_DistanceContact,	// 'T'rue,		'F'alse
	Sensor_DistanceEngage,	// 'T'rue,		'F'alse
	Sensor_IAmStronger,		// 'T'rue,		'F'alse
	Sensor_Relationship,	// 'F'riend,	'E'nnemy,		'U'ndefined

	Sensor_Unknown
};

// The conversion table
static const NLMISC::CStringConversion<TSensor>::CPair stringTableSensor [] =
{ 
	{ "MunitionsAmount",	Sensor_Test_MunitionsAmount },

	{ "IAmAlive",			Sensor_IAmAlive },
	{ "ItIsAlive",			Sensor_ItIsAlive },
	{ "ItIsA",				Sensor_ItIsA },
	{ "DistanceContact",	Sensor_DistanceContact },
	{ "DistanceEngage",		Sensor_DistanceEngage },
	{ "IAmStronger",		Sensor_IAmStronger },
	{ "IAmOnNest",			Sensor_IAmOnNest },
	{ "IAmEngaged",			Sensor_IAmEngaged },
	{ "ItIsEngagedOn",		Sensor_ItIsEngagedOn },
	{ "Relationship",		Sensor_Relationship },
	{ "IAmFleeing",			Sensor_IAmFleeing }
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

