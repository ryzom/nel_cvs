/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.12 2003/02/27 11:10:27 robert Exp $
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
	std::map<TSensor, std::pair<TSensorValue, bool> >::const_iterator itCondition;
	for (itCondition = conditionsMap.begin(); itCondition != conditionsMap.end(); itCondition++)
	{
		// We add the new sensor in the sensor map and init it with a joker value '#'
		_sensors[(*itCondition).first] = '#';

		// A new condition cell is added to the classifier condition.
		condCell = new CClassifierConditionCell(_sensors.find((*itCondition).first), (*itCondition).second.first, (*itCondition).second.second);
		classifier->Condition.push_back(condCell);
	}

	// The new classifier is added to the classifier list.
	_classifiers[_ClassifierNumber++] = classifier;
}

/// Merge two CS
void CClassifierSystem::addClassifierSystem(const CClassifierSystem &cs)
{
	std::map<sint16, CClassifier*>::const_iterator itCSClassifiers;
	for (itCSClassifiers = cs._classifiers.begin(); itCSClassifiers != cs._classifiers.end(); itCSClassifiers++)
	{
		CConditionMap conditionsMap;

		std::list<CClassifierConditionCell*>::const_iterator itCondCell;
		for (itCondCell = (*itCSClassifiers).second->Condition.begin(); itCondCell !=(*itCSClassifiers).second->Condition.end(); itCondCell++)
		{
			CClassifierConditionCell* pCondCell = (*itCondCell);
			conditionsMap.addSensorCondition(pCondCell->getSensorName(), pCondCell->getValue(),pCondCell->getSensorIsTrue());
		}
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
	TitClassifiers itClassifiers = _classifiers.begin();
	std::list<CClassifierConditionCell*>::iterator itConditions;
	bool activable;
	double totalPriority = 0;

	while (itClassifiers != _classifiers.end())
	{
		activable = true;
		itConditions = (*itClassifiers).second->Condition.begin();
		while (itConditions != (*itClassifiers).second->Condition.end())
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
			totalPriority += (*itClassifiers).second->Priority;
			mapCSweel[totalPriority] = std::make_pair(itClassifiers,myTarget);
		}
		itClassifiers++;
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
		itClassifiers = _classifiers.begin();
		while (itClassifiers != _classifiers.end())
		{
			activable = true;
			itConditions = (*itClassifiers).second->Condition.begin();
			while (itConditions != (*itClassifiers).second->Condition.end())
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
				totalPriority += (*itClassifiers).second->Priority;
				mapCSweel[totalPriority] = std::make_pair(itClassifiers,myTarget);
			}
			itClassifiers++;
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
		for (itConditions = (*itClassifiers).second->Condition.begin(); itConditions != (*itClassifiers).second->Condition.end(); itConditions++)
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

CClassifierSystem::CClassifierConditionCell::CClassifierConditionCell(TSensorMap::const_iterator itSensor, TSensorValue value, bool sensorIsTrue)
{
	_itSensor = itSensor;
	_value = value;
	_sensorIsTrue = sensorIsTrue;
}

bool CClassifierSystem::CClassifierConditionCell::isActivable() const
{
	if (_sensorIsTrue)
	{
		if ((*_itSensor).second == _value)
			return true;
		else
			return false;
	}
	else
	{
		if ((*_itSensor).second == _value)
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
	return _value;
}

bool CClassifierSystem::CClassifierConditionCell::getSensorIsTrue() const
{
	return _sensorIsTrue;
}

///////////////////////////
// CConditionMap
///////////////////////////

void CConditionMap::addIfSensorCondition(TSensor sensorName, TSensorValue sensorValue)
{
	_ConditionMap[sensorName] = std::make_pair(sensorValue, true);
}

void CConditionMap::addIfNotSensorCondition(TSensor sensorName, TSensorValue sensorValue)
{
	_ConditionMap[sensorName] = std::make_pair(sensorValue, false);
}

void CConditionMap::addSensorCondition(TSensor sensorName, TSensorValue sensorValue, bool sensorIsTrue)
{
	_ConditionMap[sensorName] = std::make_pair(sensorValue, sensorIsTrue);
}

///////////////////////////
// CActionCS
///////////////////////////
CActionCS::CActionCS(TAction name)
{
	_Name = name;
}

CActionCS::~CActionCS()
{
}

/// Return the action name
TAction CActionCS::getName() const
{
	return _Name;
}

void CActionCS::addMotivationRule (TMotivation motivationName, const CConditionMap &conditionsMap, double priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByMotivation[motivationName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

void CActionCS::addVirtualActionRule (TAction virtualActionName, const CConditionMap &conditionsMap, double priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByVirtualAction[virtualActionName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

const std::map<TMotivation, CClassifierSystem> *CActionCS::getClassifiersByMotivationMap () const
{
	return &_ClassifiersByMotivation;
}
const std::map<TAction, CClassifierSystem> *CActionCS::getClassifiersByVirtualActionMap () const
{
	return &_ClassifiersByVirtualAction;
}

/// Chaine de debug
void CActionCS::getDebugString(std::string &t) const
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
// CMotivationEnergy
///////////////////////////
CMotivationEnergy::CMotivationEnergy()
{
	_SumValue = 0;
}

CMotivationEnergy::~CMotivationEnergy()
{
}

double CMotivationEnergy::getSumValue() const
{
	return _SumValue;
}

void CMotivationEnergy::removeProvider(TMotivation providerName)
{
	_MotivationProviders.erase(providerName);
	computeMotivationValue();
}

void CMotivationEnergy::removeProvider(TAction providerName)
{
	_VirtualActionProviders.erase(providerName);
	computeMotivationValue();
}

void CMotivationEnergy::addProvider(TMotivation providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::addProvider(TAction providerName, const CMotivationEnergy& providerMotivation)
{
	_VirtualActionProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::updateProvider(TMotivation providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::updateProvider(TAction providerName, const CMotivationEnergy& providerMotivation)
{
	_VirtualActionProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::computeMotivationValue()
{
	_EnergyByMotivation.clear();

	// We look for motivation values comming directly from Motivations
	std::map<TMotivation, TEnergyByMotivation>::iterator itMotivationProviders;
	for (itMotivationProviders = _MotivationProviders.begin(); itMotivationProviders != _MotivationProviders.end(); itMotivationProviders++)
	{
		TEnergyByMotivation &motivation = (*itMotivationProviders).second;
		TEnergyByMotivation::iterator itMotivation, itMyMotivation;
		for (itMotivation = motivation.begin(); itMotivation != motivation.end(); itMotivation++)
		{
			TMotivation motivSource = (*itMotivation).first;
			double motiveValue = (*itMotivation).second.Value;
			double motivePP = (*itMotivation).second.PP;
			itMyMotivation = _EnergyByMotivation.find(motivSource);
			if (itMyMotivation != _EnergyByMotivation.end())
			{
				double myMotiveValue = (*itMyMotivation).second.Value;
				if (motiveValue > myMotiveValue)
				{
					_EnergyByMotivation[motivSource].Value = motiveValue;
					_EnergyByMotivation[motivSource].PP = motivePP;
				}
			}
			else
			{
				_EnergyByMotivation[motivSource].Value = motiveValue;
				_EnergyByMotivation[motivSource].PP = motivePP;
			}
		}
	}

	// We look for motivation values comming from virtual actions
	std::map<TAction, TEnergyByMotivation>::iterator itVirtualActionProviders;
	for (itVirtualActionProviders = _VirtualActionProviders.begin(); itVirtualActionProviders != _VirtualActionProviders.end(); itVirtualActionProviders++)
	{
		TEnergyByMotivation &motivation = (*itVirtualActionProviders).second;
		TEnergyByMotivation::iterator itMotivation, itMyMotivation;
		for (itMotivation = motivation.begin(); itMotivation != motivation.end(); itMotivation++)
		{
			TMotivation motivSource = (*itMotivation).first;
			double motiveValue = (*itMotivation).second.Value;
			double motivePP = (*itMotivation).second.PP;
			itMyMotivation = _EnergyByMotivation.find(motivSource);
			if (itMyMotivation != _EnergyByMotivation.end())
			{
				double myMotiveValue = (*itMyMotivation).second.Value;
				if (motiveValue > myMotiveValue)
				{
					_EnergyByMotivation[motivSource].Value = motiveValue;
					_EnergyByMotivation[motivSource].PP = motivePP;
				}
			}
			else
			{
				_EnergyByMotivation[motivSource].Value = motiveValue;
				_EnergyByMotivation[motivSource].PP = motivePP;
			}
		}
	}

	TEnergyByMotivation::const_iterator itEnergyByMotivation;
	double sum = 0;
	for (itEnergyByMotivation = _EnergyByMotivation.begin(); itEnergyByMotivation != _EnergyByMotivation.end(); itEnergyByMotivation++)
	{
		sum += (*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP;
	}
	_SumValue = sum;
	nlassert(_SumValue >= 0);
}

/// Donne la Puissance Propre d'une Motivation
void CMotivationEnergy::setMotivationPP(TMotivation motivationName, double PP)
{
	_SumValue -= _EnergyByMotivation[motivationName].Value * _EnergyByMotivation[motivationName].PP;
	_SumValue += _EnergyByMotivation[motivationName].Value * PP;
	_EnergyByMotivation[motivationName].PP = PP;
	_MotivationProviders[motivationName][motivationName].PP = PP;
	nlassert(_SumValue >= 0);
}

/// Fixe la valeur d'une motivation
void CMotivationEnergy::setMotivationValue(TMotivation motivationName, double value)
{
	_SumValue -= _EnergyByMotivation[motivationName].Value * _EnergyByMotivation[motivationName].PP;
	_SumValue += value * _EnergyByMotivation[motivationName].PP;
	_EnergyByMotivation[motivationName].Value = value;
	_MotivationProviders[motivationName][motivationName].Value = value;
	nlassert(_SumValue >= 0);
}

/// Chaine de debug
void CMotivationEnergy::getDebugString(std::string &t) const
{
	std::string ret;
	TEnergyByMotivation::const_iterator itEnergyByMotivation;
	
	for (itEnergyByMotivation = _EnergyByMotivation.begin(); itEnergyByMotivation!= _EnergyByMotivation.end(); itEnergyByMotivation++)
	{
		ret += " " + conversionMotivation.toString((*itEnergyByMotivation).first) + " (" + NLMISC::toString((*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP) + ") ";
	}
	t+=ret;
}

///////////////////////////
// CMHiCSagent
///////////////////////////

CMHiCSagent::CMHiCSagent(CMHiCSbase* pMHiCSbase)
{
	nlassert (pMHiCSbase != NULL);
	_pMHiCSbase = pMHiCSbase;
	_IdByActions[Action_DoNothing] = NullTargetId;
}

CMHiCSagent::~CMHiCSagent()
{
}

/// Chaine de debug
void CMHiCSagent::getDebugString(std::string &t) const
{
	std::string ret = "\n\n---------------------------";
	ret += "\nMotivations :";
	std::map<TMotivation, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity;
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		ret += "\n " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.dbgNumberOfActivations) + "<" + conversionMotivation.toString((*itClassifiersAndMotivationIntensity).first) + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getSumValue()) + "] :";
		(*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getDebugString(ret);
		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
	ret += "\nVirtual Actions :";
	std::map<TAction, CMotivateCS>::const_iterator itClassifiersAndVirtualActionIntensity;
	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
		 itClassifiersAndVirtualActionIntensity++)
	{
		ret += "\n " + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.dbgNumberOfActivations) + "<" + conversionAction.toString((*itClassifiersAndVirtualActionIntensity).first) + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getSumValue()) + "] :";
		(*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getDebugString(ret);
		std::map<TAction, TTargetId>::const_iterator itIdByActions = _IdByActions.find((*itClassifiersAndVirtualActionIntensity).first);
		nlassert (itIdByActions != _IdByActions.end())
		ret += " on target n#" + NLMISC::toString((*itIdByActions).second);
		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
	ret += "\nACTIONS :";
	std::map<TAction, CMotivationEnergy>::const_iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		ret += "\n <" + conversionAction.toString((* itActionsExecutionIntensity).first) + "> [EI=" + NLMISC::toString((*itActionsExecutionIntensity).second.getSumValue()) + "] : ";
		(*itActionsExecutionIntensity).second.getDebugString(ret);
		std::map<TAction, TTargetId>::const_iterator itIdByActions = _IdByActions.find((*itActionsExecutionIntensity).first);
		nlassert (itIdByActions != _IdByActions.end())
		ret += " on target n#" + NLMISC::toString((*itIdByActions).second);
	}
	t+=ret;
}

/// Donne la Puissance Propre d'une Motivation
void CMHiCSagent::setMotivationPP(TMotivation motivationName, double PP)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationPP(motivationName, PP);
	spreadMotivationReckon(motivationName);
}

/// Fixe la valeur d'une motivation
void CMHiCSagent::setMotivationValue(TMotivation motivationName, double value)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationValue(motivationName, value);
	spreadMotivationReckon(motivationName);
}

void CMHiCSagent::spreadMotivationReckon(TMotivation CS)
{
	std::map<TMotivation, CMotivateCS>::iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(CS);
	nlassert(itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end());
	sint16 lastClassifierNumber = (*itClassifiersAndMotivationIntensity).second.ClassifierNumber;
	if (lastClassifierNumber >=0 )
	{
		TAction lastActionName = _pMHiCSbase->getActionPart(CS, lastClassifierNumber);
		if (_pMHiCSbase->isAnAction(lastActionName))
		{
			_ActionsExecutionIntensity[lastActionName].updateProvider(CS, (*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
			// If the action doesn't receive motivation any more, we remove it.
			double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
			if (energy <= 0)
			{
				_ActionsExecutionIntensity.erase(lastActionName);
			}
		}
		else
		{
			_ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.updateProvider(CS, (*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
			spreadMotivationReckon(lastActionName);
			// If the CS doesn't receive motivation any more, we remove it.
			double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.getSumValue();
			if (energy <= 0)
			{
				_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
			}
		}
	}
}

void CMHiCSagent::spreadMotivationReckon(TAction CS)
{
	std::map<TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.find(CS);
	nlassert(itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end());
	sint16 lastClassifierNumber = (*itClassifiersAndVirtualActionIntensity).second.ClassifierNumber;
	if (lastClassifierNumber >=0 )
	{
		TAction lastActionName = _pMHiCSbase->getActionPart(CS, lastClassifierNumber);
		if (_pMHiCSbase->isAnAction(lastActionName))
		{
			_ActionsExecutionIntensity[lastActionName].updateProvider(CS, (*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity);
			// If the action doesn't receive motivation any more, we remove it.
			double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
			if (energy <= 0)
			{
				_ActionsExecutionIntensity.erase(lastActionName);
			}
		}
		else
		{
			_ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.updateProvider(CS, (*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity);
			spreadMotivationReckon(lastActionName);
			// If the CS doesn't receive motivation any more, we remove it.
			double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.getSumValue();
			if (energy <= 0)
			{
				_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
			}
		}
	}
}


void CMHiCSagent::motivationCompute()
{
	/*
	Je sélectionne par roulette weel la motivation que je vais gérer
	Je met à jour l'énergie du vainqueur
	*/
	double somme = 0;
	typedef	std::map<TMotivation, CMotivateCS>::iterator TitNameAndMotivation;
	std::map<double, TitNameAndMotivation > mapCSweel;
	std::map<TMotivation, CMotivateCS>::iterator itClassifiersAndMotivationIntensity;
	// On calcule la somme
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		CMotivateCS* pCMotivateCS = &((*itClassifiersAndMotivationIntensity).second);
		double energy = pCMotivateCS->MotivationIntensity.getSumValue();
		if (energy > 0)
		{
			somme += energy;
			mapCSweel[somme] = itClassifiersAndMotivationIntensity;
		}
	}
	if (somme>0)
	{
		// on selectionne le classeur;
		double randomeNumber = (rand()%(int(somme*100)))/100.0;
		std::map<double, TitNameAndMotivation>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
		CMotivateCS* pCSselection = &((*((*itMapCSweel).second)).second);
		TMotivation selectionName = (*((*itMapCSweel).second)).first;

		// Updating the number of activation counter
		pCSselection->dbgNumberOfActivations++;

		// On fait calculer le CS
		std::pair<sint16, TTargetId> mySelection = _pMHiCSbase->selectBehavior(selectionName,_pSensorsValues);
		sint16 selectedClassifierNumber = mySelection.first;
		if (selectedClassifierNumber < 0) return; // ***G*** Ici on décide de rien faire si on sait pas quoi faire. En fait il faudrait créer un règle.
		TAction behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);

		// We check the last action selected by the current motivation to remove the motivation influence on this action.
		sint16 lastClassifierNumber = _ClassifiersAndMotivationIntensity[selectionName].ClassifierNumber;
		if (lastClassifierNumber >= 0)
		{
			TAction lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);

			// We check if we have selected the same behavior.
			if (lastActionName != behav)
			{
				if (_pMHiCSbase->isAnAction(lastActionName))
				{
					_ActionsExecutionIntensity[lastActionName].removeProvider(selectionName);
					// If the action doesn't receive motivation any more, we remove it.
					double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
					if (energy <= 0)
					{
						_ActionsExecutionIntensity.erase(lastActionName);
					}
				}
				else
				{
					_ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.removeProvider(selectionName);
					spreadMotivationReckon(lastActionName);
					// If the CS doesn't receive motivation any more, we remove it.
					double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.getSumValue();
					if (energy <= 0)
					{
						_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
					}
				}
			}
		}

		// We store the number of the new classifier actived by this motivation.
		_ClassifiersAndMotivationIntensity[selectionName].ClassifierNumber = selectedClassifierNumber;

		// We add the current motivation energy to the selected action.
		if (_pMHiCSbase->isAnAction(behav))
		{
			_ActionsExecutionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
		}
		else
		{
			// Else it must be a virtual action (common CS)
			_ClassifiersAndVirtualActionIntensity[behav].MotivationIntensity.addProvider(selectionName, pCSselection->MotivationIntensity);
			spreadMotivationReckon(behav);
		}
		
		// We set the Id of this action.
		// For moment there's no test to see if it is the same target or not. In the futur it can be usefull to make this test
		// to avoid unwilled target switch.
		_IdByActions[behav] = mySelection.second;
	}
}


void CMHiCSagent::virtualActionCompute()
{
	/*
	Je sélectionne par roulette weel l'action virtuel que je vais gérer
	Je met à jour l'énergie du vainqueur
	*/
	double somme = 0;
	typedef	std::map<TAction, CMotivateCS>::iterator TitNameAndVirtualAction;
	std::map<double, TitNameAndVirtualAction > mapCSweel;
	std::map<TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity;
	// On calcule la somme
	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
		 itClassifiersAndVirtualActionIntensity++)
	{
		CMotivateCS* pCMotivateCS = &((*itClassifiersAndVirtualActionIntensity).second);
		double energy = pCMotivateCS->MotivationIntensity.getSumValue();
		if (energy > 0)
		{
			somme += energy;
			mapCSweel[somme] = itClassifiersAndVirtualActionIntensity;
		}
	}
	if (somme>0)
	{
		// on selectionne le classeur;
		double randomeNumber = (rand()%(int(somme*100)))/100.0;
		std::map<double, TitNameAndVirtualAction>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
		CMotivateCS* pCSselection = &((*((*itMapCSweel).second)).second);
		TAction selectionName = (*((*itMapCSweel).second)).first;

		// Updating the number of activation counter
		pCSselection->dbgNumberOfActivations++;

		// Get the target Id for this Virtual Action
		std::map<TAction, TTargetId>::const_iterator itIdByActions = _IdByActions.find(selectionName);
		nlassert (itIdByActions != _IdByActions.end())
		TTargetId myTarget = (*itIdByActions).second;

		// On fait calculer le CS
		std::pair<sint16, TTargetId> mySelection = _pMHiCSbase->selectBehavior(selectionName,_pSensorsValues, myTarget);
		sint16 selectedClassifierNumber = mySelection.first;
		if (selectedClassifierNumber < 0) return; // ***G*** Ici on décide de rien faire si on sait pas quoi faire. En fait il faudrait créer un règle.
		TAction behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);

		// We check the last action selected by the current motivation to remove the motivation influence on this action.
		sint16 lastClassifierNumber = _ClassifiersAndVirtualActionIntensity[selectionName].ClassifierNumber;
		if (lastClassifierNumber >= 0)
		{
			TAction lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);

			// We check if we have selected the same behavior.
			if (lastActionName != behav)
			{
				if (_pMHiCSbase->isAnAction(lastActionName))
				{
					_ActionsExecutionIntensity[lastActionName].removeProvider(selectionName);
					// If the action doesn't receive motivation any more, we remove it.
					double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
					if (energy <= 0)
					{
						_ActionsExecutionIntensity.erase(lastActionName);
					}
				}
				else
				{
					_ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.removeProvider(selectionName);
					spreadMotivationReckon(lastActionName);
					// If the CS doesn't receive motivation any more, we remove it.
					double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.getSumValue();
					if (energy <= 0)
					{
						_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
					}
				}
			}
		}

		// We store the number of the new classifier actived by this motivation.
		_ClassifiersAndVirtualActionIntensity[selectionName].ClassifierNumber = selectedClassifierNumber;

		// We add the current motivation energy to the selected action.
		if (_pMHiCSbase->isAnAction(behav))
		{
			_ActionsExecutionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
		}
		else
		{
			// Else it must be a virtual action (common CS)
			_ClassifiersAndVirtualActionIntensity[behav].MotivationIntensity.addProvider(selectionName, pCSselection->MotivationIntensity);
			spreadMotivationReckon(behav);
		}

		// We set the Id of this action.
		// For moment there's no test to see if it is the same target or not. In the futur it can be usefull to make this test
		// to avoid unwilled target switch.
		_IdByActions[behav] = mySelection.second;
	}
}

void CMHiCSagent::run()
{
	motivationCompute();
	virtualActionCompute();
}


void CMHiCSagent::setSensors(CCSPerception* psensorMap)
{
	_pSensorsValues = psensorMap;
}


std::pair<TAction, TTargetId>CMHiCSagent::selectBehavior()
{
	// On prend le max
	TAction retAction = Action_DoNothing;
	double executionIntensity = 0;
	std::map<TAction, CMotivationEnergy>::iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		double value = (*itActionsExecutionIntensity).second.getSumValue();
		if (value > executionIntensity)
		{
			executionIntensity = value;
			retAction = (*itActionsExecutionIntensity).first;
		}
	}
	std::map<TAction, TTargetId>::iterator itIdByActions = _IdByActions.find(retAction);
	nlassert (itIdByActions != _IdByActions.end()) // There's no activable action
	_ItCurrentAction = itIdByActions;

	return (*itIdByActions);
}

/// Inform the MHiCSAgent that an action ended
void CMHiCSagent::behaviorTerminate(TBehaviorTerminate how_does_it_terminate)
{
	// ***G*** Tant qu'il n'y a pas d'apprentissage, on se contente de retirer l'action de la liste.
	// Je pense qu'il faut aussi remettre en question toute les actions virtuel portant sur cette cible.
	// Remettre en question signifie qu'il faut gardr une trace pour favoriser la continuité d'action sur un perso.
	TTargetId maCibleRemiseEnQuestion = (*_ItCurrentAction).second;
	std::map<TAction, TTargetId>::iterator	itIdByActions;
	for (itIdByActions = _IdByActions.begin(); itIdByActions != _IdByActions.end(); itIdByActions++)
	{
		TTargetId scanedId = (*itIdByActions).second;
		if ( scanedId == maCibleRemiseEnQuestion ) 
		{
			TAction theAction = (*itIdByActions).first;
			// Removing from action
			_IdByActions.erase(itIdByActions);

			// Removing the virtual_classifier that may be associate
			_ClassifiersAndVirtualActionIntensity.erase(theAction);

			// Removing from the actionExecutionIntensity
			_ActionsExecutionIntensity.erase(theAction);
		}
	}
	run();
	//_IdByActions.erase(_ItCurrentAction);
}


///////////////////////////
// CMHiCSbase
///////////////////////////

CMHiCSbase::CMHiCSbase()
{
}

CMHiCSbase::~CMHiCSbase()
{
}

void CMHiCSbase::addVirtualActionCS(const CActionCS &action)
{
	const std::map<TMotivation, CClassifierSystem> *mapActionByMotivation = action.getClassifiersByMotivationMap();
	std::map<TMotivation, CClassifierSystem>::const_iterator ItMapActionByMotivation;
	for (ItMapActionByMotivation = mapActionByMotivation->begin(); ItMapActionByMotivation != mapActionByMotivation->end(); ItMapActionByMotivation++)
	{
		CClassifierSystem* pCS;
		TMotivation motivationName = (*ItMapActionByMotivation).first;
		const CClassifierSystem* pOtherCS = &((*ItMapActionByMotivation).second);

		pCS = &(_MotivationClassifierSystems[motivationName]);
		pCS->addClassifierSystem(*pOtherCS);
	}

	const std::map<TAction, CClassifierSystem> *mapActionByVirtualAction = action.getClassifiersByVirtualActionMap();
	std::map<TAction, CClassifierSystem>::const_iterator ItMapActionByVirtualAction;
	for (ItMapActionByVirtualAction = mapActionByVirtualAction->begin(); ItMapActionByVirtualAction != mapActionByVirtualAction->end(); ItMapActionByVirtualAction++)
	{
		CClassifierSystem* pCS;
		TAction virtualActionName = (*ItMapActionByVirtualAction).first;
		const CClassifierSystem* pOtherCS = &((*ItMapActionByVirtualAction).second);

		pCS = &(_VirtualActionClassifierSystems[virtualActionName]);
		pCS->addClassifierSystem(*pOtherCS);
	}
}

void CMHiCSbase::addActionCS(const CActionCS& action)
{
	addVirtualActionCS(action);
	_ActionSet.insert(action.getName());
}

std::pair<sint16, TTargetId> CMHiCSbase::selectBehavior(TMotivation motivationName, const CCSPerception* psensorMap)
{
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.selectBehavior(psensorMap);
}

std::pair<sint16, TTargetId> CMHiCSbase::selectBehavior(TAction motivationName, const CCSPerception* psensorMap, TTargetId target)
{
	std::map<TAction, CClassifierSystem>::iterator itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.find(motivationName);
	nlassert(itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end());
	CCSPerception neoPerception;
	neoPerception.NoTargetSensors = psensorMap->NoTargetSensors;
	std::map<TTargetId, TSensorMap>::const_iterator	itSensorMap = psensorMap->TargetSensors.find(target);
	nlassert(itSensorMap != psensorMap->TargetSensors.end())
	neoPerception.TargetSensors[target] = (*itSensorMap).second;
	return (*itVirtualActionClassifierSystems).second.selectBehavior(&neoPerception);
}

TAction CMHiCSbase::getActionPart(TMotivation motivationName, sint16 classifierNumber)
{
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.getActionPart(classifierNumber);
}

TAction CMHiCSbase::getActionPart(TAction motivationName, sint16 classifierNumber)
{
	std::map<TAction, CClassifierSystem>::iterator itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.find(motivationName);
	nlassert(itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end());
	return (*itVirtualActionClassifierSystems).second.getActionPart(classifierNumber);
}

bool CMHiCSbase::isAnAction(TAction behav) const
{
	std::set<TAction>::const_iterator itActionSet = _ActionSet.find(behav);
	return (itActionSet != _ActionSet.end());
}

/// Chaine de debug
void CMHiCSbase::getDebugString(std::string &t) const
{
	std::string ret = "\n---------------------------";
	std::map<TMotivation, CClassifierSystem>::const_iterator itMotivationClassifierSystems;
	for (itMotivationClassifierSystems = _MotivationClassifierSystems.begin(); itMotivationClassifierSystems != _MotivationClassifierSystems.end(); itMotivationClassifierSystems++)
	{
		ret += "\nMotivation : " + conversionMotivation.toString((*itMotivationClassifierSystems).first);
		(*itMotivationClassifierSystems).second.getDebugString(ret);
	}
	std::map<TAction, CClassifierSystem>::const_iterator itVirtualActionClassifierSystems;
	for (itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.begin(); itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end(); itVirtualActionClassifierSystems++)
	{
		ret += "\nVirtual Action : " + conversionAction.toString((*itVirtualActionClassifierSystems).first);
		(*itVirtualActionClassifierSystems).second.getDebugString(ret);
	}
	ret += "\nACTIONS :\n";
	std::set<TAction>::const_iterator itActionSet;
	for (itActionSet = _ActionSet.begin(); itActionSet != _ActionSet.end(); itActionSet++)
	{
		ret += conversionAction.toString((*itActionSet)) + "\n";
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
