/** \file nel_3dsmax_shared.cpp
 * Defines the entry point for the DLL application.
 *
 * $Id: nel_3dsmax_shared.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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

#include "stdafx.h"
#include "..\nel_patch_lib\rpo.h"
#include "nel_3dsmax_shared.h"
#include "..\nel_export\checkversion.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void init ()
{
	// Check version
	static bool bPassed=false;
	if (!bPassed)
	{
		bPassed=true;
		CheckPluginVersion ("plugins max\\nel_3dsmax_shared.dll");
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// The static allocator
static CPatchAllocator Allocator;

NEL_3DSMAX_SHARED_API CPatchAllocator& GetAllocator ()
{
	// Init fonction
	init ();

	return Allocator;
}

