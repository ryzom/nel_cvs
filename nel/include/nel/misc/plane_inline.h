/* plane_inline.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: plane_inline.h,v 1.3 2000/10/06 10:27:36 lecroart Exp $
 *
 * class CPlane
 */

#ifndef NL_PLANE_INLINE_H
#define NL_PLANE_INLINE_H


namespace NLMISC
{


//============================================================
inline	CVector	CPlane::getNormal() const
{
	return CVector(a,b,c);
}
//============================================================
inline	float	CPlane::distance(const CVector &p) const
{
	CVector	v= getNormal().normed();
	return (float)fabs(v*p + d);
}
//============================================================
inline	float	CPlane::operator*(const CVector &p) const
{
	return a*p.x + b*p.y + c*p.z + d;
}
//============================================================
inline	CVector CPlane::intersect(const CVector &a,const CVector &b) const
{
	float decal;
	float	da= (*this)*a;
	float	db= (*this)*b;
	if(db-da ==0)
		return a;
	decal= ( 0-da ) / ( db - da );
	return a + (b-a)*decal;
}
//============================================================
inline	CVector CPlane::project(const CVector &p0) const
{
	return intersect(p0, p0+getNormal());
}


}


#endif // NL_PLANE_H

/* End of plane.h */
