/** \file play_list_user.h
 * <File description>
 *
 * $Id: play_list_user.h,v 1.1 2001/03/29 09:55:30 berenguier Exp $
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

#ifndef NL_PLAY_LIST_USER_H
#define NL_PLAY_LIST_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_play_list.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/animation_set.h"
#include "nel/3d/channel_mixer.h"
#include "nel/3d/animation_playlist.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * UPlayList implementation. Basicly, this is a playlist, plus a ChannelMixer.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPlayListUser : public UPlayList
{
private:
	CAnimationPlaylist					_PlayList;
	CChannelMixer						_ChannelMixer;
	// just take a refernece on a animation set, so _ChannelMixer has always a good pointer.
	NLMISC::CSmartPtr<CAnimationSet>	_AnimationSet;

	friend class CPlayListManagerUser;


public:

	/// Constructor
	CPlayListUser(NLMISC::CSmartPtr<CAnimationSet>		animationSet)
	{
		nlassert(animationSet!=NULL);
		_AnimationSet= animationSet;

		_ChannelMixer.setAnimationSet(_AnimationSet);

		nlassert((uint)UPlayList::empty == (uint)CAnimationPlaylist::empty);
		nlassert((uint)UPlayList::WrapModeCount == (uint)CAnimationPlaylist::WrapModeCount);
	}


	/// \name Animation Setup.
	// @{
	virtual	void emptyPlayList ()
	{
		_PlayList.emptyPlayList ();
	}

	virtual	void setAnimation (uint8 slot, uint animation)
	{
		_PlayList.setAnimation (slot, animation);
	}

	virtual	uint getAnimation (uint8 slot) const
	{
		return _PlayList.getAnimation (slot);
	}
	// @}


	/// \name Animation Time Setup.
	// @{
	virtual	void setTimeOrigin (uint8 slot, CAnimationTime timeOrigin)
	{
		_PlayList.setTimeOrigin (slot, timeOrigin);
	}

	virtual	CAnimationTime getTimeOrigin (uint8 slot) const
	{
		return _PlayList.getTimeOrigin (slot);
	}

	virtual	void setSpeedFactor (uint8 slot, float speedFactor)
	{
		_PlayList.setSpeedFactor (slot, speedFactor);
	}

	virtual	float getSpeedFactor (uint8 slot) const
	{
		return _PlayList.getSpeedFactor (slot);
	}

	virtual	void setWrapMode (TWrapMode wrapMode)
	{
		_PlayList.setWrapMode ((CAnimationPlaylist::TWrapMode)(uint)wrapMode);
	}

	virtual	TWrapMode getWrapMode () const
	{
		return (UPlayList::TWrapMode)(uint)_PlayList.getWrapMode ();
	}

	// @}


	/// \name Animation Weight Setup.
	// @{
	virtual	void setStartWeight (uint8 slot, float startWeight, CAnimationTime time)
	{
		_PlayList.setStartWeight (slot, startWeight, time);
	}

	virtual	float getStartWeight (uint8 slot, CAnimationTime& time) const
	{
		return _PlayList.getStartWeight(slot, time);
	}

	virtual	void setEndWeight (uint8 slot, float endWeight, CAnimationTime time)
	{
		_PlayList.setEndWeight (slot, endWeight, time);
	}

	virtual	float getEndWeight (uint8 slot, CAnimationTime& time) const
	{
		return _PlayList.getEndWeight (slot, time);
	}

	virtual	void setWeightSmoothness (uint8 slot, float smoothness)
	{
		_PlayList.setWeightSmoothness (slot, smoothness);
	}

	virtual	float getWeightSmoothness (uint8 slot) const
	{
		return _PlayList.getWeightSmoothness (slot);
	}
	// @}



	/// \name Skeleton Weight Setup.
	// @{
	virtual	void setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted=false)
	{
		_PlayList.setSkeletonWeight (slot, skeletonId, inverted);
	}

	virtual	uint getSkeletonWeight (uint8 slot, bool &inverted) const
	{
		return _PlayList.getSkeletonWeight (slot, inverted);
	}
	// @}



};


} // NL3D


#endif // NL_PLAY_LIST_USER_H

/* End of play_list_user.h */
