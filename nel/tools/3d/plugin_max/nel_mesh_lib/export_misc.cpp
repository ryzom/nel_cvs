/** \file export_misc.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_misc.cpp,v 1.5 2001/07/02 16:30:58 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stdafx.h"
#include "export_nel.h"
#include <3d/texture_file.h>
#include <modstack.h> // class IDerivedObject
#include <decomp.h> // class IDerivedObject

using namespace NLMISC;
using namespace NL3D;

// --------------------------------------------------

const char* CExportNel::ErrorMessage[CodeCount]=
{
	"No error.",
	"Some vertices of the skin are not assigned.",
	"The skin doesn't use the same skeleton.",
};

// --------------------------------------------------

CExportNel::CExportNel()
{
}

// --------------------------------------------------

/// Transforme a 3dsmax view matrix to camera matrix.
Matrix3 CExportNel::viewMatrix2CameraMatrix (const Matrix3& viewMatrix)
{
	// Init the return matrix
	Matrix3	mat;
	mat.Invert();

	// Get invert viewMatrix
	// We invert view matrix to get camera matrix in viewport space
	Matrix3	invertViewMatrix=viewMatrix;
	invertViewMatrix.Invert();

	// This matrix is used to compute the coordinates to the viewport space. Take ths invert.
	Matrix3 viewportInvert;
	viewportInvert.SetRow (0, Point3(1.f, 0.f, 0.f));
	viewportInvert.SetRow (1, Point3(0.f, 0.f, 1.f));
	viewportInvert.SetRow (2, Point3(0.f, -1.f, 0.f));
	viewportInvert.SetRow (3, Point3(0.f, 0.f, 0.f));
	viewportInvert.Invert();

	// Compose with invert viewport matrix
	mat=viewportInvert*invertViewMatrix;

	// return the computed matrix
	return mat;
}

// --------------------------------------------------

// ConvertMatrix a 3dsmax matrix in NeL matrix
void CExportNel::convertMatrix (CMatrix& nelMatrix, const Matrix3& maxMatrix)
{
	// Basis vector
	CVector I, J, K, P;

	// Build the rot matrix
	I.x= maxMatrix.GetRow(0).x;
	I.y= maxMatrix.GetRow(0).y;
	I.z= maxMatrix.GetRow(0).z;
	J.x= maxMatrix.GetRow(1).x;
	J.y= maxMatrix.GetRow(1).y;
	J.z= maxMatrix.GetRow(1).z;
	K.x= maxMatrix.GetRow(2).x;
	K.y= maxMatrix.GetRow(2).y;
	K.z= maxMatrix.GetRow(2).z;

	// Build the translation vector
	P.x= maxMatrix.GetTrans().x;
	P.y= maxMatrix.GetTrans().y;
	P.z= maxMatrix.GetTrans().z;

	// *** Build the NeL matrix

	// Set it to identity to have the good flags in it
	nelMatrix.identity();

	// Set the rotation part
	nelMatrix.setRot(I, J, K);

	// Set the position part
	nelMatrix.setPos(P);
}

// --------------------------------------------------

// Convert a 3dsmax color in NeL color
void CExportNel::convertColor (CRGBA& nelColor, const Color& maxColor)
{
	// Convert from max to Nel
	float fR=maxColor.r*255.f+0.5f;
	float fG=maxColor.g*255.f+0.5f;
	float fB=maxColor.b*255.f+0.5f;
	clamp (fR, 0.f, 255.f);
	clamp (fG, 0.f, 255.f);
	clamp (fB, 0.f, 255.f);
	nelColor.R=(uint8)fR;
	nelColor.G=(uint8)fG;
	nelColor.B=(uint8)fB;

	// Alpha
	nelColor.A=255;
}

// --------------------------------------------------

// Return true if the texmap is a texture file else false
bool CExportNel::isClassIdCompatible (Animatable& anim, Class_ID& classId)
{
	// Check if it is a file texmap
	if	(
			(anim.ClassID() == classId)										||
			((anim.SuperClassID() == classId.PartA())&&(classId.PartB()==0))
		)
	{
		return true;
	}
	return false;
}

// --------------------------------------------------

// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
Animatable*	CExportNel::getSubAnimByName (Animatable& node, const char* sName)
{
	// Search for the subanim
	for (int nSub=0; nSub<node.NumSubs(); nSub++)
	{
		// Get the name
		TSTR sSubName=node.SubAnimName(nSub);

		// Good name?
		if (strcmp (sSubName, sName)==0)
		{
			// ok, return this subanim
			return node.SubAnim(nSub);
		}

		// Recurse in the sub anim..
		Animatable* subSearch=getSubAnimByName (*node.SubAnim(nSub), sName);

		// Found? return it, else continue
		if (subSearch)
			return subSearch;
	}
	// not found
	return NULL;
}

// --------------------------------------------------

// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
Control* CExportNel::getControlerByName (Animatable& node, const char* sName)
{
	// For debug
	TSTR nodeName;
	node.GetClassName (nodeName);

	// for all parameters block in this node
	for (int nBlock=0; nBlock<node.NumParamBlocks(); nBlock++)
	{
		// Get the paramblock2 pointer
		IParamBlock2 *param=node.GetParamBlock(nBlock);

		// Search for the subanim
		for (int nSub=0; nSub<param->NumParams(); nSub++)
		{
			// Index of the parameter
			ParamID id=param->IndextoID(nSub);

			// Get the paramDef
			ParamDef& paramDef=param->GetParamDef(id);

			// Good name?
			if (strcmp (paramDef.int_name, sName)==0)
			{
				// ok, return this subanim
				return param->GetController(id);
			}
		}
	}

	// Get num sub anim
	uint numSubAnim=node.NumSubs();

	// Visit sub anim
	for (uint s=0; s<numSubAnim; s++)
	{
		// ok, return this subanim controler
		if (node.SubAnim(s))
		{
			// Sub anim name
			TSTR name=node.SubAnimName (s);
			if (strcmp (name, sName)==0)
			{
				// Get the controller pointer of this sub anim
				Control* c=GetControlInterface (node.SubAnim(s));
				if (c)
					return c;
			}
		}
		// If a sub anim is here, go to visit it.
		if (node.SubAnim(s))
		{
			// Get the ctrl for sub anim
			Control* c=getControlerByName (*node.SubAnim(s), sName);

			// If it exists, return it
			if (c)
				return c;
		}
	}

	// not found
	return NULL;
}

// --------------------------------------------------

// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
bool CExportNel::getValueByNameUsingParamBlock2 (Animatable& node, const char* sName, ParamType2 type, void *pValue, TimeValue tvTime)
{
	// for all parameters block in this node
	for (int nBlock=0; nBlock<node.NumParamBlocks(); nBlock++)
	{
		// Get the paramblock2 pointer
		IParamBlock2 *param=node.GetParamBlock(nBlock);

		// Search for the subanim
		for (int nSub=0; nSub<param->NumParams(); nSub++)
		{
			// Index of the parameter
			ParamID id=param->IndextoID(nSub);

			// Get the paramDef
			ParamDef& paramDef=param->GetParamDef(id);

			// Good name?
			if (strcmp (paramDef.int_name, sName)==0)
			{
				// Check this value is good type
				if (param->GetParameterType(id)==type)
				{
					// Get the value
					Interval ivalid=FOREVER;
					BOOL bRes=FALSE;
					switch (type)
					{
					case TYPE_PCNT_FRAC:
					case TYPE_FLOAT:
						bRes=param->GetValue(id, tvTime, *(float*)pValue, ivalid);
						break;
					case TYPE_BOOL:
					case TYPE_INT:
						bRes=param->GetValue(id, tvTime, *(int*)pValue, ivalid);
						break;
					case TYPE_RGBA:
					case TYPE_POINT3:
						bRes=param->GetValue(id, tvTime, *(Point3*)pValue, ivalid);
						break;
					case TYPE_STRING:
						//bRes=param->GetValue(id, tvTime, *(TCHAR**)pValue, ivalid);
						*(std::string*)pValue = param->GetStr(id, tvTime);
						break;
					case TYPE_STRING_TAB:
						{
							std::vector<std::string> &rTab = *(std::vector<std::string>*)pValue;
							uint total = param->Count(id);
							rTab.resize(total);
							for( uint i = 0; i < total; ++i )
								rTab[i] = param->GetStr(id, tvTime, i);
						}
						break;
					}

					// Get successful ?
					if (bRes)
						return true;
				}
			}
		}
	}

		// Get num sub anim
	uint numSubAnim=node.NumSubs();
	// Visit sub anim
	for (uint s=0; s<numSubAnim; s++)
	{
		// If a sub anim is here, go to visit it.
		if (node.SubAnim(s))
		{
			// Get the ctrl for sub anim
			if( getValueByNameUsingParamBlock2 (*node.SubAnim(s), sName, type, pValue, tvTime) )
				return true;
		}
	}


	// not found
	return false;
}

// --------------------------------------------------

// Get the first modifier in the pipeline of a node by its class identifier
Modifier* CExportNel::getModifier (INode* pNode, Class_ID modCID)
{
	Object* pObj = pNode->GetObjectRef();

	if (!pObj)
		return NULL;

	ObjectState os = pNode->EvalWorldState(0);
	if (os.obj && (os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) && (os.obj->SuperClassID() != LIGHT_CLASS_ID) )
	{
		return NULL;
	}

	// For all derived objects (can be > 1)
	while (pObj && (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
	{
		IDerivedObject* pDObj = (IDerivedObject*)pObj;
		int m;
		int nNumMods = pDObj->NumModifiers();
		// Step through all modififers and verify the class id
		for (m=0; m<nNumMods; ++m) 
		{
			Modifier* pMod = pDObj->GetModifier(m);
			if (pMod) 
			{
				if (pMod->ClassID() == modCID) 
				{
					// Match! Return it
					return pMod;
				}
			}
		}
		pObj = pDObj->GetObjRef();
	}

	return NULL;
}

// --------------------------------------------------

// Get the material name
std::string	CExportNel::getName (MtlBase& mtl)
{
	// Return its name
	TSTR name;
	name=mtl.GetName();
	return std::string (name);
}

// --------------------------------------------------

// Get the node name
std::string	CExportNel::getName (INode& mtl)
{
	// Return its name
	TCHAR* name=mtl.GetName();
	return std::string (name);
}

// --------------------------------------------------

void CExportNel::decompMatrix (NLMISC::CVector& nelScale, NLMISC::CQuat& nelRot, NLMISC::CVector& nelPos, 
								const Matrix3& maxMatrix)
{
	// Use decomp part of the max SDK
	AffineParts parts;
	decomp_affine(maxMatrix, &parts);

	// Set the translation
	nelPos.x=parts.t.x;
	nelPos.y=parts.t.y;
	nelPos.z=parts.t.z;

	// Set the rotation
	nelRot.x=parts.q.x;
	nelRot.y=parts.q.y;
	nelRot.z=parts.q.z;
	nelRot.w=-parts.q.w;

	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	parts.u.MakeMatrix(srtm);
	stm = ScaleMatrix(parts.k);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);

	// Take the scale of this matrix, not very smart but..
	nelScale.x=scaleMatrix.getI().x;
	nelScale.y=scaleMatrix.getJ().y;
	nelScale.z=scaleMatrix.getK().z;
}

// --------------------------------------------------

void CExportNel::getLocalMatrix (Matrix3& localMatrix, INode& node, TimeValue time)
{
	INode *parent;
	Matrix3 parentTM, nodeTM;
	nodeTM = node.GetNodeTM (time);
	parent = node.GetParentNode ();
	if (parent)
	{
		parentTM = parent->GetNodeTM (time);
		localMatrix = nodeTM*Inverse (parentTM);
	}
	else
		localMatrix = nodeTM;
}

// --------------------------------------------------

bool CExportNel::isMesh (INode& node, TimeValue time)
{
	// Result false by default
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Object can convert itself to NeL patchmesh ?
		if (os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
			bRet=true;
	}

	// Return result
	return bRet;
}

// --------------------------------------------------

bool CExportNel::hasLightMap (INode& node, TimeValue time)
{
	if( !isMesh(node,time) )
		return false;

	int nMaterialCount=0;

	// Get primary material pointer of the node
	Mtl* pNodeMat=node.GetMtl();

	// If NULL, no material at all at this node
	if (pNodeMat==NULL)
		return false;

	// Number of sub material at in this material
	nMaterialCount=pNodeMat->NumSubMtls();

	// If it is a multisub object, export all its sub materials
	if (nMaterialCount>0)
	{
		// Check all the sub materials
		for (int nSub=0; nSub<nMaterialCount; nSub++)
		{
			// Get a pointer on the sub material
			Mtl* pSub=pNodeMat->GetSubMtl(nSub);

			// Should not be NULL
			nlassert (pSub);

			// Is there a lightmap handling wanted
			int bLightMap = 0; // false
			CExportNel::getValueByNameUsingParamBlock2 (*pSub, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, 0);
			if (bLightMap)
				return true;
		}
	}
	// Else check only this material
	else
	{
		int bLightMap = 0; // false
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, 0);
		if (bLightMap)
			return true;
	}
	return false;
}