/** \file primitive.cpp
 * <File description>
 *
 * $Id: primitive.cpp,v 1.1 2001/10/24 14:38:25 besson Exp $
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

#include "primitive.h"

using namespace NLMISC;
using namespace std;

namespace NLLIGO
{

// ***************************************************************************
void CPrimPoint::serial (IStream &f)
{
	f.xmlPushBegin ("POINT");

	f.xmlSetAttrib ("NAME");
	f.serial (Name);
	f.xmlSetAttrib ("LAYER");
	f.serial (LayerName);
	f.xmlPushEnd ();

	f.serial (Point);

	f.xmlPop ();
}

// ***************************************************************************
void CPrimPath::serial (IStream &f)
{
	f.xmlPushBegin ("PATH");
	
	f.xmlSetAttrib ("NAME");
	f.serial (Name);
	f.xmlSetAttrib ("LAYER");
	f.serial (LayerName);
	f.xmlPushEnd ();

	f.serialCont (VPoints);

	f.xmlPop ();
}

// ***************************************************************************
void CPrimZone::serial (IStream &f)
{
	f.xmlPushBegin ("ZONE");
	
	f.xmlSetAttrib ("NAME");
	f.serial (Name);
	f.xmlSetAttrib ("LAYER");
	f.serial (LayerName);
	f.xmlPushEnd ();

	f.serialCont (VPoints);

	f.xmlPop ();
}

// ***************************************************************************
void CPrimRegion::serial (IStream &f)
{
	f.xmlPushBegin ("REGION");
	
	f.xmlSetAttrib ("NAME");
	f.serial (Name);

	f.xmlPushEnd();

	sint version = 1;
	f.serialVersion (version);
	string check = "REGION";
	f.serialCheck (check);

	f.xmlPush ("POINTS");
		f.serialCont (VPoints);
	f.xmlPop ();
	f.xmlPush ("PATHES");
		f.serialCont (VPaths);
	f.xmlPop ();
	f.xmlPush ("ZONES");
		f.serialCont (VZones);
	f.xmlPop ();
}

} // namespace NLLIGO