/** \file playable.h
 * IPlayable: Playable object interface, typically one sound source or a set of sound sources
 *
 * $Id: playable.h,v 1.4 2002/11/04 15:40:44 boucher Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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
#error "Deprecated"

#ifndef NL_PLAYABLE_H
#define NL_PLAYABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector.h"

#error "deprecated"
namespace NLSOUND {


/**
 * Playable object interface, typically one sound source or a set of sound sources
 *
 * This class is used by CEnvSound to handle in the same way center sources
 * and ambiant sources.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class IPlayable //: public NLMISC::IStreamable
{
public:

	/// Serialize useful data
	/*

		The serial interface is no longer used.	The files should now be stored as
		George files. [PH, 20020620]

    */
	//virtual void		serial( NLMISC::IStream& s ) = 0;
	/** Init. You can pass a position vector to link to (if the playable has stereo source(s))
	 * When reading from a stream, call init() *after* serial().
	 */
	virtual void		initPos( const NLMISC::CVector *posvector ) { /*method not required*/ }
	/// Set the position
	virtual void		moveTo( const NLMISC::CVector& pos ) { /*method not required*/ }
	/// Enable (play with high priority) and set general gain, or disable (stop and set low priority).
	virtual void		enable( bool toplay, float gain ) = 0;
	/// Update
	virtual void		update() { /*method not required*/ }


	/// Destructor
	virtual ~IPlayable() {}

protected:

	/// Constructor
	IPlayable() {}
};


} // NLSOUND


#endif // NL_PLAYABLE_H

/* End of playable.h */
