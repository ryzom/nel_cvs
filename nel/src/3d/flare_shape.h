/** \file flare_shape.h
 * <File description>
 *
 * $Id: flare_shape.h,v 1.3 2001/07/25 10:19:22 vizerie Exp $
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

	/// Constructor
	CFlareShape();

	/// inherited from IShape
	virtual	CTransformShape		*createInstance(CScene &scene) ;

	/// inherited from IShape
	virtual bool				clip(const std::vector<CPlane>	&pyramid) ;

	/// inherited from IShape. Does nothing. A new observer was set for that
	virtual void				render(IDriver *drv, CTransformShape *trans, bool opaquePass) {}

	/// inherited from IShape
	virtual	void				getAABBox(NLMISC::CAABBox &bbox) const ;

	/// inherited from ishape
	virtual float				getNumTriangles (float distance) { return 2 ; }


	/// set a texture for the flare
	void						setTexture(ITexture *tex) 
	{ 
		_Tex = tex ; 
	}

	/// get the texture used by the flare
	ITexture					*getTexture(void) 
	{ 
		return _Tex ; 
	}

	/// get the texture used by the flare (const version)
	const ITexture				*getTexture(void) const 
	{ 
		return _Tex ; 
	}

	/// set the size of the flare
	void						setSize(float size) 
	{ 
		_Size = size ; 
	}

	/// get the size of the flare
	float						getSize(void) const 
	{ 
		return _Size ; 
	}

	/// set the color of the flare
	void						setColor(NLMISC::CRGBA col) 
	{ 
		_Color = col ; 
	}

	/// get the color of the shape
	NLMISC::CRGBA				getColor(void) const 
	{ 
		return _Color ; 
	}

	/// set the persistence of this shape, in second (the time it takes to fade from white to black)
	void						setPersistence(CAnimationTime persistence) 
	{ 
		nlassert(persistence > 0) ; 
		_Persistence = persistence ; 
	}

	/** get the persistence of this shape
	  * \see setPersistence
	  */
	CAnimationTime				getPersistence(void) const 
	{ 
		return _Persistence ; 
	}

	/// serial this shape
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

protected:

	friend class CFlareModel ;

	NLMISC::CSmartPtr<ITexture> _Tex ;
	NLMISC::CRGBA				_Color ;
	float						_Size ;
	CAnimationTime				_Persistence ;
};


} // NL3D


#endif // NL_FLARE_SHAPE_H

/* End of flare_shape.h */
