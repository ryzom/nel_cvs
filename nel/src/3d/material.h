/** \file 3d/material.h
 * <File description>
 *
 * $Id: material.h,v 1.18 2002/09/24 14:46:10 vizerie Exp $
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
#include "nel/misc/matrix.h"
#include "3d/texture.h"
#include "3d/shader.h"

#include <memory>

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
const uint32 IDRV_TOUCHED_ALPHA_TEST	=	0x00000800;
const uint32 IDRV_TOUCHED_ALPHA_TEST_THRE	=	0x00001000;


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
const uint32 IDRV_MAT_DEFMAT		=	0x00000040;	// NB: Deprecated, but may still exist streams.
const uint32 IDRV_MAT_BLEND			=	0x00000080;
const uint32 IDRV_MAT_DOUBLE_SIDED	=	0x00000100;
const uint32 IDRV_MAT_ALPHA_TEST	= 	0x00000200;
const uint32 IDRV_MAT_TEX_ADDR	    = 	0x00000400;
const uint32 IDRV_MAT_LIGHTED_VERTEX_COLOR	= 	0x00000800;
///   automatic texture coordinate generation  
const uint32 IDRV_MAT_GEN_TEX_0		= 	0x00001000;
const uint32 IDRV_MAT_GEN_TEX_1		= 	0x00002000;
const uint32 IDRV_MAT_GEN_TEX_2		= 	0x00004000;
const uint32 IDRV_MAT_GEN_TEX_3		= 	0x00008000;
const uint32 IDRV_MAT_GEN_TEX_4		= 	0x00010000;
const uint32 IDRV_MAT_GEN_TEX_5		= 	0x00020000;
const uint32 IDRV_MAT_GEN_TEX_6		= 	0x00040000;
const uint32 IDRV_MAT_GEN_TEX_7		= 	0x00080000;
///   user texture matrix
const uint32 IDRV_MAT_USER_TEX_0_MAT	= 	0x00100000;
const uint32 IDRV_MAT_USER_TEX_1_MAT	= 	0x00200000;
const uint32 IDRV_MAT_USER_TEX_2_MAT	= 	0x00400000;
const uint32 IDRV_MAT_USER_TEX_3_MAT	= 	0x00800000;
const uint32 IDRV_MAT_USER_TEX_4_MAT	= 	0x01000000;
const uint32 IDRV_MAT_USER_TEX_5_MAT	= 	0x02000000;
const uint32 IDRV_MAT_USER_TEX_6_MAT	= 	0x04000000;
const uint32 IDRV_MAT_USER_TEX_7_MAT	= 	0x08000000;
const uint32 IDRV_MAT_USER_TEX_MAT_ALL  =   0x0FF00000;

const uint32 IDRV_MAT_USER_TEX_FIRST_BIT = 20;



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

	/** Blend enums. see setSrcBlend()/setDstBlend()/setBlendFunc().
	 *	blendConstant* enums are only valid if dirver->supportBlendConstantColor().
	 *	\see IDriver::supportBlendConstantColor()
	 */
	enum TBlend				{ one=0, zero, srcalpha, invsrcalpha, srccolor, invsrccolor, 
		blendConstantColor, blendConstantInvColor, blendConstantAlpha, blendConstantInvAlpha, blendCount };

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
	 *	doesn't affect the final result (neither diffuse part nor specular part).
	 *	Blending is special. If enabled, Lightmap shader apply a standard transparency srcalpha/invsrcalpha.
	 *	- NB: if no texture in stage 0, undefined result.
	 *	- UV0 is the UV for decal Texture. UV1 is the UVs for all the lightmaps.
	 * Specular:
	 *  - Texture of stage 0 is added to the multiplication of Texture Alpha of stage 0 and Texture of stage 1
	 *  - This is done in 2 passes
	 * PerPixelLighting :
	 *  - When not supported by the driver, this is equivalent to the normal shader. This can be querried from the driver
	 *  - When supported by the driver, the strongest light is rendered using per pixel lighting. The last tex coordinate must be the S vector
	 *    of the tangent space basis (oriented in the direction where the s texture coords grows). Other lights are rendered using gouraud shaing. The light setup is done in the driver.
	 * PerPixelLighting : The same as PerPixelLighting but with no specular
	 * Caustics: NOT IMPLEMENTED	 
	 *    
	 */
	enum TShader			{ Normal=0,
							  Bump,
							  UserColor,
							  LightMap,
							  Specular,
							  Caustics,
							  PerPixelLighting,
							  PerPixelLightingNoSpec,							  
							  shaderCount};

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
	 * EMBM : apply to both color and alpha : the current texture, whose format is DSDT, is used to offset the texture in the next stage.
	 *  NB : for EMBM and InterpolateConstant, this must be supported by driver.
	 */
	enum TTexOperator		{ Replace=0, Modulate, Add, AddSigned, 
							  InterpolateTexture, InterpolatePrevious, InterpolateDiffuse, InterpolateConstant, EMBM };

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

	/** \name Texture Addressing Modes. They are valid only with the normal texture shader. 
	  *	All modes are not supported everywhere, so you should check for it in the driver.
	  * The modes are similar to those introduced with DirectX 8.0 Pixel Shaders and OpenGL
	  * TEXTURE_SHADERS_NV
	  */
	// @{
	enum TTexAddressingMode { 
							 TextureOff = 0 /* no texture */, FetchTexture, PassThrough, CullFragment,
							 OffsetTexture, OffsetTextureScale, 
							 DependentARTexture, DependentGBTexture,
							 DP3, DP3Texture2D,
							 DP3CubeMap, DP3ReflectCubeMap, DP3ConstEyeReflectCubeMap,
							 DP3DiffuseCubeMap,
							 DP3DepthReplace,
							 TexAddrCount
							};
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


	/// \name Texture Addressing Mode Method
	// @{		
	/** enable / disable the use of special texture addressing modes
	  * When enabled, all texture addressing modes are set to 'None'
	  */
	void					enableTexAddrMode(bool enable = true);

	/// test whether texture addressing mode are enabled
	bool					texAddrEnabled() const;

	/** Set a texture addressing mode for the given stage. 
	  * You should test if this mode is supported in the driver you plane to use.
	  * Texture addressing modes should have been enabled otherwise an assertion is raised
	  */
	void					setTexAddressingMode(uint8 stage, TTexAddressingMode mode);

	/// Get the texture addressing mode for the given stage
	TTexAddressingMode		getTexAddressingMode(uint8 stage);			
	// @}


	/// \name Double sided.
	// @{
	void					setDoubleSided(bool active);
	bool					getDoubleSided() const { return (_Flags&IDRV_MAT_DOUBLE_SIDED)!=0; }
	// @}

	/// \name Alpha Test
	// @{
	void					setAlphaTest(bool active);
	bool					getAlphaTest() const { return (_Flags&IDRV_MAT_ALPHA_TEST)!=0; }

	/** change the threshold against alpha is tested. Default is 0.5f.
	 *	\param thre threshold, should be in [0..1], not clamped.
	 */
	void					setAlphaTestThreshold(float thre);
	float					getAlphaTestThreshold() const { return _AlphaTestThreshold; }

	// @}


	/// \name ZBuffer.
	// @{
	void					setZFunc(ZFunc val);
	void					setZWrite(bool active);

	/// The z bias is a z dispacement of the object to solve z precision problem. The bias is gived in world coordinate units.
	/// Positive bias give a lower z priority, negative bias give a higher bias priority.
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
	void					setLighting(	bool active, 
											CRGBA emissive=CRGBA(0,0,0), 
											CRGBA ambient=CRGBA(0,0,0), 
											CRGBA diffuse=CRGBA(0,0,0), 
											CRGBA specular=CRGBA(0,0,0),
											float shininess= 10);

	/// Set the emissive part ot material. Usefull only if setLighting(true) has been done.
	void					setEmissive( CRGBA emissive=CRGBA(0,0,0) );
	/// Set the Ambient part ot material. Usefull only if setLighting(true) has been done.
	void					setAmbient( CRGBA ambient=CRGBA(0,0,0) );
	/// Set the Diffuse part ot material. Usefull only if setLighting(true) has been done. NB: opacity is NOT copied from diffuse.A.
	void					setDiffuse( CRGBA diffuse=CRGBA(0,0,0) );
	/// Set the Opacity part ot material. Usefull only if setLighting(true) has been done.
	void					setOpacity( uint8	opa );
	/// Set the specular part ot material. Usefull only if setLighting(true) has been done.
	void					setSpecular( CRGBA specular=CRGBA(0,0,0) );
	/// Set the shininess part ot material. Usefull only if setLighting(true) has been done.
	void					setShininess( float shininess );
	/// Set the color material flag. Used when the material is lighted. True to use the diffuse color of the material when lighted, false to use the color vertex.
	void					setLightedVertexColor (bool useLightedVertexColor);
	/// Get the lighted vertex color flag
	bool					getLightedVertexColor () const;


	bool					isLighted() const {return (_Flags&IDRV_MAT_LIGHTING)!=0;}

	/// Return true if this material uses color material as diffuse when lighted, else return false if it uses color vertex.
	bool					isLightedVertexColor () const { return (_Flags&IDRV_MAT_LIGHTED_VERTEX_COLOR)!=0;}

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

	void					setTexCoordGen(uint stage, bool generate);
	bool					getTexCoordGen(uint stage) const;

	// Enable a user texture matrix for the n-th stage. The initial matrix is set to identity.
	void                    enableUserTexMat(uint stage, bool enabled = true);
	// Test wether a user texture is enabled for the n-th stage
	bool                    isUserTexMatEnabled(uint stage) const;
	/// Set a new texture matrix for the given stage.	
	void					setUserTexMat(uint stage, const NLMISC::CMatrix &m);
	/** Get a const ref. on the texture matrix of the n-th stage.
	  * User texture matrix must be enabled for that stage, otherwise an assertion is raised.
	  */
	const NLMISC::CMatrix  &getUserTexMat(uint stage) const;
	/// Decompose a user texture matrix, We assume that this is only a matrix for 2d texture.
	void					decompUserTexMat(uint stage, float &uTrans, float &vTrans, float &wRot, float &uScale, float &vScale);
	  
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
	void					setLightMapFactor(uint lmapId, CRGBA factor);
	// @}


	/// \name Tools..
	// @{
	/** Init the material as unlit. normal shader, no lighting ....
	 * Default to: normal shader, no lighting, color to White(1,1,1,1), no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initUnlit();
	/** Init the material as default white lighted material. normal shader, lighting ....
	 * Default to: normal shader, full black lighting, no texture, ZBias=0, ZFunc= lessequal, ZWrite==true, no blend.
	 * All other states are undefined (such as blend function, since blend is disabled).
	 */
	void					initLighted();
	// @}

	// \todo mb Clean this flag
	bool					getStainedGlassWindow() { return _StainedGlassWindow; }
	void					setStainedGlassWindow(bool val) { _StainedGlassWindow = val; }
	
	/// Flush textures. Force texture generation.
	void					flushTextures (IDriver &driver);

	void		serial(NLMISC::IStream &f);

	// \name Multiple texture set managment
	// @{
		/** Select one texture set for all the textures of this material.
		  * This is useful only if textures of this material support selection of course (such as CTextureMultiFile)
		  */
		void		selectTextureSet(uint index);
	// @}

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
				uint32		OpRGB:4;
				uint32		SrcArg0RGB:2;
				uint32		OpArg0RGB:2;
				uint32		SrcArg1RGB:2;
				uint32		OpArg1RGB:2;

				uint32		OpAlpha:4;
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
	CRGBA					_Emissive, _Ambient, _Diffuse, _Specular;
	float					_Shininess;
	float					_AlphaTestThreshold;
	uint32					_Touched;
	bool					_StainedGlassWindow; // \todo mb : clean this flag (add a CMaterialBuil class)
	struct	CUserTexMat
	{
		NLMISC::CMatrix		TexMat[IDRV_MAT_MAXTEXTURES];		
	};
	std::auto_ptr<CUserTexMat>	_TexUserMat;		 // user texture matrix
	CSmartPtr<ITexture>		_Textures[IDRV_MAT_MAXTEXTURES];

public:
	// Private. For Driver only.
	uint8				    _TexAddrMode[IDRV_MAT_MAXTEXTURES]; // texture addressing enum packed as bytes
	CTexEnv					_TexEnvs[IDRV_MAT_MAXTEXTURES];
	CRefPtr<IShader>		pShader;

	// Private. For Driver only. LightMaps.
	struct	CLightMap
	{
		CSmartPtr<ITexture>		Texture;
		CRGBA					Factor;
		CLightMap()
		{
			Factor.set(255, 255, 255, 255);
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

#include "3d/driver_material_inline.h"

#endif // NL_MATERIAL_H

/* End of material.h */
