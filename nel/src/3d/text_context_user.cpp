/** \file text_context_user.cpp
 * <File description>
 *
 * $Id: text_context_user.cpp,v 1.5 2002/06/10 09:30:09 berenguier Exp $
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

#include "std3d.h"

#include "3d/text_context_user.h"
#include "nel/misc/hierarchical_timer.h"


namespace NL3D 
{

H_AUTO_DECL( NL3D_UI_TextContext )
H_AUTO_DECL( NL3D_Render_TextContext_2D )
H_AUTO_DECL( NL3D_Render_TextContext_3D )

#define	NL3D_HAUTO_UI_TEXTCONTEXT						H_AUTO_USE( NL3D_UI_TextContext )
#define	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT				H_AUTO_USE( NL3D_Render_TextContext_2D )
#define	NL3D_HAUTO_RENDER_3D_TEXTCONTEXT				H_AUTO_USE( NL3D_Render_TextContext_3D )


// ***************************************************************************
NLMISC::CVector UTextContext::CStringInfo::getHotSpotVector(UTextContext::THotSpot hotspot)
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	NLMISC::CVector hotspotVector(0,0,0);

	if(hotspot==UTextContext::MiddleLeft)
		hotspotVector = CVector(0,0,-StringHeight/2);
	
	if(hotspot==UTextContext::TopLeft)
		hotspotVector = CVector(0,0,StringHeight);
	
	if(hotspot==UTextContext::MiddleBottom)
		hotspotVector = CVector(-StringWidth/2,0,0);
	
	if(hotspot==UTextContext::MiddleMiddle)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight/2);
	
	if(hotspot==UTextContext::MiddleTop)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight);
	
	if(hotspot==UTextContext::BottomRight)
		hotspotVector = CVector(-StringWidth,0,0);
	
	if(hotspot==UTextContext::MiddleRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight/2);
	
	if(hotspot==UTextContext::TopRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight);

	return hotspotVector;
}



// ***************************************************************************
void CTextContextUser::setColor(NLMISC::CRGBA color)
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setColor(color);
}
void CTextContextUser::setFontSize(uint32 fontSize) 
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setFontSize(fontSize);
}
uint32 CTextContextUser::getFontSize() const  
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	return _TextContext.getFontSize();
}
void CTextContextUser::setHotSpot(THotSpot hotSpot)  
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setHotSpot((CComputedString::THotSpot)(uint32)hotSpot) ;
}
UTextContext::THotSpot CTextContextUser::getHotSpot() const 
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	return (THotSpot)(uint32)_TextContext.getHotSpot();
}
void CTextContextUser::setScaleX(float scaleX)  
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setScaleX(scaleX);
}
void CTextContextUser::setScaleY(float scaleY)  
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setScaleZ(scaleY);
}
float CTextContextUser::getScaleX() const 
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	return _TextContext.getScaleX();
}
float CTextContextUser::getScaleY() const 
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	return _TextContext.getScaleZ();
}
void CTextContextUser::setShaded(bool b) 
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setShaded(b);
}
bool CTextContextUser::getShaded() const  
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	return _TextContext.getShaded();
}
void CTextContextUser::setShadeExtent(float shext) 
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setShadeExtent(shext);
}
void			CTextContextUser::setKeep800x600Ratio(bool keep)
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	_TextContext.setKeep800x600Ratio(keep);
}
bool			CTextContextUser::getKeep800x600Ratio() const
{
	NL3D_HAUTO_UI_TEXTCONTEXT;

	return _TextContext.getKeep800x600Ratio();
}


// ***************************************************************************
uint32 CTextContextUser::textPush(const char *format, ...)  
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	return _TextContext.textPush(ucstring(str)) ;
}
uint32 CTextContextUser::textPush(const ucstring &str)  
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	return _TextContext.textPush(str) ;
}
void CTextContextUser::erase(uint32 i)  
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	_TextContext.erase(i);
}
UTextContext::CStringInfo		CTextContextUser::getStringInfo(uint32 i)
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	CComputedString		*cstr= _TextContext.getComputedString(i);
	if(!cstr)
		return CStringInfo(0,0);
	else
		return	CStringInfo(cstr->StringWidth, cstr->StringHeight);
}
void CTextContextUser::clear()  
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	_TextContext.clear();
}
void CTextContextUser::printAt(float x, float y, uint32 i) 
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	_TextContext.printAt(x, y, i);
	_DriverUser->restoreMatrixContext();
}
void CTextContextUser::printAt(float x, float y, ucstring ucstr) 
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	_TextContext.printAt(x, y, ucstr);
	_DriverUser->restoreMatrixContext();
}
void CTextContextUser::printfAt(float x, float y, const char * format, ...) 
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	_TextContext.printAt(x, y, ucstring(str)) ;
	_DriverUser->restoreMatrixContext();
}

void CTextContextUser::render3D(const CMatrix &mat, ucstring ucstr) 
{
	NL3D_HAUTO_RENDER_3D_TEXTCONTEXT;

	CComputedString computedStr;
	_TextContext.computeString(ucstr,computedStr);
	
	computedStr.render3D(*_Driver,mat);

	_DriverUser->restoreMatrixContext();
}
void CTextContextUser::render3D(const CMatrix &mat, const char *format, ...) 
{
	NL3D_HAUTO_RENDER_3D_TEXTCONTEXT;

	char *str;
	NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

	render3D(mat, ucstring(str));

	_DriverUser->restoreMatrixContext();
}


float CTextContextUser::getLastXBound() const 
{
	NL3D_HAUTO_RENDER_2D_TEXTCONTEXT;

	return _TextContext.getLastXBound();
}

// ***************************************************************************
void			CTextContextUser::dumpCacheTexture (const char *filename)
{
	_TextContext.dumpCache (filename);
}


} // NL3D
