/** \file scene_group.h
 * <File description>
 *
 * $Id: scene_group.h,v 1.11 2002/04/16 16:22:07 vizerie Exp $
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
#include "3d/point_light_named.h"
#include "3d/point_light_named_array.h"
#include "3d/ig_surface_light.h"

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

	/// Should Never be changed
	enum	{NumStaticLightPerInstance= 2};

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

		/// Precomputed Lighting. 
		// If true (false by default), then the instance don't cast shadow (used by ig_lighter.exe).
		bool	DontCastShadow;
		// If true (false by default), then the instance's lighting will not be precomputed.
		bool	AvoidStaticLightPreCompute;
		// This part is precomputed.
		bool	StaticLightEnabled;		// If false, then the instance 's lighting is not pre-computed.
		uint8 	SunContribution;		// Contribution of the Sun.
		// Ids of the lights. FF == disabled. if Lights[i]==FF, then all lights >=i are considered disabled.
		uint8	Light[NumStaticLightPerInstance];
		/** Id of the ambiant Light to take for this instance. Ambient light are stored too in ig->getPointLigths()
		 *	If 0xFF => take Ambient of the sun.
		 */
		uint8	LocalAmbientId;


		/// Constructor
		CInstance ();

		/// Serial the instance
		void serial (NLMISC::IStream& f);
	};

	/// A vector of instance.
	typedef std::vector<CInstance> TInstanceArray;

	/// \todo remove all of these methods. For the moment DO NOT USE THEM !!!

	/// Get number of instance in this group
	uint					getNumInstance () const;

	/// Get the name of the mesh referenced
	const std::string&		getShapeName (uint instanceNb) const;

	/// Get the instance name
	const std::string&		getInstanceName (uint instanceNb) const;

	/// Get an instance position
	const NLMISC::CVector&	getInstancePos (uint instanceNb) const;

	/// Get an instance rotation
	const NLMISC::CQuat&	getInstanceRot (uint instanceNb) const;

	// Get an instance scale
	const NLMISC::CVector&	getInstanceScale (uint instanceNb) const;

	// Get the instance father (-1 if this is a root)
	const sint32			getInstanceParent (uint instanceNb) const;

	// Get a const ref on the instance
	const CInstance			&getInstance(uint instanceNb) const;

	// Get a mutable ref on the instance
	CInstance				&getInstance(uint instanceNb);
	
	// Get the ig global pos
	const NLMISC::CVector &getGlobalPos() const { return _GlobalPos; }


	/**
	 * Construct, serialize and link to scene
	 */

	CInstanceGroup();
	~CInstanceGroup();

	/** Build the group
	 *	Build with an empty list of light
	 */
	void build (const CVector &vGlobalPos, const TInstanceArray& array, 
				const std::vector<CCluster>& Portals, 
				const std::vector<CPortal>& Clusters);

	/** Build the group
	 *	Build also the list of light. NB: sort by LightGroupName the array.
	 *	Give also a ptr on a retrieverGridMap to build surfaces (if not NULL).
	 */
	void build (const CVector &vGlobalPos, const TInstanceArray& array, 
				const std::vector<CCluster>& Portals, 
				const std::vector<CPortal>& Clusters,
				const std::vector<CPointLightNamed> &pointLightList,
				const CIGSurfaceLight::TRetrieverGridMap *retrieverGridMap= NULL, 
				float igSurfaceLightCellSize= 0);

	/** Retreive group information. NB: data may have changed, eg: order of lights.
	 */
	void retrieve (CVector &vGlobalPos, TInstanceArray& array, 
				std::vector<CCluster>& Portals, 
				std::vector<CPortal>& Clusters,
				std::vector<CPointLightNamed> &pointLightList) const;

	/// Serial the group
	void serial (NLMISC::IStream& f);

	/// Add all the instances to the scene
	void createRoot (CScene& scene);

	/**
	  * Add all the instances to the scene. By default, freezeHRC() those instances and the root.
	  *
	  * \param scene is the scene in which you want to add the instance group.
	  * \param driver is a pointer to a driver. If this pointer is not NULL, the textures used by
	  * the shapes will be preloaded in this driver. If the pointer is NULL (default), textures
	  * will ve loaded when the shape will be used.
	  */
	bool addToScene (CScene& scene, IDriver *driver=NULL);

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
	 * BlendShape part
	 */

	/// Get all the blendshapes from an instance group
	void getBlendShapes (std::set<std::string> &BlendShapeNames);

	/// Set the blendshape factor for the whole instance group (0 -> 100)
	void setBlendShapeFactor (const std::string &BlendShapeName, float rFactor);


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

	/// see CTransform::freezeHRC(). Do it for all instances (not clusters), and for the root of the IG.
	void		freezeHRC();

	/// see CTransform::unfreezeHRC(). Do it for all instances (not clusters), and for the root of the IG.
	void		unfreezeHRC();


public:

	/// \name RealTime lighting part
	// @{

	/// get the list of light. NB: the array is sorted by LightGroupName.
	const std::vector<CPointLightNamed> &getPointLightList() const {return _PointLightArray.getPointLights();}

	/// Get the number of point lights
	uint								 getNumPointLights() const { return _PointLightArray.getPointLights().size(); }

	/// Get a mutable ref on a point light named
	CPointLightNamed					&getPointLightNamed(uint index) 
	{ 
		return _PointLightArray.getPointLights()[index]; 
	} 

	/// set the Light factor for all pointLights "lightGroupName".
	void			setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor);

	/// See CIGSurfaceLight::getStaticLightSetup()
	bool			getStaticLightSetup(const std::string &retrieverIdentifier, sint surfaceId, const CVector &localPos,
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
	{
		return _IGSurfaceLight.getStaticLightSetup(retrieverIdentifier, surfaceId, localPos,
			pointLightList, sunContribution, localAmbient);
	}

	/// Setuped at export, tells if the ig is touched by the sun. true by default.
	void			enableRealTimeSunContribution(bool enable);
	bool			getRealTimeSunContribution() const {return _RealTimeSunContribution;}
	

	// @}


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


private:
	/// \name PointLight part
	// @{

	/// RealTimeSunContribution. Used for ig_lighter and zone_ig_lighter
	bool							_RealTimeSunContribution;

	/// Array of pointLights
	CPointLightNamedArray			_PointLightArray;

	/// Build the list of light. NB: sort by LightGroupName the array, and return index remap.
	void			buildPointLightList(const std::vector<CPointLightNamed> &pointLightList,
		std::vector<uint>	&plRemap);

	/**	The object used to light dynamic models in town and buildings
	 */
	CIGSurfaceLight		_IGSurfaceLight;

	// @}

};


} // NL3D


#endif // NL_SCENE_GROUP_H

/* End of scene_group.h */
