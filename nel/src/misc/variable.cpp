/** \file command.cpp
 * <File description>
 *
 * $Id: variable.cpp,v 1.2 2003/09/03 13:50:56 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/variable.h"

using namespace std;
using namespace NLMISC;

namespace NLMISC {


void cbVarChanged (CConfigFile::CVar &cvar)
{
	for (ICommand::TCommand::iterator comm = (*ICommand::Commands).begin(); comm != (*ICommand::Commands).end(); comm++)
	{
		if ((*comm).second->Type == ICommand::Variable && (*comm).second->_CommandName == cvar.Name)
		{
			IVariable *var = (IVariable *)((*comm).second);
			string val = cvar.asString();
			nlinfo ("Setting variable '%s' with value '%s' from config file", cvar.Name.c_str(), val.c_str());
			var->fromString(val, true);
		}
	}
}

void IVariable::init (NLMISC::CConfigFile &configFile)
{
	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		if ((*comm).second->Type == ICommand::Variable)
		{
			IVariable *var = (IVariable *)((*comm).second);
			if (var->_UseConfigFile)
			{
				configFile.setCallback(var->_CommandName, cbVarChanged);
				CConfigFile::CVar *cvar = configFile.getVarPtr(var->_CommandName);
				if (cvar != 0)
				{
					string val = cvar->asString();
					nlinfo ("Setting variable '%s' with value '%s' from config file '%s'", var->_CommandName.c_str(), val.c_str(), configFile.getFilename().c_str());
					var->fromString(val, true);
				}
				else
				{
					nlwarning ("No variable '%s' in config file '%s'", var->_CommandName.c_str(), configFile.getFilename().c_str());
				}
			}
		}
	}
}
	
} // NLMISC
