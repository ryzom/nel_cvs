/** \file water_shape.h
 * <File description>
 *
 * $Id: water_shape.h,v 1.14 2004/03/19 10:11:36 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_WATER_SHAPE_H
#define NL_WATER_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/class_id.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/polygon.h"
#include "3d/track.h"
#include "nel/3d/animation_time.h"







#include "3d/shape.h"
#include "3d/vertex_buffer.h"
#include "3d/driver.h"

#include <memory>

namespace NL3D {

// defines
const CVertexBuffer::TValue   WATER_VB_POS  = CVertexBuffer::Position;
const CVertexBuffer::TValue   WATER_VB_DX   = CVertexBuffer::TexCoord0;



// class id for water
const NLMISC::CClassId WaterModelClassId =  NLMISC::CClassId(0x41a0732e, 0x6c664506);

// class id for wave maker
const NLMISC::CClassId WaveMakerModelClassId =  NLMISC::CClassId(0x16da3356, 0x7dec65fd);

/**
 * A water shape.
 * On recent gfx boards with 4 texture stages, it uses the following :
 * -A environment map computed from the hemisphere that is above the water.
 * -A bump map used to perturbate the envmap (bump map 1).
 * -A bump map used to perturbate the bump map 1 (bump map 0). This simulate local water motion
 * -A color map whose both alpha and rgb are modulated with the envmap to get transparency and color of the water.
 * These maps can be scaled, but not rotated. The bump maps can scroll at regular speed over time
 * The envmap, when set to a CTextureBlend, will automatically blend for all shape when CWaterPoolManager::setBlend is called.
 * This may be used to simulate a night / day transition.
 * The color / alpha map may have arbritrary orientation over the surface
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CWaterShape : public IShape
{
public:
	NLMISC_DECLARE_CLASS(CWaterShape);

	///\name Object
	//@{
	/// ctor
	CWaterShape();

	/// dtor
	~CWaterShape();


	/// serial this shape
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}


	/// inherited from IShape
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// inherited from IShape
	virtual bool				clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix);

	/// inherited from IShape. Does nothing. A new traverseRender() was set for that
	virtual void				render(IDriver *drv, CTransformShape *trans, bool opaquePass) {}

	/// inherited from IShape
	virtual	void				getAABBox(NLMISC::CAABBox &bbox) const { bbox = _BBox; }

	/// inherited from ishape
	virtual float				getNumTriangles (float distance);

	/// inherited from ishape
	virtual void				flushTextures (IDriver &driver, uint selectedTexture);


	///\name Geometry setup
	//@{
		/// all water shape that have the same weter id will share the same height map for wave propagations
		void			setWaterPoolID(uint32 id) { _WaterPoolID = id; }		
		uint32			getWaterPoolID() const    { return _WaterPoolID; }
		/// set the resolution for the grid that is used for tesselation
		static void		setScreenGridSize(uint32 x, uint32 y);
		/// set the size of borders. This is needed when water move is high, to avoid hole on the border of the screen
		static void		setGridBorderSize(uint32 x, uint32 y);		
		static uint32	getScreenXGridSize()  { return _XScreenGridSize; }
		static uint32	getScreenYGridSize()  { return _YScreenGridSize; }
		static uint32	getXGridBorder()  { return _XGridBorder; }
		static uint32	getYGridBorder()  { return _YGridBorder; }

	
		// set a polygon that represent this shape. It must be a 2d polygon, with z kzpt to 0 everywhere
		void					setShape(const NLMISC::CPolygon2D &poly);

		/// get the polygon used by this shape, in the object space
		const NLMISC::CPolygon2D &getShape() const { return _Poly; }

		/// get the polygon this shape, in world space
		void getShapeInWorldSpace(NLMISC::CPolygon &poly) const;

		/// Set a factor that is applied to waves height when they are displayed. default is 1
		void	setWaveHeightFactor(float f) { _WaveHeightFactor = f; }

		/// Get the factor that is applied to waves height when they are displayed. default is 1
		float	getWaveHeightFactor() const { return _WaveHeightFactor; }

	//@}


	///\name Texture setup
	//@{
		 // set the environment reflected by water. Index should be 0 for the above texture and 1 for the below texture
		void				setEnvMap(uint index, ITexture *envMap);
		ITexture			*getEnvMap(uint index) { nlassert(index < 2); return (ITexture *) _EnvMap[index];}
		const ITexture		*getEnvMap(uint index) const { nlassert(index < 2); return (const ITexture *) _EnvMap[index];}

		/** Set a height map, used to generate a bumpmap (useful if supported by hardware...)
		  * NB : not to be confused with the height map used to modify the geometry, it  only modify texture
		  */
		void				setHeightMap(uint k, ITexture *hm);
		ITexture			*getHeightMap(uint k);
		const ITexture		*getHeightMap(uint k) const;
		void				setHeightMapScale(uint k, const NLMISC::CVector2f &scale);
		NLMISC::CVector2f	getHeightMapScale(uint k) const;
		void			    setHeightMapSpeed(uint k, const NLMISC::CVector2f &speed);	// speed given in texture units
		NLMISC::CVector2f   getHeightMapSpeed(uint k) const;	



		// set a color map
		void				setColorMap(ITexture *map);
		ITexture			*getColorMap() { return _ColorMap; }
		const ITexture		*getColorMap() const { return _ColorMap; }
		// set A 2x3 matrix used to compute position in colormap, from the x and y coordinates in world space
		void				setColorMapMat(const NLMISC::CVector2f &column0, const NLMISC::CVector2f &column1, const NLMISC::CVector2f &pos);
		void				getColorMapMat(NLMISC::CVector2f &column0, NLMISC::CVector2f &column1, NLMISC::CVector2f &pos);
	//@}

	///\name LOD
	//@{
		void	setTransitionRatio(float percent) { _TransitionRatio = percent; }		
		float   getTransitionRatio() const		   {  return _TransitionRatio; }		
	//@}

	/// \name access default tracks.
	// @{
	CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
	CTrackDefaultVector*	getDefaultScale ()		{return &_DefaultScale;}
	CTrackDefaultQuat*		getDefaultRotQuat ()	{return &_DefaultRotQuat;}

	// @}

	/// \name Lighmap
	// @{
		/// Tells this shape that it can replace its colormap by a lightmap during the zone lighting
	void				enableLightMapping(bool enable = true) { _ComputeLightmap = enable; }
	bool				isLightMappingEnabled() const  { return _ComputeLightmap; }	

private:
	friend class	CWaterModel;	
	void								computeBBox();
	void								envMapUpdate();
	void								updateHeightMapNormalizationFactors();
	static void							initVertexProgram();
	static void							setupVertexBuffer();	
private:
	NLMISC::CAABBox						_BBox;	// computed from the poly
	NLMISC::CPolygon2D					_Poly;
	uint32								_WaterPoolID;	
	NLMISC::CSmartPtr<ITexture>			_EnvMap[2];	
	NLMISC::CSmartPtr<ITexture>			_BumpMap[2];	
	NLMISC::CSmartPtr<ITexture>			_ColorMap;

	NLMISC::CVector2f					_HeightMapScale[2];		
	NLMISC::CVector2f					_HeightMapSpeed[2];		
	NLMISC::CVector2f					_ColorMapMatColumn0, _ColorMapMatColumn1, _ColorMapMatPos;	

	CTrackDefaultVector					_DefaultPos;
	CTrackDefaultVector					_DefaultScale;
	CTrackDefaultQuat					_DefaultRotQuat;
	float								_TransitionRatio;	
	float								_WaveHeightFactor;
	bool								_ComputeLightmap;
	bool								_HeightMapTouch[2];
	float								_HeightMapNormalizationFactor[2];	
		
	static uint32							_XScreenGridSize; // size with y rotation = 0
	static uint32							_YScreenGridSize;
	static uint32							_MaxGridSize; // size with max rotation around y (without borders)
	static uint32							_XGridBorder;
	static uint32							_YGridBorder;

	static CVertexBuffer					_VB;
	static CIndexBuffer						_IBUpDown;
	static CIndexBuffer						_IBDownUp;	
	static bool								_GridSizeTouched;

	//
	static std::auto_ptr<CVertexProgram>	_VertexProgramBump1;
	static std::auto_ptr<CVertexProgram>	_VertexProgramBump2;
	//
	static std::auto_ptr<CVertexProgram>	_VertexProgramBump1Diffuse;
	static std::auto_ptr<CVertexProgram>	_VertexProgramBump2Diffuse;
	//
	static std::auto_ptr<CVertexProgram>	_VertexProgramNoBump;
	static std::auto_ptr<CVertexProgram>	_VertexProgramNoBumpDiffuse;
};


/// A wave maker. It can generate waves where it is located
class CWaveMakerShape : public IShape
{
public:	
	NLMISC_DECLARE_CLASS(CWaveMakerShape);
	///\name Object
	//@{
		/// ctor
		CWaveMakerShape();

		/// dtor
		~CWaveMakerShape();


		/// serial this shape
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}


	/// inherited from IShape
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// inherited from IShape
	virtual bool				clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix);

	/// inherited from IShape. Does nothing. A new traverseRender() was set for that
	virtual void				render(IDriver *drv, CTransformShape *trans, bool opaquePass) {}

	/// inherited from IShape
	virtual	void				getAABBox(NLMISC::CAABBox &bbox) const;

	/// inherited from ishape
	virtual float				getNumTriangles (float distance) { return 0; }

	/// inherited from ishape
	virtual void				flushTextures (IDriver &driver, uint selectedTexture) {}


	/// set the period for this wave maker
	void			setPeriod(TAnimationTime period) { _Period = period; }
	TAnimationTime  getPeriod() const { return _Period; }

	/// set the intensity of the waves
	void			setIntensity(float intensity) { _Intensity = intensity; }
	float			getIntensity() const { return _Intensity; }

	/// radius of the impulsion
	void            setRadius(float radius) { _Radius = radius; }
	float			getRadius() const { return _Radius; }

	/// set the water pool that's being modified
	void			setWaterPoolID(uint32 id) { _PoolID = id; }
	uint32			getWaterPoolID() const	  { return _PoolID; }

	/// use a periodic impulsion rather than a sinus
	void			setImpulsionMode(bool on = true) { _ImpulsionMode = on; }
	bool			getImpulsionMode() const { return _ImpulsionMode; }

	CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
private:
	friend	class	CWaveMakerModel;
	TAnimationTime						_Period;
	float								_Radius;
	uint32								_PoolID;
	float								_Intensity;
	bool								_ImpulsionMode;
	CTrackDefaultVector					_DefaultPos;
};


} // NL3D


#endif // NL_WATER_SHAPE_H

/* End of water_shape.h */
