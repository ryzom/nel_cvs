/** \file texture_cube.cpp
 * Implementation of a texture cube
 *
 * $Id: texture_cube.cpp,v 1.6 2002/02/28 12:59:52 besson Exp $
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

#include "3d/texture_cube.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
CTextureCube::CTextureCube()
{
	for( uint i = 0; i < 6; ++i )
		_Textures[i] = NULL;
}

// ***************************************************************************
void CTextureCube::setTexture(TFace f, ITexture *t)
{
	_Textures[f] = t;
}

// ***************************************************************************
string CTextureCube::getShareName() const
{
	string sTemp;

	for( uint i = 0; i < 6; ++i )
		if( _Textures[i] != NULL )
			sTemp += _Textures[i]->getShareName();
	return sTemp;
}

// ***************************************************************************
void CTextureCube::doGenerate()
{
	uint i;
	ITexture *pRefTex = NULL;

	for( i = 0; i < 6; ++i )
		if( _Textures[i] != NULL )
		{
			pRefTex = _Textures[i];
			break;
		}

	if( pRefTex == NULL )
		return; // There are NO texture

	pRefTex->generate();
	// The reference texture must be square and power of 2
	// If not power of 2 resize to the power of 2 just below the current size
	/*
	if( ( !isPowerOf2(pRefTex->getWidth()) ) || ( pRefTex->getWidth() != pRefTex->getHeight() ) )
	{
		uint32 nNewSize = pRefTex->getWidth();
		if( !isPowerOf2(nNewSize) )
		{
			nNewSize = raiseToNextPowerOf2(nNewSize);	// 5 -> 8 | 20 -> 32
			nNewSize = getPowerOf2(nNewSize);			// 8 -> 3 | 32 -> 5
			nNewSize -= 1;								// 3 -> 2 | 5 -> 4
			nNewSize = 1<<nNewSize;						// 2 -> 4 | 4 -> 16
		}

		pRefTex->resample( nNewSize, nNewSize );
	}
	*/

	// All textures must be like the reference texture
	for( i = 0; i < 6; ++i )
	{
		if( _Textures[i] != NULL )
			_Textures[i]->generate();
		else
			_Textures[i] = pRefTex;
		if( ( _Textures[i]->getWidth()  != pRefTex->getWidth()  ) || 
			( _Textures[i]->getHeight() != pRefTex->getHeight() ) )
		{
			_Textures[i]->resample( pRefTex->getWidth(), pRefTex->getHeight() );
		}
		// Let's apply the flips depending on the texture
		if( ((TFace)i) == positive_x )
			_Textures[i]->flipH();
		if( ((TFace)i) == negative_x )
			_Textures[i]->flipH();
		if( ((TFace)i) == positive_y )
			_Textures[i]->flipH();
		if( ((TFace)i) == negative_y )
			_Textures[i]->flipH();
		if( ((TFace)i) == positive_z )
			_Textures[i]->flipV();
		if( ((TFace)i) == negative_z )
			_Textures[i]->flipV();
	}
}

// ***************************************************************************
void	CTextureCube::release()
{
	uint i;
	for( i = 0; i < 6; ++i )
		if( _Textures[i] != NULL )
			_Textures[i]->release();
}

// ***************************************************************************
void	CTextureCube::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	// serial the base part of ITexture.
	ITexture::serial(f);

	for( uint i = 0; i < 6; ++i )
		f.serialPolyPtr( _Textures[i] );
	if( f.isReading() )
		touch();
}



// ***************************************************************************
void CTextureCube::selectTexture(uint index)
{
	for( uint i = 0; i < 6; ++i )
	{
		_Textures[i]->selectTexture(index);
	}
	touch();
}

} // NL3D
