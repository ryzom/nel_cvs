/** \file u_material.h
 * <File description>
 *
 * $Id: u_material.h,v 1.3 2002/07/18 17:41:40 vizerie Exp $
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

#ifndef NL_U_MATERIAL_H
#define NL_U_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"


namespace NL3D 
{

using NLMISC::CRGBA;


class	UTexture;


// ***************************************************************************
/**
 * Game Interface for Material. Material for gamers are Unlighted materials!! There is only One texture stage.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UMaterial
{
protected:

	/// \name Object
	// @{
	UMaterial() {}
	virtual ~UMaterial() {}
	// @}

public:
	enum ZFunc				{ always=0,never,equal,notequal,less,lessequal,greater,greaterequal, zfuncCount };
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor, blendCount };

public:

	/// \name Texture.
	// @{
	virtual void 			setTexture(UTexture* ptex) =0;
	virtual bool			texturePresent() =0;
	/// select from a texture set for this material (if available)
	virtual void			selectTextureSet(uint id)=0;
	// @}


	/// \name Blending.
	// @{
	virtual void			setBlend(bool active) =0;
	virtual void			setBlendFunc(TBlend src, TBlend dst) =0;
	virtual void			setSrcBlend(TBlend val) =0;
	virtual void			setDstBlend(TBlend val) =0;

	virtual bool			getBlend() const =0;
	virtual TBlend			getSrcBlend(void)  const =0;
	virtual TBlend			getDstBlend(void)  const =0;
	// @}


	/// \name ZBuffer.
	// @{
	virtual void			setZFunc(ZFunc val) =0;
	virtual void			setZWrite(bool active) =0;
	virtual void			setZBias(float val) =0;

	virtual ZFunc			getZFunc(void)  const  =0;
	virtual bool			getZWrite(void)  const =0;
	virtual float			getZBias(void)  const  =0;
	// @}


	/// \name Color/Lighting..
	// @{
	/// The Color is used only if lighting is disabled. Also, color is replaced by per vertex color (if any).
	virtual void			setColor(CRGBA rgba) =0;

	virtual CRGBA			getColor(void) const =0;
	

	// @}

	/// \name Culling
	// @{
		virtual void			setDoubleSided(bool doubleSided = true)  = 0;
		virtual bool			getDoubleSided() const = 0;
	// @}


	/// \name Misc
	// @{
	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	virtual void			initUnlit() =0;
	// @}



};


} // NL3D


#endif // NL_U_MATERIAL_H

/* End of u_material.h */
