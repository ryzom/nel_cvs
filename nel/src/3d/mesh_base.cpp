/** \file mesh_base.cpp
 * <File description>
 *
 * $Id: mesh_base.cpp,v 1.24 2002/11/08 18:41:58 berenguier Exp $
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

#include "3d/mesh_base.h"
#include "3d/mesh_base_instance.h"
#include "3d/lod_character_texture.h"



namespace NL3D 
{



// ***************************************************************************
CMeshBase::CMeshBase()
{
	_UseLightingLocalAttenuation= false;

	// To have same functionnality than previous version, init to identity.
	_DefaultPos.setValue(CVector(0,0,0));
	_DefaultPivot.setValue(CVector(0,0,0));
	_DefaultRotEuler.setValue(CVector(0,0,0));
	_DefaultRotQuat.setValue(CQuat::Identity);
	_DefaultScale.setValue(CVector(1,1,1));
	_DefaultLMFactor.setValue(CRGBA(255,255,255,255));

	_AutoAnim = false;

	_LodCharacterTexture= NULL;
}


// ***************************************************************************
CMeshBase::~CMeshBase()
{
	// free if exist
	resetLodCharacterTexture();
}


// ***************************************************************************
// ***************************************************************************
// Animated material.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBase::setAnimatedMaterial(uint id, const std::string &matName)
{
	nlassert(!matName.empty());
	if(id<_Materials.size())
	{
		// add / replace animated material.
		_AnimatedMaterials[id].Name= matName;
		// copy Material default.
		_AnimatedMaterials[id].copyFromMaterial(&_Materials[id]);
	}
}

// ***************************************************************************
CMaterialBase	*CMeshBase::getAnimatedMaterial(uint id)
{
	TAnimatedMaterialMap::iterator	it;
	it= _AnimatedMaterials.find(id);
	if(it!=_AnimatedMaterials.end())
		return &it->second;
	else
		return NULL;
}


// ***************************************************************************
// ***************************************************************************
// Serial - buildBase.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMeshBase::CMeshBaseBuild::CMeshBaseBuild()
{
	DefaultPos.set(0,0,0);
	DefaultPivot.set(0,0,0);
	DefaultRotEuler.set(0,0,0);
	DefaultScale.set(1,1,1);

	bCastShadows= false;
	bRcvShadows= false;
	UseLightingLocalAttenuation= false;
}

// ***************************************************************************
#if 0
void	CMeshBase::CMeshBaseBuild::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 1:
		- Cut in version because of badly coded ITexture* serialisation. throw an exception if 
			find a version < 1.
	Version 0:
		- 1st version.
	*/
	sint	ver= f.serialVersion(1);

	if(ver<1)
		throw NLMISC::EStream(f, "MeshBuild in Stream is too old (MeshBaseBuild version < 1)");

	f.serial( DefaultPos );
	f.serial( DefaultPivot );
	f.serial( DefaultRotEuler );
	f.serial( DefaultRotQuat );
	f.serial( DefaultScale );

	f.serialCont( Materials );
}
#endif


// ***************************************************************************
void	CMeshBase::serialMeshBase(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 7:
		- _LodCharacterTexture
	Version 6:
		- _DistMax
	Version 5:
		- _AutoAnim
	Version 4:
		- _UseLightingLocalAttenuation
	Version 3:
		- _IsLightable
	Version 2:
		- Added Blend Shapes factors
	Version 1:
		- Cut in version because of badly coded ITexture* serialisation. throw an exception if 
			find a version < 1.
	Version 0:
		- 1st version.
	*/
	sint ver = f.serialVersion(7);

	if (ver >= 2)
	{
		f.serialCont (_AnimatedMorph);
	}

	if(ver<1)
		throw NLMISC::EStream(f, "Mesh in Stream is too old (MeshBase version < 1)");

	f.serial (_DefaultPos);
	f.serial (_DefaultPivot);
	f.serial (_DefaultRotEuler);
	f.serial (_DefaultRotQuat);
	f.serial (_DefaultScale);

	f.serialCont(_Materials);
	f.serialCont(_AnimatedMaterials);
	f.serialCont(_LightInfos);

	if(ver>=3)
		// read/write _IsLightable flag.
		f.serial(_IsLightable);
	else if( f.isReading() )
		// update _IsLightable flag.
		computeIsLightable();

	if(ver>=4)
		f.serial(_UseLightingLocalAttenuation);
	else if( f.isReading() )
		_UseLightingLocalAttenuation= false;

	if (ver >= 5)
	{
		f.serial(_AutoAnim);
	}

	if(ver >= 6)
		f.serial(_DistMax);

	if(ver >= 7)
		f.serialPtr(_LodCharacterTexture);

}


// ***************************************************************************
void	CMeshBase::buildMeshBase(CMeshBaseBuild &m)
{
	// Copy light information
	_LightInfos = m.LightInfoMap;

	// copy the materials.
	_Materials= m.Materials;

	// clear the animated materials.
	_AnimatedMaterials.clear();

	/// Copy default position values
	_DefaultPos.setValue (m.DefaultPos);
	_DefaultPivot.setValue (m.DefaultPivot);
	_DefaultRotEuler.setValue (m.DefaultRotEuler);
	_DefaultRotQuat.setValue (m.DefaultRotQuat);
	_DefaultScale.setValue (m.DefaultScale);

	_AnimatedMorph	.resize(m.DefaultBSFactors.size());
	for (uint32 i = 0; i < m.DefaultBSFactors.size(); ++i)
	{
		_AnimatedMorph[i].DefaultFactor.setValue (m.DefaultBSFactors[i]);
		_AnimatedMorph[i].Name = m.BSNames[i];
	}

	// update _IsLightable flag.
	computeIsLightable();
	// copy _UseLightingLocalAttenuation
	_UseLightingLocalAttenuation= m.UseLightingLocalAttenuation;
}




// ***************************************************************************
void	CMeshBase::instanciateMeshBase(CMeshBaseInstance *mi, CScene *ownerScene)
{
	uint32 i;


	// Copy ownerScene.
	mi->_OwnerScene= ownerScene;


	// setup animated blendShapes
	//===========================
	mi->_AnimatedMorphFactor.reserve(_AnimatedMorph.size());
	for(i = 0; i < _AnimatedMorph.size(); ++i)
	{
		CAnimatedMorph am(&_AnimatedMorph[i]);
		mi->_AnimatedMorphFactor.push_back (am);
	}
	
	// setup materials.
	//=================
	// Copy material. Textures are referenced only
	mi->Materials= _Materials;

	// Instanciate selectable textures (use default set)
	mi->selectTextureSet(0);

	// prepare possible AsyncTextures
	mi->AsyncTextures.resize(_Materials.size());

	// setup animated materials.
	//==========================
	TAnimatedMaterialMap::iterator	it;
	mi->_AnimatedMaterials.reserve(_AnimatedMaterials.size());
	for(it= _AnimatedMaterials.begin(); it!= _AnimatedMaterials.end(); it++)
	{
		CAnimatedMaterial	aniMat(&it->second);

		// set the target instance material.
		nlassert(it->first < mi->Materials.size());
		aniMat.setMaterial(&mi->Materials[it->first]);

		// Must set the Animatable father of the animated material (the mesh_base_instance!).
		aniMat.setFather(mi, CMeshBaseInstance::OwnerBit);

		// Append this animated material.
		mi->_AnimatedMaterials.push_back(aniMat);
	}

	// Misc
	//==========================
	
	// Setup position with the default value
	mi->ITransformable::setPos( ((CAnimatedValueVector&)_DefaultPos.getValue()).Value  );
	mi->ITransformable::setRotQuat( ((CAnimatedValueQuat&)_DefaultRotQuat.getValue()).Value  );
	mi->ITransformable::setScale( ((CAnimatedValueVector&)_DefaultScale.getValue()).Value  );
	mi->ITransformable::setPivot( ((CAnimatedValueVector&)_DefaultPivot.getValue()).Value  );

	// Check materials for transparency
	mi->setTransparency( false );
	mi->setOpacity( false );
	for( i = 0; i < mi->Materials.size(); ++i )
	if( mi->Materials[i].getBlend() )
		mi->setTransparency( true );
	else
		mi->setOpacity( true );

	// if the mesh is lightable, then the instance is
	mi->setIsLightable(this->isLightable());

	// a mesh is considered big for lightable if it uses localAttenuation
	mi->setIsBigLightable(this->useLightingLocalAttenuation());

}

// ***************************************************************************
void	CMeshBase::flushTextures(IDriver &driver)
{
	// Mat count
	uint matCount=_Materials.size();

	// Flush each material textures
	for (uint mat=0; mat<matCount; mat++)
	{
		/// Flush material textures
		_Materials[mat].flushTextures (driver);
	}
}


// ***************************************************************************
void	CMeshBase::computeIsLightable()
{
	// by default the mesh is not lightable
	_IsLightable= false;

	// Mat count
	uint matCount=_Materials.size();

	// for each material 
	for (uint mat=0; mat<matCount; mat++)
	{
		// if this one is not a lightmap, then OK, the mesh is lightable
		if( _Materials[mat].getShader()!=CMaterial::LightMap )
		{
			_IsLightable= true;
			break;
		}
	}
}


// ***************************************************************************
bool	CMeshBase::useLightingLocalAttenuation () const
{
	return _UseLightingLocalAttenuation;
}


// ***************************************************************************
void	CMeshBase::resetLodCharacterTexture()
{
	if(_LodCharacterTexture)
	{
		delete _LodCharacterTexture;
		_LodCharacterTexture= NULL;
	}
}

// ***************************************************************************
void	CMeshBase::setupLodCharacterTexture(CLodCharacterTexture &lodText)
{
	// delete old
	resetLodCharacterTexture();
	// seutp new
	_LodCharacterTexture= new CLodCharacterTexture;
	*_LodCharacterTexture= lodText;
}


} // NL3D
