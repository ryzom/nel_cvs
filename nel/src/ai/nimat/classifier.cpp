/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.1 2001/02/26 14:08:10 robert Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "nel/ai/nimat/classifier.h"


namespace NLAINIMAT
{

///////////////////////////
// CClassifierSystem
///////////////////////////

CClassifierSystem::~CClassifierSystem()
{
	std::list<CClassifier*>::iterator itClassifiers = _classifiers.begin();
	while (itClassifiers != _classifiers.end())
	{
		delete (*itClassifiers);
		itClassifiers++;
	}
}

void CClassifierSystem::addClassifier(std::map<std::string, char> conditionsMap, sint16 priority, const char* behavior)
{
	// We build a new classifier.
	CClassifier* classifier = new CClassifier();
	classifier->Behavior = behavior;
	classifier->Priority = priority;

	CClassifierConditionCell* condCell;
	std::map<std::string, char>::iterator itCondition = conditionsMap.begin();
	while (itCondition !=conditionsMap.end())
	{
		// We add the new sensor in the sensor map and init it with a joker value '#'
		_sensors[(*itCondition).first] = '#';

		// A new condition cell is added to the classifier condition.
		condCell = new CClassifierConditionCell(_sensors.find((*itCondition).first), (*itCondition).second);
		classifier->Condition.push_back(condCell);

		itCondition++;
	}

	// The new classifier is added to the classifier list.
	_classifiers.push_back(classifier);
}

std::string CClassifierSystem::selectBehavior(std::map<std::string, char> sensorMap)
{
	// We update the internal sensor values.
	std::map<std::string, char>::iterator itConditionsmap = sensorMap.begin();
	while (itConditionsmap != sensorMap.end())
	{
		_sensors[(*itConditionsmap).first] = (*itConditionsmap).second;
		itConditionsmap++;
	}

	// We select the activables classifiers
	std::list<CClassifier*> activableList;
	std::list<CClassifier*>::iterator itClassifiers = _classifiers.begin();
	std::list<CClassifierConditionCell*>::iterator itConditions;
	bool activable;
	int totalPriority = 0;

	while (itClassifiers != _classifiers.end())
	{
		activable = true;
		itConditions = (*itClassifiers)->Condition.begin();
		while (itConditions != (*itClassifiers)->Condition.end())
		{
			if (! (*itConditions)->isActivable() )
			{
				activable = false;
				break;
			}
			itConditions++;
		}
		if (activable)
		{
			activableList.push_back(*itClassifiers);
			totalPriority += (*itClassifiers)->Priority;
		}
		itClassifiers++;
	}

	// We select a classifier in the active classifier with a roullette wheel random.
	int r = rand() * totalPriority;
	r /= RAND_MAX;
	itClassifiers = activableList.begin();
	while (itClassifiers != activableList.end())
	{
		r -= (*itClassifiers)->Priority;
		if (r<=0)
		{
			return (*itClassifiers)->Behavior;
		}
		itClassifiers++;
	}

	// If no classifier is activable, we send a default joker value '#'
	return "#";
}


///////////////////////////
// CClassifier
///////////////////////////

CClassifierSystem::CClassifier::~CClassifier()
{
	std::list<CClassifierConditionCell*>::iterator itConditions = Condition.begin();
	while (itConditions != Condition.end())
	{
		delete (*itConditions );
		itConditions++;
	}
}

///////////////////////////
// CClassifierConditionCell
///////////////////////////

CClassifierSystem::CClassifierConditionCell::CClassifierConditionCell(std::map<std::string, char>::iterator itSensor, char value)
{
	_itSensor = itSensor;
	_value = value;
}

bool CClassifierSystem::CClassifierConditionCell::isActivable() const
{
	if ((*_itSensor).second == _value)
		return true;
	else
		return false;
}

} // NLAINIMAT
