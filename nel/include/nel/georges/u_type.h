/** \file u_type.h
 * Georges form loader interface
 *
 * $Id: u_type.h,v 1.1 2002/05/23 16:50:38 corvazier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_U_TYPE_H
#define NL_U_TYPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

namespace NLGEORGES 
{

/**
 * Georges type interface
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class UType : public NLMISC::CRefCount
{
public:

	// ** Type
	enum TType
	{
		UnsignedInt=0,
		SignedInt,
		Double,
		String,
		Color,
		TypeCount
	};

	/**
	  * Get the type
	  */
	virtual TType getType () const = 0;
};


} // NLGEORGES


#endif // NL_U_TYPE_H

/* End of u_type.h */
