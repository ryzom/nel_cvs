/** \file driver.cpp
 * Generic driver.
 * Low level HW classes : ITexture, Cmaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * $Id: driver.cpp,v 1.7 2000/12/04 10:55:02 coutelas Exp $
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

#include <stdio.h>

using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
const uint32 IDriver::InterfaceVersion = 0x1;

// ***************************************************************************
IDriver::IDriver()
{
	_Material=NULL;
	_CurrentTexture[0]= NULL;
	_CurrentTexture[1]= NULL;
	_CurrentTexture[2]= NULL;
	_CurrentTexture[3]= NULL;
}

// ***************************************************************************
IDriver::~IDriver()
{ 
	std::list< CRefPtr<ITextureDrvInfos> >::iterator it = _pTexDrvInfos.begin();
	while( it!=_pTexDrvInfos.end() )
	{
		it->kill();
		it++;
	}
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

