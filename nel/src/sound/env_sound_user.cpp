/** \file env_sound_user.cpp
 * CEnvSoundUser: implementation of UEnvSound
 *
 * $Id: env_sound_user.cpp,v 1.8 2001/07/19 12:50:43 cado Exp $
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

#include "env_sound_user.h"
#include "sound.h"
#include "audio_mixer_user.h"
#include "bounding_shape.h"
#include <stdlib.h>

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


/*
 * Constructor
 */
CEnvSoundUser::CEnvSoundUser() : _Play(false), _Source(NULL), _BoundingShape(NULL), _Parent(NULL), _Mark(false), _Gain(1.0f)
{
}


/*
 * Destructor
 */
CEnvSoundUser::~CEnvSoundUser()
{
	vector<CEnvSoundUser*>::iterator ipe;
	for ( ipe=_Children.begin(); ipe!=_Children.end(); ++ipe )
	{
		delete (*ipe);
	}

	vector<IPlayable*>::iterator ipp;
	for ( ipp=_SrcBank.begin(); ipp!=_SrcBank.end(); ++ipp )
	{
		delete (*ipp);
	}

	if ( _BoundingShape != NULL )
	{
		delete _BoundingShape;
	}
}


/*
 * Serialize (recursive)
 */
void CEnvSoundUser::serial( NLMISC::IStream& s )
{
	// If you change this, increment the version number in CEnvSoundUser::load() !

	s.serial( _Transition );
	s.serialPolyPtr( _BoundingShape );
	s.serialContPolyPtr( _SrcBank ); // serializes sound and looping state only

	uint16 srcindex;

	if ( s.isReading() )
	{
		// Select the source in the bank
		s.serial( srcindex );
		if ( ! _SrcBank.empty() )
		{
			_Source = _SrcBank[srcindex];
		}
		else
		{
			_Source = NULL;
		}

		// Set the position which was not serialized
		if ( (_Source!=NULL) && (_BoundingShape != NULL) )
		{
			vector<IPlayable*>::iterator ipp;
			for ( ipp=_SrcBank.begin(); ipp!=_SrcBank.end(); ++ipp )
			{
				(*ipp)->moveTo( _BoundingShape->getCenter() );
			}
		}

		// Init the source (not transition)
		if ( ! _Transition )
		{
			if ( (_Source != NULL) )
			{
				vector<IPlayable*>::iterator ipp;
				if ( _BoundingShape != NULL )
				{
					for ( ipp=_SrcBank.begin(); ipp!=_SrcBank.end(); ++ipp )
					{
						(*ipp)->initPos( const_cast<CVector*>(&(_BoundingShape->getCenter())) );
					}
				}
				else
				{
					// The world envsound will be heard at the listener
					for ( ipp=_SrcBank.begin(); ipp!=_SrcBank.end(); ++ipp )
					{
						(*ipp)->initPos( &CAudioMixerUser::instance()->getListenPosVector() );
					}
				}
			}
		}
	}
	else
	{
		uint i;
		// Find the index of the current source in the bank and serialize it out
		if ( _SrcBank.size() != 0 )
		{
			for ( i=0; i!=_SrcBank.size(); i++ )
			{
				if ( _SrcBank[i] == _Source )
				{
					break;
				}
			}
			nlassert( i != _SrcBank.size() );
			nlassert( i < 0x10000 ); // 16-bit value
			srcindex = i;
		}
		else
		{
			srcindex = 0;
		}
		s.serial( srcindex );
	}

	// Tags
	s.serialCont( _Tags );

	// Children envsounds
	s.serialPtr( _Parent );
	s.serialContPtr( _Children );
}


/*
 * Select current env
 */
void			CEnvSoundUser::selectEnv( const std::string& tag )
{
	uint i;
	for ( i=0; i!= _Tags.size(); i++ )
	{
		if ( _Tags[i] == tag )
		{
			_Source->enable( false, 1.0f );
			_Source = _SrcBank[i];
			nldebug( "AM: EnvSound: Environment changed to %s", tag.c_str() );
			CAudioMixerUser::instance()->getEnvSounds()->recompute();
			return;
		}
	}
	nldebug( "AM: EnvSound: Environment %s not found", tag.c_str() );
	// Don't change _Source if not found
}


/*
 * Serialize file header
 */
void	CEnvSoundUser::serialFileHeader( NLMISC::IStream& s )
{
	s.serialCheck( (uint32)'SEN' ); // NeL Environment Sounds
	s.serialVersion( 1 );
}


/*
 * Count the envs in the tree (call on the root)
 */
uint32	CEnvSoundUser::getCount() const
{
	uint32 cnt=1;
	vector<CEnvSoundUser*>::const_iterator ipe;
	for ( ipe=_Children.begin(); ipe!=_Children.end(); ++ipe )
	{
		cnt += (*ipe)->getCount();
	}
	return cnt;
}


/*
 * Load several envsounds and return the number of envsounds loaded
 */
uint32 CEnvSoundUser::load( CEnvSoundUser* &envSoundTreeRoot, NLMISC::IStream& s )
{
	if ( s.isReading() )
	{
		serialFileHeader( s );
		s.serialPtr( envSoundTreeRoot );
		return envSoundTreeRoot->getCount();
	}
	else
	{
		nlstop;
		return 0;
	}
}


/*
 * Update the stereo mixes (call evenly on the root) (recursive)
 */
void			CEnvSoundUser::update()
{
	if ( _Source != NULL )
	{
		_Source->update();
	}
	vector<CEnvSoundUser*>::iterator ipe;
	for ( ipe=_Children.begin(); ipe!=_Children.end(); ++ipe )
	{
		(*ipe)->update();
	}
}


/*
 * Find the area where the listener is located (recursive)
 */
CEnvSoundUser *CEnvSoundUser::findCurrentEnv( const NLMISC::CVector& listenerpos )
{
	// Find in children first (check from leaves to root)
	vector<CEnvSoundUser*>::iterator ipe = _Children.begin();
	CEnvSoundUser *found = NULL;
	while ( ! ( (found) || (ipe==_Children.end()) ) )
	{
		found = (*ipe)->findCurrentEnv( listenerpos );
		ipe++;
	}
	if ( found )
		return found;
	else if ( (_BoundingShape == NULL) || (_BoundingShape->include( listenerpos )) )
		return this;
	else
		return NULL;
}


/*
 * Return the position
 */
void CEnvSoundUser::getPos( NLMISC::CVector& pos ) const
{
	if ( _BoundingShape == NULL )
	{
		pos = CVector::Null;
	}
	else
	{
		pos = _BoundingShape->getCenter();
	}
}


/*
 * Moves the envsound (and its transition envsound if it has one)
 */
void CEnvSoundUser::setPos( const NLMISC::CVector& pos )
{
	if ( _BoundingShape != NULL )
	{
		// Get the vector between the pos of this envsound and the pos of its transition envsound
		CVector newpos;
		if ( (_Parent != NULL) && ( _Parent->_Transition ) )
		{
			newpos = pos + _Parent->_BoundingShape->getCenter() - _BoundingShape->getCenter();
		}
		else
		{
			newpos = pos;
		}

		// Set the new pos
		_BoundingShape->setCenter( pos );
		if ( (_Parent != NULL) && ( _Parent->_Transition ) )
		{
			_Parent->_BoundingShape->setCenter( newpos );
			_Parent->_Source->moveTo( newpos );
		}

		// Recompute the entire tree
		CAudioMixerUser::instance()->getEnvSounds()->recompute();
	}
}


/*
 * Return the children envsounds
 */
std::vector<UEnvSound*>& CEnvSoundUser::getChildren()
{
	return (vector<UEnvSound*>&)(_Children);
}


/*
 * Play or stop the sources (call only on the root env)
 */
void CEnvSoundUser::recompute()
{
	nlassert( isRoot() );

	// Find the area of the listener
	CVector listenerpos;
	CAudioMixerUser::instance()->getListener()->getPos( listenerpos );
	CEnvSoundUser *current = findCurrentEnv( listenerpos );

	// Mark the envs that have to play their source
	if ( current != NULL )
	{
		current->markSources( listenerpos, 1.0f );
	}
	
	// Enable/disable the sources in the hierarchy, and reset the marks
	applySourcesMarks();
}


/* Prepare the related sources to play (recursive).
 * In each children branch, there must be an env which is not a transition, for the recursion to stop
 */
void CEnvSoundUser::markSources( const NLMISC::CVector& listenerpos, float gain )
{
	// Is the listener in a transition area ?
	if ( _Transition )
	{
		//nldebug( "AM: EnvSound: Marking sources for transition between child and parent" );

		// Compute the listener position to find the ratio between up and down envs
		nlassert( (_Children.size() == 1) && (_Children[0] != NULL) && (_Parent != NULL) );
		nlassert( _BoundingShape && _Children[0]->_BoundingShape );
		float ratio = _BoundingShape->getRatio( listenerpos, _Children[0]->_BoundingShape );

		// The child env plays at gain*ratio
		// The recursion stops because the child env is not a transition area
		_Children[0]->markSources( listenerpos, gain * ratio );

		// The parent env (therefore the 3d source of the current env as well) plays at gain*(1-ratio)
		// The recursion stops because the parent env is not a transition area
		_Parent->markSources( listenerpos, gain * (1.0f-ratio) );
	}
	else
	{
		//nldebug( "AM: EnvSound: Marking sources for environnement" );

		// The listener in an environment, containing other environments (e.g. a town) or not (e.g. a room).
		// The current env plays
		_Mark = true;
		_Gain = gain;

		// The children env (next level only) play
		vector<CEnvSoundUser*>::iterator ipe;
		for( ipe=_Children.begin(); ipe!=_Children.end(); ++ipe )
		{
			(*ipe)->_Mark = true;
			(*ipe)->_Gain = gain;
		}
	}
}


/*
 * Enable/disable the source and set general gain if enabled, and reset the source mark (recursive)
 */
void CEnvSoundUser::applySourcesMarks()
{
	if ( ! _Play )
	{
		_Mark = false;
	}
	if ( _Source != NULL )
	{
		_Source->enable( _Mark, _Gain );
	}
	_Mark = false;

	// Apply on children
	vector<CEnvSoundUser*>::iterator ipe;
	for ( ipe=_Children.begin(); ipe!=_Children.end(); ++ipe )
	{
		(*ipe)->applySourcesMarks();
	}
}


/*
 * Set properties (EDIT)
 */
void		CEnvSoundUser::setProperties( bool transition, IBoundingShape *bshape )
{
	_Transition = transition;
	_BoundingShape = bshape;
}


/*
 * Add an environment source/tag (EDIT) (set a NULL source for no source at all)
 * The current source always becomes the first one.
 */
void		CEnvSoundUser::addEnvTag( IPlayable *source, const std::string& tag )
{
	_SrcBank.push_back( source );
	_Tags.push_back( tag );
	if ( _Source == NULL )
	{
		_Source = source;
	}
}


/*
 * Save (output stream only) (EDIT)
 */
void CEnvSoundUser::save( CEnvSoundUser *envSoundTreeRoot, NLMISC::IStream& s )
{
	nlassert( ! s.isReading() );
	nlassert( envSoundTreeRoot->isRoot() );

	serialFileHeader( s );
	s.serialPtr( envSoundTreeRoot );
}


/*
 * Play
 */
void CEnvSoundUser::play()
{
	_Play = true;

	// Start transition as well
	if ( (_Parent != NULL) && ( _Parent->_Transition ) )
	{
		_Parent->_Play = true;
	}

	CAudioMixerUser::instance()->getEnvSounds()->recompute();
}


/*
 * Stop playing
 */
void CEnvSoundUser::stop()
{
	_Play = false;

	// Stop transition as well
	if ( (_Parent != NULL) && ( _Parent->_Transition ) )
	{
		_Parent->_Play = false;
	}

	CAudioMixerUser::instance()->getEnvSounds()->recompute();
}


/*
 * Add a child (EDIT)
 */
void CEnvSoundUser::addChild( CEnvSoundUser *child ) 
{
	child->_Parent = this;
	_Children.push_back( child );
}


} // NLSOUND
