/** \file driver_opengl_texture.cpp
 * OpenGL driver implementation : setupTexture
 *
 * \todo yoyo: BUG with texture parameters. If parameters change are made between two renders, but the texture has not
 * changed (eg: only one texture in the whole world), those parameters are not bound!!! 
 * OPTIM: like the TexEnvMode style, a PackedParameter format should be done, to limit tests...
 *
 * $Id: driver_opengl_texture.cpp,v 1.66 2004/02/20 14:43:49 vizerie Exp $
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

#include "stdopengl.h"

#include "3d/texture_cube.h"
#include "nel/misc/rect.h"
#include "nel/misc/file.h" // temp

// Define this to force nearest filter (debug)
// #define NEL_FORCE_NEAREST

using	namespace NLMISC;
using	namespace std;


//#define NEL_DUMP_UPLOAD_TIME


#ifdef NEL_DUMP_UPLOAD_TIME
	#define NEL_MEASURE_UPLOAD_TIME_START NLMISC::TTicks startTick = CTime::getPerformanceTime();
	#define NEL_MEASURE_UPLOAD_TIME_END NLMISC::TTicks endTick = CTime::getPerformanceTime(); \
										nlinfo("upload time = %.2f ms", (float) (1000 * (CTime::ticksToSecond(endTick) - CTime::ticksToSecond(startTick))));
#else
	#define NEL_MEASURE_UPLOAD_TIME_START
	#define NEL_MEASURE_UPLOAD_TIME_END
#endif

namespace NL3D
{


// ***************************************************************************
CTextureDrvInfosGL::CTextureDrvInfosGL(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverGL *drvGl) : ITextureDrvInfos(drv, it)
{
	// The id is auto created here.
	glGenTextures(1,&ID);
	
	Compressed= false;
	MipMap= false;
	TextureMemory= 0;

	// Nb: at Driver dtor, all tex infos are deleted, so _Driver is always valid.
	_Driver= drvGl;
}
// ***************************************************************************
CTextureDrvInfosGL::~CTextureDrvInfosGL()
{
	// The id is auto deleted here.
	glDeleteTextures(1,&ID);

	// release profiling texture mem.
	_Driver->_AllocatedTextureMemory-= TextureMemory;

	// release in TextureUsed.
	_Driver->_TextureUsed.erase (this);
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
			case CBitmap::RGBA: 
				if(_ForceDXTCCompression && tex.allowDegradation() )
					texfmt= ITexture::DXTC5;
				else
					texfmt= ITexture::RGBA8888;
				break;
			case CBitmap::DXTC1: texfmt= ITexture::DXTC1; break;
			case CBitmap::DXTC1Alpha: texfmt= ITexture::DXTC1Alpha; break;
			case CBitmap::DXTC3: texfmt= ITexture::DXTC3; break;
			case CBitmap::DXTC5: texfmt= ITexture::DXTC5; break;
			case CBitmap::Luminance: texfmt= ITexture::Luminance; break;
			case CBitmap::Alpha: texfmt= ITexture::Alpha; break;
			case CBitmap::AlphaLuminance: texfmt= ITexture::AlphaLuminance; break;
			case CBitmap::DsDt: texfmt= ITexture::DsDt; break;
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
			default: break;
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
		case ITexture::DsDt: 
			if (_Extensions.NVTextureShader) return GL_DSDT_NV;
			else if (_Extensions.ATIEnvMapBumpMap) return GL_DU8DV8_ATI;
			else
			{			
				nlassert(0);
				return 0;
			}
		break;
		default: return GL_RGBA8;
	}
}


// ***************************************************************************
static GLint	getGlSrcTextureFormat(ITexture &tex, GLint glfmt)
{
	// Is destination format is alpha or lumiance ?
	if ((glfmt==GL_ALPHA8)||(glfmt==GL_LUMINANCE8_ALPHA8)||(glfmt==GL_LUMINANCE8))
	{
		switch(tex.getPixelFormat())
		{
		case CBitmap::Alpha:	return GL_ALPHA;
		case CBitmap::AlphaLuminance:	return GL_LUMINANCE_ALPHA;
		case CBitmap::Luminance:	return GL_LUMINANCE;
		default: break;
		}
	}

	if (glfmt == GL_DSDT_NV)
	{
		return GL_DSDT_NV;
	}

	if (glfmt == GL_DU8DV8_ATI)
	{
		return GL_DUDV_ATI;
	}

	// Else, not a Src format for upload, or RGBA.
	return GL_RGBA;
}

// ***************************************************************************
static GLenum getGlSrcTextureComponentType(GLint texSrcFormat)
{
	switch (texSrcFormat)
	{
		case GL_DSDT_NV:
		case GL_DU8DV8_ATI:
			return GL_BYTE; // these are signed format			
		break;		
		default:
			return GL_UNSIGNED_BYTE;
		break;

	}
}

// ***************************************************************************
uint				CDriverGL::computeMipMapMemoryUsage(uint w, uint h, GLint glfmt) const
{
	switch(glfmt)
	{
	case GL_RGBA8:		return w*h* 4;
	// Well this is ugly, but simple :). GeForce 888 is stored as 32 bits.
	case GL_RGB8:		return w*h* 4;
	case GL_RGBA4:		return w*h* 2;
	case GL_RGB5_A1:	return w*h* 2;
	case GL_RGB5:		return w*h* 2;
	case GL_LUMINANCE8:	return w*h* 1;
	case GL_ALPHA8:		return w*h* 1;
	case GL_LUMINANCE8_ALPHA8:	return w*h* 2;
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:	return w*h /2;
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:	return w*h /2;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:	return w*h* 1;
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:	return w*h* 1;
	case GL_DU8DV8_ATI:
	case GL_DSDT_NV:						return w*h* 2;
	};

	// One format has not been coded.
	nlstop;

	/// ???
	return w*h* 4;
}


// ***************************************************************************
// Translation of Wrap mode.
static inline GLenum	translateWrapToGl(ITexture::TWrapMode mode, const CGlExtensions	&extensions)
{
	if(mode== ITexture::Repeat)
		return GL_REPEAT;
	else
	{
		if(extensions.Version1_2)
			return GL_CLAMP_TO_EDGE;
		else
			return GL_CLAMP;
	}
}


// ***************************************************************************
static inline GLenum	translateMagFilterToGl(CTextureDrvInfosGL *glText)
{	
#ifdef NEL_FORCE_NEAREST
	return GL_NEAREST;
#else // NEL_FORCE_NEAREST
	switch(glText->MagFilter)
	{
		case ITexture::Linear: return GL_LINEAR;
		case ITexture::Nearest: return GL_NEAREST;
		default: break;
	}

	nlstop;
	return GL_LINEAR;
#endif // NEL_FORCE_NEAREST
}


// ***************************************************************************
static inline GLenum	translateMinFilterToGl(CTextureDrvInfosGL *glText)
{
#ifdef NEL_FORCE_NEAREST
	return GL_NEAREST;
#else // NEL_FORCE_NEAREST

	if(glText->MipMap)
	{
		switch(glText->MinFilter)
		{
			case ITexture::NearestMipMapOff: return GL_NEAREST;
			case ITexture::NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
			case ITexture::NearestMipMapLinear: return GL_NEAREST_MIPMAP_LINEAR;
			case ITexture::LinearMipMapOff: return GL_LINEAR;
			case ITexture::LinearMipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
			case ITexture::LinearMipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
			default: break;
		}
	}
	else
	{
		switch(glText->MinFilter)
		{
			case ITexture::NearestMipMapOff:
			case ITexture::NearestMipMapNearest:
			case ITexture::NearestMipMapLinear:
				 return GL_NEAREST;
			case ITexture::LinearMipMapOff:
			case ITexture::LinearMipMapNearest:
			case ITexture::LinearMipMapLinear:
				 return GL_LINEAR;
			default: break;
		}
	}

	nlstop;
	return GL_LINEAR;
#endif // NEL_FORCE_NEAREST
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
static inline bool		isDXTCFormat(GLint glfmt)
{
	if(glfmt==GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
		return true;
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		return true;
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
		return true;
	if(glfmt==GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		return true;

	return false;
}


// ***************************************************************************
bool CDriverGL::setupTexture (ITexture& tex)
{
	bool nTmp;
	return setupTextureEx (tex, true, nTmp);
}

// ***************************************************************************
bool CDriverGL::setupTextureEx (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture)
{
	bAllUploaded = false;
	
	if(tex.isTextureCube() && (!_Extensions.ARBTextureCubeMap))
		return true;

	// 0. Create/Retrieve the driver texture.
	//=======================================
	bool mustCreate = false;
	if ( !tex.TextureDrvShare )
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItTexDrvSharePtrList	it= _TexDrvShares.insert(_TexDrvShares.end());
		// create and set iterator, for future deletion.
		*it= tex.TextureDrvShare= new CTextureDrvShare(this, it);

		// Must (re)-create the texture.
		mustCreate = true;
	}

	// Does the texture has been touched ?
	if ( (!tex.touched()) && (!mustCreate) )
		return true; // Do not do anything


	// 1. If modified, may (re)load texture part or all of the texture.
	//=================================================================


	bool	mustLoadAll= false;
	bool	mustLoadPart= false;


	// To avoid any delete/new ptr problem, disable all texturing.
	/* If an old texture is deleted, _CurrentTexture[*] and _CurrentTextureInfoGL[*] are invalid. 
		But this is grave only if a new texture is created, with the same pointer (bad luck). 
		Since an newly allocated texture always pass here before use, we are sure to avoid any problems.
	*/
	for(sint stage=0 ; stage<inlGetNumTextStages() ; stage++)
	{
		activateTexture(stage, NULL);
	}


	// A. Share mgt.
	//==============
	if(tex.supportSharing())
	{
		// Try to get the shared texture.

		// Create the shared Name.
		std::string	name;
		getTextureShareName (tex, name);

		// insert or get the texture.
		{
			CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
			TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

			ItTexDrvInfoPtrMap	itTex;
			itTex= rTexDrvInfos.find(name);

			// texture not found?
			if( itTex==rTexDrvInfos.end() )
			{
				// insert into driver map. (so it is deleted when driver is deleted).
				itTex= (rTexDrvInfos.insert(make_pair(name, (ITextureDrvInfos*)NULL))).first;
				// create and set iterator, for future deletion.
				itTex->second= tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL(this, itTex, this);

				// need to load ALL this texture.
				mustLoadAll= true;
			}
			else
			{
				tex.TextureDrvShare->DrvTexture= itTex->second;

				if(bMustRecreateSharedTexture)
					// reload this shared texture (user request)
					mustLoadAll= true;
				else
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
			// Must create it. Create auto a GL id (in constructor).
			// Do not insert into the map. This un-shared texture will be deleted at deletion of the texture.
			// Inform ITextureDrvInfos by passing NULL _Driver.
			tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL(NULL, ItTexDrvInfoPtrMap(), this);

			// need to load ALL this texture.
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
		_DriverGLStates.activeTextureARB(0);
		if(tex.isTextureCube())
		{
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);
			// Bind this texture, for reload...
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, gltext->ID);				
		}
		else
		{
			_DriverGLStates.setTextureMode(CDriverGLStates::Texture2D);
			// Bind this texture, for reload...
			glBindTexture(GL_TEXTURE_2D, gltext->ID);				
		}


		glPixelStorei(GL_UNPACK_ALIGNMENT,1);

		// a. Load All the texture case.
		//==============================
		if (mustLoadAll)
		{
			// profiling. sub old textre memory usage, and reset.
			_AllocatedTextureMemory-= gltext->TextureMemory;
			gltext->TextureMemory= 0;


			if(tex.isTextureCube())
			{
				static GLenum face_map[6] = {	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
												GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
												GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
												GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
												GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
												GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB };
				CTextureCube *pTC = NLMISC::safe_cast<CTextureCube *>(&tex);
				// Regenerate all the texture.
				tex.generate();					
				for(uint nText = 0; nText < 6; ++nText)
				if(pTC->getTexture((CTextureCube::TFace)nText) != NULL)
				{
					ITexture *pTInTC = pTC->getTexture((CTextureCube::TFace)nText);
					// Get the correct texture format from texture...
					GLint	glfmt= getGlTextureFormat(*pTInTC, gltext->Compressed);
					GLint	glSrcFmt= getGlSrcTextureFormat(*pTInTC, glfmt);
					GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

					sint	nMipMaps;
					if(glSrcFmt==GL_RGBA && pTInTC->getPixelFormat()!=CBitmap::RGBA )
						pTInTC->convertToType(CBitmap::RGBA);
					if(tex.mipMapOn())
					{
						pTInTC->buildMipMaps();
						nMipMaps= pTInTC->getMipMapCount();
					}
					else
						nMipMaps= 1;

					// MipMap upload?
					gltext->MipMap= nMipMaps>1;

					// Fill mipmaps.
					for(sint i=0;i<nMipMaps;i++)
					{
						void	*ptr= pTInTC->getPixels(i).getPtr();
						uint	w= pTInTC->getWidth(i);
						uint	h= pTInTC->getHeight(i);
						if (bUpload)
						{
							NEL_MEASURE_UPLOAD_TIME_START
							glTexImage2D (face_map[nText], i, glfmt, w, h, 0, glSrcFmt, glSrcType, ptr);
							bAllUploaded = true;
							NEL_MEASURE_UPLOAD_TIME_END
								
						}
						else
						{
							NEL_MEASURE_UPLOAD_TIME_START
							glTexImage2D (face_map[nText], i, glfmt, w, h, 0, glSrcFmt, glSrcType, NULL);
							NEL_MEASURE_UPLOAD_TIME_END
						}
						// profiling: count TextureMemory usage.
						gltext->TextureMemory+= computeMipMapMemoryUsage(w, h, glfmt);
					}
				}
			}
			else
			{
				// Regenerate all the texture.
				tex.generate();

				if(tex.getSize()>0)
				{
					// Get the correct texture format from texture...
					GLint	glfmt= getGlTextureFormat(tex, gltext->Compressed);
					GLint	glSrcFmt= getGlSrcTextureFormat(tex, glfmt);
					GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

					// DXTC: if same format, and same mipmapOn/Off, use glTexCompressedImage*.
					// We cannot build the mipmaps if they are not here.
					if(_Extensions.EXTTextureCompressionS3TC && sameDXTCFormat(tex, glfmt))
					{
						sint	nMipMaps = 1;
						
						if(tex.mipMapOn())
							nMipMaps= tex.getMipMapCount();

						// MipMap upload?
						gltext->MipMap= nMipMaps>1;

						// Degradation in Size allowed only if DXTC texture are provided with mipmaps.
						// Because use them to resize !!!
						uint	decalMipMapResize= 0;
						if(_ForceTextureResizePower>0 && tex.allowDegradation() && nMipMaps>1)
						{
							decalMipMapResize= min(_ForceTextureResizePower, (uint)(nMipMaps-1));
						}

						// Fill mipmaps.
						for(sint i=decalMipMapResize;i<nMipMaps;i++)
						{
							void	*ptr= tex.getPixels(i).getPtr();
							sint	size= tex.getPixels(i).size();
							if (bUpload)
							{
								nglCompressedTexImage2DARB (GL_TEXTURE_2D, i-decalMipMapResize, glfmt, 
															tex.getWidth(i),tex.getHeight(i), 0, size, ptr);
								bAllUploaded = true;
							}
							else
							{
								//nglCompressedTexImage2DARB (GL_TEXTURE_2D, i-decalMipMapResize, glfmt, 
								//							tex.getWidth(i),tex.getHeight(i), 0, size, NULL);
								NEL_MEASURE_UPLOAD_TIME_START
								glTexImage2D (GL_TEXTURE_2D, i-decalMipMapResize, glfmt, tex.getWidth(i), tex.getHeight(i), 
												0, glSrcFmt, glSrcType, NULL);
								NEL_MEASURE_UPLOAD_TIME_END
							}

							// profiling: count TextureMemory usage.
							gltext->TextureMemory+= tex.getPixels(i).size();
						}
					}
					else
					{
						sint	nMipMaps;
						if(glSrcFmt==GL_RGBA && tex.getPixelFormat()!=CBitmap::RGBA )
						{
							bUpload = true; // Force all upload
							tex.convertToType(CBitmap::RGBA);
						}

						// Degradation in Size.
						if(_ForceTextureResizePower>0 && tex.allowDegradation())
						{
							uint	w= tex.getWidth(0) >> _ForceTextureResizePower;
							uint	h= tex.getHeight(0) >> _ForceTextureResizePower;
							w= max(1U, w);
							h= max(1U, h);
							tex.resample(w, h);
						}

						if(tex.mipMapOn())
						{
							tex.buildMipMaps();
							nMipMaps= tex.getMipMapCount();
						}
						else
							nMipMaps= 1;

						// MipMap upload?
						gltext->MipMap= nMipMaps>1;

						// Fill mipmaps.
						for(sint i=0;i<nMipMaps;i++)
						{
							void	*ptr= tex.getPixels(i).getPtr();
							uint	w= tex.getWidth(i);
							uint	h= tex.getHeight(i);

							if (bUpload)
							{	
								NEL_MEASURE_UPLOAD_TIME_START
								glTexImage2D (GL_TEXTURE_2D, i, glfmt, w, h, 0,glSrcFmt, glSrcType, ptr);
								NEL_MEASURE_UPLOAD_TIME_END
								bAllUploaded = true;
							}
							else
							{	
								NEL_MEASURE_UPLOAD_TIME_START
								glTexImage2D (GL_TEXTURE_2D, i, glfmt, w, h, 0,glSrcFmt, glSrcType, NULL);								
								NEL_MEASURE_UPLOAD_TIME_END
							}
							// profiling: count TextureMemory usage.
							gltext->TextureMemory += computeMipMapMemoryUsage (w, h, glfmt);
						}
					}
				}
			}
			//printf("%d,%d,%d\n", tex.getMipMapCount(), tex.getWidth(0), tex.getHeight(0));


			// profiling. add new TextureMemory usage.
			_AllocatedTextureMemory+= gltext->TextureMemory;
		}
		// b. Load part of the texture case.
		//==================================
		// \todo yoyo: TODO_DXTC
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
				GLint	glSrcFmt= getGlSrcTextureFormat(tex, glfmt);
				GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);

				sint	nMipMaps;
				if(glSrcFmt==GL_RGBA && tex.getPixelFormat()!=CBitmap::RGBA )
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
						void	*ptr= tex.getPixels(i).getPtr();
						sint	w= tex.getWidth(i);
						sint	h= tex.getHeight(i);
						clamp(x0, 0, w);
						clamp(y0, 0, h);
						clamp(x1, x0, w);
						clamp(y1, y0, h);

						glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
						glPixelStorei(GL_UNPACK_SKIP_ROWS, y0);
						glPixelStorei(GL_UNPACK_SKIP_PIXELS, x0);
						if (bUpload)
							glTexSubImage2D (GL_TEXTURE_2D, i, x0, y0, x1-x0, y1-y0, glSrcFmt,glSrcType, ptr);
						else
							glTexSubImage2D (GL_TEXTURE_2D, i, x0, y0, x1-x0, y1-y0, glSrcFmt,glSrcType, NULL);

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
		if(tex.isTextureCube())
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_S, translateWrapToGl(ITexture::Clamp, _Extensions));
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_T, translateWrapToGl(ITexture::Clamp, _Extensions));
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_WRAP_R, translateWrapToGl(ITexture::Clamp, _Extensions));
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, translateWrapToGl(gltext->WrapS, _Extensions));
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, translateWrapToGl(gltext->WrapT, _Extensions));
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));
		}



		// Disable texture 0
		_CurrentTexture[0]= NULL;
		_CurrentTextureInfoGL[0]= NULL;
		_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
	}


	// The texture is correctly setuped.
	tex.clearTouched();
	return true;
}


// ***************************************************************************
bool CDriverGL::uploadTexture (ITexture& tex, CRect& rect, uint8 nNumMipMap)
{
	if (tex.TextureDrvShare == NULL)
		return false; // Texture not created
	if (tex.TextureDrvShare->DrvTexture == NULL)
		return false; // Texture not created
	if (tex.isTextureCube())
		return false;

	nlassert(nNumMipMap<(uint8)tex.getMipMapCount());

	// validate rect.
	sint x0 = rect.X;
	sint y0 = rect.Y;
	sint x1 = rect.X+rect.Width;
	sint y1 = rect.Y+rect.Height;
	sint w = tex.getWidth (nNumMipMap);
	sint h = tex.getHeight (nNumMipMap);
	clamp (x0, 0, w);
	clamp (y0, 0, h);
	clamp (x1, x0, w);
	clamp (y1, y0, h);

	// bind the texture to upload 
	CTextureDrvInfosGL*	gltext;
	gltext = getTextureGl (tex);

	// system of "backup the previous binded texture" seems to not work with some drivers....
	_DriverGLStates.activeTextureARB (0);
	_DriverGLStates.setTextureMode (CDriverGLStates::Texture2D);
	// Bind this texture, for reload...
	glBindTexture (GL_TEXTURE_2D, gltext->ID);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	bool dummy;
	GLint glfmt = getGlTextureFormat (tex, dummy);
	GLint glSrcFmt = getGlSrcTextureFormat (tex, glfmt);
	GLenum  glSrcType= getGlSrcTextureComponentType(glSrcFmt);
	// If DXTC format
	if (_Extensions.EXTTextureCompressionS3TC && sameDXTCFormat(tex, glfmt))
	{

		sint nUploadMipMaps;
		if (tex.mipMapOn())
			nUploadMipMaps = tex.getMipMapCount();
		else
			nUploadMipMaps = 1;

		uint decalMipMapResize = 0;
		if (_ForceTextureResizePower>0 && tex.allowDegradation() && nUploadMipMaps>1)
		{
			decalMipMapResize = min(_ForceTextureResizePower, (uint)(nUploadMipMaps-1));
		}

		// Compute src compressed size and location
		sint imageSize = (x1-x0)*(y1-y0);
		void *ptr = tex.getPixels(nNumMipMap).getPtr();

		// If DXTC1 or DXTC1A, then 4 bits/texel else 8 bits/texel
		if (glfmt == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || glfmt == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		{
			imageSize /= 2;
			ptr = (uint8*)ptr + y0*w/2 + x0/2;
		}
		else
		{
			ptr = (uint8*)ptr + y0*w + x0;
		}

		// Upload

		if (decalMipMapResize > nNumMipMap)
		{
			_CurrentTexture[0]= NULL;
			_CurrentTextureInfoGL[0]= NULL;
			_DriverGLStates.setTextureMode (CDriverGLStates::TextureDisabled);
			return false;
		}
			

		nlassert (((x0&3) == 0) && ((y0&3) == 0));
		if ((w>=4) && (h>=4))
		{
			nglCompressedTexSubImage2DARB (	GL_TEXTURE_2D, nNumMipMap-decalMipMapResize, 
											x0, y0, (x1-x0), (y1-y0), glfmt, imageSize, ptr );
		}
		else
		{
			// The CompressedTexSubImage2DARB function do not functionnate properly if width or height
			// of the mipmap is less than 4 pixel so we use the other form. (its not really time critical
			// to upload 16 bytes so we can do it twice if texture is cut)
			imageSize = tex.getPixels(nNumMipMap).size();
			nglCompressedTexImage2DARB (GL_TEXTURE_2D, nNumMipMap-decalMipMapResize, 
										glfmt, w, h, 0, imageSize, ptr);
		}
	}
	else
	{
		// glSrcFmt and ITexture format must be identical
		nlassert (glSrcFmt!=GL_RGBA || tex.getPixelFormat()==CBitmap::RGBA);

		void	*ptr= tex.getPixels(nNumMipMap).getPtr();
		glPixelStorei (GL_UNPACK_ROW_LENGTH, w);
		glPixelStorei (GL_UNPACK_SKIP_ROWS, y0);
		glPixelStorei (GL_UNPACK_SKIP_PIXELS, x0);
		glTexSubImage2D (GL_TEXTURE_2D, nNumMipMap, x0, y0, x1-x0, y1-y0, glSrcFmt,glSrcType, ptr);

		// Reset the transfer mode...
		glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);
	}

	// Disable texture 0
	_CurrentTexture[0]= NULL;
	_CurrentTextureInfoGL[0]= NULL;
	_DriverGLStates.setTextureMode (CDriverGLStates::TextureDisabled);

	return true;
}

// ***************************************************************************
bool CDriverGL::uploadTextureCube (ITexture& tex, CRect& rect, uint8 nNumMipMap, uint8 nNumFace)
{
	if (tex.TextureDrvShare == NULL)
		return false; // Texture not created
	if (!tex.isTextureCube())
		return false;

	return true;
}

// ***************************************************************************
bool CDriverGL::activateTexture(uint stage, ITexture *tex)
{	
	if (this->_CurrentTexture[stage]!=tex)
	{
		_DriverGLStates.activeTextureARB(stage);
		if(tex)
		{
			// get the drv info. should be not NULL.
			CTextureDrvInfosGL*	gltext;
			gltext= getTextureGl(*tex);

			// Profile, log the use of this texture
			//=========================================
			if (_SumTextureMemoryUsed)
			{
				// Insert the pointer of this texture
				_TextureUsed.insert (gltext);
			}


			if(tex->isTextureCube())
			{
				// setup texture mode, after activeTextureARB()
				_DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);

				if(_Extensions.ARBTextureCubeMap)
				{
					// Activate texturing...
					//======================

					// If the shared texture is the same than before, no op.
					if(_CurrentTextureInfoGL[stage] != gltext)
					{
						// Cache setup.
						_CurrentTextureInfoGL[stage]= gltext;

						// setup this texture
						glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, gltext->ID);
						

						// Change parameters of texture, if necessary.
						//============================================
						if(gltext->MagFilter!= tex->getMagFilter())
						{
							gltext->MagFilter= tex->getMagFilter();
							glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
						}
						if(gltext->MinFilter!= tex->getMinFilter())
						{
							gltext->MinFilter= tex->getMinFilter();
							glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));
						}
					}
				}
			}
			else
			{
				// setup texture mode, after activeTextureARB()
				_DriverGLStates.setTextureMode(CDriverGLStates::Texture2D);								

				// Activate texture...
				//======================

				// If the shared texture is the same than before, no op.
				if(_CurrentTextureInfoGL[stage] != gltext)
				{
					// Cache setup.
					_CurrentTextureInfoGL[stage]= gltext;

					// setup this texture
					glBindTexture(GL_TEXTURE_2D, gltext->ID);								


					// Change parameters of texture, if necessary.
					//============================================
					if(gltext->WrapS!= tex->getWrapS())
					{
						gltext->WrapS= tex->getWrapS();
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, translateWrapToGl(gltext->WrapS, _Extensions));
					}
					if(gltext->WrapT!= tex->getWrapT())
					{
						gltext->WrapT= tex->getWrapT();
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, translateWrapToGl(gltext->WrapT, _Extensions));
					}
					if(gltext->MagFilter!= tex->getMagFilter())
					{
						gltext->MagFilter= tex->getMagFilter();
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, translateMagFilterToGl(gltext));
					}
					if(gltext->MinFilter!= tex->getMinFilter())
					{
						gltext->MinFilter= tex->getMinFilter();					
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, translateMinFilterToGl(gltext));
					}
				}
			}
		}
		else
		{
			// Force no texturing for this stage.
			_CurrentTextureInfoGL[stage]= NULL;
			// setup texture mode, after activeTextureARB()
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);									
		}

		this->_CurrentTexture[stage]= tex;	
	}	

	return true;
}

// ***************************************************************************
void		CDriverGL::forceActivateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env)
{
	// This maps the CMaterial::TTexOperator
	static	const	GLenum	operatorLUT[9]= { GL_REPLACE, GL_MODULATE, GL_ADD, GL_ADD_SIGNED_EXT, 
		GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_INTERPOLATE_EXT, GL_BUMP_ENVMAP_ATI };

	// This maps the CMaterial::TTexSource
	static	const	GLenum	sourceLUT[4]= { GL_TEXTURE, GL_PREVIOUS_EXT, GL_PRIMARY_COLOR_EXT, GL_CONSTANT_EXT };

	// This maps the CMaterial::TTexOperand
	static	const	GLenum	operandLUT[4]= { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };

	// This maps the CMaterial::TTexOperator, used for openGL Arg2 setup.
	static	const	GLenum	interpolateSrcLUT[8]= { GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, GL_TEXTURE, 
		GL_TEXTURE, GL_PREVIOUS_EXT, GL_PRIMARY_COLOR_EXT, GL_CONSTANT_EXT };



	// cache mgt.
	_CurrentTexEnv[stage].EnvPacked= env.EnvPacked;
	// Disable Special tex env f().
	_CurrentTexEnvSpecial[stage]= TexEnvSpecialDisabled;


	// Setup the gl env mode.
	_DriverGLStates.activeTextureARB(stage);
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
void		CDriverGL::activateTexEnvColor(uint stage, NLMISC::CRGBA col)
{	
	if (col != _CurrentTexEnv[stage].ConstantColor)
	{	
		forceActivateTexEnvColor(stage, col);	
	}
}


// ***************************************************************************
void		CDriverGL::activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env)
{
	// If a special Texture environnement is setuped, or if not the same normal texture environnement,
	// must setup a new normal Texture environnement.
	if( _CurrentTexEnvSpecial[stage] != TexEnvSpecialDisabled || _CurrentTexEnv[stage].EnvPacked!= env.EnvPacked)
	{ 
		forceActivateTexEnvMode(stage, env);
	}
}


// ***************************************************************************
void		CDriverGL::activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env)
{
	if(_CurrentTexEnv[stage].ConstantColor!= env.ConstantColor)
	{ 
		forceActivateTexEnvColor(stage, env);
	}
}


// ***************************************************************************
void		CDriverGL::forceDXTCCompression(bool dxtcComp)
{
	_ForceDXTCCompression= dxtcComp;
}

// ***************************************************************************
void		CDriverGL::forceTextureResize(uint divisor)
{
	clamp(divisor, 1U, 256U);

	// 16 -> 4.
	_ForceTextureResizePower= getPowerOf2(divisor);
}


// ***************************************************************************
void		CDriverGL::swapTextureHandle(ITexture &tex0, ITexture &tex1)
{
	// ensure creation of both texture
	setupTexture(tex0);
	setupTexture(tex1);

	// avoid any problem, disable all textures
	for(sint stage=0; stage<inlGetNumTextStages() ; stage++)
	{
		activateTexture(stage, NULL);
	}

	// get the handle.
	CTextureDrvInfosGL	*t0= getTextureGl(tex0);
	CTextureDrvInfosGL	*t1= getTextureGl(tex1);

	/* Swap contents. Can't swap directly the pointers cause would have to change all CTextureDrvShare which point on
		Can't do swap(*t0, *t1), because must keep the correct _DriverIterator
	*/
	swap(t0->ID, t1->ID);
	swap(t0->MipMap, t1->MipMap);
	swap(t0->Compressed, t1->Compressed);
	swap(t0->TextureMemory, t1->TextureMemory);
	swap(t0->WrapS, t1->WrapS);
	swap(t0->WrapT, t1->WrapT);
	swap(t0->MagFilter, t1->MagFilter);
	swap(t0->MinFilter, t1->MinFilter);

}


// ***************************************************************************
uint CDriverGL::getTextureHandle(const ITexture &tex)
{
	// If DrvShare not setuped
	if(!tex.TextureDrvShare)
		return 0;

	// If DrvInfo not setuped
	const CTextureDrvInfosGL	*t0= (const CTextureDrvInfosGL*)(const ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
	if(!t0)
		return 0;

	return t0->ID;
}


} // NL3D





















