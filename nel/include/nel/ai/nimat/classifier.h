/** \file classifier.h
 * A simple Classifier System.
 *
 * $Id: classifier.h,v 1.1 2001/02/26 14:08:10 robert Exp $
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

namespace NLAINIMAT
{

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
		CClassifierConditionCell(std::map<std::string, char>::iterator itSensor, char value);
		bool isActivable() const;

	private :
		std::map<std::string, char>::iterator	_itSensor;	// A reference to the sensor associate with this condition.
		char									_value;		// The condition value;
	};

	 // A classifier is a three parts components (condition, priority, behavior).
	class CClassifier
	{
	public:
		~CClassifier();

	public :
		std::list<CClassifierConditionCell*>	Condition;
		sint16									Priority;
		std::string								Behavior;
	};

private :
	std::map<std::string, char>	_sensors;		// The sensors are the inputs of the classifier system.
	std::list<CClassifier*>		_classifiers;	// The classifiers are the rules of the classifier system.

public :
	/// Destructor
	~CClassifierSystem();

	/**
	  * Add a new classifier in the classifier system.
	  * \param conditionsMap is a map whose key is the sensor name and value the sensor value.
	  * \param priority is the importance of this rule. The value should be between 0 an 100.
	  * \param behavior is the action to execute if this classifier is selected.
	  */
	void addClassifier(std::map<std::string, char> conditionsMap, sint16 priority, const char* behavior);

	/**
	  * Select a behavior according to the values in the sensorMap.
	  * \param sensorMap is a map whose key is the sensor name and value the sensor value.
	  * \return is the behvior of the selected classifier.
	  */
	std::string selectBehavior(std::map<std::string, char> sensorMap);
};


} // NLAINIMAT


#endif // NL_CLASSIFIER_H

/* End of classifier.h */
