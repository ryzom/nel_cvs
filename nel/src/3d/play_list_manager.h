/** \file play_list_manager.h
 * TODO: File description
 *
 * $Id: play_list_manager.h,v 1.7 2005/02/22 10:19:11 besson Exp $
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

#ifndef NL_PLAY_LIST_MANAGER_H
#define NL_PLAY_LIST_MANAGER_H

#include "nel/misc/types_nl.h"
#include "animation_playlist.h"
#include "channel_mixer.h"
#include <map>


namespace NL3D 
{


// ***************************************************************************
/**
 * This class handles a list of pair PlayList/ChannelMixer RefPointer. It is just a container, with a animate() method.
 * PlayList/ChannelMixer are sotred with RefPtr. They are auto-removed from the list if the playlist or the channelmixer is
 * deleted (done in animate()).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPlayListManager
{
public:

	/// Constructor
	CPlayListManager() {}

	/// add a playlist/channelmixer to the manager. nlassert(plist). chanMixer replaced if playlist already IN.
	void	addPlaylist(CAnimationPlaylist* plist, CChannelMixer *chanMixer);

	/// remove a playlist/channelmixer from the manager. nlassert(plist). no op if not here.
	void	removePlaylist(CAnimationPlaylist* plist);

	/** animate all the playlist: setupMixer() the channelMixer, and eval(false) (eval global part).
	 * If a playlist/channelmixer has been deleted (RefPtr), the node is removed from the list.
	 */
	void	animate(TGlobalAnimationTime time);
	
	/** steup all the playlist: setupMixer() the channelMixer.
	 * If a playlist/channelmixer has been deleted (RefPtr), the node is removed from the list.
	 */
	void	setup(TGlobalAnimationTime time);

	/** Perform a deletion of all pair playlist channel mixer
	 */
	void	deleteAll();
// ***************************
private:
	struct	CNode
	{
		NLMISC::CRefPtr<CAnimationPlaylist>		PlayList;
		NLMISC::CRefPtr<CChannelMixer>			ChannelMixer;

		CNode() {}
		CNode(CAnimationPlaylist *pl, CChannelMixer *mix)
		{
			PlayList= pl;
			ChannelMixer= mix;
		}
	};

	/// CAnimationPlaylist* is just the key.
	typedef	std::map<CAnimationPlaylist*, CNode>		TPlayListList;
	TPlayListList				_List;
};


} // NL3D


#endif // NL_PLAY_LIST_MANAGER_H

/* End of play_list_manager.h */
