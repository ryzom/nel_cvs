/** \file automata_desc.h
 * A class which describe a simple success/fail automat.
 *
 * $Id: automata_desc.h,v 1.6 2004/03/12 16:44:58 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_AUTOMATADESC_H
#define NL_AUTOMATADESC_H

#include "nel/misc/stream.h"
#include <list>
#include <map>
#include <set>


/**
 * A class which describe a simple success/fail automat.
 * Each state is identified by an unique Id.
 * The Id IDSUCCESS (-1) is the automat Success state Id.
 * The Id IDFAIL (-2) is the automat Fail state Id.
 * \author Gabriel ROBERT
 * \author Nevrax France
 * \date 2001
 */
class CAutomataDesc : public NLMISC::IStreamable
{
public:
	static const sint32 IDSUCCESS;	// = -1
	static const sint32 IDFAIL;		// = -2

	/// Constructor
	CAutomataDesc();
	/// Destructor
	virtual ~CAutomataDesc() {}

	/// Give the automat name.
	std::string	getAutomatName() const;

	/**
	  * Gives the States linked to the automat entry.
	  * \return A list of state Id.
	  */
	const std::list<sint32> &getEntryStates() const;

	/**
	  * Gives the States linked to the stateId success output.
	  * \return A list of state Id.
	  */
	const std::list<sint32> &getSuccessStates(sint32 stateId) const;

	/**
	  * Gives the States linked to the stateId fail output.
	  * \return A list of state Id.
	  */
	const std::list<sint32> &getFailStates(sint32 stateId) const;

	/// Gives the name of a state.
	std::string getStateName(sint32 stateId) const;

	/** \name Build Fonctions
	 * Those functions are used for building a CAutomataDesc.
	 */
	//@{
	void setAutomatName(std::string name);
	void addState(sint32 id, std::string name);
	void addSuccessState(sint32 id, sint32 successId);
	void addFailState(sint32 id, sint32 faileId);
	void addEntryState(sint32 entryId);
	void setVisitedState(sint32 stateId);
	bool visitedState(sint32 stateId);
	void setExploredState(sint32 stateId);
	//@}

	/// \name Derived from NLMISC::IStreamable
	//@{
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialCont(_States);
		f.serialCont(_EntryStates);
		f.serial(_AutomatName);
	}
	std::string getClassName();
	//@}


	void generateScript();
	std::string removeSpaces(std::string &);

private :
	struct CState
	{
		std::string			Name;
		std::list<sint32>	SuccessStates;
		std::list<sint32>	FailStates;

		void serial(NLMISC::IStream &f)
		{
			f.serial(Name);
			f.serialCont(SuccessStates);
			f.serialCont(FailStates);
		}
	};
	
	std::map<sint32,CState>	_States;
	std::list<sint32>		_EntryStates;
	std::set<sint32>		_VisitedState;
	std::set<sint32>		_ExploredState;
	std::string				_AutomatName;

	bool exploredState(sint32 stateId);
};


#endif // NL_AUTOMATADESC_H

/* End of automata_desc.h */
