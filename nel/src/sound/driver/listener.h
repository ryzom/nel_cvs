/** \file listener.h
 * IListener: sound listener interface
 *
 * $Id: listener.h,v 1.1 2001/06/26 15:28:10 cado Exp $
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

#ifndef NL_LISTENER_H
#define NL_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NLSOUND {


/**
 * Sound listener interface (implemented in sound driver dynamic library)
 *
 * - If the buffer is mono, the source is played in 3D mode. The coordinate
 * system is the same as OpenGl's : X points right, Y points up and Z points
 * toward the viewer/camera.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class IListener
{
public:

	/// \name Listener properties
	//@{
	/// Set the position vector (default: (0,0,0)) (3D mode only)
	virtual void			setPosition( const NLMISC::CVector& pos ) = 0;
	/** Get the position vector.
	 * See setPosition() for details.
	 */
	virtual void			getPosition( NLMISC::CVector& pos ) const = 0;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void			setVelocity( const NLMISC::CVector& vel ) = 0;
	/// Get the velocity vector
	virtual void			getVelocity( NLMISC::CVector& vel ) const = 0;
	/// Set the orientation vectors (3D mode only, ignored in stereo mode) (default: (0,1,0), (0,0,-1))
	virtual void			setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up ) = 0;
	/// Get the orientation vectors
	virtual void			getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const = 0;
	//@}

	/// \name Global properties
	//@{
	/// Set the doppler factor (default: 1) to exaggerate or not the doppler effect
	virtual void			setDopplerFactor( float f ) = 0;
	/// Set the rolloff factor (default: 1) to scale the distance attenuation effect
	virtual void			setRolloffFactor( float f ) = 0;
	/// Set DSPROPERTY_EAXLISTENER_ENVIRONMENT and DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE if EAX available (see EAX listener properties)
	virtual void			setEnvironment( uint env, float size=7.5f ) = 0;
	/// Set any EAX listener property if EAX available
	virtual void			setEAXProperty( uint prop, void *value, uint valuesize ) = 0;
	//@}

	/// Get the instance of the singleton
	static IListener		*instance()		{ return _Instance; }

	/// Constructor
							IListener();

	/// Destructor
	virtual					~IListener() { _Instance = NULL; }

protected:

	// The listener instance
	static IListener		*_Instance;
};


} // NLSOUND


#endif // NL_LISTENER_H

/* End of listener.h */
