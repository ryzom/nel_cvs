/** \file physics.h
 * Physics computation for particles (and thus snowballs)
 *
 * $Id: physics.h,v 1.1 2001/07/20 14:36:30 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef PHYSICS_H
#define PHYSICS_H

//
// Includes
//

#include <nel/misc/vector.h>
#include <nel/misc/time_nl.h>

//
// External variables
//

// The gravity force value
extern float		Gravity;


//
// External classes
//

class CTrajectory
{
	NLMISC::CVector		_StartPosition;
	NLMISC::CVector		_StartSpeed;
	NLMISC::TTime		_StartTime;

public:
	void				init(const NLMISC::CVector &position, const NLMISC::CVector &speed, NLMISC::TTime startTime)
	{
		_StartPosition = position;
		_StartSpeed = speed;
		_StartTime = startTime;
	}

//	void				compute(const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, NLMISC::TTime startTime);

	NLMISC::CVector		eval(NLMISC::TTime t) const;
	NLMISC::CVector		evalSpeed(NLMISC::TTime t) const;

	NLMISC::TTime		getStartTime() const { return _StartTime; }
	NLMISC::CVector		getStartPosition() const { return _StartPosition; }
	NLMISC::CVector		getStartSpeed() const { return _StartSpeed; }
};

#endif // PHYSICS_H

/* End of physics.h */
