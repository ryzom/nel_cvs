/** \file scene_group.h
 * <File description>
 *
 * $Id: scene_group.h,v 1.4 2001/08/09 09:19:39 besson Exp $
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
#include "3d/portal.h"
#include "3d/cluster.h"
#include "3d/transform.h"

#include <vector>

namespace NLMISC
{
class CRGBA;
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

	/**
	 * Instance part
	 */

	/// An element of the group.
	class CInstance
	{
	public:
		/// Name of the Shape Instance
		std::string Name;

		/// The parent instance (-1 if root)
		sint32 nParent;

		/// The clusters the instance belongs to
		std::vector<uint32> Clusters;

		/// Transformations
		NLMISC::CQuat Rot;
		NLMISC::CVector Pos;
		NLMISC::CVector Scale;

		// Name of the instance
		std::string InstanceName;
		bool DontAddToScene;

		/// Serial the instance
		void serial (NLMISC::IStream& f);
	};

	/// A vector of instance.
	typedef std::vector<CInstance> TInstanceArray;

	/// \todo remove all of these methods. For the moment DO NOT USE THEM !!!

	/// Get number of instance in this group
	uint getNumInstance () const;

	/// Get the name of the mesh referenced
	const std::string& getShapeName (uint instanceNb) const;

	/// Get the instance name
	const std::string& getInstanceName (uint instanceNb) const;

	/// Get an instance position
	const NLMISC::CVector& getInstancePos (uint instanceNb) const;

	/// Get an instance rotation
	const NLMISC::CQuat& getInstanceRot (uint instanceNb) const;

	// Get an instance scale
	const NLMISC::CVector& getInstanceScale (uint instanceNb) const;

	// Get the instance father (-1 if this is a root)
	const sint32 getInstanceParent (uint instanceNb) const;


	/**
	 * Construct, serialize and link to scene
	 */

	CInstanceGroup();
	~CInstanceGroup();

	/// Build the group
	void build (CVector &vGlobalPos, const TInstanceArray& array, 
				const std::vector<CCluster>& Portals, 
				const std::vector<CPortal>& Clusters);

	/// Serial the group
	void serial (NLMISC::IStream& f);

	/// Add all the instances to the scene
	void createRoot (CScene& scene);

	/// Add all the instances to the scene
	bool addToScene (CScene& scene);

	/// Remove all the instances from the scene
	bool removeFromScene (CScene& scene);


	/**
	 * LightMap part
	 */

	/// Get all lights (lightmaps) from an instance group
	void getLights (std::set<std::string> &LightNames);

	/// Set the lightmap factor for the whole instance group
	void setLightFactor (const std::string &LightName, NLMISC::CRGBA nFactor);


	/**
	 * Cluster/Portal system part
	 */

	/// To construct the cluster system by hand
	void addCluster (CCluster *pCluster);

	/// Set the cluster system to test for instances that are not in a cluster
	void setClusterSystem (CInstanceGroup *pIG);

	/// Get all dynamic portals of an instance group
	void getDynamicPortals (std::vector<std::string> &names);

	/// Set the state of a dynamic portal (true=opened, false=closed)
	void setDynamicPortal (std::string& name, bool opened);

	/// Get the state of a dynamic portal (true=opened, false=closed)
	bool getDynamicPortal (std::string& name);


	/**
	 * Transformation part
	 */

	/// Set the position of the IG
	void setPos (const CVector &pos);

	/// Set the rotation of the IG
	void setRotQuat (const CQuat &quat);

	/// Get the position of the IG
	CVector getPos ();
	
	/// Get the rotation of the IG
	CQuat getRotQuat ();

private:

	/// Look through all hierarchy our clusters that must be linked to our parent
	bool linkToParent (CInstanceGroup*pFather);

public:

	TInstanceArray					_InstancesInfos;
	std::vector<CTransformShape*>	_Instances;

	std::vector<CPortal>	_Portals;
	std::vector<CCluster>	_ClusterInfos;
	std::vector<CCluster*>	_ClusterInstances;

	CTransform *_Root;

	CClipTrav *_ClipTrav;
	CInstanceGroup *_ClusterSystem;

	NLMISC::CVector _GlobalPos;
};


} // NL3D


#endif // NL_SCENE_GROUP_H

/* End of scene_group.h */
