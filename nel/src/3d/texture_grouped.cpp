/** \file texture_grouped.cpp
 * <File description>
 *
 * $Id: texture_grouped.cpp,v 1.5 2001/08/28 13:09:13 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "3d/texture_grouped.h"
#include "nel/misc/common.h"
#include <algorithm>

namespace NL3D {


/*
 * Constructor
 */
CTextureGrouped::CTextureGrouped() : _NbTex(0)
{
	setFilterMode(Linear, LinearMipMapOff);
}


// copy ctor
CTextureGrouped::CTextureGrouped(const CTextureGrouped &src) : ITexture(src)
{
	// copy the part associated with us ;
	duplicate(src) ;
}

void CTextureGrouped::duplicate(const CTextureGrouped &src)
{
	_NbTex = src._NbTex ;
	TTexList texCopy(src._Textures.begin(), src._Textures.end()) ;
	_Textures.swap(texCopy) ;
	TFourUVList	uvCopy(_TexUVs.begin(), _TexUVs.end()) ;
	_TexUVs.swap(uvCopy) ;
}

/// = operator
CTextureGrouped &CTextureGrouped::operator=(const CTextureGrouped &src)
{
	ITexture::operator=(src) ; // copy parent part
	duplicate(src) ;
	return *this ;
}


bool CTextureGrouped::areValid(CSmartPtr<ITexture> *textureTab, uint nbTex)
{
	

	CSmartPtr<ITexture> *pt = textureTab ;

	(*pt)->generate() ; // we need this to get the size ;
	uint32 xSize = (*pt)->getWidth(), ySize = (*pt)->getHeight() ;


	// we must have 2 power of 2
	nlassert(NLMISC::isPowerOf2(xSize)) ;
	nlassert(NLMISC::isPowerOf2(ySize)) ;

	CBitmap::TType pixelFormat = (*pt)->PixelFormat ;

	for(uint k = 0 ; k < nbTex ; ++k, ++pt)
	{
		(*pt)->generate() ;
		if ((*pt)->getWidth() != xSize) return false ;
		if ((*pt)->getHeight() != ySize) return false ;	
		if ((*pt)->PixelFormat != pixelFormat) return false ;
	}
	return true ;
}


void CTextureGrouped::setTextures(CSmartPtr<ITexture> *textureTab, uint nbTex)
{
	nlassert(nbTex > 0) ;
	nlassert(areValid(textureTab, nbTex)) ;

	CSmartPtr<ITexture> *pt = textureTab ;
	

	(*pt)->generate() ; // we need this to get the size ;

	const uint32 width = (*pt)->getWidth() ;
	const uint32 height = (*pt)->getHeight() ;

	const uint32 totalHeight = height * nbTex ;

	const uint32 realHeight = NLMISC::raiseToNextPowerOf2(totalHeight) ;
	

	// now we got a height that is a power of 2

	resize(width, realHeight, (*pt)->PixelFormat ) ;


	_TexUVs.clear() ;
	_Textures.clear() ;

	

	const float deltaV = (float(totalHeight) / float(realHeight)) * (1.0f / nbTex) ;

	_DeltaUV = CUV(1,  deltaV) ;

	CUV currentUV(0, 0) ;

	for(uint k = 0 ; k < nbTex ; ++k, ++pt)
	{	
		TFourUV uvs ;
		_Textures.push_back(*pt) ;

		uvs.uv0 = currentUV ;
		uvs.uv1 = currentUV + CUV(1, 0) ;
		uvs.uv2 = currentUV + _DeltaUV ;
		uvs.uv3 = currentUV + CUV(0, deltaV) ;

		currentUV.V += deltaV ;
		_TexUVs.push_back(uvs) ;
	}

	_NbTex = nbTex ;

	touch() ; // the texture need regeneration
}


/** 
	 * Generate the texture.	 
	 */	
void CTextureGrouped::doGenerate()
{

	if (!_NbTex)
	{
		makeDummy() ;
	}
	else
	{
		TTexList::const_iterator  pt = _Textures.begin(), ptEnd = _Textures.end() ;			
		const uint32 height = (*pt)->getHeight() ;

		sint32 currY =  0 ;
		for(; pt != ptEnd ; ++pt)
		{			
			/// make sure that datas are available
			(*pt)->generate() ;
			this->blit(*pt, 0, currY) ;
			currY += height ;			
		}
	}	
}


	
	


void CTextureGrouped::getTextures(CSmartPtr<ITexture> *textureTab) const
{
	CSmartPtr<ITexture> *dest = textureTab ;

	for(TTexList::const_iterator it = _Textures.begin() ; it != _Textures.end() ; ++it, ++dest)
	{
		*dest = *it ;
	}
}


bool	CTextureGrouped::supportSharing() const
{
	// all textures in this group must support sharing for this one to support it
	for(TTexList::const_iterator it = _Textures.begin() ; it != _Textures.end() ; ++it)
	{
		if (!(*it)->supportSharing()) return false ;
	}

	return true ;
}


std::string		CTextureGrouped::getShareName() const
{
	nlassert(supportSharing()) ;

	std::string shareName("groupedTex:") ;
	for(TTexList::const_iterator it = _Textures.begin() ; it != _Textures.end() ; ++it)
	{
		shareName += (*it)->getShareName() + std::string("|") ;
	}
	return shareName ;
}



void CTextureGrouped::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;
	if (f.isReading())
	{
		TTexList texList ;
		uint32 nbTex ;

		f.serial(nbTex) ;

		ITexture *ptTex = NULL ;

		for (uint k = 0 ; k < nbTex ; ++k)
		{
			f.serialPolyPtr(ptTex) ;
			texList.push_back(ptTex) ;
		}		
		
		setTextures(&texList[0], nbTex) ;

		touch() ;
	}
	else
	{
		f.serial(_NbTex) ;
		ITexture *ptTex ;
		for (TTexList::iterator it = _Textures.begin() ; it != _Textures.end() ; ++it)
		{
			ptTex = *it ;
			f.serialPolyPtr(ptTex) ;
		}		
	}
}


	


} // NL3D
