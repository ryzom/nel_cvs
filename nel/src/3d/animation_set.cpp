/** \file animation_set.cpp
 * <File description>
 *
 * $Id: animation_set.cpp,v 1.17 2004/03/24 16:36:58 berenguier Exp $
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

#include "3d/animation_set.h"
#include "nel/misc/stream.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"


#include <memory>



namespace NL3D 
{

// ***************************************************************************
CAnimationSet::CAnimationSet ()
{
}
	
// ***************************************************************************
CAnimationSet::~CAnimationSet ()
{
	// Erase all animations.
	for (uint a=0; a<_Animation.size(); a++)
		delete _Animation[a];
	for (uint s=0; s<_SkeletonWeight.size(); s++)
		delete _SkeletonWeight[s];
}

// ***************************************************************************
uint CAnimationSet::getNumChannelId () const
{
	return _ChannelIdByName.size ();
}

// ***************************************************************************
uint CAnimationSet::addAnimation (const char* name, CAnimation* animation)
{
	// Add an animation
	_Animation.push_back (animation);
	_AnimationName.push_back (name);

	// Add an entry name / animation
	_AnimationIdByName.insert (std::map <std::string, uint32>::value_type (name, _Animation.size()-1));

	// Return animation id
	return _Animation.size()-1;
}

// ***************************************************************************
uint CAnimationSet::addSkeletonWeight (const char* name, CSkeletonWeight* skeletonWeight)
{
	// Add an animation
	_SkeletonWeight.push_back (skeletonWeight);
	_SkeletonWeightName.push_back (name);

	// Add an entry name / animation
	_SkeletonWeightIdByName.insert (std::map <std::string, uint32>::value_type (name, _SkeletonWeight.size()-1));

	// Return animation id
	return _SkeletonWeight.size()-1;
}

// ***************************************************************************
void CAnimationSet::reset ()
{
	_Animation.clear();
	_SkeletonWeight.clear();
	_AnimationName.clear();
	_SkeletonWeightName.clear();
	_ChannelIdByName.clear();
	_AnimationIdByName.clear();
	_SkeletonWeightIdByName.clear();
}

// ***************************************************************************
void CAnimationSet::build ()
{
	// Clear the channel map
	_ChannelIdByName.clear ();

	// Set of names
	std::set<std::string> channelNames;

	// For each animation in the set
	for (uint a=0; a<_Animation.size(); a++)
	{
		// Fill the set of channel names
		getAnimation (a)->getTrackNames (channelNames);
	}

	// Add this name in the map with there iD
	uint id=0;
	std::set<std::string>::iterator ite=channelNames.begin ();
	while (ite!=channelNames.end ())
	{
		// Insert an entry
		_ChannelIdByName.insert (std::map <std::string, uint32>::value_type (*ite, id++));

		// Next entry
		ite++;
	}
}

// ***************************************************************************
void CAnimationSet::serial (NLMISC::IStream& f)
{
	// Serial an header
	f.serialCheck ((uint32)'_LEN');
	f.serialCheck ((uint32)'MINA');
	f.serialCheck ((uint32)'TES_');

	// Serial a version
	(void)f.serialVersion (0);

	// Serial the class
	f.serialContPtr (_Animation);
	f.serialContPtr (_SkeletonWeight);
	f.serialCont (_AnimationName);
	f.serialCont (_SkeletonWeightName);
	f.serialCont(_ChannelIdByName);
	f.serialCont(_AnimationIdByName);
	f.serialCont(_SkeletonWeightIdByName);
}

// ***************************************************************************
bool CAnimationSet::loadFromFiles(const std::string &path, bool recurse /* = true*/, const char *ext /*= "anim"*/, bool wantWarningMessage /*= true*/)
{
	bool everythingOk = true;
	std::vector<std::string> anims;
	NLMISC::CPath::getPathContent(path, recurse, false, true, anims);
	for (uint k = 0; k < anims.size(); ++k)
	{
		std::string fileExt = NLMISC::CFile::getExtension(anims[k]);
		if (fileExt == ext) // an animation file ?
		{
			try
			{
				NLMISC::CIFile	iFile;
				iFile.open(anims[k]);
				std::auto_ptr<CAnimation> anim(new CAnimation);
				anim->serial(iFile);
				addAnimation(NLMISC::CFile::getFilenameWithoutExtension(anims[k]).c_str(), anim.release());
				iFile.close();

			}
			catch (NLMISC::EStream &e)
			{
				if (wantWarningMessage)
				{
					nlinfo("Unable to load an automatic animation : %s", e.what());
				}
				everythingOk = false;
			}
		}
	}
	build();
	return everythingOk;
}


} // NL3D
