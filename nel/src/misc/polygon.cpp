/** \file polygon.cpp
 * <File description>
 *
 * $Id: polygon.cpp,v 1.2 2001/10/26 08:30:59 vizerie Exp $
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

#include "nel/misc/polygon.h"
#include "nel/misc/plane.h"
using namespace std;
using namespace NLMISC;


namespace NLMISC 
{


// ***************************************************************************
CPolygon::CPolygon(const CVector &a, const CVector &b, const CVector &c)
{
	Vertices.push_back(a);
	Vertices.push_back(b);
	Vertices.push_back(c);
}


// ***************************************************************************
void			CPolygon::clip(const CPlane	 *planes, uint nPlanes)
{
	if(nPlanes==0 || getNumVertices()==0)
		return;

	// The final polygon has at maximum currentVertices+number of clipping planes.
	// For performance, the vectors are static, so reallocation rarely occurs.
	static	vector<CVector>		tab0, tab1;
	tab0.resize(getNumVertices()+nPlanes);
	tab1.resize(getNumVertices()+nPlanes);
	// Init tab0 with Vertices.
	copy(Vertices.begin(), Vertices.end(), tab0.begin());
	CVector				*in=&(*tab0.begin()), *out= &(*tab1.begin());
	sint				nin= getNumVertices(), nout;
	for(sint i=0;i<(sint)nPlanes;i++)
	{
		nout= planes[i].clipPolygonBack(in, out, nin);
		swap(in, out);
		nin= nout;
		if(nin==0)
			break;
	}

	// Final result in "in".
	Vertices.resize(nin);
	if(nin>0)
	{
		memcpy(&(*Vertices.begin()), in, nin*sizeof(CVector));
	}
}


// ***************************************************************************
void			CPolygon::clip(const std::vector<CPlane> &planes)
{
	if(planes.size()==0)
		return;
	clip(&(*planes.begin()), planes.size());
}



// ***************************************************************************
void CPolygon::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	f.serialCont(Vertices);
}

} // NLMISC
