/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.9 2003/01/21 16:35:44 robert Exp $
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

void CClassifierSystem::addClassifier(const TSensorMap &conditionsMap, double priority, CActionsBox::TAction behavior)
{
	// We build a new classifier.
	CClassifier* classifier = new CClassifier();
	classifier->Behavior = behavior;
	classifier->Priority = priority;

	CClassifierConditionCell* condCell;
	std::map<CSensorsBox::TSensor, char>::const_iterator itCondition;
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
		addClassifier(conditionsMap, (*itCSClassifiers).second->Priority, (*itCSClassifiers).second->Behavior);
	}
}

sint16 CClassifierSystem::selectBehavior( const TSensorMap &sensorMap)
{
	// We update the internal sensor values.
	std::map<CSensorsBox::TSensor, char>::const_iterator itConditionsmap;
	for (itConditionsmap = sensorMap.begin(); itConditionsmap != sensorMap.end(); itConditionsmap++)
	{
		char c =  (*itConditionsmap).second;
		CSensorsBox::TSensor sensName = (*itConditionsmap).first;
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
		CSensorsBox::TSensor sensName = (*itConditionsmap).first;
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

CActionsBox::TAction CClassifierSystem::getActionPart(sint16 classifierNumber)
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
			dbg += " (" + NLMISC::toString(condCell->getSensorName()) + "=" + condCell->getValue() + ") +"; // ***G*** maitre le vrai debug du sensor
		}
		std::string actionName = CActionsBox::getNameFromId((*itClassifiers).second->Behavior);
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

CSensorsBox::TSensor CClassifierSystem::CClassifierConditionCell::getSensorName() const
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
CActionCS::CActionCS(CActionsBox::TAction name)
{
	_Name = name;
}

CActionCS::~CActionCS()
{
}

/// Return the action name
CActionsBox::TAction CActionCS::getName() const
{
	return _Name;
}

void CActionCS::addMotivationRule (CMotivationsBox::TMotivation motivationName, const TSensorMap &conditionsMap, double priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByMotivation[motivationName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

void CActionCS::addVirtualActionRule (CActionsBox::TAction virtualActionName, const TSensorMap &conditionsMap, double priority)
{
	CClassifierSystem* pCS;

	pCS = &(_ClassifiersByVirtualAction[virtualActionName]);
	pCS->addClassifier(conditionsMap, priority, _Name);
}

const std::map<CMotivationsBox::TMotivation, CClassifierSystem> *CActionCS::getClassifiersByMotivationMap () const
{
	return &_ClassifiersByMotivation;
}
const std::map<CActionsBox::TAction, CClassifierSystem> *CActionCS::getClassifiersByVirtualActionMap () const
{
	return &_ClassifiersByVirtualAction;
}

/// Chaine de debug
void CActionCS::getDebugString(std::string &t) const
{
	std::string ret = "\nACTION :\t" + CActionsBox::getNameFromId(_Name) + "\n";
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>::const_iterator ItClassifiersByMotivation;
	for (ItClassifiersByMotivation = _ClassifiersByMotivation.begin(); ItClassifiersByMotivation != _ClassifiersByMotivation.end(); ItClassifiersByMotivation++)
	{
		ret += "\nMotivation : " + CMotivationsBox::getNameFromId((*ItClassifiersByMotivation).first) + "\n";
		(*ItClassifiersByMotivation).second.getDebugString(ret);
	}
	std::map<CActionsBox::TAction, CClassifierSystem>::const_iterator ItClassifiersByVirtualAction;
	for (ItClassifiersByVirtualAction = _ClassifiersByVirtualAction.begin(); ItClassifiersByVirtualAction != _ClassifiersByVirtualAction.end(); ItClassifiersByVirtualAction++)
	{
		ret += "\nMotivation : " + CActionsBox::getNameFromId((*ItClassifiersByVirtualAction).first) + "\n";
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

void CMotivationEnergy::removeProvider(CMotivationsBox::TMotivation providerName)
{
	_MotivationProviders.erase(providerName);
	computeMotivationValue();
}

void CMotivationEnergy::removeProvider(CActionsBox::TAction providerName)
{
	_VirtualActionProviders.erase(providerName);
	computeMotivationValue();
}

void CMotivationEnergy::addProvider(CMotivationsBox::TMotivation providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::addProvider(CActionsBox::TAction providerName, const CMotivationEnergy& providerMotivation)
{
	_VirtualActionProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::updateProvider(CMotivationsBox::TMotivation providerName, const CMotivationEnergy& providerMotivation)
{
	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::updateProvider(CActionsBox::TAction providerName, const CMotivationEnergy& providerMotivation)
{
	_VirtualActionProviders[providerName] = providerMotivation._EnergyByMotivation ;
	computeMotivationValue();
}

void CMotivationEnergy::computeMotivationValue()
{
	_EnergyByMotivation.clear();

	// We look for motivation values comming directly from Motivations
	std::map<CMotivationsBox::TMotivation, TEnergyByMotivation>::iterator itMotivationProviders;
	for (itMotivationProviders = _MotivationProviders.begin(); itMotivationProviders != _MotivationProviders.end(); itMotivationProviders++)
	{
		TEnergyByMotivation &motivation = (*itMotivationProviders).second;
		TEnergyByMotivation::iterator itMotivation, itMyMotivation;
		for (itMotivation = motivation.begin(); itMotivation != motivation.end(); itMotivation++)
		{
			CMotivationsBox::TMotivation motivSource = (*itMotivation).first;
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
	std::map<CActionsBox::TAction, TEnergyByMotivation>::iterator itVirtualActionProviders;
	for (itVirtualActionProviders = _VirtualActionProviders.begin(); itVirtualActionProviders != _VirtualActionProviders.end(); itVirtualActionProviders++)
	{
		TEnergyByMotivation &motivation = (*itVirtualActionProviders).second;
		TEnergyByMotivation::iterator itMotivation, itMyMotivation;
		for (itMotivation = motivation.begin(); itMotivation != motivation.end(); itMotivation++)
		{
			CMotivationsBox::TMotivation motivSource = (*itMotivation).first;
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
void CMotivationEnergy::setMotivationPP(CMotivationsBox::TMotivation motivationName, double PP)
{
	_SumValue -= _EnergyByMotivation[motivationName].Value * _EnergyByMotivation[motivationName].PP;
	_SumValue += _EnergyByMotivation[motivationName].Value * PP;
	_EnergyByMotivation[motivationName].PP = PP;
	_MotivationProviders[motivationName][motivationName].PP = PP;
	nlassert(_SumValue >= 0);
}

/// Fixe la valeur d'une motivation
void CMotivationEnergy::setMotivationValue(CMotivationsBox::TMotivation motivationName, double value)
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
		ret += " " + CMotivationsBox::getNameFromId((*itEnergyByMotivation).first) + " (" + NLMISC::toString((*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP) + ") ";
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
	std::map<CMotivationsBox::TMotivation, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity;
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		ret += "\n " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.dbgNumberOfActivations) + "<" + CMotivationsBox::getNameFromId((*itClassifiersAndMotivationIntensity).first) + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getSumValue()) + "] :";
		(*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getDebugString(ret);
		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
	ret += "\nVirtual Actions :";
	std::map<CActionsBox::TAction, CMotivateCS>::const_iterator itClassifiersAndVirtualActionIntensity;
	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
		 itClassifiersAndVirtualActionIntensity++)
	{
		ret += "\n " + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.dbgNumberOfActivations) + "<" + CActionsBox::getNameFromId((*itClassifiersAndVirtualActionIntensity).first) + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getSumValue()) + "] :";
		(*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getDebugString(ret);
		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
	ret += "\nACTIONS :";
	std::map<CActionsBox::TAction, CMotivationEnergy>::const_iterator itActionsExecutionIntensity;
	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
	{
		ret += "\n <" + CActionsBox::getNameFromId((* itActionsExecutionIntensity).first) + "> [EI=" + NLMISC::toString((*itActionsExecutionIntensity).second.getSumValue()) + "] : ";
		(*itActionsExecutionIntensity).second.getDebugString(ret);
	}
	t+=ret;
}

/// Donne la Puissance Propre d'une Motivation
void CMHiCSagent::setMotivationPP(CMotivationsBox::TMotivation motivationName, double PP)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationPP(motivationName, PP);
	spreadMotivationReckon(motivationName);
}

/// Fixe la valeur d'une motivation
void CMHiCSagent::setMotivationValue(CMotivationsBox::TMotivation motivationName, double value)
{
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationValue(motivationName, value);
	spreadMotivationReckon(motivationName);
}

void CMHiCSagent::spreadMotivationReckon(CMotivationsBox::TMotivation CS)
{
	std::map<CMotivationsBox::TMotivation, CMotivateCS>::iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(CS);
	nlassert(itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end());
	sint16 lastClassifierNumber = (*itClassifiersAndMotivationIntensity).second.ClassifierNumber;
	if (lastClassifierNumber >=0 )
	{
		CActionsBox::TAction lastActionName = _pMHiCSbase->getActionPart(CS, lastClassifierNumber);
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

void CMHiCSagent::spreadMotivationReckon(CActionsBox::TAction CS)
{
	std::map<CActionsBox::TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.find(CS);
	nlassert(itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end());
	sint16 lastClassifierNumber = (*itClassifiersAndVirtualActionIntensity).second.ClassifierNumber;
	if (lastClassifierNumber >=0 )
	{
		CActionsBox::TAction lastActionName = _pMHiCSbase->getActionPart(CS, lastClassifierNumber);
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
	typedef	std::map<CMotivationsBox::TMotivation, CMotivateCS>::iterator TitNameAndMotivation;
	std::map<double, TitNameAndMotivation > mapCSweel;
	std::map<CMotivationsBox::TMotivation, CMotivateCS>::iterator itClassifiersAndMotivationIntensity;
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
		CMotivationsBox::TMotivation selectionName = (*((*itMapCSweel).second)).first;

		// Updating the number of activation counter
		pCSselection->dbgNumberOfActivations++;

		// On fait calculer le CS
		sint16 selectedClassifierNumber = _pMHiCSbase->selectBehavior(selectionName,_SensorsValues);
		if (selectedClassifierNumber < 0) return; // ***G*** Ici on décide de rien faire si on sait pas quoi faire. En fait il faudrait créer un règle.
		CActionsBox::TAction behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);

		// We check the last action selected by the current motivation to remove the motivation influence on this action.
		sint16 lastClassifierNumber = _ClassifiersAndMotivationIntensity[selectionName].ClassifierNumber;
		if (lastClassifierNumber >= 0)
		{
			CActionsBox::TAction lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);

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
	}
}


void CMHiCSagent::virtualActionCompute()
{
	/*
	Je sélectionne par roulette weel l'action virtuel que je vais gérer
	Je met à jour l'énergie du vainqueur
	*/
	double somme = 0;
	typedef	std::map<CActionsBox::TAction, CMotivateCS>::iterator TitNameAndVirtualAction;
	std::map<double, TitNameAndVirtualAction > mapCSweel;
	std::map<CActionsBox::TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity;
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
		CActionsBox::TAction selectionName = (*((*itMapCSweel).second)).first;

		// Updating the number of activation counter
		pCSselection->dbgNumberOfActivations++;

		// On fait calculer le CS
		sint16 selectedClassifierNumber = _pMHiCSbase->selectBehavior(selectionName,_SensorsValues);
		if (selectedClassifierNumber < 0) return; // ***G*** Ici on décide de rien faire si on sait pas quoi faire. En fait il faudrait créer un règle.
		CActionsBox::TAction behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);

		// We check the last action selected by the current motivation to remove the motivation influence on this action.
		sint16 lastClassifierNumber = _ClassifiersAndVirtualActionIntensity[selectionName].ClassifierNumber;
		if (lastClassifierNumber >= 0)
		{
			CActionsBox::TAction lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);

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
	}
}

void CMHiCSagent::run()
{
	motivationCompute();
	virtualActionCompute();
}


void CMHiCSagent::setSensors(const TSensorMap &sensorMap)
{
	_SensorsValues = sensorMap;
}


CActionsBox::TAction CMHiCSagent::selectBehavior()
{
	// On prend le max
	CActionsBox::TAction ret;
	double executionIntensity = 0;
	std::map<CActionsBox::TAction, CMotivationEnergy>::iterator itActionsExecutionIntensity;
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
//	std::map<CMotivationsBox::TMotivation, CClassifierSystem>	_ClassifiersByMotivation;
//	std::map<CActionsBox::TAction, CClassifierSystem>			_ClassifiersByVirtualAction;

	const std::map<CMotivationsBox::TMotivation, CClassifierSystem> *mapActionByMotivation = action.getClassifiersByMotivationMap();
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>::const_iterator ItMapActionByMotivation;
	for (ItMapActionByMotivation = mapActionByMotivation->begin(); ItMapActionByMotivation != mapActionByMotivation->end(); ItMapActionByMotivation++)
	{
		CClassifierSystem* pCS;
		CMotivationsBox::TMotivation motivationName = (*ItMapActionByMotivation).first;
		const CClassifierSystem* pOtherCS = &((*ItMapActionByMotivation).second);

		pCS = &(_MotivationClassifierSystems[motivationName]);
		pCS->addClassifierSystem(*pOtherCS);
	}

	const std::map<CActionsBox::TAction, CClassifierSystem> *mapActionByVirtualAction = action.getClassifiersByVirtualActionMap();
	std::map<CActionsBox::TAction, CClassifierSystem>::const_iterator ItMapActionByVirtualAction;
	for (ItMapActionByVirtualAction = mapActionByVirtualAction->begin(); ItMapActionByVirtualAction != mapActionByVirtualAction->end(); ItMapActionByVirtualAction++)
	{
		CClassifierSystem* pCS;
		CActionsBox::TAction virtualActionName = (*ItMapActionByVirtualAction).first;
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

sint16 CMHiCSbase::selectBehavior(CMotivationsBox::TMotivation motivationName, const TSensorMap &sensorMap)
{
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.selectBehavior(sensorMap);
}

sint16 CMHiCSbase::selectBehavior(CActionsBox::TAction motivationName, const TSensorMap &sensorMap)
{
	std::map<CActionsBox::TAction, CClassifierSystem>::iterator itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.find(motivationName);
	nlassert(itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end());
	return (*itVirtualActionClassifierSystems).second.selectBehavior(sensorMap);
}

CActionsBox::TAction CMHiCSbase::getActionPart(CMotivationsBox::TMotivation motivationName, sint16 classifierNumber)
{
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.getActionPart(classifierNumber);
}

CActionsBox::TAction CMHiCSbase::getActionPart(CActionsBox::TAction motivationName, sint16 classifierNumber)
{
	std::map<CActionsBox::TAction, CClassifierSystem>::iterator itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.find(motivationName);
	nlassert(itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end());
	return (*itVirtualActionClassifierSystems).second.getActionPart(classifierNumber);
}

bool CMHiCSbase::isAnAction(CActionsBox::TAction behav) const
{
	std::set<CActionsBox::TAction>::const_iterator itActionSet = _ActionSet.find(behav);
	return (itActionSet != _ActionSet.end());
}

/// Chaine de debug
void CMHiCSbase::getDebugString(std::string &t) const
{
	std::string ret = "\n---------------------------";
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>::const_iterator itMotivationClassifierSystems;
	for (itMotivationClassifierSystems = _MotivationClassifierSystems.begin(); itMotivationClassifierSystems != _MotivationClassifierSystems.end(); itMotivationClassifierSystems++)
	{
		ret += "\nMotivation : " + CMotivationsBox::getNameFromId((*itMotivationClassifierSystems).first);
		(*itMotivationClassifierSystems).second.getDebugString(ret);
	}
	std::map<CActionsBox::TAction, CClassifierSystem>::const_iterator itVirtualActionClassifierSystems;
	for (itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.begin(); itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end(); itVirtualActionClassifierSystems++)
	{
		ret += "\nVirtual Action : " + CActionsBox::getNameFromId((*itVirtualActionClassifierSystems).first);
		(*itVirtualActionClassifierSystems).second.getDebugString(ret);
	}
	ret += "\nACTIONS :\n";
	std::set<CActionsBox::TAction>::const_iterator itActionSet;
	for (itActionSet = _ActionSet.begin(); itActionSet != _ActionSet.end(); itActionSet++)
	{
		ret += CActionsBox::getNameFromId((*itActionSet)) + "\n";
	}
	t+=ret;
}

///////////////////////////
// CSensorsBox
///////////////////////////

CSensorsBox::CSensorsBox()
{
	;
}

CSensorsBox::~CSensorsBox()
{
	;
}

char CSensorsBox::getSensorValue(TSensor sensorId, uint64 targetID)
{
	switch (sensorId)
	{
	case MunitionsAmount :
	case FoodType :
	case IsAlive :
	case IAmStronger :
	case IAmAttacked :
	default :
		return '#';
	}
}

std::string CSensorsBox::getNameFromId(TSensor sensorId)
{
	switch (sensorId)
	{
	case MunitionsAmount :
		return "MunitionsAmount";
	case FoodType :
		return "FoodType";
	case IsAlive :
		return "IsAlive";
	case IAmStronger :
		return "IAmStronger";
	case IAmAttacked :
		return "IAmAttacked";
	default :
		return "NULL";
	}
}

///////////////////////////
// CActionsBox
///////////////////////////

CActionsBox::CActionsBox()
{
	;
}

CActionsBox::~CActionsBox()
{
	;
}

void CActionsBox::executeAction(TAction actionId, uint64 targetID)
{
	switch (actionId)
	{
	case closeCombat :
	case distanceCombat :
	case flee :
	case eat :
	case rest :
	case v_fight :
	default :
		return;
	}
}

std::string CActionsBox::getNameFromId(TAction actionId)
{
	switch (actionId)
	{
	case closeCombat :
		return "closeCombat";
	case distanceCombat :
		return "distanceCombat";
	case flee :
		return "flee";
	case eat :
		return "eat";
	case rest :
		return "rest";
	case v_fight :
			return "v_fight";
	default :
		return "NULL";
	}
}

///////////////////////////
// CMotivationsBox
///////////////////////////
CMotivationsBox::CMotivationsBox()
{
	;
}

CMotivationsBox::~CMotivationsBox()
{
	;
}
	
double CMotivationsBox::getMotivationValue(TMotivation motivationId)
{
	switch (motivationId)
	{
	case HUNGER :
	case AGGRO :
	case FEAR :
	case FATIGUE :
	default :
		return 0;	// ***G*** To Implement
	}
}

std::string CMotivationsBox::getNameFromId(TMotivation motivationId)
{
	switch (motivationId)
	{
	case HUNGER :
		return "HUNGER";
	case AGGRO :
		return "AGGRO";
	case FEAR :
		return "FEAR";
	case FATIGUE :
		return "FATIGUE";
	default :
		return "NULL";
	}
}

} // NLAINIMAT
