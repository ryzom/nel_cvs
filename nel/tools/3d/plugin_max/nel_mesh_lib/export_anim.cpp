/** \file export_anim.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_anim.cpp,v 1.30 2002/05/24 16:13:17 berenguier Exp $
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
#include <3d/camera.h>
#include <3d/transform.h>
#include <3d/animation.h>
#include <3d/animated_material.h>
#include <3d/key.h>
#include <3d/track.h>
#include <3d/particle_system_model.h>
#include <notetrck.h>

#include "calc_lm.h"
#include "export_lod.h"

using namespace NLMISC;
using namespace NL3D;

static Class_ID DefNoteTrackClassID(NOTETRACK_CLASS_ID, 0);

#define BOOL_CONTROL_CLASS_ID 0x984b8d27

// --------------------------------------------------

class CExportDesc
{
public:

	// Default cstr;
	CExportDesc ()
	{
		reset ();
	}

	// Reset the desc
	void reset ()
	{
		Specular=false;
	}


	// This value is specular
	bool		Specular;
	StdMat2		*Material;
};

// --------------------------------------------------

// Add track in this animation
void CExportNel::addAnimation (CAnimation& animation, INode& node, const char* sBaseName, bool root)
{
	// Get the TM controler
	Control *transform=node.GetTMController();

	// Is it a biped node ?
	if (transform && (transform->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		// Export biped skeleton animation
		addBipedNodeTracks (animation, node, sBaseName, root);
	}	
	else
	{
		// Add node tracks
		addNodeTracks (animation, node, sBaseName, NULL, NULL, root);

		// Get the object pointer
		Object* obj=node.GetObjectRef();

		// Export the object if it exists
		if (obj)
			addObjTracks (animation, *obj, sBaseName);

		// Export material tracks of this object
		Mtl* mtl=node.GetMtl();
		if (mtl)
		{
			// Add material tracks in the animation
			addMtlTracks (animation, *mtl, sBaseName);
		}

		// Add bones tracks
		//addBonesTracks (animation, node, sBaseName);

		// Add light tracks
		addLightTracks (animation, node, sBaseName);

		// Add particle system tracks
		addParticleSystemTracks(animation, node, sBaseName);

		// Add morph tracks
		addMorphTracks (animation, node, sBaseName);

		// Add bones track
		uint childrenCont=(uint)node.NumberOfChildren();
		for (uint children=0; children<childrenCont; children++)
		{
			INode *child=node.GetChildNode(children);
			addBoneTracks (animation, *child, sBaseName, false);
		}
	}

	// check for note track export (a string track used to create events)
	int exportNoteTrack = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, -1);	

	if (exportNoteTrack)
	{
		addNoteTrack(animation, node);		
	}
}

// --------------------------------------------------

void CExportNel::addNoteTrack(NL3D::CAnimation& animation, INode& node)
{
	// check for the first Note Track		
	NoteTrack *nt = node.GetNoteTrack(0);

					
    if(nt && (nt->ClassID() == DefNoteTrackClassID))
    {

		CTrackKeyFramerConstString *st = new CTrackKeyFramerConstString;

        DefNoteTrack &dnt = *(DefNoteTrack *)nt;
        int noteCount = dnt.keys.Count();
		float firstDate = 0, lastDate = 0;

        for(int noteIndex = 0; noteIndex < noteCount; ++noteIndex)
        {

            NoteKey *note = dnt.keys[noteIndex];

            if(note)

            {
				CKeyString ks;
				if (noteIndex == 0)
				{
					firstDate = CExportNel::convertTime (note->time);
				}				
				ks.Value = std::string(note->note);
				lastDate = CExportNel::convertTime (note->time);
				st->addKey(ks , lastDate );
				
            }
        }
		st->unlockRange (firstDate, lastDate);
		animation.addTrack(std::string("NoteTrack"), st);		
	}	
	
	

}

// --------------------------------------------------
void CExportNel::addParticleSystemTracks(CAnimation& animation, INode& node, const char* parentName)
{	
	Class_ID  clid = node.GetObjectRef()->ClassID() ;
	/// is this a particle system ?
	if (clid.PartA() != NEL_PARTICLE_SYSTEM_CLASS_ID)
		return ;

	// Export desc
	CExportDesc desc;
	const char *paramName[] = { "PSParam0", "PSParam1", "PSParam2", "PSParam3" } ;
	

	for (uint k = 0 ; k < 4 ; ++k)
	{
		Control *ctrl = getControlerByName(node, paramName[k]) ;
		if (ctrl)
		{
			ITrack *pTrack=buildATrack (animation, *ctrl, typeFloat, node, desc, NULL, NULL);
			if (pTrack)
			{
				std::string name=parentName+std::string (NL3D::CParticleSystemModel::getPSParamName((uint) NL3D::CParticleSystemModel::PSParam0 + k));
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}	
	}

	Control *ctrl = getControlerByName(node, "PSTrigger");
	if (ctrl)
	{
		ITrack *pTrack = buildOnOffTrack(*ctrl);
		std::string name=parentName+std::string("PSTrigger");
		if (animation.getTrackByName (name.c_str()))
		{
			delete pTrack;
		}
		else
		{
			animation.addTrack(name, pTrack);
		}
	}
}


// --------------------------------------------------

void CExportNel::addNodeTracks (CAnimation& animation, INode& node, const char* parentName,
								std::set<TimeValue>* previousKeys, std::set<TimeValue>* previousKeysSampled, bool root, bool bodyBiped)
{
	// Tmp track*
	ITrack *pTrack;

	// Tmp name
	std::string name;

	// Export desc
	CExportDesc desc;

	// Export scale
	desc.reset();

	// Get the transformation controler
	Control *transform=node.GetTMController();

	// It exists ?
	if (transform)
	{
		// Get the Scale controler
		Control *c=transform->GetScaleController ();
		if (c)
		{
			// Build the track
			pTrack=buildATrack (animation, *c, typeScale, node, desc, previousKeys, previousKeysSampled, bodyBiped);
			if (pTrack)
			{
				name=parentName+std::string (ITransformable::getScaleValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}

		// Export rotation
		desc.reset();

		// Get the Rotation controler
		c=transform->GetRotationController ();
		if (c)
		{
			pTrack=buildATrack (animation, *c, typeRotation, node, desc, previousKeys, previousKeysSampled, bodyBiped);
			if (pTrack)
			{
				// Choose the good name for this track
				name=parentName+std::string (ITransformable::getRotQuatValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}

		// Export position
		desc.reset();

		// Get the Position controler
		c=transform->GetPositionController ();
		if (c)
		{
			pTrack=buildATrack (animation, *c, typePos, node, desc, previousKeys, previousKeysSampled, bodyBiped);
			if (pTrack)
			{
				// Choose the good name for this track
				name=parentName+std::string (ITransformable::getPosValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
		else
		{
			// try to find a biped controller for position
			pTrack=buildATrack (animation, *transform, typePos, node, desc, previousKeys, previousKeysSampled, bodyBiped);
			if (pTrack)
			{
				// Choose the good name for this track
				name=parentName+std::string (ITransformable::getPosValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}

		// Export roll for camera
		desc.reset();

		// Get the Roll controler
		c=transform->GetRollController ();
		if (c)
		{
			pTrack=buildATrack (animation, *c, typeFloat, node, desc, previousKeys, previousKeysSampled, bodyBiped);
			if (pTrack)
			{
				name=parentName+std::string (CCamera::getRollValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addBipedNodeTracks (CAnimation& animation, INode& node, const char* parentName, bool root)
{
	// Get the matrix controler
	Control *transform=node.GetTMController();
	nlassert (transform);
	nlassert (transform->ClassID() == BIPBODY_CONTROL_CLASS_ID);

	// Backup inplace y mode
	bool inPlaceMode;
	bool inPlaceYMode;
	bool inPlaceXMode;
	bool res = getBipedInplaceMode (getName (node).c_str(), "inPlaceYMode", inPlaceYMode);
	nlassert (res);
	res = getBipedInplaceMode (getName (node).c_str(), "inPlaceXMode", inPlaceXMode);
	nlassert (res);
	res = getBipedInplaceMode (getName (node).c_str(), "inPlaceMode", inPlaceMode);
	nlassert (res);

	// No inplace mode
	res = setBipedInplaceMode (getName (node).c_str(), "inPlaceMode", false);
	res = setBipedInplaceMode (getName (node).c_str(), "inPlaceXMode", false);
	res = setBipedInplaceMode (getName (node).c_str(), "inPlaceYMode", false);

	// Export animations for the biped except the root
	std::set<TimeValue> keysSampled;
	std::set<TimeValue> keys;
	addBipedNodeTrack (animation, node, parentName, keys, keysSampled, root);

	// Export root animation in special tracks for interactive interpolation
	//addBipedPathTrack (animation, node, parentName);

	// Set default mode
	res = setBipedInplaceMode (getName (node).c_str(), "inPlaceYMode", inPlaceYMode);
	res = setBipedInplaceMode (getName (node).c_str(), "inPlaceXMode", inPlaceXMode);
	res = setBipedInplaceMode (getName (node).c_str(), "inPlaceMode", inPlaceMode);
}

// --------------------------------------------------
/*
void CExportNel::addBipedPathTrack (CAnimation& animation, INode& node, const char* parentName)
{
	// Get the matrix controler
	Control *transform=node.GetTMController();
	nlassert (transform);
	nlassert (transform->ClassID() == BIPBODY_CONTROL_CLASS_ID);

	//Get the Biped Export Interface from the controller 
	IBipedExport *BipIface = (IBipedExport *) transform->GetInterface(I_BIPINTERFACE);
	nlassert (BipIface);

	// Remove the non uniform scale
	BipIface->RemoveNonUniformScale(1);

	// Export desc
	CExportDesc desc;

	// Export position
	desc.reset();

	// Get keys position for biped tracks
	std::set<TimeValue> keysSampled;
	std::set<TimeValue> keys;

	// Add keys
	addBipedKeyTime (*transform, keys, keysSampled, true, getName (*(INode*)&node).c_str());

	// Get the position controler
	ITrack *pTrack;
	pTrack=buildATrack (animation, *transform, typePos, node, desc, &keys, &keysSampled);
	if (pTrack)
	{
		// Name path track
		std::string name=parentName+std::string (BIPED_PATH_POS_NAME);
		if (animation.getTrackByName (name.c_str()))
		{
			// Delete this track because it already exist.
			delete pTrack;
		}
		else
		{
			// Add tracks in the animation
			animation.addTrack (name.c_str(), pTrack);
		}
	}

	// Get the rotation controler
	pTrack=buildATrack (animation, *transform, typeRotation, node, desc, &keys, &keysSampled);
	if (pTrack)
	{
		// Name path track
		std::string name=parentName+std::string (BIPED_PATH_ROT_NAME);
		if (animation.getTrackByName (name.c_str()))
		{
			// Delete this track because it already exist.
			delete pTrack;
		}
		else
		{
			// Add tracks in the animation
			animation.addTrack (name.c_str(), pTrack);
		}
	}

	// Restaure the non uniform scale
	BipIface->RemoveNonUniformScale(0);
}
*/
// --------------------------------------------------

void CExportNel::addBipedNodeTrack (CAnimation& animation, INode& node, const char* parentName,
									std::set<TimeValue>& previousKeys, std::set<TimeValue>& previousKeysSampled, bool root)
{
	// Get the matrix controler
	Control *transform=node.GetTMController();
	nlassert (transform);

	// Biped node
	if ((transform->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
		(transform->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		//Get the Biped Export Interface from the controller 
		IBipedExport *BipIface = (IBipedExport *) transform->GetInterface(I_BIPINTERFACE);
		nlassert (BipIface);

		// Remove the non uniform scale
		BipIface->RemoveNonUniformScale(1);

		// Get keys position for biped tracks
		std::set<TimeValue> keysSampled=previousKeys;
		std::set<TimeValue> keys=previousKeys;

		// Body biped ?
		bool bodyBiped = ((transform->ClassID() == BIPBODY_CONTROL_CLASS_ID) != 0);

		// Animation in subkeyframes ?
		if (bodyBiped)
		{
			addBipedKeyTime (*transform, keys, keysSampled, true, getName (*(INode*)&node).c_str());
		}
		else
		{
			addBipedKeyTime (*transform, keys, keysSampled, false, getName (*(INode*)&node).c_str());
		}

		// Create a track name
		std::string name;
		
		// Choose the good name for this track
		if (!root)
			name=parentName + getName (node) + ".";
		else
			name=parentName;

		// Export keyframes
		addNodeTracks (animation, node, name.c_str(), &keys, &keysSampled, root, bodyBiped);

		// Restaure the non uniform scale
		BipIface->RemoveNonUniformScale(0);

		// Add child tracks
		uint childrenCont=(uint)node.NumberOfChildren();
		for (uint children=0; children<childrenCont; children++)
		{
			INode *child=node.GetChildNode(children);
			addBipedNodeTrack (animation, *child, parentName, keys, keysSampled, false);
		}
	}
	else
	{
		// Add normal tracks
		addBoneTracks (animation, node, parentName, root);
	}
}

// --------------------------------------------------

void CExportNel::addBoneTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, bool root)
{
	// Create a track name
	std::string name=parentName + getName (node) + ".";

	// Get the TM controler
	Control *transform=node.GetTMController();

	// Is it a biped node ?
	if (transform && (transform->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		// Export biped skeleton animation
		addBipedNodeTracks (animation, node, parentName, root);
	}
	else
	{
		// Go for normal export!
		addNodeTracks (animation, node, name.c_str(), NULL, NULL, root);
	}

	// Recursive call
	uint childrenCont=(uint)node.NumberOfChildren();
	for (uint children=0; children<childrenCont; children++)
	{
		INode *child=node.GetChildNode(children);
		addBoneTracks (animation, *child, parentName, false);
	}
}

// --------------------------------------------------

/*void CExportNel::addBonesTracks (NL3D::CAnimation& animation, INode& node, const char* parentName)
{
	// Get root bone
	INode *root=getSkeletonRootBone (node);

	// Root exist ?
	if (root)
	{
		// Go for normal export!
		addBoneTracks (animation, *root, parentName);
	}
}*/

// --------------------------------------------------

void CExportNel::addLightTracks (NL3D::CAnimation& animation, INode& node, const char* parentName)
{
	CExportDesc desc;

	Object *obj = node.EvalWorldState(0).obj;

	// Check if there is an object
	if (!obj)
		return;

	// Get a GenLight from the node
	if( ! (obj->SuperClassID()==LIGHT_CLASS_ID) )
		return;

	// Create a track name
	std::string name = CExportNel::getLightGroupName (&node);
	name = "LightmapController." + name;

	int bAnimated = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_LM_ANIMATED, 0);
	if( bAnimated )
	{
		Control *c = getControlerByName(node,"Color");
		if( c )
		{
			ITrack *pTrack=buildATrack (animation, *c, typeColor, node, desc, NULL, NULL);
			if (pTrack)
			{
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addMorphTracks (NL3D::CAnimation& animation, INode& node, const char* parentName)
{
	CExportDesc desc;
	Modifier *pMorphMod = getModifier (&node, MAX_MORPHER_CLASS_ID);

	if (pMorphMod == NULL)
		return;

	uint32 i;

	for (i = 0; i < 100; ++i)
	{
		INode *pNode = (INode*)pMorphMod->GetReference (101+i);
		if (pNode == NULL)
			continue;
		std::string name = parentName;
		name += pNode->GetName();
		name += "MorphFactor";
		
		IParamBlock *pb = (IParamBlock*)(pMorphMod->SubAnim (i+1));
		Control *c = pb->GetController (0);
		
		if (c != NULL)
		{
			ITrack *pTrack = buildATrack (animation, *c, typeFloat, node, desc, NULL, NULL);
			if (pTrack)
			{
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
	}
}


// --------------------------------------------------

void CExportNel::addObjTracks (CAnimation& animation, Object& obj, const char* parentName)
{
	// Export fov for camera
	CExportDesc desc;
	desc.reset();

	// Get the FOV controler
	Control *c=getControlerByName (obj, "FOV");
	if (c)
	{
		ITrack *pTrack=buildATrack (animation, *c, typeFloat, obj, desc, NULL, NULL);
		if (pTrack)
		{
			std::string name=parentName+std::string (CCamera::getFovValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addMtlTracks (CAnimation& animation, Mtl& mtl, const char* parentName)
{	

	// Material name
	std::string mtlName=std::string(parentName)+getName (mtl)+".";

	// Tmp track*
	ITrack *pTrack;

	// Tmp name
	std::string name;

	// Export desc
	CExportDesc desc;

	// *** Export ambient
	desc.reset();

	// Get a controller pointer
	Control* c=getControlerByName (mtl, "ambient");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getAmbientValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}

	// *** Export diffuse
	desc.reset();

	// Get a controller pointer
	c=getControlerByName (mtl, "diffuse");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getDiffuseValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}

	// *** Export specular
	desc.reset();
	desc.Specular=true;
	desc.Material=(StdMat2*)&mtl;

	// Get a controller pointer
	c=getControlerByName (mtl, "specular");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getSpecularValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}

	// *** Export emissive
	desc.reset();

	// Get a controller pointer
	c=getControlerByName (mtl, "selfIllumColor");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getEmissiveValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
     
	// *** Export opacity
	desc.reset();

	// Get a controller pointer
	c=getControlerByName (mtl, "opacity");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeFloat, mtl, desc, NULL, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getOpacityValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
	

	// Export sub materials tracks
	int s;
	int subMtl=mtl.NumSubMtls();
	for (s=0; s<subMtl; s++)
		addMtlTracks (animation, *mtl.GetSubMtl(s), parentName);

	// Export sub texmaps tracks
	/*subMtl=mtl.NumSubTexmaps();
	for (s=0; s<subMtl; s++)
		addTexTracks (animation, *mtl.GetSubTexmap(s), parentName);*/

	// *** export textures matrix animation if enabled

	/// test wether texture matrix animation should be exported
	int bExportTexMatAnim;
	CExportNel::getValueByNameUsingParamBlock2 (mtl, "bExportTextureMatrix", (ParamType2)TYPE_BOOL, &bExportTexMatAnim, 0);

	if (bExportTexMatAnim != 0)
	{
		for (uint i=0; i<IDRV_MAT_MAXTEXTURES; i++)
		{			
			// Get the enable flag name
			char enableSlotName[100];
			smprintf (enableSlotName, 100, "bEnableSlot_%d", i+1);

			// Get the enable flag 
			int bEnableSlot = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, enableSlotName, (ParamType2)TYPE_BOOL, &bEnableSlot, 0);
			if (bEnableSlot)
			{
				// Get the texture arg name
				char textureName[100];
				smprintf (textureName, 100, "tTexture_%d", i+1);

				// Get the texture pointer
				Texmap *pTexmap = NULL;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, textureName, (ParamType2)TYPE_TEXMAP, &pTexmap, 0);
				if (pTexmap)
				{
					addTexTracks(animation, *pTexmap, i, mtlName.c_str());
				}
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addTexTracks (CAnimation& animation, Texmap& tex, uint stage, const char* parentName)
{
	// Texmap name
	

	// Tmp track*
	ITrack *pTrack;

	std::string name;

	CExportDesc desc;


	/// export the u translation
	// Get a controller pointer
	Control* c = getControlerByName (tex, "U Offset");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatUTransName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the v translation
	// Get a controller pointer
                                    	c=getControlerByName (tex, "V Offset");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatVTransName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the u scale
	// Get a controller pointer
	c = getControlerByName (tex, "U Tiling");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatUScaleName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the v scale
	// Get a controller pointer
	c=getControlerByName (tex, "V Tiling");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatVScaleName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the w rotation
	// Get a controller pointer
	c=getControlerByName (tex, "W Angle");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatWRotName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
}



// --------------------------------------------------

// Build nel keys
void CExportNel::buildNelKey (CKeyFloat& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=maxKey.val;
}

// --------------------------------------------------

void CExportNel::buildNelKey (CKeyInt& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=(sint32)maxKey.val;
}

// --------------------------------------------------

void CExportNel::buildNelKey (CKeyBool& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=fabs(maxKey.val)<=FLOAT_EPSILON;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyVector& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		nelKey.Value.x*=shininess;
		nelKey.Value.y*=shininess;
		nelKey.Value.z*=shininess;
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyRGBA& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.R=(uint8)maxKey.val.x;
	nelKey.Value.G=(uint8)maxKey.val.y;
	nelKey.Value.B=(uint8)maxKey.val.z;
	nelKey.Value.A=255;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		clamp (shininess, 0.f, 1.f);
		nelKey.Value.modulateFromui (nelKey.Value, (uint8)(shininess*255.f));
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyQuat& nelKey, ILinRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.w=-maxKey.val.w;
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyVector& nelKey, ILinScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	maxKey.val.q.MakeMatrix(srtm);
	stm = ScaleMatrix(maxKey.val.s);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);
	
	// Export it
	nelKey.Value.x=scaleMatrix.getI().x;
	nelKey.Value.y=scaleMatrix.getJ().y;
	nelKey.Value.z=scaleMatrix.getK().z;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierFloat& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=maxKey.val;
	nelKey.InTan=maxKey.intan;
	nelKey.OutTan=maxKey.outtan;
	
	// Step mode ?
	if (GetOutTanType(maxKey.flags)==BEZKEY_STEP)
		nelKey.Step=true;
	else
		nelKey.Step=false;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBool& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=fabs(maxKey.val)<=FLOAT_EPSILON;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
	nelKey.InTan.x=ticksPerSecond*maxKey.intan.x;
	nelKey.InTan.y=ticksPerSecond*maxKey.intan.y;
	nelKey.InTan.z=ticksPerSecond*maxKey.intan.z;
	nelKey.OutTan.x=ticksPerSecond*maxKey.outtan.x;
	nelKey.OutTan.y=ticksPerSecond*maxKey.outtan.y;
	nelKey.OutTan.z=ticksPerSecond*maxKey.outtan.z;
	
	// Step mode ?
	if (GetOutTanType(maxKey.flags)==BEZKEY_STEP)
		nelKey.Step=true;
	else
		nelKey.Step=false;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		nelKey.Value.x*=shininess;
		nelKey.Value.y*=shininess;
		nelKey.Value.z*=shininess;
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierQuat& nelKey, IBezQuatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.w=-maxKey.val.w;
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	maxKey.val.q.MakeMatrix(srtm);
	stm = ScaleMatrix(maxKey.val.s);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);

	// Export it
	nelKey.Value.x=scaleMatrix.getI().x;
	nelKey.Value.y=scaleMatrix.getJ().y;
	nelKey.Value.z=scaleMatrix.getK().z;
	nelKey.InTan.x=maxKey.intan.x;
	nelKey.InTan.y=maxKey.intan.y;
	nelKey.InTan.z=maxKey.intan.z;
	nelKey.OutTan.x=maxKey.outtan.x;
	nelKey.OutTan.y=maxKey.outtan.y;
	nelKey.OutTan.z=maxKey.outtan.z;
	
	// Step mode ?
	if (GetOutTanType(maxKey.flags)==BEZKEY_STEP)
		nelKey.Step=true;
	else
		nelKey.Step=false;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBFloat& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=maxKey.val;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBool& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=fabs(maxKey.val)<=FLOAT_EPSILON;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		nelKey.Value.x*=shininess;
		nelKey.Value.y*=shininess;
		nelKey.Value.z*=shininess;
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBQuat& nelKey, ITCBRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.Angle=-maxKey.val.angle;
	nelKey.Value.Axis.x=maxKey.val.axis.x;
	nelKey.Value.Axis.y=maxKey.val.axis.y;
	nelKey.Value.Axis.z=maxKey.val.axis.z;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	maxKey.val.q.MakeMatrix(srtm);
	stm = ScaleMatrix(maxKey.val.s);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);
	
	// Export it
	nelKey.Value.x=scaleMatrix.getI().x;
	nelKey.Value.y=scaleMatrix.getJ().y;
	nelKey.Value.z=scaleMatrix.getK().z;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;
}

//--------------------------------------------------------


// Build a Nel bool track from a On/Off max Controller (doesn't work with buildATRack, which require a keyframer interface
// , which isn't provided by an on / off controller)
 NL3D::CTrackKeyFramerConstBool*			CExportNel::buildOnOffTrack(Control& c)
 {
	// make sure this is the controler we want
//	nlassert(c.ClassID() == Class_ID(BOOL_CONTROL_CLASS_ID, 0));	
	float value = 0.f;
	
	CTrackKeyFramerConstBool *track = new CTrackKeyFramerConstBool;

	// ** Get the range of the controler
	Interval range=c.GetTimeRange (TIMERANGE_ALL);

	// ** Get the out of range type
	int oRT=c.GetORT (ORT_AFTER);

	// Set the range
	if ((!(FOREVER==range))&&(!(NEVER==range)))
		track->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));

	// Set the out of range type
	switch (oRT)
	{
	case ORT_LOOP:
		track->setLoopMode(true);
		break;
	case ORT_CONSTANT:
	case ORT_CYCLE:
	default:
		track->setLoopMode(false);
		break;
	}

	// Enum the keys
	int numKeys = c.NumKeys();
	float firstKey;
	float lastKey;
	for (int i=0; i<numKeys; i++) 
	{		
		// First key ?
		if (i==0)
			firstKey = convertTime (c.GetKeyTime(i));

		// Last key ?
		lastKey = convertTime (c.GetKeyTime(i));

		// Allocate the key
		CKey<bool> nelKey;		
		c.GetValue(c.GetKeyTime(i), &value, range, CTRL_ABSOLUTE);
		nelKey.Value = value > 0.f;
		track->addKey(nelKey, convertTime (c.GetKeyTime(i)));		
	}

	// Invalid interval ? Take the interval of the keyfarmer
	if ((FOREVER==range)||(NEVER==range))
		track->unlockRange (firstKey, lastKey);

	
	return track;
 }



// --------------------------------------------------

// Create a keyframer
template<class TTracker, class TKey, class TMaxKey>
ITrack* createKeyFramer (IKeyControl *ikeys, TTracker*, TKey*, TMaxKey*, float ticksPerSecond, const Interval& range, int oRT, 
						 const CExportDesc& desc, Control &c)
{
	// Allocate the tracker
	TTracker *pLinTrack=new TTracker;

	// Set the range
	if ((!(FOREVER==range))&&(!(NEVER==range)))
		pLinTrack->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));

	// Set the out of range type
	switch (oRT)
	{
	case ORT_LOOP:
		pLinTrack->setLoopMode(true);
		break;
	case ORT_CONSTANT:
	case ORT_CYCLE:
	default:
		pLinTrack->setLoopMode(false);
		break;
	}

	// Enum the keys
	TMaxKey key;
	int numKeys = ikeys->GetNumKeys();
	float firstKey;
	float lastKey;
	for (int i=0; i<numKeys; i++) 
	{
		// Get the key
		ikeys->GetKey(i, &key);

		// First key ?
		if (i==0)
			firstKey=CExportNel::convertTime (key.time);

		// Last key ?
		lastKey=CExportNel::convertTime (key.time);

		// Allocate the key
		TKey nelKey;

		// Build the key
		CExportNel::buildNelKey (nelKey, key, ticksPerSecond, desc, c);

		// Add the good key
		pLinTrack->addKey (nelKey, CExportNel::convertTime (key.time));
	}

	// Invalid interval ? Take the inteval of the keyfarmer
	if ((FOREVER==range)||(NEVER==range))
		pLinTrack->unlockRange (firstKey, lastKey);


	// Return the keyframer
	return pLinTrack;
}

// --------------------------------------------------

class CDoomyKey : public IKey
{
	char toto[2048];
};

// --------------------------------------------------

// Create the matrix tracks
void CExportNel::createBipedKeyFramer (ITrack *&nelRot, ITrack *&nelPos, bool isRot, bool isPos, float ticksPerSecond, 
									   const Interval& range, int oRT, const CExportDesc& desc, INode& node, 
									   std::set<TimeValue>* previousKeys, std::set<TimeValue>* previousKeysSampled)
{
	nelRot=NULL;
	nelPos=NULL;

	// Type of the track
	typedef CTrackKeyFramerTCBVector	TMPos;
	typedef CTrackKeyFramerTCBQuat		TMRot;
	typedef CKeyTCBVector				TMPosKey;
	typedef CKeyTCBQuat					TMRotKey;

	// Number of keys
	uint numKeys = previousKeysSampled->size ();

	// No null keys ?
	if (numKeys!=0)
	{
		// Allocate the keyframer
		if(isRot)			
			nelRot=new TMRot;
		if (isPos)
			nelPos=new TMPos;

		// Set the range
		if ((!(FOREVER==range))&&(!(NEVER==range)))
		{
			if (isRot)
				((TMRot*)nelRot)->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));
			if (isPos)
				((TMPos*)nelPos)->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));
		}

		// Set the out of range type
		switch (oRT)
		{
		case ORT_LOOP:
			if (isRot)
				((TMRot*)nelRot)->setLoopMode(true);
			if (isPos)
				((TMPos*)nelPos)->setLoopMode(true);
			break;
		case ORT_CONSTANT:
		case ORT_CYCLE:
		default:
			if (isRot)
				((TMRot*)nelRot)->setLoopMode(false);
			if (isPos)
				((TMPos*)nelPos)->setLoopMode(false);
			break;
		}

		// Enum the keys
		float firstKey;
		float lastKey;
		CQuat previous;
		std::set<TimeValue>::const_iterator ite=previousKeysSampled->begin();
		uint key=0;
		while (ite!=previousKeysSampled->end())
		{
			// Real key ?
			bool hardKey=(previousKeys->find (*ite)!=previousKeys->end());

			// First key ?
			if (ite==previousKeysSampled->begin())
				firstKey=convertTime (*ite);

			// Last key ?
			lastKey=convertTime (*ite);

			// Allocate the key
			TMRotKey		theRot;
			TMPosKey		thePos;

			// Access biped ease / bias / continuity / tension by script !!
			// Get tcb value..
			
			// Rotation
			if (isRot)
			{
				if (hardKey)
				{
					// Use script to access some values
					if (getBipedKeyInfo (getName (node).c_str(), "bias", key, theRot.Bias))
						theRot.Bias=(theRot.Bias-25.f)/25.f;
					else
						theRot.Bias=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "continuity", key, theRot.Continuity))
						theRot.Continuity=(theRot.Continuity-25.f)/25.f;
					else
						theRot.Continuity=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "tension", key, theRot.Tension))
						theRot.Tension=(theRot.Tension-25.f)/25.f;
					else
						theRot.Tension=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "easeTo", key, theRot.EaseTo))
						theRot.EaseTo/=50.f;
					else
						theRot.EaseTo=0.f;
					if (getBipedKeyInfo ( getName (node).c_str(), "easeFrom", key, theRot.EaseFrom))
						theRot.EaseFrom/=50.f;
					else
						theRot.EaseFrom=0.f;
				}
				else
				{
					// Use script to access some values
					theRot.Bias=0.f;
					theRot.Continuity=0.f;
					theRot.Tension=0.f;
					theRot.EaseTo=0.f;
					theRot.EaseFrom=0.f;
				}
			}

			if (isPos)
			{
				if (hardKey)
				{
					// Use script to access some values
					if (getBipedKeyInfo (getName (node).c_str(), "bias", key, thePos.Bias))
						thePos.Bias=(thePos.Bias-25.f)/25.f;
					else
						thePos.Bias=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "continuity", key, thePos.Continuity))
						thePos.Continuity=(thePos.Continuity-25.f)/25.f;
					else
						thePos.Continuity=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "tension", key, thePos.Tension))
						thePos.Tension=(thePos.Tension-25.f)/25.f;
					else
						thePos.Tension=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "easeTo", key, thePos.EaseTo))
						thePos.EaseTo/=50.f;
					else
						thePos.EaseTo=0.f;
					if (getBipedKeyInfo (getName (node).c_str(), "easeFrom", key, thePos.EaseFrom))
						thePos.EaseFrom/=50.f;
					else
						thePos.EaseFrom=0.f;
				}
				else
				{
					// Use script to access some values
					thePos.Bias=0.f;
					thePos.Continuity=0.f;
					thePos.Tension=0.f;
					thePos.EaseTo=0.f;
					thePos.EaseFrom=0.f;
				}
			}

			// Get the local matrix
			Matrix3 localMatrix;
			getLocalMatrix (localMatrix, node, *ite);

			// Decomp in scale, rot and pos.
			CQuat tmpQuat;
			CQuat tmpPrevious=previous;
			CVector scale;
			decompMatrix (scale, tmpQuat, thePos.Value, localMatrix);
			tmpQuat.normalize();

			// Make closest with previous
			if (ite!=previousKeysSampled->begin())
			{
				tmpQuat.makeClosest (tmpPrevious);
			}

			// Set previous
			previous=tmpQuat;

			// Relative quaternion
			if (ite!=previousKeysSampled->begin())
			{
				tmpQuat*=tmpPrevious.inverted();
			}

			// Convert quat to angle axis.
			theRot.Value=tmpQuat.getAngleAxis();

			// Add the good keys
			if (isRot)
				((TMRot*)nelRot)->addKey (theRot, convertTime (*ite));
			if (isPos)
				((TMPos*)nelPos)->addKey (thePos, convertTime (*ite));

			// Ite ++
			if (hardKey)
				key++;
			ite++;
		}

		// Invalid interval ? Take the inteval of the keyfarmer
		if ((FOREVER==range)||(NEVER==range))
		{
			if (isRot)
				((TMRot*)nelRot)->unlockRange (firstKey, lastKey);
			if (isPos)
				((TMPos*)nelPos)->unlockRange (firstKey, lastKey);
		}
	}
}

// --------------------------------------------------

NL3D::TAnimationTime	CExportNel::convertTime (TimeValue time)
{
	return (float)time/((float)GetTicksPerFrame()*(float)GetFrameRate());
}

// --------------------------------------------------

// Build a NeL track with a 3dsmax controler.
ITrack* CExportNel::buildATrack (CAnimation& animation, Control& c, TNelValueType type, Animatable& node, const CExportDesc& desc,
								 std::set<TimeValue>* previousKeys, std::set<TimeValue>* previousKeysSampled, bool bodyBiped)
{
	// What kind of controler?

	/*
	 * Those controlers are supported:
	 *
	 * LININTERP_FLOAT_CLASS_ID
	 * LININTERP_POSITION_CLASS_ID
	 * LININTERP_ROTATION_CLASS_ID
	 * LININTERP_SCALE_CLASS_ID
	 * HYBRIDINTERP_FLOAT_CLASS_ID
	 * HYBRIDINTERP_POSITION_CLASS_ID
	 * HYBRIDINTERP_ROTATION_CLASS_ID
	 * HYBRIDINTERP_POINT3_CLASS_ID
	 * HYBRIDINTERP_SCALE_CLASS_ID
	 * TCBINTERP_FLOAT_CLASS_ID
	 * TCBINTERP_POSITION_CLASS_ID
	 * TCBINTERP_ROTATION_CLASS_ID
	 * TCBINTERP_POINT3_CLASS_ID
	 * TCBINTERP_SCALE_CLASS_ID
	 * BIPSLAVE_CONTROL_CLASS_ID
	 * BIPBODY_CONTROL_CLASS_ID
	 *
	 * Thoses controlers are not supported:
	 *
	 * LOOKAT_CONTROL_CLASS_ID
	 * PATH_CONTROL_CLASS_ID
	 * EXPR_POS_CONTROL_CLASS_ID
	 * EXPR_P3_CONTROL_CLASS_ID
	 * EXPR_FLOAT_CONTROL_CLASS_ID
	 * EXPR_SCALE_CONTROL_CLASS_ID
	 * EXPR_ROT_CONTROL_CLASS_ID
	 * SURF_CONTROL_CLASSID
	 * LINKCTRL_CLASSID
	 *
	 * May be supported in futur:
	 *
	 * EULER_CONTROL_CLASS_ID
	 * FLOATNOISE_CONTROL_CLASS_ID
	 * POSITIONNOISE_CONTROL_CLASS_ID
	 * POINT3NOISE_CONTROL_CLASS_ID
	 * ROTATIONNOISE_CONTROL_CLASS_ID
	 * SCALENOISE_CONTROL_CLASS_ID
	 * HYBRIDINTERP_COLOR_CLASS_ID
	 * FOOTPRINT_CLASS_ID
	 */

	ITrack* pTrack=NULL;

	// ** Get the 3dsmax key control.
	IKeyControl *ikeys=GetKeyControlInterface ((&c));

	// ** Get number of ticks per second
	float ticksPerSecond=(float)TIME_TICKSPERSEC;

	// ** Get the range of the controler
	Interval range=c.GetTimeRange (TIMERANGE_ALL);

	// ** Get the out of range type
	int oRT=c.GetORT (ORT_AFTER);

	// ** Check all keyframer if it is one.
	if (ikeys)
	{
		if (ikeys->GetNumKeys()>0)
		{
			// ** Linear nel keyframe

			if (c.ClassID()==Class_ID(LININTERP_FLOAT_CLASS_ID,0))
			{
				// For float
				if (type==typeFloat)
					pTrack=createKeyFramer<CTrackKeyFramerLinearFloat, CKeyFloat, ILinFloatKey> (ikeys,
						(CTrackKeyFramerLinearFloat*)NULL, (CKeyFloat*)NULL, (ILinFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For int
				if (type==typeInt)
					pTrack=createKeyFramer<CTrackKeyFramerLinearInt, CKeyInt, ILinFloatKey> (ikeys,
						(CTrackKeyFramerLinearInt*)NULL, (CKeyInt*)NULL, (ILinFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For bool
				if (type==typeBoolean)
					pTrack=createKeyFramer<CTrackKeyFramerConstBool, CKeyBool, ILinFloatKey> (ikeys,
						(CTrackKeyFramerConstBool*)NULL, (CKeyBool*)NULL, (ILinFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(LININTERP_POSITION_CLASS_ID,0))
			{
				// For vector
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerLinearVector, CKeyVector, ILinPoint3Key> (ikeys,
						(CTrackKeyFramerLinearVector*)NULL, (CKeyVector*)NULL, (ILinPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For color
				if (type==typeColor)
					pTrack=createKeyFramer<CTrackKeyFramerLinearRGBA, CKeyRGBA, ILinPoint3Key> (ikeys,
						(CTrackKeyFramerLinearRGBA*)NULL, (CKeyRGBA*)NULL, (ILinPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(LININTERP_ROTATION_CLASS_ID,0))
			{
				// For quaternion
				if (type==typeRotation)
					pTrack=createKeyFramer<CTrackKeyFramerLinearQuat, CKeyQuat, ILinRotKey> (ikeys,
						(CTrackKeyFramerLinearQuat*)NULL, (CKeyQuat*)NULL, (ILinRotKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(LININTERP_SCALE_CLASS_ID,0))
			{
				// For scale
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerLinearVector, CKeyVector, ILinScaleKey> (ikeys,
						(CTrackKeyFramerLinearVector*)NULL, (CKeyVector*)NULL, (ILinScaleKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}

			// ** Bezier

			if (c.ClassID()==Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))
			{
				// For float
				if (type==typeFloat)
					pTrack=createKeyFramer<CTrackKeyFramerBezierFloat, CKeyBezierFloat, IBezFloatKey> (ikeys,
						(CTrackKeyFramerBezierFloat*)NULL, (CKeyBezierFloat*)NULL, (IBezFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For int
				if (type==typeInt)
					pTrack=createKeyFramer<CTrackKeyFramerBezierInt, CKeyBezierFloat, IBezFloatKey> (ikeys,
						(CTrackKeyFramerBezierInt*)NULL, (CKeyBezierFloat*)NULL, (IBezFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For bool
				if (type==typeBoolean)
					pTrack=createKeyFramer<CTrackKeyFramerConstBool, CKeyBool, IBezFloatKey> (ikeys,
						(CTrackKeyFramerConstBool*)NULL, (CKeyBool*)NULL, (IBezFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if ((c.ClassID()==Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0))||
				(c.ClassID()==Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0)))
			{
				// For vector
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerBezierVector, CKeyBezierVector, IBezPoint3Key> (ikeys,
						(CTrackKeyFramerBezierVector*)NULL, (CKeyBezierVector*)NULL, (IBezPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For color
				if (type==typeColor)
					pTrack=createKeyFramer<CTrackKeyFramerBezierRGBA, CKeyBezierVector, IBezPoint3Key> (ikeys,
						(CTrackKeyFramerBezierRGBA*)NULL, (CKeyBezierVector*)NULL, (IBezPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID,0))
			{
				// For quaternion
				if (type==typeRotation)
					pTrack=createKeyFramer<CTrackKeyFramerBezierQuat, CKeyBezierQuat, IBezQuatKey> (ikeys,
						(CTrackKeyFramerBezierQuat*)NULL, (CKeyBezierQuat*)NULL, (IBezQuatKey *)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0))
			{
				// For scale
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerBezierVector, CKeyBezierVector, IBezScaleKey> (ikeys,
						(CTrackKeyFramerBezierVector*)NULL, (CKeyBezierVector*)NULL, (IBezScaleKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(HYBRIDINTERP_COLOR_CLASS_ID,0))
			{
			}

			// ** TCB

			if (c.ClassID()==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0))
			{
				// For float
				if (type==typeFloat)
					pTrack=createKeyFramer<CTrackKeyFramerTCBFloat, CKeyTCBFloat, ITCBFloatKey> (ikeys,
						(CTrackKeyFramerTCBFloat*)NULL, (CKeyTCBFloat*)NULL, (ITCBFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For int
				if (type==typeInt)
					pTrack=createKeyFramer<CTrackKeyFramerTCBInt, CKeyTCBFloat, ITCBFloatKey> (ikeys,
						(CTrackKeyFramerTCBInt*)NULL, (CKeyTCBFloat*)NULL, (ITCBFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For bool
				if (type==typeBoolean)
					pTrack=createKeyFramer<CTrackKeyFramerConstBool, CKeyBool, ITCBFloatKey> (ikeys,
						(CTrackKeyFramerConstBool*)NULL, (CKeyBool*)NULL, (ITCBFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if ((c.ClassID()==Class_ID(TCBINTERP_POSITION_CLASS_ID,0))||
				(c.ClassID()==Class_ID(TCBINTERP_POINT3_CLASS_ID,0)))
			{
				// For vector
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerTCBVector, CKeyTCBVector, ITCBPoint3Key> (ikeys,
						(CTrackKeyFramerTCBVector*)NULL, (CKeyTCBVector*)NULL, (ITCBPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For color
				if (type==typeColor)
					pTrack=createKeyFramer<CTrackKeyFramerTCBRGBA, CKeyTCBVector, ITCBPoint3Key> (ikeys,
						(CTrackKeyFramerTCBRGBA*)NULL, (CKeyTCBVector*)NULL, (ITCBPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0))
			{
				// For quaternion
				if (type==typeRotation)
					pTrack=createKeyFramer<CTrackKeyFramerTCBQuat, CKeyTCBQuat, ITCBRotKey> (ikeys,
						(CTrackKeyFramerTCBQuat*)NULL, (CKeyTCBQuat*)NULL, (ITCBRotKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(TCBINTERP_SCALE_CLASS_ID,0))
			{
				// For scale
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerTCBVector, CKeyTCBVector, ITCBScaleKey> (ikeys,
						(CTrackKeyFramerTCBVector*)NULL, (CKeyTCBVector*)NULL, (ITCBScaleKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}			
		}
	}

	// No keyframer controler
	if ((c.ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
		bodyBiped)
	{
		NL3D::ITrack	*doomy;

		// For rotation
		if (type==typeRotation)
		{
			if (previousKeys&&previousKeysSampled)
			{
				createBipedKeyFramer (pTrack, doomy, true, false, ticksPerSecond, range, oRT, desc, *(INode*)&node, 
									previousKeys, previousKeysSampled);
			}
		}
	}

	if (bodyBiped)
	{
		NL3D::ITrack	*doomy;

		// For position
		if (type==typePos)
		{
			// 
			// Create key set
			if (previousKeys&&previousKeysSampled)
			{
				createBipedKeyFramer (doomy, pTrack, false, true, ticksPerSecond, range, oRT, desc, *(INode*)&node, 
										previousKeys, previousKeysSampled);
			}
			else
			{
				MessageBox (NULL, "Warning: no pos track exported!", "Tmp NEL", MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
	
	// Return the track
	return pTrack;
}

// --------------------------------------------------

// TODO: remove interpolation when IK is available
#define NEL3D_IKBLEND_ALLOW_OVERSAMPLING
#define NEL3D_IKBLEND_OVERSAMPLING 30

void CExportNel::addBipedKeyTime (Control& c, std::set<TimeValue>& keys, std::set<TimeValue>& keysSampled, bool subKeys, 
								  const char *nodeName)
{
	// ** Get the 3dsmax key control.
	IKeyControl *ikeys=GetKeyControlInterface ((&c));

	// IK blend available ?
	float ikBlend;
	bool ikBlendAvalaible = false;
	if (nodeName)
		ikBlendAvalaible = getBipedKeyInfo (nodeName, "IkBlend", 0, ikBlend);

	// Ikey controler ?
	if (ikeys)
	{
		// Key the keys
		CDoomyKey key;
		int numKeys = ikeys->GetNumKeys();
		
		// Step for interpolation
		int interpolationStep=(NEL3D_IKBLEND_OVERSAMPLING*GetTicksPerFrame())/GetFrameRate();

		for (int i=0; i<numKeys; i++) 
		{
			// Get the key
			ikeys->GetKey(i, &key);

			// Get the next Ik blend
			float nextIkBlend;
			if (ikBlendAvalaible)
			{
				CDoomyKey nextKey;
				if (i<numKeys-1)
				{
					// Copy Ik blend value
					nextIkBlend=ikBlend;

					// Get next ik blend value
					getBipedKeyInfo (nodeName, "IkBlend", i+1, nextIkBlend);

					// Get next key
					ikeys->GetKey(i+1, &nextKey);

					// Oversampling ?
#ifdef	NEL3D_IKBLEND_ALLOW_OVERSAMPLING
					if ( (ikBlend > 0.001) || (nextIkBlend > 0.001) )
					{
						// Ok, let's oversample
						TimeValue thisTime=key.time+interpolationStep;
						while (thisTime<nextKey.time)
						{
							// Add ghost frames
							keysSampled.insert (thisTime);

							// Next ghost key
							thisTime+=interpolationStep;
						}
					}
#endif
				}
			}

			// Inset the key time
			keys.insert (key.time);
			keysSampled.insert (key.time);

			// Copy ikblend
			ikBlend=nextIkBlend;
		}
	}

	// Sub keys ?
	if (subKeys)
	{
		// Get num sub anim
		uint numSubAnim=c.NumSubs();

		// Visit sub anim
		for (uint s=0; s<numSubAnim; s++)
		{
			// ok, return this subanim controler
			if (c.SubAnim(s))
			{
				// Get the controller pointer of this sub anim
				Control* c2=GetControlInterface (c.SubAnim(s));
				if (c2)
					addBipedKeyTime (*c2, keys, keysSampled, true, NULL);
			}
		}
	}
}

// --------------------------------------------------

bool CExportNel::getBipedKeyInfo (const char* nodeName, const char* paramName, uint key, float& res)
{
	// Make the script sequence
	char script[512];
	_snprintf (script, 512, "(biped.getKey $'%s'.controller %d).%s", nodeName, key+1, paramName);

	// Call the script
	return scriptEvaluate (script, &res, scriptFloat);
}

// --------------------------------------------------

bool CExportNel::getBipedInplaceMode (const char* nodeName, const char* inplaceFunction, bool &res)
{
	// Make the script sequence
	char script[512];
	_snprintf (script, 512, "$'%s'.controller.%s", nodeName, inplaceFunction);

	// Call the script
	return scriptEvaluate (script, &res, scriptBool);
}

// --------------------------------------------------

bool CExportNel::setBipedInplaceMode (const char* nodeName, const char* inplaceFunction, bool onOff)
{
	// Make the script sequence
	char script[512];
	_snprintf (script, 512, "$'%s'.controller.%s = %s", nodeName, inplaceFunction, onOff ? "true" : "false");

	// Call the script
	return scriptEvaluate (script, NULL, scriptNothing);
}

// --------------------------------------------------

