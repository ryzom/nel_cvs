/** \file text_context.h
 * <File description>
 *
 * $Id: text_context.h,v 1.9 2001/01/05 13:48:11 coutelas Exp $
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

#include "nel/3d/font_manager.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/computed_string.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/material.h"
#include "nel/3d/nelu.h"


namespace NL3D {


/**
 * CTextContext
 *
	example : 
	CTextContext textContext;
	textContext.setFontGenerator(fontName);
	textContext.setWindowSize(width,height);
	textContext.setColor(color);
	textContext.setFontSize(fontSize);
	textContext.print(str, x, y);
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextContext
{
	/// font manager
	NL3D::CFontManager _FontManager;
	
	/// font generator
	NL3D::CFontGenerator * _FontGen;
	
	/// font size;
	uint32 _FontSize;
	
	/// current text color
	NLMISC::CRGBA _Color;

	/// hotspot
	NL3D::CComputedString::THotSpot _HotSpot;

	/// X scale 
	float _ScaleX;
	
	/// Z scale
	float _ScaleZ;

	/// Y axe rotation angle
	float _RotateY;

	/// computed strings stack
	std::vector<CComputedString> _StringList;

	/// max x coordinate of last string printed
	float _XBound;


public:

	/// Constructor
	CTextContext()
	{
		_FontGen = NULL;

		_FontSize = 12;

		_Color = NLMISC::CRGBA(0,0,0);

		_HotSpot = NL3D::CComputedString::BottomLeft;

		_ScaleX = 1;
		_ScaleZ = 1;
		_RotateY = 0;

		_XBound = 0;
	}
	
	/**
	 *	setFontGenerator
	 * init the font generator. Must be called before any print
	 * \param (cf CFontGenerator constructor parameters)
	 */
	void setFontGenerator(const std::string fontFileName, const std::string fontExFileName = "")
	{
		_FontGen = new NL3D::CFontGenerator(fontFileName, fontExFileName);
	}

	/**
	 *	setColor
	 * set the font color
	 * \param color the font color
	 */
	void setColor(NLMISC::CRGBA color)
	{
		_Color = color;
	}

	/**
	 *	setFontSize
	 * set the font size. Should be called before the first print
	 * \param fonSize the font size
	 */
	void setFontSize(uint32 fontSize)
	{
		_FontSize = fontSize;
	}

	/**
	 *	getFontSize
	 * get the font size
	 * \return the font size
	 */
	uint32 getFontSize() const
	{
		return _FontSize;
	}

	/**
	 *	setHotSpot
	 * set the hot spot
	 * \param fonSize the font size
	 */
	void setHotSpot(NL3D::CComputedString::THotSpot hotSpot)
	{
		_HotSpot = hotSpot;
	}

	/**
	 *	setScaleX
	 * set the X scale
	 * \param scaleX the X scale
	 */
	void setScaleX(float scaleX)
	{
		_ScaleX = scaleX;
	}

	/**
	 *	setScaleZ
	 * set the Z scale
	 * \param scaleZ the Z scale
	 */
	void setScaleZ(float scaleZ)
	{
		_ScaleZ = scaleZ;
	}

	/**
	 *	getScaleX
	 * \return the X scale
	 */
	float getScaleX() const
	{
		return _ScaleX;
	}

	/**
	 *	getScaleZ
	 * \return the Z scale
	 */
	float getScaleZ() const
	{
		 return _ScaleZ;
	}


	/**
	 *	getHotSpot
	 * get the hot spot
	 * \return the hot spot
	 */
	NL3D::CComputedString::THotSpot getHotSpot() const
	{
		return _HotSpot;
	}

	/**
	 *	textPush
	 * compute and add a string to the stack
	 * \param a string
	 * \return the index where string has been inserted
	 */
	uint32 textPush(const char * format, ...) 
	{ 
		nlassert(_FontGen);

		char str[1024];
		va_list args;
		va_start(args, format);
		vsprintf(str, format, args);
		va_end(args);

		NL3D::CComputedString cptdstr;
		_FontManager.computeString(str,_FontGen,_Color,_FontSize,NL3D::CNELU::Driver,cptdstr);
		_StringList.push_back(cptdstr);
		return _StringList.size()-1;
	}

	/**
	 *	textPush
	 * computes an ucstring and adds the result to the stack
	 * \param an ucstring
	 * \return the index where computed string has been inserted
	 */
	uint32 textPush(ucstring str) 
	{ 
		nlassert(_FontGen);

		NL3D::CComputedString cptdstr;
		_FontManager.computeString(str,_FontGen,_Color,_FontSize,NL3D::CNELU::Driver,cptdstr);
		_StringList.push_back(cptdstr);
		return _StringList.size()-1;
	}
	
	/**
	 *	erase
	 * remove a string from the list
	 */
	void erase(uint32 i) 
	{ 
		std::vector<CComputedString>::iterator it = _StringList.begin();
		uint32 index = 0;
		while(it!=_StringList.end() && index!=i)
		{
			it++;
			index++;
		};
	}

	/**
	 *	printAt
	 * print a string of the list
	 * (rq : it leaves the string in the stack)
	 */
	void printAt(float x, float z, uint32 i)
	{
		nlassert(i<_StringList.size());

		_StringList[i].render2D(*NL3D::CNELU::Driver,
								x,z,
								_HotSpot,
								_ScaleX,_ScaleZ);

		_XBound = x + _StringList[i].StringWidth;
	}

	/**
	 *	printAt
	 * compute and print a ucstring at the location
	 */
	void printAt(float x, float z, ucstring ucstr)
	{
		NL3D::CComputedString cptdstr;
		_FontManager.computeString(ucstr,_FontGen,_Color,_FontSize,NL3D::CNELU::Driver,cptdstr);
		cptdstr.render2D(*NL3D::CNELU::Driver,
							x,z,
							_HotSpot,
							_ScaleX,_ScaleZ);

		_XBound = x + cptdstr.StringWidth;
	}
	
	/**
	 *	printfAt
	 * compute and print a string at the location
	 */
	void printfAt(float x, float z, const char * format, ...)
	{
		nlassert(_FontGen);

		char str[1024];
		va_list args;
		va_start(args, format);
		vsprintf(str, format, args);
		va_end(args);

		NL3D::CComputedString cptdstr;
		_FontManager.computeString(str,_FontGen,_Color,_FontSize,NL3D::CNELU::Driver,cptdstr);
		cptdstr.render2D(*NL3D::CNELU::Driver,x,z,_HotSpot,_ScaleX,_ScaleZ);

		_XBound = x + cptdstr.StringWidth;
	}
	
	/**
	 *	getStringListSize
	 * \return the size of the string list
	 */
	uint32 getStringListSize() const
	{
		return _StringList.size();
	}

	/**
	 *	operator[]
	 * \return the computed string
	 */
	const CComputedString operator[](uint32 i) const
	{
		nlassert(i<_StringList.size());
		return _StringList[i];
	}
	
	/**
	 *	operator[]
	 * \return the computed string
	 */
	CComputedString& operator[](uint32 i)
	{
		nlassert(i<_StringList.size());
		return _StringList[i];
	}

	/**
	 * Compute a string as primitive blocks using the
	 * font manager's method computeString
	 * \param a string
	 * \param the computed string
	 */
	void computeString(const std::string& s, CComputedString& output)
	{
		_FontManager.computeString(s,_FontGen,_Color,_FontSize,NL3D::CNELU::Driver,output);
	}

	/**
	 * Compute a ucstring as primitive blocks using the
	 * font manager's method computeString
	 * \param an ucstring
	 * \param the computed string
	 */
	void computeString(const ucstring& s, CComputedString& output)
	{
		_FontManager.computeString(s,_FontGen,_Color,_FontSize,NL3D::CNELU::Driver,output);
	}

	/**
	 * Return max x coordinate of last string printed. Useful to know if a string
	 * goes out of the screen (screen limit is at x==4/3).
	 *	\return x coordinate
	 */
	float getLastXBound() const
	{
		return _XBound;
	}


	/// destructor
	~CTextContext()
	{
		if(_FontGen) delete _FontGen;
	}
};


} // NL3D


#endif // NL_TEXT_CONTEXT_H

/* End of text_context.h */
