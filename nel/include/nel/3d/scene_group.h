/** \file scene_group.h
 * <File description>
 *
 * $Id: scene_group.h,v 1.1 2001/04/06 13:31:20 besson Exp $
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

#ifndef NL_SCENE_GROUP_H
#define NL_SCENE_GROUP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include <vector>

namespace NLMISC
{
class IStream;
struct EStream;
}

namespace NL3D {

class CScene;
class CTransformShape;
/**
  * A CInstanceGroup is a group of mesh instance and so composed by
  *  - A reference to a mesh (refered by the name)
  *  - The transformations to get it to the world
  *  - The parent
  *
  * This class can initialize a scene and be serialized.
  *
  * \author Matthieu Besson
  * \author Nevrax France
  * \date 2001
  */
class CInstanceGroup
{
public:
	/// An element of the group.
	class CInstance
	{
	public:
		/// Name of the instance
		std::string Name;

		// The parent instance (-1 if root)
		sint32 nParent;

		// Transformations
		NLMISC::CQuat Rot;
		NLMISC::CVector Pos;
		NLMISC::CVector Scale;

		/// Serial the node
		void serial (NLMISC::IStream& f);
	};

	/// A vector of instance.
	typedef std::vector<CInstance> TInstanceArray;

	/// Get number of instance in this group
	uint getNumInstance () const;

	/// Get an instance name (the name of the mesh referenced)
	const std::string& getInstanceName (uint instanceNb) const;

	/// Get an instance position
	const NLMISC::CVector& getInstancePos (uint instanceNb) const;

	/// Get an instance rotation
	const NLMISC::CQuat& getInstanceRot (uint instanceNb) const;

	// Get an instance scale
	const NLMISC::CVector& getInstanceScale (uint instanceNb) const;

	// Get the instance father (-1 if this is a root)
	const int getInstanceParent (uint instanceNb) const;

	/// Build the group
	void build (const TInstanceArray& array);

	/// Serial the group
	void serial (NLMISC::IStream& f);

	/// Add all the instances to the scene
	bool addToScene (CScene& scene);

	/// Remove all the instances from the scene
	bool removeFromScene (CScene& scene);

private:
	TInstanceArray					_InstancesInfos;
	std::vector<CTransformShape*>	_Instances;
};


} // NL3D


#endif // NL_SCENE_GROUP_H

/* End of scene_group.h */
