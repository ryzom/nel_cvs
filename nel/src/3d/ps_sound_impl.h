/** \file ps_sound_impl.h
 * <File description>
 *
 * $Id: ps_sound_impl.h,v 1.2 2001/08/09 08:03:19 vizerie Exp $
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

#ifndef NL_PS_SOUND_IMPL_H
#define NL_PS_SOUND_IMPL_H

#include "nel/sound/u_audio_mixer.h"
#include "nel/misc/types_nl.h"


namespace NL3D {


/**
 * This class implements PS sound server. It warps the calls to NEL sound. Everything is in a .h file to avoid dependency
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSSoundServImpl : public IPSSoundServer
{
public:	
	IPSSoundInstance *createSound(const std::string &soundName)
	{
		// for now, we just load a .wav
		USoucre *source = createSource(
	}

};


} // NL3D


#endif // NL_PS_SOUND_IMPL_H

/* End of ps_sound_impl.h */
