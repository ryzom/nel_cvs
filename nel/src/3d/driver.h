/** \file driver.h
 * Generic driver header.
 * Low level HW classes : ITexture, CMaterial, CVertexBuffer, CPrimitiveBlock, IDriver
 *
 * $Id: driver.h,v 1.3 2001/07/03 09:12:34 berenguier Exp $
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
#include "nel/misc/common.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/misc/uv.h"
#include "3d/texture.h"
#include "3d/shader.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "nel/misc/mutex.h"

#include <vector>
#include <list>

namespace NLMISC
{
class IEventEmitter;
class CRect;
};

namespace NL3D
{

using NLMISC::CRefPtr;
using NLMISC::CRefCount;
using NLMISC::CSmartPtr;
using NLMISC::CRGBA;
using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CSynchronized;


class CMaterial;
class CPrimitiveBlock;
class CLight;


//****************************************************************************
/// A Graphic Mode descriptor.
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



//****************************************************************************
// Exceptions.
struct EBadDisplay : public NLMISC::Exception
{
	EBadDisplay(const std::string &reason) : Exception(reason) { }
};



//****************************************************************************
// Enalbe normal transformation mode.
#define	NL3D_VERTEX_MODE_NORMAL			0
// Use Skinning.
#define	NL3D_VERTEX_MODE_SKINNING		1
// NL3D_VERTEX_MODE_???  2, 4, 8 etc....


//****************************************************************************
// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IDriver
{
public:
	/// Version of the driver interface. To increment when the interface change.
	static const uint32						InterfaceVersion;

public:
	enum TMessageBoxId { okId=0, yesId, noId, abortId, retryId, cancelId, ignoreId, idCount };
	enum TMessageBoxType { okType=0, okCancelType, yesNoType, abortRetryIgnoreType, yesNoCancelType, retryCancelType, typeCount };
	enum TMessageBoxIcon { noIcon=0, handIcon, questionIcon, exclamationIcon, asteriskIcon, warningIcon, errorIcon, informationIcon, stopIcon, iconCount };

	/**
	  * Driver's polygon modes.
	  *
	  * \see setPolygonMode, getPolygonMode
	  */
	enum TPolygonMode { Filled=0, Line, Point };


	/**
	  * Driver Max matrix count.
	  *
	  * \see setupModelMatrix()
	  */
	enum TMatrixCount { MaxModelMatrix= 16 };


protected:

	CSynchronized<TTexDrvInfoPtrMap> _SyncTexDrvInfos;

	TTexDrvSharePtrList		_TexDrvShares;
	TShaderPtrList			_Shaders;
	TVBDrvInfoPtrList		_VBDrvInfos;
	TPolygonMode			_PolygonMode;

public:
							IDriver(void);
	virtual					~IDriver(void);

	virtual bool			init(void)=0;

	virtual ModeList		enumModes()=0;

	// first param is the associated window. 
	// Must be a HWND for Windows (WIN32).
	virtual bool			setDisplay(void* wnd, const GfxMode& mode) throw(EBadDisplay)=0;

	// Return is the associated window information. (Implementation dependant)
	// Must be a HWND for Windows (WIN32).
	virtual void*			getDisplay() =0;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)=0;

	/// Get the number of texture stage avaliable, for multitexturing (Normal material shaders). Valid only after setDisplay().
	virtual	sint			getNbTextureStages() const =0;

	/// is the texture is set up in the driver
	virtual bool			isTextureExist(const ITexture&tex)=0;

	virtual NLMISC::IEventEmitter*	getEventEmitter(void)=0;

	virtual bool			clear2D(CRGBA rgba)=0;

	virtual bool			clearZBuffer(float zval=1)=0;

	virtual bool			setupTexture(ITexture& tex)=0;

	virtual bool			setupMaterial(CMaterial& mat)=0;

	// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective=true)=0;

	/** setup the view matrix (inverse of camera matrix).
	 *
	 * NB: you must setupViewMatrix() BEFORE setupModelMatrix(), or else undefined results.
	 */
	virtual void			setupViewMatrix(const CMatrix& mtx)=0;

	/** setup a model matrix. IDdriver::MaxModelMatrix (16) can be setuped.
	 * The 0th model matrix is the principal one. Others are only usefull fro skinning (see CVertexBuffer, and setupVertexMode).
	 *
	 * NB: you must setupModelMatrix() AFTER setupViewMatrix(), or else undefined results.
	 *
	 * \see setupVertexMode
	 */
	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0)=0;

	virtual CMatrix			getViewMatrix(void)const=0;


	/** setup the vertex transformation Mode. (or vertex program). This should be a ORed of NL3D_VMODE_*
	 * Default is NL3D_VERTEX_MODE_NORMAL.
	 */
	virtual	void			setupVertexMode(uint vmode)=0;


	/** Force input normal to be normalized by the driver. default is false.
	 * NB: driver force the normalisation himself if:
	 *		- current VB has hardware skinning.
	 *		- current Model matrix has a scale.
	 */
	virtual	void			forceNormalize(bool normalize)=0;


	/** return the forceNormalize() state.
	 */
	virtual	bool			isForceNormalize() const =0;



	/** create a IVertexBufferHard. delete it with deleteVertexBufferHard.
	 *	NB: user should (must) keep a CRefPtr<> on this ptr, because if driver is deleted (for any reason)
	 *	the pointer will be no longer valid.
	 */
	virtual	IVertexBufferHard	*createVertexBufferHard(uint32 vertexFormat, uint32 numVertices) =0;


	/** delete a IVertexBufferHard. NB: VertexBufferHard are automatically deleted at IDriver::release();
	 */
	virtual	void			deleteVertexBufferHard(IVertexBufferHard *VB) =0;


	/** active a current VB Hard, for future render().
	 *
	 * NB: software skinning is not possible with this method. User should test supportPaletteSkinning() to know
	 * if skinning can be done in hardware. If not, he should not use VB Hard, but standard VB.
	 *
	 * \see setupVertexMode
	 */
	virtual void			activeVertexBufferHard(IVertexBufferHard *VB)=0;



	/** active a current VB, for future render().
	 * This method suppose that all vertices in the VB will be used.
	 *
	 * NB: software skinning (if any) will be actuallay done in render*() call, only one time per vertex.
	 * Vertex Skinning Flags are reseted in activeVertexBuffer().
	 *
	 *  Skinning is enabled only when VB has skinning, and when vertexMode has flag NL3D_VERTEX_MODE_SKINNING.
	 *
	 * \see setupVertexMode
	 */
	virtual bool			activeVertexBuffer(CVertexBuffer& VB)=0;


	/** active a current VB, for future render().
	 * This method suppose that only vertices in given range will be used in future render(). 
	 * This could be usefull for DX or OpenGL driver, but it is usefull for software skinning too.
	 * Undefined results if primitives in render() use vertices not in this range.
	 *
	 * NB: software skinning (if any) will be actuallay done in render*() call, only one time per vertex.
	 * Vertex Skinning Flags are reseted in activeVertexBuffer(), but only for given range here!
	 *
	 *  Skinning is enabled only when VB has skinning, and when vertexMode has flag NL3D_VERTEX_MODE_SKINNING.
	 *
	 * \param VB the vertexBuffer to activate.
	 * \param first the first vertex important for render (begin to 0). nlassert(first<=end);
	 * \param end the last vertex important for render, +1. count==end-first. nlassert(end<=VB.getNumVertices);
	 *
	 * \see setupVertexMode
	 */
	virtual bool			activeVertexBuffer(CVertexBuffer& VB, uint first, uint end)=0;


	/** render a block of primitive with previously setuped VertexBuffer / Matrixes.
	 * NB: nlassert() if setupModelMatrix() or setupViewMatrix() has been called between activeVertexBuffer() and render*().
	 */
	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat)=0;

	/** render a list of triangles with previously setuped VertexBuffer / Matrixes.
	 * NB: nlassert() if setupModelMatrix() or setupViewMatrix() has been called between activeVertexBuffer() and render*().
	 * NB: this is usefull for landscape....
	 */
	virtual void			renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris)=0;

	/** render points with previously setuped VertexBuffer / Matrixes.
	 * NB: nlassert() if setupModelMatrix() or setupViewMatrix() has been called between activeVertexBuffer() and render*().
	 */
	virtual void			renderPoints(CMaterial& Mat, uint32 numPoints)=0;


	/// Swap the back and front buffers.
	virtual bool			swapBuffers(void)=0;

	/**
	  * Returns the number of model matrices supported in hardware by the driver.
	  * NeL will support from 2 to 4 matrices by vertices.
	  * If the user uses a model with a greater count of matrices than the hardware can support,
	  * the skinning will be made in software.
	  *
	  * For the time, driver opengl supports 2 matrices in hardware on Geforce, and implement nothing in software.
	  * (Use paletted skinning instead)
	  */
	virtual uint			getNumMatrix()=0;

	/**
	  * Returns true if the hardware support PaletteSkinning.
	  * NeL will support 4 matrices by vertices, and up to IDriver::MaxModelMatrix (16) model matrixes.
	  * If the user uses PaletteSkinning while the hardware does not support it, 
	  * the skinning will be made in software.
	  */
	virtual bool			supportPaletteSkinning()=0;


	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled()=0;
	virtual	void			enableFog(bool enable)=0;
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color)=0;
	// @}

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


	/** Set the current Scissor.
	  * \param viewport is a viewport to setup the current Scissor, in Window relative coordinate (0,1).
	  */
	virtual void			setupScissor (const class CViewport& viewport)=0;


	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion () const = 0;

	/**
	  * Get driver informations.
	  * get the nel name of the driver (ex: "Opengl 1.2 NeL Driver")
	  */
	virtual const char*		getDriverInformation () = 0;

	/**
	  * Get videocard informations.
	  * get the official name of the driver
	  */
	virtual const char*		getVideocardInformation () = 0;

	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor (bool b) = 0;

	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos (float x, float y) = 0;

	/// Get the width and the height of the window
	virtual void			getWindowSize (uint32 &width, uint32 &height) = 0;

	/// If true, capture the mouse to force it to stay under the window.
	virtual void			setCapture (bool b) = 0;

	/** get the RGBA back buffer
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  */
	virtual void			getBuffer (CBitmap &bitmap) = 0;

	/** get the ZBuffer (back buffer).
	  *
	  * \param zbuffer the returned array of Z. size of getWindowSize() .
	  */
	virtual void			getZBuffer (std::vector<float>  &zbuffer) = 0;

	/** get a part of the RGBA back buffer
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  * \param rect the in/out (wanted/clipped) part of Color buffer to retrieve.
	  */
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) = 0;

	/** get a part of the ZBuffer (back buffer).
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param zbuffer the returned array of Z. size of rec.Width*rec.Height.
	  * \param rect the in/out (wanted/clipped) part of ZBuffer to retrieve.
	  */
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) = 0;

	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode)
	{
		_PolygonMode=mode;
	}

	/** 
	  * Setup a light.
	  *
	  * You must call enableLight() to active the ligth.
	  *
	  * \param num is the number of the light to set.
	  * \param light is a light to set in this slot.
	  * \see enableLight()
	  */
	virtual void			setLight (uint8 num, const CLight& light) = 0;

	/** 
	  * Enable / disable light.
	  *
	  * You must call setLight() if you active the ligth.
	  *
	  * \param num is the number of the light to enable / disable.
	  * \param enable is true to enable the light, false to disable it.
	  * \see setLight()
	  */
	virtual void			enableLight (uint8 num, bool enable=true) = 0;

	/** 
	  * Set ambiant.
	  *
	  * \param color is the new global ambiant color for the scene.
	  * \see setLight(), enableLight()
	  */
	virtual void			setAmbientColor (CRGBA color) = 0;

	/** Get the global polygon mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see setPolygonMode(), TPolygonMode
	  */
	TPolygonMode 	getPolygonMode ()
	{
		return _PolygonMode;
	}

protected:
	friend	class	IVBDrvInfos;
	friend	class	CTextureDrvShare;
	friend	class	ITextureDrvInfos;
	friend	class	IShader;

	/// remove ptr from the lists in the driver.
	void			removeVBDrvInfoPtr(ItVBDrvInfoPtrList  vbDrvInfoIt);
	void			removeTextureDrvInfoPtr(ItTexDrvInfoPtrMap texDrvInfoIt);
	void			removeTextureDrvSharePtr(ItTexDrvSharePtrList texDrvShareIt);
	void			removeShaderPtr(ItShaderPtrList shaderIt);
};

// --------------------------------------------------

}

#endif // NL_DRV_H

