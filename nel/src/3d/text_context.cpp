/** \file text_context.cpp
 * <File description>
 *
 * $Id: text_context.cpp,v 1.5 2002/02/28 12:59:51 besson Exp $
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

#include "std3d.h"

#include "3d/text_context.h"
#include "3d/font_generator.h"

namespace NL3D {



uint32 CTextContext::textPush(const char *format, ...)
{ 
	nlassert(_FontGen);

	// convert the string.
	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	// Compute the string after insert in map, to avoid copy of vector<>
	NL3D::CComputedString cptdstr;
	_MaxIndex++;
	std::map<uint32,CComputedString>::iterator	it;
	it= ( _StringList.insert(std::make_pair(_MaxIndex,cptdstr)) ).first;

	// compute the string.
	NL3D::CComputedString	&strToFill= it->second;
	_FontManager->computeString(str,_FontGen,_Color,_FontSize,_Driver, strToFill, _Keep800x600Ratio);

	return _MaxIndex;
}

/**
 * computes an ucstring and adds the result to the stack
 * \param an ucstring
 * \return the index where computed string has been inserted
 */
uint32 CTextContext::textPush(const ucstring &str)
{ 
	nlassert(_FontGen);

	// Compute the string after insert in map, to avoid copy of vector<>
	NL3D::CComputedString cptdstr;
	_MaxIndex++;
	std::map<uint32,CComputedString>::iterator	it;
	it= ( _StringList.insert(std::make_pair(_MaxIndex,cptdstr)) ).first;

	// compute the string.
	NL3D::CComputedString	&strToFill= it->second;
	_FontManager->computeString(str,_FontGen,_Color,_FontSize,_Driver, strToFill, _Keep800x600Ratio);

	return _MaxIndex;
}


/**
 * init the font generator. Must be called before any print
 * \param (cf CFontGenerator constructor parameters)
 */
void CTextContext::setFontGenerator(const std::string fontFileName, const std::string fontExFileName)
{
	_FontGen = new NL3D::CFontGenerator(fontFileName, fontExFileName);
}

/// destructor
CTextContext::~CTextContext()
{
	if(_FontGen) delete _FontGen;
}


} // NL3D
