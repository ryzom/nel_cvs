/** \file u_texture.h
 * <File description>
 *
 * $Id: u_texture.h,v 1.5 2003/04/23 10:29:18 besson Exp $
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

#ifndef NL_U_TEXTURE_H
#define NL_U_TEXTURE_H

#include "nel/misc/types_nl.h"


namespace NL3D {


// ***************************************************************************
/**
 * Game interface for manipulating Basic texture.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTexture
{
protected:

	/// \name Object
	// @{
	UTexture() {}
	virtual	~UTexture() {}
	// @}


public:
	// Those enums MUST be the same than in ITexture!!

	enum	TWrapMode
	{
		Repeat= 0,
		Clamp,

		WrapModeCount
	};

	enum	TUploadFormat
	{
		Auto= 0,
		RGBA8888,
		RGBA4444,
		RGBA5551,
		RGB888,
		RGB565,
		DXTC1,
		DXTC1Alpha,
		DXTC3,
		DXTC5,
		Luminance,
		Alpha,
		AlphaLuminance,
		DsDt,
		UploadFormatCount
	};


	/** Magnification mode.
	 * Same behavior as OpenGL.
	 */
	enum	TMagFilter
	{
		Nearest=0,
		Linear,

		MagFilterCount
	};

	/** Minifying mode.
	 * Same behavior as OpenGL. If the bitmap has no mipmap, and mipmap is required, then mipmaps are computed.
	 */
	enum	TMinFilter
	{
		NearestMipMapOff=0,
		NearestMipMapNearest,
		NearestMipMapLinear,
		LinearMipMapOff,
		LinearMipMapNearest,
		LinearMipMapLinear,

		MinFilterCount
	};

public:


	/// \name Texture parameters.
	/** By default, parameters are:
		- WrapS==Repeat
		- WrapT==Repeat
		- UploadFormat== Auto
		- MagFilter== Linear.
		- MinFilter= LinearMipMapLinear.

		NB: if multiple ITexture acces the same data via the sharing system (such as a CTextureFile), then:
			- WrapS/WrapT is LOCAL for each ITexture (ie each ITexture will have his own Wrap mode) => no duplication
				is made.
			- UploadFormat may duplicate the texture in video memory. There is one texture per different UploadFormat.
			- MinFilter may duplicate the texture in video memory in the same way, wether the texture has mipmap or not.
	 */
	// @{
	virtual	void			setWrapS(TWrapMode mode) =0;
	virtual	void			setWrapT(TWrapMode mode) =0;
	virtual	TWrapMode		getWrapS() const =0;
	virtual	TWrapMode		getWrapT() const =0;
	/** Replace the uploaded format of the texture.
	 * If "Auto", the driver use CBitmap::getPixelFormat() to find the best associated pixelFormat.
	 * When no alpha is wanted (RGB, Luminance....), texture default output is 1.0.
	 * For "Alpha" mode, RGB output is (0,0,0).
	 */
	virtual	void			setUploadFormat(TUploadFormat pf) =0;
	virtual	TUploadFormat	getUploadFormat() const =0;
	virtual	void			setFilterMode(TMagFilter magf, TMinFilter minf) =0;
	virtual	TMagFilter		getMagFilter() const =0;
	virtual	TMinFilter		getMinFilter() const =0;
	virtual	bool			mipMapOff() const =0;
	virtual	bool			mipMapOn() const =0;
	// @}

	virtual	void			setReleasable(bool bReleasable) =0;
	virtual	NLMISC::CRGBA	getPixelColor(sint32 x, sint32 y) const =0;
};



// ***************************************************************************
/**
 * Game interface for manipulating texture File.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	UTextureFile : virtual public UTexture
{
public:

	/** 
	 * Set the name of the file containing the texture
	 * \param name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	virtual	void setFileName(std::string s) =0;


	/** 
	 * get the name of the file containing the texture
	 * \return name of the file
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	virtual	std::string getFileName() const =0;

	/// tells if this texture allow the driver to degrade it (default is false for UTextureFile).
	virtual bool	allowDegradation() const =0;
	/// Change the degradation mode. NB: this must be done before first render(), ie just after creation.
	virtual void	setAllowDegradation(bool allow) =0;

};


// ***************************************************************************
/**
 * Game interface for manipulating texture RAW.  \todo yoyo: TODO_TEXTURERAW!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	UTextureRaw : virtual public UTexture
{
public:

	/// \todo yoyo: TODO_TEXTURERAW!!
};



} // NL3D


#endif // NL_U_TEXTURE_H

/* End of u_texture.h */
