/** \file track.h
 * CTrack: a source selected for playing
 *
 * $Id: track.h,v 1.1 2001/07/10 16:48:03 cado Exp $
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
	CTrack();

	/// Destructor
	virtual			~CTrack()							{ delete DrvSource; }
	
	/// Return availability
	bool			available() const					{ return _Available; }
	/// Set availability
	void			setAvailable( bool av )				{ _Available = av; }

	/// Source played
	ISource			*DrvSource;

#ifdef NL_DEBUG
	// Debug info
	CSourceUser		*UserSource;
#endif

private:
	
	bool			_Available;

};


} // NLSOUND


#endif // NL_TRACK_H

/* End of track.h */
