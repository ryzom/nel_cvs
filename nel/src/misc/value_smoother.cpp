/** \file value_smoother.cpp
 * <File description>
 *
 * $Id: value_smoother.cpp,v 1.4 2002/10/30 17:00:13 berenguier Exp $
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

#include "stdmisc.h"

#include "nel/misc/value_smoother.h"


using namespace std;


namespace NLMISC 
{


CValueSmoother::CValueSmoother(uint n)
{
	nlassert(n!=0);
	init(n);
}

void		CValueSmoother::reset()
{
	fill(_LastFrames.begin(), _LastFrames.end(), 0);

	_CurFrame= 0;
	_NumFrame= 0;
	_FrameSum= 0;
}

void		CValueSmoother::init(uint n)
{
	nlassert(n!=0);

	// reset all the array to 0.
	_LastFrames.clear();
	_LastFrames.resize(n, 0);

	_CurFrame= 0;
	_NumFrame= 0;
	_FrameSum= 0;
}

void		CValueSmoother::addValue(float dt)
{
	// update the frame sum. NB: see init(), at start, array is full of 0. so it works even for un-inited values.
	_FrameSum-= _LastFrames[_CurFrame];
	_FrameSum+= dt;

	// bkup this value in the array.
	_LastFrames[_CurFrame]= dt;

	// next frame.
	_CurFrame++;
	_CurFrame%=_LastFrames.size();

	// update the number of frames added.
	_NumFrame++;
	_NumFrame= min(_NumFrame, _LastFrames.size());
}

float		CValueSmoother::getSmoothValue()
{
	if(_NumFrame)
		return _FrameSum / _NumFrame;
	else
		return 0;
}



} // NLMISC
