/** \file driver.h
 * Generic driver header.
 * Low level HW classes : ITexture, CMaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * \todo yoyo: garbage collector system, to remove NULL _Shaders, _TexDrvShares and _VBDrvInfos entries. 
 * Add lights mgt to the driver.
 *
 * $Id: driver.h,v 1.44 2001/01/05 10:57:30 berenguier Exp $
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

#ifndef NL_DRV_H
#define NL_DRV_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/3d/texture.h"
#include "nel/3d/uv.h"

#include <vector>
#include <list>

namespace NLMISC
{
class IEventEmitter;
};

namespace NL3D
{

using NLMISC::CRefPtr;
using NLMISC::CRefCount;
using NLMISC::CSmartPtr;
using NLMISC::CRGBA;
using NLMISC::CVector;
using NLMISC::CMatrix;

class IShader;
class IVBDrvInfos;
class CMaterial;
class CVertexBuffer;
class CPrimitiveBlock;

// --------------------------------------------------


class GfxMode 
{
public:
	bool				Windowed;
	uint16				Width;
	uint16				Height;
	uint8				Depth;

						GfxMode(void) 
						{ 
							Windowed=false;
							Width=0;
							Height=0;
							Depth=0;
						}
						GfxMode(uint16 w, uint16 h, uint8 d, bool windowed= true);
};

typedef std::vector<GfxMode> ModeList;

// --------------------------------------------------
// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IDriver
{
public:
	/// Version of the driver interface. To increment when the interface change.
	static const uint32						InterfaceVersion;

protected:
	// The map of shared textures.
	typedef	std::map< std::string, CRefPtr<ITextureDrvInfos> >	TTexDrvInfoPtrMap;
	// The list of pointer on shared textures.
	typedef	std::list< CRefPtr<CTextureDrvShare> >	TTexDrvSharePtrList;
	typedef	std::list< CRefPtr<IShader> >			TShaderPtrList;
	typedef	std::list< CRefPtr<IVBDrvInfos> >		TVBDrvInfoPtrList;
	typedef	TTexDrvInfoPtrMap::iterator				ItTexDrvInfoPtrMap;
	typedef	TTexDrvSharePtrList::iterator			ItTexDrvSharePtrList;
	typedef	TShaderPtrList::iterator				ItShaderPtrList;
	typedef	TVBDrvInfoPtrList::iterator				ItVBDrvInfoPtrList;


protected:
	TTexDrvInfoPtrMap		_TexDrvInfos;
	TTexDrvSharePtrList		_TexDrvShares;
	TShaderPtrList			_Shaders;
	TVBDrvInfoPtrList		_VBDrvInfos;

public:
	enum TMessageBoxId { okId=0, yesId, noId, abortId, retryId, cancelId, ignoreId };
	enum TMessageBoxType { okType=0, okCancelType, yesNoType, abortRetryIgnoreType, yesNoCancelType, retryCancelType, typeCount };
	enum TMessageBoxIcon { noIcon=0, handIcon, questionIcon, exclamationIcon, asteriskIcon, warningIcon, errorIcon, informationIcon, stopIcon, iconCount };

							IDriver(void);
	virtual					~IDriver(void);

	virtual bool			init(void)=0;

	virtual ModeList		enumModes()=0;

	// first param is the associated window. 
	// Must be a HWND for Windows (WIN32).
	virtual bool			setDisplay(void* wnd, const GfxMode& mode)=0;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)=0;

	virtual NLMISC::IEventEmitter*	getEventEmitter(void)=0;

	virtual bool			clear2D(CRGBA rgba)=0;

	virtual bool			clearZBuffer(float zval=1)=0;

	virtual bool			setupTexture(ITexture& tex)=0;

	virtual bool			setupMaterial(CMaterial& mat)=0;

	// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective=true)=0;

	virtual void			setupViewMatrix(const CMatrix& mtx)=0;

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0)=0;

	virtual CMatrix			getViewMatrix(void)const=0;

	virtual bool			activeVertexBuffer(CVertexBuffer& VB)=0;

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat)=0;

	virtual bool			swapBuffers(void)=0;

	/// Deriver should calls IDriver::release() first, to destroy all driver components (textures, shaders, VBuffers).
	virtual bool			release(void);

	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive()=0;

	/** Output a system message box and print a message with an icon. This method can be call even if the driver is not initialized.
	  * This method is used to return internal driver problem when string can't be displayed in the driver window.
	  * If the driver can't open a messageBox, it should not override this method and let the IDriver class manage it with the ASCII console.
	  *
	  * \param message This is the message to display in the message box.
	  * \param title This is the title of the message box.
	  * \param type This is the type of the message box, ie number of button and label of buttons.
	  * \param icon This is the icon of the message box should use like warning, error etc...
	  */
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	/** Set the current viewport
	  *
	  * \param viewport is a viewport to setup as current viewport.
	  */
	virtual void			setupViewport (const class CViewport& viewport)=0;

	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion () const =0;

	/**
	  * Get driver informations.
	  *
	  */
	virtual const char*		getDriverInformation ()=0;

	/// show cursor if b is true, or hide it if b is false
	virtual void showCursor(bool b) = 0;

	/// x and y must be between 0.0 and 1.0
	virtual void setMousePos(float x, float y) = 0;

	/// Get the width and the height of the window
	virtual void getWindowSize(uint32 &width, uint32 &height) = 0;

};

// --------------------------------------------------

}

#endif // NL_DRV_H

