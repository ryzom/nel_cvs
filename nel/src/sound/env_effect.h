/** \file env_effect.h
 * CEnvEffect: environmental effects and where they are applied
 *
 * $Id: env_effect.h,v 1.3 2001/07/17 14:21:54 cado Exp $
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
#include "bounding_shape.h"

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
	/// Destructor
	virtual ~CEnvEffect() {}
	/// Serialize
	void			serial( NLMISC::IStream& s )
	{
		// If you change this, increment the version number in serialFileHeader()
		s.serialPolyPtr( _BoundingShape );
		s.serialCont( _EnvNums );
		s.serialCont( _Tags );
	}
	/// Serialize file header
	static void		serialFileHeader( NLMISC::IStream& s, uint32& nb );
	/// Load several EnvEffects and return the number of effects loaded
	static uint32	load( std::vector<CEnvEffect*>& container, NLMISC::IStream& s );

	/// Return true if the environment includes the specified point
	bool			include( const NLMISC::CVector& pos ) { return _BoundingShape->include( pos ); }
	/// Select the current environment
	void			selectEnv( const std::string& tag );
	/// Return the environment type
	uint			getEnvNum() const		{ return _EnvNums[_Current]; }
	/// Return the environment size
	float			getEnvSize() const		{ return _BoundingShape->getDiameter(); }


	/// Set the environment type (EDIT)
	void			addEnvNum( uint8 num, const std::string& tag="" );
	/// Access the bounding shape (EDIT)
	IBoundingShape	*getBoundingShape()		{ return _BoundingShape; }
	/// Set the bounding shape (EDIT)
	void			setBoundingShape( IBoundingShape *bs ) { _BoundingShape = bs; }
	/// Save (output stream only) (EDIT)
	static void		save( const std::vector<CEnvEffect>& container, NLMISC::IStream& s );


private:

	// Index of the current environment type
	uint8						_Current;

	// Types of environment
	std::vector<uint8>			_EnvNums;

	// Environment tags (indicate which env to select)
	std::vector<std::string>	_Tags;

	// Bounding shape
	IBoundingShape				*_BoundingShape;
};


} // NLSOUND


#endif // NL_ENV_EFFECT_H

/* End of env_effect.h */
