/** \file mesh_instance.cpp
 * <File description>
 *
 * $Id: mesh_instance.cpp,v 1.6 2001/06/15 16:24:43 corvazier Exp $
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

#include "3d/mesh_instance.h"
#include "3d/mesh.h"
#include <list>

using namespace std;

namespace NL3D 
{


// ***************************************************************************
void		CMeshInstance::registerBasic()
{
	CMOT::registerModel(MeshInstanceId, TransformShapeId, CMeshInstance::creator);
	CMOT::registerObs(AnimDetailTravId, MeshInstanceId, CMeshInstanceAnimDetailObs::creator);
}


// ***************************************************************************
void		CMeshInstance::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix);

	// Add any materials.
	for(uint i=0;i<_AnimatedMaterials.size();i++)
	{
		// append material  matname.*
		_AnimatedMaterials[i].registerToChannelMixer(chanMixer, prefix + _AnimatedMaterials[i].getMaterialName() + ".");
	}
}


// ***************************************************************************
ITrack*		CMeshInstance::getDefaultTrack (uint valueId)
{
	// Pointer on the CMesh
	CMesh* pMesh=(CMesh*)(IShape*)Shape;

	// Switch the value
	switch (valueId)
	{
	case CTransform::PosValue:			
		return pMesh->getDefaultPos();
	case CTransform::RotEulerValue:		
		return pMesh->getDefaultRotEuler();
	case CTransform::RotQuatValue:		
		return pMesh->getDefaultRotQuat();
	case CTransform::ScaleValue:		
		return pMesh->getDefaultScale();
	case CTransform::PivotValue:		
		return pMesh->getDefaultPivot();
	default:
		// Problem, new values ?
		nlstop;
	};
	return NULL;
}

// ***************************************************************************
uint32 CMeshInstance::getNbLightMap()
{
	CMesh* pMesh=(CMesh*)(IShape*)Shape;
	return pMesh->_LightInfos.size();
}

// ***************************************************************************
void CMeshInstance::getLightMapName( uint32 nLightMapNb, std::string &LightMapName )
{
	CMesh* pMesh=(CMesh*)(IShape*)Shape;
	if( nLightMapNb >= pMesh->_LightInfos.size() )
		return;
	CMesh::TLightInfoMap::iterator itMap = pMesh->_LightInfos.begin();
	for( uint32 i = 0; i < nLightMapNb; ++i ) ++itMap;
	LightMapName = itMap->first;
}

// ***************************************************************************
void CMeshInstance::setLightMapFactor( const std::string &LightMapName, CRGBA Factor )
{
	CMesh* pMesh=(CMesh*)(IShape*)Shape;
	CMesh::TLightInfoMap::iterator itMap = pMesh->_LightInfos.find( LightMapName );
	if( itMap == pMesh->_LightInfos.end() )
		return;
	CMesh::CLightInfoMapList::iterator itList = itMap->second.begin();
	uint32 nNbElt = itMap->second.size();
	for( uint32 i = 0; i < nNbElt; ++i )
	{
		Materials[itList->nMatNb].setLightMapFactor( itList->nStageNb, Factor );
		++itList;
	}
}


} // NL3D
