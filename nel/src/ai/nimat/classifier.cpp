/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.8 2002/12/26 14:46:37 robert Exp $
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

void CClassifierSystem::addClassifier(const TSensorMap &conditionsMap, double priority, const char* behavior)
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
	std::map<double, TitClassifiers> mapCSweel;
	std::map<sint16, CClassifier*>::iterator itClassifiers = _classifiers.begin();
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
		double randomeNumber = (rand()%(int(totalPriority*100)))/100.0;
		std::map<double, TitClassifiers>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
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
void CActionCS::addMotivationRule (std::string motivationName, const TSensorMap &conditionsMap, double priority)
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

double CMotivationEnergy::getSumValue() const
{
	return _SumValue;
}

void CMotivationEnergy::removeProvider(std::string providerName)
{
	_MotivationProviders.erase(providerName);
	computeMotivationValue();
}

void CMotivationEnergy::addProvider(std::string providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::updateProvider(std::string providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::computeMotivationValue()
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
void CMotivationEnergy::setMotivationPP(std::string motivationName, double PP)
{
	_SumValue -= _EnergyByMotivation[motivationName].Value * _EnergyByMotivation[motivationName].PP;
	_SumValue += _EnergyByMotivation[motivationName].Value * PP;
	_EnergyByMotivation[motivationName].PP = PP;
	_MotivationProviders[motivationName][motivationName].PP = PP;
	nlassert(_SumValue >= 0);
}

/// Fixe la valeur d'une motivation
void CMotivationEnergy::setMotivationValue(std::string motivationName, double value)
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
		ret += " " + (*itEnergyByMotivation).first + " (" + NLMISC::toString((*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP) + ") ";
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
	ret += "\nMotivations :";
	std::map<std::string, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity;
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		ret += "\n " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.NumberOfActivations) + "<" + (*itClassifiersAndMotivationIntensity).first + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getSumValue()) + "] :";
		(*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getDebugString(ret);
		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
	ret += "\nACTIONS :";
	std::map<std::string, CMotivationEnergy>::const_iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		ret += "\n <" + (* itActionsExecutionIntensity).first + "> [EI=" + NLMISC::toString((*itActionsExecutionIntensity).second.getSumValue()) + "] : ";
		(*itActionsExecutionIntensity).second.getDebugString(ret);
	}
	t+=ret;
}

/// Donne la Puissance Propre d'une Motivation
void CMHiCSagent::setMotivationPP(std::string motivationName, double PP)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationPP(motivationName, PP);
	spreadMotivationReckon(motivationName);
}

/// Fixe la valeur d'une motivation
void CMHiCSagent::setMotivationValue(std::string motivationName, double value)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationValue(motivationName, value);
	spreadMotivationReckon(motivationName);
}

void CMHiCSagent::spreadMotivationReckon(std::string commonCS)
{
	std::map<std::string, CMotivateCS>::iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(commonCS);
	nlassert(itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end());
	sint16 lastClassifierNumber = (*itClassifiersAndMotivationIntensity).second.ClassifierNumber;
	if (lastClassifierNumber >=0 )
	{
		std::string lastActionName = _pMHiCSbase->getActionPart(commonCS, lastClassifierNumber);
		if (_pMHiCSbase->isAnAction(lastActionName))
		{
			_ActionsExecutionIntensity[lastActionName].updateProvider(commonCS, (*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
			// If the action doesn't receive motivation any more, we remove it.
			double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
			if (energy <= 0)
			{
				_ActionsExecutionIntensity.erase(lastActionName);
			}
		}
		else
		{
			_ClassifiersAndMotivationIntensity[lastActionName].MotivationIntensity.updateProvider(commonCS, (*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
			spreadMotivationReckon(lastActionName);
			// If the CS doesn't receive motivation any more, we remove it.
			double energy = _ClassifiersAndMotivationIntensity[lastActionName].MotivationIntensity.getSumValue();
			if (energy <= 0)
			{
				_ClassifiersAndMotivationIntensity.erase(lastActionName);
			}
		}
	}
}

void CMHiCSagent::run()
{
	/*
	Je sélectionne par roulette weel le classeur que je vais gérer
	Je met à jour l'énergie du vainqueur
	*/
	double somme = 0;
	typedef	std::map<std::string, CMotivateCS>::iterator TitNameAndMotivation;
	std::map<double, TitNameAndMotivation > mapCSweel;
	std::map<std::string, CMotivateCS>::iterator itClassifiersAndMotivationIntensity;
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
		std::string selectionName = (*((*itMapCSweel).second)).first;

		// Updating the number of activation counter
		pCSselection->NumberOfActivations++;

		// On fait calculer le CS
		sint16 selectedClassifierNumber = _pMHiCSbase->selectBehavior(selectionName,_SensorsValues);
		if (selectedClassifierNumber < 0) return; // ***G*** Ici on décide de rien faire si on sait pas quoi faire. En fait il faudrait créer un règle.
		std::string behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);

		// We check the last action selected by the current motivation to remove the motivation influence on this action.
		sint16 lastClassifierNumber = _ClassifiersAndMotivationIntensity[selectionName].ClassifierNumber;
		if (lastClassifierNumber >= 0)
		{
			std::string lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);

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
					_ClassifiersAndMotivationIntensity[lastActionName].MotivationIntensity.removeProvider(selectionName);
					spreadMotivationReckon(lastActionName);
					// If the CS doesn't receive motivation any more, we remove it.
					double energy = _ClassifiersAndMotivationIntensity[lastActionName].MotivationIntensity.getSumValue();
					if (energy <= 0)
					{
						_ClassifiersAndMotivationIntensity.erase(lastActionName);
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
			_ClassifiersAndMotivationIntensity[behav].MotivationIntensity.addProvider(selectionName, pCSselection->MotivationIntensity);
			spreadMotivationReckon(behav);
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
	double executionIntensity = 0;
	std::map<std::string, CMotivationEnergy>::iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		double value = (*itActionsExecutionIntensity).second.getSumValue();
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
