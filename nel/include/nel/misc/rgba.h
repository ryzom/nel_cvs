/** \file rgba.h
 * ARGB pixel format
 *
 * $Id: rgba.h,v 1.6 2000/10/25 09:06:51 corvazier Exp $
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

#ifndef NL_RGBA_H
#define NL_RGBA_H

#include "nel/misc/types_nl.h"


namespace NLMISC 
{

class	IStream;

/**
 * Class pixel RGBA
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CRGBA
{
public:

	/// Default constructor. do nothing
	CRGBA() {};

	/** 
	 * Constructor.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	CRGBA(uint8 r, uint8 g, uint8 b, uint8 a=255) :
		R(r), G(g), B(b), A(a) {}

	/** 
	 * Return a packed pixel
	 */
	uint	getPacked() const {return ((uint)R<<24) + ((uint)G<<16) + ((uint)B<<8) + A;}

	/** 
	 * Comparison operator.
	 */
	bool	operator<(const CRGBA &c) const {return getPacked()<c.getPacked();}

	/** 
	 * Equality operator.
	 */
	bool	operator==(const CRGBA &c) const  
		{return R==c.R && G==c.G && B==c.B && A==c.A;}

	/** 
	 * Serialisation.
	 * \param f Stream used for serialisation.
	 */
	void    serial(class NLMISC::IStream &f);

	/** 
	 * Blend two colors.
	 * \param c0 Color 0.
	 * \param c1 Color 1.
	 * \param factor Blend factor. 0~255. 0 return c0 and 255 return c1.
	 */
	void blendFromui(CRGBA &c0, CRGBA &c1, uint factor); 

	/** 
	 * Set colors.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	void set(uint8 r, uint8 g, uint8 b, uint8 a);

	/// Red componant.
	uint8	R;
	/// Green componant.
	uint8	G;
	/// Blue componant.
	uint8	B;
	/// Alpha componant.
	uint8	A;
};


/**
 * Class pixel BGRA, Windows style pixel.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CBGRA
{
public:

	/// Default constructor. do nothing
	CBGRA() {};

	/**
	 * Constructor from a CRGBA
	 * \param c CRGBA color.
	 */
	CBGRA(const CRGBA& c) 
	{
		R=c.R;
		G=c.G;
		B=c.B;
		A=c.A;
	};

	/** 
	 * Constructor.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	CBGRA(uint8 r, uint8 g, uint8 b, uint8 a=255) :
		R(r), G(g), B(b), A(a) {}

	/**
	 * Cast operator to CRGBA.
	 */
	operator CRGBA()
	{
		return CRGBA (R, G, B, A);
	}

	/** 
	 * Return a packed pixel
	 */
	uint	getPacked() const 
	{
		return ((uint)R<<24) + ((uint)G<<16) + ((uint)B<<8) + A;
	}

	/** 
	 * Comparison operator.
	 */
	bool	operator<(const CBGRA &c) const 
	{
		return getPacked()<c.getPacked();
	}

	/** 
	 * Equality operator.
	 */
	bool	operator==(const CBGRA &c) const  
	{
		return R==c.R && G==c.G && B==c.B && A==c.A;
	}

	/** 
	 * Serialisation.
	 * \param f Stream used for serialisation.
	 */
	void    serial(class NLMISC::IStream &f);

	/** 
	 * Blend two colors.
	 * \param c0 Color 0.
	 * \param c1 Color 1.
	 * \param factor Blend factor. 0~255. 0 return c0 and 255 return c1.
	 */
	void blendFromui(CBGRA &c0, CBGRA &c1, uint factor); 

	/** 
	 * Set colors.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	void set(uint8 r, uint8 g, uint8 b, uint8 a);

	/// Blue componant.
	uint8	B;
	/// Green componant.
	uint8	G;
	/// Red componant.
	uint8	R;
	/// Alpha componant.
	uint8	A;
};


/**
 * Class pixel float RGBA
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CRGBAF
{
public:
	/// Default constructor. do nothing
	CRGBAF ()
	{}

	/** 
	 * Constructor.
	 * \param _r Red componant.
	 * \param _g Green componant.
	 * \param _b Blue componant.
	 * \param _a Alpha componant.
	 */
	CRGBAF (float _r, float _g, float _b, float _a=1.f)
	{
		R=_r;
		G=_g;
		B=_b;
		A=_a;
	}

	/** 
	 * Constructor with a CRGBA.
	 * \param c CRGBA color.
	 */
	CRGBAF (const CRGBA& c)
	{
		R=(float)c.R/255.f;
		G=(float)c.G/255.f;
		B=(float)c.B/255.f;
		A=(float)c.A/255.f;
	}

	/** 
	 * Cast operator to CRGBA.
	 */
	operator CRGBA() const
	{
		uint8 _r=(uint8)(R*255.f);
		uint8 _g=(uint8)(G*255.f);
		uint8 _b=(uint8)(B*255.f);
		uint8 _a=(uint8)(A*255.f);
		return CRGBA (_r, _g, _b, _a);
	}

	/** 
	 * Normalize component between [0.f,1.f]
	 */
	void normalize ()
	{
		R= (R>1.f) ? 1.f : (R<0.f) ? 0.f : R;
		G= (G>1.f) ? 1.f : (G<0.f) ? 0.f : G;
		B= (B>1.f) ? 1.f : (B<0.f) ? 0.f : B;
		A= (A>1.f) ? 1.f : (A<0.f) ? 0.f : A;
	}

	/** 
	 * Add operator. Sum components.
	 * \param c CRGBA color.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator+ (const CRGBAF& c) const
	{
		return CRGBAF (R+c.R, G+c.G, B+c.B, A+c.A);
	}

	/** 
	 * Sub operator. Substract components.
	 * \param c CRGBA color.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator- (const CRGBAF& c) const
	{
		return CRGBAF (R-c.R, G-c.G, B-c.B, A-c.A);
	}

	/** 
	 * Mul operator. Mul components.
	 * \param c CRGBA color.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator* (const CRGBAF& c) const
	{
		return CRGBAF (R*c.R, G*c.G, B*c.B, A*c.A);
	}

	/** 
	 * Mul float operator. Mul each component by f.
	 * \param f Float factor.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator* (float f) const
	{
		return CRGBAF (R*f, G*f, B*f, A*f);
	}

	/** 
	 * Div float operator. Div each component by f.
	 * \param f Float factor.
	 * \return Return the result of the opertor
	 */
	CRGBAF operator/ (float f) const
	{
		return CRGBAF (R/f, G/f, B/f, A/f);
	}

	/** 
	 * Add operator. Add each component.
	 * \param c CRGBA color.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator+= (const CRGBAF& c)
	{
		R+=c.R;
		G+=c.G;
		B+=c.B;
		A+=c.A;
		return *this;
	}

	/** 
	 * Sub operator. Substract each component.
	 * \param c CRGBA color.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator-= (const CRGBAF& c)
	{
		R-=c.R;
		G-=c.G;
		B-=c.B;
		A-=c.A;
		return *this;
	}

	/** 
	 * Mul operator. Multiplate each component.
	 * \param c CRGBA color.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator*= (const CRGBAF& c)
	{
		R*=c.R;
		G*=c.G;
		B*=c.B;
		A*=c.A;
		return *this;
	}

	/** 
	 * Mul float operator. Multiplate each component by f.
	 * \param f Float factor.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator*= (float f)
	{
		R*=f;
		G*=f;
		B*=f;
		A*=f;
		return *this;
	}

	/** 
	 * Div float operator. Divide each component by f.
	 * \param f Float factor.
	 * \return Return a reference on the caller object
	 */
	CRGBAF& operator/= (float f)
	{
		R/=f;
		G/=f;
		B/=f;
		A/=f;
		return *this;
	}

	/** 
	 * Serialisation.
	 * \param f Stream used for serialisation.
	 */
	void    serial(class NLMISC::IStream &f);

	/** 
	 * Set colors.
	 * \param r Red componant.
	 * \param g Green componant.
	 * \param b Blue componant.
	 * \param a Alpha componant.
	 */
	void set(float r, float g, float b, float a);

	/// Red componant.
	float	R;
	/// Green componant.
	float	G;
	/// Blue componant.
	float	B;
	/// Alpha componant.
	float	A;
};

/** 
 * Mul float operator. Multiplate each component by f.
 * \param f Float factor.
 * \return Return the result
 */
inline CRGBAF operator* (float f, const CRGBAF& c)
{
	return CRGBAF (c.R*f, c.G*f, c.B*f, c.A*f);
}

} // NLMISC


#endif // NL_RGBA_H

/* End of rgba.h */
