/** \file key.h
 * class CKey
 *
 * $Id: key.h,v 1.5 2001/03/16 16:46:21 berenguier Exp $
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
#include "nel/misc/stream.h"
#include "nel/misc/rgba.h"

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

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		sint version=f.serialVersion (0);

		// Serial the value
		f.serial (Value);
	};

	/// The key value
	T					Value;


public:
	// Runtime information (used by ITrack)!!! Do not use.
	float				OODeltaTime;

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

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		sint version=f.serialVersion (0);

		// Serial the value
		f.serial (Value);
		f.serial (Tension);
		f.serial (Continuity);
		f.serial (Bias);
		f.serial (EaseTo);
		f.serial (EaseFrom);
	};

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

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		sint version=f.serialVersion (0);

		// Serial the value
		f.serial (Value);
		f.serial (InTan);
		f.serial (OutTan);
	};

	T		InTan;
	T		OutTan;
};

// Predefined types

// ** Linear keys
typedef	CKey<std::string>		CKeyString;
typedef	CKey<bool>				CKeyBool;
typedef	CKey<float>				CKeyFloat;
typedef	CKey<NLMISC::CVector>	CKeyVector;
typedef	CKey<NLMISC::CQuat>		CKeyQuat;
typedef	CKey<sint32>			CKeyInt;
typedef	CKey<NLMISC::CRGBA>		CKeyRGBA;

// ** TCB keys
typedef	CKeyTCB<float>				CKeyTCBFloat;
typedef	CKeyTCB<NLMISC::CVector>	CKeyTCBVector;
typedef	CKeyTCB<NLMISC::CAngleAxis>	CKeyTCBQuat;
typedef	CKeyTCB<sint32>				CKeyTCBInt;
typedef	CKeyTCB<NLMISC::CRGBA>		CKeyTCBRGBA;

// ** Bezier keys
typedef	CKeyBezier<float>			CKeyBezierFloat;
typedef	CKeyBezier<NLMISC::CVector>	CKeyBezierVector;
typedef	CKeyBezier<NLMISC::CQuat>	CKeyBezierQuat;
typedef	CKeyBezier<sint32>			CKeyBezierInt;
typedef	CKeyBezier<NLMISC::CRGBA>	CKeyBezierRGBA;



} // NL3D


#endif // NL_KEY_H

/* End of key.h */
