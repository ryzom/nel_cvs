/** \file mhics.h
 * The MHiCS architecture. (Modular Hierarchical Classifiers System)
 *
 * $Id: mhics.h,v 1.6 2003/07/24 17:03:15 robert Exp $
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

#ifndef NL_MHICS_H
#define NL_MHICS_H

#include "nel/misc/types_nl.h"
#include <list>
#include <map>
#include <set>
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
#include "nel/ai/nimat/classifier.h"

namespace NLAINIMAT
{

extern const TTargetId NullTargetId;

class CMHiCSagent;

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
//	void	removeProvider(TAction providerName);
	void	addProvider(TMotivation providerName, TClassifierPriority classifierPriority);
//	void	addProvider(TAction providerName, const CMotivationEnergy& providerMotivation);
//	void	updateProvider(TMotivation providerName, const CMotivationEnergy& providerMotivation);
//	void	updateProvider(TAction providerName, const CMotivationEnergy& providerMotivation);

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

	void setMHiCSagent(CMHiCSagent* pmhicsAgent) {_MHiCSagent = pmhicsAgent;}
	
private :
	void computeMotivationValue();

	CMHiCSagent*								_MHiCSagent;
	double										_SumValue;
	std::multimap<TMotivation, TClassifierPriority>	_MotivationProviders;
//	std::map<TMotivation, TEnergyByMotivation>	_MotivationProviders;
	//	std::map<TAction, TEnergyByMotivation>		_VirtualActionProviders;
	TEnergyByMotivation							_EnergyByMotivation;	// <MotivationSource, motivationValue>
	CMotivationValue							_MyMotivationValue;
};

/**
  * 
  A Modular Hierarchical Classifier System.
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
	void addActionCS(const CActionClassifiers &action);

	/** Add a new virtual action in the net. A virtual action is an action without operator wich is also a motivation.
	 Exemple : Figthing is a virtual action. It may satisfy the anger motivation and is a motivation for guive a sword slash.
	 */
	void addVirtualActionCS(const CActionClassifiers &action);

	/**
	  * Select a behavior according to the values in the sensorMap.
	  * \param motivationName is the name of the CS that must be activated
	  * \param sensorMap is a map whose key is the sensor name and value the sensor value.
	  * \param lastClassifierNumber is the number of the last classifier selected. Also used as a return value.
	  * \param lastTarget is the ID of the last target associated with the last classifier. Also used as a return value.
	  * \param lastSelectionMaxPriority is the Priority given to the last selected classifier
	  *									(witch is the max of all the last activable classifier priority). Also used as a return value.
	*/
	void selectBehavior(TMotivation motivationName,
						const CCSPerception* psensorMap,
						std::multimap<double, std::pair<TClassifierNumber, TTargetId> > &mapActivableCS);

//	void selectBehavior(TAction VirtualActionName,
//						const CCSPerception* psensorMap,
//						std::multimap<double, std::pair<TClassifierNumber, TTargetId> > &mapActivableCS,
//						TTargetId	&target);
		
	/**
	  * Give the action part of a given Classifier.
	  * \param motivationName is the name of the CS
	  * \param classifierNumber is the number of the classifier.
	  * \return is the condition part of the wanted Classifier.
	  */
	TAction getActionPart(TMotivation motivationName, TClassifierNumber classifierNumber);
//	TAction getActionPart(TAction motivationName, TClassifierNumber classifierNumber);
	TClassifierPriority getPriorityPart(TMotivation motivationName, TClassifierNumber classifierNumber);
	
	/// To now if a behav selected by a CS is an action (if not, it's a common CS)
	bool isAnAction(TAction behav) const;

	/// Chaine de debug
	void getDebugString(std::string &t) const;
	void printDebugString() const;

	/// Load classifiers from a file. Return false if thereis a probleme
	bool loadClassifierFromFile(std::string fileName);
	
	CActionResources*	pActionResources;	// Used to select compatible actions.

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
	double	getMotivationValue(TMotivation motivationName) const;

//	/// Retourne l'intensité de motivation reçu par un action virtuel
//	double getMotivationIntensity(TAction virtualAction) const;

	/// Retourne l'intensité d'exécution d'une action
//	double getExecutionIntensity(TAction action) const;

	/// Return the Behavior that must be active
	const std::map<TTargetId, std::map<TAction, CMotivationEnergy> >* selectBehavior();

	/// Inform the MHiCSAgent that an action ended
//	void behaviorTerminate(TBehaviorTerminate how_does_it_terminate);

	/// Update the values in the NetCS
	void run();

	/// Set the snesor source
	void setSensors(CCSPerception *psensorMap);

	/// Chaine de debug
	void getDebugString(std::string &t) const;

	const CMHiCSbase* getMHiCSbase() const {return _pMHiCSbase;}

private :
	class CMotivateCS
	{
	public :
		TClassifierNumber	ClassifierNumber;		// Number of the last classifier actived by this motivation
		TTargetId			TargetId;				// Id of the laste TaretID selected by this motivation
		double				LastSelectionMaxPriority;//LastSelectionMaxPriority is the Priority given to the last selected classifier
		CMotivationEnergy	MotivationIntensity;
		void setMHiCSagent(CMHiCSagent* pmhicsAgent) {_MHiCSagent = pmhicsAgent; MotivationIntensity.setMHiCSagent(_MHiCSagent);}

	public :
		CMotivateCS()
		{
			_MHiCSagent = NULL;
			ClassifierNumber		= -1;
			TargetId = 0; //NullTargetId;
			LastSelectionMaxPriority = 0;
		}

	private :
		CMHiCSagent* _MHiCSagent;
	};

//	class CActionToExecute
//	{
//	public :
//		TAction				Action;
//		TTargetId			TargetId;
//		CMotivationEnergy	ExecutionIntensity;
//
//	public :
//		CActionToExecute()
//		{
//			Action = NLAINIMAT::Action_DoNothing;
//			TargetId = NLAINIMAT::NullTargetId;
//		}
//	};

	// Will spread the reckon of the motivation value along a motivation branch.
//	void spreadMotivationReckon(TMotivation CS);
//	void spreadMotivationReckon(TAction CS);

	void motivationCompute();
// 	void virtualActionCompute();

	private :
	/// function used in debug to change a TTargetId in a string
	std::string targetId2String(TTargetId id) const;

	CMHiCSbase*								_pMHiCSbase;							// A pointer on the rules base.
	std::map<TMotivation, CMotivateCS>		_ClassifiersAndMotivationIntensity;		// <motivationName, classeur> the motivationName is also the CS name.
//	std::map<TAction, CMotivateCS>			_ClassifiersAndVirtualActionIntensity;	// <virtualActionName, classeur> the virtualActionName is also the CS name.
	CCSPerception*							_pSensorsValues;						// Valeurs des senseurs
	std::map<TTargetId, std::map<TAction, CMotivationEnergy> >	_ActionsExecutionIntensityByTarget;
//	std::map<TAction, CMotivationEnergy>	_ActionsExecutionIntensity;				// <actionName, ExecutionIntensity>
//	std::map<TAction, TTargetId>			_IdByActions;							// Id associate with each action (virtual or not).
//	std::map<TAction, TTargetId>::iterator	_ItCurrentAction;						// Iterator on the current active action in _IdByActions
};

} // NLAINIMAT


#endif // NL_MHICS_H

/* End of mhics.h */
