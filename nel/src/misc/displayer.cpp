/* displayer.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: displayer.cpp,v 1.2 2000/09/25 11:14:23 cado Exp $
 *
 * Little easy displayers implementation
 */

#include <stdio.h>

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif

#include "nel/misc/displayer.h"

namespace NLMISC
{

void CStdDisplayer::display (const std::string& str)
{
	printf("%s", str.c_str ());
	
#ifdef NL_OS_WINDOWS
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

