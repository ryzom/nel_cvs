#include "stdafx.h"
#include <windows.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define NEL_CHECK_PLUGIN_VERSION_BASE_PATH "\\\\server\\code\\tools\\"

BOOL CheckPluginVersion (const char* sServerPluginFileName)
{

	char sDrive[256];
	char sDir[256];
	char sFile[256];
	char sExt[256];
	char sModuleName[256];
	char sNetworkPath[256];
	sprintf (sNetworkPath, "%s%s", NEL_CHECK_PLUGIN_VERSION_BASE_PATH, sServerPluginFileName);
	_splitpath (sServerPluginFileName, sDrive, sDir, sFile, sExt);
	sprintf (sModuleName, "%s%s", sFile, sExt);

	// recherche le module de la dll...
	HMODULE hModule = GetModuleHandle(sModuleName);
	assert (hModule);
	char sModulePath[256];
	int res=GetModuleFileName(hModule, sModulePath, 256);
	assert(res);
	HANDLE h1=CreateFile( sNetworkPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h1==INVALID_HANDLE_VALUE)
	{
		// pas bonne version
		char sMsg[256];
		sprintf (sMsg, "Impossible de trouver le plugin sur la base: %s\nLe serveur est-il en service?", sNetworkPath);
		MessageBox (NULL, sMsg, "Version du plugin", MB_OK|MB_ICONEXCLAMATION);
		return TRUE;
	}
	HANDLE h2=CreateFile( sModulePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(h2!=INVALID_HANDLE_VALUE);
	FILETIME Tmp;
	FILETIME lpLastWriteTime1;
	FILETIME lpLastWriteTime2;
	res=GetFileTime(h1, &Tmp, &Tmp, &lpLastWriteTime1);
	assert (res);
	res=GetFileTime(h2, &Tmp, &Tmp, &lpLastWriteTime2);
	assert (res);
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
	return TRUE;
}