/** \file driver_opengl_texture.cpp
 * OpenGL driver implementation : setupTexture
 *
 * $Id: driver_opengl_texture.cpp,v 1.13 2001/01/08 17:58:30 corvazier Exp $
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

#include <string>

#include "driver_opengl.h"
#include "nel/misc/rect.h"
using	namespace NLMISC;
using	namespace std;

namespace NL3D
{


// ***************************************************************************
// Get the glText mirror of an existing setuped texture.
static	inline CTextureDrvInfosGL*	getTextureGl(ITexture& tex)
{
	CTextureDrvInfosGL*	gltex;
	gltex= (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
	return gltex;
}


// ***************************************************************************
// Translation of TexFmt mode.
static	inline GLint	getGlTextureFormat(ITexture& tex)
{
	ITexture::TUploadFormat		texfmt= tex.getUploadFormat();

	// If auto, retrieve the pixel format of the bitmap.
	if(texfmt== ITexture::Auto)
	{
		switch(tex.getPixelFormat())
		{
			case CBitmap::RGBA: texfmt= ITexture::RGBA8888; break;
			case CBitmap::DXTC1: texfmt= ITexture::DXTC1; break;
			case CBitmap::DXTC1Alpha: texfmt= ITexture::DXTC1Alpha; break;
			case CBitmap::DXTC3: texfmt= ITexture::DXTC3; break;
			case CBitmap::DXTC5: texfmt= ITexture::DXTC5; break;
			case CBitmap::Luminance: texfmt= ITexture::Luminance; break;
			case CBitmap::Alpha: texfmt= ITexture::Alpha; break;
			case CBitmap::AlphaLuminance: texfmt= ITexture::AlphaLuminance; break;
			default: texfmt= ITexture::RGBA8888; break;
		}
	}

	// Get gl tex format.
	switch(texfmt)
	{
		case ITexture::RGBA8888: return GL_RGBA8;
		case ITexture::RGBA4444: return GL_RGBA4;
		case ITexture::RGBA5551: return GL_RGB5_A1;
		case ITexture::RGB888: return GL_RGB8;
		case ITexture::RGB565: return GL_RGB5;
		// TODO_DXTC!!!!
		case ITexture::DXTC1: return  GL_RGB8;
		case ITexture::DXTC1Alpha: return GL_RGBA8;
		case ITexture::DXTC3: return GL_RGBA8;
		case ITexture::DXTC5: return GL_RGBA8;
		// End tododxtc.
		case ITexture::Luminance: return GL_LUMINANCE8;
		case ITexture::Alpha: return GL_ALPHA8;
		case ITexture::AlphaLuminance: return GL_LUMINANCE8_ALPHA8;
		default: return GL_RGBA8;
	}
}


// ***************************************************************************
// Translation of Wrap mode.
static inline GLenum	translateWrapToGl(ITexture::TWrapMode mode)
{
	if(mode== ITexture::Repeat)
		return GL_REPEAT;
	else
		return GL_CLAMP;
}


// ***************************************************************************
static inline GLenum	translateMagFilterToGl(ITexture::TMagFilter mode)
{
	switch(mode)
	{
		case ITexture::Linear: return GL_LINEAR;
		case ITexture::Nearest: return GL_NEAREST;
	}

	nlstop;
	return GL_LINEAR;
}


// ***************************************************************************
static inline GLenum	translateMinFilterToGl(ITexture::TMinFilter mode)
{
	switch(mode)
	{
		case ITexture::NearestMipMapOff: return GL_NEAREST;
		case ITexture::NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case ITexture::NearestMipMapLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case ITexture::LinearMipMapOff: return GL_LINEAR;
		case ITexture::LinearMipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case ITexture::LinearMipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
	}

	nlstop;
	return GL_LINEAR;
}


// ***************************************************************************
bool CDriverGL::setupTexture(ITexture& tex)
{
	// 0. Create/Retrieve the driver texture.
	//=======================================
	if ( !tex.TextureDrvShare )
	{
		// Create the shared texture.
		tex.TextureDrvShare= new CTextureDrvShare;

		// Insert into Driver list.
		_TexDrvShares.push_back(tex.TextureDrvShare);

		// Must (re)-create the texture.
		tex.touch();
	}


	// 1. If modified, may (re)load texture part or all of the texture.
	//=================================================================
	if ( tex.touched() )
	{
		bool	mustLoadAll= false;
		bool	mustLoadPart= false;

		// A. Share mgt.
		//==============
		if(tex.supportSharing())
		{
			// Try to get the shared texture.

			// Create the shared Name.
			std::string	name= tex.getShareName();
			// append format Id of the texture.
			static char	fmt[256];
			sprintf(fmt, "@Fmt:%d", (uint32)tex.getUploadFormat());
			name+= fmt;
			// append mipmap info
			if(tex.mipMapOn())
				name+= "@MMp:On";
			else
				name+= "@MMp:Off";


			// insert or get the texture.
			ITextureDrvInfos	*gltext= _TexDrvInfos[name];
			/* There is a trick here: this test both if:
				- the textureptr was not in the map. (because CRefPtr() is inited to NULL)
				- the texture was deleted (by smartptr) by TextureDrvShare. Hence the entry is not deleted even if
					the ptr is.
			*/
			if(gltext==NULL)
			{
				_TexDrvInfos[name]= tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL;
				// need to load ALL this texture.
				mustLoadAll= true;
			}
			else
			{
				tex.TextureDrvShare->DrvTexture= gltext;
				// Do not need to reload this texture, even if the format/mipmap has changed, since we found this 
				// couple in the map.
				mustLoadAll= false;
			}

			// Do not test if part of texture may need to be computed, because Rect invalidation is incompatible 
			// with texture sharing.
		}
		else
		{
			// If texture not already created.
			if(!tex.TextureDrvShare->DrvTexture)
			{
				// Must create it.
				// Create auto a GL id (in constructor).
				// Do not insert into the map. This un-shared texture will be deleted at deletion of the texture.
				tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL;
				mustLoadAll= true;
			}
			else if(tex.isAllInvalidated())
				mustLoadAll= true;
			else if(tex.touched())
				mustLoadPart= true;
		}

		// B. Setup texture.
		//==================
		if(mustLoadAll || mustLoadPart)
		{
			CTextureDrvInfosGL*	gltext;
			gltext= getTextureGl(tex);
			glBindTexture(GL_TEXTURE_2D, gltext->ID);
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);

			// a. Load All the texture case.
			//==============================
			if (mustLoadAll)
			{
				// Regenerate all the texture.
				tex.generate();

				if(tex.getSize()>0)
				{
					// Get the correct texture format from texture...
					GLint	glfmt= getGlTextureFormat(tex);

					// TODO_DXTC
					sint	nMipMaps;
					tex.convertToType(CBitmap::RGBA);
					if(tex.mipMapOn())
					{
						tex.buildMipMaps();
						nMipMaps= tex.getMipMapCount();
					}
					else
						nMipMaps= 1;

					// Fill mipmaps.
					for(sint i=0;i<nMipMaps;i++)
					{
						void	*ptr= &(*tex.getPixels(i).begin());
						glTexImage2D(GL_TEXTURE_2D,i,glfmt,tex.getWidth(i),tex.getHeight(i),0,GL_RGBA,GL_UNSIGNED_BYTE, ptr );
					}
				}
				//printf("%d,%d,%d\n", tex.getMipMapCount(), tex.getWidth(0), tex.getHeight(0));
			}
			// b. Load part of the texture case.
			//==================================
			else if (mustLoadPart)
			{
				// Regenerate wanted part of the texture.
				tex.generate();

				if(tex.getSize()>0)
				{
					// Get the correct texture format from texture...
					//===============================================
					GLint	glfmt= getGlTextureFormat(tex);

					// TODO_DXTC
					sint	nMipMaps;
					tex.convertToType(CBitmap::RGBA);
					if(tex.mipMapOn())
					{
						bool	hadMipMap= tex.getMipMapCount()>1;
						tex.buildMipMaps();
						nMipMaps= tex.getMipMapCount();
						// If the texture had no mipmap before, release them.
						if(!hadMipMap)
						{
							tex.releaseMipMaps();
						}
					}
					else
						nMipMaps= 1;

					// For all rect, update the texture/mipmap.
					//===============================================
					list<NLMISC::CRect>::iterator	itRect;
					for(itRect=tex._ListInvalidRect.begin(); itRect!=tex._ListInvalidRect.end(); itRect++)
					{
						CRect	&rect= *itRect;
						sint	x0= rect.X;
						sint	y0= rect.Y;
						sint	x1= rect.X+rect.Width;
						sint	y1= rect.Y+rect.Height;

						// Fill mipmaps.
						for(sint i=0;i<nMipMaps;i++)
						{
							void	*ptr= &(*tex.getPixels(i).begin());
							sint	w= tex.getWidth(i);
							sint	h= tex.getHeight(i);
							clamp(x0, 0, w);
							clamp(y0, 0, h);
							clamp(x1, x0, w);
							clamp(y1, y0, h);

							glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
							glPixelStorei(GL_UNPACK_SKIP_ROWS, y0);
							glPixelStorei(GL_UNPACK_SKIP_PIXELS, x0);
							glTexSubImage2D(GL_TEXTURE_2D,i, x0, y0, x1-x0, y1-y0, GL_RGBA,GL_UNSIGNED_BYTE, ptr );

							// Next mipmap!!
							// floor .
							x0= x0/2;
							y0= y0/2;
							// ceil.
							x1= (x1+1)/2;
							y1= (y1+1)/2;
						}
					}

					// Reset the transfer mode...
					glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
					glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
					glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
				}
			}


			// Release, if wanted.
			if(tex.getReleasable())
				tex.release();

			// Basic parameters.
			//==================
			gltext->WrapS= tex.getWrapS();
			gltext->WrapT= tex.getWrapT();
			gltext->MagFilter= tex.getMagFilter();
			gltext->MinFilter= tex.getMinFilter();
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, translateWrapToGl(gltext->WrapS));
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, translateWrapToGl(gltext->WrapT));
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext->MagFilter));
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext->MinFilter));
		}


		// The texture is correctly setuped.
		tex.clearTouched();
	}
	return true;
}


// ***************************************************************************
bool CDriverGL::activateTexture(uint stage, ITexture *tex)
{
	/// \todo: yoyo: ARB_multitexture.

	if (this->_CurrentTexture[stage]!=tex)
	{
		if(tex)
		{
			glEnable(GL_TEXTURE_2D);
			CTextureDrvInfosGL*	gltext;
			gltext= getTextureGl(*tex);
			glBindTexture(GL_TEXTURE_2D, getTextureGl(*tex)->ID);
			if(gltext->WrapS!= tex->getWrapS())
			{
				gltext->WrapS= tex->getWrapS();
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, translateWrapToGl(gltext->WrapS));
			}
			if(gltext->WrapT!= tex->getWrapT())
			{
				gltext->WrapT= tex->getWrapT();
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, translateWrapToGl(gltext->WrapT));
			}
			if(gltext->MagFilter!= tex->getMagFilter())
			{
				gltext->MagFilter= tex->getMagFilter();
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext->MagFilter));
			}
			if(gltext->MinFilter!= tex->getMinFilter())
			{
				gltext->MinFilter= tex->getMinFilter();
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext->MinFilter));
			}
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
		}

		this->_CurrentTexture[stage]= tex;
	}
	return true;
}


} // NL3D
