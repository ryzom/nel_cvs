/** \file bezier_patch.cpp
 * <File description>
 *
 * $Id: bezier_patch.cpp,v 1.1 2000/10/23 12:14:34 berenguier Exp $
 */

/** Copyright, 2000 Nevrax Ltd.
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
CVector		CBezierPatch::eval(float pu, float pv)
{
	CVector	p;

	float pu2 = pu * pu;
	float pu1 = 1.0f - pu;
	float pu12 = pu1 * pu1;
	float u0 = pu12 * pu1;
	float u1 = 3.0f * pu * pu12;
	float u2 = 3.0f * pu2 * pu1;
	float u3 = pu2 * pu;
	float pv2 = pv * pv;
	float pv1 = 1.0f - pv;
	float pv12 = pv1 * pv1;
	float v0 = pv12 * pv1;
	float v1 = 3.0f * pv * pv12;
	float v2 = 3.0f * pv2 * pv1;
	float v3 = pv2 * pv;

	p = Vertices[0]	* u0 * v0	+ 
		Tangents[7] * u1 * v0	+ 
		Tangents[6] * u2 * v0	+ 
		Vertices[3] * u3 * v0;
	p+= Tangents[0] * u0 * v1	+ 
		Interiors[0]* u1 * v1	+ 
		Interiors[3]* u2 * v1	+ 
		Tangents[5] * u3 * v1;
	p+=	Tangents[1] * u0 * v2	+ 
		Interiors[1]* u1 * v2	+ 
		Interiors[2]* u2 * v2	+ 
		Tangents[4] * u3 * v2;
	p+=	Vertices[1] * u0 * v3	+ 
		Tangents[2] * u1 * v3	+ 
		Tangents[3] * u2 * v3	+ 
		Vertices[2] * u3 * v3;
	
	return p;
}
// ***************************************************************************
CVectorD	CBezierPatch::evalDouble(double pu, double pv)
{
	CVectorD	p;

	double pu2 = pu * pu;
	double pu1 = 1.0f - pu;
	double pu12 = pu1 * pu1;
	double u0 = pu12 * pu1;
	double u1 = 3.0f * pu * pu12;
	double u2 = 3.0f * pu2 * pu1;
	double u3 = pu2 * pu;
	double pv2 = pv * pv;
	double pv1 = 1.0f - pv;
	double pv12 = pv1 * pv1;
	double v0 = pv12 * pv1;
	double v1 = 3.0f * pv * pv12;
	double v2 = 3.0f * pv2 * pv1;
	double v3 = pv2 * pv;

	p = CVectorD(Vertices[0]) * u0 * v0	+ 
		CVectorD(Tangents[7]) * u1 * v0	+ 
		CVectorD(Tangents[6]) * u2 * v0	+ 
		CVectorD(Vertices[3]) * u3 * v0;
	p+= CVectorD(Tangents[0]) * u0 * v1	+ 
		CVectorD(Interiors[0])* u1 * v1	+ 
		CVectorD(Interiors[3])* u2 * v1	+ 
		CVectorD(Tangents[5]) * u3 * v1;
	p+=	CVectorD(Tangents[1]) * u0 * v2	+ 
		CVectorD(Interiors[1])* u1 * v2	+ 
		CVectorD(Interiors[2])* u2 * v2	+ 
		CVectorD(Tangents[4]) * u3 * v2;
	p+=	CVectorD(Vertices[1]) * u0 * v3	+ 
		CVectorD(Tangents[2]) * u1 * v3	+ 
		CVectorD(Tangents[3]) * u2 * v3	+ 
		CVectorD(Vertices[2]) * u3 * v3;

	return p;
}


// ***************************************************************************
CVector		CBezierPatch::evalNormal(float pu, float pv)
{
	CVector	tgtS, tgtT;

	float u0,u1,u2,u3;
	float v0,v1,v2,v3;
	float pu2 = pu * pu;
	float pu1 = 1.0f - pu;
	float pu12 = pu1 * pu1;
	float pv2 = pv * pv;
	float pv1 = 1.0f - pv;
	float pv12 = pv1 * pv1;

	// Compute tangentS
	//=================
	// u/du.
	u0 = -3* pu12;
	u1 = 9*pu2 + 3 -12*pu;
	u2 =-9*pu2 + 6*pu ;
	u3 = 3* pu2;
	v0 = pv12 * pv1;
	v1 = 3.0f * pv * pv12;
	v2 = 3.0f * pv2 * pv1;
	v3 = pv2 * pv;

	tgtS =	Vertices[0]	* u0 * v0	+ 
			Tangents[7] * u1 * v0	+ 
			Tangents[6] * u2 * v0	+ 
			Vertices[3] * u3 * v0;
	tgtS+=	Tangents[0] * u0 * v1	+ 
			Interiors[0]* u1 * v1	+ 
			Interiors[3]* u2 * v1	+ 
			Tangents[5] * u3 * v1;
	tgtS+=	Tangents[1] * u0 * v2	+ 
			Interiors[1]* u1 * v2	+ 
			Interiors[2]* u2 * v2	+ 
			Tangents[4] * u3 * v2;
	tgtS+=	Vertices[1] * u0 * v3	+ 
			Tangents[2] * u1 * v3	+ 
			Tangents[3] * u2 * v3	+ 
			Vertices[2] * u3 * v3;
	
	// Compute tangentT
	//=================
	// u/du.
	u0 = pu12 * pu1;
	u1 = 3.0f * pu * pu12;
	u2 = 3.0f * pu2 * pu1;
	u3 = pu2 * pu;
	// v/dv.
	v0 = -3* pv12;
	v1 = 9*pv2 + 3 -12*pv;
	v2 =-9*pv2 + 6*pv ;
	v3 = 3* pv2;

	tgtT =	Vertices[0]	* u0 * v0	+ 
			Tangents[7] * u1 * v0	+ 
			Tangents[6] * u2 * v0	+ 
			Vertices[3] * u3 * v0;
	tgtT+=	Tangents[0] * u0 * v1	+ 
			Interiors[0]* u1 * v1	+ 
			Interiors[3]* u2 * v1	+ 
			Tangents[5] * u3 * v1;
	tgtT+=	Tangents[1] * u0 * v2	+ 
			Interiors[1]* u1 * v2	+ 
			Interiors[2]* u2 * v2	+ 
			Tangents[4] * u3 * v2;
	tgtT+=	Vertices[1] * u0 * v3	+ 
			Tangents[2] * u1 * v3	+ 
			Tangents[3] * u2 * v3	+ 
			Vertices[2] * u3 * v3;
	

	// Return the normal.
	return tgtS^tgtT;
}



} // NL3D
