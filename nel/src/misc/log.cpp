/* log.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: log.cpp,v 1.1 2000/09/21 12:30:15 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include <stdarg.h>

#include <algorithm>
#include <vector>

#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

namespace NLMISC
{

void CLog::addDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL) return;

	std::vector<IDisplayer *>::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		_Displayers.push_back (displayer);
	}
}

void CLog::removeDisplayer (IDisplayer *displayer)
{
	if (displayer == NULL) return;

	std::vector<IDisplayer *>::iterator idi = std::find (_Displayers.begin (), _Displayers.end (), displayer);
	if (idi == _Displayers.end ())
	{
		/// Displayer not found
	}
	else
	{
		_Displayers.erase (idi);
	}
}

void CLog::display (const std::string format, ...)
{
	char		string[1024];

	va_list args;
	va_start (args, format);
	vsprintf (string, format.c_str (), args);
	va_end (args);

	for (std::vector<IDisplayer *>::iterator idi = _Displayers.begin (); idi < _Displayers.end (); idi++)
	{
		(*idi)->display (string);
	}
}

} // NLMISC