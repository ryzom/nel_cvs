/** \file driver.cpp
 * Generic driver.
 * Low level HW classes : ITexture, Cmaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * $Id: driver.cpp,v 1.14 2001/01/03 09:14:57 lecroart Exp $
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


#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"
#include "nel/3d/shader.h"
#include "nel/3d/vertex_buffer.h"

#include <stdio.h>

using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
const uint32 IDriver::InterfaceVersion = 0x5;

// ***************************************************************************
IDriver::IDriver()
{
}

// ***************************************************************************
IDriver::~IDriver()
{
	// Must clean up everything before closing driver.
	// Must doing this in release(), so assert here if not done...

	nlassert(_TexDrvInfos.size()==0);
	nlassert(_TexDrvShares.size()==0);
	nlassert(_Shaders.size()==0);
	nlassert(_VBDrvInfos.size()==0);
}


// ***************************************************************************
bool		IDriver::release(void)
{
	// Called by derived classes.

	// DO THIS FIRST => to auto kill real textures (by smartptr).
	// First, Because must not kill a pointer owned by a CSmartPtr.
	// Release Textures drv.
	ItTexDrvSharePtrList		ittex = _TexDrvShares.begin();
	while( ittex!=_TexDrvShares.end() )
	{
		ittex->kill();
		ittex++;
	}
	_TexDrvShares.clear();


	// Release refptr of TextureDrvInfos. Should be all null (because of precedent pass).
	ItTexDrvInfoPtrMap		ittexmap = _TexDrvInfos.begin();
	while( ittexmap!=_TexDrvInfos.end() )
	{
		// Do not need to kill the pointer must be NULL.
		nlassert((*ittexmap).second==NULL);
		ittexmap++;
	}
	_TexDrvInfos.clear();


	// Release Shader drv.
	ItShaderPtrList		itshd = _Shaders.begin();
	while( itshd!=_Shaders.end() )
	{
		itshd->kill();
		itshd++;
	}
	_Shaders.clear();

	// Release VBs drv.
	ItVBDrvInfoPtrList		itvb = _VBDrvInfos.begin();
	while( itvb!=_VBDrvInfos.end() )
	{
		itvb->kill();
		itvb++;
	}
	_VBDrvInfos.clear();

	return true;
}


// ***************************************************************************
GfxMode::GfxMode(uint16 w, uint16 h, uint8 d, bool windowed)
{
	Windowed= windowed;
	Width= w;
	Height= h;
	Depth= d;
}

// ***************************************************************************
IDriver::TMessageBoxId IDriver::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, IDriver::TMessageBoxIcon icon)
{
	static const char* icons[iconCount]=
	{
		"",
		"WAIT:\n",
		"QUESTION:\n",
		"HEY!\n",
		"",
		"WARNING!\n",
		"ERROR!\n",
		"INFORMATION:\n",
		"STOP:\n"
	};
	static const char* messages[typeCount]=
	{
		"Press any key...",
		"(O)k or (C)ancel ?",
		"(Y)es or (N)o ?",
		"(A)bort (R)etry (I)gnore ?",
		"(Y)es (N)o (C)ancel ?",
		"(R)etry (C)ancel ?"
	};
	printf ("%s%s\n%s", icons[icon], title, message);
	while (1)
	{
		printf ("\n%s", messages[type]);
		int c=getchar();
		if (type==okType)
			return okId;
		switch (c)
		{
		case 'O':
		case 'o':
			if ((type==okType)||(type==okCancelType))
				return okId;
			break;
		case 'C':
		case 'c':
			if ((type==yesNoCancelType)||(type==okCancelType)||(type==retryCancelType))
				return cancelId;
			break;
		case 'Y':
		case 'y':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return yesId;
			break;
		case 'N':
		case 'n':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return noId;
			break;
		case 'A':
		case 'a':
			if (type==abortRetryIgnoreType)
				return abortId;
			break;
		case 'R':
		case 'r':
			if (type==abortRetryIgnoreType)
				return retryId;
			break;
		case 'I':
		case 'i':
			if (type==abortRetryIgnoreType)
				return ignoreId;
			break;
		}
	}
	nlassert (0);		// no!
	return okId;
}

}

