/** \file mixing_track.h
 * CTrack: a source selected for playing
 *
 * $Id: mixing_track.h,v 1.2 2002/06/28 19:32:24 hanappe Exp $
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

#ifndef NL_TRACK_H
#define NL_TRACK_H

#include "nel/misc/types_nl.h"
#include "driver/sound_driver.h"
#include "driver/source.h"


namespace NLSOUND {


class CSourceUser;


/**
 * A source selected for playing
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CTrack
{
public:

	/// Constructor
	CTrack() : DrvSource(NULL), _UserSource(NULL) {}
	/// Init
	void			init( ISoundDriver *sd )			{ DrvSource = sd->createSource(); }
	/// Destructor
	virtual			~CTrack()							{ if ( DrvSource != NULL ) delete DrvSource; }


	/// Return availability
	// FIXME: SWAPTEST
	//bool			isAvailable() const					{ return (_UserSource==NULL); }
	bool			isAvailable() const					{ nlassert( DrvSource != NULL ); return (_UserSource==NULL) && DrvSource->isStopped(); }
	/// Returns true if the track is physically playing (different from getUserSource()->isPlaying())
	bool			isPlaying() const					{ nlassert( DrvSource != NULL ); return DrvSource->isPlaying(); }
	/// Set logical source (if NULL, the track becomes available)
	void			setUserSource( CSourceUser *src)	{ _UserSource = src; }
	/// Return the logical source
	CSourceUser		*getUserSource()					{ return _UserSource; }


	/// Physical source played by the driver
	ISource			*DrvSource;

private:
	
	/// The current logical source
	CSourceUser		*_UserSource;

};


} // NLSOUND


#endif // NL_TRACK_H

/* End of mixing_track.h */
