/** \file mrm_parameters.h
 * <File description>
 *
 * $Id: mrm_parameters.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
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

#ifndef NL_MRM_PARAMETERS_H
#define NL_MRM_PARAMETERS_H

#include "nel/misc/types_nl.h"


namespace NL3D {


// ***************************************************************************
/**
 * This class is to be used with CMRMBuilder. It describe parameters of MRM build process.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMRMParameters
{
public:
	/// numbers of LODs wanted (11 by default).
	uint32			NLods;
	/// minimum faces wanted (a divisor of number of faces in baseMesh, 50 by default)
	uint32			Divisor;


	/// Constructor
	CMRMParameters()
	{
		NLods= 11;
		Divisor= 50;
	}

};


} // NL3D


#endif // NL_MRM_PARAMETERS_H

/* End of mrm_parameters.h */
