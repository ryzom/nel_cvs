/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.18 2003/07/24 17:03:29 robert Exp $
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
	std::map<TClassifierNumber, CClassifier*>::iterator itClassifiers = _Classifiers.begin();
	while (itClassifiers != _Classifiers.end())
	{
		delete (*itClassifiers).second;
		itClassifiers++;
	}
}

void CClassifierSystem::addClassifier(const CConditionMap &conditionsMap, TClassifierPriority priority, TAction behavior)
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
		TSensor bibu = (*itCondition).first;
		_Sensors[(*itCondition).first] = '#';

		// A new condition cell is added to the classifier condition.
		condCell = new CClassifierConditionCell(_Sensors.find((*itCondition).first),
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
	_Classifiers[_ClassifierNumber++] = classifier;
}

/// Merge two CS
void CClassifierSystem::addClassifierSystem(const CClassifierSystem &cs)
{
	std::map<TClassifierNumber, CClassifier*>::const_iterator itCSClassifiers;

	// Pour chacun des classeurs de cs
	for (itCSClassifiers = cs._Classifiers.begin(); itCSClassifiers != cs._Classifiers.end(); itCSClassifiers++)
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

// Function used in selectBehavior
void	CClassifierSystem::updateNoTargetSensors(const CCSPerception* psensorMap)
{
	// We use an internal sensor map, because each condition cell is mapped to this intrenal map.
	TSensorMap::iterator itSensors;
	TSensorMap::const_iterator itNoTargetSensors;

	for (itSensors = _Sensors.begin(); itSensors != _Sensors.end(); itSensors++)
	{
		TSensor sensName = (*itSensors).first;
		itNoTargetSensors = psensorMap->NoTargetSensors.find(sensName);
		if (itNoTargetSensors !=psensorMap->NoTargetSensors.end())
		{
			TSensorValue c = (*itNoTargetSensors).second;
			(*itSensors).second = c;
		}
	}
}

// Function used in selectBehavior
void	CClassifierSystem::updateTargetSensors(const CCSPerception* psensorMap, TTargetId target)
{
	// We update the internal sensor values for the target sensors
	TSensorMap::iterator itSensors;
	TSensorMap::const_iterator itTargetSensors;
	std::map<TTargetId, TSensorMap>::const_iterator itTargetSensorMap = psensorMap->TargetSensors.find(target);
	nlassert( itTargetSensorMap != psensorMap->TargetSensors.end() );

	for (itSensors = _Sensors.begin(); itSensors != _Sensors.end(); itSensors++)
	{
		TSensor sensName = (*itSensors).first;
		itTargetSensors = (*itTargetSensorMap).second.find(sensName);
		if (itTargetSensors != (*itTargetSensorMap).second.end())
		{
			TSensorValue c = (*itTargetSensors).second;
			(*itSensors).second = c;
		}
	}
}

// Function used in selectBehavior
void	CClassifierSystem::RAZTargetSensors()
{
	TSensorMap::iterator itSensorBorder = _Sensors.upper_bound(Sensors_WITHTARGET);
	TSensorMap::iterator itSensors;
	for (itSensors = itSensorBorder; itSensors != _Sensors.end(); itSensors++)
	{
		(*itSensors).second = '#';
	}
}

// Function used in selectBehavior
void	CClassifierSystem::RAZNoTargetSensors()
{
	TSensorMap::iterator itSensorBorder = _Sensors.upper_bound(Sensors_WITHTARGET);
	TSensorMap::iterator itSensors;
	for (itSensors = _Sensors.begin(); itSensors != itSensorBorder; itSensors++)
	{
		(*itSensors).second = '#';
	}
}

// Function used in selectBehavior
TClassifierPriority	CClassifierSystem::computeMaxPriorityDoingTheSameAction(const CCSPerception* psensorMap,
																TClassifierNumber lastClassifierNumber, 
																TTargetId lastTarget, 
																TClassifierPriority lastSelectionMaxPriority,
																std::multimap<TClassifierPriority, std::pair<TitClassifiers, TTargetId> > &mapActivableCS)
{
	TClassifierPriority maxPriorityDoingTheSameAction = 0;
	if (lastClassifierNumber != -1)
	{
		std::map<TClassifierNumber, CClassifier*>::iterator itClassifiers = _Classifiers.find(lastClassifierNumber);
		nlassert ( itClassifiers != _Classifiers.end() );
		TAction lastAction = (*itClassifiers).second->Behavior;

		// We search for a classfier doing the same action on the same target
		if (lastTarget != NullTargetId)
		{
			// It was an action with a target so we update the internal sensor values for the target sensors
			std::map<TTargetId, TSensorMap>::const_iterator itTargetSensorMap = psensorMap->TargetSensors.find(lastTarget);
			if ( itTargetSensorMap != psensorMap->TargetSensors.end() )
			{
				updateTargetSensors(psensorMap, lastTarget);
			}
			else
			{
				// The target didn't exist anymore in my perception, so the maxPriority is set to 0.
				return 0;
			}
		}

		// We select the activables classifiers
		for (itClassifiers = _Classifiers.begin(); itClassifiers != _Classifiers.end(); itClassifiers++)
		{
			if ( (*itClassifiers).second->Behavior == lastAction)
				if ( (*itClassifiers).second->isActivable())
				{
					TClassifierPriority thePriority = (*itClassifiers).second->Priority;
					/*	If it's the same classifier than the last time, we give it the same priority than the last Time.
					 *	It's to allow a classifier that has been selected with a low priority by luck to continue to express
					 *	himself more than just for one update.
					 */
					if ((*itClassifiers).first == lastClassifierNumber )
					{
						nlassert (thePriority <= lastSelectionMaxPriority);
						thePriority = lastSelectionMaxPriority;
					}
					maxPriorityDoingTheSameAction = std::max(thePriority, maxPriorityDoingTheSameAction);

					mapActivableCS.insert(std::make_pair((*itClassifiers).second->Priority, std::make_pair(itClassifiers, lastTarget)));
				}
		}		
	}
	RAZTargetSensors();
	return maxPriorityDoingTheSameAction;
}

// Function used in selectBehavior
TClassifierPriority	CClassifierSystem::computeHigherPriority(const CCSPerception* psensorMap,
												 TClassifierPriority maxPriorityDoingTheSameAction,
												 std::multimap<TClassifierPriority, std::pair<TitClassifiers, TTargetId> > &mapActivableCS)
{
	TitClassifiers itClassifiers;
	TClassifierPriority higherPriority = maxPriorityDoingTheSameAction;
	
	for (itClassifiers = _Classifiers.begin();
	itClassifiers != _Classifiers.end();
	itClassifiers++)
	{
		// We first check if the classifier priority is higher than maxPriorityDoingTheSameAction
		if ((*itClassifiers).second->Priority > maxPriorityDoingTheSameAction)
			// S'il y a des conditions dépendantes d'une cible, ce n'est pas activable.
			if ((*itClassifiers).second->ConditionWithTarget.begin() == (*itClassifiers).second->ConditionWithTarget.end())
				if ( (*itClassifiers).second->isActivable())
				{
					higherPriority = std::max((*itClassifiers).second->Priority, higherPriority);
					mapActivableCS.insert(std::make_pair((*itClassifiers).second->Priority, std::make_pair(itClassifiers, NullTargetId)));
				}
	}
	
	// We now do the same, but with target sensors.
	std::map<TTargetId, TSensorMap>::const_iterator itTargetSensorMap;
	for (itTargetSensorMap = psensorMap->TargetSensors.begin(); itTargetSensorMap != psensorMap->TargetSensors.end(); itTargetSensorMap++)
	{
		TTargetId myTarget = (*itTargetSensorMap).first;
		// We update the internal sensor values for the target sensors
		updateTargetSensors(psensorMap, myTarget);
		
		// We select the activables classifiers
		for (itClassifiers = _Classifiers.begin();
		itClassifiers != _Classifiers.end();
		itClassifiers++)
		{
			// We first check if the classifier priority is higher than maxPriorityDoingTheSameAction
			if ((*itClassifiers).second->Priority > maxPriorityDoingTheSameAction)
				if ( (*itClassifiers).second->isActivable())
				{
					higherPriority = std::max((*itClassifiers).second->Priority, higherPriority);
					mapActivableCS.insert(std::make_pair((*itClassifiers).second->Priority, std::make_pair(itClassifiers, myTarget)));
				}
		}
		RAZTargetSensors();
	}
	
	return higherPriority;
}

// Function used in selectBehavior
std::multimap<TClassifierPriority, std::pair<CClassifierSystem::TitClassifiers, TTargetId> >::iterator 
	CClassifierSystem::roulletteWheelVariation(std::multimap<TClassifierPriority, std::pair<TitClassifiers, TTargetId> > &mapActivableCS,
											   std::multimap<TClassifierPriority, std::pair<TitClassifiers, TTargetId> >::iterator itMapActivableCS)
{
	if (itMapActivableCS == mapActivableCS.begin())
	{
		return itMapActivableCS;
	}
	
	TClassifierPriority bestPrio = (*itMapActivableCS).first;
	std::multimap<TClassifierPriority, std::pair<TitClassifiers, TTargetId> >::iterator itMapActivableCSMinus = itMapActivableCS;
	itMapActivableCSMinus--;
	TClassifierPriority lowPrio = (*itMapActivableCSMinus).first;
	TClassifierPriority laSomme = bestPrio + lowPrio;
	nlassert (laSomme > 0.0001);
	TClassifierPriority randomeNumber = (rand()%(int(laSomme*1000.0)))/1000.0;
	if (randomeNumber > bestPrio )
	{
		return roulletteWheelVariation(mapActivableCS, itMapActivableCSMinus);
	}
	else
	{
		return itMapActivableCS;
	}
	
}

void CClassifierSystem::selectBehavior( const CCSPerception* psensorMap,
										std::multimap<TClassifierPriority, std::pair<TClassifierNumber, TTargetId> > &mapActivableCS)
{
	// We update the internal sensor values for the no target sensors
	updateNoTargetSensors(psensorMap);

	TitClassifiers itClassifiers;
	
	for (itClassifiers = _Classifiers.begin();
	itClassifiers != _Classifiers.end();
	itClassifiers++)
	{
		// S'il y a des conditions dépendantes d'une cible, ce n'est pas activable.
		if ((*itClassifiers).second->ConditionWithTarget.begin() == (*itClassifiers).second->ConditionWithTarget.end())
		{
			if ( (*itClassifiers).second->isActivable())
			{
				mapActivableCS.insert(std::make_pair((*itClassifiers).second->Priority, std::make_pair((*itClassifiers).first, NullTargetId)));
			}
		}
	}
	
	// We now do the same, but with target sensors.
	std::map<TTargetId, TSensorMap>::const_iterator itTargetSensorMap;
	for (itTargetSensorMap = psensorMap->TargetSensors.begin(); itTargetSensorMap != psensorMap->TargetSensors.end(); itTargetSensorMap++)
	{
		TTargetId myTarget = (*itTargetSensorMap).first;
		// We update the internal sensor values for the target sensors
		updateTargetSensors(psensorMap, myTarget);
		
		// We select the activables classifiers
		for (itClassifiers = _Classifiers.begin();
		itClassifiers != _Classifiers.end();
		itClassifiers++)
		{
			// S'il n'y a pas de conditions dépendantes d'une cible, ça a déjà été traité au dessus.
			if ((*itClassifiers).second->ConditionWithTarget.begin() != (*itClassifiers).second->ConditionWithTarget.end())
			{
				if ( (*itClassifiers).second->isActivable())
				{
					mapActivableCS.insert(std::make_pair((*itClassifiers).second->Priority, std::make_pair((*itClassifiers).first, myTarget)));
				}
			}
		}
		RAZTargetSensors();
	}
	
	// We set the sensors back to the default value.
	RAZNoTargetSensors();

//		// We set the return values.
//		lastClassifierNumber = selectionNumber;
//		lastTarget = myTarget;
//		lastSelectionMaxPriority = higherPriority;
}

TAction CClassifierSystem::getActionPart(TClassifierNumber classifierNumber)
{
	std::map<TClassifierNumber, CClassifier*>::iterator itClassifiers = _Classifiers.find(classifierNumber);
	nlassert(itClassifiers != _Classifiers.end());
	return (*itClassifiers).second->Behavior;
}

TClassifierPriority CClassifierSystem::getPriorityPart(TClassifierNumber classifierNumber)
{
	std::map<TClassifierNumber, CClassifier*>::iterator itClassifiers = _Classifiers.find(classifierNumber);
	nlassert(itClassifiers != _Classifiers.end());
	return (*itClassifiers).second->Priority;
}

void CClassifierSystem::getDebugString(std::string &t) const
{
	std::string dbg = "\n";

	std::map<TClassifierNumber, CClassifier*>::const_iterator itClassifiers;
	for (itClassifiers = _Classifiers.begin(); itClassifiers != _Classifiers.end(); itClassifiers++)
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
		TClassifierPriority		prio = (*itClassifiers).second->Priority;
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

bool CClassifierSystem::CClassifier::isActivable() const
{
	std::list<CClassifierConditionCell*>::const_iterator itConditions;
	
	// On parcour la liste de sensor indépendant d'une cible
	for (itConditions = ConditionWithoutTarget.begin(); itConditions != ConditionWithoutTarget.end(); itConditions++)
	{
		if (! (*itConditions)->isActivable() )
		{
			return false;
		}
	}
	// On parcour la liste de sensor dépendant d'une cible
	for (itConditions = ConditionWithTarget.begin(); itConditions != ConditionWithTarget.end(); itConditions++)
	{
		if (! (*itConditions)->isActivable() )
		{
			return false;
		}
	}
	return true;
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
CActionClassifiers::CActionClassifiers()
{
	_Name = Action_Unknown;
}

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

void CActionClassifiers::addMotivationRule (TMotivation motivationName, const CConditionMap &conditionsMap, TClassifierPriority priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByMotivation[motivationName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

void CActionClassifiers::addVirtualActionRule (TAction virtualActionName, const CConditionMap &conditionsMap, TClassifierPriority priority)
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



