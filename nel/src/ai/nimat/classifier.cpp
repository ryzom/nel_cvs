/** \file classifier.cpp
 * A simple Classifier System.
 *
 * $Id: classifier.cpp,v 1.3 2002/10/10 08:35:18 coutelas Exp $
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
}

CClassifierSystem::~CClassifierSystem()
{
	std::list<CClassifier*>::iterator itClassifiers = _classifiers.begin();
	while (itClassifiers != _classifiers.end())
	{
		delete (*itClassifiers);
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
	_classifiers.push_back(classifier);
}

/// Merge two CS
void CClassifierSystem::addClassifierSystem(const CClassifierSystem &cs)
{
	std::list<CClassifier*>::const_iterator itCSClassifiers;
	for (itCSClassifiers = cs._classifiers.begin(); itCSClassifiers != cs._classifiers.end(); itCSClassifiers++)
	{
		TSensorMap conditionsMap;

		std::list<CClassifierConditionCell*>::const_iterator itCondCell;
		for (itCondCell = (*itCSClassifiers)->Condition.begin(); itCondCell !=(*itCSClassifiers)->Condition.end(); itCondCell++)
		{
			CClassifierConditionCell* pCondCell = (*itCondCell);
			conditionsMap[pCondCell->getSensorName()] = pCondCell->getValue();
		}
		addClassifier(conditionsMap, (*itCSClassifiers)->Priority, (*itCSClassifiers)->Behavior.c_str());
	}
}

std::string CClassifierSystem::selectBehavior( const TSensorMap &sensorMap)
{
	// We update the internal sensor values.
	std::map<std::string, char>::const_iterator itConditionsmap;
	for (itConditionsmap = sensorMap.begin(); itConditionsmap != sensorMap.end(); itConditionsmap++)
	{
		char c =  (*itConditionsmap).second;
		std::string sensName = (*itConditionsmap).first;
		_sensors[sensName ] = c;
		int size = _sensors.size();
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

	// We set the sensors back to the default value.
	for (itConditionsmap = sensorMap.begin(); itConditionsmap != sensorMap.end(); itConditionsmap++)
	{
		char c =  (*itConditionsmap).second;
		std::string sensName = (*itConditionsmap).first;
		_sensors[sensName ] = '#';
		int size = _sensors.size();
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

void CClassifierSystem::getDebugString(std::string &t) const
{
	std::string dbg = "\n";

	std::list<CClassifier*>::const_iterator itClassifiers;
	for (itClassifiers = _classifiers.begin(); itClassifiers != _classifiers.end(); itClassifiers++)
	{
		std::list<CClassifierConditionCell*>::const_iterator itConditions;
		for (itConditions = (*itClassifiers)->Condition.begin(); itConditions != (*itClassifiers)->Condition.end(); itConditions++)
		{
			CClassifierConditionCell* condCell = (*itConditions);
			dbg += " (" + condCell->getSensorName() + "=" + condCell->getValue() + ") +";
		}
		std::string actionName = (*itClassifiers)->Behavior;
		sint16		prio = (*itClassifiers)->Priority;
		//char v[8];
		//dbg += "> " + actionName + " [" + itoa(prio,v,10) + "]\n";
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
}

CMotivationEnergy::~CMotivationEnergy()
{
}

sint16 CMotivationEnergy::getSumValue() const
{
	std::map<std::string, sint16>::const_iterator itEnergyByMotivation;
	sint16 sum = 0;
	for (itEnergyByMotivation = _EnergyByMotivation.begin(); itEnergyByMotivation != _EnergyByMotivation.end(); itEnergyByMotivation)
	{
		sum += (*itEnergyByMotivation).second;
	}
	return sum;
}

uint64 CMotivationEnergy::getID() const
{
	return _ID;
}

void CMotivationEnergy::setID(uint64 id)
{
	_ID = id;
}

void CMotivationEnergy::raz()
{
	_ID = 0;
	_EnergyByMotivation.clear();
}


///////////////////////////
// CNetCS
///////////////////////////

CNetCS::CNetCS()
{
}

CNetCS::~CNetCS()
{
}

void CNetCS::addVirtualActionCS(const CActionCS &action)
{
	const std::map<std::string, CClassifierSystem> *mapAction = action.getClassifiersMap();
	std::map<std::string, CClassifierSystem>::const_iterator ItMapAction;
	for (ItMapAction = mapAction->begin(); ItMapAction != mapAction->end(); ItMapAction++)
	{
		CClassifierSystem* pCS;
		std::string motivationName = (*ItMapAction).first;
		const CClassifierSystem* pOtherCS = &((*ItMapAction).second);

		pCS = &(_ClassifiersAndMotivationIntensity[motivationName].CS);
		pCS->addClassifierSystem(*pOtherCS);
	}
}

void CNetCS::addActionCS(const CActionCS& action)
{
	addVirtualActionCS(action);
	CMotivationEnergy motivalue;
	_ActionsExecutionIntensity[action.getName()] = motivalue;
}


/// Chaine de debug
void CNetCS::getDebugString(std::string &t) const
{
	std::string ret;
	std::map<std::string, CMotivateCS>::const_iterator itClassifiers;
	for (itClassifiers = _ClassifiersAndMotivationIntensity.begin(); itClassifiers!= _ClassifiersAndMotivationIntensity.end(); itClassifiers++)
	{
		ret += "\nMotivation : " + (*itClassifiers).first + "\n";
		(*itClassifiers).second.CS.getDebugString(ret);
	}
	t+=ret;
}

/// Donne la Puissance Propre d'une Motivation
void CNetCS::setMotivationPP(std::string motivationName, uint16 PP)
{
	_MotivationsValues[motivationName].PP = PP;
}

/// Fixe la valeur d'une motivation
void CNetCS::setMotivationValue(std::string motivationName, uint16 value)
{
	_MotivationsValues[motivationName].Value = value;
	//***G*** Rajouter le calcul de valeur du systeme de classeur
}

void CNetCS::run()
{
	/*
	Je sélectionne par roulette weel le classeur que je vais gérer
	Je met à jour l'énergie du vainqueur
	*/
	sint16 somme;
	std::map<sint16, CMotivateCS*> mapCSweel;
	std::map<std::string, CMotivateCS>::iterator itClassifiers;
	// On calcule la somme
	for (itClassifiers = _ClassifiersAndMotivationIntensity.begin(); itClassifiers != _ClassifiersAndMotivationIntensity.end(); itClassifiers++)
	{
		CMotivateCS* pCMotivateCS = &((*itClassifiers).second);
		sint16 energy = pCMotivateCS->MotivationIntensity.getSumValue();
		if (energy > 0)
		{
			somme += energy;
			mapCSweel[somme] = pCMotivateCS;
		}
	}
	if (somme>0)
	{
		// on selectionne le classeur;
		sint16 randomeNumber = rand()%(somme);
		std::map<sint16, CMotivateCS*>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
		CMotivateCS* pCSselection = (*itMapCSweel).second;

		// On vérifie que l'ID est toujours valide
		/*
		 ***G*** PROBLEME :
		 Les Motivation initial ont une énergie indépendament des ID.
		 Elles n'ont donc pas d'ID associé initiallement.
		 */
		uint64 id = pCSselection->MotivationIntensity.getID();
		std::map<uint64, TSensorMap>::iterator itSensorsValues = _SensorsValues.find(id);
		if (itSensorsValues == _SensorsValues.end())
		{
			// Sinon on remet l'énergie à 0.
			pCSselection->MotivationIntensity.raz();
			return;
		}

		// On fait calculer le CS
		TSensorMap* pSensorMap = &((*itSensorsValues).second);
		std::string behav = pCSselection->CS.selectBehavior(*pSensorMap);

		// On récupère le pointeur sur le modul auquel on va transmettre la motivation
		CMotivationEnergy* pEnergy2Evolve;
		std::map<std::string, CMotivationEnergy>::iterator itActionsExecutionIntensity;
		itActionsExecutionIntensity = _ActionsExecutionIntensity.find(behav);
		if (itActionsExecutionIntensity != _ActionsExecutionIntensity.end())
		{
			pEnergy2Evolve = &((*itActionsExecutionIntensity).second);
		}
		else
		{
			std::map<std::string, CMotivateCS>::iterator itClassifiersAndMotivationIntensity;
			itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(behav);
			nlassert (itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end()); // Sinon c'est que j'ai une action qui ne correspond ni à une vrai action ni à un autre classeur.
			pEnergy2Evolve = &((*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
		}
		nlassert(pEnergy2Evolve);

		// On change la valeur de motivation de la cible.
		
		/*
		on regarde quel ID apporte le plus d'énergie
		 */
	}
}

std::string CNetCS::selectBehavior(const TSensorMap &sensorMap)
{
	/*
		Il va falloir gérer :
		- les energies
		- les valeurs des senseurs des différents systèmes de classeur ?

  Je pense que les senseurs devraient être mis à jour de façon indépendante de la requete de selection.

  Pour le select :
  Je regarde la valeur d'IE de chacune de mes action. [Il faut une map <nomAction,IE>]
  S'il n'y a pas d'action courante [Il faut une fonction pour dire qu'une action est terminée] on donne une IEactioncourante de 0.
  On fait la liste de toute les actions dont l'IE est > à IEactioncourante.
  On fait une roullette weel et on retourne le vainqueur. [Il faut noter l'action courante]
  [Il faut donner à l'action courante une IE plus élevée]
	*/
	return "";
}


} // NLAINIMAT
