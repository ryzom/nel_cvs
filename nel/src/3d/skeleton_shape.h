/** \file skeleton_shape.h
 * <File description>
 *
 * $Id: skeleton_shape.h,v 1.4 2001/08/02 08:34:32 berenguier Exp $
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

#ifndef NL_SKELETON_SHAPE_H
#define NL_SKELETON_SHAPE_H

#include "nel/misc/types_nl.h"
#include "3d/shape.h"
#include "3d/bone.h"
#include <vector>
#include <map>


namespace NL3D 
{


// ***************************************************************************
/**
 * a  definition of a skeleton. can be instanciated into a CSkeletonModel.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkeletonShape : public IShape
{
public:

	/// Constructor
	CSkeletonShape() {}

	/** Build a skeletonShape, replacing old.
	 * WARNING: bones must be organized in Depth-first order (this is not checked).
	 */
	void			build(const std::vector<CBoneBase> &bones);



	/// Return the id of a bone, from it's name. -1 if not present.
	sint32			getBoneIdByName(const std::string &name) const;


	/// \name From IShape
	// @{

	/// Create a CSkeletonModel, which contains bones.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this skeleton.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix);

	/// render() this skeletonshape in a driver  (no-op)
	virtual void	render(IDriver *drv, CTransformShape *trans, bool opaquePass)
	{
	}

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this skeletonshape.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CSkeletonShape);

	// @}

private:
	std::vector<CBoneBase>			_Bones;
	std::map<std::string, uint32>	_BoneMap;

};


} // NL3D


#endif // NL_SKELETON_SHAPE_H

/* End of skeleton_shape.h */
