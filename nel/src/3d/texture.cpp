/** \file texture.cpp
 * ITexture & CTextureFile
 *
 * $Id: texture.cpp,v 1.20 2002/08/21 09:39:54 lecroart Exp $
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

#include "std3d.h"

#include "3d/texture.h"
#include "3d/driver.h"
#include <vector>


namespace NL3D 
{


/*==================================================================*\
							ITEXTURE
\*==================================================================*/

ITexture::ITexture()
{
	_Touched= false;
	_GoodGenerate= false;
	_Releasable= true;
	_WrapS= _WrapT= Repeat;
	_UploadFormat= Auto;
	_MagFilter= Linear;
	_MinFilter= LinearMipMapLinear;
}


ITexture::~ITexture()
{
	// Must kill the drv mirror of this texture.
	TextureDrvShare.kill();
}


ITexture &ITexture::operator=(const ITexture &tex)
{
	// The operator= do not copy drv info
	// set touched=true. _Releasable is copied.
	_UploadFormat= tex._UploadFormat;
	_Releasable= tex._Releasable;
	_WrapS= tex._WrapS;
	_WrapT= tex._WrapT;
	_MagFilter= tex._MagFilter;
	_MinFilter= tex._MinFilter; 
	touch();
	return *this;
}


void		ITexture::setUploadFormat(TUploadFormat pf)
{
	if(pf!=_UploadFormat)
	{
		_UploadFormat= pf;
		// All the texture may be reloaded...
		touch();
	}
}


void		ITexture::setFilterMode(TMagFilter magf, TMinFilter minf)
{
	_MagFilter= magf;
	// If the MipMap mode has siwtched Off/On, then must recompute...
	bool	precOff= mipMapOff();
	_MinFilter= minf;
	bool	nowOff= mipMapOff();

	if(precOff!=nowOff)
	{
		// Must recompute mipmaps!!
		touch();
	}
}



// ***************************************************************************
CTextureDrvShare::~CTextureDrvShare()
{
	_Driver->removeTextureDrvSharePtr(_DriverIterator);
}
// ***************************************************************************
ITextureDrvInfos::~ITextureDrvInfos()
{
	// NB: _Driver may be NULL because texture may not be stored in the share texture map.
	// so there is no need to remove it from this map!!
	if(_Driver)
		_Driver->removeTextureDrvInfoPtr(_DriverIterator);
}


// ***************************************************************************
void	ITexture::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	(void)f.serialVersion(0);

	f.serialEnum(_UploadFormat);
	f.serialEnum(_WrapS);
	f.serialEnum(_WrapT);
	f.serialEnum(_MinFilter);
	f.serialEnum(_MagFilter);
}



} // NL3D
