/** \file classifier.h
 * A simple Classifier System.
 *
 * $Id: classifier.h,v 1.12 2003/03/14 14:28:50 robert Exp $
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
#include "sensors_motivations_actions_def.h"

namespace NLAINIMAT
{

///////////////////////////
// All the Enums
///////////////////////////

/// This type give all the actions and virtual actions (high level action) of an Agent.
//extern enum TAction;

/// This type give all the motivations of an Agent.
//extern enum TMotivation;

/// This type give all the sensors of an Agent.
//extern enum TSensor;


/// Used to know how a behaviour terminate
enum TBehaviorTerminate
{
	BehaviorTerminate_Success,
	BehaviorTerminate_Failure,
	BehaviorTerminate_Interupt,
	BehaviorTerminate_Unknown
};
static const NLMISC::CStringConversion<TBehaviorTerminate>::CPair stringTableBehaviorTerminate [] =
{ 
	{ "Success",	BehaviorTerminate_Success },
	{ "Failure",	BehaviorTerminate_Failure },
	{ "Interupt",	BehaviorTerminate_Interupt },
};
static NLMISC::CStringConversion<TBehaviorTerminate> conversionBehaviorTerminate
(
 stringTableBehaviorTerminate,
 sizeof(stringTableBehaviorTerminate) / sizeof(stringTableBehaviorTerminate[0]),
 BehaviorTerminate_Unknown
);

typedef	char	TSensorValue;
typedef	std::map<TSensor, TSensorValue>	TSensorMap;
typedef uint32	TTargetId;

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
  * A condition map.
  * Used to describes all the conditions that must be associate to an CActionCS.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2003
  */
class CConditionMap
{
public:
	struct CSensor
	{
		TSensorValue	SensorValue;
		bool			TruthValue;
		bool			NeedTarget;
	};
	
	/// Used to add an if sensor == condition rule
	void addIfSensorCondition(TSensor sensorName, TSensorValue sensorValue);

	/// Used to add an if sensor != condition rule
	void addIfNotSensorCondition(TSensor sensorName, TSensorValue sensorValue);

	void addSensorCondition(TSensor sensorName, TSensorValue sensorValue, bool sensorIsTrue);

	std::map<TSensor, CSensor >::const_iterator begin() const {return _ConditionMap.begin();}
	std::map<TSensor, CSensor >::const_iterator end() const {return _ConditionMap.end();}
	void clear(){_ConditionMap.clear();}

private:
	std::map<TSensor, CSensor > _ConditionMap;// The boolean is used to know if the condition is sensor==value or sensor!=value.
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
		CClassifierConditionCell(TSensorMap::const_iterator itSensor, TSensorValue value, bool sensorIsTrue);
		bool			isActivable()	const;
		TSensor			getSensorName()	const;
		TSensorValue	getValue()		const;
		bool			getSensorIsTrue() const;

	private :
		std::map<TSensor, TSensorValue>::const_iterator	_itSensor;			// A reference to the sensor associate with this condition.
		TSensorValue									_Value;				// The condition value.
		bool											_SensorIsTrue;		// Used to know if the condition is sensor==value or sensor!=value.
	};

	 // A classifier is a three parts components (condition, priority, behavior).
	class CClassifier
	{
	public:
		CClassifier();
		virtual ~CClassifier();

	public :
		std::list<CClassifierConditionCell*>	ConditionWithTarget;
		std::list<CClassifierConditionCell*>	ConditionWithoutTarget;
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
	void addClassifier(const CConditionMap &conditionsMap, double priority, TAction behavior);

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
	void addMotivationRule (TMotivation motivationName, const CConditionMap &conditionsMap, double priority);

	/// Ajout d'une nouvelle règle motivant cette action dans une action virtuel
	void addVirtualActionRule (TAction virtualActionName, const CConditionMap &conditionsMap, double priority);

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

	/// Retourne la Puissance Propre d'une Motivation
	double getMotivationPP(TMotivation motivationName) const;

	/// Fixe la valeur d'une motivation
	void	setMotivationValue(TMotivation motivationName, double value);

	/// Retourne la valeur d'une motiation
	double	getMotivationValue(TMotivation motivationName) const;
	
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
	std::pair<sint16, TTargetId> selectBehavior(TAction motivationName, const CCSPerception* psensorMap, TTargetId target);

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

	/// Retourne la Puissance Propre d'une Motivation
	double getMotivationPP(TMotivation motivationName) const;
	
	/// Fixe la valeur d'une motivation
	void setMotivationValue(TMotivation motivationName, double value);

	/// Retourne la valeur d'une motiation
	double	getMotivationValue(TMotivation motivationName) const ;

	/// Return the Behavior that must be active
	std::pair<TAction, TTargetId> selectBehavior();

	/// Inform the MHiCSAgent that an action ended
	void behaviorTerminate(TBehaviorTerminate how_does_it_terminate);

	/// Update the values in the NetCS
	void run();

	/// Set the snesor source
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
	std::map<TAction, TTargetId>			_IdByActions;							// Id associate with each action (virtual or not).
	std::map<TAction, TTargetId>::iterator	_ItCurrentAction;						// Iterator on the current active action in _IdByActions
};

} // NLAINIMAT


#endif // NL_CLASSIFIER_H

/* End of classifier.h */
