/** \file u_env_sound.h
 * UEnvSound: Game interface for localized environment sounds
 *
 * $Id: u_env_sound.h,v 1.1 2001/07/10 16:51:20 cado Exp $
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

#ifndef NL_U_ENV_SOUND_H
#define NL_U_ENV_SOUND_H

#include "nel/misc/types_nl.h"


namespace NLSOUND {


class USource;


/**
 * Game interface for localized environment sounds
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class UEnvSound
{
public:

	/// Return the corresponding localized 3D sound source
	virtual USource			*get3DSource() = 0;

	/// Recompute the EnvSound. Call this method after having moved the 3D source.
	virtual void			recompute() = 0;

	/// Play
	virtual void			play() = 0;

	/// Stop playing
	virtual void			stop() = 0;

	/// Destructor
	virtual					~UEnvSound() {}

protected:

	/// Constructor
	UEnvSound() {}

};


} // NLSOUND


#endif // NL_U_ENV_SOUND_H

/* End of u_env_sound.h */
