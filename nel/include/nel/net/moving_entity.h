/** \file moving_entity.h
 * Interface for all moving entities
 *
 * $Id: moving_entity.h,v 1.12 2000/12/19 16:06:09 cado Exp $
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
#include <string>


namespace NLNET {


/// Entity identifiers.
typedef uint32 TEntityId;

/// Entity types.
typedef uint32 TEntityType;

/// Position. Unit: meter
typedef float TPosUnit;

/// Duration. Unit: second
typedef float TDuration;

/// Velocity. Unit: TPosUnit per second
typedef TPosUnit TVelocity;

/// Angle. Unit: radian
typedef float TAngle;

/// Angular velocity. Unit: radian per second
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
	IMovingEntity( TEntityType t,
				   const NLMISC::CVector& pos,
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


	///@name Entity state properties
	//@{

	/// Returns identifier
	TEntityId				id() const			{ return _Id; }

	const std::string&		name() const		{ return _Name; }

	/// Returns entity type
	TEntityType				type() const		{ return _EntityType; }

	/// Returns position
	const NLMISC::CVector&	pos() const			{ return _Pos; }

	/// Returns heading
	const NLMISC::CVector&	bodyHeading() const	{ return _BodyHdg; }

	/// Returns roll angle
	const TAngle&			rollAngle() const	{ return _RollAngle; }

	/// Returns trajectory vector
	const NLMISC::CVector&	trajVector() const	{ return _Vector; }

	/// Returns angular velocity
	const TAngVelocity		angularVelocity() const	{ return _AngVel; }

	/// Returns true if the entity is in ground mode
	bool					groundMode() const	{ return _GroundMode; }
	
	//@}


	/// Angle around z axis from x axis (do not use for camera positionning, it won't work)
	TAngle					angleAroundZ();
	/// Angle around x axis from y axis (do not use for camera positionning, it won't work)
	TAngle					angleAroundX();
	/// Angle around y axis from z axis (do not use for camera positionning, it won't work)
	TAngle					angleAroundY();

	/// Returns the rotations of the body around the three axes (do not use for camera positionning, it won't work)
	NLMISC::CVector			bodyRot();

	/// Sets altitude (ground mode only)
	void					setAltitude( TPosUnit z );

	/// Assignment operator
	IMovingEntity&			operator= ( const IMovingEntity& other )
	{
		_Id = other._Id;
		_Name = other._Name;
		_EntityType = other._EntityType;
		_Pos = other._Pos;
		_BodyHdg = other._BodyHdg;
		_RollAngle = other._RollAngle;
		_Vector = other._Vector;
		_AngVel = other._AngVel;
		_GroundMode = other._GroundMode;
		_PrevPos = other._PrevPos;
		Tag = other.Tag;
		return *this;
	}

	/// Comparison operator
	friend bool				operator== ( const IMovingEntity& e1, const IMovingEntity& e2 )
	{
		// Note: maybe this could be replaced by a comparison of _Id ?
		return ( e1._EntityType == e2._EntityType
			  && e1._Pos == e2._Pos // what about _Id and _GroundMode ?
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

	/// Sets name
	void					setName( const std::string& name )
	{
		_Name = name;
	}

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

// protected:

	/// Set entity type
	void					setType( TEntityType t )						{ _EntityType = t; }

	///@name Set position
	//@{
	void					setPos ( const NLMISC::CVector& p )				{ _Pos = p; }
	void					setPos ( TPosUnit x, TPosUnit y, TPosUnit z )	{ _Pos.set( x, y, z ); }
	//@}

	///@name Set heading vector for moves and for body
	//@{
	void					setTrajVector ( const NLMISC::CVector& hdg )			{ _Vector = hdg; }
	void					setTrajVector ( TPosUnit x, TPosUnit y, TPosUnit z )	{ _Vector.set( x, y, z ); }
	void					setBodyHeading ( const NLMISC::CVector& hdg )			{ _BodyHdg = hdg; }
	void					setBodyHeading ( TPosUnit x, TPosUnit y, TPosUnit z )	{ _BodyHdg.set( x, y, z ); }
	void					setRollAngle ( TAngle rollangle )						{ _RollAngle = rollangle; }
	//@}

	/// Sets angular velocity
	void					setAngularVelocity ( TAngVelocity av )			{ _AngVel = av; }

	/// Returns the previous pos (that was valid before the latest update)
	const NLMISC::CVector&	previousPos() const								{ return _PrevPos; }

	/// Sets the previous pos
	void					setPreviousPos( const NLMISC::CVector& prevpos ) { _PrevPos = prevpos; }

	// Custom data
	uint32					Tag;
	//void*					Data;

protected:

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

	// Name
	std::string				_Name;
	
	// Type of entity
	TEntityType				_EntityType;

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

	/// Previous position (that was valid before the latest update)
	NLMISC::CVector			_PrevPos;
};


} // NLNET


#endif // NL_MOVING_ENTITY_H

/* End of moving_entity.h */
