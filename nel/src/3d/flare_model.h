/** \file flare_model.h
 * <File description>
 *
 * $Id: flare_model.h,v 1.3 2001/07/26 17:16:59 vizerie Exp $
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

#ifndef NL_FLARE_MODEL_H
#define NL_FLARE_MODEL_H

#include "nel/misc/types_nl.h"
#include "3d/transform_shape.h"


namespace NL3D {


/**
 * <Class description>
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CFlareModel : public CTransformShape
{
public:

	/// Constructor
	CFlareModel();

	// register this model and his observers
	static void registerBasic() ;	
	static IModel *creator() { return new CFlareModel; }

protected:
	friend class CFlareShape ;
	friend class CFlareRenderObs ;
	float				_Intensity ;	
	CScene				*_Scene		  ;	
};



class	CFlareRenderObs : public CTransformShapeRenderObs
{
public:

	virtual	void	traverse(IObs *caller);	
	static IObs	    *creator() {return new CFlareRenderObs;}
};



} // NL3D


#endif // NL_FLARE_MODEL_H

/* End of flare_model.h */
