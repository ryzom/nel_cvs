/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.15 2003/03/18 12:44:48 robert Exp $
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
	using namespace NLMISC;

	static const TTargetId NullTargetId = 0;
				
///////////////////////////
// CClassifierSystem
///////////////////////////

CClassifierSystem::CClassifierSystem()
{
	_ClassifierNumber = 0;
}

CClassifierSystem::~CClassifierSystem()
{
	std::map<sint16, CClassifier*>::iterator itClassifiers = _classifiers.begin();
	while (itClassifiers != _classifiers.end())
	{
		delete (*itClassifiers).second;
		itClassifiers++;
	}
}

void CClassifierSystem::addClassifier(const CConditionMap &conditionsMap, double priority, TAction behavior)
{
	// We build a new classifier.
	CClassifier* classifier = new CClassifier();
	classifier->Behavior = behavior;
	classifier->Priority = priority;

	CClassifierConditionCell* condCell;
	std::map<TSensor, CConditionMap::CSensor >::const_iterator itCondition;
	for (itCondition = conditionsMap.begin(); itCondition != conditionsMap.end(); itCondition++)
	{
		// We add the new sensor in the sensor map and init it with a joker value '#'
		_sensors[(*itCondition).first] = '#';

		// A new condition cell is added to the classifier condition.
		condCell = new CClassifierConditionCell(_sensors.find((*itCondition).first),
												(*itCondition).second.SensorValue,
												(*itCondition).second.TruthValue);
		if ((*itCondition).second.NeedTarget)
		{
			classifier->ConditionWithTarget.push_back(condCell);
		}
		else
		{
			classifier->ConditionWithoutTarget.push_back(condCell);
		}
	}

	// The new classifier is added to the classifier list.
	_classifiers[_ClassifierNumber++] = classifier;
}

/// Merge two CS
void CClassifierSystem::addClassifierSystem(const CClassifierSystem &cs)
{
	std::map<sint16, CClassifier*>::const_iterator itCSClassifiers;

	// Pour chacun des classeurs de cs
	for (itCSClassifiers = cs._classifiers.begin(); itCSClassifiers != cs._classifiers.end(); itCSClassifiers++)
	{
		CConditionMap conditionsMap;
		std::list<CClassifierConditionCell*>::const_iterator itCondCell;

		// Pour chacune des condition sans cible d'un de ces classeur
		for (itCondCell = (*itCSClassifiers).second->ConditionWithoutTarget.begin();
			itCondCell !=(*itCSClassifiers).second->ConditionWithoutTarget.end();
			itCondCell++)
		{
			CClassifierConditionCell* pCondCell = (*itCondCell);
			// Je construit une liste de condition
			conditionsMap.addSensorCondition(pCondCell->getSensorName(), pCondCell->getValue(),pCondCell->getSensorIsTrue());
		}

		// Pour chacune des condition avec cible d'un de ces classeur
		for (itCondCell = (*itCSClassifiers).second->ConditionWithTarget.begin();
			itCondCell !=(*itCSClassifiers).second->ConditionWithTarget.end();
			itCondCell++)
		{
			CClassifierConditionCell* pCondCell = (*itCondCell);
			// Je construit une liste de condition
			conditionsMap.addSensorCondition(pCondCell->getSensorName(), pCondCell->getValue(),pCondCell->getSensorIsTrue());
		}
		
		// je rajoute dans mon cs un nouveau classeur avec ces conditions, la priorité et le comportement du classeur
		addClassifier(conditionsMap, (*itCSClassifiers).second->Priority, (*itCSClassifiers).second->Behavior);
	}
}

std::pair<sint16, TTargetId> CClassifierSystem::selectBehavior( const CCSPerception* psensorMap)
{
	TTargetId myTarget = NullTargetId;
	// We update the internal sensor values for the no target sensors
	// We use an internal sensor map, because each condition cell is mapped to this intrenal map.
	TSensorMap::iterator itSensors;
	TSensorMap::const_iterator itNoTargetSensors;
	for (itSensors = _sensors.begin(); itSensors != _sensors.end(); itSensors++)
	{
		TSensor sensName = (*itSensors).first;
		itNoTargetSensors = psensorMap->NoTargetSensors.find(sensName);
		if (itNoTargetSensors != psensorMap->NoTargetSensors.end())
		{
			TSensorValue c = (*itNoTargetSensors).second;
			(*itSensors).second = c;
		}
	}

	// We select the activables classifiers
	typedef	std::map<sint16, CClassifier*>::iterator  TitClassifiers;
	std::map<double, std::pair<TitClassifiers, TTargetId> > mapCSweel;
	TitClassifiers itClassifiers;
	std::list<CClassifierConditionCell*>::iterator itConditions;
	bool activable;
	double totalPriority = 0;

	for (itClassifiers = _classifiers.begin();
		itClassifiers != _classifiers.end();
		itClassifiers++)
	{
		activable = true;
		// S'il y a des conditions dépendantes d'une cible, ce n'est pas activable.
		if ((*itClassifiers).second->ConditionWithTarget.begin() == (*itClassifiers).second->ConditionWithTarget.end())
		{
			// On parcour la liste de sensor indépendant d'une cible
			for (itConditions = (*itClassifiers).second->ConditionWithoutTarget.begin();
				itConditions != (*itClassifiers).second->ConditionWithoutTarget.end();
				itConditions++)
			{
				if (! (*itConditions)->isActivable() )
				{
					activable = false;
					break;
				}
			}
			if (activable)
			{
				totalPriority += (*itClassifiers).second->Priority;
				mapCSweel[totalPriority] = std::make_pair(itClassifiers,myTarget);
			}
		}
	}

	// We now do the same, but with target sensors.
	std::map<TTargetId, TSensorMap>::const_iterator itTargetSensorMap;
	for (itTargetSensorMap = psensorMap->TargetSensors.begin(); itTargetSensorMap != psensorMap->TargetSensors.end(); itTargetSensorMap++)
	{
		myTarget = (*itTargetSensorMap).first;
		// We update the internal sensor values for the target sensors
		TSensorMap::const_iterator itTargetSensors;
		for (itSensors = _sensors.begin(); itSensors != _sensors.end(); itSensors++)
		{
			TSensor sensName = (*itSensors).first;
			itTargetSensors = (*itTargetSensorMap).second.find(sensName);
			if (itTargetSensors != (*itTargetSensorMap).second.end())
			{
				TSensorValue c = (*itTargetSensors).second;
				(*itSensors).second = c;
			}
		}
		
		// We select the activables classifiers
		for (itClassifiers = _classifiers.begin();
			itClassifiers != _classifiers.end();
			itClassifiers++)
		{
			activable = true;
			
			// On parcour la liste de sensor indépendant d'une cible
			for (itConditions = (*itClassifiers).second->ConditionWithoutTarget.begin();
				itConditions != (*itClassifiers).second->ConditionWithoutTarget.end();
				itConditions++)
			{
				if (! (*itConditions)->isActivable() )
				{
					activable = false;
					break;
				}
			}
			// On parcour la liste de sensor dépendant d'une cible
			for (itConditions = (*itClassifiers).second->ConditionWithTarget.begin();
				itConditions != (*itClassifiers).second->ConditionWithTarget.end();
				itConditions++)
			{
				if (! (*itConditions)->isActivable() )
				{
					activable = false;
					break;
				}
			}
			if (activable)
			{
				totalPriority += (*itClassifiers).second->Priority;
				mapCSweel[totalPriority] = std::make_pair(itClassifiers,myTarget);
			}
		}		
	}
	
	// We set the sensors back to the default value.
	for (itSensors = _sensors.begin(); itSensors != _sensors.end(); itSensors++)
	{
		(*itSensors).second = '#';
	}

	// If totalPriority == 0, there's no activable classifier. ***G*** But here we must add a rule creation mechanisme.
	if(totalPriority>0)
	{
		// We select a classifier in the active classifier with a roullette wheel random.
		double randomeNumber = (rand()%(int(totalPriority*100)))/100.0;
		std::map<double, std::pair<TitClassifiers, TTargetId> >::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
		CClassifier* pClassifierSelection = (*((*itMapCSweel).second.first)).second;
		sint16 selectionNumber = (*((*itMapCSweel).second.first)).first;
		myTarget = (*itMapCSweel).second.second;

		return std::make_pair(selectionNumber, myTarget);
	}
	else
	{
		return std::make_pair(-1,NullTargetId);
	}
}

TAction CClassifierSystem::getActionPart(sint16 classifierNumber)
{
	std::map<sint16, CClassifier*>::iterator itClassifiers = _classifiers.find(classifierNumber);
	nlassert(itClassifiers != _classifiers.end());
	return (*itClassifiers).second->Behavior;
}

void CClassifierSystem::getDebugString(std::string &t) const
{
	std::string dbg = "\n";

	std::map<sint16, CClassifier*>::const_iterator itClassifiers;
	for (itClassifiers = _classifiers.begin(); itClassifiers != _classifiers.end(); itClassifiers++)
	{
		dbg += "<" + NLMISC::toString((*itClassifiers).first) + "> ";
		std::list<CClassifierConditionCell*>::const_iterator itConditions;
		// On parcour la liste de sensor indépendant d'une cible
		for (itConditions = (*itClassifiers).second->ConditionWithoutTarget.begin();
			itConditions != (*itClassifiers).second->ConditionWithoutTarget.end();
			itConditions++)
		{
			CClassifierConditionCell* condCell = (*itConditions);
			if (condCell->getSensorIsTrue())
			{
				dbg += " (" + conversionSensor.toString(condCell->getSensorName()) + "= " + condCell->getValue() + ") +";
			}
			else
			{
				dbg += " (" + conversionSensor.toString(condCell->getSensorName()) + "=!" + condCell->getValue() + ") +";
			}
		}
		// On parcour la liste de sensor dépendant d'une cible
		for (itConditions = (*itClassifiers).second->ConditionWithTarget.begin();
			itConditions != (*itClassifiers).second->ConditionWithTarget.end();
			itConditions++)
		{
			CClassifierConditionCell* condCell = (*itConditions);
			if (condCell->getSensorIsTrue())
			{
				dbg += " (" + conversionSensor.toString(condCell->getSensorName()) + "(x)= " + condCell->getValue() + ") +";
			}
			else
			{
				dbg += " (" + conversionSensor.toString(condCell->getSensorName()) + "(x)=!" + condCell->getValue() + ") +";
			}
		}
		std::string actionName = conversionAction.toString((*itClassifiers).second->Behavior);
		double		prio = (*itClassifiers).second->Priority;
		dbg += "> " + actionName + " [" + NLMISC::toString(prio) + "]\n";
	}
	t += dbg;
}

///////////////////////////
// CClassifier
///////////////////////////

CClassifierSystem::CClassifier::CClassifier()
{
}

CClassifierSystem::CClassifier::~CClassifier()
{
	std::list<CClassifierConditionCell*>::iterator itConditions = ConditionWithTarget.begin();
	while (itConditions != ConditionWithTarget.end())
	{
		delete (*itConditions );
		itConditions++;
	}
	itConditions = ConditionWithoutTarget.begin();
	while (itConditions != ConditionWithoutTarget.end())
	{
		delete (*itConditions );
		itConditions++;
	}
}

///////////////////////////
// CClassifierConditionCell
///////////////////////////

CClassifierSystem::CClassifierConditionCell::CClassifierConditionCell(TSensorMap::const_iterator itSensor,
																	  TSensorValue value, bool sensorIsTrue)
{
	_itSensor = itSensor;
	_Value = value;
	_SensorIsTrue = sensorIsTrue;
}

bool CClassifierSystem::CClassifierConditionCell::isActivable() const
{
	if (_SensorIsTrue)
	{
		if ((*_itSensor).second == _Value)
			return true;
		else
			return false;
	}
	else
	{
		if ((*_itSensor).second == _Value)
			return false;
		else
			return true;
	}
}

TSensor CClassifierSystem::CClassifierConditionCell::getSensorName() const
{
	return (*_itSensor).first;
}

TSensorValue CClassifierSystem::CClassifierConditionCell::getValue() const
{
	return _Value;
}

bool CClassifierSystem::CClassifierConditionCell::getSensorIsTrue() const
{
	return _SensorIsTrue;
}

///////////////////////////
// CConditionMap
///////////////////////////

void CConditionMap::addIfSensorCondition(TSensor sensorName, TSensorValue sensorValue)
{
	CSensor albator;
	albator.SensorValue = sensorValue;
	albator.TruthValue = true;
	albator.NeedTarget = (sensorName > Sensors_WITHTARGET);

	_ConditionMap[sensorName] = albator;
}

void CConditionMap::addIfNotSensorCondition(TSensor sensorName, TSensorValue sensorValue)
{
	CSensor albator;
	albator.SensorValue = sensorValue;
	albator.TruthValue = false;
	albator.NeedTarget = (sensorName > Sensors_WITHTARGET);
	
	_ConditionMap[sensorName] = albator;
}

void CConditionMap::addSensorCondition(TSensor sensorName, TSensorValue sensorValue, bool sensorIsTrue)
{
	CSensor albator;
	albator.SensorValue = sensorValue;
	albator.TruthValue = sensorIsTrue;
	albator.NeedTarget = (sensorName > Sensors_WITHTARGET);
	
	_ConditionMap[sensorName] = albator;
}

///////////////////////////
// CActionClassifiers
///////////////////////////
CActionClassifiers::CActionClassifiers(TAction name)
{
	_Name = name;
}

CActionClassifiers::~CActionClassifiers()
{
}

/// Return the action name
TAction CActionClassifiers::getName() const
{
	return _Name;
}

void CActionClassifiers::addMotivationRule (TMotivation motivationName, const CConditionMap &conditionsMap, double priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByMotivation[motivationName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

void CActionClassifiers::addVirtualActionRule (TAction virtualActionName, const CConditionMap &conditionsMap, double priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByVirtualAction[virtualActionName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

const std::map<TMotivation, CClassifierSystem> *CActionClassifiers::getClassifiersByMotivationMap () const
{
	return &_ClassifiersByMotivation;
}
const std::map<TAction, CClassifierSystem> *CActionClassifiers::getClassifiersByVirtualActionMap () const
{
	return &_ClassifiersByVirtualAction;
}

/// Chaine de debug
void CActionClassifiers::getDebugString(std::string &t) const
{
	std::string ret = "\nACTION :\t" + conversionAction.toString(_Name) + "\n";
	std::map<TMotivation, CClassifierSystem>::const_iterator ItClassifiersByMotivation;
	for (ItClassifiersByMotivation = _ClassifiersByMotivation.begin(); ItClassifiersByMotivation != _ClassifiersByMotivation.end(); ItClassifiersByMotivation++)
	{
		ret += "\nMotivation : " + conversionMotivation.toString((*ItClassifiersByMotivation).first) + "\n";
		(*ItClassifiersByMotivation).second.getDebugString(ret);
	}
	std::map<TAction, CClassifierSystem>::const_iterator ItClassifiersByVirtualAction;
	for (ItClassifiersByVirtualAction = _ClassifiersByVirtualAction.begin(); ItClassifiersByVirtualAction != _ClassifiersByVirtualAction.end(); ItClassifiersByVirtualAction++)
	{
		ret += "\nMotivation : " + conversionAction.toString((*ItClassifiersByVirtualAction).first) + "\n";
		(*ItClassifiersByVirtualAction).second.getDebugString(ret);
	}
	t+=ret;
}

///////////////////////////
// CCSPerception
///////////////////////////
CCSPerception::CCSPerception()
{
	;
}

CCSPerception::~CCSPerception()
{
	;
}


} // NLAINIMAT



