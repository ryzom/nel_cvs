/** \file classifier.h
 * A simple Classifier System.
 *
 * $Id: classifier.h,v 1.6 2002/12/26 14:46:52 robert Exp $
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

typedef	std::map< std::string , char>	TSensorMap;

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
		std::string getSensorName() const;
		char getValue();

	private :
		std::map<std::string, char>::const_iterator	_itSensor;			// A reference to the sensor associate with this condition.
		char										_value;				// The condition value;
	};

	 // A classifier is a three parts components (condition, priority, behavior).
	class CClassifier
	{
	public:
		CClassifier();
		~CClassifier();

	public :
		std::list<CClassifierConditionCell*>	Condition;
		double									Priority;
		std::string								Behavior;
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
	void addClassifier(const TSensorMap &conditionsMap, double priority, const char* behavior);

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
	std::string getActionPart(sint16 classifierNumber);

	void getDebugString(std::string &t) const;
};

/* ***G***
 * Ce que je dois encore ajouter :
 * Une fonction d'apprentissage
 * Une fonction de création de nouveaux classeurs
 * Une "anticipation" à ma façon
 *
 * Architecture NetCS
 * 
 */

/**
  * An action for a Classifier System.
  * \author Gabriel ROBERT
  * \author Nevrax France
  * \date 2002
  */
class CActionCS
{
public :
	CActionCS(std::string name);
	virtual ~CActionCS();

	/// Return the action name
	std::string getName() const;

	/// Ajout d'une nouvelle règle motivant cette action
	void addMotivationRule (std::string motivationName, const TSensorMap &conditionsMap, double priority);

	/// Chaine de debug
	void getDebugString (std::string &t) const;

	const std::map<std::string, CClassifierSystem> *getClassifiersMap () const;

private :
	std::map<std::string, CClassifierSystem>	_ClassifiersByMotivation;
	std::string									_Name;
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

	typedef	std::map< std::string , CMotivationValue>	TEnergyByMotivation;

public :
	CMotivationEnergy();
	virtual ~CMotivationEnergy();

	double	getSumValue() const;
	void	removeProvider(std::string providerName);
	void	addProvider(std::string providerName, const CMotivationEnergy& providerMotivation);
	void	updateProvider(std::string providerName, const CMotivationEnergy& providerMotivation);

	/// Donne la Puissance Propre d'une Motivation
	void setMotivationPP(std::string motivationName, double PP);

	/// Fixe la valeur d'une motivation
	void setMotivationValue(std::string motivationName, double value);

	/// Chaine de debug
	void getDebugString (std::string &t) const;

private :
	void computeMotivationValue();

	double										_SumValue;
	std::map<std::string, TEnergyByMotivation>	_MotivationProviders;
	TEnergyByMotivation							_EnergyByMotivation; // <MotivationSource, motivationValue>
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
	sint16 selectBehavior(std::string motivationName, const TSensorMap &sensorMap);

	/**
	  * Give the action part of a given Classifier.
	  * \param motivationName is the name of the CS
	  * \param classifierNumber is the number of the classifier.
	  * \return is the condition part of the wanted Classifier.
	  */
	std::string getActionPart(std::string motivationName, sint16 classifierNumber);

	// To now if a behav selected by a CS is an action (if not, it's a common CS)
	bool isAnAction(std::string behav) const;

	/// Chaine de debug
	void getDebugString(std::string &t) const;
	
private :
	std::map<std::string, CClassifierSystem>	_ClassifierSystems;	// <motivationName, classeur> CS by motivation name.
	std::set<std::string>						_ActionSet;			// Set of all executablle actions
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
	void setMotivationPP(std::string motivationName, double PP);

	/// Fixe la valeur d'une motivation
	void setMotivationValue(std::string motivationName, double value);

	/// Return the Behavior that must be active
	std::string selectBehavior();

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
		sint16				ClassifierNumber;
		CMotivationEnergy	MotivationIntensity;
		uint16				NumberOfActivations;
	public :
		CMotivateCS()
		{
			ClassifierNumber	= -1;
			NumberOfActivations	= 0;
		}
	};

	// Will spread the reckon of the motivation value along a motivation branch.
	void spreadMotivationReckon(std::string commonCS);

	CMHiCSbase*									_pMHiCSbase;						// A pointer on the rules base.
	std::map<std::string, CMotivateCS>			_ClassifiersAndMotivationIntensity;	// <motivationName, classeur> the motivationName is also the CS name.
	TSensorMap									_SensorsValues;						// Valeurs des senseurs
	std::map<std::string, CMotivationEnergy>	_ActionsExecutionIntensity;			// <actionName, ExecutionIntensity>
};

/****
	CNetCS :
		- renvois l'action à effectuer
		- contient tous les CS
		+ addAction
		+ setPerception
		+ run
		+ selectBehavior

	CMotivateTrack :
		- Il y en a une par ID d'objet (plus une quand y'a personne ?)
		- contient les niveaux de motivations de chaque CS ainsi que les actions élues de chaque CS.

	CSensorCenter :
		- centralise toutes la valeurs des senseurs
		- il y a une liste par ID
 ****/

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

 ****/

} // NLAINIMAT


#endif // NL_CLASSIFIER_H

/* End of classifier.h */
