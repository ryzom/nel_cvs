/** \file driver_opengl_texture.cpp
 * OpenGL driver implementation : setupTexture
 *
 * $Id: driver_opengl_texture.cpp,v 1.8 2000/12/15 15:32:08 corvazier Exp $
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <gl/gl.h>
#include "driver_opengl.h"
using	namespace std;


namespace NL3D
{


// Get the Id of an existing setuped texture.
static	inline GLint	getTextureGlId(ITexture& tex)
{
	CTextureDrvInfosGL*	gltex;
	gltex= (CTextureDrvInfosGL*)(ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
	return gltex->ID;
}


bool CDriverGL::setupTexture(ITexture& tex)
{
	bool	touched= tex.touched();

	// 0. Create/Retrieve the driver texture.
	//=======================================
	if ( !tex.TextureDrvShare )
	{
		// Create the shared texture.
		tex.TextureDrvShare= new CTextureDrvShare;

		// Insert into Driver list.
		_TexDrvShares.push_back(tex.TextureDrvShare);

		// Must (re)-create the texture.
		touched= true;
	}

	// 1. If modified, (re)load texture.
	//==================================
	if ( touched )
	{
		bool	mustload= true;
		// Share mgt.
		//===========
		if(tex.supportSharing())
		{
			// Try to get the shared texture.
			string	name= tex.getShareName();
			// insert or get the texture.
			ITextureDrvInfos	*gltext= _TexDrvInfos[name];
			/* There is a trick here: this test both if:
				- the textureptr was not in the map. (because CRefPtr() is inited to NULL)
				- the texture was deleted (by smartptr) by TextureDrvShare. Hence the entry is not deleted even if
					the ptr is.
			*/
			if(gltext==NULL)
				_TexDrvInfos[name]= tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL;
			else
			{
				tex.TextureDrvShare->DrvTexture= gltext;
				// Do not need to reload this texture.
				mustload= false;
			}
		}
		else
		{
			// Else must create it.
			// Create auto a GL id (in constructor).
			tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosGL;
			// Do not insert into the map. This un-shared texture will be deleted at deletion of the texture.
		}

		// Setup texture.
		//===============
		if(mustload)
		{
			glBindTexture(GL_TEXTURE_2D, getTextureGlId(tex));

			tex.generate();

			if(tex.getSize()>0)
			{
				GLint	texfmt=GL_RGBA8;
				tex.convertToType(CBitmap::RGBA);
				tex.buildMipMaps();
				for(sint i=0;i<(sint)tex.getMipMapCount();i++)
				{
					void	*ptr= &(*tex.getPixels(i).begin());
					glTexImage2D(GL_TEXTURE_2D,i,GL_RGBA8,tex.getWidth(i),tex.getHeight(i),0,GL_RGBA,GL_UNSIGNED_BYTE, ptr );
				}
			}

			//printf("%d,%d,%d\n", tex.getMipMapCount(), tex.getWidth(0), tex.getHeight(0));

			tex.clearTouched();
			// Release, if wanted.
			if(tex.getReleasable())
				tex.release();

			// Basic parameters.
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		}

	}
	return(true);
}

// --------------------------------------------------

bool CDriverGL::activateTexture(uint stage, ITexture *tex)
{
	/// \todo: yoyo: ARB_multitexture.

	if (this->_CurrentTexture[stage]!=tex)
	{
		if(tex)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, getTextureGlId(*tex));
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
		}

		this->_CurrentTexture[stage]= tex;
	}
	return(true);
}


}
