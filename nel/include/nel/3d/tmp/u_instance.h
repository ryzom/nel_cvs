/** \file u_instance.h
 * <File description>
 *
 * $Id: u_instance.h,v 1.1 2001/02/28 16:19:35 berenguier Exp $
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

#ifndef NL_U_INSTANCE_H
#define NL_U_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_transform.h"


namespace NL3D
{


// ***************************************************************************
/**
 * Game interface for manipulating Objects, animations etc...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstance : virtual public UTransform
{
protected:

	/// \name Object
	// @{
	UInstance() {}
	virtual	~UInstance() {}
	// @}

public:


};


} // NL3D


#endif // NL_U_INSTANCE_H

/* End of u_instance.h */
