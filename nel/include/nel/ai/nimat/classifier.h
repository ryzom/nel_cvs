/** \file classifier.h
 * A simple Classifier System.
 *
 * $Id: classifier.h,v 1.7 2003/01/21 16:35:26 robert Exp $
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

#ifndef NL_CLASSIFIER_H
#define NL_CLASSIFIER_H

#include "nel/misc/types_nl.h"
#include <list>
#include <map>
#include <set>

namespace NLAINIMAT
{

/**
  * This class store all the actions and virtual actions (high level action) of an Agent.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2003
  */
class CActionsBox
{
public :
	enum TAction
	{
		closeCombat,
		distanceCombat,
		flee,
		eat,
		rest,
		v_fight
	};

	CActionsBox();
	virtual ~CActionsBox();
	
	/**
	  * Execute a given action on a given target Id.
	  * \param actionId is the Id of the requested action
	  * \param targetID is the Id of the target on wich the action will be executed.
	  */
	void executeAction(TAction actionId, uint64 targetID);

	static std::string getNameFromId(TAction actionId);
};

/**
  * This class store all the motivations of an Agent.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2003
  */
class CMotivationsBox
{
public :
	enum TMotivation
	{
		HUNGER,
		AGGRO,
		FEAR,
		FATIGUE
	};

	CMotivationsBox();
	virtual ~CMotivationsBox();
	
	/**
	  * Give the Motivation Value of a Motivation.
	  * \param actionId is the Id of the requested action
	  * \param targetID is the Id of the target on wich the action will be executed.
	  */
	double getMotivationValue(TMotivation motivationId);

	static std::string getNameFromId(TMotivation motivationId);
};

/**
  * This class store all the sensors of an Agent.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2003
  */
class CSensorsBox
{
public :
	enum TSensor
	{
		MunitionsAmount,
		FoodType,
		IsAlive,
		IAmStronger,
		IAmAttacked
	};

	CSensorsBox();
	virtual ~CSensorsBox();
	
	/**
	  * Give the value of a given sensor and a given target Id.
	  * \param sensorId is the Id of the requested sensor
	  * \param targetID is the Id of the target on wich the sensor must compute.
	  * \return the sensor value.
	  */
	char getSensorValue(TSensor sensorId, uint64 targetID);

	static std::string getNameFromId(TSensor sensorId);
};

typedef	std::map<CSensorsBox::TSensor, char>	TSensorMap;

/**
  * A simple and minimal version of a Classifier System.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2001
  */
class CClassifierSystem
{
private :
	// It's an atomic condition in the condition part of a CClassifier
	class CClassifierConditionCell
	{
	public :
		CClassifierConditionCell(TSensorMap::const_iterator itSensor, char value);
		bool isActivable() const;
		CSensorsBox::TSensor getSensorName() const;
		char getValue();

	private :
		std::map<CSensorsBox::TSensor, char>::const_iterator	_itSensor;			// A reference to the sensor associate with this condition.
		char													_value;				// The condition value;
	};

	 // A classifier is a three parts components (condition, priority, behavior).
	class CClassifier
	{
	public:
		CClassifier();
		virtual ~CClassifier();

	public :
		std::list<CClassifierConditionCell*>	Condition;
		double									Priority;
		CActionsBox::TAction 					Behavior;
	};

private :
	TSensorMap						_sensors;			// The sensors are the inputs of the classifier system.
	std::map<sint16, CClassifier*>	_classifiers;		// The classifiers are the rules of the classifier system.
	sint16							_ClassifierNumber;	// Number of classifiers

public :
	/// Destructor
	CClassifierSystem();
	virtual ~CClassifierSystem();

	/**
	  * Add a new classifier in the classifier system.
	  * \param conditionsMap is a map whose key is the sensor name and value the sensor value.
	  * \param priority is the importance of this rule. The value should be between 0 an 1.
	  * \param behavior is the action to execute if this classifier is selected.
	  */
	void addClassifier(const TSensorMap &conditionsMap, double priority, CActionsBox::TAction behavior);

	/// Merge two CS
	void addClassifierSystem(const CClassifierSystem &cs);

	/**
	  * Select a behavior according to the values in the sensorMap.
	  * \param sensorMap is a map whose key is the sensor name and value the sensor value.
	  * \return is the number of the selected classifier.
	  */
	sint16 selectBehavior(const TSensorMap &sensorMap);

	/**
	  * Give the action part of a given Classifier.
	  * \param classifierNumber is the number of the classifier.
	  * \return is the condition part of the wanted Classifier.
	  */
	CActionsBox::TAction getActionPart(sint16 classifierNumber);

	void getDebugString(std::string &t) const;
};

/**
  * An action for a Classifier System.
  * Used to describes all the rules (classifiers) wich will have this action in the condition part.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2002
  */
class CActionCS
{
public :
	CActionCS(CActionsBox::TAction name);
	virtual ~CActionCS();

	/// Return the action name
	CActionsBox::TAction getName() const;

	/// Ajout d'une nouvelle règle motivant cette action dans une motivation
	void addMotivationRule (CMotivationsBox::TMotivation motivationName, const TSensorMap &conditionsMap, double priority);

	/// Ajout d'une nouvelle règle motivant cette action dans une action virtuel
	void addVirtualActionRule (CActionsBox::TAction virtualActionName, const TSensorMap &conditionsMap, double priority);

	/// Chaine de debug
	void getDebugString (std::string &t) const;

	const std::map<CMotivationsBox::TMotivation, CClassifierSystem> *getClassifiersByMotivationMap () const;
	const std::map<CActionsBox::TAction, CClassifierSystem>			*getClassifiersByVirtualActionMap () const;

private :
//	std::map<std::string, CClassifierSystem>	_ClassifiersByMotivation;
//	std::string									_Name;
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>	_ClassifiersByMotivation;
	std::map<CActionsBox::TAction, CClassifierSystem>			_ClassifiersByVirtualAction;
	CActionsBox::TAction										_Name;
};

/**
  * A Class for manage witch source motivate a CS or an action
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2002
  */
class CMotivationEnergy
{
	class CMotivationValue
	{
	public :
		double Value;
		double PP;
		CMotivationValue()
		{
			Value = 0;
			PP = 0;
		}
	};

	typedef	std::map< CMotivationsBox::TMotivation, CMotivationValue>	TEnergyByMotivation;

public :
	CMotivationEnergy();
	virtual ~CMotivationEnergy();

	double	getSumValue() const;

	/// Gestion des classeurs qui apportent la motivation
	void	removeProvider(CMotivationsBox::TMotivation providerName);
	void	removeProvider(CActionsBox::TAction providerName);
	void	addProvider(CMotivationsBox::TMotivation providerName, const CMotivationEnergy& providerMotivation);
	void	addProvider(CActionsBox::TAction providerName, const CMotivationEnergy& providerMotivation);
	void	updateProvider(CMotivationsBox::TMotivation providerName, const CMotivationEnergy& providerMotivation);
	void	updateProvider(CActionsBox::TAction providerName, const CMotivationEnergy& providerMotivation);

	/// Donne la Puissance Propre d'une Motivation
	void setMotivationPP(CMotivationsBox::TMotivation motivationName, double PP);

	/// Fixe la valeur d'une motivation
	void setMotivationValue(CMotivationsBox::TMotivation motivationName, double value);

	/// Chaine de debug
	void getDebugString (std::string &t) const;

private :
	void computeMotivationValue();

	double														_SumValue;
	std::map<CMotivationsBox::TMotivation, TEnergyByMotivation>	_MotivationProviders;
	std::map<CActionsBox::TAction, TEnergyByMotivation>			_VirtualActionProviders;
	TEnergyByMotivation											_EnergyByMotivation; // <MotivationSource, motivationValue>
};

/**
  * A Modular Hierarchical Classifier System.
  * This is the base component where all rules are stored.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2002
  */
class CMHiCSbase
{
public :
	CMHiCSbase();
	virtual ~CMHiCSbase();

	/// Add a new action in the net.
	void addActionCS(const CActionCS &action);

	/** Add a new virtual action in the net. A virtual action is an action without operator wich is also a motivation.
	 Exemple : Figthing is a virtual action. It may satisfy the anger motivation and is a motivation for guive a sword slash.
	 */
	void addVirtualActionCS(const CActionCS &action);

	/**
	  * Select a behavior according to the values in the sensorMap.
	  * \param motivationName is the name of the CS that must be activated
	  * \param sensorMap is a map whose key is the sensor name and value the sensor value.
	  * \return is the number of the the selected classifier.
	  */
	sint16 selectBehavior(CMotivationsBox::TMotivation motivationName, const TSensorMap &sensorMap);
	sint16 selectBehavior(CActionsBox::TAction motivationName, const TSensorMap &sensorMap);

	/**
	  * Give the action part of a given Classifier.
	  * \param motivationName is the name of the CS
	  * \param classifierNumber is the number of the classifier.
	  * \return is the condition part of the wanted Classifier.
	  */
	CActionsBox::TAction getActionPart(CMotivationsBox::TMotivation motivationName, sint16 classifierNumber);
	CActionsBox::TAction getActionPart(CActionsBox::TAction motivationName, sint16 classifierNumber);

	/// To now if a behav selected by a CS is an action (if not, it's a common CS)
	bool isAnAction(CActionsBox::TAction behav) const;

	/// Chaine de debug
	void getDebugString(std::string &t) const;
	
private :
	std::map<CMotivationsBox::TMotivation, CClassifierSystem>	_MotivationClassifierSystems;		// <motivationName, classeur> CS by motivation name.
	std::map<CActionsBox::TAction, CClassifierSystem>			_VirtualActionClassifierSystems;	// <virtualActionName, classeur> CS by motivation name.
	std::set<CActionsBox::TAction>								_ActionSet;							// Set of all executablle actions
//	std::map<std::string, CClassifierSystem>	_ClassifierSystems;		// <motivationName, classeur> CS by motivation name.
//	std::set<std::string>						_ActionSet;				// Set of all executablle actions
};

/**
  * A Modular Hierarchical Classifier System.
  * This is the agent component where motivations levels and perceptions are stored.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2002
  */
class CMHiCSagent
{
public :

	CMHiCSagent(CMHiCSbase* pMHiCSbase);
	virtual ~CMHiCSagent();

	/// Donne la Puissance Propre d'une Motivation
	void setMotivationPP(CMotivationsBox::TMotivation motivationName, double PP);

	/// Fixe la valeur d'une motivation
	void setMotivationValue(CMotivationsBox::TMotivation motivationName, double value);

	/// Return the Behavior that must be active
	CActionsBox::TAction selectBehavior();

	/// Update the values in the NetCS
	void run();

	/// Update the sensors value
	void setSensors(const TSensorMap &sensorMap);

	/// Chaine de debug
	void getDebugString(std::string &t) const;

private :
	class CMotivateCS
	{
	public :
		sint16				ClassifierNumber;		// Number of the last classifier actived by this motivation
		CMotivationEnergy	MotivationIntensity;
		uint16				dbgNumberOfActivations;	// For debug purpose
	public :
		CMotivateCS()
		{
			ClassifierNumber	= -1;
			dbgNumberOfActivations	= 0;
		}
	};

	// Will spread the reckon of the motivation value along a motivation branch.
	void spreadMotivationReckon(CMotivationsBox::TMotivation CS);
	void spreadMotivationReckon(CActionsBox::TAction CS);

	void motivationCompute();
	void virtualActionCompute();

	private :
	CMHiCSbase*											_pMHiCSbase;							// A pointer on the rules base.
	std::map<CMotivationsBox::TMotivation, CMotivateCS>	_ClassifiersAndMotivationIntensity;		// <motivationName, classeur> the motivationName is also the CS name.
	std::map<CActionsBox::TAction, CMotivateCS>			_ClassifiersAndVirtualActionIntensity;	// <virtualActionName, classeur> the virtualActionName is also the CS name.
	TSensorMap											_SensorsValues;							// Valeurs des senseurs
	std::map<CActionsBox::TAction, CMotivationEnergy>	_ActionsExecutionIntensity;				// <actionName, ExecutionIntensity>
};


/****
MHiCS s'articule autoure d'une partie active (MHiCSagent) associé à chaque agent et une partie passive (MHiCSbase) qui est la base des règles partagée.

MHiCSagent :
------------
Composents :
 - Une map de valeurs de senseurs
 - Un ensemble de valeurs de motivations (MV & RP)
 - Une valeur de motivation d'ordre (associé directement à une action virtuel ou non)
 - Une référence à un MHiCSbase
 - Une trace des classeurs actifs (MI)
 - Une trace des actions actives (EI)

MHiCSbase :
-----------
Composents :
 - Un ensemble de CS


  NOTES :
  -------
  Il va falloir que je sépare mes entrées en action, virtual action et motivations.

 ****/

} // NLAINIMAT


#endif // NL_CLASSIFIER_H

/* End of classifier.h */
