/** \file driver_opengl_texture.cpp
 * OpenGL driver implementation : setupTexture
 *
 * \todo yoyo: BUG with texture parameters. If parameters change are made between two renders, but the texture has not
 * changed (eg: only one texture in the whole world), those parameters are not bound!!! 
 * OPTIM: like the TexEnvMode style, a PackedParameter format should be done, to limit tests...
 *
 * $Id: driver_opengl_texture.cpp,v 1.23 2001/04/23 09:14:27 besson Exp $
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
CTextureDrvInfosGL::CTextureDrvInfosGL()
{
	// The id is auto created here.
	glGenTextures(1,&ID);
	Compressed= false;
}
// ***************************************************************************
CTextureDrvInfosGL::~CTextureDrvInfosGL()
{
	// The id is auto deleted here.
	glDeleteTextures(1,&ID);
}



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
GLint	CDriverGL::getGlTextureFormat(ITexture& tex, bool &compressed)
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


	// Get gl tex format, try S3TC compressed ones.
	if(_Extensions.EXTTextureCompressionS3TC)
	{
		compressed= true;
		// Try Compressed ones.
		switch(texfmt)
		{
			case ITexture::DXTC1:		return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			case ITexture::DXTC1Alpha:	return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			case ITexture::DXTC3:		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			case ITexture::DXTC5:		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		}
	}


	// Get standard gl tex format.
	compressed= false;
	switch(texfmt)
	{
		case ITexture::RGBA8888: return GL_RGBA8;
		case ITexture::RGBA4444: return GL_RGBA4;
		case ITexture::RGBA5551: return GL_RGB5_A1;
		case ITexture::RGB888: return GL_RGB8;
		case ITexture::RGB565: return GL_RGB5;
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
static inline bool		sameDXTCFormat(ITexture &tex, GLint glfmt)
{
	if(glfmt==GL_COMPRESSED_RGB_S3TC_DXT1_EXT && tex.PixelFormat==CBitmap::DXTC1)
		return true;
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT1_EXT && tex.PixelFormat==CBitmap::DXTC1Alpha)
		return true;
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT3_EXT && tex.PixelFormat==CBitmap::DXTC3)
		return true;
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT5_EXT && tex.PixelFormat==CBitmap::DXTC5)
		return true;

	return false;
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
			smprintf(fmt, 256, "@Fmt:%d", (uint32)tex.getUploadFormat());
			name+= fmt;
			// append mipmap info
			if(tex.mipMapOn())
				name+= "@MMp:On";
			else
				name+= "@MMp:Off";


			// insert or get the texture.
			ITextureDrvInfos	*gltext;
			{
				CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
				TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
				gltext = rTexDrvInfos[name];
		
				/* There is a trick here: this test both if:
					- the textureptr was not in the map. (because CRefPtr() is inited to NULL)
					- the texture was deleted (by smartptr) by TextureDrvShare. Hence the entry is not deleted even if
						the ptr is.
				*/
				if(gltext==NULL)
				{
					rTexDrvInfos[name]= tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL;
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

			// system of "backup the previous binded texture" seems to not work with some drivers....
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);


			// Bind this texture, for reload...
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
					GLint	glfmt= getGlTextureFormat(tex, gltext->Compressed);

					// DXTC: if same format, and same mipmapOn/Off, use glTexCompressedImage*.
					// We cannot build the mipmaps if they are not here.
					if(_Extensions.EXTTextureCompressionS3TC && sameDXTCFormat(tex, glfmt) &&
						(tex.mipMapOff() || tex.getMipMapCount()>1) )
					{
						sint	nMipMaps;
						if(tex.mipMapOn())
							nMipMaps= tex.getMipMapCount();
						else
							nMipMaps= 1;

						// Fill mipmaps.
						for(sint i=0;i<nMipMaps;i++)
						{
							void	*ptr= &(*tex.getPixels(i).begin());
							sint	size= tex.getPixels(i).size();
							glCompressedTexImage2DARB(GL_TEXTURE_2D, i, glfmt, tex.getWidth(i),tex.getHeight(i), 0, 
								size, ptr );
						}
					}
					else
					{
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
				}
				//printf("%d,%d,%d\n", tex.getMipMapCount(), tex.getWidth(0), tex.getHeight(0));
			}
			// b. Load part of the texture case.
			//==================================
			// TODO_DXTC
			// Replace parts of a compressed image. Maybe don't work with the actual system of invalidateRect()...
			else if (mustLoadPart && !gltext->Compressed)
			{
				// Regenerate wanted part of the texture.
				tex.generate();

				if(tex.getSize()>0)
				{
					// Get the correct texture format from texture...
					//===============================================
					bool	dummy;
					GLint	glfmt= getGlTextureFormat(tex, dummy);

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


			// reset the stage 0
			glDisable(GL_TEXTURE_2D);
			_CurrentTexture[0]= NULL;
		}


		// The texture is correctly setuped.
		tex.clearTouched();
	}
	return true;
}


// ***************************************************************************
bool CDriverGL::activateTexture(uint stage, ITexture *tex)
{
	if (this->_CurrentTexture[stage]!=tex)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+stage);
		if(tex)
		{
			// Activate texturing...
			//======================
			glEnable(GL_TEXTURE_2D);
			CTextureDrvInfosGL*	gltext;
			gltext= getTextureGl(*tex);
			glBindTexture(GL_TEXTURE_2D, getTextureGl(*tex)->ID);

			// Change parameters of texture, if necessary.
			//============================================
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

// ***************************************************************************
void		CDriverGL::activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env)
{
	// This maps the CMaterial::TTexOperator
	static	const	GLenum	operatorLUT[8]= { GL_REPLACE, GL_MODULATE, GL_ADD, GL_ADD_SIGNED_EXT, 
		GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT };

	// This maps the CMaterial::TTexSource
	static	const	GLenum	sourceLUT[4]= { GL_TEXTURE, GL_PREVIOUS_EXT, GL_PRIMARY_COLOR_EXT, GL_CONSTANT_EXT };

	// This maps the CMaterial::TTexOperand
	static	const	GLenum	operandLUT[4]= { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };

	// This maps the CMaterial::TTexOperator, used for openGL Arg2 setup.
	static	const	GLenum	interpolateSrcLUT[8]= { GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, 
		GL_TEXTURE, GL_PREVIOUS_EXT, GL_PRIMARY_COLOR_EXT, GL_CONSTANT_EXT };



	// cache mgt.
	_CurrentTexEnv[stage].EnvPacked= env.EnvPacked;

	// Setup the gl env mode.
	glActiveTextureARB(GL_TEXTURE0_ARB+stage);
	// "Normal drivers", setup EnvCombine.
	if(_Extensions.EXTTextureEnvCombine)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);


		// RGB.
		//=====
		// Operator.
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, operatorLUT[env.Env.OpRGB] );
		// Arg0.
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, sourceLUT[env.Env.SrcArg0RGB] );
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, operandLUT[env.Env.OpArg0RGB]);
		// Arg1.
		if(env.Env.OpRGB > CMaterial::Replace)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, sourceLUT[env.Env.SrcArg1RGB] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, operandLUT[env.Env.OpArg1RGB]);
			// Arg2.
			if(env.Env.OpRGB >= CMaterial::InterpolateTexture )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, interpolateSrcLUT[env.Env.OpRGB] );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
			}
		}


		// Alpha.
		//=====
		// Operator.
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, operatorLUT[env.Env.OpAlpha] );
		// Arg0.
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, sourceLUT[env.Env.SrcArg0Alpha] );
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, operandLUT[env.Env.OpArg0Alpha]);
		// Arg1.
		if(env.Env.OpAlpha > CMaterial::Replace)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, sourceLUT[env.Env.SrcArg1Alpha] );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, operandLUT[env.Env.OpArg1Alpha]);
			// Arg2.
			if(env.Env.OpAlpha >= CMaterial::InterpolateTexture )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, interpolateSrcLUT[env.Env.OpAlpha] );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			}
		}
	}
	// Very Bad drivers.
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

}

// ***************************************************************************
void		CDriverGL::activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env)
{
	static	const float	OO255= 1.0f/255;
	const CRGBA		&col= env.ConstantColor;

	// cache mgt.
	_CurrentTexEnv[stage].ConstantColor= col;

	// Setup the gl cte color.
	glActiveTextureARB(GL_TEXTURE0_ARB+stage);
	GLfloat		glcol[4];
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, glcol);
}


} // NL3D
