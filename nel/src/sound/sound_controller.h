/** \file sound_controller.h
 *
 * Used for the continuous control of volume envelopes and frequencies. 
 *
 * $Id: sound_controller.h,v 1.1 2002/06/28 20:38:54 hanappe Exp $
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


#ifndef NL_SOUND_CONTROLLER_H
#define NL_SOUND_CONTROLLER_H

namespace NLSOUND
{

class ISoundController 
{
public:

	ISoundController() {}
	virtual ~ISoundController() {}

	virtual float getValue(float time) = 0;
};

} // namespace

#endif // NL_SOUND_CONTROLLER_H
