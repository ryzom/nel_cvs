/** \file scene_group.cpp
 * <File description>
 *
 * $Id: scene_group.cpp,v 1.41 2002/08/09 09:31:13 berenguier Exp $
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

#include "std3d.h"

#include "3d/scene_group.h"
#include "nel/misc/stream.h"
#include "nel/misc/matrix.h"
#include "3d/scene.h"
#include "3d/transform_shape.h"
#include "3d/mesh_instance.h"
#include "3d/shape_bank.h"
#include "nel/3d/u_instance_group.h"

using namespace NLMISC;
using namespace std;

namespace NL3D 
{

// ---------------------------------------------------------------------------
// CInstance
// ---------------------------------------------------------------------------

// ***************************************************************************
CInstanceGroup::CInstance::CInstance ()
{
	DontAddToScene = false;
	AvoidStaticLightPreCompute= false;
	StaticLightEnabled= false;
	DontCastShadow= false;
	LocalAmbientId= 0xFF;
}

// ***************************************************************************
void CInstanceGroup::CInstance::serial (NLMISC::IStream& f)
{
	/*
	Version 4:
		- LocalAmbientId.
	Version 3:
		- StaticLight.
	Version 2:
		- gameDev data.
	Version 1:
		- Clusters
	*/
	// Serial a version number
	sint version=f.serialVersion (4);


	// Serial the LocalAmbientId.
	if (version >= 4)
	{
		f.serial(LocalAmbientId);
	}
	else if(f.isReading())
	{
		LocalAmbientId= 0xFF;
	}

	// Serial the StaticLight
	if (version >= 3)
	{
		f.serial (AvoidStaticLightPreCompute);
		f.serial (DontCastShadow);
		f.serial (StaticLightEnabled);
		f.serial (SunContribution);
		nlassert(CInstanceGroup::NumStaticLightPerInstance==2);
		f.serial (Light[0]);
		f.serial (Light[1]);
	}
	else if(f.isReading())
	{
		AvoidStaticLightPreCompute= false;
		StaticLightEnabled= false;
		DontCastShadow= false;
	}

	// Serial the gamedev data
	if (version >= 2)
	{
		f.serial (InstanceName);
		f.serial (DontAddToScene);
	}

	// Serial the clusters
	if (version >= 1)
		f.serialCont (Clusters);

	// Serial the name
	f.serial (Name);

	// Serial the position vector
	f.serial (Pos);

	// Serial the rotation vector
	f.serial (Rot);

	// Serial the scale vector
	f.serial (Scale);

	// Serial the parent location in the vector (-1 if no parent)
	f.serial (nParent);
}

// ---------------------------------------------------------------------------
// CInstanceGroup
// ---------------------------------------------------------------------------

// ***************************************************************************

uint CInstanceGroup::getNumInstance () const
{
	return _InstancesInfos.size();
}

// ***************************************************************************

const string& CInstanceGroup::getShapeName (uint instanceNb) const
{
	// Return the name of the n-th instance
	return _InstancesInfos[instanceNb].Name;
}

// ***************************************************************************

const string& CInstanceGroup::getInstanceName (uint instanceNb) const
{
	// Return the name of the n-th instance
	return _InstancesInfos[instanceNb].InstanceName;
}

// ***************************************************************************

const CVector& CInstanceGroup::getInstancePos (uint instanceNb) const
{
	// Return the position vector of the n-th instance
	return _InstancesInfos[instanceNb].Pos;
}

// ***************************************************************************

const CQuat& CInstanceGroup::getInstanceRot (uint instanceNb) const
{
	// Return the rotation vector of the n-th instance
	return _InstancesInfos[instanceNb].Rot;
}

// ***************************************************************************

const CVector& CInstanceGroup::getInstanceScale (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].Scale;
}

// ***************************************************************************

void CInstanceGroup::getInstanceMatrix(uint instanceNb,NLMISC::CMatrix &dest) const
{
	dest.identity();	
	dest.translate(getInstancePos(instanceNb));
	dest.rotate(getInstanceRot(instanceNb));
	dest.scale(getInstanceScale(instanceNb));	
}

	

// ***************************************************************************

const sint32 CInstanceGroup::getInstanceParent (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].nParent;
}


// ***************************************************************************
const CInstanceGroup::CInstance		&CInstanceGroup::getInstance(uint instanceNb) const
{
	return _InstancesInfos[instanceNb];
}

// ***************************************************************************
CInstanceGroup::CInstance		&CInstanceGroup::getInstance(uint instanceNb)
{
	return _InstancesInfos[instanceNb];
}

// ***************************************************************************
CTransformShape				*CInstanceGroup::getTransformShape(uint instanceNb) const
{
	if(instanceNb>_Instances.size())
		return NULL;
	return _Instances[instanceNb];
}

// ***************************************************************************
CInstanceGroup::CInstanceGroup()
{
	_IGSurfaceLight.setOwner(this);
	_GlobalPos = CVector(0,0,0);
	_Root = NULL;
	_ClusterSystem = NULL;
	_RealTimeSunContribution= true;
	_AddToSceneState = StateNotAdded;
	_TransformName = NULL;
	_AddRemoveInstance = NULL;
	_IGAddBeginCallback = NULL;
}

// ***************************************************************************
CInstanceGroup::~CInstanceGroup()
{
}

// ***************************************************************************
void CInstanceGroup::build (const CVector &vGlobalPos, const TInstanceArray& array, 
							const std::vector<CCluster>& Clusters, 
							const std::vector<CPortal>& Portals,
							const std::vector<CPointLightNamed> &pointLightList,
							const CIGSurfaceLight::TRetrieverGridMap *retrieverGridMap, 
							float igSurfaceLightCellSize)
{
	_GlobalPos = vGlobalPos;
	// Copy the array
	_InstancesInfos = array;

	_Portals = Portals;
	_ClusterInfos = Clusters;

	// Link portals and clusters
	uint32 i, j, k;
	for (i = 0; i < _Portals.size(); ++i)
	{
		for (j = 0; j < _ClusterInfos.size(); ++j)
		{
			bool bPortalInCluster = true;
			for (k = 0; k < _Portals[i]._Poly.size(); ++k)
				if (!_ClusterInfos[j].isIn (_Portals[i]._Poly[k]) )
				{
					bPortalInCluster = false;
					break;
				}
			if (bPortalInCluster)
			{				
				_Portals[i].setCluster(&_ClusterInfos[j]);
				_ClusterInfos[j].link (&_Portals[i]);
			}
		}
	}

	// Create Meta Cluster if needed
	/*
	CCluster clusterTemp;
	bool mustAdd = false;
	for (i = 0; i < _Portals.size(); ++i)
	if (_Portals[i].getNbCluster() == 1)
	{
		mustAdd = true;
		break;
	}
	if (mustAdd)
	{
		CCluster clusterTemp;
		_ClusterInfos.push_back(clusterTemp);
		CCluster *pMetaCluster = &_ClusterInfos[_ClusterInfos.size()-1];
		pMetaCluster->setMetaCluster();
		for (i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getNbCluster() == 1)
		{
			_Portals[i].setCluster(pMetaCluster);
			pMetaCluster->link(&_Portals[i]);
		}
	}*/


	// Build the list of light. NB: sort by LightGroupName the array.
	std::vector<uint>	plRemap;
	buildPointLightList(pointLightList, plRemap);

	// Build IgSurfaceLight
	// clear
	_IGSurfaceLight.clear();
	if(retrieverGridMap)
	{
		//build
		_IGSurfaceLight.build(*retrieverGridMap, igSurfaceLightCellSize, plRemap);
	}
}


// ***************************************************************************
void CInstanceGroup::build (const CVector &vGlobalPos, const TInstanceArray& array, 
							const std::vector<CCluster>& Clusters, 
							const std::vector<CPortal>& Portals)
{
	// empty pointLightList
	std::vector<CPointLightNamed> pointLightList;

	build(vGlobalPos, array, Clusters, Portals, pointLightList);
}


// ***************************************************************************
void CInstanceGroup::retrieve (CVector &vGlobalPos, TInstanceArray& array, 
				std::vector<CCluster>& Clusters, 
				std::vector<CPortal>& Portals,
				std::vector<CPointLightNamed> &pointLightList) const
{
	// Just copy infos. NB: light information order have change but is still valid
	vGlobalPos= _GlobalPos;
	array= _InstancesInfos;

	Portals= _Portals;
	Clusters= _ClusterInfos;
	// Must reset links to all portals and clusters.
	uint	i;
	for(i=0; i<Portals.size(); i++)
		Portals[i].resetClusterLinks();
	for(i=0; i<Clusters.size(); i++)
		Clusters[i].resetPortalLinks();


	pointLightList= getPointLightList();
}


// ***************************************************************************

void CInstanceGroup::serial (NLMISC::IStream& f)
{
	// Serial a header
	f.serialCheck ((uint32)'TPRG');

	/*
	Version 5:
		_ _RealTimeSunContribution
	Version 4:
		_ IGSurfaceLight
	Version 3:
		- PointLights
	*/
	// Serial a version number
	sint version=f.serialVersion (5);


	// _RealTimeSunContribution
	if (version >= 5)
	{
		f.serial(_RealTimeSunContribution);
	}
	else if(f.isReading())
	{
		_RealTimeSunContribution= true;
	}


	// Serial the IGSurfaceLight
	if (version >= 4)
	{
		f.serial(_IGSurfaceLight);
	}
	else if(f.isReading())
	{
		_IGSurfaceLight.clear();
	}


	// Serial the PointLights info
	if (version >= 3)
	{
		f.serial(_PointLightArray);
	}
	else if(f.isReading())
	{
		_PointLightArray.clear();
	}


	if (version >= 2)
		f.serial(_GlobalPos);

	if (version >= 1)
	{
		f.serialCont (_ClusterInfos);
		f.serialCont (_Portals);
		// Links
		if (f.isReading())
		{
			uint32 i, j;
			for (i = 0; i < _ClusterInfos.size(); ++i)
			{
				uint32 nNbPortals;
				f.serial (nNbPortals);
				_ClusterInfos[i]._Portals.resize (nNbPortals);
				// Recreate clusters to portals links
				for (j = 0; j < nNbPortals; ++j)
				{
					sint32 nPortalNb;
					f.serial (nPortalNb);
					_ClusterInfos[i]._Portals[j] = &_Portals[nPortalNb];
					_Portals[nPortalNb].setCluster (&_ClusterInfos[i]);
				}
			}
		}
		else // We are writing to the stream
		{
			uint32 i, j;
			for (i = 0; i < _ClusterInfos.size(); ++i)
			{
				uint32 nNbPortals = _ClusterInfos[i]._Portals.size();
				f.serial (nNbPortals);
				for (j = 0; j < nNbPortals; ++j)
				{
					sint32 nPortalNb = (_ClusterInfos[i]._Portals[j] - &_Portals[0]);
					f.serial (nPortalNb);
				}
			}
		}
	}

	// Serial the array
	f.serialCont (_InstancesInfos);
}

// ***************************************************************************
void CInstanceGroup::createRoot (CScene& scene)
{
	_Root = (CTransform*)scene.createModel (TransformId);
	_Root->setDontUnfreezeChildren (true);
	setPos (CVector(0,0,0));
}

// ***************************************************************************
void CInstanceGroup::setTransformNameCallback (ITransformName *pTN)
{
	_TransformName = pTN;
}


// ***************************************************************************
void CInstanceGroup::setAddRemoveInstanceCallback(IAddRemoveInstance *callback)
{
	_AddRemoveInstance = callback;
}

// ***************************************************************************
void CInstanceGroup::setIGAddBeginCallback(IIGAddBegin *callback)
{
	_IGAddBeginCallback = callback;
}

// ***************************************************************************
bool CInstanceGroup::addToScene (CScene& scene, IDriver *driver)
{
	uint32 i;

	_Instances.resize (_InstancesInfos.size(), NULL);

	if (_IGAddBeginCallback)
		_IGAddBeginCallback->startAddingIG(_InstancesInfos.size());

	// Creation and positionning of the new instance

	vector<CInstance>::iterator it = _InstancesInfos.begin();

	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;
		if (!rInstanceInfo.DontAddToScene)
		{
			string shapeName;
			
			bool getShapeName = true;
			
			// If there is a callback added to this instance group then transform
			// the name of the shape to load.
			if (_TransformName != NULL && !rInstanceInfo.InstanceName.empty())
			{												
				shapeName = _TransformName->transformName (i, rInstanceInfo.InstanceName);								
				if (!shapeName.empty())
					getShapeName = false;
			}
			
			if (getShapeName)
			{			
				if (rInstanceInfo.Name.find('.') == std::string::npos)
				{
					shapeName = rInstanceInfo.Name + ".shape";
				}
				else	// extension has already been added
				{
					shapeName = rInstanceInfo.Name;
				}
			}
			strlwr (shapeName);

					
			_Instances[i] = scene.createInstance (shapeName);
			if( _Instances[i] == NULL )
			{
				
				nlwarning("Not found '%s' file\n", shapeName.c_str());
				//#if defined(NL_DEBUG) && defined(__STL_DEBUG)
				//	nlstop;
				//#endif

				/*
					for (uint32 j = 0; j < i; ++j)
					{
						scene.deleteInstance(_Instances[j]);
						_Instances[j] = NULL;
					}
					throw NLMISC::Exception("CInstanceGroup::addToScene : unable to create %s shape file", rInstanceInfo.Name.c_str());
				*/
			}
		}
	}

	return addToSceneWhenAllShapesLoaded (scene, driver);
}

// ***************************************************************************
// Private method
bool CInstanceGroup::addToSceneWhenAllShapesLoaded (CScene& scene, IDriver *driver)
{
	uint32 i, j;
	vector<CInstance>::iterator it = _InstancesInfos.begin();
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;

		if (!rInstanceInfo.DontAddToScene)
		{
			if (_Instances[i])
			{
				_Instances[i]->setPos (rInstanceInfo.Pos);
				_Instances[i]->setRotQuat (rInstanceInfo.Rot);
				_Instances[i]->setScale (rInstanceInfo.Scale);
				_Instances[i]->setPivot (CVector::Null);

				// Static Light Setup
				if( rInstanceInfo.StaticLightEnabled )
				{
					// Count lights.
					uint numPointLights;
					for(numPointLights= 0; numPointLights<CInstanceGroup::NumStaticLightPerInstance; numPointLights++)
					{
						if(rInstanceInfo.Light[numPointLights]==0xFF)
							break;
					}
					// Max allowed.
					numPointLights= min(numPointLights, (uint)NL3D_MAX_LIGHT_CONTRIBUTION);

					// Get pl ptrs.
					CPointLight		*pls[CInstanceGroup::NumStaticLightPerInstance];
					for(uint j=0; j<numPointLights;j++)
					{
						uint	plId= rInstanceInfo.Light[j];
						pls[j]= (CPointLight*)(&_PointLightArray.getPointLights()[plId]);
					}

					// get frozenAmbientlight.
					CPointLight *frozenAmbientlight;
					if(rInstanceInfo.LocalAmbientId == 0xFF)
						// must take the sun one.
						frozenAmbientlight= NULL;
					else
						// ok, take the local ambient one.
						frozenAmbientlight= (CPointLight*)(&_PointLightArray.getPointLights()[rInstanceInfo.LocalAmbientId]);

					// Setup the instance.
					_Instances[i]->freezeStaticLightSetup(pls, numPointLights, rInstanceInfo.SunContribution, frozenAmbientlight);
				}

				// Driver not NULL ?
				if (driver)
				{
					// Flush shape's texture with this driver
					_Instances[i]->Shape->flushTextures (*driver);
				}
			}
		}
		else
		{
			_Instances[i] = NULL;
		}
	}

	// Setup the hierarchy
	// We just have to set the traversal HRC (Hierarchy)
	ITrav *pHrcTrav = scene.getTrav (HrcTravId);

	if (_Root == NULL)
	{
		createRoot (scene);
	}
	it = _InstancesInfos.begin();
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	if (!_InstancesInfos[i].DontAddToScene && _Instances[i] != NULL)
	{
		CInstance &rInstanceInfo = *it;
		if( rInstanceInfo.nParent != -1 ) // Is the instance get a parent
			pHrcTrav->link (_Instances[rInstanceInfo.nParent], _Instances[i]);
		else
			pHrcTrav->link (_Root, _Instances[i]);
	}
	// Attach the root of the instance group to the root of the hierarchy traversal
	pHrcTrav->link (NULL, _Root);

	// Cluster / Portals
	// -----------------

	CClipTrav *pClipTrav = (CClipTrav*)(scene.getTrav (ClipTravId));
	_ClipTrav = pClipTrav;

	// Create the MOT links (create the physical clusters)
	_ClusterInstances.resize (_ClusterInfos.size());
	for (i = 0; i < _ClusterInstances.size(); ++i)
	{
		_ClusterInstances[i] = (CCluster*)scene.createModel (ClusterId);
		_ClusterInstances[i]->Group = this;
		_ClusterInstances[i]->_Portals = _ClusterInfos[i]._Portals;
		_ClusterInstances[i]->_LocalVolume = _ClusterInfos[i]._LocalVolume;
		_ClusterInstances[i]->_LocalBBox = _ClusterInfos[i]._LocalBBox;
		_ClusterInstances[i]->_Volume = _ClusterInfos[i]._Volume;
		_ClusterInstances[i]->_BBox = _ClusterInfos[i]._BBox;
		_ClusterInstances[i]->FatherVisible = _ClusterInfos[i].FatherVisible;
		_ClusterInstances[i]->VisibleFromFather = _ClusterInfos[i].VisibleFromFather;
		_ClusterInstances[i]->Name = _ClusterInfos[i].Name;
		pClipTrav->registerCluster (_ClusterInstances[i]);
		pClipTrav->unlink (NULL, _ClusterInstances[i]);
	}

	// Relink portals with newly created clusters
	for (i = 0; i < _Portals.size(); ++i)
	for (j = 0; j < 2; ++j)
	{
		sint32 nClusterNb;
		nClusterNb = (_Portals[i]._Clusters[j] - &_ClusterInfos[0]);
		_Portals[i]._Clusters[j] = _ClusterInstances[nClusterNb];
	}

	// Link shapes to clusters
	for (i = 0; i < _Instances.size(); ++i)
	if (_Instances[i] != NULL && !_InstancesInfos[i].DontAddToScene)
	{
		if (_InstancesInfos[i].Clusters.size() > 0)
		{
			pClipTrav->unlink (NULL, _Instances[i]);
			for (j = 0; j < _InstancesInfos[i].Clusters.size(); ++j)
				pClipTrav->link (_ClusterInstances[_InstancesInfos[i].Clusters[j]], _Instances[i]);
			// For the first time we have to set all the instances to NOT move (and not be rebinded)
			_Instances[i]->freeze();
			_Instances[i]->setClusterSystem (this);
		}
		else
		{
			// These instances are not attached to a cluster at this level so we cannot freeze them
			// Moreover we must set their clustersystem they will be tested against
			_Instances[i]->setClusterSystem (_ClusterSystem);
		}
	}
	_Root->freeze();

	// HRC OBS like
	for (i = 0; i < _ClusterInstances.size(); ++i)
	{
		_ClusterInstances[i]->setWorldMatrix (_Root->getMatrix());

		for (j = 0; j < _ClusterInstances[i]->getNbPortals(); ++j)
		{
			CPortal *pPortal = _ClusterInstances[i]->getPortal(j);
			pPortal->setWorldMatrix (_Root->getMatrix());
		}

		// Re affect the cluster to the accelerator if not the root
		if (!_ClusterInstances[i]->isRoot())
		{
			_ClipTrav->Accel.erase (_ClusterInstances[i]->AccelIt);
			_ClipTrav->registerCluster (_ClusterInstances[i]);
		}
	}


	// Link the instance group to the parent
	linkToParent (scene.getGlobalInstanceGroup());

	// Attach the clusters to the root of the instance group
	for (i = 0; i < _ClusterInstances.size(); ++i)
		pHrcTrav->link (_Root, _ClusterInstances[i]);


	// Default: freezeHRC all instances.
	freezeHRC();


	// Register the instanceGroup for light animation
	// -----------------
	// If some PointLight to animate
	if(_PointLightArray.getPointLights().size() > 0)
		scene.addInstanceGroupForLightAnimation(this);

	_AddToSceneState = StateAdded;

	if (_AddRemoveInstance)
		_AddRemoveInstance->instanceGroupAdded();
	return true;
}

// ***************************************************************************
bool CInstanceGroup::addToSceneAsync (CScene& scene, IDriver *driver)
{
	uint32 i;

	_AddToSceneState = StateAdding;
	_AddToSceneTempScene = &scene;
	_AddToSceneTempDriver = driver;

	_Instances.resize (_InstancesInfos.size(), NULL);

	if (_IGAddBeginCallback)
		_IGAddBeginCallback->startAddingIG(_InstancesInfos.size());

	// Creation and positionning of the new instance

	vector<CInstance>::iterator it = _InstancesInfos.begin();
	set<string> allShapesToLoad;
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;
		if (!rInstanceInfo.DontAddToScene)
		{
			string shapeName;
			bool   getShapeName = true;

			if (_TransformName != NULL && !rInstanceInfo.InstanceName.empty())
			{												
				shapeName = _TransformName->transformName (i, rInstanceInfo.InstanceName);								
				if (!shapeName.empty())
					getShapeName = false;
			}
			

			if (getShapeName)
			{						
				if (rInstanceInfo.Name.find('.') == std::string::npos)
				{
					shapeName = rInstanceInfo.Name + ".shape";
				}
				else	// extension has already been added
				{
					shapeName  = rInstanceInfo.Name;
				}
			}
			shapeName = strlwr (shapeName);

			shapeName = strlwr (shapeName);
			if (allShapesToLoad.find(shapeName) == allShapesToLoad.end())
			{
				allShapesToLoad.insert (shapeName);
				if (scene.getShapeBank()->isPresent(shapeName) != CShapeBank::Present)
				{
					// Load it from file asynchronously
					scene.getShapeBank()->loadAsync (shapeName, scene.getDriver(), &_AddToSceneSignal);
				}
			}
		}
	}
	_AddToSceneSignal = false;
	//CAsyncFileManager::getInstance().signal (&_AddToSceneSignal);
	return true;
}

// ***************************************************************************
void CInstanceGroup::stopAddToSceneAsync ()
{
	if (_AddToSceneState != StateAdding)
		return;
	vector<CInstance>::iterator it = _InstancesInfos.begin();
	CAsyncFileManager::getInstance().cancelSignal (&_AddToSceneSignal);
	for (uint32 i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CInstance &rInstanceInfo = *it;
		if (!rInstanceInfo.DontAddToScene)
		{
			string shapeName;


			bool getShapeName = true;

			if (_TransformName != NULL && !rInstanceInfo.InstanceName.empty())
			{												
				shapeName = _TransformName->transformName (i, rInstanceInfo.InstanceName);								
				if (!shapeName.empty())
					getShapeName = false;
			}

			
			if (getShapeName)
			{			
				if (rInstanceInfo.Name.find('.') == std::string::npos)
					shapeName = rInstanceInfo.Name + ".shape";
				else	// extension has already been added
					shapeName  = rInstanceInfo.Name;
			}

			shapeName = strlwr (shapeName);
			_AddToSceneTempScene->getShapeBank()->cancelLoadAsync (shapeName);
		}
	}
	_AddToSceneState = StateNotAdded;
}

// ***************************************************************************
CInstanceGroup::TState CInstanceGroup::getAddToSceneState ()
{
	// If we are adding but we have finished loading shapes (all shapes are here)
	if (_AddToSceneState == StateAdding)
	{
		if (_AddToSceneSignal)
		{
			addToScene (*_AddToSceneTempScene, _AddToSceneTempDriver);
		}
	}
	return _AddToSceneState;
}

// ***************************************************************************
// Search in the hierarchy of ig the most low level (child) ig that contains the clusters that
// are flagged to be visible from father or which father is visible
bool CInstanceGroup::linkToParent (CInstanceGroup *pFather)
{		
	uint32 i, j;
	bool ret;

	

	for (i = 0; i < pFather->_ClusterInstances.size(); ++i)
	{
		for(j = 0; j < pFather->_ClusterInstances[i]->Children.size(); ++j)
		{
			if (linkToParent(pFather->_ClusterInstances[i]->Children[j]->Group))
				return true;
		}
	}	
	ret = false;
	if (this != pFather)
	{		
		for (j = 0; j < this->_ClusterInstances.size(); ++j)
		{
			if ((this->_ClusterInstances[j]->FatherVisible) ||
				(this->_ClusterInstances[j]->VisibleFromFather))
			{
				for (i = 0; i < pFather->_ClusterInstances .size(); ++i)
				{			
					
					if (pFather->_ClusterInstances[i]->isIn(this->_ClusterInstances[j]->getBBox()))
					{
						if (this->_ClusterInstances[j]->Father != pFather->_ClusterInstances[i]) // not already sons of the right cluster ?
						{						
							// unlink from parent
							this->_ClusterInstances[j]->unlinkFromParent();

							// relink in hierarchy
							pFather->_ClusterInstances[i]->Children.push_back(this->_ClusterInstances[j]);
							this->_ClusterInstances[j]->Father = pFather->_ClusterInstances[i];
						}
						ret = true;
					}
				}
			}
		}
	}
	return ret;
}

// ***************************************************************************
bool CInstanceGroup::removeFromScene (CScene& scene)
{
	uint32 i, j, k;
	// Remove shapes
	vector<CTransformShape*>::iterator it = _Instances.begin();
	for (i = 0; i < _InstancesInfos.size(); ++i, ++it)
	{
		CTransformShape *pTShape = *it;
		if(pTShape)
		{
			// For security, unfreeze any StaticLightSetup setuped.
			pTShape->unfreezeStaticLightSetup();
			// delete the instance
			scene.deleteInstance (pTShape);
			*it = NULL;
		}
	}

	// Relink portals with old clusters
	for (i = 0; i < _Portals.size(); ++i)
	for (k = 0; k < 2; ++k)
	{
		for (j = 0; j < _ClusterInstances.size(); ++j)
			if( _Portals[i]._Clusters[k] == _ClusterInstances[j] )
				break;

		nlassert (j!=_ClusterInstances.size());
		_Portals[i]._Clusters[k] = &_ClusterInfos[j];
	}

	// Remove clusters
	CClipTrav *pClipTrav = (CClipTrav*)(scene.getTrav (ClipTravId));
	for (i = 0; i < _ClusterInstances.size(); ++i)
	{
		pClipTrav->unregisterCluster (_ClusterInstances[i]);
		scene.deleteModel (_ClusterInstances[i]);
	}

	scene.deleteModel (_Root);
	_Root = NULL;


	// UnRegister the instanceGroup for light animation
	// -----------------
	// If some PointLight to animate
	if(_PointLightArray.getPointLights().size() > 0)
		scene.removeInstanceGroupForLightAnimation(this);

	if (_AddRemoveInstance)
		_AddRemoveInstance->instanceGroupRemoved();
	return true;
}


// ***************************************************************************
void CInstanceGroup::getLights( set<string> &LightNames )
{
	LightNames.clear();
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshInstance *pMI = dynamic_cast<CMeshInstance*>(_Instances[i]);
		if( pMI != NULL )
		{
			uint32 nNbLM = pMI->getNbLightMap();
			for( uint32 j = 0; j < nNbLM; ++j )
			{
				string sTmp;
				pMI->getLightMapName( j, sTmp );
				set<string>::iterator itSet =  LightNames.find(sTmp);
				if( itSet == LightNames.end() )
					LightNames.insert( sTmp );
			}
		}
	}
}

// ***************************************************************************
void CInstanceGroup::setLightFactor( const string &LightName, CRGBA Factor )
{
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshBaseInstance *pMI = dynamic_cast<CMeshBaseInstance*>(_Instances[i]);
		if( pMI != NULL )
		{
			pMI->setLightMapFactor( LightName, Factor );
		}
	}
}

// ***************************************************************************
void CInstanceGroup::getBlendShapes( set<string> &BlendShapeNames )
{
	BlendShapeNames.clear();
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshBaseInstance *pMBI = dynamic_cast<CMeshBaseInstance*>(_Instances[i]);
		if (pMBI != NULL)
		{
			uint32 nNbBS = pMBI->getNbBlendShape();
			for( uint32 j = 0; j < nNbBS; ++j )
			{
				string sTmp;
				pMBI->getBlendShapeName( j, sTmp );
				set<string>::iterator itSet =  BlendShapeNames.find(sTmp);
				if( itSet == BlendShapeNames.end() )
					BlendShapeNames.insert( sTmp );
			}
		}
	}
}

// ***************************************************************************
void CInstanceGroup::setBlendShapeFactor( const string &BlendShapeName, float rFactor )
{
	for( uint32 i = 0; i < _Instances.size(); ++i )
	{
		CMeshBaseInstance *pMI = dynamic_cast<CMeshBaseInstance*>(_Instances[i]);
		if( pMI != NULL )
		{
			pMI->setBlendShapeFactor( BlendShapeName, rFactor );
		}
	}
}

// ***************************************************************************
void CInstanceGroup::addCluster(CCluster *pCluster)
{
	_ClusterInstances.push_back(pCluster);
}

// ***************************************************************************
void CInstanceGroup::setClusterSystem(CInstanceGroup *pIG)
{
	_ClusterSystem = pIG;
	for (uint32 i = 0; i < _Instances.size(); ++i)
		if (_Instances[i] && _InstancesInfos[i].Clusters.size() == 0)
			_Instances[i]->setClusterSystem (_ClusterSystem);
}

// ***************************************************************************
void CInstanceGroup::getDynamicPortals (std::vector<std::string> &names)
{
	for (uint32 i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getName() != "")
			names.push_back (_Portals[i].getName());
}

// ***************************************************************************
void CInstanceGroup::setDynamicPortal (std::string& name, bool opened)
{
	for (uint32 i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getName() == name)
			_Portals[i].open (opened);
}

// ***************************************************************************
bool CInstanceGroup::getDynamicPortal (std::string& name)
{
	for (uint32 i = 0; i < _Portals.size(); ++i)
		if (_Portals[i].getName() == name)
			return _Portals[i].isOpened ();
	return false;
}

// ***************************************************************************
void CInstanceGroup::setPos (const CVector &pos)
{
	if (_Root != NULL)
		/// \todo Make this work (precision): _Root->setPos (_GlobalPos+pos);
		_Root->setPos (pos);
}

// ***************************************************************************
void CInstanceGroup::setRotQuat (const CQuat &quat)
{
	if (_Root != NULL)
		_Root->setRotQuat (quat);
}

// ***************************************************************************
CVector CInstanceGroup::getPos ()
{
	if (_Root != NULL)
		return _Root->getPos ();
	else
		return CVector(0.0f, 0.0f, 0.0f);
}

// ***************************************************************************
CQuat CInstanceGroup::getRotQuat ()
{
	if (_Root != NULL)
		return _Root->getRotQuat ();
	else
		return CQuat();
}

// ***************************************************************************
void		CInstanceGroup::linkRoot (CScene &scene, CTransform *father)
{
	if(_Root)
	{
		ITrav *pHrcTrav = scene.getTrav (HrcTravId);
		pHrcTrav->link(father, _Root);
	}
}

// ***************************************************************************
void		CInstanceGroup::freezeHRC()
{
	// For all instances.
	for (uint i=0; i < _Instances.size(); i++)
	{
		if(_Instances[i])
			_Instances[i]->freezeHRC();
	}
	// and for root.
	_Root->freezeHRC();
}


// ***************************************************************************
void		CInstanceGroup::unfreezeHRC()
{
	// For all instances.
	for (uint i=0; i < _Instances.size(); i++)
	{
		if(_Instances[i])
			_Instances[i]->unfreezeHRC();
	}
	// and for root.
	_Root->unfreezeHRC();
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CInstanceGroup::buildPointLightList(const std::vector<CPointLightNamed> &pointLightList,
	std::vector<uint>	&plRemap)
{
	// build.
	_PointLightArray.build(pointLightList, plRemap);

	// remap Instance precalc lighted.
	for(uint i=0; i<_InstancesInfos.size(); i++)
	{
		CInstance	&inst= _InstancesInfos[i];
		// If the instance has no precomputed lighting, skip
		if(!inst.StaticLightEnabled)
			continue;

		// remap pointlights
		for(uint l=0; l<CInstanceGroup::NumStaticLightPerInstance; l++)
		{
			// If NULL light, break and continue to next instance
			if(inst.Light[l]== 0xFF)
				break;
			else
			{
				// Check good index.
				nlassert(inst.Light[l] < _PointLightArray.getPointLights().size());
				// Remap index, because of light sorting.
				inst.Light[l]= plRemap[inst.Light[l]];
			}
		}

		// remap ambient light
		if(inst.LocalAmbientId!=0xFF)
		{
			nlassert(inst.LocalAmbientId < _PointLightArray.getPointLights().size());
			inst.LocalAmbientId= plRemap[inst.LocalAmbientId];
		}
	}

}

// ***************************************************************************
void			CInstanceGroup::setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor)
{
	_PointLightArray.setPointLightFactor(lightGroupName, nFactor);
}


// ***************************************************************************
void			CInstanceGroup::enableRealTimeSunContribution(bool enable)
{
	_RealTimeSunContribution= enable;
}


} // NL3D
