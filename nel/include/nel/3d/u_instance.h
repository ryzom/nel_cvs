/** \file u_instance.h
 * <File description>
 *
 * $Id: u_instance.h,v 1.2 2001/06/29 14:27:40 berenguier Exp $
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

#ifndef NL_U_INSTANCE_H
#define NL_U_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_transform.h"
#include "nel/misc/aabbox.h"


namespace NL3D
{


class	UInstanceMaterial;


// ***************************************************************************
/**
 * Game interface for manipulating Objects, animations etc...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstance : virtual public UTransform
{
protected:

	/// \name Object
	// @{
	UInstance() {}
	virtual	~UInstance() {}
	// @}

public:


	/** Get the untransformed AABBox of the mesh. NULL (gtSize()==0) if no mesh.
	 */
	virtual void				getShapeAABBox(NLMISC::CAABBox &bbox) const =0;


	/// \name Material access.
	// @{
	/// return number of materials this mesh instance use.
	virtual	uint				getNumMaterials() const =0;
	/// return a local access on a material, to change its values. (NB: overwrited, if animated).
	virtual	UInstanceMaterial	&getMaterial(uint materialId)=0;
	// @}

};


} // NL3D


#endif // NL_U_INSTANCE_H

/* End of u_instance.h */
