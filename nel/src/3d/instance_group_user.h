/** \file instance_group_user.h
 * Implementation of the user interface managing instance groups.
 *
 * $Id: instance_group_user.h,v 1.22 2002/11/18 17:54:16 vizerie Exp $
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

#ifndef NL_INSTANCE_GROUP_USER_H
#define NL_INSTANCE_GROUP_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_instance_group.h"
#include "3d/scene_group.h"

#define NL3D_MEM_IG						NL_ALLOC_CONTEXT( 3dIg )

namespace NLMISC
{
	class CVector;
	class CQuat;
}

namespace NL3D 
{

class UScene;
class UInstanceGroup;
class CInstanceUser;

/**
 * Implementation of the user interface managing instance groups.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CInstanceGroupUser : public UInstanceGroup
{
public:
	CInstanceGroupUser ();
	// Init with a scene.
	//bool load (const std::string &instanceGroup);

	// Init without a scene
	bool init (const std::string &instanceGroup);
	//bool init (const std::string &instanceGroup, CScene &scene, UDriver *driver);

private:
	// From UInstanceGroup
	void setTransformNameCallback (ITransformName *pTN);
	void setAddRemoveInstanceCallback(IAddRemoveInstance *callback);
	void setIGAddBeginCallback(IIGAddBegin *callback);

	
	void addToScene (class UScene& scene, UDriver *driver);
	void addToScene (class CScene& scene, IDriver *driver);

	void addToSceneAsync (class UScene& scene, UDriver *driver);
	TState getAddToSceneState ();
	void stopAddToSceneAsync ();

	virtual void			setDistMax(uint instance, float dist);	
	virtual float			getDistMax(uint instance) const;	
	virtual void		    setCoarseMeshDist(uint instance, float dist);
	virtual float           getCoarseMeshDist(uint instance) const;


	void removeFromScene (class UScene& scene);
	uint getNumInstance () const;
	const std::string& getShapeName (uint instanceNb) const;
	const std::string& getInstanceName (uint instanceNb) const;
	virtual void				getInstanceMatrix(uint instanceNb, NLMISC::CMatrix &dest) const;
	const NLMISC::CVector& getInstancePos (uint instanceNb) const;
	const NLMISC::CQuat& getInstanceRot (uint instanceNb) const;
	const NLMISC::CVector& getInstanceScale (uint instanceNb) const;
	UInstance *getByName (std::string& name);
	const UInstance *getByName (std::string& name) const;

	void setLightFactor (const std::string &LightName, NLMISC::CRGBA nFactor);
	void setBlendShapeFactor (const std::string &bsName, float rFactor);

	void createRoot (UScene &scene);
	void setClusterSystem (UInstanceGroup *pClusterSystem);
	bool linkToParentCluster(UInstanceGroup *father);
	void getDynamicPortals (std::vector<std::string> &names);
	void setDynamicPortal (std::string& name, bool opened);
	bool getDynamicPortal (std::string& name);


	void setPos (const NLMISC::CVector &pos);
	void setRotQuat (const NLMISC::CQuat &q);

	void setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor);
	bool getStaticLightSetup(const std::string &retrieverIdentifier, sint surfaceId, const NLMISC::CVector &localPos, 
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient);

	NLMISC::CVector getPos ();
	NLMISC::CQuat	getRotQuat ();

	// The real instance group
	CInstanceGroup	_InstanceGroup;
	std::map<std::string,CInstanceUser*> _Instances;
	// Async stuff
	TState _AddToSceneState;
	UScene *_AddToSceneTempScene;
	UDriver *_AddToSceneTempDriver;

	virtual void			freezeHRC();
	virtual void			unfreezeHRC();

	friend class CTransformUser;
	friend class CSceneUser;

public:
	// Debug purpose only.
	CInstanceGroup	&getInternalIG() 
	{
		NL3D_MEM_IG
		return _InstanceGroup;
	}
};


} // NL3D


#endif // NL_INSTANCE_GROUP_USER_H

/* End of instance_group_user.h */
