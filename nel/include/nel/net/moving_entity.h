/** \file moving_entity.h
 * Interface for all moving entities
 *
 * $Id: moving_entity.h,v 1.4 2000/11/07 16:44:44 cado Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_MOVING_ENTITY_H
#define NL_MOVING_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NLNET {


/// Entity identifier type.
typedef uint32 TEntityId;

/// Position type. Unit: meter
typedef float TPosUnit;

/// Duration. Unit: second
typedef float TDuration;

/// Velocity type. Unit: TPosUnit per second
typedef TPosUnit TVelocity;

/// Angle type. Unit: radian
typedef float TAngle;

/// Angular velocity type. Unit: radian per second
typedef TAngle TAngVelocity;


/**
 * Base class for all moving entities.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class IMovingEntity
{
public:

	/// Constructor
	IMovingEntity();

	/// Alt. constructor
	IMovingEntity( const NLMISC::CVector& pos,
				   const NLMISC::CVector& hdg,
				   const TAngle rollangle,
				   const NLMISC::CVector& vec,
				   const TAngVelocity av,
   				   bool groundmode );

	/// Copy constructor
	IMovingEntity( const IMovingEntity& other );

	/// Sets ground mode on/off
	void					setGroundMode( bool gm )	{ _GroundMode = gm; }

	/// Update the entity state
	void					update( TDuration deltatime )
	{
		computePosAfterDuration( deltatime );
	}


	/// Entity state properties
	//@{

	/// Returns identifier
	TEntityId				id() const			{ return _Id; }

	/// Returns position
	const NLMISC::CVector&	pos() const			{ return _Pos; }

	/// Returns heading
	const NLMISC::CVector&	bodyHeading() const	{ return _BodyHdg; }

	/// Returns left vector
	const TAngle&			rollAngle() const	{ return _RollAngle; }

	/// Returns trajectory vector
	const NLMISC::CVector&	trajVector() const	{ return _Vector; }

	/// Returns angular velocity
	const TAngVelocity		angularVelocity() const	{ return _AngVel; }

	/// Returns true if the entity is in ground mode
	bool					groundMode() const	{ return _GroundMode; }
	
	//@}


	/// Angle around z axis from x axis
	TAngle					angleAroundZ();
	/// Angle around x axis from y axis
	TAngle					angleAroundX();
	/// Angle around y axis from z axis
	TAngle					angleAroundY();

	/// Sets altitude (ground mode only)
	void					setAltitude( TPosUnit z );

	/// Assignment operator
	IMovingEntity&			operator= ( const IMovingEntity& other )
	{
		_Id = other._Id;
		_Pos = other._Pos;
		_BodyHdg = other._BodyHdg;
		_RollAngle = other._RollAngle;
		_Vector = other._Vector;
		_AngVel = other._AngVel;
		_GroundMode = other._GroundMode;
		return *this;
	}

	/// Comparison operator
	friend bool				operator== ( const IMovingEntity& e1, const IMovingEntity& e2 )
	{
		return ( e1._Pos == e2._Pos // what about _Id and _GroundMode ?
			  && e1._BodyHdg == e2._BodyHdg
			  && e1._RollAngle == e2._RollAngle
			  && e1._Vector == e2._Vector
			  && e1._AngVel == e2._AngVel );
	}

	/** Serialization.
	 * Notes: the serialization is different whether ground mode is on or off.
	 * The receiver must know the ground mode by an external way.
	 * The body heading is never transmitted.
	 */
	void					serial ( NLMISC::IStream &s );

	/// Sets id from outside
	void					setId( TEntityId id )
	{
		_Id = id;
	}

	/// Returns a free identifier
	static TEntityId		getNewId()
	{
		TEntityId newid = _MaxId;
		_MaxId++;
		return newid;
	}

	/// Returns highest id
	static TEntityId		lastId()
	{
		return _MaxId;
	}

protected:

	/// Set position
	//@{
	void					setPos ( const NLMISC::CVector& p )				{ _Pos = p; }
	void					setPos ( TPosUnit x, TPosUnit y, TPosUnit z )	{ _Pos.set( x, y, z ); }
	//@}

	/// Set heading vector for moves and for body
	//@{
	void					setTrajVector ( const NLMISC::CVector& hdg )			{ _Vector = hdg; }
	void					setTrajVector ( TPosUnit x, TPosUnit y, TPosUnit z )	{ _Vector.set( x, y, z ); }
	void					setBodyHeading ( const NLMISC::CVector& hdg )			{ _BodyHdg = hdg; }
	void					setBodyHeading ( TPosUnit x, TPosUnit y, TPosUnit z )	{ _BodyHdg.set( x, y, z ); }
	void					setRollAngle ( TAngle rollangle )						{ _RollAngle = rollangle; }
	//@}

	/// Sets angular velocity
	void					setAngularVelocity ( TAngVelocity av )			{ _AngVel = av; }

	/// Computes position using heading and velocity
	void					computeNextPos()
	{
		_Pos += _Vector;
	}

	/// Computes position using heading and velocity
	void					computePosAfterDuration( TDuration d );

private:

// Properties:

	// Entity identifier
	TEntityId				_Id;

	/// Position
	NLMISC::CVector			_Pos;

	/// Body heading (always normalized)
	NLMISC::CVector			_BodyHdg;

	/// Roll angle
	TAngle					_RollAngle;

	/// Trajectory vector
	NLMISC::CVector			_Vector;

	/// Angular velocity
	TAngVelocity			_AngVel;

	/// If the entity follows the ground (then we transmit only 2 coordinates)
	bool					_GroundMode;

	// Highest Id
	static TEntityId		_MaxId;

};


} // NLNET


#endif // NL_MOVING_ENTITY_H

/* End of moving_entity.h */
