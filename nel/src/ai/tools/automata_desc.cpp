/** \file automata_desc.cpp
 * A class which describe a simple success/fail automat.
 *
 * $Id: automata_desc.cpp,v 1.4 2001/03/29 16:02:30 portier Exp $
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

#include "nel/ai/tools/automata_desc.h"
#include <fstream.h>

const sint32 CAutomataDesc::IDSUCCESS	= -1;
const sint32 CAutomataDesc::IDFAIL		= -2;

	/*
	 * Constructor
	 */
	CAutomataDesc::CAutomataDesc()
	{
		addState(IDSUCCESS, "AUTOMAT SUCCESS STATE");
		addState(IDFAIL, "AUTOMAT FAIL STATE");
	}

	std::string	CAutomataDesc::getAutomatName() const
	{
		return _AutomatName;
	}

	const std::list<sint32> &CAutomataDesc::getEntryStates() const
	{
		return _EntryStates;
	}

	const std::list<sint32> &CAutomataDesc::getSuccessStates(sint32 stateId) const
	{
		std::map<sint32,CState>::const_iterator iStates;
		iStates = _States.find(stateId);
		if (iStates != _States.end())
		{
			return (*iStates).second.SuccessStates;
		}
		else
		{
			std::list<sint32> ret;
			return ret;
		}
	}

	const std::list<sint32> &CAutomataDesc::getFailStates(sint32 stateId) const
	{
		std::map<sint32,CState>::const_iterator iStates;
		iStates = _States.find(stateId);
		if (iStates != _States.end())
		{
			return (*iStates).second.FailStates;
		}
		else
		{
			std::list<sint32> ret;
			return ret;
		}
	}

	std::string CAutomataDesc::getStateName(sint32 stateId) const
	{
		std::map<sint32,CState>::const_iterator iStates;
		iStates = _States.find(stateId);
		if (iStates != _States.end())
		{
			return (*iStates).second.Name;
		}
		else
		{
			return "ERROR : Invalid State ID";
		}
	}

	void CAutomataDesc::setAutomatName(std::string name)
	{
		_AutomatName = name;
	}

	void CAutomataDesc::addState(sint32 id, std::string name)
	{
		if (!exploredState(id))
		{
			_States[id].Name = name;
		}
	}

	void CAutomataDesc::addSuccessState(sint32 id, sint32 successId)
	{
		if (!exploredState(id))
		{
			_States[id].SuccessStates.push_back(successId);
		}
	}

	void CAutomataDesc::addFailState(sint32 id, sint32 faileId)
	{
		if (!exploredState(id))
		{
			_States[id].FailStates.push_back(faileId);
		}
	}

	void CAutomataDesc::addEntryState(sint32 entryId)
	{
		_EntryStates.push_back(entryId);
	}

	bool CAutomataDesc::exploredState(sint32 stateId)
	{
		std::set<sint32>::const_iterator iSet = _ExploredState.find(stateId);
		return (iSet != _ExploredState.end());
	}

	void CAutomataDesc::setExploredState(sint32 stateId)
	{
		_ExploredState.insert(stateId);
	}

	void CAutomataDesc::serial(NLMISC::IStream &f)
	{
		f.serialMap(_States);
		f.serialCont(_EntryStates);
		f.serial(_AutomatName);
	}

	std::string CAutomataDesc::getClassName()
	{
		return "CAutomataDesc";
	}

	void CAutomataDesc::generateScript()
	{
		ofstream tmp_script( "c:\\ryzom\\code\\nel\\tools\\ai\\fsm_script.txt");
		
		// Succes and failure states
/*		tmp_script << "From Actor : Define SuccessState" << endl << "{" << endl;
		tmp_script << "}" << endl << endl;
		tmp_script << "From Actor : Define FailureState" << endl << "{" << endl;
		tmp_script << "}" << endl << endl;
*/
		// Generates states
		std::map<sint32,CState>::const_iterator it_m = _States.begin();
		while ( it_m != _States.end() )
		{
			sint32 state_id = (*it_m).first;
			
			// State name
			std::string state_name = removeSpaces( getStateName( state_id ) );

			tmp_script << "From Actor : Define " << "Actor" << state_name << endl << "{" << endl;

			if ( state_id != IDSUCCESS && state_id != IDFAIL )
			{
#ifdef NL_DEBUG
				tmp_script << "\tOnEnterState()" << endl;
				tmp_script << "\t\tPrint('Etat " << state_name << " actif');" << endl;
				tmp_script << "\tEnd" << endl << endl;
#endif

				// Success transitions
				tmp_script << "\tRunTell(SuccessMsg msg)" << endl;
				std::list<sint32>::const_iterator it_s = getSuccessStates( state_id ).begin();
				while ( it_s != getSuccessStates( state_id ).end() )
				{
					state_name = removeSpaces( getStateName( *it_s ) );
					tmp_script << "\t\tswitch('" << state_name << "');" << endl;
					it_s++;
				}
				tmp_script << "\t\tReturn msg;" << endl;
				tmp_script << "\tEnd" << endl << endl;

				// Failure transitions
				tmp_script << "\tRunTell(FailureMsg msg)" << endl;
				std::list<sint32>::const_iterator it_f = getFailStates( state_id ).begin();
				while ( it_f != getFailStates( state_id ).end() )
				{
					state_name = removeSpaces( getStateName( *it_f ) );
					tmp_script << "\t\tswitch('" << state_name << "');" << endl;
					it_f++;
				}
				tmp_script << "\t\tReturn msg;" << endl;
				tmp_script << "\tEnd" << endl;
			}
			else
			{
				tmp_script << "\tRun()" << endl;
				if ( state_id == IDSUCCESS )
					tmp_script << "\t\tFather().Send(new SuccessMsg(0.0));" << endl;
				else
					tmp_script << "\t\tFather().Send(new FailureMsg(0.0));" << endl;
				tmp_script << "\tEnd" << endl;
			}

			// End of the State class
			tmp_script << "}" << endl << endl;

			it_m++;
		}

		// Generates FSM
		std::string fsm_name = removeSpaces( getAutomatName() );
		tmp_script << "From Fsm : Define " << fsm_name << endl << "{" << endl;

		// Generates states as static components of the FSM
		tmp_script << "\tComponent:" << endl;
		
		it_m = _States.begin();
		while ( it_m != _States.end() )
		{
			// State name
			std::string state_name = removeSpaces( getStateName( (*it_m).first ) );
			tmp_script << "\t\tActor" << state_name << "<'" << state_name << "'>;" << endl;
			it_m++;
		}
		tmp_script << "\tEnd" << endl << endl;

		// Activates entry states
		tmp_script << "\tConstructor()" << endl;
		std::list<sint32>::const_iterator it_e = getEntryStates().begin();
		while ( it_e != getEntryStates().end() )
		{
			std::string state_name = removeSpaces( getStateName( *it_e ) );
			tmp_script << "\t\t" << state_name << ".activate();" << endl;
			it_e++;
		}
		tmp_script << "\tEnd" << endl;

		// End of the Fsm Class
		tmp_script << "}" << endl;

		tmp_script.close();
	}

	std::string CAutomataDesc::removeSpaces(std::string &txt)
	{
		int i = 0;
		char buffer[1024];
		while ( txt[i] != 0 )
		{
			if ( txt[i] == ' ')
				buffer[i] = '_';
			else
				buffer[i] = txt[i];
			i++;
		}
		buffer[i] = 0;
		return std::string( buffer );
	}
