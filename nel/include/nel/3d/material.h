/** \file material.h
 * <File description>
 *
 * $Id: material.h,v 1.17 2001/05/30 16:40:53 berenguier Exp $
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

#ifndef NL_MATERIAL_H
#define NL_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/3d/texture.h"
#include "nel/3d/shader.h"

namespace NL3D {

using NLMISC::CRefCount;
using NLMISC::CRGBA;
using NLMISC::CSmartPtr;
using NLMISC::CRefPtr;

// --------------------------------------------------

const uint32 IDRV_MAT_MAXTEXTURES	=	4;

const uint32 IDRV_TOUCHED_BLENDFUNC	=		0x00000001;
const uint32 IDRV_TOUCHED_BLEND			=	0x00000002;
const uint32 IDRV_TOUCHED_SHADER		=	0x00000004;
const uint32 IDRV_TOUCHED_ZFUNC			=	0x00000008;
const uint32 IDRV_TOUCHED_ZBIAS			=	0x00000010;
const uint32 IDRV_TOUCHED_COLOR			=	0x00000020;
const uint32 IDRV_TOUCHED_LIGHTING		=	0x00000040;
const uint32 IDRV_TOUCHED_DEFMAT		=	0x00000080;
const uint32 IDRV_TOUCHED_ZWRITE		=	0x00000100;
const uint32 IDRV_TOUCHED_DOUBLE_SIDED	=	0x00000200;
const uint32 IDRV_TOUCHED_LIGHTMAP		=	0x00000400;

// Start texture touch at 0x10000.
const uint32 IDRV_TOUCHED_TEX[IDRV_MAT_MAXTEXTURES]		=
	{0x00010000, 0x00020000, 0x00040000, 0x00080000};
const uint32 IDRV_TOUCHED_ALL		=	0xFFFFFFFF;


const uint32 IDRV_MAT_HIDE			=	0x00000001;
const uint32 IDRV_MAT_TSP			=	0x00000002;
const uint32 IDRV_MAT_ZWRITE		=	0x00000004;
const uint32 IDRV_MAT_ZLIST			=	0x00000008;
const uint32 IDRV_MAT_LIGHTING		=	0x00000010;
const uint32 IDRV_MAT_SPECULAR		=	0x00000020;
const uint32 IDRV_MAT_DEFMAT		=	0x00000040;
const uint32 IDRV_MAT_BLEND			=	0x00000080;
const uint32 IDRV_MAT_DOUBLE_SIDED	=	0x00000100;

// ***************************************************************************
/**
 * A material represent ALL the states relatives to the aspect of a primitive.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CMaterial : public CRefCount
{
public:

	enum ZFunc				{ always=0,never,equal,notequal,less,lessequal,greater,greaterequal, zfuncCount };
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor, blendCount };
	/**
	 * Normal shader:
	 *	- use simple multitexturing. see texEnv*() methods.
	 * Bump:
	 *	- not implemented yet.
	 * UserColor:
	 *	- UserColor (see setUserColor()) is blended with precomputed texture/textureAlpha.
	 *	- Alpha Blending ignore Alpha of texture (of course :) ), but use Alpha diffuse (vertex/material color).
	 * LightMap:
	 *	- Texture of stage 0 is blended with sum of lightmaps (see setLightmap()). Vertex Color (or color, or lighting)
	 *	doesn't affect the final result (neither diffuse part nor specular part). No Blending is applied too.
	 *	- NB: if no texture in stage 0, undefined result.
	 *	- UV0 is the UV for decal Texture. UV1 is the UVs for all the lightmaps.
	 */
	enum TShader			{ Normal=0, Bump, UserColor, LightMap, shaderCount};

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
	 */
	enum TTexOperator		{ Replace=0, Modulate, Add, AddSigned, 
		InterpolateTexture, InterpolatePrevious, InterpolateDiffuse, InterpolateConstant };

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
	/// \name Object.
	// @{
	/** ctor.
	 * By default, shader is normal, SrcBlend is srcalpha, dstblend is invsrcalpha, ZFunction is lessequal, ZBias is 0,
	 * Color is White: (255,255,255,255), not double sided.
	 */
	CMaterial();
	/// see operator=.
	CMaterial(const CMaterial &mat) {_Touched= 0;_Flags=0; operator=(mat);}
	/// dtor.
	~CMaterial();
	/// Do not copy DrvInfos, copy all infos and set IDRV_TOUCHED_ALL.
	CMaterial				&operator=(const CMaterial &mat);
	// @}

	/** Set the shader for this material.
	 * All textures are reseted!!
	 */
	void					setShader(TShader val);
	/// get the current material shadertype.
	TShader					getShader() const {return _ShaderType;}

	/// \name Texture.
	// @{
	/**
	 * set a texture for a special stage. Different usage according to shader:
	 *	- Normal shader do multitexturing (see texEnv*() methods).
	 *	- UserColor assert if stage!=0. (NB: internal only: UserColor setup texture to stage 0 and 1).
	 *	- LightMap assert if stage!=0.
	 */ 
	void 					setTexture(uint8 stage, ITexture* ptex);

	ITexture*				getTexture(uint8 stage) const;
	bool					texturePresent(uint8 stage) const;
	// @}


	/// \name Blending.
	// @{
	void					setBlend(bool active);
	void					setBlendFunc(TBlend src, TBlend dst);
	void					setSrcBlend(TBlend val);
	void					setDstBlend(TBlend val);

	bool					getBlend() const { return (_Flags&IDRV_MAT_BLEND)!=0; }
	TBlend					getSrcBlend(void)  const { return(_SrcBlend); }
	TBlend					getDstBlend(void)  const { return(_DstBlend); }
	// @}


	/// \name Double sided.
	// @{
	void					setDoubleSided(bool active);

	bool					detDoubleSided() const { return (_Flags&IDRV_MAT_DOUBLE_SIDED)!=0; }
	// @}


	/// \name ZBuffer.
	// @{
	void					setZFunc(ZFunc val);
	void					setZWrite(bool active);
	void					setZBias(float val);

	ZFunc					getZFunc(void)  const { return(_ZFunction); }		
	bool					getZWrite(void)  const{ return (_Flags&IDRV_MAT_ZWRITE)!=0; }
	float					getZBias(void)  const { return(_ZBias); }
	// @}


	/// \name Color/Lighting..
	// @{
	/// The Color is used only if lighting is disabled. Also, color is replaced by per vertex color (if any).
	void					setColor(CRGBA rgba);

	/// Batch setup lighting. Opacity is in diffuse.A.
	void					setLighting(	bool active, bool DefMat=true,
											CRGBA emissive=CRGBA(0,0,0), 
											CRGBA ambient=CRGBA(0,0,0), 
											CRGBA diffuse=CRGBA(0,0,0), 
											CRGBA specular=CRGBA(0,0,0),
											float shininess= 10);

	/// Set the emissive part ot material. Usefull only if setLighting(true, false) has been done.
	void					setEmissive( CRGBA emissive=CRGBA(0,0,0) );
	/// Set the Ambient part ot material. Usefull only if setLighting(true, false) has been done.
	void					setAmbient( CRGBA ambient=CRGBA(0,0,0) );
	/// Set the Diffuse part ot material. Usefull only if setLighting(true, false) has been done. NB: opacity is NOT copied from diffuse.A.
	void					setDiffuse( CRGBA diffuse=CRGBA(0,0,0) );
	/// Set the Opacity part ot material. Usefull only if setLighting(true, false) has been done.
	void					setOpacity( uint8	opa );
	/// Set the specular part ot material. Usefull only if setLighting(true, false) has been done.
	void					setSpecular( CRGBA specular=CRGBA(0,0,0) );
	/// Set the shininess part ot material. Usefull only if setLighting(true, false) has been done.
	void					setShininess( float shininess );


	bool					isLighted() const {return (_Flags&IDRV_MAT_LIGHTING)!=0;}

	CRGBA					getColor(void) const { return(_Color); }
	CRGBA					getEmissive() const { return _Emissive;}
	CRGBA					getAmbient() const { return _Ambient;}
	/// return diffuse part. NB: A==opacity.
	CRGBA					getDiffuse() const { return _Diffuse;}
	uint8					getOpacity() const { return _Diffuse.A;}
	CRGBA					getSpecular() const { return _Specular;}
	float					getShininess() const { return _Shininess;}
	// @}


	/// \name Texture environnement. Normal shader only.
	/** This part is valid for Normal shaders (nlassert). It maps the EXT_texture_env_combine opengl extension.
	 * A stage is enabled iff his texture is !=NULL. By default, all stages are setup to Modulate style:
	 *  AlphaOp=RGBOp= Modulate, RGBArg0= TextureSrcColor, RGBArg1= PreviousSrcColor,
	 *  AlphaArg0= TextureSrcAlpha, AlphaArg1= PreviousSrcAlpha.  ConstantColor default to White(255,255,255,255).
	 *
	 * For compatibility problems:
	 * - no scaling is allowed (some cards do not implement this well).
	 * - Texture can be the source only for Arg0 (DirectX restriction). nlassert...
	 *
	 * NB: for Alpha Aguments, only operands SrcAlpha and InvSrcAlpha are valid (nlassert..).
	 */
	// @{
	void					texEnvOpRGB(uint stage, TTexOperator ope);
	void					texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvOpAlpha(uint stage, TTexOperator ope);
	void					texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper);
	void					texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper);
	/// Setup the constant color for a stage. Used for the TTexSource:Constant.
	void					texConstantColor(uint stage, CRGBA color);
	/// For push/pop only, get the packed version of the environnment mode.
	uint32					getTexEnvMode(uint stage);
	/// For push/pop only, set the packed version of the environnment mode.
	void					setTexEnvMode(uint stage, uint32 packed);
	CRGBA					getTexConstantColor(uint stage);
	// @}


	/// \name Texture UserColor. UserColor shader only.
	/** This part is valid for Normal shaders (nlassert).
	 * \see TShader.
	 */
	// @{
	void					setUserColor(CRGBA userColor);
	CRGBA					getUserColor() const;
	// @}

	/// \name LightMap. LightMap shader only.
	/** This part is valid for LightMap shaders (nlassert).
	 * \see TShader.
	 */
	// @{
	/// Set the ith lightmap. undef results if holes in lightmap array.
	void					setLightMap(uint lmapId, ITexture *lmap);
	/// Get the ith lightmap. (NULl if none)
	ITexture				*getLightMap(uint lmapId) const;
	/// Set the lightmap intensity. (default to 255).
	void					setLightMapFactor(uint lmapId, uint8 factor);
	// @}


	/// \name Tools..
	// @{
	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initUnlit();
	/** Init the material as default white lighted material. normal shader, lighting ....
	 * Default to: normal shader, lighting to default material, no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initLighted();
	// @}


	void		serial(NLMISC::IStream &f);


// **********************************
// Private part.
public:
	// Private. For Driver only.
	struct CTexEnv
	{
		union
		{
			uint32	EnvPacked;
			struct
			{
				uint32		OpRGB:3;
				uint32		SrcArg0RGB:2;
				uint32		OpArg0RGB:2;
				uint32		SrcArg1RGB:2;
				uint32		OpArg1RGB:2;

				uint32		OpAlpha:3;
				uint32		SrcArg0Alpha:2;
				uint32		OpArg0Alpha:2;
				uint32		SrcArg1Alpha:2;
				uint32		OpArg1Alpha:2;
			}		Env;
		};
		CRGBA		ConstantColor;

		void		setDefault()
		{
			// Don't worry, Visual optimize it quite well...
			// We cannot do better, because bit fields ordeinrg seems not to be standardized, so we can not 
			// set Packed directly.
			Env.OpRGB= Modulate;
			Env.SrcArg0RGB= Texture;
			Env.OpArg0RGB= SrcColor;
			Env.SrcArg1RGB= Previous;
			Env.OpArg1RGB= SrcColor;

			Env.OpAlpha= Modulate;
			Env.SrcArg0Alpha= Texture;
			Env.OpArg0Alpha= SrcAlpha;
			Env.SrcArg1Alpha= Previous;
			Env.OpArg1Alpha= SrcAlpha;

			ConstantColor.set(255,255,255,255);
		}

		void		serial(NLMISC::IStream &f)
		{
			Env.OpRGB= f.serialBitField8(Env.OpRGB);
			Env.SrcArg0RGB= f.serialBitField8(Env.SrcArg0RGB);
			Env.OpArg0RGB= f.serialBitField8(Env.OpArg0RGB);
			Env.SrcArg1RGB= f.serialBitField8(Env.SrcArg1RGB);
			Env.OpArg1RGB= f.serialBitField8(Env.OpArg1RGB);

			Env.OpAlpha= f.serialBitField8(Env.OpAlpha);
			Env.SrcArg0Alpha= f.serialBitField8(Env.SrcArg0Alpha);
			Env.OpArg0Alpha= f.serialBitField8(Env.OpArg0Alpha);
			Env.SrcArg1Alpha= f.serialBitField8(Env.SrcArg1Alpha);
			Env.OpArg1Alpha= f.serialBitField8(Env.OpArg1Alpha);

			f.serial(ConstantColor);
		}


		CTexEnv()
		{
			setDefault();
		}
	};

private:

	TShader					_ShaderType;
	uint32					_Flags;
	TBlend					_SrcBlend,_DstBlend;
	ZFunc					_ZFunction;
	float					_ZBias;
	CRGBA					_Color;
	CRGBA					_Emissive,_Ambient,_Diffuse,_Specular;
	float					_Shininess;
	uint32					_Touched;

	CSmartPtr<ITexture>		_Textures[IDRV_MAT_MAXTEXTURES];

public:
	// Private. For Driver only.
	CTexEnv					_TexEnvs[IDRV_MAT_MAXTEXTURES];
	CRefPtr<IShader>		pShader;

	// Private. For Driver only. LightMaps.
	struct	CLightMap
	{
		CSmartPtr<ITexture>		Texture;
		uint8					Factor;
		CLightMap()
		{
			Factor= 255;
		}

		void	serial(NLMISC::IStream &f);
	};
	typedef	std::vector<CLightMap>	TTexturePtrs;
	TTexturePtrs			_LightMaps;
	

	uint32					getFlags() const {return _Flags;}
	uint32					getTouched(void)  const { return(_Touched); }
	void					clearTouched(uint32 flag) { _Touched&=~flag; }

};

} // NL3D

#include "nel/3d/driver_material_inline.h"

#endif // NL_MATERIAL_H

/* End of material.h */
