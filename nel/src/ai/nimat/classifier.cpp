/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.7 2002/12/05 18:28:51 robert Exp $
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
#include "nel/misc/debug.h"

namespace NLAINIMAT
{

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

void CClassifierSystem::addClassifier(const TSensorMap &conditionsMap, sint16 priority, const char* behavior)
{
	// We build a new classifier.
	CClassifier* classifier = new CClassifier();
	classifier->Behavior = behavior;
	classifier->Priority = priority;

	CClassifierConditionCell* condCell;
	std::map<std::string, char>::const_iterator itCondition;
	for (itCondition = conditionsMap.begin(); itCondition != conditionsMap.end(); itCondition++)
	{
		// We add the new sensor in the sensor map and init it with a joker value '#'
		_sensors[(*itCondition).first] = '#';

		// A new condition cell is added to the classifier condition.
		condCell = new CClassifierConditionCell(_sensors.find((*itCondition).first), (*itCondition).second);
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
		TSensorMap conditionsMap;

		std::list<CClassifierConditionCell*>::const_iterator itCondCell;
		for (itCondCell = (*itCSClassifiers).second->Condition.begin(); itCondCell !=(*itCSClassifiers).second->Condition.end(); itCondCell++)
		{
			CClassifierConditionCell* pCondCell = (*itCondCell);
			conditionsMap[pCondCell->getSensorName()] = pCondCell->getValue();
		}
		addClassifier(conditionsMap, (*itCSClassifiers).second->Priority, (*itCSClassifiers).second->Behavior.c_str());
	}
}

sint16 CClassifierSystem::selectBehavior( const TSensorMap &sensorMap)
{
	// We update the internal sensor values.
	std::map<std::string, char>::const_iterator itConditionsmap;
	for (itConditionsmap = sensorMap.begin(); itConditionsmap != sensorMap.end(); itConditionsmap++)
	{
		char c =  (*itConditionsmap).second;
		std::string sensName = (*itConditionsmap).first;
		_sensors[sensName ] = c;
	}

	// We select the activables classifiers
	typedef	std::map<sint16, CClassifier*>::iterator  TitClassifiers;
	std::map<sint16, TitClassifiers> mapCSweel;
	std::map<sint16, CClassifier*>::iterator itClassifiers = _classifiers.begin();
	std::list<CClassifierConditionCell*>::iterator itConditions;
	bool activable;
	int totalPriority = 0;

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
			mapCSweel[totalPriority] = itClassifiers;
		}
		itClassifiers++;
	}

	// We set the sensors back to the default value.
	for (itConditionsmap = sensorMap.begin(); itConditionsmap != sensorMap.end(); itConditionsmap++)
	{
		std::string sensName = (*itConditionsmap).first;
		_sensors[sensName ] = '#';
	}

	// If totalPriority == 0, there's no activable classifier. ***G*** But here we must add a rule creation mechanisme.
	if(totalPriority>0)
	{
		// We select a classifier in the active classifier with a roullette wheel random.
		sint16 randomeNumber = rand()%(totalPriority);
		std::map<sint16, TitClassifiers>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
		CClassifier* pClassifierSelection = (*((*itMapCSweel).second)).second;
		sint16 selectionNumber = (*((*itMapCSweel).second)).first;

		return selectionNumber;
	}
	else
	{
		return -1;
	}
}

std::string CClassifierSystem::getActionPart(sint16 classifierNumber)
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
			dbg += " (" + condCell->getSensorName() + "=" + condCell->getValue() + ") +";
		}
		std::string actionName = (*itClassifiers).second->Behavior;
		sint16		prio = (*itClassifiers).second->Priority;
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

CClassifierSystem::CClassifierConditionCell::CClassifierConditionCell(TSensorMap::const_iterator itSensor, char value)
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

std::string CClassifierSystem::CClassifierConditionCell::getSensorName() const
{
	return (*_itSensor).first;
}

char CClassifierSystem::CClassifierConditionCell::getValue()
{
	return _value;
}

///////////////////////////
// CActionCS
///////////////////////////
CActionCS::CActionCS(std::string name)
{
	_Name = name;
}

CActionCS::~CActionCS()
{
}

/// Return the action name
std::string CActionCS::getName() const
{
	return _Name;
}

/// Ajout d'une nouvelle règle motivant cette action
void CActionCS::addMotivationRule (std::string motivationName, const TSensorMap &conditionsMap, sint16 priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByMotivation[motivationName]);
	pCS->addClassifier(conditionsMap, priority, _Name.c_str());
}

const std::map<std::string, CClassifierSystem> *CActionCS::getClassifiersMap () const
{
	return &_ClassifiersByMotivation;
}

/// Chaine de debug
void CActionCS::getDebugString(std::string &t) const
{
	std::string ret = "\nACTION :\t" + _Name + "\n";
	std::map<std::string, CClassifierSystem>::const_iterator ItClassifiersByMotivation;
	for (ItClassifiersByMotivation = _ClassifiersByMotivation.begin(); ItClassifiersByMotivation != _ClassifiersByMotivation.end(); ItClassifiersByMotivation++)
	{
		ret += "\nMotivation : " + (*ItClassifiersByMotivation).first + "\n";
		(*ItClassifiersByMotivation).second.getDebugString(ret);
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

sint16 CMotivationEnergy::getSumValue() const
{
	return _SumValue;
}

void	CMotivationEnergy::removeProvider(std::string providerName)
{
	_MotivationProviders.erase(providerName);
	computeMotivationValue();
}

void	CMotivationEnergy::addProvider(std::string providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void	CMotivationEnergy::computeMotivationValue()
{
	_EnergyByMotivation.clear();
	std::map<std::string, TEnergyByMotivation>::iterator itMotivationProviders;

	for (itMotivationProviders = _MotivationProviders.begin(); itMotivationProviders != _MotivationProviders.end(); itMotivationProviders++)
	{
		TEnergyByMotivation &motivation = (*itMotivationProviders).second;
		TEnergyByMotivation::iterator itMotivation, itMyMotivation;
		for (itMotivation = motivation.begin(); itMotivation != motivation.end(); itMotivation++)
		{
			std::string motivSource = (*itMotivation).first;
			sint16 motiveValue = (*itMotivation).second.Value;
			sint16 motivePP = (*itMotivation).second.PP;
			itMyMotivation = _EnergyByMotivation.find(motivSource);
			if (itMyMotivation != _EnergyByMotivation.end())
			{
				sint16 myMotiveValue = (*itMyMotivation).second.Value;
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
	sint16 sum = 0;
	for (itEnergyByMotivation = _EnergyByMotivation.begin(); itEnergyByMotivation != _EnergyByMotivation.end(); itEnergyByMotivation++)
	{
		sum += (*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP;
	}
	_SumValue = sum;
}

/// Donne la Puissance Propre d'une Motivation
void CMotivationEnergy::setMotivationPP(std::string motivationName, sint16 PP)
{
	_SumValue -= _EnergyByMotivation[motivationName].Value * _EnergyByMotivation[motivationName].PP;
	_SumValue += _EnergyByMotivation[motivationName].Value * PP;
	_EnergyByMotivation[motivationName].PP = PP;
	_MotivationProviders[motivationName][motivationName].PP = PP;
}

/// Fixe la valeur d'une motivation
void CMotivationEnergy::setMotivationValue(std::string motivationName, sint16 value)
{
	_SumValue -= _EnergyByMotivation[motivationName].Value * _EnergyByMotivation[motivationName].PP;
	_SumValue += value * _EnergyByMotivation[motivationName].PP;
	_EnergyByMotivation[motivationName].Value = value;
	_MotivationProviders[motivationName][motivationName].Value = value;
}

/// Chaine de debug
void CMotivationEnergy::getDebugString(std::string &t) const
{
	std::string ret;
	TEnergyByMotivation::const_iterator itEnergyByMotivation;
	
	for (itEnergyByMotivation = _EnergyByMotivation.begin(); itEnergyByMotivation!= _EnergyByMotivation.end(); itEnergyByMotivation++)
	{
		ret += " Motivation source : " + (*itEnergyByMotivation).first + " (" + NLMISC::toString((*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP) + ")\n";
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
}

CMHiCSagent::~CMHiCSagent()
{
}

/// Chaine de debug
void CMHiCSagent::getDebugString(std::string &t) const
{
	std::string ret = "\n\n---------------------------";
	std::map<std::string, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity;
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		ret += "\nMotivation name : " + (*itClassifiersAndMotivationIntensity).first;
		ret += "\n";
		(*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getDebugString(ret);
		ret += "Rule number " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.ClassifierNumber); 
		//***G*** J'ai plus qu'à affiché la règle du classeur en plus du numéro.
	}
	ret += "\nACTIONS :\n";
	std::map<std::string, CMotivationEnergy>::const_iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		ret += (* itActionsExecutionIntensity).first + " :\n";
		(*itActionsExecutionIntensity).second.getDebugString(ret);
	}
	t+=ret;
}

/// Donne la Puissance Propre d'une Motivation
void CMHiCSagent::setMotivationPP(std::string motivationName, sint16 PP)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationPP(motivationName, PP);
}

/// Fixe la valeur d'une motivation
void CMHiCSagent::setMotivationValue(std::string motivationName, sint16 value)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationValue(motivationName, value);
}

void CMHiCSagent::run()
{
	/*
	Je sélectionne par roulette weel le classeur que je vais gérer
	Je met à jour l'énergie du vainqueur
	*/
	sint16 somme = 0;
	typedef	std::map<std::string, CMotivateCS>::iterator TitNameAndMotivation;
	std::map<sint16, TitNameAndMotivation > mapCSweel;
	std::map<std::string, CMotivateCS>::iterator itClassifiersAndMotivationIntensity;
	// On calcule la somme
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		CMotivateCS* pCMotivateCS = &((*itClassifiersAndMotivationIntensity).second);
		sint16 energy = pCMotivateCS->MotivationIntensity.getSumValue();
		if (energy > 0)
		{
			somme += energy;
			mapCSweel[somme] = itClassifiersAndMotivationIntensity;
		}
	}
	if (somme>0)
	{
		// on selectionne le classeur;
		sint16 randomeNumber = rand()%(somme);
		std::map<sint16, TitNameAndMotivation>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
		CMotivateCS* pCSselection = &((*((*itMapCSweel).second)).second);
		std::string selectionName = (*((*itMapCSweel).second)).first;

		// On fait calculer le CS
		sint16 selectedClassifierNumber = _pMHiCSbase->selectBehavior(selectionName,_SensorsValues);
		std::string behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);

		// We check the last action selected by the current motivation to remove the motivation influence on this action.
		sint16 lastClassifierNumber = _ClassifiersAndMotivationIntensity[selectionName].ClassifierNumber;
		if (lastClassifierNumber >=0 )
		{
			std::string lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);
			if (_pMHiCSbase->isAnAction(lastActionName))
			{
				_ActionsExecutionIntensity[lastActionName].removeProvider(selectionName);
				// ***G*** Retirer l'action si l'energie est null
			}
			else
			{
				_ClassifiersAndMotivationIntensity[lastActionName].MotivationIntensity.removeProvider(selectionName);
				// ***G*** Retirer l'action si l'energie est null
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
			_ClassifiersAndMotivationIntensity[behav].MotivationIntensity.addProvider(selectionName, pCSselection->MotivationIntensity);
		}
	}
}

void CMHiCSagent::setSensors(const TSensorMap &sensorMap)
{
	_SensorsValues = sensorMap;
}


std::string CMHiCSagent::selectBehavior()
{
	// On prend le max
	std::string ret = "";
	sint16 executionIntensity = 0;
	std::map<std::string, CMotivationEnergy>::iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		sint16 value = (*itActionsExecutionIntensity).second.getSumValue();
		if (value > executionIntensity)
		{
			executionIntensity = value;
			ret = (*itActionsExecutionIntensity).first;
		}
	}
	return ret;
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
	const std::map<std::string, CClassifierSystem> *mapAction = action.getClassifiersMap();
	std::map<std::string, CClassifierSystem>::const_iterator ItMapAction;
	for (ItMapAction = mapAction->begin(); ItMapAction != mapAction->end(); ItMapAction++)
	{
		CClassifierSystem* pCS;
		std::string motivationName = (*ItMapAction).first;
		const CClassifierSystem* pOtherCS = &((*ItMapAction).second);

		pCS = &(_ClassifierSystems[motivationName]);
		pCS->addClassifierSystem(*pOtherCS);
	}
}

void CMHiCSbase::addActionCS(const CActionCS& action)
{
	addVirtualActionCS(action);
	_ActionSet.insert(action.getName());
}

sint16 CMHiCSbase::selectBehavior(std::string motivationName, const TSensorMap &sensorMap)
{
	std::map<std::string, CClassifierSystem>::iterator itClassifierSystems = _ClassifierSystems.find(motivationName);
	nlassert(itClassifierSystems != _ClassifierSystems.end());
	return (*itClassifierSystems).second.selectBehavior(sensorMap);
}

std::string CMHiCSbase::getActionPart(std::string motivationName, sint16 classifierNumber)
{
	std::map<std::string, CClassifierSystem>::iterator itClassifierSystems = _ClassifierSystems.find(motivationName);
	nlassert(itClassifierSystems != _ClassifierSystems.end());
	return (*itClassifierSystems).second.getActionPart(classifierNumber);
}


bool CMHiCSbase::isAnAction(std::string behav) const
{
	std::set<std::string>::const_iterator itActionSet = _ActionSet.find(behav);
	return (itActionSet != _ActionSet.end());
}

/// Chaine de debug
void CMHiCSbase::getDebugString(std::string &t) const
{
	std::string ret = "\n---------------------------";
	std::map<std::string, CClassifierSystem>::const_iterator itClassifierSystems;
	for (itClassifierSystems = _ClassifierSystems.begin(); itClassifierSystems!= _ClassifierSystems.end(); itClassifierSystems++)
	{
		ret += "\nMotivation : " + (*itClassifierSystems).first;
		(*itClassifierSystems).second.getDebugString(ret);
	}
	ret += "\nACTIONS :\n";
	std::set<std::string>::const_iterator itActionSet;
	for (itActionSet = _ActionSet.begin(); itActionSet != _ActionSet.end(); itActionSet++)
	{
		ret += (*itActionSet) + "\n";
	}
	t+=ret;
}

} // NLAINIMAT
