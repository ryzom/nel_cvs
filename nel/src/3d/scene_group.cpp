/** \file scene_group.cpp
 * <File description>
 *
 * $Id: scene_group.cpp,v 1.4 2001/06/11 09:25:58 besson Exp $
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

#include "nel/3d/scene_group.h"
#include "nel/misc/stream.h"
#include "nel/3d/scene.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_instance.h"

using namespace NLMISC;
using namespace std;

namespace NL3D 
{


// ***************************************************************************

uint CInstanceGroup::getNumInstance () const
{
	return _InstancesInfos.size();
}

// ***************************************************************************

const string& CInstanceGroup::getInstanceName (uint instanceNb) const
{
	// Return the name of the n-th instance
	return _InstancesInfos[instanceNb].Name;
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

const int CInstanceGroup::getInstanceParent (uint instanceNb) const
{
	// Return the scale vector of the n-th instance
	return _InstancesInfos[instanceNb].nParent;
}

// ***************************************************************************

void CInstanceGroup::build (const TInstanceArray& array)
{
	// Copy the array
	_InstancesInfos=array;
}

// ***************************************************************************

void CInstanceGroup::serial (NLMISC::IStream& f)
{
	// Serial a header
	f.serialCheck ((uint32)'TPRG');

	// Serial a version number
	sint version=f.serialVersion (0);

	// Serial the array
	f.serialCont (_InstancesInfos);
}

// ***************************************************************************

void CInstanceGroup::CInstance::serial (NLMISC::IStream& f)
{
	// Serial a version number
	sint version=f.serialVersion (0);

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

// ***************************************************************************
bool CInstanceGroup::addToScene (CScene& scene)
{
	sint i;

	_Instances.resize( _InstancesInfos.size() );

	vector<CInstance>::iterator it = _InstancesInfos.begin();
	for( i=0; i<(sint)_InstancesInfos.size(); ++i,++it )
	{
		CInstance &rInstanceInfo = *it;

		// Creation and positionning of the new instance
		_Instances[i] = scene.createInstance ( rInstanceInfo.Name + ".shape" );

		// TempYoyo.
		// --------	
		sint	j;
		CMeshInstance	*mi= (CMeshInstance	*)_Instances[i];
		for(j=0;j<mi->Materials.size();j++)
		{
			//mi->Materials[j].setTexture(0, NULL);
			//mi->Materials[j].getTexture(1)->setFilterMode(ITexture::Nearest, ITexture::NearestMipMapOff);
			CMaterial &mat = mi->Materials[j]; 
			mi->Materials[j].setColor(CRGBA(255,255,255,255));


			// Put lightmap for all materials
		/*	
			mi->Materials[j].texEnvOpRGB( 0, CMaterial::Replace );
			mi->Materials[j].texEnvArg0RGB( 0, CMaterial::&Texture, CMaterial::SrcColor );

			mi->Materials[j].texEnvOpRGB( 1, CMaterial::Replace );
			mi->Materials[j].texEnvArg0RGB( 1, CMaterial::Texture, CMaterial::SrcColor );
		*/	
		}
		// --------	
		// TempYoyo end
	

		if( _Instances[i] == NULL )
		{
			printf("Not found %s.shape file\n", rInstanceInfo.Name.c_str());
			nlstop;
		}
	
		if (_Instances[i])
		{
			_Instances[i]->setPos( rInstanceInfo.Pos );
			_Instances[i]->setRotQuat( rInstanceInfo.Rot );
			_Instances[i]->setScale( rInstanceInfo.Scale );
			_Instances[i]->setPivot( CVector::Null );
		}
	}

	// Setup the hierarchy
	// We just have to set the traversal HRC (Hierarchy)

	ITrav *pTrav = scene.getTrav( HrcTravId );

	it = _InstancesInfos.begin();
	for( i=0; i<(sint)_InstancesInfos.size(); ++i,++it )
	{
		CInstance &rInstanceInfo = *it;
		if( rInstanceInfo.nParent != -1 ) // Is the instance get a parent
			pTrav->link( _Instances[rInstanceInfo.nParent], _Instances[i] );
	}

	return true;
}

// ***************************************************************************

bool CInstanceGroup::removeFromScene (CScene& scene)
{
	vector<CTransformShape*>::iterator it = _Instances.begin();
	for( int i=0; i<(sint)_InstancesInfos.size(); ++i,++it )
	{
		CTransformShape *pTShape = *it;

		scene.deleteInstance( pTShape );
	}
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
		CMeshInstance *pMI = dynamic_cast<CMeshInstance*>(_Instances[i]);
		if( pMI != NULL )
		{
			pMI->setLightMapFactor( LightName, Factor );
		}
	}
}




} // NL3D
