/** \file driver_opengl_texture.cpp
 * OpenGL driver implementation : setupTexture
 *
 * $Id: driver_opengl_texture.cpp,v 1.4 2000/12/04 16:58:59 berenguier Exp $
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



namespace NL3D
{


bool CDriverGL::setupTexture(ITexture& tex)
{
	bool	touched= tex.touched();

	// 0. Create/Retrieve the driver texture.
	//=======================================
	if ( !tex.DrvInfos )
	{
		// Create auto a GL id (in constructor).
		tex.DrvInfos= new CTextureDrvInfosGL;
		// Insert into Driver list.
		_TexDrvInfos.push_back(tex.DrvInfos);

		// Must (re)-create the texture.
		touched= true;
	}

	// 
	// 1. If modified, load texture.
	//==============================
	if ( touched )
	{
		glBindTexture(GL_TEXTURE_2D,((CTextureDrvInfosGL*)(ITextureDrvInfos*)tex.DrvInfos)->ID);

		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		// Create the texture.
		tex.generate();

		if(tex.getSize()>0)
		{
			tex.convertToType(CBitmap::RGBA);
			void	*ptr= &(*tex.getPixels().begin());
			glTexImage2D(GL_TEXTURE_2D,0,4,tex.getWidth(),tex.getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE, ptr );
		}

		tex.clearTouched();
		// Release, if wanted.
		if(tex.getReleasable())
			tex.release();
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
			ITextureDrvInfos*	iinfos;
			CTextureDrvInfosGL*	cinfos;
			iinfos=(ITextureDrvInfos*)tex->DrvInfos;
			cinfos=(CTextureDrvInfosGL*)iinfos;
			glBindTexture(GL_TEXTURE_2D,cinfos->ID);
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
