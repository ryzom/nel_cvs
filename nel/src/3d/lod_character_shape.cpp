/** \file lod_character_shape.cpp
 * <File description>
 *
 * $Id: lod_character_shape.cpp,v 1.3 2002/05/15 16:55:55 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "3d/lod_character_shape.h"
#include "nel/misc/vectord.h"
#include "3d/fast_floor.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
void	CLodCharacterShapeBuild::serial(NLMISC::IStream &f)
{
	// NEL_CLODBULD
	f.serialCheck((uint32)'_LEN');
	f.serialCheck((uint32)'DOLC');
	f.serialCheck((uint32)'DLUB');

	sint	ver= f.serialVersion(0);

	f.serialCont(Vertices);
	f.serialCont(SkinWeights);
	f.serialCont(BonesNames);
	f.serialCont(TriangleIndices);
}


// ***************************************************************************
CLodCharacterShape::CLodCharacterShape()
{
	_NumVertices= 0;
	_NumTriangles= 0;
}

// ***************************************************************************
void			CLodCharacterShape::buildMesh(const std::string &name, const CLodCharacterShapeBuild &lodBuild)
{
	uint	numVertices= lodBuild.Vertices.size();
	const vector<uint32>				&triangleIndices= lodBuild.TriangleIndices;
	const vector<CMesh::CSkinWeight>	&skinWeights= lodBuild.SkinWeights;

	nlassert(numVertices>0);
	nlassert(triangleIndices.size()>0);
	nlassert((triangleIndices.size()%3)==0);
	nlassert(skinWeights.size() == numVertices);

	// reset data
	contReset(_Anims);
	contReset(_AnimMap);
	contReset(_Bones);
	contReset(_BoneMap);
	contReset(_TriangleIndices);

	// Copy data.
	_Name= name;
	_NumVertices= numVertices;
	_NumTriangles= triangleIndices.size()/3;
	_TriangleIndices= triangleIndices;

	// check indices.
	uint	i;
	for(i=0;i<triangleIndices.size();i++)
	{
		nlassert(triangleIndices[i]<_NumVertices);
	}

	// Copy bone names, and compute bone Map
	_Bones.resize(lodBuild.BonesNames.size());
	for(i=0; i<_Bones.size(); i++)
	{
		_Bones[i].Name= lodBuild.BonesNames[i];
		_BoneMap.insert( make_pair(_Bones[i].Name, i) );
	}

	// "Normalize" SkinWeights for CLodCharacterShape
	for(i=0;i<skinWeights.size();i++)
	{
		nlassert(skinWeights[i].Weights[0]>0);
		// for all slots not 0
		for(uint j=0;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
		{
			// if this this slot is used.
			if(skinWeights[i].Weights[j]>0)
			{
				uint boneId= skinWeights[i].MatrixId[j];
				nlassert(boneId < _Bones.size());
				// init the vInf data
				CVertexInf	vInf;
				vInf.VertexId= i;
				vInf.Influence= skinWeights[i].Weights[j];
				// Insert this vertex influence in the bone.
				_Bones[boneId].InfVertices.push_back(vInf);
			}
			else
				// stop for this vertex.
				break;
		}
	}
}

// ***************************************************************************
bool			CLodCharacterShape::addAnim(const CAnimBuild &animBuild)
{
	// first, verify don't exist.
	if(getAnimIdByName(animBuild.Name)!=-1)
		return false;

	// build basics of the animation
	CAnim	dstAnim;
	dstAnim.Name= animBuild.Name;
	dstAnim.AnimLength= animBuild.AnimLength;
	nlassert(dstAnim.AnimLength>0);
	dstAnim.OOAnimLength= 1.0f / animBuild.AnimLength;
	dstAnim.NumKeys= animBuild.NumKeys;
	// verify size of the array
	nlassert(dstAnim.NumKeys>0);
	nlassert(dstAnim.NumKeys * _NumVertices == animBuild.Keys.size());
	// resize dest array
	dstAnim.Keys.resize(animBuild.Keys.size());


	// Pack animation. 1st pass: compute max size over the animation vertices
	uint	i;
	// minimum shape size is , say, 1 cm :)
	CVector		maxSize(0.01f, 0.01f, 0.01f);
	for(i=0;i<animBuild.Keys.size();i++)
	{
		// take the maxSize of the abs values
		maxSize.maxof(maxSize, -animBuild.Keys[i]);
		maxSize.maxof(maxSize, animBuild.Keys[i]);
	}

	// compute the UnPackScaleFactor ie maxSize, to be multiplied by max Abs value of a sint16
	dstAnim.UnPackScaleFactor= maxSize * (1.0f/32767);

	// Pack animation. 2st pass: pack.
	CVectorD		packScaleFactor;
	packScaleFactor.x= 1.0 / dstAnim.UnPackScaleFactor.x;
	packScaleFactor.y= 1.0 / dstAnim.UnPackScaleFactor.y;
	packScaleFactor.z= 1.0 / dstAnim.UnPackScaleFactor.z;
	// For all key vertices
	for(i=0;i<animBuild.Keys.size();i++)
	{
		CVector		v= animBuild.Keys[i];
		CVector3s	&dstV= dstAnim.Keys[i];

		// compress
		v.x= float(v.x*packScaleFactor.x);
		v.y= float(v.y*packScaleFactor.y);
		v.z= float(v.z*packScaleFactor.z);
		// clamp to sint16 limits (for float precision problems).
		clamp(v.x, -32767, 32767);
		clamp(v.y, -32767, 32767);
		clamp(v.z, -32767, 32767);
		// get into the vector3s
		dstV.x= (sint16)floor(v.x);
		dstV.y= (sint16)floor(v.y);
		dstV.z= (sint16)floor(v.z);
	}


	// Add the anim to the array, and add an entry to the map
	_Anims.push_back(dstAnim);
	_AnimMap.insert(make_pair(dstAnim.Name, _Anims.size()-1));

	return true;
}

// ***************************************************************************
void			CLodCharacterShape::CAnim::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serial(Name);
	f.serial(NumKeys);
	f.serial(AnimLength);
	f.serial(OOAnimLength);
	f.serial(UnPackScaleFactor);
	f.serialCont(Keys);
}


// ***************************************************************************
void			CLodCharacterShape::CBoneInfluence::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serial(Name);
	f.serialCont(InfVertices);
}


// ***************************************************************************
void			CLodCharacterShape::serial(NLMISC::IStream &f)
{
	// NEL_CLODSHAP
	f.serialCheck((uint32)'_LEN');
	f.serialCheck((uint32)'DOLC');
	f.serialCheck((uint32)'PAHS');

	sint	ver= f.serialVersion(0);

	f.serial(_Name);
	f.serial(_NumVertices);
	f.serial(_NumTriangles);
	f.serialCont(_Bones);
	f.serialCont(_BoneMap);
	f.serialCont(_TriangleIndices);
	f.serialCont(_Anims);
	f.serialCont(_AnimMap);
}

// ***************************************************************************
sint			CLodCharacterShape::getAnimIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _AnimMap.find(name);
	if(it == _AnimMap.end())
		return -1;
	else
		return it->second;
}


// ***************************************************************************
sint			CLodCharacterShape::getBoneIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _BoneMap.find(name);
	if(it == _BoneMap.end())
		return -1;
	else
		return it->second;
}


// ***************************************************************************
const uint32	*CLodCharacterShape::getTriangleArray() const
{
	if(_NumTriangles)
		return &_TriangleIndices[0];
	else
		return NULL;
}

// ***************************************************************************
const CLodCharacterShape::CVector3s	*CLodCharacterShape::getAnimKey(uint animId, TGlobalAnimationTime time, bool wrapMode, CVector &unPackScaleFactor) const
{
	float	localTime;

	if(animId>=_Anims.size())
		return NULL;

	// get the anim.
	const CAnim &anim= _Anims[animId];

	// scale info
	unPackScaleFactor= anim.UnPackScaleFactor;

	// Loop mgt.
	if(wrapMode)
		localTime= (float)fmod(time, anim.AnimLength);
	else
		localTime= (float)time;

	// Clamp to the range.
	clamp(localTime, 0, anim.AnimLength);

	// get the key.
	sint	keyId= (sint)floor( (localTime/anim.AnimLength) * anim.NumKeys );
	clamp(keyId, 0, sint(anim.NumKeys-1));

	// return the key.
	return &anim.Keys[keyId * _NumVertices];
}


// ***************************************************************************
// ***************************************************************************
// Bone coloring
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLodCharacterShape::startBoneColor(std::vector<NLMISC::CRGBAF>	&tmpColors) const
{
	// alocate
	tmpColors.resize(getNumVertices());
	// fill with black (in Alpha too)
	CRGBAF	black(0,0,0,0);
	fill(tmpColors.begin(), tmpColors.end(), black);
}

// ***************************************************************************
void			CLodCharacterShape::addBoneColor(uint boneId, CRGBA	color, std::vector<NLMISC::CRGBAF> &tmpColors) const
{
	nlassert(boneId<_Bones.size());
	const CBoneInfluence	&bone= _Bones[boneId];

	// for all vertices influenced by this bone, must influence the color
	for(uint i=0; i<bone.InfVertices.size(); i++)
	{
		const CVertexInf	&vInf= bone.InfVertices[i];
		// Add the color, modulated by the vertex weight
		tmpColors[vInf.VertexId].R+= color.R * vInf.Influence;
		tmpColors[vInf.VertexId].G+= color.G * vInf.Influence;
		tmpColors[vInf.VertexId].B+= color.B * vInf.Influence;
		// Add the weight in alpha channel
		tmpColors[vInf.VertexId].A+= vInf.Influence;
	}
}

// ***************************************************************************
void			CLodCharacterShape::endBoneColor(const std::vector<NLMISC::CRGBAF> &tmpColors, std::vector<NLMISC::CRGBA>	&dstColors) const
{
	// The default value if vertex is not influenced at all
	CRGBA	defaultColor(128,128,128,0);

	// resize dstColors
	nlassert(tmpColors.size() == getNumVertices());
	dstColors.resize(getNumVertices());

	// For all vertices
	for(uint i=0; i<dstColors.size(); i++)
	{
		// If the vertex is not influenced, set transparent
		if(tmpColors[i].A==0)
			dstColors[i]= defaultColor;
		else
		{
			// Average the color.
			float	R= tmpColors[i].R / tmpColors[i].A;
			float	G= tmpColors[i].G / tmpColors[i].A;
			float	B= tmpColors[i].B / tmpColors[i].A;
			// store.
			dstColors[i].R= (uint8)OptFastFloor(R);
			dstColors[i].G= (uint8)OptFastFloor(G);
			dstColors[i].B= (uint8)OptFastFloor(B);
			dstColors[i].A= 255;
		}
	}
}



} // NL3D
