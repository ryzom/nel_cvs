/** \file zone_template_vertex.cpp
 * Ligo zone template vertex implementation
 *
 * $Id: zone_template_vertex.cpp,v 1.1 2001/10/12 13:26:01 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "zone_template_vertex.h"

using namespace std;
using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************

CZoneTemplateVertex::CZoneTemplateVertex (const NLMISC::CVector& pos, uint edgeFlags)
{
	_Position = pos;
	_EdgeFlags = edgeFlags;
}

// ***************************************************************************

void CZoneTemplateVertex::serial (IStream &s)
{
	// Node for the Position
	s.xmlPush ("POSITION");

		// Serial the position
		s.serial (_Position);

	// Close the node
	s.xmlPop ();

	// Node for the Flags
	s.xmlPush ("FLAGS");

		// Serial the position
		s.serial (_EdgeFlags);

	// Close the node
	s.xmlPop ();
}

// ***************************************************************************

}