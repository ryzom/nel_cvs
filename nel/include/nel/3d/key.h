/** \file key.h
 * class CKey
 *
 * $Id: key.h,v 1.1 2001/02/05 16:52:44 corvazier Exp $
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

#ifndef NL_KEY_H
#define NL_KEY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"

// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.

namespace NL3D 
{


/**
 * Interface for a key of a keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CKey
{
public:
	T					Value;
};


/**
 * Implementation of CKey for TCB keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CKeyTCB : public CKey<T>
{
public:
	float	Tension;
	float	Continuity;
	float	Bias;
	float	EaseTo;
	float	EaseFrom;
};


/**
 * Implementation of CKey for Bezier keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CKeyBezier : public CKey<T>
{
public:
	T		InTan;
	T		OutTan;
};

// Predefined types
typedef CKeyTCB<float> CKeyTCBFloat;
typedef CKeyTCB<NLMISC::CVector> CKeyTCBVector;
typedef CKeyTCB<NLMISC::CQuat> CKeyTCBQuat;
typedef CKeyTCB<int> CKeyTCBInt;

typedef CKeyBezier<float> CKeyBezierFloat;
typedef CKeyBezier<NLMISC::CVector> CKeyBezierVector;
typedef CKeyBezier<NLMISC::CQuat> CKeyBezierQuat;
typedef CKeyBezier<int> CKeyBezierInt;

typedef CKey<std::string> CKeyString;
typedef CKey<bool> CKeyBool;


} // NL3D


#endif // NL_KEY_H

/* End of key.h */
