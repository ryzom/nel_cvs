/** \file mhics.cpp
 * The MHiCS architecture. (Modular Hierarchical Classifiers System)
 *
 * $Id: mhics.cpp,v 1.11 2003/08/21 15:41:43 robert Exp $
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
	_WasPreviouslyActived = false;
	_StartingTime = NLMISC::CTime::getSecondsSince1970();
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

void CMotivationEnergy::addProvider(TMotivation providerName, TClassifierNumber classifierNumber)
{
	_MotivationProviders[providerName].insert(classifierNumber);
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
void CMotivationEnergy::setWasPreviouslyActived(bool yesOrNo)
{
	_WasPreviouslyActived = yesOrNo;
}


void CMotivationEnergy::computeMotivationValue()
{
	_EnergyByMotivation.clear();

	TMotivation lastMotivationName = Motivation_Unknown;
	double lastMaxMotiveValue = 0.0;
	// We look for motivation values comming directly from Motivations
	std::multimap<TMotivation, std::set<TClassifierNumber> >::iterator itMotivationProviders;
	for (itMotivationProviders = _MotivationProviders.begin();
		 itMotivationProviders != _MotivationProviders.end();
		 itMotivationProviders++)
	{
		TMotivation motivationName = (*itMotivationProviders).first;
		std::set<TClassifierNumber>::iterator itClassifierNumber;
		for (itClassifierNumber = (*itMotivationProviders).second.begin(); itClassifierNumber != (*itMotivationProviders).second.end(); itClassifierNumber++)
		{
			TClassifierNumber classierNumber = (*itClassifierNumber);
			uint32 classifierTimer;
			if (_WasPreviouslyActived)
			{
				classifierTimer = _MHiCSagent->getTemporaryClassifierPriorityTime(motivationName, classierNumber)/2;
			}
			else
			{
				classifierTimer = _MHiCSagent->getMHiCSbase()->getPriorityPart(motivationName, classierNumber).getClassifierTimer();
			}
			double motiveValue = _MHiCSagent->getMotivationValue(motivationName);
			double motivePP = _MHiCSagent->getMotivationPP(motivationName);
			double priorityTimer = _MHiCSagent->getMHiCSbase()->getPriorityPart(motivationName, classierNumber).getPriorityTimer();
			double combinedValue = (motiveValue * 1000000) - priorityTimer - classifierTimer;
			if (combinedValue > lastMaxMotiveValue)
			{
				lastMaxMotiveValue = combinedValue;
				_EnergyByMotivation[motivationName].Value = combinedValue;
				_EnergyByMotivation[motivationName].PP = motivePP;
			}
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

const std::map<TMotivation, std::set<TClassifierNumber> >* CMotivationEnergy::getProviders() const
{
	return &_MotivationProviders;
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
								std::multimap<CClassifierPriority, std::pair<TClassifierNumber, TTargetId> > &mapActivableCS)
{
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	if (itMotivationClassifierSystems != _MotivationClassifierSystems.end())
	{
		(*itMotivationClassifierSystems).second.selectBehavior(psensorMap, mapActivableCS);
	}
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

TAction CMHiCSbase::getActionPart(TMotivation motivationName, TClassifierNumber classifierNumber) const
{
	if (classifierNumber == -1) return Action_DoNothing;

	std::map<TMotivation, CClassifierSystem>::const_iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
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

CClassifierPriority CMHiCSbase::getPriorityPart(TMotivation motivationName, TClassifierNumber classifierNumber) const
{
	if (classifierNumber == -1) return CClassifierPriority();
	
	std::map<TMotivation, CClassifierSystem>::const_iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	return (*itMotivationClassifierSystems).second.getPriorityPart(classifierNumber);
}

//void CMHiCSbase::dividePriorityByTheMinPriorityPartInAMotivation(TMotivation motivationName)
//{
//	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
//	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
//	(*itMotivationClassifierSystems).second.dividePriorityByTheMinPriorityPart();
//}


void CMHiCSbase::setPriorityValue(TMotivation motivationName, TClassifierNumber classifierNumber, CClassifierPriority priority)
{
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	(*itMotivationClassifierSystems).second.setPriorityPart(classifierNumber, priority);
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
		CClassifierPriority laforce;
		laforce.setPriorityTimer( atof(leMot.c_str()) );

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

void CMHiCSbase::dbgPrintClassifierPriorityInFile(std::string fileName) const
{
	int i;
	std::map<TMotivation, CClassifierSystem>::const_iterator itMotivationClassifierSystems;
	std::string nomDuFichier = "D:\\jgab\\doc\\IA\\_These\\gnuplot\\TFC\\";
	nomDuFichier += fileName;
	
	NLMISC::CIFile baltazar;
	bool yaqqunauboutdufil = baltazar.open(nomDuFichier.c_str(), true);
	baltazar.close();
	
	NLMISC::COFile melkior;
	std::string ohlabellephrase = "";
	
	if (!yaqqunauboutdufil)
	{
		melkior.open(nomDuFichier.c_str(), false, true);
		
		for(itMotivationClassifierSystems  = _MotivationClassifierSystems.begin();
			itMotivationClassifierSystems != _MotivationClassifierSystems.end();
			itMotivationClassifierSystems++)
		{
			TMotivation laMotive = (*itMotivationClassifierSystems).first;
			for (i= 0; i< (*itMotivationClassifierSystems).second.getClassifierNumber(); i++)
			{
				ohlabellephrase += NLAINIMAT::conversionMotivation.toString(laMotive);
				ohlabellephrase += " ";
				(*itMotivationClassifierSystems).second.getDebugString(i,ohlabellephrase);
//				ohlabellephrase += NLMISC::toString(i);
				
				ohlabellephrase += ";";
			}
		}
		ohlabellephrase += "\n";
	}
	else
	{
		melkior.open(nomDuFichier.c_str(), true, true);
	}

	for(itMotivationClassifierSystems  = _MotivationClassifierSystems.begin();
		itMotivationClassifierSystems != _MotivationClassifierSystems.end();
		itMotivationClassifierSystems++)
	{
		TMotivation laMotive = (*itMotivationClassifierSystems).first;
		for (i= 0; i< (*itMotivationClassifierSystems).second.getClassifierNumber(); i++)
		{
			CClassifierPriority laSuperPrio = getPriorityPart(laMotive, i);
			ohlabellephrase += NLMISC::toString(laSuperPrio.getPriority());
			ohlabellephrase += ";";
		}
	}
	ohlabellephrase += "\n";
	
	
	uint8 *buf = (uint8 *)ohlabellephrase.c_str();
	uint len = ohlabellephrase.length();
	melkior.serialBuffer(buf,len);
	melkior.close();
}

void CMHiCSbase::learningUpdatePriorityValueTimeToSuccess(TMotivation motivationName, TClassifierNumber classifierNumber, uint32 timeToSuccess)
{
	// Le but est de faire une moyenne sur les valeurs de fitness. Pour ça on fait une moyenne sur les 10 pas de temps précédents.
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	uint32 oldTime2Success = (*itMotivationClassifierSystems).second.getPriorityPart(classifierNumber).getPriorityTimer();
	uint32 newTime2Success = (timeToSuccess + oldTime2Success)/2;
	CClassifierPriority newPrio;
	newPrio.setPriorityTimer(newTime2Success);
	newPrio.SetClassifierTimer((*itMotivationClassifierSystems).second.getPriorityPart(classifierNumber).getClassifierTimer());
	(*itMotivationClassifierSystems).second.setPriorityPart(classifierNumber, newPrio);
}
void CMHiCSbase::learningUpdatePriorityValueClassifierTime(TMotivation motivationName, TClassifierNumber classifierNumber, uint32 time)
{
	// Le but est de faire une moyenne sur les valeurs de fitness. Pour ça on fait une moyenne sur les 10 pas de temps précédents.
	std::map<TMotivation, CClassifierSystem>::iterator itMotivationClassifierSystems = _MotivationClassifierSystems.find(motivationName);
	nlassert(itMotivationClassifierSystems != _MotivationClassifierSystems.end());
	uint32 oldTime = (*itMotivationClassifierSystems).second.getPriorityPart(classifierNumber).getClassifierTimer();
	uint32 newTime = (time+oldTime)/2;
	CClassifierPriority newPrio;
	newPrio.setPriorityTimer((*itMotivationClassifierSystems).second.getPriorityPart(classifierNumber).getPriorityTimer());
	newPrio.SetClassifierTimer(newTime);
	(*itMotivationClassifierSystems).second.setPriorityPart(classifierNumber, newPrio);
}


///////////////////////////
// CMHiCSagent
///////////////////////////

CMHiCSagent::CMHiCSagent(CMHiCSbase* pMHiCSbase)
{
	nlassert (pMHiCSbase != NULL);
	_pMHiCSbase = pMHiCSbase;
	_pActionsExecutionIntensityByTarget = new std::map<TTargetId, std::map<TAction, CMotivationEnergy> >();
	_pOldActionsExecutionIntensityByTarget = new std::map<TTargetId, std::map<TAction, CMotivationEnergy> >();
	_Learning = true;
//	_ActionsExecutionIntensity[Action_DoNothing] = CMotivationEnergy();
//	_IdByActions[Action_DoNothing] = NullTargetId;
//	_ItCurrentAction = _IdByActions.find(Action_DoNothing);
}

CMHiCSagent::~CMHiCSagent()
{
	delete _pOldActionsExecutionIntensityByTarget;
	delete _pActionsExecutionIntensityByTarget;
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
	std::map<TMotivation, CMotivationEnergy>::const_iterator itClassifiersAndMotivationIntensity;
	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		ret += "\n <" + conversionMotivation.toString((*itClassifiersAndMotivationIntensity).first) + "> ";
		ret += "[MI=" + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.getSumValue()) + "] :";
		(*itClassifiersAndMotivationIntensity).second.getDebugString(ret);
//		ret += "\n  -> Classifier number " + NLMISC::toString((*itClassifiersAndMotivationIntensity).second.ClassifierNumber); 
		ret += "\n";
	}
//	ret += "\nVirtual Actions :";
//	std::map<TAction, CMotivationEnergy>::const_iterator itClassifiersAndVirtualActionIntensity;
//	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
//		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
//		 itClassifiersAndVirtualActionIntensity++)
//	{
//		ret += "\n <" + conversionAction.toString((*itClassifiersAndVirtualActionIntensity).first) + "> ";
//		ret += "[MI=" + NLMISC::toString((*itClassifiersAndVirtualActionIntensity).second.getSumValue()) + "] :";
//		(*itClassifiersAndVirtualActionIntensity).second.getDebugString(ret);
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
	_ClassifiersAndMotivationIntensity[motivationName].setMotivationPP(motivationName, PP);
//	spreadMotivationReckon(motivationName);
}

/// Retourne la Puissance Propre d'une Motivation
double CMHiCSagent::getMotivationPP(TMotivation motivationName) const
{
	std::map<TMotivation, CMotivationEnergy>::const_iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(motivationName);
	if (itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end()) 
	{
		return (*itClassifiersAndMotivationIntensity).second.getMotivationPP(motivationName);
	}
	else
	{
		return -1;
	}
}

/// Fixe la valeur d'une motivation
void CMHiCSagent::setMotivationValue(TMotivation motivationName, double value)
{
//	if (_Learning)
//	{
//		double lastMotiveValue = _ClassifiersAndMotivationIntensity[motivationName].getMotivationValue(motivationName);
//		// Si la valeur de motivation a diminuée, il est temps d'apprendre
//		if (lastMotiveValue > value)
//		{
//			learningComputationMotivationDecrease(motivationName);
//		}
//	}

	std::map<TMotivation, CMotivationEnergy>::iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(motivationName);
	if (itClassifiersAndMotivationIntensity == _ClassifiersAndMotivationIntensity.end())
	{
		itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.insert(std::make_pair(motivationName,CMotivationEnergy())).first;

		// On en profite pour mettre à zero la date de la première réponse positive d'une motivation
//		_TimeOfLastMotivationValueDecrease[motivationName] = NLMISC::CTime::getSecondsSince1970();
	}
	(*itClassifiersAndMotivationIntensity).second.setMHiCSagent(this);
	(*itClassifiersAndMotivationIntensity).second.setMotivationValue(motivationName, value);
	//	spreadMotivationReckon(motivationName);
}

double	CMHiCSagent::getMotivationValue(TMotivation motivationName) const
{
	std::map<TMotivation, CMotivationEnergy>::const_iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(motivationName);
	if (itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end()) 
	{
		return (*itClassifiersAndMotivationIntensity).second.getMotivationValue(motivationName);
	}
	else
	{
		return -1;
	}
}

/// Retourne l'intensité de motivation reçu par un action virtuel
//double CMHiCSagent::getMotivationIntensity(TAction virtualAction) const
//{
//	std::map<TAction, CMotivationEnergy>::const_iterator itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.find(virtualAction);
//	if (itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end()) 
//	{
//		return (*itClassifiersAndVirtualActionIntensity).second.getSumValue();
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
	itActionsExecutionIntensityByTarget = _pActionsExecutionIntensityByTarget->find(target);
	if (itActionsExecutionIntensityByTarget == _pActionsExecutionIntensityByTarget->end())
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
//	std::map<TMotivation, CMotivationEnergy>::iterator itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(CS);
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
//			std::map<TAction, CMotivationEnergy>::iterator itClassifiersAndVirtualActionIntensity = 
//														_ClassifiersAndVirtualActionIntensity.find(lastActionName);
//			// Test if the virtual action selected hasn't been removed.
//			if (itClassifiersAndVirtualActionIntensity == _ClassifiersAndVirtualActionIntensity.end()) return;
//			
//			// Update the motivation provider for the virtual action execution intensity.
//			(*itClassifiersAndVirtualActionIntensity).second.updateProvider(CS,
//																(*itClassifiersAndMotivationIntensity).second.MotivationIntensity);
//			spreadMotivationReckon(lastActionName);
//			// If the CS doesn't receive motivation any more, we remove it.
//			double energy = (*itClassifiersAndVirtualActionIntensity).second.getSumValue();
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
//	std::map<TAction, CMotivationEnergy>::iterator itClassifiersAndVirtualActionIntensityORIGIN = 
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
//			std::map<TAction, CMotivationEnergy>::iterator itClassifiersAndVirtualActionIntensity = 
//				_ClassifiersAndVirtualActionIntensity.find(lastActionName);
//			// Test if the virtual action selected hasn't been removed.
//			if (itClassifiersAndVirtualActionIntensity == _ClassifiersAndVirtualActionIntensity.end()) return;
//			
//			// Update the motivation provider for the virtual action execution intensity.
//			(*itClassifiersAndVirtualActionIntensity).second.updateProvider(CS,
//														(*itClassifiersAndVirtualActionIntensityORIGIN).second.MotivationIntensity);
//			spreadMotivationReckon(lastActionName);
//			// If the CS doesn't receive motivation any more, we remove it.
//			double energy = (*itClassifiersAndVirtualActionIntensity).second.getSumValue();
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

	std::map<TMotivation, CMotivationEnergy>::iterator itClassifiersAndMotivationIntensity;

	for (itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.begin();
		 itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end();
		 itClassifiersAndMotivationIntensity++)
	{
		CMotivationEnergy* pCSselection = &((*itClassifiersAndMotivationIntensity).second);
		TMotivation selectionName = (*itClassifiersAndMotivationIntensity).first;
		double energy = pCSselection->getSumValue();
		if (energy > 0)
		{
			// On fait calculer le CS
//			TClassifierNumber lastClassifierNumber = pCSselection->ClassifierNumber;
//			TClassifierNumber selectedClassifierNumber = lastClassifierNumber;
//			TTargetId currentTargetId = pCSselection->TargetId;
//			double lastSelectionMaxPriority = pCSselection->LastSelectionMaxPriority;
			std::multimap<CClassifierPriority, std::pair<TClassifierNumber, TTargetId> > mapActivableCS;
			std::multimap<CClassifierPriority, std::pair<TClassifierNumber, TTargetId> >::iterator itMapActivableCS;

			_pMHiCSbase->selectBehavior(selectionName,_pSensorsValues, mapActivableCS);

			for (itMapActivableCS = mapActivableCS.begin(); itMapActivableCS != mapActivableCS.end(); itMapActivableCS++)
			{
				TClassifierNumber selectedClassifierNumber = (*itMapActivableCS).second.first;
				TTargetId currentTargetId = (*itMapActivableCS).second.second;
				behav = _pMHiCSbase->getActionPart(selectionName, selectedClassifierNumber);
//				TClassifierPriority classifierPriority = _pMHiCSbase->getPriorityPart(selectionName, selectedClassifierNumber);
					
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
//							_ClassifiersAndVirtualActionIntensity[lastActionName].removeProvider(selectionName);
//							spreadMotivationReckon(lastActionName);
//							// If the CS doesn't receive motivation any more, we remove it.
//							double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].getSumValue();
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
					std::map<TTargetId, std::map<TAction, CMotivationEnergy> >::const_iterator itOldActionsExecutionIntensityByTarget = (*_pOldActionsExecutionIntensityByTarget).find(currentTargetId);
						if (itOldActionsExecutionIntensityByTarget != (*_pOldActionsExecutionIntensityByTarget).end() )
						{
							std::map<TAction, CMotivationEnergy>::const_iterator itIntensityByTarget = (*itOldActionsExecutionIntensityByTarget).second.find(behav);
							if (itIntensityByTarget != (*itOldActionsExecutionIntensityByTarget).second.end())
							{
								(*_pActionsExecutionIntensityByTarget)[currentTargetId][behav].setWasPreviouslyActived(true);
							}
						}
						(*_pActionsExecutionIntensityByTarget)[currentTargetId][behav].setMHiCSagent(this);
						(*_pActionsExecutionIntensityByTarget)[currentTargetId][behav].addProvider(selectionName, selectedClassifierNumber);
//						_ActionsExecutionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
//					}
//					else
//					{
//						// Else it must be a virtual action (common CS)
//						_ClassifiersAndVirtualActionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
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
//	typedef	std::map<TAction, CMotivationEnergy>::iterator TitNameAndVirtualAction;
//	std::map<double, TitNameAndVirtualAction > mapCSweel;
//	std::map<TAction, CMotivationEnergy>::iterator itClassifiersAndVirtualActionIntensity;
//	// On calcule la somme
//	for (itClassifiersAndVirtualActionIntensity = _ClassifiersAndVirtualActionIntensity.begin();
//		 itClassifiersAndVirtualActionIntensity != _ClassifiersAndVirtualActionIntensity.end();
//		 itClassifiersAndVirtualActionIntensity++)
//	{
//		CMotivationEnergy* pCMotivationEnergy = &((*itClassifiersAndVirtualActionIntensity).second);
//		double energy = pCMotivationEnergy->getSumValue();
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
//		CMotivationEnergy* pCSselection = &((*((*itMapCSweel).second)).second);
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
//					_ClassifiersAndVirtualActionIntensity[lastActionName].removeProvider(selectionName);
//					spreadMotivationReckon(lastActionName);
//					// If the CS doesn't receive motivation any more, we remove it.
//					double energy = _ClassifiersAndVirtualActionIntensity[lastActionName].getSumValue();
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
//				_ClassifiersAndVirtualActionIntensity[behav].addProvider(selectionName, pCSselection->MotivationIntensity);
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
	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >	*bibu = _pOldActionsExecutionIntensityByTarget;
	_pOldActionsExecutionIntensityByTarget = _pActionsExecutionIntensityByTarget;
	_pActionsExecutionIntensityByTarget = bibu;
	_pActionsExecutionIntensityByTarget->clear();
	motivationCompute();
//	virtualActionCompute();
}


void CMHiCSagent::setSensors(CCSPerception* psensorMap)
{
	_pSensorsValues = psensorMap;
}

void CMHiCSagent::setLearning(bool active)
{
	_Learning = active;
}

//void CMHiCSagent::learningComputationMotivationDecrease(TMotivation motivationName)
//{
//	// On calcule le temps en seconde depuis la dernière récompense
//	uint32 previousLastTime = 0;
//	uint32 newTime = NLMISC::CTime::getSecondsSince1970();
//
//	std::map<TMotivation, uint32>::const_iterator itTimeOfLastMotivationValueDecrease = _TimeOfLastMotivationValueDecrease.find(motivationName);
//	if (itTimeOfLastMotivationValueDecrease != _TimeOfLastMotivationValueDecrease.end())
//	{
//		previousLastTime = (*itTimeOfLastMotivationValueDecrease).second;
//	}
//
//	_TimeOfLastMotivationValueDecrease[motivationName] = newTime;
//	TClassifierPriority diffTime = newTime - previousLastTime;
//	nlassert (diffTime > 0);
//
//	// Pour tous les classeurs actifs de la motivation, je met à jour la fitness avec cette nouvelle valeur de temps (et je met à jour le temps).
//	std::multimap<TMotivation, std::pair <TClassifierNumber, uint32> >::iterator itActiveClassifiersByMotivation = _ActiveClassifiersByMotivation.find(motivationName);
//	if (itActiveClassifiersByMotivation != _ActiveClassifiersByMotivation.end() )
//	{
//		uint i;
//		for (i = 0; i < _ActiveClassifiersByMotivation.count(motivationName); i++ )
//		{
//			TClassifierNumber leNumeroDuClasseur = (*itActiveClassifiersByMotivation).second.first;
//			_ActiveClassifiersByMotivation.insert(std::make_pair(motivationName, std::make_pair(leNumeroDuClasseur, newTime)));
//
//			//***G*** TODO : Faire qqchose avec leNumeroDuClasseur
//			_pMHiCSbase->learningUpdatePriorityValue(motivationName, leNumeroDuClasseur, diffTime);
//		}
//	}
//}


void CMHiCSagent::learningComputation()
{
	/*
	 *	Le but de l'apprentissage par ajustement des valeurs de priorité :
	 *	La valeur de priorité est utilisé pour choisir un classeur lorsque plusieurs classeurs sont activable simultanement
	 *	et partagent une même ressource d'action.
	 *	On veut la faire évoluer afin que dans une situation donnée, c'est le classeur le plus apte à satisfaire la motivation 
	 *	qui soit sélectionné.
	 *	Celon le principe de bucket brigade, un classeur qui mène à satisfaire une motivation est un bon classeur.
	 *	Ensuite un classeur qui a défaut de satisfaire la motivation mène à un classeur pouvant la satisfaire est bon mais un peut moins, etc.
	 */
	//	On note les descentes de motivations
//	uint32 previousLastTime;
	uint32 newTime = NLMISC::CTime::getSecondsSince1970();
	
//	std::map<TMotivation, uint32> motivationDiffTime;
	std::set<TMotivation> decreasingMotivations;
//	std::map<TMotivation, double> motivationProgressionByMotivation;
	std::map<TMotivation, CMotivationEnergy>::iterator	itOldClassifiersAndMotivationIntensity, itClassifiersAndMotivationIntensity;
	for(itOldClassifiersAndMotivationIntensity = _OldClassifiersAndMotivationIntensity.begin();
		itOldClassifiersAndMotivationIntensity != _OldClassifiersAndMotivationIntensity.end();
		itOldClassifiersAndMotivationIntensity++)
	{
		TMotivation motivationName = (*itOldClassifiersAndMotivationIntensity).first;
		double oldMV = (*itOldClassifiersAndMotivationIntensity).second.getMotivationValue(motivationName);
		itClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity.find(motivationName);
		nlassert(itClassifiersAndMotivationIntensity != _ClassifiersAndMotivationIntensity.end());
		double newMV = (*itClassifiersAndMotivationIntensity).second.getMotivationValue(motivationName);
		if (newMV < oldMV)
		{
//			motivationProgressionByMotivation[motivationName] = oldMV - newMV;
			// On calcule le temps en seconde depuis la dernière récompense
//			std::map<TMotivation, uint32>::const_iterator itTimeOfLastMotivationValueDecrease = _TimeOfLastMotivationValueDecrease.find(motivationName);
//			nlassert (itTimeOfLastMotivationValueDecrease != _TimeOfLastMotivationValueDecrease.end());
//			previousLastTime = (*itTimeOfLastMotivationValueDecrease).second;
//			_TimeOfLastMotivationValueDecrease[motivationName] = newTime;
//			uint32 diffTime = newTime - previousLastTime;
//			motivationDiffTime[motivationName] = diffTime;
			decreasingMotivations.insert(motivationName);
		}
	}

	// On établit la liste des classeurs utilisés précédemment
	std::map<TMotivation, std::set<TClassifierNumber> > oldClassifierByMotivation;
	std::map<TMotivation, std::set<TClassifierNumber> >::iterator itOldClassifierByMotivation;
	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >::iterator itOldActionsExecutionIntensityByTarget;
	std::map<TAction, CMotivationEnergy>::iterator itOldActionsExecutionIntensity;
	for(itOldActionsExecutionIntensityByTarget  = _pOldActionsExecutionIntensityByTarget->begin();
		itOldActionsExecutionIntensityByTarget != _pOldActionsExecutionIntensityByTarget->end();
		itOldActionsExecutionIntensityByTarget++)
	{
		for(itOldActionsExecutionIntensity  = (*itOldActionsExecutionIntensityByTarget).second.begin();
			itOldActionsExecutionIntensity != (*itOldActionsExecutionIntensityByTarget).second.end();
			itOldActionsExecutionIntensity++)
		{
			const std::map<TMotivation, std::set<TClassifierNumber> >* provounet = (*itOldActionsExecutionIntensity).second.getProviders();
			std::map<TMotivation, std::set<TClassifierNumber> >::const_iterator itProvounet;
			// On met à jour les classeurs qui ont participés à l'évolution des motivations.
			for(itProvounet  = provounet->begin();
				itProvounet != provounet->end();
				itProvounet++)
			{
				TMotivation motivationName = (*itProvounet).first;
				std::set<TClassifierNumber>::const_iterator itClassifierNumber;
				for (itClassifierNumber = (*itProvounet).second.begin(); itClassifierNumber != (*itProvounet).second.end(); itClassifierNumber++)
				{
					TClassifierNumber classifierNumber = (*itClassifierNumber);
//					std::set<TMotivation>::iterator itDecreasingMotivations = decreasingMotivations.find(motivationName);
//					if ( itDecreasingMotivations != decreasingMotivations.end())
//					{
//						CClassifierPriority prio;
//						uint32 diffTimeClassifier = newTime - _TemporaryPriority[motivationName][classifierNumber].StartTime;
//						prio.setPriorityTimer(diffTimeClassifier);
//						prio.SetClassifierTimer(diffTimeClassifier);
//						_pMHiCSbase->learningUpdatePriorityValue(motivationName, classifierNumber, prio);
//						//					double laSuperValeurDeDifferenceDeMotivation = (*itMotivationProgressionByMotivation).second;
//						//					double laPriorityPrecedente = _pMHiCSbase->getPriorityPart(motivationName, classifierNumber);
//						//					double laNouvellePriority = (laPriorityPrecedente+ (laPriorityPrecedente + ((1 - laPriorityPrecedente) * laSuperValeurDeDifferenceDeMotivation)))/2;
//						//					_pMHiCSbase->setPriorityValue(motivationName, classifierNumber, laNouvellePriority);
//						
//						// On en profite pour rediviser l'ensemble des poids par le plus petit afin de ne pas se bloquer vers 1
//						// (vu qu'il n'y a que des renforcements positifs)
//						//					_pMHiCSbase->dividePriorityByTheMinPriorityPartInAMotivation(motivationName);
//					}
					// On établit une liste des classeurs ayant été actifs.
					oldClassifierByMotivation[motivationName].insert(classifierNumber);
				}
			}
		}
	}
	// On remet à zero les compteurs de temps pour les actions qui on satisfait une motivation
	std::set<TMotivation>::iterator itDecreasingMotivations;
	for (itDecreasingMotivations = decreasingMotivations.begin(); itDecreasingMotivations != decreasingMotivations.end(); itDecreasingMotivations++)
	{
		TMotivation motivationName = (*itDecreasingMotivations);
		std::map<TMotivation, std::map<TClassifierNumber, CTemporaryPriority> >::iterator itTemporaryPriority = _TemporaryPriority.find(motivationName);
		if (itTemporaryPriority != _TemporaryPriority.end())
		{
			std::map<TClassifierNumber, CTemporaryPriority>::iterator itPrioByCl;
			for (itPrioByCl = (*itTemporaryPriority).second.begin(); itPrioByCl != (*itTemporaryPriority).second.end(); itPrioByCl++ )
			{
				uint32 timeToSatisfaction = newTime - (*itPrioByCl).second.StartTime;
				TClassifierNumber classifierNumber = (*itPrioByCl).first;
				_pMHiCSbase->learningUpdatePriorityValueTimeToSuccess(motivationName, classifierNumber, timeToSatisfaction);
			}
		}
		_TemporaryPriority.erase(motivationName);
	}
		
	// On établit la liste des classeurs nouveaux utilisés à ce pas ci
	std::map<TMotivation, std::set<TClassifierNumber> > classifierByMotivation;
	std::map<TMotivation, std::set<TClassifierNumber> > stillRunningClassifierByMotivation;
	std::map<TMotivation, std::set<TClassifierNumber> >::iterator itClassifierByMotivation;
	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >::iterator itActionsExecutionIntensityByTarget;
	std::map<TAction, CMotivationEnergy>::iterator itActionsExecutionIntensity;
	std::set<TClassifierNumber>::iterator itUnChtitClassifierNumber;
	for(itActionsExecutionIntensityByTarget  = _pActionsExecutionIntensityByTarget->begin();
		itActionsExecutionIntensityByTarget != _pActionsExecutionIntensityByTarget->end();
		itActionsExecutionIntensityByTarget++)
	{
		for(itActionsExecutionIntensity  = (*itActionsExecutionIntensityByTarget).second.begin();
			itActionsExecutionIntensity != (*itActionsExecutionIntensityByTarget).second.end();
			itActionsExecutionIntensity++)
		{
			const std::map<TMotivation, std::set<TClassifierNumber> >* provounet = (*itActionsExecutionIntensity).second.getProviders();
			std::map<TMotivation, std::set<TClassifierNumber> >::const_iterator itProvounet;
			for(itProvounet  = provounet->begin();
				itProvounet != provounet->end();
				itProvounet++)
			{
				TMotivation motivationName = (*itProvounet).first;
				std::set<TClassifierNumber>::const_iterator itClassifierNumber;
				for (itClassifierNumber = (*itProvounet).second.begin(); itClassifierNumber != (*itProvounet).second.end(); itClassifierNumber++)
				{
					TClassifierNumber classifierNumber = (*itClassifierNumber);
					itOldClassifierByMotivation = oldClassifierByMotivation.find(motivationName);
					if (itOldClassifierByMotivation != oldClassifierByMotivation.end() )
					{
						classifierByMotivation[motivationName].insert(classifierNumber);
						itUnChtitClassifierNumber = (*itOldClassifierByMotivation).second.find(classifierNumber);
						if (itUnChtitClassifierNumber != (*itOldClassifierByMotivation).second.end() )
						{
							// S'il y avait des classeurs de la même motivation, et pas le même classeur, c'est que ce classeur
							// est ancien au seins de cette motivation.
							stillRunningClassifierByMotivation[motivationName].insert(classifierNumber);
						}
					}
//					else
//					{
//						classifierByMotivation[motivationName].insert(classifierNumber);
//					}
				}
			}
		}
	}

	// On regarde quelles sont les nouveaux classeurs activés
	for(itClassifierByMotivation  = classifierByMotivation.begin();
		itClassifierByMotivation != classifierByMotivation.end();
		itClassifierByMotivation++)
	{
		TMotivation motivationName = (*itClassifierByMotivation).first;
		// je commence par prendre le min des prios de mes nouvelles actions à ce tour ci.
		TClassifierPriorityValue leMin = 1000000;
		for(itUnChtitClassifierNumber  = (*itClassifierByMotivation).second.begin();
			itUnChtitClassifierNumber != (*itClassifierByMotivation).second.end();
			itUnChtitClassifierNumber++)
		{
			TClassifierNumber leNumberDuClasseur = (*itUnChtitClassifierNumber);
			TClassifierPriorityValue laPrio = _pMHiCSbase->getPriorityPart(motivationName,leNumberDuClasseur).getPriority();
			leMin = std::min(laPrio, leMin);

			// Si l'action ne fait pas partit des actions précédentes, c'est qu'elle est nouvelle et mérite un newTime et d'être rajoutée dans les classeurs activés.
			std::map<TMotivation, std::set<TClassifierNumber> >::iterator itStillRunningClassifierByMotivation = stillRunningClassifierByMotivation.find(motivationName);
			if (itStillRunningClassifierByMotivation != stillRunningClassifierByMotivation.end())
			{
				std::set<TClassifierNumber>::iterator itLeNumeroDuClasseur = (*itStillRunningClassifierByMotivation).second.find(leNumberDuClasseur);
				if (itLeNumeroDuClasseur == (*itStillRunningClassifierByMotivation).second.end())
				{
					_TemporaryPriority[motivationName][leNumberDuClasseur].StartTime = newTime;
//					_ActivedClassifier[motivationName].insert(leNumberDuClasseur);
				}
			}
			else
			{
				_TemporaryPriority[motivationName][leNumberDuClasseur].StartTime = newTime;
			}
		}
		// Puis je met à jour les anciens classeurs
		itOldClassifierByMotivation = oldClassifierByMotivation.find(motivationName);
		nlassert (itOldClassifierByMotivation != oldClassifierByMotivation.end()); // Normalement c'est assuré par la boucle précédente
		for(itUnChtitClassifierNumber  = (*itOldClassifierByMotivation).second.begin();
			itUnChtitClassifierNumber != (*itOldClassifierByMotivation).second.end();
			itUnChtitClassifierNumber++)
		{
			TClassifierNumber leNumberDuClasseur = (*itUnChtitClassifierNumber);

			std::map<TMotivation, std::set<TClassifierNumber> >::iterator itStillRunningClassifierByMotivation = stillRunningClassifierByMotivation.find(motivationName);
			if (itStillRunningClassifierByMotivation != stillRunningClassifierByMotivation.end())
			{
				std::set<TClassifierNumber>::iterator itLeNumeroDuClasseur = (*itStillRunningClassifierByMotivation).second.find(leNumberDuClasseur);
				if (itLeNumeroDuClasseur != (*itStillRunningClassifierByMotivation).second.end())
				{
					// Le classeur est toujours actif, je mets seulement à jour la valeur de min
//					TClassifierPriorityValue bibuPrio = _TemporaryPriority[motivationName][leNumberDuClasseur].MinPriorityInOtherActiveClassifier;
//					bibuPrio = std::min(bibuPrio, leMin);
//					_TemporaryPriority[motivationName][leNumberDuClasseur].MinPriorityInOtherActiveClassifier = bibuPrio;
					uint32 diffTime = newTime - _TemporaryPriority[motivationName][leNumberDuClasseur].StartTime;
					_TemporaryPriority[motivationName][leNumberDuClasseur].TemporaryClassifierPriorityTime = diffTime;
				}
				else
				{
					// Le classeur vient de finir son activité, je calcule sa nouvelle priorité.
//					TClassifierPriorityValue bibuPrio = _TemporaryPriority[motivationName][leNumberDuClasseur].MinPriorityInOtherActiveClassifier;
//					bibuPrio = std::min(bibuPrio, leMin);
					uint32 diffTime = newTime - _TemporaryPriority[motivationName][leNumberDuClasseur].StartTime;
//					_TemporaryPriority[motivationName].erase(leNumberDuClasseur);
//					CClassifierPriority newPrio;
//					newPrio.setPriorityTimer(bibuPrio);
//					newPrio.SetClassifierTimer(diffTime);
					_pMHiCSbase->learningUpdatePriorityValueClassifierTime(motivationName, leNumberDuClasseur, diffTime);
				}
			}
			else
			{
				// Le classeur vient de finir son activité, je calcule sa nouvelle priorité. (idem ci-dessus)
//					TClassifierPriorityValue bibuPrio = _TemporaryPriority[motivationName][leNumberDuClasseur].MinPriorityInOtherActiveClassifier;
//					bibuPrio = std::min(bibuPrio, leMin);
					uint32 diffTime = newTime - _TemporaryPriority[motivationName][leNumberDuClasseur].StartTime;
//					_TemporaryPriority[motivationName].erase(leNumberDuClasseur);
//					CClassifierPriority newPrio;
//					newPrio.setPriorityTimer(bibuPrio);
//					newPrio.SetClassifierTimer(diffTime);
					_pMHiCSbase->learningUpdatePriorityValueClassifierTime(motivationName, leNumberDuClasseur, diffTime);
			}
		}
	}
}


const std::map<TTargetId, std::map<TAction, CMotivationEnergy> >* CMHiCSagent::selectBehavior()
{
	//We sort actions by priority
	double priority;
	TAction action;
	TTargetId target;
	std::multimap<double, std::pair<TTargetId,TAction> > actionsToRemove;

	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >::iterator itActionsExecutionIntensityByTarget;
	std::map<TAction, CMotivationEnergy>::const_iterator itMotiveByAction;
	for (itActionsExecutionIntensityByTarget  = _pActionsExecutionIntensityByTarget->begin();
		 itActionsExecutionIntensityByTarget != _pActionsExecutionIntensityByTarget->end();
		 itActionsExecutionIntensityByTarget++)
	{
		for (itMotiveByAction  = (*itActionsExecutionIntensityByTarget).second.begin();
			 itMotiveByAction != (*itActionsExecutionIntensityByTarget).second.end();
			 itMotiveByAction++)
		{
			priority = (*itMotiveByAction).second.getSumValue();
			action = (*itMotiveByAction).first;
			target = (*itActionsExecutionIntensityByTarget).first;

			// on rajoute du bruit sur les priorité afin d'avoir une diversité si des priorités sont proches
//			double randomeNumber = (rand()%3);
//			priority += randomeNumber;
			
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
		 itActionsExecutionIntensityByTarget = _pActionsExecutionIntensityByTarget->find(target);
		 nlassert (itActionsExecutionIntensityByTarget != _pActionsExecutionIntensityByTarget->end());
		 itMotiveByAction = (*itActionsExecutionIntensityByTarget).second.find(action);
		 nlassert (itMotiveByAction != (*itActionsExecutionIntensityByTarget).second.end());
		 (*itActionsExecutionIntensityByTarget).second.erase(action);
		 if ( (*itActionsExecutionIntensityByTarget).second.begin() == (*itActionsExecutionIntensityByTarget).second.end() )
		 {
			 _pActionsExecutionIntensityByTarget->erase(target);
		 }
	}

	if (_Learning)
	{
		learningComputation();

		_OldClassifiersAndMotivationIntensity = _ClassifiersAndMotivationIntensity;
	}

	return _pActionsExecutionIntensityByTarget;
}

uint32 CMHiCSagent::getTemporaryClassifierPriorityTime(TMotivation motivationName, TClassifierNumber classifierNumber) const
{
	std::map<TMotivation, std::map<TClassifierNumber, CTemporaryPriority> >::const_iterator	itTemporaryPriority = _TemporaryPriority.find(motivationName);
	if (itTemporaryPriority == _TemporaryPriority.end())
	{
		return 0;
	}
	std::map<TClassifierNumber, CTemporaryPriority>::const_iterator itIteratorBis = (*itTemporaryPriority).second.find(classifierNumber);
	if (itIteratorBis == (*itTemporaryPriority).second.end())
	{
		return 0;
	}
	uint32 bibureturn = (*itIteratorBis).second.TemporaryClassifierPriorityTime;
	return bibureturn;
}


//// Inform the MHiCSAgent that an action ended
//void CMHiCSagent::behaviorTerminate(TAction action, TTargetId target, TBehaviorTerminate how_does_it_terminate)
//{
//	(*_pActionsExecutionIntensityByTarget)[target].erase(action);
//}

} // NLAINIMAT

