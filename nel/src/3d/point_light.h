/** \file point_light.h
 * <File description>
 *
 * $Id: point_light.h,v 1.1 2002/02/06 16:54:56 berenguier Exp $
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

#ifndef NL_POINT_LIGHT_H
#define NL_POINT_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"
#include "nel/misc/stl_block_list.h"


namespace NL3D 
{


using NLMISC::CVector;
using NLMISC::CRGBA;


class	CLight;
class	CTransform;


// ***************************************************************************
// Size of a block for allocation of lighted models nodes.
#define	NL3D_LIGHTED_MODEL_ALLOC_BLOCKSIZE	1024


// ***************************************************************************
/**
 *	Description of a light. Owned by an IG, or a CPointLightModel.
 *	With the special sunLight, this is the only light which can interact with CTransform models in the
 *	standard lighting system.
 *
 *	Only Positionnal with or without attenuation are supported. No spot, no directionnal.
 *	This restriction is for faster rendering, especially if VertexProgram is used.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 * \see CLightingManager
 */
class CPointLight
{
public:
	/// The list of model this light influence.
	typedef	NLMISC::CSTLBlockList<CTransform*>	TTransformList;
	typedef	TTransformList::iterator			ItTransformList;

public:

	/** Constructor
	 *	Default ambient is Black, Diffuse and Specular are white.
	 *	Position is CVector::Null.
	 *	Attenuation is 10-30.
	 */
	CPointLight();
	/// call resetLightModels.
	~CPointLight();


	/// \name Light setup
	// @{

	/// Set the position in WorldSpace.
	void			setPosition(const CVector &v) {_Position= v;}
	/// Get the position in WorldSpace.
	const CVector	&getPosition() const {return _Position;}


	/// Set the ambient color of the light. Default to Black.
	void			setAmbient (NLMISC::CRGBA ambient)	{_Ambient=ambient;}
	/// Set the diffuse color of the light. Default to White
	void			setDiffuse (NLMISC::CRGBA diffuse)	{_Diffuse=diffuse;}
	/// Set the specular color of the light. Default to White
	void			setSpecular (NLMISC::CRGBA specular)	{_Specular=specular;}
	/// Set the diffuse and specular color of the light to the same value. don't modify _Ambient.
	void			setColor (NLMISC::CRGBA color)	{_Diffuse= _Specular= color;}

	/// Get the ambient color of the light.
	NLMISC::CRGBA	getAmbient () const	{return _Ambient;}
	/// Get the diffuse color of the light.
	NLMISC::CRGBA	getDiffuse () const	{return _Diffuse;}
	/// Get the specular color of the light.
	NLMISC::CRGBA	getSpecular () const	{return _Specular;}


	/** setup the attenuation of the light. if (0,0) attenuation is disabled.
	 *	clamp(attenuationBegin,0 , +oo) and calmp(attenuationEnd, attenuationBegin, +oo)
	 */
	void			setupAttenuation(float attenuationBegin, float attenuationEnd);
	/// get the begin radius of the attenuation.
	float			getAttenuationBegin() const {return _AttenuationBegin;}
	/// get the end radius of the attenuation.
	float			getAttenuationEnd() const {return _AttenuationEnd;}


	// serial
	void			serial(NLMISC::IStream &f);


	// @}



	/// \name Render tools.
	// @{

	/// setup the CLight with current pointLight state. factor is used to modulate the colors.
	void			setupDriverLight(CLight &light, uint8 factor);

	/// Dirt all models this light influence
	void			resetLightedModels();

	/// append a model to the list. called by CLightingManager.
	ItTransformList	appendLightedModel(CTransform *model);
	/// remove a model from the list. called by CTransform.
	void			removeLightedModel(ItTransformList it);

	// @}


// ******************
private:
	// The position.
	CVector		_Position;

	// The light color.
	NLMISC::CRGBA		_Ambient;
	NLMISC::CRGBA		_Diffuse;
	NLMISC::CRGBA		_Specular;

	// Attenuation. setup / preComputed.
	float		_AttenuationBegin, _AttenuationEnd;
	float		_ConstantAttenuation;
	float		_LinearAttenuation;
	float		_QuadraticAttenuation;


	// The memory for list of LightedModels
	static	NLMISC::CBlockMemory<CTransform*, false>		_LightedModelListMemory;
	// LightedModels. NB: do not contains models that have this light in their FrozenStaticLightSetup
	TTransformList		_LightedModels;


	void		computeAttenuationFactors();

};


} // NL3D


#endif // NL_POINT_LIGHT_H

/* End of point_light.h */
