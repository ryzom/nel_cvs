/** \file bezier_patch.cpp
 * <File description>
 *
 * $Id: bezier_patch.cpp,v 1.3 2000/11/02 13:48:50 berenguier Exp $
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

#include "nel/3d/bezier_patch.h"
using namespace NLMISC;


namespace NL3D {


// ***************************************************************************
void		CBezierPatch::make(CVector vertices[4], CVector	normals[4])
{
	sint	i;
	static	sint	starts[8]= {0,1, 1,2, 2,3, 3,0};
	static	sint	ends[8]= {1,0, 2,1, 3,2, 0,3};

	for(i=0;i<4;i++)
		Vertices[i]= vertices[i];

	// For all tangents.
	for(i=0;i<8;i++)
	{
		CVector		tgt= Vertices[ends[i]] - Vertices[starts[i]];
		CVector		I,J,K;
		J= normals[starts[i]];
		I= J^tgt;
		K= I^J;
		K.normalize();
		// Yes, we store tangents as position, not vectors...
		Tangents[i]= Vertices[starts[i]] + K*tgt.norm()/3;
	}

	makeInteriors();
}
// ***************************************************************************
void		CBezierPatch::makeInteriors()
{
	// Lam-ed from Max sdk.
	CVector		&a = Vertices[0];
	CVector		&b = Vertices[1];
	CVector		&c = Vertices[2];
	CVector		&d = Vertices[3];
	Interiors[0] = Tangents[7] + Tangents[0] - a;
	Interiors[1] = Tangents[1] + Tangents[2] - b;
	Interiors[2] = Tangents[3] + Tangents[4] - c;
	Interiors[3] = Tangents[5] + Tangents[6] - d;
}

// ***************************************************************************
CVector		CBezierPatch::eval(float ps, float pt)
{
	CVector	p;

	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float s0 = ps12 * ps1;
	float s1 = 3.0f * ps * ps12;
	float s2 = 3.0f * ps2 * ps1;
	float s3 = ps2 * ps;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;
	float t0 = pt12 * pt1;
	float t1 = 3.0f * pt * pt12;
	float t2 = 3.0f * pt2 * pt1;
	float t3 = pt2 * pt;

	p = Vertices[0]	* s0 * t0	+ 
		Tangents[7] * s1 * t0	+ 
		Tangents[6] * s2 * t0	+ 
		Vertices[3] * s3 * t0;
	p+= Tangents[0] * s0 * t1	+ 
		Interiors[0]* s1 * t1	+ 
		Interiors[3]* s2 * t1	+ 
		Tangents[5] * s3 * t1;
	p+=	Tangents[1] * s0 * t2	+ 
		Interiors[1]* s1 * t2	+ 
		Interiors[2]* s2 * t2	+ 
		Tangents[4] * s3 * t2;
	p+=	Vertices[1] * s0 * t3	+ 
		Tangents[2] * s1 * t3	+ 
		Tangents[3] * s2 * t3	+ 
		Vertices[2] * s3 * t3;
	
	return p;
}
// ***************************************************************************
CVectorD	CBezierPatch::evalDouble(double ps, double pt)
{
	CVectorD	p;

	double ps2 = ps * ps;
	double ps1 = 1.0f - ps;
	double ps12 = ps1 * ps1;
	double s0 = ps12 * ps1;
	double s1 = 3.0f * ps * ps12;
	double s2 = 3.0f * ps2 * ps1;
	double s3 = ps2 * ps;
	double pt2 = pt * pt;
	double pt1 = 1.0f - pt;
	double pt12 = pt1 * pt1;
	double t0 = pt12 * pt1;
	double t1 = 3.0f * pt * pt12;
	double t2 = 3.0f * pt2 * pt1;
	double t3 = pt2 * pt;

	p = CVectorD(Vertices[0]) * s0 * t0	+ 
		CVectorD(Tangents[7]) * s1 * t0	+ 
		CVectorD(Tangents[6]) * s2 * t0	+ 
		CVectorD(Vertices[3]) * s3 * t0;
	p+= CVectorD(Tangents[0]) * s0 * t1	+ 
		CVectorD(Interiors[0])* s1 * t1	+ 
		CVectorD(Interiors[3])* s2 * t1	+ 
		CVectorD(Tangents[5]) * s3 * t1;
	p+=	CVectorD(Tangents[1]) * s0 * t2	+ 
		CVectorD(Interiors[1])* s1 * t2	+ 
		CVectorD(Interiors[2])* s2 * t2	+ 
		CVectorD(Tangents[4]) * s3 * t2;
	p+=	CVectorD(Vertices[1]) * s0 * t3	+ 
		CVectorD(Tangents[2]) * s1 * t3	+ 
		CVectorD(Tangents[3]) * s2 * t3	+ 
		CVectorD(Vertices[2]) * s3 * t3;

	return p;
}


// ***************************************************************************
CVector		CBezierPatch::evalNormal(float ps, float pt)
{
	CVector	tgtS, tgtT;

	float s0,s1,s2,s3;
	float t0,t1,t2,t3;
	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;

	// Compute tangentS
	//=================
	// s/ds.
	s0 = -3* ps12;
	s1 = 9*ps2 + 3 -12*ps;
	s2 =-9*ps2 + 6*ps ;
	s3 = 3* ps2;
	// t/dt.
	t0 = pt12 * pt1;
	t1 = 3.0f * pt * pt12;
	t2 = 3.0f * pt2 * pt1;
	t3 = pt2 * pt;

	tgtS =	Vertices[0]	* s0 * t0	+ 
			Tangents[7] * s1 * t0	+ 
			Tangents[6] * s2 * t0	+ 
			Vertices[3] * s3 * t0;
	tgtS+=	Tangents[0] * s0 * t1	+ 
			Interiors[0]* s1 * t1	+ 
			Interiors[3]* s2 * t1	+ 
			Tangents[5] * s3 * t1;
	tgtS+=	Tangents[1] * s0 * t2	+ 
			Interiors[1]* s1 * t2	+ 
			Interiors[2]* s2 * t2	+ 
			Tangents[4] * s3 * t2;
	tgtS+=	Vertices[1] * s0 * t3	+ 
			Tangents[2] * s1 * t3	+ 
			Tangents[3] * s2 * t3	+ 
			Vertices[2] * s3 * t3;
	
	// Compute tangentT
	//=================
	// s/ds.
	s0 = ps12 * ps1;
	s1 = 3.0f * ps * ps12;
	s2 = 3.0f * ps2 * ps1;
	s3 = ps2 * ps;
	// t/dt.
	t0 = -3* pt12;
	t1 = 9*pt2 + 3 -12*pt;
	t2 =-9*pt2 + 6*pt ;
	t3 = 3* pt2;

	tgtT =	Vertices[0]	* s0 * t0	+ 
			Tangents[7] * s1 * t0	+ 
			Tangents[6] * s2 * t0	+ 
			Vertices[3] * s3 * t0;
	tgtT+=	Tangents[0] * s0 * t1	+ 
			Interiors[0]* s1 * t1	+ 
			Interiors[3]* s2 * t1	+ 
			Tangents[5] * s3 * t1;
	tgtT+=	Tangents[1] * s0 * t2	+ 
			Interiors[1]* s1 * t2	+ 
			Interiors[2]* s2 * t2	+ 
			Tangents[4] * s3 * t2;
	tgtT+=	Vertices[1] * s0 * t3	+ 
			Tangents[2] * s1 * t3	+ 
			Tangents[3] * s2 * t3	+ 
			Vertices[2] * s3 * t3;
	

	// Return the normal.
	return tgtS^tgtT;
}



} // NL3D
