/** \file classifier.h
 * A simple Classifier System.
 *
 * $Id: classifier.h,v 1.8 2003/01/30 18:06:15 robert Exp $
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
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"

namespace NLAINIMAT
{

///////////////////////////
// All the Enums
///////////////////////////

/// This type give all the actions and virtual actions (high level action) of an Agent.
enum TAction
{
	Action_CloseCombat = 0,
	Action_DistanceCombat,
	Action_Flee,
	Action_Eat,
	Action_Rest,
	Action_V_Fight,
	Action_Unknown
};

static const NLMISC::CStringConversion<TAction>::CPair stringTableAction [] =
{ 
	{ "CloseCombat", Action_CloseCombat },
	{ "DistanceCombat", Action_DistanceCombat },
	{ "Flee", Action_Flee },
	{ "Eat", Action_Eat },
	{ "Rest", Action_Rest },
	{ "V_Fight", Action_V_Fight }
};
static NLMISC::CStringConversion<TAction> conversionAction(stringTableAction, sizeof(stringTableAction) / sizeof(stringTableAction[0]),  Action_Unknown);

/// This type give all the motivations of an Agent.
enum TMotivation
{
	Motivation_Hunger = 0,
	Motivation_Aggro,
	Motivation_Fear,
	Motivation_Fatigue,
	Motivation_Unknown
};

static const NLMISC::CStringConversion<TMotivation>::CPair stringTableMotivation [] =
{ 
	{ "Hunger", Motivation_Hunger },
	{ "Aggro", Motivation_Aggro },
	{ "Fear", Motivation_Fear },
	{ "Fatigue", Motivation_Fatigue }
};
static NLMISC::CStringConversion<TMotivation> conversionMotivation(stringTableMotivation, sizeof(stringTableMotivation) / sizeof(stringTableMotivation[0]),  Motivation_Unknown);

/// This type give all the sensors of an Agent.
enum TSensor
{
	Sensor_MunitionsAmount = 0,
	Sensor_FoodType,
	Sensor_IsAlive,
	Sensor_IAmStronger,	
	Sensor_IAmAttacked,
	Sensor_Unknown
};

// The conversion table
static const NLMISC::CStringConversion<TSensor>::CPair stringTableSensor [] =
{ 
	{ "MunitionsAmount", Sensor_MunitionsAmount },
	{ "FoodType", Sensor_FoodType },
	{ "IsAlive", Sensor_IsAlive },
	{ "IAmStronger", Sensor_IAmStronger },
	{ "IAmAttacked", Sensor_IAmAttacked }
};
static NLMISC::CStringConversion<TSensor> conversionSensor(stringTableSensor, sizeof(stringTableSensor) / sizeof(stringTableSensor[0]),  Sensor_Unknown);

typedef	std::map<TSensor, char>	TSensorMap;
typedef uint64	TTargetId;

class CCSPerception
{
public:
	CCSPerception();
	virtual ~CCSPerception();

	/// Sensors with no target Id.
	TSensorMap						NoTargetSensors;
	/// Sensors with a target (defined by an uint64) Id.
	std::map<TTargetId, TSensorMap>	TargetSensors;
};


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
		bool	isActivable()	const;
		TSensor	getSensorName()	const;
		char	getValue();

	private :
		std::map<TSensor, char>::const_iterator	_itSensor;			// A reference to the sensor associate with this condition.
		char									_value;				// The condition value;
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
		TAction 								Behavior;
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
	void addClassifier(const TSensorMap &conditionsMap, double priority, TAction behavior);

	/// Merge two CS
	void addClassifierSystem(const CClassifierSystem &cs);

	/**
	  * Select a behavior according to the values in the sensorMap.
	  * \param sensorMap is a map whose key is the sensor name and value the sensor value.
	  * \return is the number of the selected classifier, and the Id of the target.
	  */
	std::pair<sint16, TTargetId> selectBehavior(const CCSPerception* psensorMap);

	/**
	  * Give the action part of a given Classifier.
	  * \param classifierNumber is the number of the classifier.
	  * \return is the condition part of the wanted Classifier.
	  */
	TAction getActionPart(sint16 classifierNumber);

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
	CActionCS(TAction name);
	virtual ~CActionCS();

	/// Return the action name
	TAction getName() const;

	/// Ajout d'une nouvelle règle motivant cette action dans une motivation
	void addMotivationRule (TMotivation motivationName, const TSensorMap &conditionsMap, double priority);

	/// Ajout d'une nouvelle règle motivant cette action dans une action virtuel
	void addVirtualActionRule (TAction virtualActionName, const TSensorMap &conditionsMap, double priority);

	/// Chaine de debug
	void getDebugString (std::string &t) const;

	const std::map<TMotivation, CClassifierSystem>	*getClassifiersByMotivationMap ()		const;
	const std::map<TAction, CClassifierSystem>		*getClassifiersByVirtualActionMap ()	const;

private :
	std::map<TMotivation, CClassifierSystem>	_ClassifiersByMotivation;
	std::map<TAction, CClassifierSystem>		_ClassifiersByVirtualAction;
	TAction										_Name;
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

	typedef	std::map< TMotivation, CMotivationValue>	TEnergyByMotivation;

public :
	CMotivationEnergy();
	virtual ~CMotivationEnergy();

	double	getSumValue() const;

	/// Gestion des classeurs qui apportent la motivation
	void	removeProvider(TMotivation providerName);
	void	removeProvider(TAction providerName);
	void	addProvider(TMotivation providerName, const CMotivationEnergy& providerMotivation);
	void	addProvider(TAction providerName, const CMotivationEnergy& providerMotivation);
	void	updateProvider(TMotivation providerName, const CMotivationEnergy& providerMotivation);
	void	updateProvider(TAction providerName, const CMotivationEnergy& providerMotivation);

	/// Donne la Puissance Propre d'une Motivation
	void setMotivationPP(TMotivation motivationName, double PP);

	/// Fixe la valeur d'une motivation
	void setMotivationValue(TMotivation motivationName, double value);

	/// Chaine de debug
	void getDebugString (std::string &t) const;

private :
	void computeMotivationValue();

	double										_SumValue;
	std::map<TMotivation, TEnergyByMotivation>	_MotivationProviders;
	std::map<TAction, TEnergyByMotivation>		_VirtualActionProviders;
	TEnergyByMotivation							_EnergyByMotivation;	// <MotivationSource, motivationValue>
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
	std::pair<sint16, TTargetId> selectBehavior(TMotivation motivationName, const CCSPerception* psensorMap);
	std::pair<sint16, TTargetId> selectBehavior(TAction motivationName, const CCSPerception* psensorMap);

	/**
	  * Give the action part of a given Classifier.
	  * \param motivationName is the name of the CS
	  * \param classifierNumber is the number of the classifier.
	  * \return is the condition part of the wanted Classifier.
	  */
	TAction getActionPart(TMotivation motivationName, sint16 classifierNumber);
	TAction getActionPart(TAction motivationName, sint16 classifierNumber);

	/// To now if a behav selected by a CS is an action (if not, it's a common CS)
	bool isAnAction(TAction behav) const;

	/// Chaine de debug
	void getDebugString(std::string &t) const;
	
private :
	std::map<TMotivation, CClassifierSystem>	_MotivationClassifierSystems;		// <motivationName, classeur> CS by motivation name.
	std::map<TAction, CClassifierSystem>		_VirtualActionClassifierSystems;	// <virtualActionName, classeur> CS by motivation name.
	std::set<TAction>							_ActionSet;							// Set of all executablle actions
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
	void setMotivationPP(TMotivation motivationName, double PP);

	/// Fixe la valeur d'une motivation
	void setMotivationValue(TMotivation motivationName, double value);

	/// Return the Behavior that must be active
	TAction selectBehavior();

	/// Update the values in the NetCS
	void run();

	/// Update the sensors value
	void setSensors(CCSPerception *psensorMap);

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
			ClassifierNumber		= -1;
			dbgNumberOfActivations	= 0;
		}
	};

	// Will spread the reckon of the motivation value along a motivation branch.
	void spreadMotivationReckon(TMotivation CS);
	void spreadMotivationReckon(TAction CS);

	void motivationCompute();
	void virtualActionCompute();

	private :
	CMHiCSbase*								_pMHiCSbase;							// A pointer on the rules base.
	std::map<TMotivation, CMotivateCS>		_ClassifiersAndMotivationIntensity;		// <motivationName, classeur> the motivationName is also the CS name.
	std::map<TAction, CMotivateCS>			_ClassifiersAndVirtualActionIntensity;	// <virtualActionName, classeur> the virtualActionName is also the CS name.
	CCSPerception*							_pSensorsValues;						// Valeurs des senseurs
	std::map<TAction, CMotivationEnergy>	_ActionsExecutionIntensity;				// <actionName, ExecutionIntensity>
	std::map<TAction, TTargetId>			_IdByActions;							// Id associate with each action.
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
But : tester des règles pour chaque ID d'une perception.
Je dois donc pour chaque senseur savoir s'il dépent d'un ID ou pas.
Pour le moment on passait une liste de senseurs, maintenant se sera une liste d'Id.
Pour faire la selection de l'action d'un CS, il faut maintenant un premier passage des
 senseurs sans ID, puis pour chaque cible un passage des senseurs avec Id.
je pourrais aussi avoir un objet senseur qui sur demande me renvois une liste de tableau de valeurs de senseurs. 


 ****/

} // NLAINIMAT


#endif // NL_CLASSIFIER_H

/* End of classifier.h */
