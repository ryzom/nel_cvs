/** \file texture_multi_file.cpp
 * <File description>
 *
 * $Id: texture_multi_file.cpp,v 1.2 2002/02/28 12:59:52 besson Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "std3d.h"

#include "3d/texture_multi_file.h"
#include "3d/texture_file.h"


namespace NL3D 
{

///===========================================================	
CTextureMultiFile::CTextureMultiFile(uint numTexs /* = 0 */) : _FileNames(numTexs), _CurrSelectedTexture(0)
{	
}
	

///===========================================================	
void CTextureMultiFile::setNumTextures(uint numTexs)
{
	_FileNames.resize(numTexs);
	_CurrSelectedTexture = (uint) std::min((sint) _CurrSelectedTexture, std::min((sint) 0, (sint) (numTexs - 1)));
}


///===========================================================	
void CTextureMultiFile::setFileName(uint index, const char *fileName)
{	
	_FileNames[index] = fileName;
	if (index == _CurrSelectedTexture) touch();

}


///===========================================================	
void CTextureMultiFile::doGenerate()
{
	uint usedTexture = _CurrSelectedTexture >= _FileNames.size() ? 0 : _CurrSelectedTexture;
	
	if (_FileNames[usedTexture].empty())
	{
		if (usedTexture != 0 && !_FileNames[0].empty())
		{
			usedTexture = 0;
		}
		else
		{
			makeDummy();
			return;
		}
	}	
	CTextureFile::buildBitmapFromFile(*this, _FileNames[usedTexture]);	
}

///===========================================================	
void	CTextureMultiFile::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	// serial the base part of ITexture.
	ITexture::serial(f);

	f.serialCont(_FileNames);
	f.serial(_CurrSelectedTexture);

	if(f.isReading())
		touch();
}



///===========================================================	
std::string		CTextureMultiFile::getShareName() const
{ 
	if (_CurrSelectedTexture >= _FileNames.size())
	{
		return "CTextureMultiFile:Dummy"; // invalid...
	}
	else
	{
		return _FileNames[_CurrSelectedTexture];
	}
}

///===========================================================	
void CTextureMultiFile::selectTexture(uint index)
{
	if (index != _CurrSelectedTexture)
	{
		_CurrSelectedTexture = index;
		touch();
	}
}


} // NL3D
