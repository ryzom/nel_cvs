/** \file key.h
 * class CKey
 *
 * $Id: key.h,v 1.4 2001/03/13 17:08:58 corvazier Exp $
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
class CKey : public NLMISC::IStreamable
{
public:

	/// Serial
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		sint version=f.serialVersion (0);

		// Serial the value
		f.serial (Value);
	};

	/// The key value
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

	/// Serial
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
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
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
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
class CKeyString : public CKey<std::string>
{
public:
	NLMISC_DECLARE_CLASS (CKeyString);
};
class CKeyBool : public CKey<bool>
{
public:
	NLMISC_DECLARE_CLASS (CKeyBool);
};
class CKeyFloat : public CKey<float>
{
public:
	NLMISC_DECLARE_CLASS (CKeyFloat);
};
class CKeyVector : public CKey<NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CKeyVector);
};
class CKeyQuat : public CKey<NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CKeyQuat);
};
class CKeyInt : public CKey<sint32>
{
public:
	NLMISC_DECLARE_CLASS (CKeyInt);
};
class CKeyRGBA : public CKey<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CKeyRGBA);
};

// ** TCB keys
class CKeyTCBFloat : public CKeyTCB<float>
{
public:
	NLMISC_DECLARE_CLASS (CKeyTCBFloat);
};
class CKeyTCBVector : public CKeyTCB<NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CKeyTCBVector);
};
class CKeyTCBQuat : public CKeyTCB<NLMISC::CAngleAxis>
{
public:
	NLMISC_DECLARE_CLASS (CKeyTCBQuat);
};
class CKeyTCBInt : public CKeyTCB<sint32>
{
public:
	NLMISC_DECLARE_CLASS (CKeyTCBInt);
};
class CKeyTCBRGBA : public CKeyTCB<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CKeyTCBRGBA);
};

// ** Bezier keys
class CKeyBezierFloat : public CKeyBezier<float>
{
public:
	NLMISC_DECLARE_CLASS (CKeyBezierFloat);
};
class CKeyBezierVector : public CKeyBezier<NLMISC::CVector>
{
public:
	NLMISC_DECLARE_CLASS (CKeyBezierVector);
};
class CKeyBezierQuat : public CKeyBezier<NLMISC::CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CKeyBezierQuat);
};
class CKeyBezierInt : public CKeyBezier<sint32>
{
public:
	NLMISC_DECLARE_CLASS (CKeyBezierInt);
};
class CKeyBezierRGBA : public CKeyBezier<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CKeyBezierRGBA);
};



} // NL3D


#endif // NL_KEY_H

/* End of key.h */
