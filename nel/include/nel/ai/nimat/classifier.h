/** \file classifier.h
 * A simple Classifier System.
 *
 * $Id: classifier.h,v 1.15 2003/07/04 15:09:52 robert Exp $
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
#include "nel/ai/nimat/sensors_motivations_actions_def.h"

namespace NLAINIMAT
{

/// Used to know how a behaviour terminate
enum TBehaviorTerminate
{
	BehaviorTerminate_Success,
	BehaviorTerminate_Failure,
	BehaviorTerminate_Interupt, //***G*** Heu... pour le moment une action n'est pas prévenue
								//quand elle est intérompue et à priori ne s'intérompt pas elle même
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

typedef	char							TSensorValue;
typedef	std::map<TSensor, TSensorValue>	TSensorMap;

/**
  * Base interface for an action function.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2003
  */
class ICSAction
{
public :
	ICSAction(){;}
	virtual ~ICSAction();

	/// Used to know if the action is target dependent.
	virtual bool needTarget();

	/// Used to know if an action is ativable.
	virtual bool isActivable();
	virtual bool isActivable(TTargetId);
};

/**
  * Storage of perception maps.
  * Used to store the perception dependant and independant of target.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2003
  */
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
  * Used to describes all the conditions that must be associate to an action.
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

		bool isActivable() const;

	public :
		std::list<CClassifierConditionCell*>	ConditionWithTarget;
		std::list<CClassifierConditionCell*>	ConditionWithoutTarget;
		double									Priority;
		TAction 								Behavior;
	};

	typedef	std::map<sint16, CClassifier*>::iterator  TitClassifiers;

private :
	// Functions used in selectBehavior
	void	updateNoTargetSensors(const CCSPerception* psensorMap);
	void	updateTargetSensors(const CCSPerception* psensorMap, TTargetId target);
	void	RAZTargetSensors();
	void	RAZNoTargetSensors();
	double	computeMaxPriorityDoingTheSameAction(const CCSPerception* psensorMap,
												 sint16 lastClassifierNumber, 
												 TTargetId lastTarget, 
												 double lastSelectionMaxPriority,
												 std::multimap<double, std::pair<TitClassifiers, TTargetId> > &mapActivableCS);
	double	computeHigherPriority(const CCSPerception* psensorMap,
								  double maxPriorityDoingTheSameAction,
								  std::multimap<double, std::pair<TitClassifiers, TTargetId> > &mapActivableCS);
	std::multimap<double, std::pair<TitClassifiers, TTargetId> >::iterator 	roulletteWheelVariation(std::multimap<double, std::pair<TitClassifiers, TTargetId> > &mapActivableCS,
																									std::multimap<double, std::pair<TitClassifiers, TTargetId> >::iterator itMapActivableCS);
		
private :
	TSensorMap						_Sensors;			// The sensors are the inputs of the classifier system.
	std::map<sint16, CClassifier*>	_Classifiers;		// The classifiers are the rules of the classifier system.
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
	  * \param mapActivableCS is the return value for the map containing all the activable CS. double is the priority, sint16 the classifier number and TTargetId la cible de l'action
	  */
	void selectBehavior(const CCSPerception* psensorMap, 
						std::multimap<double, std::pair<sint16, TTargetId> > &mapActivableCS);

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
class CActionClassifiers
{
public :
	CActionClassifiers();
	CActionClassifiers(TAction name);
	virtual ~CActionClassifiers();

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

} // NLAINIMAT


#endif // NL_CLASSIFIER_H

/* End of classifier.h */
