/** \file track.cpp
 * <File description>
 *
 * $Id: track.cpp,v 1.6 2001/03/08 11:02:52 corvazier Exp $
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

#include "nel/3d/track.h"


namespace NL3D 
{

// Some compilation check: force Visual to compile to template
CTrackKeyFramerTCBFloat ttoto0;
CTrackKeyFramerTCBVector ttoto1;
CTrackKeyFramerTCBQuat ttoto2;
CTrackKeyFramerTCBInt ttoto3;


CTrackKeyFramerBezierFloat ttoto4;
CTrackKeyFramerBezierVector ttoto5;
CTrackKeyFramerBezierQuat ttoto6;
CTrackKeyFramerBezierInt ttoto7;


CTrackKeyFramerLinearFloat	lattoto10;
CTrackKeyFramerLinearVector	lattoto11;
CTrackKeyFramerLinearQuat	lattoto12;
CTrackKeyFramerLinearInt	lattoto13;


CTrackKeyFramerConstFloat	attoto10;
CTrackKeyFramerConstVector	attoto11;
CTrackKeyFramerConstQuat	attoto12;
CTrackKeyFramerConstInt		attoto13;
CTrackKeyFramerConstString	attoto14;
CTrackKeyFramerConstBool	attoto15;


CTrackDefaultFloat ttoto10;
CTrackDefaultVector ttoto11;
CTrackDefaultQuat ttoto12;
CTrackDefaultInt ttoto13;
CTrackDefaultString ttoto14;
CTrackDefaultBool ttoto15;


// ***************************************************************************
// ***************************************************************************
// Quaternions special implementation..
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void CTrackKeyFramerLinear<CKeyQuat, NLMISC::CQuat>::evalKey (	
			const CKeyQuat* previous, const CKeyQuat* next, 
			CAnimationTime datePrevious, CAnimationTime dateNext, 
			CAnimationTime date )
{
	if(previous && next)
	{
		// slerp from previous to cur.
		date-= datePrevious;
		date/= (dateNext-datePrevious);
		NLMISC::clamp(date, 0,1);
		_Value.Value= NLMISC::CQuat::slerp(previous->Value, next->Value, date);
	}
	else
	{
		if (previous)
			_Value.Value=previous->Value;
		else
			if (next)
				_Value.Value=next->Value;
	}

}




} // NL3D
