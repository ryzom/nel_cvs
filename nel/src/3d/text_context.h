/** \file text_context.h
 * <File description>
 *
 * $Id: text_context.h,v 1.11 2003/01/23 17:59:29 berenguier Exp $
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

#ifndef NL_TEXT_CONTEXT_H
#define NL_TEXT_CONTEXT_H

#include "3d/font_manager.h"
#include "3d/computed_string.h"


namespace NL3D {

class CFontGenerator;

/**
 * CTextContext
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextContext
{
public:

	/** 
	 * Constructor 
	 * defaults : fontsize=12, color=(0,0,0,255), hotspot=bottomleft, scale=1, shaded=false, shadeExtent=0.001f
	 *			  shadecolor=(0,0,0,255), 800x600ratio=true
	 */
	CTextContext ();

	/// Destructor
	~CTextContext();

	/**
	 * Inits
	 */

	/// set the driver.
	void init (IDriver *drv, CFontManager *fmg)
	{
		nlassert(drv && fmg);
		_Driver= drv;
		_FontManager= fmg;
	}

	// get the fontManager
	CFontManager	*getFontManager() const {return _FontManager;}

	/// Must be called before any print
	void setFontGenerator (const std::string fontFileName, const std::string fontExFileName = "");
	NL3D::CFontGenerator *getFontGenerator () { return _FontGen; }

	/**
	 * Accessors SET
	 */

	void setColor (NLMISC::CRGBA color) { _Color = color; }

	void setFontSize (uint32 fontSize)	{ _FontSize = fontSize; }

	void setHotSpot (CComputedString::THotSpot hotSpot)	{ _HotSpot = hotSpot; }

	void setScaleX (float scaleX) { _ScaleX = scaleX; }

	void setScaleZ (float scaleZ) { _ScaleZ = scaleZ; }

	void setShaded (bool b) { _Shaded = b; }

	void setShadeExtent (float shext) { _ShadeExtent = shext; }

	void setShadeColor (NLMISC::CRGBA color) { _ShadeColor = color; }

	/// If true the CFontManager look at Driver window size, and resize fontSize to keep the same
	/// size than if it was in 800x600...
	void setKeep800x600Ratio (bool keep) { _Keep800x600Ratio = keep; }

	/**
	 * Accessors GET
	 */

	NLMISC::CRGBA				getColor () const { return _Color; }

	uint32						getFontSize () const { return _FontSize; }

	CComputedString::THotSpot	getHotSpot() const { return _HotSpot; }

	float						getScaleX() const { return _ScaleX; }

	float						getScaleZ() const { return _ScaleZ; }

	bool						getShaded() const  { return _Shaded; }

	bool						getKeep800x600Ratio() const {return _Keep800x600Ratio;}

	NLMISC::CRGBA				getShadeColor () const { return _ShadeColor; }

	/**
	 * Cache methods
	 */

	/// compute and add a string to the cache (return the index)
	uint32 textPush (const char *format, ...);

	/// computes an ucstring and adds the result to the cache (return the index)
	uint32 textPush (const ucstring &str);

	/// remove a string from the cache
	void erase (uint32 index);

	/// Clear the cache
	void clear();

	/**
	 * Printing methods
	 */

	/// Print a string that is in the cache from its index (it leaves the string in the cache)
	void printAt (float x, float z, uint32 index)
	{
		nlassert (index < _CacheStrings.size());
		CComputedString &rCS = _CacheStrings[index];
		if(_Shaded)
		{
			CRGBA	bkup = rCS.Color;
			rCS.Color = _ShadeColor;
			rCS.Color.A = bkup.A;
			rCS.render2D (*_Driver, x+_ShadeExtent, z-_ShadeExtent, _HotSpot, _ScaleX, _ScaleZ);
			rCS.Color= bkup;
		}
		rCS.render2D (*_Driver, x, z, _HotSpot, _ScaleX, _ScaleZ);
	}

	/// Clip and print a string that is in the cache (it leaves the string in the cache)
	void printClipAt (CRenderStringBuffer &rdrBuffer, float x, float z, uint32 index, float xmin, float ymin, float xmax, float ymax)
	{
		nlassert (index < _CacheStrings.size());
		CComputedString &rCS = _CacheStrings[index];
		if(_Shaded)
		{
			CRGBA	bkup = rCS.Color;
			rCS.Color= _ShadeColor;
			rCS.Color.A = bkup.A;
			rCS.render2DClip(*_Driver, rdrBuffer, x+_ShadeExtent, z-_ShadeExtent, xmin, ymin, xmax, ymax);
			rCS.Color= bkup;
		}
		rCS.render2DClip (*_Driver, rdrBuffer, x, z, xmin, ymin, xmax, ymax);
	}

	/// Directly print a string
	void printAt (float x, float z, const ucstring &ucstr)
	{
		nlassert(_FontGen);

		// compute the string just one time
		_FontManager->computeString (ucstr, _FontGen, _Color, _FontSize, _Driver, _TempString, _Keep800x600Ratio);

		// draw shaded
		if(_Shaded)
		{
			CRGBA	bkup = _TempString.Color;
			_TempString.Color= _ShadeColor;
			_TempString.Color.A = bkup.A;
			_TempString.render2D (*_Driver,x+_ShadeExtent,z-_ShadeExtent,_HotSpot,_ScaleX,_ScaleZ);
			_TempString.Color= bkup;
		}

		// draw
		_TempString.render2D (*_Driver, x, z, _HotSpot, _ScaleX, _ScaleZ);
	}
	
	/// Directly print a string
	void printfAt (float x, float z, const char * format, ...)
	{
		nlassert(_FontGen);

		// compute the string just one time
		char *str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);
		_FontManager->computeString (str, _FontGen, _Color, _FontSize, _Driver, _TempString, _Keep800x600Ratio);

		// draw shaded
		if(_Shaded)
		{
			CRGBA	bkup = _TempString.Color;
			_TempString.Color = _ShadeColor;
			_TempString.Color.A = bkup.A;
			_TempString.render2D (*_Driver,x+_ShadeExtent,z-_ShadeExtent,_HotSpot,_ScaleX,_ScaleZ);
			_TempString.Color= bkup;
		}

		// draw
		_TempString.render2D (*_Driver, x, z, _HotSpot, _ScaleX, _ScaleZ);
	}
	
	/// Get computed string from index
	CComputedString& operator[](uint32 index)
	{
		nlassert (index < _CacheStrings.size());
		return _CacheStrings[index];
	}

	CComputedString* getComputedString (uint32 index)
	{
		if (index < _CacheStrings.size())
			return &_CacheStrings[index];
		else
			return NULL;
	}


	/**
	 * Compute a string as primitive blocks using the
	 * font manager's method computeString
	 * \param a string
	 * \param the computed string
	 */
	void computeString (const std::string& s, CComputedString& output)
	{
		_FontManager->computeString (s, _FontGen, _Color, _FontSize, _Driver, output, _Keep800x600Ratio);
	}

	/**
	 * Compute a ucstring as primitive blocks using the
	 * font manager's method computeString
	 * \param an ucstring
	 * \param the computed string
	 */
	void computeString (const ucstring& s, CComputedString& output)
	{
		_FontManager->computeString (s, _FontGen, _Color, _FontSize, _Driver, output, _Keep800x600Ratio);
	}

	void computeStringInfo (const ucstring& s, CComputedString& output)
	{
		_FontManager->computeStringInfo (s, _FontGen, _Color, _FontSize, _Driver, output, _Keep800x600Ratio);
	}

	/// Debug : write to the disk the texture cache
	void dumpCache (const char *filename)
	{
		_FontManager->dumpCache (filename);
	}

private:

  	/// Driver
	IDriver		*_Driver;

	/// Font manager
	NL3D::CFontManager	*_FontManager;
	
	/// Font generator
	NL3D::CFontGenerator * _FontGen;

	/**
	 * Text Style properties
	 */

	/// Font size;
	uint32						_FontSize;
	
	/// Current text color
	NLMISC::CRGBA				_Color;

	/// Hotspot
	CComputedString::THotSpot	_HotSpot;

	/// X scale 
	float						_ScaleX;
	
	/// Z scale
	float						_ScaleZ;

	/// true if text is shaded
	bool						_Shaded;

	/// shade's extent (shadow size)
	float						_ShadeExtent;

	/// Shade color (default is black)
	NLMISC::CRGBA				_ShadeColor;

	/// resize the font to keep the same aspect ratio than in 800x600
	bool						_Keep800x600Ratio;

	/**
	 * Strings Caches
	 */

	/// Cache to manipulate strings with indexes
	std::vector<CComputedString>	_CacheStrings;
	std::vector<uint32>				_CacheFreePlaces;
	uint32							_CacheNbFreePlaces;


	/// Cache for for printAt() and printfAt().
	/// This prevents from creating VBdrvinfos each time they are called (N*each frame!!).
	CComputedString		_TempString;

};


} // NL3D


#endif // NL_TEXT_CONTEXT_H

/* End of text_context.h */
