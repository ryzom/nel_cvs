/** \file text_context_user.h
 * <File description>
 *
 * $Id: text_context_user.h,v 1.2 2001/02/28 16:19:51 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_TEXT_CONTEXT_USER_H
#define NL_TEXT_CONTEXT_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_text_context.h"
#include "nel/3d/text_context.h"


namespace NL3D
{


/**
 * UTextContext implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTextContextUser : public UTextContext
{
private:
	CTextContext	_TextContext;
	IDriver			*_Driver;

public:

	/// Constructor
	CTextContextUser(const std::string fontFileName, const std::string fontExFileName, IDriver *drv, CFontManager *fmg)
	{
		// The enum of CComputedString and UTextContext MUST be the same!!!
		nlassert(UTextContext::HotSpotCount== CComputedString::HotSpotCount);

		_Driver= drv;
		_TextContext.init(drv, fmg);
		_TextContext.setFontGenerator(fontFileName, fontExFileName);
	}
	virtual ~CTextContextUser() {}


	/// \name Text look.
	// @{
	void setColor(NLMISC::CRGBA color)
	{
		_TextContext.setColor(color);
	}
	void setFontSize(uint32 fontSize) 
	{
		_TextContext.setFontSize(fontSize);
	}
	uint32 getFontSize() const  
	{
		return _TextContext.getFontSize();
	}
	void setHotSpot(THotSpot hotSpot)  
	{
		_TextContext.setHotSpot((CComputedString::THotSpot)(uint32)hotSpot) ;
	}
	THotSpot getHotSpot() const 
	{
		return (THotSpot)(uint32)_TextContext.getHotSpot();
	}
	void setScaleX(float scaleX)  
	{
		_TextContext.setScaleX(scaleX);
	}
	void setScaleY(float scaleY)  
	{
		_TextContext.setScaleZ(scaleY);
	}
	float getScaleX() const 
	{
		return _TextContext.getScaleX();
	}
	float getScaleY() const 
	{
		return _TextContext.getScaleZ();
	}
	void setShaded(bool b) 
	{
		_TextContext.setShaded(b);
	}
	bool getShaded() const  
	{
		return _TextContext.getShaded();
	}
	void setShadeExtent(float shext) 
	{
		_TextContext.setShadeExtent(shext);
	}
	// @}


	/// \name Rendering.
	/** All rendering are done in current UDriver matrix context. So verify your 2D/3D modes.
	 *
	 */
	// @{
	uint32 textPush(const char *format, ...)  
	{
		char *str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

		return _TextContext.textPush(ucstring(str)) ;
	}
	uint32 textPush(ucstring str)  
	{
		return _TextContext.textPush(str) ;
	}
	void erase(uint32 i)  
	{
		_TextContext.erase(i);
	}
	void clear()  
	{
		_TextContext.clear();
	}
	void printAt(float x, float y, uint32 i) 
	{
		_TextContext.printAt(x, y, i);
	}
	void printAt(float x, float y, ucstring ucstr) 
	{
		_TextContext.printAt(x, y, ucstr);
	}
	void printfAt(float x, float y, const char * format, ...) 
	{
		char *str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

		_TextContext.printAt(x, y, ucstring(str)) ;
	}

	void render3D(const CMatrix &mat, ucstring ucstr) 
	{
		CComputedString computedStr;
		_TextContext.computeString(ucstr,computedStr);
		
		computedStr.render3D(*_Driver,mat);
	}
	void render3D(const CMatrix &mat, const char *format, ...) 
	{
		char *str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

		render3D(mat, ucstring(str));
	}


	float getLastXBound() const 
	{
		return _TextContext.getLastXBound();
	}
	// @}

};


} // NL3D


#endif // NL_TEXT_CONTEXT_USER_H

/* End of text_context_user.h */
