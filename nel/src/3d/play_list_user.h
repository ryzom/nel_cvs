/** \file play_list_user.h
 * <File description>
 *
 * $Id: play_list_user.h,v 1.4 2001/09/05 11:45:28 corvazier Exp $
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
#include "nel/3d/u_play_list.h"
#include "nel/misc/smart_ptr.h"
#include "3d/animation_set.h"
#include "3d/channel_mixer.h"
#include "3d/animation_playlist.h"
#include "3d/transform_user.h"


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


	/// \name Animatable Link.
	// @{
	virtual	void	registerTransform(UTransform *object, const char* prefix)
	{
		CTransformUser	*obj= dynamic_cast<CTransformUser*>(object);
		nlassert(obj);

		// Register the transform to the channel mixer.
		obj->getTransform()->registerToChannelMixer(&_ChannelMixer, prefix);
	}
	virtual	void	resetAllChannels()
	{
		_ChannelMixer.resetChannels();
	}

	// @}


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
	virtual	void setTimeOrigin (uint8 slot, double timeOrigin)
	{
		_PlayList.setTimeOrigin (slot, timeOrigin);
	}

	virtual	double getTimeOrigin (uint8 slot) const
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

	virtual	void setWrapMode (uint8 slot, TWrapMode wrapMode)
	{
		_PlayList.setWrapMode (slot, (CAnimationPlaylist::TWrapMode)(uint)wrapMode);
	}

	virtual	TWrapMode getWrapMode (uint8 slot) const
	{
		return (UPlayList::TWrapMode)(uint)_PlayList.getWrapMode (slot);
	}

	// @}


	/// \name Animation Weight Setup.
	// @{
	virtual	void setStartWeight (uint8 slot, float startWeight, double time)
	{
		_PlayList.setStartWeight (slot, startWeight, time);
	}

	virtual	float getStartWeight (uint8 slot, double& time) const
	{
		return _PlayList.getStartWeight(slot, time);
	}

	virtual	void setEndWeight (uint8 slot, float endWeight, double time)
	{
		_PlayList.setEndWeight (slot, endWeight, time);
	}

	virtual	float getEndWeight (uint8 slot, double& time) const
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

	virtual	void setWeight (uint8 slot, float weight)
	{
		_PlayList.setStartWeight (slot, weight, 0);
		_PlayList.setEndWeight (slot, weight, 0);
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


	/// \name Special channel operation.
	// @{

	virtual	void enableChannel (uint channelId, bool enable)
	{
		_ChannelMixer.enableChannel(channelId, enable);
	}

	virtual	bool isChannelEnabled (uint channelId) const
	{
		return _ChannelMixer.isChannelEnabled (channelId) ;
	}

	// @}


};


} // NL3D


#endif // NL_PLAY_LIST_USER_H

/* End of play_list_user.h */
