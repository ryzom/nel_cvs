/** \file material.h
 * <File description>
 *
 * $Id: material.h,v 1.4 2001/01/05 10:57:30 berenguier Exp $
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

const uint32 IDRV_TOUCHED_BLENDFUNC	=	0x00000001;
const uint32 IDRV_TOUCHED_BLEND		=	0x00000002;
const uint32 IDRV_TOUCHED_SHADER	=	0x00000004;
const uint32 IDRV_TOUCHED_ZFUNC		=	0x00000008;
const uint32 IDRV_TOUCHED_ZBIAS		=	0x00000010;
const uint32 IDRV_TOUCHED_COLOR		=	0x00000020;
const uint32 IDRV_TOUCHED_LIGHTING	=	0x00000040;
const uint32 IDRV_TOUCHED_DEFMAT	=	0x00000080;
const uint32 IDRV_TOUCHED_ZWRITE	=	0x00000100;


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

/**
 * <Class description>
 *
 * *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CMaterial : public CRefCount
{
public:

	enum ZFunc				{ always,never,equal,notequal,less,lessequal,greater,greaterequal };
	enum TBlend				{ one, zero, srcalpha, invsrcalpha };
	enum TShader			{ normal, user_color, envmap, bump};

public:
	/// \name Object.
	// @{
	/// ctor.
	CMaterial() {_Touched= 0;_Flags=0;}
	/// see operator=.
	CMaterial(const CMaterial &mat) {_Touched= 0;_Flags=0; operator=(mat);}
	/// dtor.
	~CMaterial();
	/// Do not copy DrvInfos, copy all infos and set IDRV_TOUCHED_ALL.
	CMaterial				&operator=(const CMaterial &mat);
	// @}

	/// Set the shader for this material.
	void					setShader(TShader val);

	/// \name Texture.
	// @{
	void 					setTexture(ITexture* ptex, uint8 n=0);

	ITexture*				getTexture(uint8 n);
	bool					texturePresent(uint8 n);
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
	void					setLighting(	bool active, bool DefMat=true,
											CRGBA emissive=CRGBA(0,0,0), 
											CRGBA ambient=CRGBA(0,0,0), 
											CRGBA diffuse=CRGBA(0,0,0), 
											CRGBA specular=CRGBA(0,0,0) );

	CRGBA					getColor(void) const { return(_Color); }
	CRGBA					getEmissive() const { return _Emissive;}
	CRGBA					getAmbient() const { return _Ambient;}
	CRGBA					getDiffuse() const { return _Diffuse;}
	CRGBA					getSpecular() const { return _Specular;}
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
private:

	TShader					_ShaderType;
	uint32					_Flags;
	TBlend					_SrcBlend,_DstBlend;
	ZFunc					_ZFunction;
	float					_ZBias;
	CRGBA					_Color;
	CRGBA					_Emissive,_Ambient,_Diffuse,_Specular;
	uint32					_Touched;

	CSmartPtr<ITexture>		_Textures[IDRV_MAT_MAXTEXTURES];

public:
	// Private. For Driver only.
	CRefPtr<IShader>		pShader;

	uint32					getFlags() const {return _Flags;}
	uint32					getTouched(void)  const { return(_Touched); }
	void					clearTouched(uint32 flag) { _Touched&=~flag; }

};

} // NL3D

#include "nel/3d/driver_material_inline.h"

#endif // NL_MATERIAL_H

/* End of material.h */
