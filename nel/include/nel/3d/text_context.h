/** \file text_context.h
 * <File description>
 *
 * $Id: text_context.h,v 1.2 2000/12/19 15:15:50 coutelas Exp $
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
	
	/// display decriptor (window description)
	NL3D::CDisplayDescriptor _DispDesc;

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


public:

	/// Constructor
	CTextContext()
	{
		_FontGen = NULL;

		_FontSize = 12;
		
		_DispDesc.ResX = 800;
		_DispDesc.ResY = 600;
		_DispDesc.FontRatio = 1.0f;

		_Color = NLMISC::CRGBA(0,0,0);

		_HotSpot = NL3D::CComputedString::BottomLeft;

		_ScaleX = 1;
		_ScaleZ = 1;
		_RotateY = 0;
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
	 *	setWindowSize
	 * set the window size infos. Should be called before the first print
	 * \param width the window width
	 * \param height the window height 
	 */
	void setWindowSize(uint16 width, uint16 height)
	{
		_DispDesc.ResX = width;
		_DispDesc.ResY = height;
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
		_FontManager.computeString(str,_FontGen,_Color,_FontSize,_DispDesc,cptdstr);
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
	 * \param x (between 0 and 1)
	 * \param y (between 0 and 1)
	 * \param i index in list
	 */
	void printAt(float x, float y, uint32 i)
	{
		nlassert(i<_StringList.size());

		_StringList[i].render2D(*NL3D::CNELU::Driver,
								x,y,
								_HotSpot,
								_ScaleX,_ScaleZ);
	}

	/**
	 *	printAt
	 * print a ucstring at the location
	 * \param x (between 0 and 1)
	 * \param y (between 0 and 1)
	 * \param ucstr the ucstring
	 */
	void printAt(float x, float y, ucstring ucstr)
	{
		NL3D::CComputedString cptdstr;
		_FontManager.computeString(ucstr,_FontGen,_Color,_FontSize,_DispDesc,cptdstr);
		cptdstr.render2D(*NL3D::CNELU::Driver,
							x,y,
							_HotSpot,
							_ScaleX,_ScaleZ);
	}

	/**
	 *	printfAt
	 * compute and print a string at the location
	 * \param x (between 0 and 1)
	 * \param y (between 0 and 1)
	 * \param a string
	 */
	void printfAt(float x, float y, const char * format, ...)
	{
		nlassert(_FontGen);

		char str[1024];
		va_list args;
		va_start(args, format);
		vsprintf(str, format, args);
		va_end(args);

		NL3D::CComputedString cptdstr;
		_FontManager.computeString(str,_FontGen,_Color,_FontSize,_DispDesc,cptdstr);
		cptdstr.render2D(*NL3D::CNELU::Driver,x,y,_HotSpot,_ScaleX,_ScaleZ);
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
	
	/// destructor
	~CTextContext()
	{
		if(_FontGen) delete _FontGen;
	}
};


} // NL3D


#endif // NL_TEXT_CONTEXT_H

/* End of text_context.h */
