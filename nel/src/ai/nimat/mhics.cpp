/** \file mhics.cpp
 * The MHiCS architecture. (Modular Hierarchical Classifiers System)
 *
 * $Id: mhics.cpp,v 1.8 2003/07/28 13:19:41 robert Exp $
 */

/* Copyright, 2003 Nevrax Ltd.
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

#include "nel/ai/nimat/mhics.h"
#include "nel/misc/file.h"

namespace NLAINIMAT
{
	using namespace NLMISC;

	static const TTargetId NullTargetId = 0;

///////////////////////////
// CMotivationEnergy
///////////////////////////
CMotivationEnergy::CMotivationEnergy()
{
	_SumValue = 0;
	_MHiCSagent = NULL;
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

//void CMotivationEnergy::removeProvider(TAction providerName)
//{
//	_VirtualActionProviders.erase(providerName);
//	computeMotivationValue();
//}

void CMotivationEnergy::addProvider(TMotivation providerName, TClassifierPriority classifierPriority)
{
	_MotivationProviders.insert(std::make_pair(providerName,classifierPriority));
	computeMotivationValue();
}

//void CMotivationEnergy::addProvider(TAction providerName, const CMotivationEnergy& providerMotivation)
//{
//	_VirtualActionProviders[providerName] = providerMotivation._EnergyByMotivation ;
//	computeMotivationValue();
//}

//void CMotivationEnergy::updateProvider(TMotivation providerName, const CMotivationEnergy& providerMotivation)
//{
//	_MotivationProviders[providerName] = providerMotivation._EnergyByMotivation ;
//	computeMotivationValue();
//}

//void CMotivationEnergy::updateProvider(TAction providerName, const CMotivationEnergy& providerMotivation)
//{
//	_VirtualActionProviders[providerName] = providerMotivation._EnergyByMotivation ;
//	computeMotivationValue();
//}

void CMotivationEnergy::computeMotivationValue()
{
	_EnergyByMotivation.clear();

	TMotivation lastMotivationName = Motivation_Unknown;
	double lastMaxMotiveValue = 0.0;
	// We look for motivation values comming directly from Motivations
	std::multimap<TMotivation, TClassifierPriority>::iterator itMotivationProviders;
	for (itMotivationProviders = _MotivationProviders.begin();
		 itMotivationProviders != _MotivationProviders.end();
		 itMotivationProviders++)
	{
		TMotivation motivationName = (*itMotivationProviders).first;
		TClassifierPriority classifierPriority = (*itMotivationProviders).second;
		double motiveValue = _MHiCSagent->getMotivationValue(motivationName);
		double motivePP = _MHiCSagent->getMotivationPP(motivationName);
		double combinedValue = motiveValue * classifierPriority;
		if (combinedValue > lastMaxMotiveValue)
		{
			lastMaxMotiveValue = combinedValue;
			_EnergyByMotivation[motivationName].Value = combinedValue;
			_EnergyByMotivation[motivationName].PP = motivePP;
		}
	}

	TEnergyByMotivation::const_iterator itEnergyByMotivation;
	double sum = 0;
	for (itEnergyByMotivation = _EnergyByMotivation.begin(); itEnergyByMotivation != _EnergyByMotivation.end(); itEnergyByMotivation++)
	{
		sum += (*itEnergyByMotivation).second.Value * (*itEnergyByMotivation).second.PP;
	}
	sum += _MyMotivationValue.PP * _MyMotivationValue.Value;
	_SumValue = sum;
	nlassert(_SumValue >= 0);
}

/// Donne la Puissance Propre d'une Motivation
void CMotivationEnergy::setMotivationPP(TMotivation motivationName, double PP)
{
	_SumValue -= _MyMotivationValue.Value * _MyMotivationValue.PP;
	_SumValue += _MyMotivationValue.Value * PP;
	_EnergyByMotivation[motivationName].PP = PP;
	_MyMotivationValue.PP = PP;
	nlassert(_SumValue >= 0);
}

/// Retourne la Puissance Propre d'une Motivation
double CMotivationEnergy::getMotivationPP(TMotivation motivationName) const
{
	return _MyMotivationValue.PP;
}

/// Fixe la valeur d'une motivation
void CMotivationEnergy::setMotivationValue(TMotivation motivationName, double value)
{
	nlassert(value >= 0);
	_SumValue -= _MyMotivationValue.Value * _MyMotivationValue.PP;
	nlassert(_SumValue >= 0);
	_SumValue += value * _MyMotivationValue.PP;
	_EnergyByMotivation[motivationName].Value = value;
	_MyMotivationValue.Value = value;

	nlassert(_SumValue >= 0);
}

/// Retourne la valeur d'une motiation
double	CMotivationEnergy::getMotivationValue(TMotivation motivationName) const
{
	return _MyMotivationValue.Value;
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
// CMHiCSbase
///////////////////////////

CMHiCSbase::CMHiCSbase()
{
	pActionResources = new CActionResources();
}

CMHiCSbase::~CMHiCSbase()
{
	delete pActionResources;
}

void CMHiCSbase::addVirtualActionCS(const CActionClassifiers &action)
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

void CMHiCSbase::addActionCS(const CActionClassifiers& action)
{
	nlassert ( action.getName() < Action_VIRTUAL_ACTIONS);
	addVirtualActionCS(action);
	_ActionSet.insert(action.getName());
}

void CMHiCSbase::selectBehavior(TMotivation motivationName,
								const CCSPerception* psensorMap,
								std::multimap<double, std::pair<TClassifierNumber, TTargetId> > &mapActivableCS)
{
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	(*itMotivationClassifierSystems).second.selectBehavior(psensorMap, mapActivableCS);
}

//void CMHiCSbase::selectBehavior(TAction VirtualActionName,
//								const CCSPerception* psensorMap,
//								std::multimap<double, std::pair<TClassifierNumber, TTargetId> > &mapActivableCS,
//								TTargetId	&target)
//{
//	std::map<TAction, CClassifierSystem>::iterator itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.find(VirtualActionName);
//	nlassert(itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end());
//	// When we select an high level action, we limit the perception to the target associated with this action.
//	CCSPerception neoPerception;
//	neoPerception.NoTargetSensors = psensorMap->NoTargetSensors;
//	std::map<TTargetId, TSensorMap>::const_iterator	itSensorMap = psensorMap->TargetSensors.find(target);
//	if(itSensorMap != psensorMap->TargetSensors.end())
//	{
//		neoPerception.TargetSensors[target] = (*itSensorMap).second;
//	}
//	(*itVirtualActionClassifierSystems).second.selectBehavior(&neoPerception, mapActivableCS);
//}

TAction CMHiCSbase::getActionPart(TMotivation motivationName, TClassifierNumber classifierNumber)
{
	if (classifierNumber == -1) return Action_DoNothing;

	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.getActionPart(classifierNumber);
}

//TAction CMHiCSbase::getActionPart(TAction motivationName, TClassifierNumber classifierNumber)
//{
//	if (classifierNumber == -1) return Action_DoNothing;
//	
//	std::map<TAction, CClassifierSystem>::iterator itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.find(motivationName);
//	nlassert(itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end());
//	return (*itVirtualActionClassifierSystems).second.getActionPart(classifierNumber);
//}

TClassifierPriority CMHiCSbase::getPriorityPart(TMotivation motivationName, TClassifierNumber classifierNumber)
{
	if (classifierNumber == -1) return 0;
	
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.getPriorityPart(classifierNumber);
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


void CMHiCSbase::printDebugString() const
{
	std::string ret = "";
	nldebug("\n---------------------------");
	std::map<TMotivation, CClassifierSystem>::const_iterator itMotivationClassifierSystems;
	for (itMotivationClassifierSystems = _MotivationClassifierSystems.begin(); itMotivationClassifierSystems != _MotivationClassifierSystems.end(); itMotivationClassifierSystems++)
	{
		ret += (("\nMotivation : " + conversionMotivation.toString((*itMotivationClassifierSystems).first)).c_str());
		(*itMotivationClassifierSystems).second.getDebugString(ret);
		nldebug(ret.c_str());
		ret = "";
	}
	std::map<TAction, CClassifierSystem>::const_iterator itVirtualActionClassifierSystems;
	for (itVirtualActionClassifierSystems = _VirtualActionClassifierSystems.begin(); itVirtualActionClassifierSystems != _VirtualActionClassifierSystems.end(); itVirtualActionClassifierSystems++)
	{
		ret += (("\nVirtual Action : " + conversionAction.toString((*itVirtualActionClassifierSystems).first)).c_str());
		(*itVirtualActionClassifierSystems).second.getDebugString(ret);
		nldebug(ret.c_str());
		ret = "";
	}
	ret += ("\nACTIONS :\n");
	std::set<TAction>::const_iterator itActionSet;
	for (itActionSet = _ActionSet.begin(); itActionSet != _ActionSet.end(); itActionSet++)
	{
		ret += ((conversionAction.toString((*itActionSet)) + "\n").c_str());
		nldebug(ret.c_str());
		ret = "";
	}
}


/// Load classifiers from a file. Return false if thereis a probleme
bool CMHiCSbase::loadClassifierFromFile(std::string fileName)
{
	bool			ret;
	const uint32	aboeufSize = 2048;
	char			aboeuf[aboeufSize];
	std::string		laLigne, leMot;
	uint			lastPos = 0;
	uint			nextPos = 0;
	NLMISC::CIFile	melkior;
	CConditionMap	conditionsMap;
	std::map<TAction, CActionClassifiers >	actionsMap;
	std::vector<TSensor>					sensorVector;
	ret = melkior.open(fileName, true);
	if (!ret) return false;
	// 1ère ligne : titre conditions
	melkior.getline(aboeuf, aboeufSize);
	// 2ème ligne : Motivations; [Condition]*; Actions; priority; blabla
	melkior.getline(aboeuf, aboeufSize);
	laLigne = aboeuf;
	// on construit une map avec les conditions
	nextPos = laLigne.find_first_of(";",lastPos);
	leMot = laLigne.substr(lastPos,nextPos - lastPos);
	nlassert(leMot == "Motivations");
	lastPos = nextPos+1;
	nextPos = laLigne.find_first_of(";", lastPos);
	leMot = laLigne.substr(lastPos, nextPos-lastPos);
	while (leMot != "Actions")
	{
		// on regarde le type du senseur :
		TSensor titi = conversionSensor.fromString(leMot);
		nlassert (titi != Sensor_Unknown);
		sensorVector.push_back(titi);

		lastPos = nextPos+1;
		nextPos = laLigne.find_first_of(";", lastPos);
		leMot = laLigne.substr(lastPos, nextPos-lastPos);
	}
	// on parse le reste
	melkior.getline(aboeuf, aboeufSize);
	laLigne = aboeuf;
	lastPos = 0;
	nextPos = 0;
	while (laLigne.size() > 0)
	{
		TMotivation laMotive;
		TAction		laVirtuelle;
		// On récupère le nom de la motivation
		nextPos = laLigne.find_first_of(";",lastPos);
		leMot = laLigne.substr(lastPos,nextPos - lastPos);
		if (leMot.size() == 0) break;
		laMotive = conversionMotivation.fromString(leMot);
		if (laMotive == Motivation_Unknown)
		{
			// Si c'est pas une motivation, c'est peut-être une action virtuelle.
			laVirtuelle = conversionAction.fromString(leMot);
			nlassert(laVirtuelle != Action_Unknown);
		}
		
		lastPos = nextPos+1;
		nextPos = laLigne.find_first_of(";",lastPos);
		leMot = laLigne.substr(lastPos,nextPos - lastPos);
		
		// On récupère la liste des conditions
		uint ii;
		for (ii = 0; ii < sensorVector.size(); ii++)
		{
			if (leMot.size() >0)
			{
				TSensor sensorName = sensorVector[ii];
				if (leMot[0] == '!')
				{
					conditionsMap.addIfNotSensorCondition(sensorName,leMot[1]);
				}
				else
				{
					conditionsMap.addIfSensorCondition(sensorName,leMot[0]);
				}
			}

			lastPos = nextPos+1;
			nextPos = laLigne.find_first_of(";",lastPos);
			leMot = laLigne.substr(lastPos,nextPos - lastPos);
		}
		// on récupère le nom de l'action
		TAction actionName = conversionAction.fromString(leMot);
		nlassert(actionName != Action_Unknown);

		lastPos = nextPos+1;
		nextPos = laLigne.find_first_of(";",lastPos);
		leMot = laLigne.substr(lastPos,nextPos - lastPos);

		// on récupère la force du classeur
		double laforce = atof(leMot.c_str());

		// on rajoute la règle dans les actions.
		std::map<TAction, CActionClassifiers >::iterator itActionsMap = actionsMap.find(actionName);
		if (itActionsMap == actionsMap.end())
		{
			CActionClassifiers bibu(actionName);
			actionsMap.insert(std::make_pair(actionName, bibu));
		}
		CActionClassifiers mon_action(actionName);
		// Si la motivation est inconnue, c'est que c'est une action virtuelle.
		if (laMotive == Motivation_Unknown)
		{
			actionsMap[actionName].addVirtualActionRule(laVirtuelle,conditionsMap, laforce);
		}
		else
		{
			actionsMap[actionName].addMotivationRule(laMotive,conditionsMap, laforce);
		}

		conditionsMap.clear();
		melkior.getline(aboeuf, aboeufSize);
		laLigne = aboeuf;
		lastPos = 0;
		nextPos = 0;
	}
	melkior.close();

	std::map<TAction, CActionClassifiers >::iterator itActionsMap;
	for (itActionsMap = actionsMap.begin(); itActionsMap != actionsMap.end(); itActionsMap++)
	{
		//***G*** ajouter aussi la gestion des actions virtuels
		TAction testAction = (*itActionsMap).second.getName();
		nlassert (testAction != Action_Unknown);
		if (testAction > Action_VIRTUAL_ACTIONS)
		{
			addVirtualActionCS( (*itActionsMap).second );
		}
		else
		{
			addActionCS( (*itActionsMap).second );
		}
	}

	return true;
}

///////////////////////////
// CMHiCSagent
///////////////////////////

CMHiCSagent::CMHiCSagent(CMHiCSbase* pMHiCSbase)
{
	nlassert (pMHiCSbase != NULL);
	_pMHiCSbase = pMHiCSbase;
//	_ActionsExecutionIntensity[Action_DoNothing] = CMotivationEnergy();
//	_IdByActions[Action_DoNothing] = NullTargetId;
//	_ItCurrentAction = _IdByActions.find(Action_DoNothing);
}

CMHiCSagent::~CMHiCSagent()
{
}

/// function used in debug to change a TTargetId in a string
std::string CMHiCSagent::targetId2String(TTargetId id) const
{
/*	// Le format est celui pour afficher en debug le N° comme dans Ryzom.
	uint32 aiBoteId = id;
	uint32 managerID = (aiBoteId>>(8+12))&( (1<<10)-1 );
	uint32 groupeID = (aiBoteId>>8)&( (1<<12)-1 );
	uint32 boteID = aiBoteId&( (1<<8)-1 );
	char result[30];
	sprintf(result,"AI:%04x:BOT:%04x:%04x:%04x",aiBoteId,managerID,groupeID,boteID);
	return result;
*/
	std::string ret = NLMISC::toString(id);
	return ret;
}

/// Chaine de debug
void CMHiCSagent::getDebugString(std::string &t) const
{
	std::string ret = "\n\n---------------------------\n";
	ret += "\nPerceptions :";
	ret += "\n Without target";
	NLAINIMAT::TSensorMap::const_iterator itNoTargetSensors;
	for (itNoTargetSensors  = _pSensorsValues->NoTargetSensors.begin();
		 itNoTargetSensors != _pSensorsValues->NoTargetSensors.end();
		 itNoTargetSensors++)
	{
		ret += "\n  " + conversionSensor.toString((*itNoTargetSensors).first) + "(" + (*itNoTargetSensors).second + ")";
	}
	std::map<TTargetId, TSensorMap>::const_iterator itTargetSensors;
	for (itTargetSensors  = _pSensorsValues->TargetSensors.begin();
		 itTargetSensors != _pSensorsValues->TargetSensors.end();
		 itTargetSensors++)
	{
		ret += "\n On target n#" + targetId2String((*itTargetSensors).first);
		for (itNoTargetSensors  = (*itTargetSensors).second.begin();
			 itNoTargetSensors != (*itTargetSensors).second.end();
			 itNoTargetSensors++)
		{
			 ret += "\n  " + conversionSensor.toString((*itNoTargetSensors).first) + "(" + (*itNoTargetSensors).second + ")";
		}
	}
	ret += "\n\nMotivations :";
	std::map<TMotivation, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity;
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		ret += "\n <" + conversionMotivation.toString((*itClassifiersAndMotivationIntensity).first) + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getSumValue()) + "] :";
		(*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getDebugString(ret);
		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
//	ret += "\nVirtual Actions :";
//	std::map<TAction, CMotivateCS>::const_iterator itClassifiersAndVirtualActionIntensity;
//	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
//		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
//		 itClassifiersAndVirtualActionIntensity++)
//	{
//		ret += "\n <" + conversionAction.toString((*itClassifiersAndVirtualActionIntensity).first) + "> ";
//		ret += "[MI=" + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getSumValue()) + "] :";
//		(*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getDebugString(ret);
////		std::map<TAction, TTargetId>::const_iterator itIdByActions = _IdByActions.find((*itClassifiersAndVirtualActionIntensity).first);
////		nlassert (itIdByActions != _IdByActions.end());
////		ret += " on target n#" + NLMISC::toString((*itIdByActions).second);
////		ret += " on target n#" + targetId2String((*itIdByActions).second);
//		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.ClassifierNumber); 
//		ret += "\n";
//	}
	ret += "\nACTIONS :";
	std::map<TAction, CMotivationEnergy>::const_iterator itActionsExecutionIntensity;
//	for (itActionsExecutionIntensity = _ActionsExecutionIntensity.begin(); itActionsExecutionIntensity != _ActionsExecutionIntensity.end(); itActionsExecutionIntensity++)
//	{
//		ret += "\n <" + conversionAction.toString((* itActionsExecutionIntensity).first) + "> [EI=" + NLMISC::toString((*itActionsExecutionIntensity).second.getSumValue()) + "] : ";
//		(*itActionsExecutionIntensity).second.getDebugString(ret);
//		std::map<TAction, TTargetId>::const_iterator itIdByActions = _IdByActions.find((*itActionsExecutionIntensity).first);
//		nlassert (itIdByActions != _IdByActions.end());
////		ret += " on target n#" + NLMISC::toString((*itIdByActions).second);
//		ret += " on target n#" + targetId2String((*itIdByActions).second);
//	}
//	if (_ItCurrentAction != _IdByActions.end())
//	{
////		ret += "\nACTION ACTIVE : " + NLAINIMAT::conversionAction.toString((*_ItCurrentAction).first) + " on " + NLMISC::toString((*_ItCurrentAction).second);
//		ret += "\nACTION ACTIVE : " + NLAINIMAT::conversionAction.toString((*_ItCurrentAction).first) + " on " + targetId2String((*_ItCurrentAction).second);
//	}
	t+=ret;
}

/// Donne la Puissance Propre d'une Motivation
void CMHiCSagent::setMotivationPP(TMotivation motivationName, double PP)
{
	_ClassifiersAndMotivationIntensity[motivationName].setMHiCSagent(this);
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationPP(motivationName, PP);
//	spreadMotivationReckon(motivationName);
}

/// Retourne la Puissance Propre d'une Motivation
double CMHiCSagent::getMotivationPP(TMotivation motivationName) const
{
	std::map<TMotivation, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(motivationName);
	if (itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end()) 
	{
		return (*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getMotivationPP(motivationName);
	}
	else
	{
		return -1;
	}
}

/// Fixe la valeur d'une motivation
void CMHiCSagent::setMotivationValue(TMotivation motivationName, double value)
{
	_ClassifiersAndMotivationIntensity[motivationName].setMHiCSagent(this);
	_ClassifiersAndMotivationIntensity[motivationName].MotivationIntensity.setMotivationValue(motivationName, value);
//	spreadMotivationReckon(motivationName);
}

double	CMHiCSagent::getMotivationValue(TMotivation motivationName) const
{
	std::map<TMotivation, CMotivateCS>::const_iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(motivationName);
	if (itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end()) 
	{
		return (*itClassifiersAndMotivationIntensity).second.MotivationIntensity.getMotivationValue(motivationName);
	}
	else
	{
		return -1;
	}
}

/// Retourne l'intensité de motivation reçu par un action virtuel
//double CMHiCSagent::getMotivationIntensity(TAction virtualAction) const
//{
//	std::map<TAction, CMotivateCS>::const_iterator itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.find(virtualAction);
//	if (itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end()) 
//	{
//		return (*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getSumValue();
//	}
//	else
//	{
//		return -1;
//	}
//}

/// Retourne l'intensité d'exécution d'une action
double CMHiCSagent::getExecutionIntensity(TAction action, TTargetId target) const
{
	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >::const_iterator itActionsExecutionIntensityByTarget;
	itActionsExecutionIntensityByTarget = _ActionsExecutionIntensityByTarget.find(target);
	if (itActionsExecutionIntensityByTarget == _ActionsExecutionIntensityByTarget.end())
	{
		return -1;
	}
	std::map<TAction, CMotivationEnergy>::const_iterator itActionsExecutionIntensity = (*itActionsExecutionIntensityByTarget).second.find(action);
	if (itActionsExecutionIntensity != (*itActionsExecutionIntensityByTarget).second.end()) 
	{
		return (*itActionsExecutionIntensity).second.getSumValue();
	}
	else
	{
		return -1;
	}
}


//void CMHiCSagent::spreadMotivationReckon(TMotivation CS)
//{
//	std::map<TMotivation, CMotivateCS>::iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(CS);
//	nlassert(itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end());
//	TClassifierNumber lastClassifierNumber = (*itClassifiersAndMotivationIntensity).second.ClassifierNumber;
//	if (lastClassifierNumber >=0 )
//	{
//		TAction lastActionName = _pMHiCSbase->getActionPart(CS, lastClassifierNumber);
//		if (_pMHiCSbase->isAnAction(lastActionName))
//		{
//			std::map<TAction, CMotivationEnergy>::iterator itActionsExecutionIntensity =
//															_ActionsExecutionIntensity.find(lastActionName);
//			// Test if the action selected hasn't been removed.
//			if (itActionsExecutionIntensity == _ActionsExecutionIntensity.end()) return;
//
//			// Update the motivation provider for the action execution intensity.
//			(*itActionsExecutionIntensity).second.updateProvider(CS,
//																 (*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
//			// If the action doesn't receive motivation any more, we remove it.
//			double energy = (*itActionsExecutionIntensity).second.getSumValue();
//			if (energy <= 0)
//			{
//				_ActionsExecutionIntensity.erase(lastActionName);
//				_IdByActions.erase(lastActionName);
//				// we check if it was the current action
//				if ((*_ItCurrentAction).first == lastActionName)
//				{
//					_ItCurrentAction = _IdByActions.find(Action_DoNothing);
//					nlassert (_ItCurrentAction != _IdByActions.end());
//				}
//			}
//		}
//		else
//		{
//			std::map<TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity = 
//														_ClassifiersAndVirtualActionIntensity.find(lastActionName);
//			// Test if the virtual action selected hasn't been removed.
//			if (itClassifiersAndVirtualActionIntensity == _ClassifiersAndVirtualActionIntensity.end()) return;
//			
//			// Update the motivation provider for the virtual action execution intensity.
//			(*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.updateProvider(CS,
//																(*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
//			spreadMotivationReckon(lastActionName);
//			// If the CS doesn't receive motivation any more, we remove it.
//			double energy = (*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getSumValue();
//			if (energy <= 0)
//			{
//				_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
//			}
//		}
//	}
//}
//
//void CMHiCSagent::spreadMotivationReckon(TAction CS)
//{
//	std::map<TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensityORIGIN = 
//																	_ClassifiersAndVirtualActionIntensity.find(CS);
//	nlassert(itClassifiersAndVirtualActionIntensityORIGIN != _ClassifiersAndVirtualActionIntensity.end());
//	TClassifierNumber lastClassifierNumber = (*itClassifiersAndVirtualActionIntensityORIGIN).second.ClassifierNumber;
//	if (lastClassifierNumber >=0 )
//	{
//		TAction lastActionName = _pMHiCSbase->getActionPart(CS, lastClassifierNumber);
//		if (_pMHiCSbase->isAnAction(lastActionName))
//		{
//			std::map<TAction, CMotivationEnergy>::iterator itActionsExecutionIntensity =
//				_ActionsExecutionIntensity.find(lastActionName);
//			// Test if the action selected hasn't been removed.
//			if (itActionsExecutionIntensity == _ActionsExecutionIntensity.end()) return;
//			
//			// Update the motivation provider for the action execution intensity.
//			(*itActionsExecutionIntensity).second.updateProvider(CS,
//				(*itClassifiersAndVirtualActionIntensityORIGIN).second.MotivationIntensity);
//			// If the action doesn't receive motivation any more, we remove it.
//			double energy = (*itActionsExecutionIntensity).second.getSumValue();
//			if (energy <= 0)
//			{
//				_ActionsExecutionIntensity.erase(lastActionName);
//				_IdByActions.erase(lastActionName);
//				// we check if it was the current action
//				if ((*_ItCurrentAction).first == lastActionName)
//				{
//					_ItCurrentAction = _IdByActions.find(Action_DoNothing);
//					nlassert (_ItCurrentAction != _IdByActions.end());
//				}
//			}
//		}
//		else
//		{
//			std::map<TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity = 
//				_ClassifiersAndVirtualActionIntensity.find(lastActionName);
//			// Test if the virtual action selected hasn't been removed.
//			if (itClassifiersAndVirtualActionIntensity == _ClassifiersAndVirtualActionIntensity.end()) return;
//			
//			// Update the motivation provider for the virtual action execution intensity.
//			(*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.updateProvider(CS,
//														(*itClassifiersAndVirtualActionIntensityORIGIN).second.MotivationIntensity);
//			spreadMotivationReckon(lastActionName);
//			// If the CS doesn't receive motivation any more, we remove it.
//			double energy = (*itClassifiersAndVirtualActionIntensity).second.MotivationIntensity.getSumValue();
//			if (energy <= 0)
//			{
//				_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
//			}
//		}
//	}
//}

void CMHiCSagent::motivationCompute()
{
	/*
	Avant je sélectionnait par roulette wheel une motivation parmis les autres de façon à aléger le temps de calcul.
	En fait je vais les faires toutes calculer ensemble pour simplifier mon problème.
	*/
	TAction behav;

	std::map<TMotivation, CMotivateCS>::iterator itClassifiersAndMotivationIntensity;

	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		CMotivateCS* pCSselection = &((*itClassifiersAndMotivationIntensity).second);
		TMotivation selectionName = (*itClassifiersAndMotivationIntensity).first;
		double energy = pCSselection->MotivationIntensity.getSumValue();
		if (energy > 0)
		{
			// On fait calculer le CS
//			TClassifierNumber lastClassifierNumber = pCSselection->ClassifierNumber;
//			TClassifierNumber selectedClassifierNumber = lastClassifierNumber;
//			TTargetId currentTargetId = pCSselection->TargetId;
//			double lastSelectionMaxPriority = pCSselection->LastSelectionMaxPriority;
			std::multimap<double, std::pair<TClassifierNumber, TTargetId> > mapActivableCS;
			std::multimap<double, std::pair<TClassifierNumber, TTargetId> >::iterator itMapActivableCS;

			_pMHiCSbase->selectBehavior(selectionName,_pSensorsValues, mapActivableCS);

			for (itMapActivableCS = mapActivableCS.begin(); itMapActivableCS != mapActivableCS.end(); itMapActivableCS++)
			{
				TClassifierNumber selectedClassifierNumber = (*itMapActivableCS).second.first;
				TTargetId currentTargetId = (*itMapActivableCS).second.second;
				behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);
				TClassifierPriority classifierPriority = _pMHiCSbase->getPriorityPart(selectionName, selectedClassifierNumber);
					
//				// We check the last action selected by the current motivation to remove the motivation influence on this action.
//				if (lastClassifierNumber >= 0)
//				{
//					TAction lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);
//					
//					// We check if we have selected the same behavior.
//					if (lastActionName != behav)
//					{
//						if (_pMHiCSbase->isAnAction(lastActionName))
//						{
//							_ActionsExecutionIntensity[lastActionName].removeProvider(selectionName);
//							// If the action doesn't receive motivation any more, we remove it.
//							double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
//							if (energy <= 0)
//							{
//								_ActionsExecutionIntensity.erase(lastActionName);
//								_IdByActions.erase(lastActionName);
//								// we check if it was the current action
//								if ((*_ItCurrentAction).first == lastActionName)
//								{
//									_ItCurrentAction = _IdByActions.find(Action_DoNothing);
//									nlassert (_ItCurrentAction != _IdByActions.end());
//								}
//							}
//						}
//						else
//						{
//							_ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.removeProvider(selectionName);
//							spreadMotivationReckon(lastActionName);
//							// If the CS doesn't receive motivation any more, we remove it.
//							double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.getSumValue();
//							if (energy <= 0)
//							{
//								_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
//							}
//						}
//					}
//				}
//				// We store the number of the new classifier actived by this motivation.
//				_ClassifiersAndMotivationIntensity[selectionName].ClassifierNumber = selectedClassifierNumber;
//				_ClassifiersAndMotivationIntensity[selectionName].TargetId = currentTargetId;
//				_ClassifiersAndMotivationIntensity[selectionName].LastSelectionMaxPriority = lastSelectionMaxPriority;
				
				// We add the current motivation energy to the selected action.
				if (behav != Action_DoNothing)
				{
//					if (_pMHiCSbase->isAnAction(behav))
//					{
						_ActionsExecutionIntensityByTarget[currentTargetId][behav].setMHiCSagent(this);
						_ActionsExecutionIntensityByTarget[currentTargetId][behav].addProvider(selectionName, classifierPriority);
//						_ActionsExecutionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
//					}
//					else
//					{
//						// Else it must be a virtual action (common CS)
//						_ClassifiersAndVirtualActionIntensity[behav].MotivationIntensity.addProvider(selectionName, pCSselection->MotivationIntensity);
//						spreadMotivationReckon(behav);
//					}
					
					// We set the Id of this action.
					// For moment there's no test to see if it is the same target or not. In the futur it can be usefull to make this test
					// to avoid unwilled target switch.
//					_IdByActions[behav] = currentTargetId;
				}
			}
		}
	}
}



//void CMHiCSagent::virtualActionCompute()
//{
//	/*
//	Je sélectionne par roulette weel l'action virtuel que je vais gérer
//	Je met à jour l'énergie du vainqueur
//	*/
//	double somme = 0;
//	typedef	std::map<TAction, CMotivateCS>::iterator TitNameAndVirtualAction;
//	std::map<double, TitNameAndVirtualAction > mapCSweel;
//	std::map<TAction, CMotivateCS>::iterator itClassifiersAndVirtualActionIntensity;
//	// On calcule la somme
//	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
//		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
//		 itClassifiersAndVirtualActionIntensity++)
//	{
//		CMotivateCS* pCMotivateCS = &((*itClassifiersAndVirtualActionIntensity).second);
//		double energy = pCMotivateCS->MotivationIntensity.getSumValue();
//		if (energy > 0)
//		{
//			somme += energy;
//			mapCSweel[somme] = itClassifiersAndVirtualActionIntensity;
//		}
//	}
//	if (somme>0)
//	{
//		// on selectionne le classeur;
//		double randomeNumber = (rand()%(int(somme*100)))/100.0;
//		std::map<double, TitNameAndVirtualAction>::iterator itMapCSweel = mapCSweel.upper_bound(randomeNumber);
//		CMotivateCS* pCSselection = &((*((*itMapCSweel).second)).second);
//		TAction selectionName = (*((*itMapCSweel).second)).first;
//
//		// Get the target Id for this Virtual Action
//		std::map<TAction, TTargetId>::const_iterator itIdByActions = _IdByActions.find(selectionName);
//		nlassert (itIdByActions != _IdByActions.end());
//		TTargetId myTarget = (*itIdByActions).second;
//
//		// On fait calculer le CS
//		TClassifierNumber lastClassifierNumber = _ClassifiersAndVirtualActionIntensity[selectionName].ClassifierNumber;
//		TClassifierNumber selectedClassifierNumber = lastClassifierNumber;
//		TTargetId currentTargetId = myTarget;
//		double lastSelectionMaxPriority = _ClassifiersAndVirtualActionIntensity[selectionName].LastSelectionMaxPriority;
//		std::multimap<double, std::pair<TClassifierNumber, TTargetId> > mapActivableCS;
//		
//		_pMHiCSbase->selectBehavior(selectionName,_pSensorsValues, mapActivableCS, currentTargetId);
//
///*		if (selectedClassifierNumber < 0)
//		{
//			// ***G*** Ici on décide de rien faire si on sait pas quoi faire. En fait il faudrait créer un règle.
//			_ClassifiersAndVirtualActionIntensity[selectionName].ClassifierNumber = selectedClassifierNumber;
//			_ClassifiersAndVirtualActionIntensity[selectionName].TargetId = currentTargetId;
//			_ClassifiersAndVirtualActionIntensity[selectionName].LastSelectionMaxPriority = lastSelectionMaxPriority;
//			return; 
//		}
//*/		
//		TAction behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);
//
//		// We check the last action selected by the current motivation to remove the motivation influence on this action.
//		if (lastClassifierNumber >= 0)
//		{
//			TAction lastActionName = _pMHiCSbase->getActionPart(selectionName, lastClassifierNumber);
//
//			// We check if we have selected the same behavior.
//			if (lastActionName != behav)
//			{
//				if (_pMHiCSbase->isAnAction(lastActionName))
//				{
//					_ActionsExecutionIntensity[lastActionName].removeProvider(selectionName);
//					// If the action doesn't receive motivation any more, we remove it.
//					double energy = _ActionsExecutionIntensity[lastActionName].getSumValue();
//					if (energy <= 0)
//					{
//						_ActionsExecutionIntensity.erase(lastActionName);
//						_IdByActions.erase(lastActionName);
//						// we check if it was the current action
//						if ((*_ItCurrentAction).first == lastActionName)
//						{
//							_ItCurrentAction = _IdByActions.find(Action_DoNothing);
//							nlassert (_ItCurrentAction != _IdByActions.end());
//						}
//					}
//				}
//				else
//				{
//					_ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.removeProvider(selectionName);
//					spreadMotivationReckon(lastActionName);
//					// If the CS doesn't receive motivation any more, we remove it.
//					double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].MotivationIntensity.getSumValue();
//					if (energy <= 0)
//					{
//						_ClassifiersAndVirtualActionIntensity.erase(lastActionName);
//					}
//				}
//			}
//		}
//
//		// We store the number of the new classifier actived by this motivation.
//		_ClassifiersAndVirtualActionIntensity[selectionName].ClassifierNumber = selectedClassifierNumber;
//		_ClassifiersAndVirtualActionIntensity[selectionName].TargetId = currentTargetId;
//		_ClassifiersAndVirtualActionIntensity[selectionName].LastSelectionMaxPriority = lastSelectionMaxPriority;
//		
//		if (selectedClassifierNumber >= 0)
//		{
//			// We add the current motivation energy to the selected action.
//			if (_pMHiCSbase->isAnAction(behav))
//			{
//				_ActionsExecutionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
//			}
//			else
//			{
//				// Else it must be a virtual action (common CS)
//				_ClassifiersAndVirtualActionIntensity[behav].MotivationIntensity.addProvider(selectionName, pCSselection->MotivationIntensity);
//				spreadMotivationReckon(behav);
//			}
//
//			// We set the Id of this action.
//			// For moment there's no test to see if it is the same target or not. In the futur it can be usefull to make this test
//			// to avoid unwilled target switch.
//			_IdByActions[behav] = currentTargetId;
//		}
//	}
//}

void CMHiCSagent::run()
{
	_ActionsExecutionIntensityByTarget.clear();
	motivationCompute();
//	virtualActionCompute();
}


void CMHiCSagent::setSensors(CCSPerception* psensorMap)
{
	_pSensorsValues = psensorMap;
}


const std::map<TTargetId, std::map<TAction, CMotivationEnergy> >* CMHiCSagent::selectBehavior()
{
	//We sort actions by priority
	double priority;
	TAction action;
	TTargetId target;
	std::multimap<double, std::pair<TTargetId,TAction> > actionsToRemove;

	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >::iterator itActionsExecutionIntensityByTarget;
	std::map<TAction, CMotivationEnergy>::iterator itMotiveByAction;
	for (itActionsExecutionIntensityByTarget  = _ActionsExecutionIntensityByTarget.begin();
		 itActionsExecutionIntensityByTarget != _ActionsExecutionIntensityByTarget.end();
		 itActionsExecutionIntensityByTarget++)
	{
		for (itMotiveByAction  = (*itActionsExecutionIntensityByTarget).second.begin();
			 itMotiveByAction != (*itActionsExecutionIntensityByTarget).second.end();
			 itMotiveByAction++)
		{
			priority = (*itMotiveByAction).second.getSumValue();
			action = (*itMotiveByAction).first;
			target = (*itActionsExecutionIntensityByTarget).first;
				 
			actionsToRemove.insert(std::make_pair(priority, std::make_pair(target,action)));
		}
	}

	_pMHiCSbase->pActionResources->filterMyActions(actionsToRemove);

	std::multimap<double, std::pair<TTargetId,TAction> >::iterator itActionsToRemove;
	for (itActionsToRemove = actionsToRemove.begin();
		 itActionsToRemove != actionsToRemove.end();
		 itActionsToRemove++)
	{
		 target = (*itActionsToRemove).second.first;
		 action = (*itActionsToRemove).second.second;
		 itActionsExecutionIntensityByTarget = _ActionsExecutionIntensityByTarget.find(target);
		 nlassert (itActionsExecutionIntensityByTarget != _ActionsExecutionIntensityByTarget.end());
		 itMotiveByAction = (*itActionsExecutionIntensityByTarget).second.find(action);
		 nlassert (itMotiveByAction != (*itActionsExecutionIntensityByTarget).second.end());
		 (*itActionsExecutionIntensityByTarget).second.erase(action);
		 if ( (*itActionsExecutionIntensityByTarget).second.begin() == (*itActionsExecutionIntensityByTarget).second.end() )
		 {
			 _ActionsExecutionIntensityByTarget.erase(target);
		 }
	}

	return &_ActionsExecutionIntensityByTarget;
}

/// Inform the MHiCSAgent that an action ended
//void CMHiCSagent::behaviorTerminate(TBehaviorTerminate how_does_it_terminate)
//{
//	// ***G*** Tant qu'il n'y a pas d'apprentissage, on se contente de retirer l'action de la liste.
//	// Je pense qu'il faut aussi remettre en question toute les actions virtuel portant sur cette cible.
//	// Remettre en question signifie qu'il faut gardr une trace pour favoriser la continuité d'action sur un perso.
//	TTargetId maCibleRemiseEnQuestion = (*_ItCurrentAction).second;
//	std::map<TAction, TTargetId>::iterator	itIdByActions;
//	for (itIdByActions = _IdByActions.begin(); itIdByActions != _IdByActions.end(); itIdByActions++)
//	{
//		TTargetId scanedId = (*itIdByActions).second;
//		if ( scanedId == maCibleRemiseEnQuestion ) 
//		{
//			TAction theAction = (*itIdByActions).first;
//			if (theAction != Action_DoNothing) // Donothing is a never ending action
//			{
//				// Removing from action
//				_IdByActions.erase(theAction);
//
//				// Removing the virtual_classifier that may be associate
//				_ClassifiersAndVirtualActionIntensity.erase(theAction);
//
//				// Removing from the actionExecutionIntensity
//				_ActionsExecutionIntensity.erase(theAction);
//				_IdByActions.erase(theAction);
//			}
//		}
//
//		_ItCurrentAction = _IdByActions.find(Action_DoNothing);
//		nlassert (_ItCurrentAction != _IdByActions.end());
//	}
//	run();
//}

} // NLAINIMAT

