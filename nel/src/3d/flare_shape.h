/** \file flare_shape.h
 * <File description>
 *
 * $Id: flare_shape.h,v 1.4 2001/07/26 17:16:59 vizerie Exp $
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

#ifndef NL_FLARE_SHAPE_H
#define NL_FLARE_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include "nel/3d/animation_time.h"
#include "3d/shape.h"
#include "3d/texture.h"



namespace NL3D {


// class id for flares
const NLMISC::CClassId FlareModelClassId =  NLMISC::CClassId(0x6d674c32, 0x53b961a0) ;

// max number of flares
const uint MaxFlareNum= 8 ;

/**
 * shape for a flare
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CFlareShape : public IShape
{
public:
	NLMISC_DECLARE_CLASS(CFlareShape) ;

	///\name Object
		//{@
		/// Constructor
		CFlareShape();

		/// serial this shape
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
		//@}


	/// inherited from IShape
	virtual	CTransformShape		*createInstance(CScene &scene) ;

	/// inherited from IShape
	virtual bool				clip(const std::vector<CPlane>	&pyramid) ;

	/// inherited from IShape. Does nothing. A new observer was set for that
	virtual void				render(IDriver *drv, CTransformShape *trans, bool opaquePass) {}

	/// inherited from IShape
	virtual	void				getAABBox(NLMISC::CAABBox &bbox) const ;

	/// inherited from ishape
	virtual float				getNumTriangles (float distance) ;


	/** set a texture for the flare
	  * \param index the index of the flare to set. Vaklue ranges from 0 to MaxFlareNum - 1
	  * \param tex the texture to set. NULL removes the texture
	  */
	void						setTexture(uint index, ITexture *tex) 
	{ 
		nlassert(index < MaxFlareNum) ;
		_Tex[index] = tex ; 
	}

	/** get the nth texture used by the flare.
	  *  \param index the index of the flare to set. Value ranges from 0 to MaxFlareNum - 1
	  */
	ITexture					*getTexture(uint index) 
	{ 
		nlassert(index < MaxFlareNum) ;
		return _Tex[index] ;
	}

	/// get the texture used by the flare (const version)
	const ITexture				*getTexture(uint index) const 
	{ 
		nlassert(index < MaxFlareNum) ;
		return _Tex[index] ; 
	}

	/** set the size of the nth flare flare
	  * \param index the index of the flare to set. Value ranges from 0 to MaxFlareNum - 1
	  */
	void						setSize(uint index, float size) 
	{ 
		nlassert(index < MaxFlareNum) ;
		_Size[index]  = size ; 
	}

	/** get the size of the nth flare
	  * \param index the index of the flare to set. Value ranges from 0 to MaxFlareNum - 1
	  */
	float						getSize(uint index) const 
	{ 
		return _Size[index] ; 
	}

	/// set the color of flares
	void						setColor(NLMISC::CRGBA col) 
	{ 		
		_Color = col ; 
	}

	/// get the color of flares
	NLMISC::CRGBA				getColor(void) const 
	{ 
		return _Color ; 
	}

	/// set the flares spacing 
	void						setFlareSpacing(float spacing)
	{
		_Spacing = spacing ;
	}

	/** Get the flares spacing : A spacing of 1.f means thta the last flare will reach the center of the screen
	  * , a spacing of 0.5f means only the half way to the middle of the screen will be reached
	  */
	float						getFlareSpacing(void) const
	{
		return _Spacing ;
	}

	/// set the persistence of this shape, in second (the time it takes to fade from white to black)
	void						setPersistence(CAnimationTime persistence) 
	{ 	
		_Persistence = persistence ; 
	}

	/** get the persistence of this shape
	  * \see setPersistence
	  */
	CAnimationTime				getPersistence(void) const 
	{ 
		return _Persistence ; 
	}


	/// force radial attenuation of the flares
	void		setAttenuable(bool enable = true)	{ _Attenuable = enable ; }

	/// check wether radial :attenuation is on
	bool		getAttenuable(void) const			{ return _Attenuable ;   }

	/// set the range for attenuation
	void		setAttenuationRange(float range)    { _AttenuationRange = range ; }

	/// get the attenuation rnage
	float		getAttenuationRange(void) const		{ return _AttenuationRange ; }


	/// force the first flare to keep its real size (e.g the isze on screen doesn't remains constant)
	void		setFirstFlareKeepSize(bool enable = true) { _FirstFlareKeepSize = enable ; }

	/// test wether the first flare keep its real size
	bool		getFirstFlareKeepSize(void) const		  { return _FirstFlareKeepSize ; }


protected:
	friend class CFlareModel ;	
	NLMISC::CSmartPtr<ITexture> _Tex[MaxFlareNum] ;
	NLMISC::CRGBA				_Color ;
	float						_Size[MaxFlareNum] ;
	CAnimationTime				_Persistence ;
	float						_Spacing ;
	bool					    _Attenuable ;
	float					    _AttenuationRange ;
	bool						_FirstFlareKeepSize ;
};


} // NL3D


#endif // NL_FLARE_SHAPE_H

/* End of flare_shape.h */
