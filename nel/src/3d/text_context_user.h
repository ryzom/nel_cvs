/** \file text_context_user.h
 * <File description>
 *
 * $Id: text_context_user.h,v 1.13 2003/01/23 17:59:29 berenguier Exp $
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
#include "nel/3d/u_text_context.h"
#include "3d/text_context.h"
#include "3d/driver_user.h"

#define NL3D_MEM_TEXT_CONTEXT						NL_ALLOC_CONTEXT( 3dTxtCx )

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
	CDriverUser		*_DriverUser;
	IDriver			*_Driver;
	CComputedString _CacheString; // Performance Optimisation

public:

	/// Constructor
	CTextContextUser(const std::string fontFileName, const std::string fontExFileName, CDriverUser *drv, CFontManager *fmg)
	{
		NL3D_MEM_TEXT_CONTEXT
		nlassert(drv);
		_DriverUser= drv;

		// The enum of CComputedString and UTextContext MUST be the same!!!
		nlassert((uint)UTextContext::HotSpotCount== (uint)CComputedString::HotSpotCount);

		_Driver= drv->getDriver();
		_TextContext.init(_Driver, fmg);
		_TextContext.setFontGenerator(fontFileName, fontExFileName);
	}
	virtual ~CTextContextUser() 
	{
		NL3D_MEM_TEXT_CONTEXT
	}


	/// \name Text look.
	// @{
	void setColor(NLMISC::CRGBA color);
	void setFontSize(uint32 fontSize) ;
	uint32 getFontSize() const  ;
	void setHotSpot(THotSpot hotSpot)  ;
	THotSpot getHotSpot() const ;
	void setScaleX(float scaleX)  ;
	void setScaleY(float scaleY)  ;
	float getScaleX() const ;
	float getScaleY() const ;
	void setShaded(bool b) ;
	bool			getShaded() const  ;
	void			setShadeExtent(float shext) ;
	void			setShadeColor (NLMISC::CRGBA sc);
	NLMISC::CRGBA	getShadeColor () const;
	void setKeep800x600Ratio(bool keep);
	bool getKeep800x600Ratio() const;
	// @}


	/// \name Rendering.
	/** All rendering are done in current UDriver matrix context. So verify your 2D/3D modes.
	 *
	 */
	// @{
	uint32 textPush(const char *format, ...)  ;
	uint32 textPush(const ucstring &str)  ;
	void setStringColor(uint32 i, CRGBA newCol);
	void setStringSelection(uint32 i, uint32 selectStart, uint32 selectSize);
	void resetStringSelection(uint32 i);
	void erase(uint32 i)  ;
	virtual	CStringInfo		getStringInfo (uint32 i);
	virtual	CStringInfo		getStringInfo (const ucstring &ucstr);

	void clear()  ;
	void printAt(float x, float y, uint32 i) ;
	void printClipAt(URenderStringBuffer &renderBuffer, float x, float y, uint32 i, float xmin, float ymin, float xmax, float ymax) ;

	void printAt(float x, float y, ucstring ucstr) ;
	void printfAt(float x, float y, const char * format, ...) ;

	void render3D(const CMatrix &mat, ucstring ucstr) ;
	void render3D(const CMatrix &mat, const char *format, ...) ;

	float getLastXBound() const ;
	// @}

	void			dumpCacheTexture (const char *filename);

	virtual URenderStringBuffer		*createRenderBuffer();
	virtual void					deleteRenderBuffer(URenderStringBuffer *buffer);
	virtual void					flushRenderBuffer(URenderStringBuffer *buffer);

};


} // NL3D


#endif // NL_TEXT_CONTEXT_USER_H

/* End of text_context_user.h */
