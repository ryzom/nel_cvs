/** \file texture_file.cpp
 * <File description>
 *
 * $Id: texture_file.cpp,v 1.9 2001/10/29 09:37:03 corvazier Exp $
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

#include "3d/texture_file.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
using namespace std;
using namespace NLMISC;


namespace NL3D
{

/*==================================================================*\
							CTEXTUREFILE
\*==================================================================*/

/*------------------------------------------------------------------*\
							doGenerate()
\*------------------------------------------------------------------*/
void CTextureFile::doGenerate()
{
	NLMISC::CIFile f;
	//nldebug(_FileName.c_str());
	try
	{
		string	file= CPath::lookup(_FileName);
		if(f.open(file))
			load(f);
		else throw EPathNotFound(_FileName);
	}
	catch(EPathNotFound &)
	{
		// Not found...
		makeDummy();
		nlwarning("Missing textureFile: %s", _FileName.c_str());
	}
	
}


// ***************************************************************************
void	CTextureFile::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 1:
		- AllowDegradation.
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(1);

	// serial the base part of ITexture.
	ITexture::serial(f);

	f.serial(_FileName);
	if(ver>=1)
		f.serial(_AllowDegradation);
	else if(f.isReading())
		_AllowDegradation= true;

	if(f.isReading())
		touch();
}


// ***************************************************************************
void	CTextureFile::setAllowDegradation(bool allow)
{
	_AllowDegradation= allow;
}


} // NL3D
