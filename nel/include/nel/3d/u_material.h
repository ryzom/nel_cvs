/** \file u_material.h
 * <File description>
 *
 * $Id: u_material.h,v 1.5 2004/03/23 10:10:41 vizerie Exp $
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
class   UDriver;


// ***************************************************************************
/**
 * Game Interface for Material. Material for gamers are Unlighted materials!! Only normal material unlighted is supported.
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

	/// \name Texture Env Modes.
	// @{
	/** Environements operators:
	 * Replace:			out= arg0
	 * Modulate:		out= arg0 * arg1
	 * Add:				out= arg0 + arg1
	 * AddSigned:		out= arg0 + arg1 -0.5
	 * Interpolate*:	out= arg0*As + arg1*(1-As),  where As is taken from the SrcAlpha of 
	 *		Texture/Previous/Diffuse/Constant, respectively if operator is
	 *		InterpolateTexture/InterpolatePrevious/InterpolateDiffuse/InterpolateConstant.
	 * Multiply-Add (Mad) out= arg0 * arg1 + arg2. Must be supported by driver	 
	 * EMBM : apply to both color and alpha : the current texture, whose format is DSDT, is used to offset the texture in the next stage.
	 *  NB : for EMBM and InterpolateConstant, this must be supported by driver.
	 */
	enum TTexOperator		{ Replace=0, Modulate, Add, AddSigned, 
							  InterpolateTexture, InterpolatePrevious, InterpolateDiffuse, InterpolateConstant, EMBM, Mad };

	/** Source argument.
	 * Texture:		the arg is taken from the current texture of the stage.
	 * Previous:	the arg is taken from the previous enabled stage. If stage 0, Previous==Diffuse.
	 * Diffuse:		the arg is taken from the primary color vertex.
	 * Constant:	the arg is taken from the constant color setuped for this texture stage.
	 */
	enum TTexSource			{ Texture=0, Previous, Diffuse, Constant };

	/** Operand for the argument.
	 * For Alpha arguments, only SrcAlpha and InvSrcAlpha are Valid!! \n
	 * SrcColor:	arg= ColorSource.
	 * InvSrcColor:	arg= 1-ColorSource.
	 * SrcAlpha:	arg= AlphaSource.
	 * InvSrcAlpha:	arg= 1-AlphaSource.
	 */
	enum TTexOperand		{ SrcColor=0, InvSrcColor, SrcAlpha, InvSrcAlpha };
	// @}

public:	
	/// \name Texture.
	// @{
	// Set a texture in a stage
	virtual void 			setTexture(uint stage, UTexture* ptex) =0;
	// Is a texture present in the stage ?
	virtual bool			texturePresent (uint stage)=0;
	/// select from a texture set for this material (if available)
	virtual void			selectTextureSet(uint id)=0;

	// Deprecated use setTexture(uint stage, UTexture* ptex)
	virtual void 			setTexture(UTexture* ptex) =0;
	// Deprecated use texturePresent (uint stage)
	virtual bool			texturePresent() =0;
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

	/// \name Texture environnement.
	// @{
	virtual void			texEnvOpRGB(uint stage, TTexOperator ope) =0;
	virtual void			texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper) =0;
	virtual void			texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper) =0;
	virtual void			texEnvArg2RGB(uint stage, TTexSource src, TTexOperand oper) =0;
	virtual void			texEnvOpAlpha(uint stage, TTexOperator ope) =0;
	virtual void			texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper) =0;
	virtual void			texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper) =0;
	virtual void			texEnvArg2Alpha(uint stage, TTexSource src, TTexOperand oper) =0;
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

	// test if the given driver will support rendering of that material
	virtual	bool			isSupportedByDriver(UDriver &drv) = 0;
	// @}



};


} // NL3D


#endif // NL_U_MATERIAL_H

/* End of u_material.h */
