/** \file line.h
 * A simple couple of vertex.
 *
 * $Id: line.h,v 1.1 2001/01/18 16:12:53 berenguier Exp $
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

#ifndef NL_LINE_H
#define NL_LINE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NL3D 
{


using NLMISC::CVector;


// ***************************************************************************
/**
 * A simple couple of vertex.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLine
{
public:
	CVector		V0, V1;

public:
	/// Constructor
	CLine() {}

};


} // NL3D


#endif // NL_LINE_H

/* End of line.h */
