/* matrix.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: matrix.cpp,v 1.7 2000/09/21 12:40:18 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */


#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include <algorithm>
using namespace std;



namespace	NLMISC
{

// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// State Bits.
#define	MAT_TRANS		1
#define	MAT_ROT			2
#define	MAT_SCALEUNI	4
#define	MAT_SCALEANY	8
#define	MAT_PROJ		16
// Say if mat is valid.
#define	MAT_MVALID		32
// The identity is nothing but a valid matrix.
#define	MAT_IDENTITY	32


#define	CHECK_VALID() assert(StateBit & MAT_MVALID)


// Matrix elements.
#define	a11		M[0]
#define	a21		M[1]
#define	a31		M[2]
#define	a41		M[3]
#define	a12		M[4]
#define	a22		M[5]
#define	a32		M[6]
#define	a42		M[7]
#define	a13		M[8]
#define	a23		M[9]
#define	a33		M[10]
#define	a43		M[11]
#define	a14		M[12]
#define	a24		M[13]
#define	a34		M[14]
#define	a44		M[15]



// ======================================================================================================
// ======================================================================================================
// ======================================================================================================




// ======================================================================================================
CMatrix::CMatrix(const CMatrix &m)
{
	(*this)= m;
}
// ======================================================================================================
CMatrix		&CMatrix::operator=(const CMatrix &m)
{
	assert(m.StateBit & MAT_MVALID);

	StateBit= m.StateBit;
	Scale33= m.Scale33;
	memcpy(M, m.M, 16*sizeof(float));
	return *this;
}


// ======================================================================================================
void		CMatrix::identity()
{
	StateBit= MAT_IDENTITY;
	Scale33=1;
	a11=1; a12=0; a13=0; a14=0; 
	a21=0; a22=1; a23=0; a24=0; 
	a31=0; a32=0; a33=1; a34=0; 
	a41=0; a42=0; a43=0; a44=1; 
}
// ======================================================================================================
void		CMatrix::setRot(const CVector &i, const CVector &j, const CVector &k, bool hintNoScale)
{
	CHECK_VALID();
	StateBit|= MAT_ROT | MAT_SCALEANY;
	if(hintNoScale)
		StateBit&= ~(MAT_SCALEANY|MAT_SCALEUNI);
	a11= i.x; a12= j.x; a13= k.x; 
	a21= i.y; a22= j.y; a23= k.y; 
	a31= i.z; a32= j.z; a33= k.z; 
}
// ======================================================================================================
void		CMatrix::setRot(const float m33[9], bool hintNoScale)
{
	CHECK_VALID();
	StateBit|= MAT_ROT | MAT_SCALEANY;
	if(hintNoScale)
		StateBit&= ~(MAT_SCALEANY|MAT_SCALEUNI);
	a11= m33[0]; a12= m33[3]; a13= m33[6]; 
	a21= m33[1]; a22= m33[4]; a23= m33[7]; 
	a31= m33[2]; a32= m33[5]; a33= m33[8]; 
}
// ======================================================================================================
void		CMatrix::setPos(const CVector &v)
{
	CHECK_VALID();
	a14= v.x;
	a24= v.y;
	a34= v.z;
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		StateBit&= ~MAT_TRANS;
}
// ======================================================================================================
void		CMatrix::movePos(const CVector &v)
{
	CHECK_VALID();
	a14+= v.x;
	a24+= v.y;
	a34+= v.z;
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		StateBit&= ~MAT_TRANS;
}
// ======================================================================================================
void		CMatrix::setProj(const float proj[4])
{
	a41= proj[0];
	a42= proj[1];
	a43= proj[2];
	a44= proj[3];

	// Check Proj state.
	if(a41!=0 || a42!=0 || a43!=0 || a44!=1)
		StateBit|= MAT_PROJ;
	else
		StateBit&= ~MAT_PROJ;
}
// ======================================================================================================
void		CMatrix::resetProj()
{
	a41= 0;
	a42= 0;
	a43= 0;
	a44= 1;
	StateBit&= ~MAT_PROJ;
}
// ======================================================================================================
void		CMatrix::set(const float m44[16])
{
	// The matrix is now a valid matrix.
	StateBit= MAT_IDENTITY;

	StateBit|= MAT_ROT | MAT_SCALEANY;
	memcpy(M, m44, 16*sizeof(float));

	// Check Trans state.
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		StateBit&= ~MAT_TRANS;

	// Check Proj state.
	if(a41!=0 || a42!=0 || a43!=0 || a44!=1)
		StateBit|= MAT_PROJ;
	else
		StateBit&= ~MAT_PROJ;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::getRot(CVector &i, CVector &j, CVector &k) const
{
	CHECK_VALID();
	i.set(a11, a21, a31);
	j.set(a12, a22, a32);
	k.set(a13, a23, a33);
}
// ======================================================================================================
void		CMatrix::getRot(float m33[9]) const
{
	CHECK_VALID();
	m33[0]= a11;
	m33[1]= a21;
	m33[2]= a31;

	m33[3]= a12;
	m33[4]= a22;
	m33[5]= a32;

	m33[6]= a13;
	m33[7]= a23;
	m33[8]= a33;
}
// ======================================================================================================
void		CMatrix::getPos(CVector &v) const
{
	CHECK_VALID();
	v.set(a14, a24, a34);
}
// ======================================================================================================
CVector		CMatrix::getPos() const
{
	CHECK_VALID();
	return CVector(a14, a24, a34);
}
// ======================================================================================================
void		CMatrix::getProj(float proj[4]) const
{
	proj[0]= a41;
	proj[1]= a42;
	proj[2]= a43;
	proj[3]= a44;
}
// ======================================================================================================
CVector		CMatrix::getI() const
{
	CHECK_VALID();
	return CVector(a11, a21, a31);
}
// ======================================================================================================
CVector		CMatrix::getJ() const
{
	CHECK_VALID();
	return CVector(a12, a22, a32);
}
// ======================================================================================================
CVector		CMatrix::getK() const
{
	CHECK_VALID();
	return CVector(a13, a23, a33);
}
// ======================================================================================================
void		CMatrix::get(float m44[16]) const
{
	CHECK_VALID();
	memcpy(m44, M, 16*sizeof(float));
}
// ======================================================================================================
const float *CMatrix::get() const
{
	return M;
}
/*// ======================================================================================================
CVector		CMatrix::toEuler(TRotOrder ro) const
{
	CHECK_VALID();

}*/


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::translate(const CVector &v)
{
	CHECK_VALID();

	// SetTrans.
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		a14+= a11*v.x + a12*v.y + a13*v.z;
		a24+= a21*v.x + a22*v.y + a23*v.z;
		a34+= a31*v.x + a32*v.y + a33*v.z;
	}
	else
	{
		a14+= v.x;
		a24+= v.y;
		a34+= v.z;
	}

	// SetProj.
	if( StateBit & MAT_PROJ)
		a44+= a41*v.x + a42*v.y + a43*v.z;

	// Check Trans.
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
		StateBit&= ~MAT_TRANS;
}
// ======================================================================================================
void		CMatrix::rotateX(float a)
{
	CHECK_VALID();

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		float	b12=a12, b22=a22, b32=a32;
		float	b13=a13, b23=a23, b33=a33;
		a12= b12*ca + b13*sa;
		a22= b22*ca + b23*sa;
		a32= b32*ca + b33*sa;
		a13= b13*ca - b12*sa;
		a23= b23*ca - b22*sa;
		a33= b33*ca - b32*sa;
	}
	else
	{
		a12= 0; a22=ca; a32=sa;
		a13= 0; a23=-sa; a33=ca;
	}

	// SetProj.
	if( StateBit & MAT_PROJ)
	{
		float	b42=a42, b43=a43;
		a42= b42*ca + b43*sa;
		a43= b43*ca - b42*sa;
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotateY(float a)
{
	CHECK_VALID();

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		float	b11=a11, b21=a21, b31=a31;
		float	b13=a13, b23=a23, b33=a33;
		a11= b11*ca - b13*sa;
		a21= b21*ca - b23*sa;
		a31= b31*ca - b33*sa;
		a13= b13*ca + b11*sa;
		a23= b23*ca + b21*sa;
		a33= b33*ca + b31*sa;
	}
	else
	{
		a11= ca; a21=0; a31=-sa;
		a13= sa; a23=0; a33=ca;
	}

	// SetProj.
	if( StateBit & MAT_PROJ)
	{
		float	b41=a41, b43=a43;
		a41= b41*ca - b43*sa;
		a43= b43*ca + b41*sa;
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotateZ(float a)
{
	CHECK_VALID();

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		float	b11=a11, b21=a21, b31=a31;
		float	b12=a12, b22=a22, b32=a32;
		a11= b11*ca + b12*sa;
		a21= b21*ca + b22*sa;
		a31= b31*ca + b32*sa;
		a12= b12*ca - b11*sa;
		a22= b22*ca - b21*sa;
		a32= b32*ca - b31*sa;
	}
	else
	{
		a11= ca; a21=sa; a31=0;
		a12= -sa; a22=ca; a32=0;
	}

	// SetProj.
	if( StateBit & MAT_PROJ)
	{
		float	b41=a41, b42=a42;
		a41= b41*ca + b42*sa;
		a42= b42*ca - b41*sa;
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotate(const CVector &v, TRotOrder ro)
{
	CHECK_VALID();
	CMatrix		rot;
	rot.identity();
	switch(ro)
	{
		case XYZ: rot.rotateX(v.x); rot.rotateY(v.y); rot.rotateZ(v.z); break;
		case XZY: rot.rotateX(v.x); rot.rotateZ(v.z); rot.rotateY(v.y); break;
		case YXZ: rot.rotateY(v.y); rot.rotateX(v.x); rot.rotateZ(v.z); break;
		case YZX: rot.rotateY(v.y); rot.rotateZ(v.z); rot.rotateX(v.x); break;
		case ZXY: rot.rotateZ(v.z); rot.rotateX(v.x); rot.rotateY(v.y); break;
		case ZYX: rot.rotateZ(v.z); rot.rotateY(v.y); rot.rotateX(v.x); break;
	}

	(*this)*= rot;
}
// ======================================================================================================
void		CMatrix::scale(float f)
{
	CHECK_VALID();

	if(f==1.0f) return;
	if(StateBit & MAT_SCALEANY)
	{
		scale(CVector(f,f,f));
	}
	else
	{
		StateBit|= MAT_SCALEUNI;
		Scale33*=f;
		a11*= f; a12*=f; a13*=f;
		a21*= f; a22*=f; a23*=f;
		a31*= f; a32*=f; a33*=f;

		// SetProj.
		if( StateBit & MAT_PROJ)
		{
			a41*=f; a42*=f; a43*=f;
		}
	}
}
// ======================================================================================================
void		CMatrix::scale(const CVector &v)
{
	CHECK_VALID();

	if( v==CVector(1,1,1) ) return;
	if( !(StateBit & MAT_SCALEANY) && v.x==v.y && v.x==v.z)
	{
		scale(v.x);
	}
	else
	{
		StateBit|=MAT_SCALEANY;
		a11*= v.x; a12*=v.y; a13*=v.z;
		a21*= v.x; a22*=v.y; a23*=v.z;
		a31*= v.x; a32*=v.y; a33*=v.z;

		// SetProj.
		if( StateBit & MAT_PROJ)
		{
			a41*=v.x;
			a42*=v.y;
			a43*=v.z;
		}
	}
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
CMatrix		CMatrix::operator*(const CMatrix &m) const
{
	CHECK_VALID();
	assert(m.StateBit & MAT_MVALID);

	CMatrix		ret;
	// Do ret= M1*M2, where M1=*this and M2=m.
	ret.StateBit= StateBit | m.StateBit;


	// Build Rot part.
	//===============
	bool	M1Identity= ! (StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY));
	bool	M2Identity= ! (m.StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY));
	bool	M1ScaleOnly= ! (StateBit & MAT_ROT);
	bool	M2ScaleOnly= ! (m.StateBit & MAT_ROT);

	// If one or all of the 3x3 matrix is an identity, just do a copy
	if( M1Identity || M2Identity )
	{
		// Copy the non identity matrix (if possible).
		const CMatrix	*c= M2Identity? this : &m;
		ret.a11= c->a11; ret.a12= c->a12; ret.a13= c->a13;
		ret.a21= c->a21; ret.a22= c->a22; ret.a23= c->a23;
		ret.a31= c->a31; ret.a32= c->a32; ret.a33= c->a33;
	}
	// If two 3x3 matrix are just scaleOnly matrix, do a scaleFact.
	else if( M1ScaleOnly && M2ScaleOnly )
	{
		// same process for scaleUni or scaleAny.
		ret.a11= a11*m.a11; ret.a12= 0; ret.a13= 0; 
		ret.a21= 0; ret.a22= a22*m.a22; ret.a23= 0; 
		ret.a31= 0; ret.a32= 0; ret.a33= a33*m.a33;
	}
	// If one of the matrix is a scaleOnly matrix, do a scale*Rot.
	else if( M1ScaleOnly && !M2ScaleOnly )
	{
		ret.a11= a11*m.a11; ret.a12= a11*m.a12; ret.a13= a11*m.a13;
		ret.a21= a22*m.a21; ret.a22= a22*m.a22; ret.a23= a22*m.a23;
		ret.a31= a33*m.a31; ret.a32= a33*m.a32; ret.a33= a33*m.a33;
	}
	else if( !M1ScaleOnly && M2ScaleOnly )
	{
		ret.a11= a11*m.a11; ret.a12= a12*m.a22; ret.a13= a13*m.a33;
		ret.a21= a21*m.a11; ret.a22= a22*m.a22; ret.a23= a23*m.a33;
		ret.a31= a31*m.a11; ret.a32= a32*m.a22; ret.a33= a33*m.a33;
	}
	// Else, general case: blending of two rotations.
	else
	{
		ret.a11= a11*m.a11 + a12*m.a21 + a13*m.a31;
		ret.a12= a11*m.a12 + a12*m.a22 + a13*m.a32;
		ret.a13= a11*m.a13 + a12*m.a23 + a13*m.a33;

		ret.a21= a21*m.a11 + a22*m.a21 + a23*m.a31;
		ret.a22= a21*m.a12 + a22*m.a22 + a23*m.a32;
		ret.a23= a21*m.a13 + a22*m.a23 + a23*m.a33;

		ret.a31= a31*m.a11 + a32*m.a21 + a33*m.a31;
		ret.a32= a31*m.a12 + a32*m.a22 + a33*m.a32;
		ret.a33= a31*m.a13 + a32*m.a23 + a33*m.a33;
	}

	// If M1 has translate and M2 has projective, rotation is modified.
	if( (StateBit & MAT_TRANS) && (m.a41!=0 || m.a42!=0 || m.a43!=0))
	{
		ret.StateBit|= MAT_ROT|MAT_SCALEANY;

		ret.a11+= a14*m.a41;
		ret.a12+= a14*m.a42;
		ret.a13+= a14*m.a43;

		ret.a21+= a24*m.a41;
		ret.a22+= a24*m.a42;
		ret.a23+= a24*m.a43;

		ret.a31+= a34*m.a41;
		ret.a32+= a34*m.a42;
		ret.a33+= a34*m.a43;
	}
	// Modify Scale.
	if( (ret.StateBit & MAT_SCALEUNI) && !(ret.StateBit & MAT_SCALEANY) )
		ret.Scale33= Scale33*m.Scale33;
	else
		ret.Scale33=1;


	// Build Trans part.
	//=================
	if( ret.StateBit & MAT_TRANS )
	{
		// Compose M2 part.
		if( M1Identity )
		{
			ret.a14= m.a14;
			ret.a24= m.a24;
			ret.a34= m.a34;
		}
		else if (M1ScaleOnly )
		{
			ret.a14= a11*m.a14;
			ret.a24= a22*m.a24;
			ret.a34= a33*m.a34;
		}
		else
		{
			ret.a14= a11*m.a14 + a12*m.a24 + a13*m.a34;
			ret.a24= a21*m.a14 + a22*m.a24 + a23*m.a34;
			ret.a34= a31*m.a14 + a32*m.a24 + a33*m.a34;
		}
		// Compose M1 part.
		if(StateBit & MAT_TRANS)
		{
			if(m.StateBit & MAT_PROJ)
			{
				ret.a14+= a14*m.a44;
				ret.a24+= a24*m.a44;
				ret.a34+= a34*m.a44;
			}
			else
			{
				ret.a14+= a14;
				ret.a24+= a24;
				ret.a34+= a34;
			}
		}
	}
	else
	{
		ret.a14= 0;
		ret.a24= 0;
		ret.a34= 0;
	}


	// Build Proj part.
	//=================
	if( ret.StateBit & MAT_PROJ )
	{
		// optimise nothing... (projection matrix are rare).
		ret.a41= a41*m.a11 + a42*m.a21 + a43*m.a31 + a44*m.a41;
		ret.a42= a41*m.a12 + a42*m.a22 + a43*m.a32 + a44*m.a42;
		ret.a43= a41*m.a13 + a42*m.a23 + a43*m.a33 + a44*m.a43;
		ret.a44= a41*m.a14 + a42*m.a24 + a43*m.a34 + a44*m.a44;
	}
	else
	{
		ret.a41= 0;
		ret.a42= 0;
		ret.a43= 0;
		ret.a44= 1;
	}


	return ret;
}
// ======================================================================================================
CMatrix		&CMatrix::operator*=(const CMatrix &m)
{
	CHECK_VALID();

	*this= *this*m;

	return *this;
}
// ======================================================================================================
void		CMatrix::invert()
{
	CHECK_VALID();

	*this= inverted();
}

// ======================================================================================================
void	CMatrix::fastInvert33(CMatrix &ret) const
{
	// Fast invert of 3x3 rot matrix.
	// Work if no scale and if MAT_SCALEUNI. doesn't work if MAT_SCALEANY.

	if(StateBit & MAT_SCALEUNI)
	{
		double	s,S;	// important for precision.
		// Must divide the matrix by 1/Scale 2 times, to set unit, and to have a Scale=1/Scale.
		S=1.0/Scale33;
		ret.Scale33=S;
		s=S*S;
		// The matrix is a base, so just transpose it.
		ret.a11= a11*s; ret.a12= a21*s; ret.a13=a31*s;
		ret.a21= a12*s; ret.a22= a22*s; ret.a23=a32*s;
		ret.a31= a13*s; ret.a32= a23*s; ret.a33=a33*s;
	}
	else
	{
		ret.Scale33=1;
		// The matrix is a base, so just transpose it.
		ret.a11= a11; ret.a12= a21; ret.a13=a31;
		ret.a21= a12; ret.a22= a22; ret.a23=a32;
		ret.a31= a13; ret.a32= a23; ret.a33=a33;
	}

	// 15 cycles if no scale.
	// 35 cycles if scale.
}
// ======================================================================================================
bool	CMatrix::slowInvert33(CMatrix &ret) const
{
	CVector	invi,invj,invk;
	CVector	i,j,k;
	double	s;

	i= getI();
	j= getJ();
	k= getK();
	// Compute cofactors (minors *(-1)^(i+j)).
	invi.x= j.y*k.z - k.y*j.z;
	invi.y= j.z*k.x - k.z*j.x;
	invi.z= j.x*k.y - k.x*j.y;
	invj.x= k.y*i.z - i.y*k.z;
	invj.y= k.z*i.x - i.z*k.x;
	invj.z= k.x*i.y - i.x*k.y;
	invk.x= i.y*j.z - j.y*i.z;
	invk.y= i.z*j.x - j.z*i.x;
	invk.z= i.x*j.y - j.x*i.y;
	// compute determinant.
	s= invi.x*i.x + invj.x*j.x + invk.x*k.x;
	if(s==0)
		return false;
	// Transpose the Comatrice, and divide by determinant.
	s=1.0/s;
	ret.a11= invi.x*s; ret.a12= invi.y*s; ret.a13= invi.z*s;
	ret.a21= invj.x*s; ret.a22= invj.y*s; ret.a23= invj.z*s;
	ret.a31= invk.x*s; ret.a32= invk.y*s; ret.a33= invk.z*s;

	return true;
	// Roundly 82 cycles. (1Div=10 cycles).
}
// ======================================================================================================
bool	CMatrix::slowInvert44(CMatrix &ret) const
{
	sint	i,j;
	double	s;

	// Compute Cofactors
	//==================
	for(i=0;i<=3;i++)
	{
		for(j=0;j<=3;j++)
		{
			sint	l1,l2,l3;
			sint	c1,c2,c3;
			getCofactIndex(i,l1,l2,l3);
			getCofactIndex(j,c1,c2,c3);

			ret.mat(i,j)= 0;
			ret.mat(i,j)+= mat(l1,c1) * mat(l2,c2) * mat(l3,c3);
			ret.mat(i,j)+= mat(l1,c2) * mat(l2,c3) * mat(l3,c1);
			ret.mat(i,j)+= mat(l1,c3) * mat(l2,c1) * mat(l3,c2);

			ret.mat(i,j)-= mat(l1,c1) * mat(l2,c3) * mat(l3,c2);
			ret.mat(i,j)-= mat(l1,c2) * mat(l2,c1) * mat(l3,c3);
			ret.mat(i,j)-= mat(l1,c3) * mat(l2,c2) * mat(l3,c1);

			if( (i+j)&1 )
				ret.mat(i,j)=-ret.mat(i,j);
		}
	}

	// Compute determinant.
	//=====================
	s= ret.mat(0,0) * mat(0,0) + ret.mat(0,1) * mat(0,1) + ret.mat(0,2) * mat(0,2) + ret.mat(0,3) * mat(0,3);
	if(s==0)
		return false;

	// Divide by determinant.
	//=======================
	s=1.0/s;
	for(i=0;i<=3;i++)
	{
		for(j=0;j<=3;j++)
			ret.mat(i,j)*= s;
	}

	// Transpose the comatrice.
	//=========================
	for(i=0;i<=3;i++)
	{
		for(j=i+1;j<=3;j++)
		{
			swap(ret.mat(i,j), ret.mat(j,i));
		}
	}

	return true;
}
// ======================================================================================================
CMatrix		CMatrix::inverted() const
{
	CHECK_VALID();

	CMatrix	ret;

	// Do a conventionnal 44 inversion.
	//=================================
	if(StateBit & MAT_PROJ)
	{
		if(!slowInvert44(ret))
		{
			ret.identity();
			return ret;
		}

		// Well, don't know what happens to matrix, so set all StateBit :).
		ret.StateBit= MAT_TRANS|MAT_ROT|MAT_SCALEANY|MAT_PROJ|MAT_MVALID;

		// Check Trans state.
		if(ret.a14!=0 || ret.a24!=0 || ret.a34!=0)
			ret.StateBit|= MAT_TRANS;
		else
			ret.StateBit&= ~MAT_TRANS;

		// Check Proj state.
		if(ret.a41!=0 || ret.a42!=0 || ret.a43!=0 || ret.a44!=1)
			ret.StateBit|= MAT_PROJ;
		else
			ret.StateBit&= ~MAT_PROJ;
	}

	// Do a speed 34 inversion.
	//=========================
	else
	{
		// Invert the rotation part.
		if(StateBit & MAT_SCALEANY)
		{
			if(!slowInvert33(ret))
			{
				ret.identity();
				return ret;
			}
		}
		else
			fastInvert33(ret);
		// Scale33 is updated in fastInvert33().

		// Invert the translation part.
		if(StateBit & MAT_TRANS)
		{
			// Invert the translation part.
			// This can only work if 4th line is 0 0 0 1.
			// formula: InvVp= InvVi*(-Vp.x) + InvVj*(-Vp.y) + InvVk*(-Vp.z)
			ret.a14= ret.a11*(-a14) + ret.a12*(-a24) + ret.a13*(-a34);
			ret.a24= ret.a21*(-a14) + ret.a22*(-a24) + ret.a23*(-a34);
			ret.a34= ret.a31*(-a14) + ret.a32*(-a24) + ret.a33*(-a34);
		}
		else
		{
			ret.a14= 0;
			ret.a24= 0;
			ret.a34= 0;
		}

		// The projection part is unmodified.
		ret.a41= 0; ret.a42= 0; ret.a43= 0; ret.a44= 1;

		// The matrix inverted keep the same state bits.
		ret.StateBit= StateBit;
	}
	
	
	return ret;
}
// ======================================================================================================
bool		CMatrix::normalize(TRotOrder ro)
{
	CHECK_VALID();

	if( (StateBit & (MAT_SCALEUNI|MAT_SCALEANY))==0 )
		return true;	// Matrix is already an orthonormal basis.

	CVector	ti,tj,tk;
	ti= getI();
	tj= getJ();
	tk= getK();

	// Normalize with help of ro
	CVector	*t0, *t1, *t2;
	switch(ro)
	{
		case XYZ: t2=&ti; t1=&tj; t0=&tk; break;
		case XZY: t2=&ti; t1=&tk; t0=&tj; break;
		case YXZ: t2=&tj; t1=&ti; t0=&tk; break;
		case YZX: t2=&tj; t1=&tk; t0=&ti; break;
		case ZXY: t2=&tk; t1=&ti; t0=&tj; break;
		case ZYX: t2=&tk; t1=&tj; t0=&ti; break;
	}
	t2->normalize();
	*t0= *t1^*t2;
	t0->normalize();
	*t1= *t2^*t0;

	// Check, and set result.
	if( ti.isNull() || tj.isNull() || tk.isNull() )
		return false;
	a11= ti.x; a12= tj.x; a13= tk.x; 
	a21= ti.y; a22= tj.y; a23= tk.y; 
	a31= ti.z; a32= tj.z; a33= tk.z; 
	// Scale is reseted.
	StateBit&= ~(MAT_SCALEUNI|MAT_SCALEANY);
	Scale33=1;

	return true;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
CVector		CMatrix::mulVector(const CVector &v) const
{
	CHECK_VALID();

	CVector	ret;

	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		ret.x= a11*v.x + a12*v.y + a13*v.z;
		ret.y= a21*v.x + a22*v.y + a23*v.z;
		ret.z= a31*v.x + a32*v.y + a33*v.z;
		return ret;
	}
	else
		return v;
}
// ======================================================================================================
CVector		CMatrix::mulPoint(const CVector &v) const
{
	CHECK_VALID();

	CVector	ret;

	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		// Compose with translation too.
		ret.x= a11*v.x + a12*v.y + a13*v.z + a14;
		ret.y= a21*v.x + a22*v.y + a23*v.z + a24;
		ret.z= a31*v.x + a32*v.y + a33*v.z + a34;
		return ret;
	}
	else if( StateBit & MAT_TRANS )
	{
		ret.x= v.x + a14;
		ret.y= v.y + a24;
		ret.z= v.z + a34;
		return ret;
	}
	else	// Identity!!
		return v;
}


// ======================================================================================================
CPlane		operator*(const CPlane &p, const CMatrix &m)
{
	assert(m.StateBit & MAT_MVALID);

	CPlane	ret;

	if( m.StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY|MAT_PROJ) )
	{
		// Compose with translation too.
		ret.a= p.a*m.a11 + p.b*m.a21 + p.c*m.a31 + p.d*m.a41;
		ret.b= p.a*m.a12 + p.b*m.a22 + p.c*m.a32 + p.d*m.a42;
		ret.c= p.a*m.a13 + p.b*m.a23 + p.c*m.a33 + p.d*m.a43;
		ret.d= p.a*m.a14 + p.b*m.a24 + p.c*m.a34 + p.d*m.a44;
		return ret;
	}
	else if( m.StateBit & MAT_TRANS )
	{

		// Compose just with a translation.
		ret.a= p.a;
		ret.b= p.b;
		ret.c= p.c;
		ret.d= p.a*m.a14 + p.b*m.a24 + p.c*m.a34 + p.d*m.a44;
		return ret;
	}
	else	// Identity!!
		return p;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::serial(IStream &f)
{
	if(!f.isReading())
		CHECK_VALID();

	// Use versionning, maybe for futur improvement.
	sint	ver= f.serialVersion(0);

	if(f.isReading())
		identity();
	f.serial(StateBit);
	f.serial(Scale33);
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		f.serial(a11, a12, a13);
		f.serial(a21, a22, a23);
		f.serial(a31, a32, a33);
	}
	if( StateBit & (MAT_TRANS) )
	{
		f.serial(a14, a24, a34);
	}
	if( StateBit & (MAT_PROJ) )
	{
		f.serial(a41, a42, a43, a44);
	}
}




}