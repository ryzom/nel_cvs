/** \file mesh_base.cpp
 * <File description>
 *
 * $Id: mesh_base.cpp,v 1.2 2001/06/15 14:34:56 berenguier Exp $
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

#include "nel/3d/mesh_base.h"


namespace NL3D 
{



// ***************************************************************************
CMeshBase::CMeshBase()
{
	// To have same functionnality than previous version, init to identity.
	_DefaultPos.setValue(CVector(0,0,0));
	_DefaultPivot.setValue(CVector(0,0,0));
	_DefaultRotEuler.setValue(CVector(0,0,0));
	_DefaultRotQuat.setValue(CQuat::Identity);
	_DefaultScale.setValue(CVector(1,1,1));
}


// ***************************************************************************
// ***************************************************************************
// Animated material.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBase::setAnimatedMaterial(uint id, const std::string &matName)
{
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
void	CMeshBase::CMeshBaseBuild::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	f.serial( DefaultPos );
	f.serial( DefaultPivot );
	f.serial( DefaultRotEuler );
	f.serial( DefaultRotQuat );
	f.serial( DefaultScale );

	f.serialCont( Materials );
}


// ***************************************************************************
void	CMeshBase::serialMeshBase(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	f.serial (_DefaultPos);
	f.serial (_DefaultPivot);
	f.serial (_DefaultRotEuler);
	f.serial (_DefaultRotQuat);
	f.serial (_DefaultScale);

	f.serialCont(_Materials);
	f.serialCont(_AnimatedMaterials);
	f.serialCont(_LightInfos);
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

}



} // NL3D
