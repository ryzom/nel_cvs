/** \file displayer.cpp
 * Little easy displayers implementation
 *
 * $Id: displayer.cpp,v 1.4 2000/11/08 14:59:33 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include <stdio.h>

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on wiindows 95
#define _WIN32_WINDOWS	0x0410
#define WINVER			0x0400
#include <windows.h>
#endif

#include "nel/misc/displayer.h"

namespace NLMISC
{

void CStdDisplayer::display (const std::string& str)
{
	printf("%s", str.c_str ());
	
#ifdef NL_OS_WINDOWS
	// don't install signal is the application is started in debug mode
	if (IsDebuggerPresent ())
		OutputDebugString(str.c_str ());
#endif
}

//****************************************************************************

void CFileDisplayer::display (const std::string& str)
{
	if (_FileName.size () == 0) return;

	FILE *fp = fopen (_FileName.c_str (), "a");
	if (fp == NULL) return;

	fprintf (fp, "%s", str.c_str ());
	
	fclose (fp);
}


//****************************************************************************

void CMsgBoxDisplayer::display (const std::string& str)
{
	MessageBox (NULL, str.c_str (), "", MB_OK | MB_ICONEXCLAMATION);
}



} // MKMISC

