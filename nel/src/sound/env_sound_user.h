/** \file env_sound_user.h
 * CEnvSoundUser: implementation of UEnvSound
 *
 * $Id: env_sound_user.h,v 1.3 2001/07/17 14:21:54 cado Exp $
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

#ifndef NL_ENV_SOUND_USER_H
#define NL_ENV_SOUND_USER_H

#include "nel/misc/types_nl.h"
#include "nel/sound/u_env_sound.h"
#include "source_user.h"
#include <vector>


namespace NLSOUND {


class CListenerUser;
class IPlayable;
class IBoundingShape;


// Number of environment channels
#define MAX_ENV_CHANNELS 3
#define AMBIANT_CH1 0
#define AMBIANT_CH2 1
#define SPARSE_CH 2


/**
 * Implementation of UEnvSound
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CEnvSoundUser : public UEnvSound
{
public:

	/// Constructor
	CEnvSoundUser();

	/// Destructor
	virtual					~CEnvSoundUser();

	
	/// Serialize
	void					serial( NLMISC::IStream& s );
	/// Serialize file header
	static void				serialFileHeader( NLMISC::IStream& s );
	/// Load several envsounds and return the number of envsounds loaded
	static uint32			load( CEnvSoundUser* &envSoundTreeRoot, NLMISC::IStream& s );


	/// Return the position
	virtual void			getPos( NLMISC::CVector& pos ) const;
	/// Moves the envsound (and its transition envsound if it has one)
	virtual void			setPos( const NLMISC::CVector& pos );
	/// Return the children envsounds
	virtual std::vector<UEnvSound*>& getChildren();


	/// Recompute the EnvSound. Call this method after having moved the 3D source (call only on the root env).
	void					recompute();
	/// Update the stereo mixes (call evenly on the root) (recursive)
	void					update();


	/// Play
	virtual void			play();
	/// Stop playing
	virtual void			stop();
	/// Return the play flag
	bool					mustPlay() const		{ return _Play; }


	/// Set properties (EDIT)
	void					setProperties( bool transition,
										   IBoundingShape *bshape,	// set NULL for all world
										   IPlayable *source );		// set NULL for no source
	/// Add a child (EDIT)
	void					addChild( CEnvSoundUser *child );
	/// Save (output stream only) (EDIT)
	static void				save( CEnvSoundUser *envSoundTreeRoot, NLMISC::IStream& s );
	/// Return the bounding shape (EDIT)
	IBoundingShape			*getBoundingShape() { return _BoundingShape; }
	/// Return the source (EDIT)
	IPlayable				*getSource()		{ return _Source; }
	/// Return the parent (EDIT)
	CEnvSoundUser			*getParent()		{ return _Parent; }

protected:

	// Return true if the envsound is a leaf in the hierarchy tree
	//bool					isLeaf() const	{ return _Children.empty(); }

	/// Return true if the envsound is the root of the hierarchy tree
	bool					isRoot() const	{ return _Parent==NULL; }

	// Enable/disable the source and set general gain if enabled, and reset the source mark (recursive)
	void					applySourcesMarks();

	/// Find the area where the listener is located (recursive)
	CEnvSoundUser			*findCurrentEnv( const NLMISC::CVector& listenerpos );

	/** Prepare the related sources to play (recursive).
	 * In each children branch, there must be an env which is not a transition, for the recursion to stop
	 */
	void					markSources( const NLMISC::CVector& listenerpos, float gain, CEnvSoundUser *except=NULL );

	/// Count the envs in the tree (call on the root)
	uint32					getCount() const;

private:

	// Must play or not
	bool						_Play;

	// Sound source (3D source or ambiant source)
	IPlayable					*_Source;

	// Area
	IBoundingShape				*_BoundingShape;

	// Transition shape ?
	bool						_Transition;

	// Hierarchy
	CEnvSoundUser				*_Parent;
	std::vector<CEnvSoundUser*>	_Children;

	// Playing preparation
	bool						_Mark;
	float						_Gain;

};


} // NLSOUND


#endif // NL_ENV_SOUND_USER_H

/* End of env_sound_user.h */
