/** \file u_instance_group.h
 * Game interface for managing group instance.
 *
 * $Id: u_instance_group.h,v 1.1 2001/04/09 14:27:58 corvazier Exp $
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

#ifndef NL_U_INSTANCE_GROUP_H
#define NL_U_INSTANCE_GROUP_H

#include "nel/misc/types_nl.h"

namespace NLMISC
{
	class CVector;
	class CQuat;
}

namespace NL3D 
{

/**
 * Game interface for managing group instance.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UInstanceGroup
{
public:

	/**
	  * Return the count of shape instance in this group.
	  */
	virtual uint getNumInstance () const=0;

	/**
	  * Return the name of an instance of the group.
	  *
	  * \param instanceNb is the number of the instance.
	  * \return the name of the instance.
	  */
	virtual const std::string& getInstanceName (uint instanceNb) const=0;

	/**
	  * Return the position of an instance of the group.
	  *
	  * \param instanceNb is the number of the instance.
	  * \return the position of the instance.
	  */
	virtual const NLMISC::CVector& getInstancePos (uint instanceNb) const=0;

	/**
	  * Return the rotation of an instance of the group.
	  *
	  * \param instanceNb is the number of the instance.
	  * \return the rotation of the instance.
	  */
	virtual const NLMISC::CQuat& getInstanceRot (uint instanceNb) const=0;

	/**
	  * Return the scale of an instance of the group.
	  *
	  * \param instanceNb is the number of the instance.
	  * \return the scale of the instance.
	  */
	virtual const NLMISC::CVector& getInstanceScale (uint instanceNb) const=0;
};


} // NL3D


#endif // NL_U_INSTANCE_GROUP_H

/* End of u_instance_group.h */
