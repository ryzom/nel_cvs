/** \file env_effect.h
 * CEnvEffect: environmental effects and where they are applied
 *
 * $Id: env_effect.h,v 1.1 2001/07/10 16:48:03 cado Exp $
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

#ifndef NL_ENV_EFFECT_H
#define NL_ENV_EFFECT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector.h"

namespace NLSOUND {


/**
 * Environmental effects and where they are applied
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CEnvEffect
{
public:

	/// Constructor
	CEnvEffect();
	/// Serialize
	void			serial( NLMISC::IStream& s )
	{
		// If you change this, increment the version number in serialFileHeader()
		s.serial( _Corner1 );
		s.serial( _Corner2 );
		s.serialCont( _EnvNums );
		s.serialCont( _Tags );
	}
	/// Serialize file header
	static void		serialFileHeader( NLMISC::IStream& s, uint32& nb );
	/// Load several EnvEffects and return the number of effects loaded
	static uint32	load( std::vector<CEnvEffect*>& container, NLMISC::IStream& s );

	/// Does the box include a point ?
	bool			include( const NLMISC::CVector& pos );
	/// Select the current environment
	void			selectEnv( const std::string& tag );


	/// Return the environment type
	uint			getEnvNum() const		{ return _EnvNums[_Current]; }
	/// Return the environment size
	float			getEnvSize() const;


	/// Return the corners (EDIT)
	void			getCorners( NLMISC::CVector& c1, NLMISC::CVector& c2 )
											{ c1 = _Corner1; c2 = _Corner2; }
	/// Set the environment type (EDIT)
	void			addEnvNum( uint8 num, const std::string& tag="" );
	/// Set the corners (the corners must be ordered: 1 has smaller x, y, z) (EDIT)
	void			setCorners( const NLMISC::CVector& c1, const NLMISC::CVector& c2 )
											{ _Corner1 = c1; _Corner2 = c2; }
	/// Save (output stream only) (EDIT)
	static void		save( const std::vector<CEnvEffect>& container, NLMISC::IStream& s );


private:

	// Index of the current environment type
	uint8						_Current;

	// Types of environment
	std::vector<uint8>			_EnvNums;

	// Environment tags (indicate which env to select)
	std::vector<std::string>	_Tags;

	// For now, its a box (the corners must be ordered: 1 has smaller x, y, z)
	NLMISC::CVector				_Corner1, _Corner2;
};


} // NLSOUND


#endif // NL_ENV_EFFECT_H

/* End of env_effect.h */
