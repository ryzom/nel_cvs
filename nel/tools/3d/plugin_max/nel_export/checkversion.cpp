/** \file checkversion.cpp
 *
 * $Id: checkversion.cpp,v 1.2 2001/07/18 14:48:51 corvazier Exp $
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

#include "std_afx.h"
#include <nel/misc/debug.h>

#define RYKOL_CHECK_PLUGIN_VERSION_BASE_PATH "\\\\server\\code\\tools\\"

BOOL CheckPluginVersion (const char* sServerPluginFileName)
{
#ifndef NL_DEBUG
	char sDrive[256];
	char sDir[256];
	char sFile[256];
	char sExt[256];
	char sModuleName[256];
	char sNetworkPath[256];
	sprintf (sNetworkPath, "%s%s", RYKOL_CHECK_PLUGIN_VERSION_BASE_PATH, sServerPluginFileName);
	_splitpath (sServerPluginFileName, sDrive, sDir, sFile, sExt);
	sprintf (sModuleName, "%s%s", sFile, sExt);

	// recherche le module de la dll...
	HMODULE hModule = GetModuleHandle(sModuleName);
	nlassert (hModule);
	char sModulePath[256];
	int res=GetModuleFileName(hModule, sModulePath, 256);
	nlassert(res);
	HANDLE h1=CreateFile( sNetworkPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h1==INVALID_HANDLE_VALUE)
	{
		// pas bonne version
		/*char sMsg[256];
		sprintf (sMsg, "Impossible de trouver le plugin sur la base: %s\nLe serveur est-il en service?", sNetworkPath);
		MessageBox (NULL, sMsg, "Version du plugin", MB_OK|MB_ICONEXCLAMATION);*/
		return TRUE;
	}
	HANDLE h2=CreateFile( sModulePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	nlassert(h2!=INVALID_HANDLE_VALUE);
	FILETIME Tmp;
	FILETIME lpLastWriteTime1;
	FILETIME lpLastWriteTime2;
	res=GetFileTime(h1, &Tmp, &Tmp, &lpLastWriteTime1);
	nlassert (res);
	res=GetFileTime(h2, &Tmp, &Tmp, &lpLastWriteTime2);
	nlassert (res);
	LONG nComp=CompareFileTime(&lpLastWriteTime1, &lpLastWriteTime2);
	CloseHandle(h1);
	CloseHandle(h2);
	if (nComp==+1)
	{
		// pas bonne version
		char sMsg[256];
		sprintf (sMsg, "Recupérer la nouvelle version du plugin %s", sNetworkPath);
		MessageBox (NULL, sMsg, "Version du plugin", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	if (nComp==-1)
	{
		// pas bonne version
		/*char sMsg[256];
		sprintf (sMsg, "Votre plugin est plus récent que celui sur la base: %s", sNetworkPath);
		MessageBox (NULL, sMsg, "Version du plugin", MB_OK|MB_ICONEXCLAMATION);*/
		return TRUE;
	}
#endif // NL_DEBUG
	return TRUE;
}