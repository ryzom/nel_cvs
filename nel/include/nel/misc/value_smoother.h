/** \file value_smoother.h
 * <File description>
 *
 * $Id: value_smoother.h,v 1.4 2002/10/30 17:00:37 berenguier Exp $
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

#ifndef NL_VALUE_SMOOTHER_H
#define NL_VALUE_SMOOTHER_H

#include "nel/misc/types_nl.h"

#include <vector>


namespace NLMISC {


// ***************************************************************************
/**
 * A smoother of values.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CValueSmoother
{
public:

	/// Constructor
	CValueSmoother(uint n=16);

	/// reset the ValueSmoother, and set the number of frame to smooth.
	void		init(uint n);

	/// reset only the ValueSmoother
	void		reset();

	/// add a new value to be smoothed.
	void		addValue(float dt);

	/// get the smoothed value.
	float		getSmoothValue();

private:
	std::vector<float>		_LastFrames;
	uint					_CurFrame;
	uint					_NumFrame;
	float					_FrameSum;


};


} // NLMISC


#endif // NL_VALUE_SMOOTHER_H

/* End of value_smoother.h */
