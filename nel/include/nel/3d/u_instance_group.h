/** \file u_instance_group.h
 * Game interface for managing group instance.
 *
 * $Id: u_instance_group.h,v 1.17 2002/05/23 09:30:56 vizerie Exp $
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

#ifndef NL_U_INSTANCE_GROUP_H
#define NL_U_INSTANCE_GROUP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/3d/point_light_influence.h"

#include <vector>
#include <string>

namespace NLMISC
{
	class CVector;
	class CQuat;
}

namespace NL3D 
{
	class UInstance;
	class UDriver;

/**
 * Callback class used at instancegroup loading
 *
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2002
 */

class ITransformName
{

public:
	/**
	 * transformName is called at the addToScene(Async) moment to transform the name of an
	 * instance to another name. This should be useful to replace various shapes by others 
	 * like in a shape-template. The function is called with the name of the shape and the
	 * user must return the new name of the shape.
	 * \param Name is the name of the shape contained in the instance group
	 * \return the new name of the shape which must be loaded in place of the param
	 */
	virtual std::string transformName (const std::string &Name) = 0;
};



/** Callback to know when an instance group is added / removed from the scene.
  */
struct IAddRemoveInstance
{
	// All instances have been been added to the scene
	virtual void instanceGroupAdded() = 0;
	// All instances have been removed from the scene
	virtual void instanceGroupRemoved() = 0;
};




/**
 * Game interface for managing group instance.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UInstanceGroup
{
public:

	enum TState { StateNotAdded = 0, StateAdding, StateAdded };

	/**
	 * Instance group
	 */
	virtual ~UInstanceGroup () {};


	/**
	 * Set up the callback used at loading and instanciating of the instanceGroup
	 * \param pTN is the transformName class that hold the callback
	 */
	virtual void setTransformNameCallback (ITransformName *pTN)=0;

	/// set a callback to know when an instance has been added / removed from scene
	virtual void setAddRemoveInstanceCallback(IAddRemoveInstance *callback) = 0;

	/**
	 * Add all the instances to the scene. By default, freezeHRC() those instances and the root.
	 *
	 * \param scene is the scene in which you want to add the instance group.
	 * \param driver is a pointer to a driver. If this pointer is not NULL, the textures used by
	 * the shapes will be preloaded in this driver. If the pointer is NULL (default), textures
	 * will ve loaded when the shape will be used.
	 */
	virtual void addToScene (class UScene& scene, UDriver *driver=NULL)=0;
	virtual void addToSceneAsync (class UScene& scene, UDriver *driver=NULL)=0;
	virtual void stopAddToSceneAsync ()=0;
	virtual TState getAddToSceneState ()=0;

	/**
	 *	remove this instance group from the given scene
	 */
	virtual void removeFromScene(class UScene& scene)=0;
	
	/**
	 *	This is the static function create a stand alone instance group.
	 */
	static	UInstanceGroup	*createInstanceGroup (const std::string &instanceGroup);
	static	void createInstanceGroupAsync (const std::string &instanceGroup, UInstanceGroup **pIG);
	static	void stopCreateInstanceGroupAsync (UInstanceGroup **pIG);


	/**
	 * Return the count of shape instance in this group.
	 */
	virtual uint getNumInstance () const=0;

	/**
	 * Return the name of the shape refered by an instance of the group.
	 *
	 * \param instanceNb is the number of the instance.
	 * \return the name of the shape.
	 */
	virtual const std::string& getShapeName (uint instanceNb) const=0;

	/**
	 * Return the name of an instance of the group.
	 *
	 * \param instanceNb is the number of the instance.
	 * \return the name of the instance.
	 */
	virtual const std::string& getInstanceName (uint instanceNb) const=0;

	/**
	 * Return the position of an instance of the group.
	 *
	 * \param instanceNb is the number of the instance.
	 * \return the position of the instance.
	 */
	virtual const NLMISC::CVector& getInstancePos (uint instanceNb) const=0;

	/**
	 * Return the rotation of an instance of the group.
	 *
	 * \param instanceNb is the number of the instance.
	 * \return the rotation of the instance.
	 */
	virtual const NLMISC::CQuat& getInstanceRot (uint instanceNb) const=0;

	/**
	 * Return the scale of an instance of the group.
	 *
	 * \param instanceNb is the number of the instance.
	 * \return the scale of the instance.
	 */
	virtual const NLMISC::CVector& getInstanceScale (uint instanceNb) const=0;

	/**
	 * Return the instance at a given position
	 */
	virtual UInstance *getByName (std::string& name) =0;

	/**
	 * Return the instance at a given position (const version)
	 */
	virtual const UInstance *getByName (std::string& name) const=0;


	/**
	 * Control a lightmap
	 */
	virtual void setLightFactor (const std::string &LightName, NLMISC::CRGBA nFactor)=0;

	/**
	 * Control a blendshape
	 */
	virtual void setBlendShapeFactor (const std::string &bsName, float rFactor)=0;

	/**
	 * Create the link between scene and instance group before the addToScene
	 * to allow the instance group to be moved without being linked to a father 
	 * cluster system.
	 */
	virtual void createRoot (class UScene &scene)=0;

	/**
	 * All the instances must be checked against the following cluster system
	 */
	virtual void setClusterSystem (class UInstanceGroup *pClusterSystem)=0;

	/**
	 * Get all dynamic portals in the instance group
	 * \return a vector of portal names
	 */
	virtual void getDynamicPortals (std::vector<std::string> &names)=0;

	/**
	 * Set the state of a dynamic portal
	 * \param name is the name of the dynamic portal to open or close
	 * \param opened is the state the portal will have (true=opened, false=closed)
	 */
	virtual void setDynamicPortal (std::string& name, bool opened)=0;

	/**
	 * Get the state of a dynamic portal
	 * \return the state (true=opened, false=closed) of a dynamic portal
	 */
	virtual bool getDynamicPortal (std::string& name)=0;

	/**
	 * Set the position of the instance group
	 * Take care if the instance group is a cluster system, you must not move it
	 * outside its father cluster which is assigned at addToScene moment.
	 */
	virtual void			setPos (const NLMISC::CVector &pos)=0;

	/**
	 * Set the rotation of the instance group
	 */
	virtual void			setRotQuat (const NLMISC::CQuat &q)=0;

	/**
	 * Get the position
	 */
	virtual NLMISC::CVector getPos ()=0;

	/**
	 * Get the rotation
	 */
	virtual NLMISC::CQuat	getRotQuat ()=0;


	/// see UTransform::freezeHRC(). Do it for all instances (not clusters), and for the root of the IG.
	virtual void			freezeHRC()=0;

	/// see UTransform::unfreezeHRC(). Do it for all instances (not clusters), and for the root of the IG.
	virtual void			unfreezeHRC()=0;


	/// \name PointLight part
	// @{

	/// set the Light factor for all pointLights of "lightGroupName".
	virtual void			setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor) =0;

	/** get the StaticLightSetup for a model at position localPos/retrieverIdentifier/surfaceId.
	 *	NB: all are get with help of UGlobalPosition. localPos= gp.LocalPosition.Estimated
	 */
	virtual	bool			getStaticLightSetup(const std::string &retrieverIdentifier, sint surfaceId, const NLMISC::CVector &localPos, 
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient) =0;

	// @}



};


} // NL3D


#endif // NL_U_INSTANCE_GROUP_H

/* End of u_instance_group.h */
