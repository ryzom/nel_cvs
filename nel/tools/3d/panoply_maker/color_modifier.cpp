/** \file color_modifier.cpp
 * A class describing color modifications
 *
 * $Id: color_modifier.cpp,v 1.1 2002/02/06 10:12:55 vizerie Exp $
 */

/* Copyright, 2000, 2001, 2002 Nevrax Ltd.
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

#include "color_modifier.h"
#include <nel/misc/bitmap.h>



///=================================================================================================
void CColorModifier::convertBitmap(NLMISC::CBitmap &destBitmap, const NLMISC::CBitmap &srcBitmap, const NLMISC::CBitmap &maskBitmap) const
{	
	/// make sure all bitmap have the same size
	nlassert(destBitmap.getWidth() == srcBitmap.getWidth() && srcBitmap.getWidth() == maskBitmap.getWidth()
			 && destBitmap.getHeight() == srcBitmap.getHeight() && srcBitmap.getHeight() == maskBitmap.getHeight());

	float h, s, l;
	evalBitmapStats(srcBitmap, maskBitmap, h, s, l);
	nlinfo("Bitmap stats : (H, L, S) = (%g, %g, %g)", h, s, l);
	float deltaH = Hue - h;
	
	
	const NLMISC::CRGBA  *src   = (NLMISC::CRGBA *) &srcBitmap.getPixels()[0];
	const NLMISC::CRGBA  *mask =  (NLMISC::CRGBA *) &maskBitmap.getPixels()[0];
		  NLMISC::CRGBA  *dest =  (NLMISC::CRGBA *) &destBitmap.getPixels()[0];


	for (uint y = 0; y < srcBitmap.getHeight(); ++y)
	{
		for (uint x = 0; x < srcBitmap.getWidth(); ++x)
		{
			if (src->convertToHLS(h, l, s)) // achromatic ?
			{
				h = 0;
			}

			NLMISC::CRGBA result;

			result.buildFromHLS(h + deltaH, l + Lightness, s + Saturation);

			/// apply contrasts			
			sint meanValue = ((sint) result.R + (sint) result.G + (sint) result.B) / 3;

			float r = 255.f * Luminosity + (float) meanValue + Contrast * ((sint) result.R - meanValue);				  
			NLMISC::clamp(r, 0, 255);
			result.R = (uint8) r;

			float g = 255.f * Luminosity + (float) meanValue + Contrast * ((sint) result.G - meanValue);				  
			NLMISC::clamp(g, 0, 255);
			result.G = (uint8) g;

			float b = Luminosity + (float) meanValue + Contrast * ((sint) (uint) result.B - meanValue);				  
			NLMISC::clamp(b, 0, 255);
			result.B = (uint8) b;


			// blend to the destination by using the mask alpha			
			dest->blendFromui(*dest, result, mask->A);
			

			/// keep alpha from the source
			dest->A = src->A;
			
			++ src;
			++ mask;
			++ dest;
		}
	}
}

///=================================================================================================
void CColorModifier::evalBitmapStats(const NLMISC::CBitmap &srcBitmap, const NLMISC::CBitmap &maskBitmap, float &H, float &S, float &L)
{

	nlassert(srcBitmap.getWidth() == maskBitmap.getWidth()
			 && srcBitmap.getHeight() == maskBitmap.getHeight());

	float hTotal  = 0;
	float hWeight = 0;

	float lTotal  = 0;
	float lWeight = 0;

	float sTotal  = 0;
	float sWeight = 0;

	const NLMISC::CRGBA *src = (NLMISC::CRGBA *) &srcBitmap.getPixels()[0];
	const NLMISC::CRGBA *mask = (NLMISC::CRGBA *) &maskBitmap.getPixels()[0];

	for (uint y = 0; y < srcBitmap.getHeight(); ++y)
	{
		for (uint x = 0; x < srcBitmap.getWidth(); ++x)
		{
			float h, l, s;
		
			float intensity = mask->A * (1.f / 255.f);			
			bool achromatic = src->convertToHLS(h, l, s);
			
			lTotal  += intensity * l;
			lWeight += intensity;
			sTotal  += s * intensity;
			sWeight += intensity;

			if (!achromatic)
			{
				hTotal  += h * intensity;	
				hWeight += intensity;

			}			

			++mask;
			++src;
		}
	}

	H = (hWeight != 0) ? hTotal / hWeight : 0.f;
	S = (sWeight != 0) ? sTotal / sWeight : 0.f;
	L = (lWeight != 0) ? lTotal / lWeight : 0.f;	
}

