/** \file u_instance_material.h
 * <File description>
 *
 * $Id: u_instance_material.h,v 1.13 2003/11/17 14:25:09 besson Exp $
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

#ifndef NL_U_INSTANCE_MATERIAL_H
#define NL_U_INSTANCE_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"


namespace NL3D
{


using NLMISC::CRGBA;


// ***************************************************************************
/**
 * Base interface for manipulating Material retrieved from UInstance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstanceMaterial
{
protected:

	/// \name Object
	// @{
	UInstanceMaterial() {}
	virtual	~UInstanceMaterial() {}
	// @}

public:
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor, blendCount };
public:

	/// \name Modes.
	// @{
	virtual	bool				isLighted() const =0;
	virtual	bool				isUserColor() const =0;
	// @}

	/// \name Blending.
	// @{
	virtual void			setBlend(bool active) =0;
	virtual void			setBlendFunc(TBlend src, TBlend dst) =0;
	virtual void			setSrcBlend(TBlend val) =0;
	virtual void			setDstBlend(TBlend val) =0;
	virtual void			setAlphaTestThreshold(float at) =0;
	virtual void			setZWrite(bool active) =0;

	virtual bool			getBlend() const =0;
	virtual TBlend			getSrcBlend(void)  const =0;
	virtual TBlend			getDstBlend(void)  const =0;
	// @}


	/// \name Lighted material mgt. Has effect only if isLighted().
	// @{

	/// Set the emissive part ot material. Usefull only if isLighted()
	virtual	void				setEmissive( CRGBA emissive=CRGBA(0,0,0) )=0;
	/// Set the Ambient part ot material. Usefull only if isLighted()
	virtual	void				setAmbient( CRGBA ambient=CRGBA(0,0,0) )=0;
	/// Set the Diffuse part ot material. Usefull only if isLighted()
	virtual	void				setDiffuse( CRGBA diffuse=CRGBA(0,0,0) )=0;
	/// Set the Opacity part ot material. Usefull only if isLighted()
	virtual	void				setOpacity( uint8	opa )=0;
	/// Set the specular part ot material. Usefull only if isLighted()
	virtual	void				setSpecular( CRGBA specular=CRGBA(0,0,0) )=0;
	/// Set the shininess part ot material. Usefull only if isLighted()
	virtual	void				setShininess( float shininess )=0;

	virtual	CRGBA				getEmissive() const =0;
	virtual	CRGBA				getAmbient() const =0;
	/// return diffuse part. NB: A==opacity.
	virtual	CRGBA				getDiffuse() const =0;
	virtual	uint8				getOpacity() const =0;
	virtual	CRGBA				getSpecular() const =0;
	virtual	float				getShininess() const =0;

	// @}


	/// \name UnLighted material mgt. Has effect only if !isLighted().
	// @{
	virtual	void				setColor(CRGBA rgba) =0;
	virtual	CRGBA				getColor(void) const =0;
	// @}

	/// \name Per stage constant color
	// @{
	virtual void				setConstantColor(uint stage, NLMISC::CRGBA color) = 0;
	virtual NLMISC::CRGBA		getConstantColor(uint stage) const = 0;
	// @}


	/// \name Texture UserColor. No effect if !isUserColor(). (getUserColor() return CRGBA(0,0,0,0))
	// @{
	virtual	void				setUserColor(CRGBA userColor)=0;
	virtual	CRGBA				getUserColor() const =0;
	// @}

	/// \name Texture files specific
	// @{
	/// Get the last stage that got a texture. -1 means there is no textures.
	virtual sint				getLastTextureStage() const = 0;
	/// Check wether the texture of the n-th stage is a texture file
	virtual bool				isTextureFile(uint stage = 0) const = 0;
	/// Get the fileName used by the n-th texture file. (must be a texture file or an assertion is raised)
	virtual std::string			getTextureFileName(uint stage = 0) const = 0;
	// Empty the texture at the given stage
	virtual void				emptyTexture(uint stage = 0) = 0;
	/** Set the fileName used by the n-th texture file. (must be a texture file or an assertion is raised)
	 *	NB: if success and if instanceOwner->getAsyncTextureMode()==true, then instanceOwner->setAsyncTextureDirty(true) 
	 *	is called
	 */
	virtual void				setTextureFileName(const std::string &fileName, uint stage = 0) = 0;		

	/// \name Texture matrix
	// @{
	// Enable a user texture matrix for the n-th stage. The initial matrix is set to identity.
	virtual void                    enableUserTexMat(uint stage, bool enabled = true) = 0;
	// Test wether a user texture is enabled for the n-th stage
	virtual bool                    isUserTexMatEnabled(uint stage) const = 0;
	/// Set a new texture matrix for the given stage.	
	virtual void					setUserTexMat(uint stage, const NLMISC::CMatrix &m) = 0;
	/** Get a const ref. on the texture matrix of the n-th stage.	  
	  */
	virtual const NLMISC::CMatrix  &getUserTexMat(uint stage) const = 0;		
	// @}
};


} // NL3D


#endif // NL_U_INSTANCE_MATERIAL_H

/* End of u_instance_material.h */
