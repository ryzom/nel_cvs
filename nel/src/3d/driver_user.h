/** \file driver_user.h
 * <File description>
 *
 * $Id: driver_user.h,v 1.27 2003/04/30 09:43:11 berenguier Exp $
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

#ifndef NL_DRIVER_USER_H
#define NL_DRIVER_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/3d/u_driver.h"
#include "3d/event_mouse_listener.h"
#include "3d/driver.h"
#include "3d/register_3d.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/font_manager.h"
#include "3d/material_user.h"
#include "3d/ptr_set.h"
#include "3d/shape_bank_user.h"
#include "3d/light_user.h"
#include "3d/mesh_skin_manager.h"
#include "3d/async_texture_manager.h"
#include "3d/lod_character_manager.h"

#define NL3D_MEM_DRIVER						NL_ALLOC_CONTEXT( 3dDrv )

namespace NL3D 
{


class	CTextureUser;
class	CTextContextUser;
class	CSceneUser;


// ***************************************************************************
/** UDriver implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CDriverUser : public UDriver
{
// **********************
protected:
	struct	CMatrixContext
	{
		CScissor	Scissor;		// Init to fullscreen.
		CViewport	Viewport;		// Init to fullscreen.
		CFrustum	Frustum;
		CMatrix		ViewMatrix;
		CMatrix		ModelMatrix;
	};


protected:
	IDriver					*_Driver;
	bool					_WindowInit;
	CMatrixContext			_CurrentMatrixContext;
	CFontManager			_FontManager;
	// Components List.
	typedef	CPtrSet<CTextureUser>		TTextureSet;
	typedef	CPtrSet<CMaterialUser>		TMaterialSet;
	typedef	CPtrSet<CTextContextUser>	TTextContextSet;
	typedef	CPtrSet<CSceneUser>			TSceneSet;
	TTextureSet				_Textures;
	TMaterialSet			_Materials;
	TTextContextSet			_TextContexts;
	TSceneSet				_Scenes;
	CShapeBankUser			_ShapeBank;
	// There is one MeshSkinManager per driver, and for all scenes.
	CMeshSkinManager		_MeshSkinManager;
	// There is one AsyncTextureManager per driver, and for all scenes
	CAsyncTextureManager	_AsyncTextureManager;
	// There is one LodCharacterManager per driver, and for all scenes
	CLodCharacterManager	_LodCharacterManager;

	// For 2D/3D Interface.
	CVertexBuffer			_VBFlat;
	CVertexBuffer			_VBColor;
	CVertexBuffer			_VBUv;
	CVertexBuffer			_VBColorUv;
	CPrimitiveBlock			_PBLine, _PBTri, _PBQuad;

	CVertexBuffer			_VBQuadsColUv;
	CVertexBuffer			_VBQuadsColUv2;
	// For security, texture are initUnlit() at init()/release().
	CMaterialUser			_MatFlat;
	CMaterialUser			_MatText;


	// StaticInit
	static	bool			_StaticInit;

protected:
	void			setupMatrixContext();
	CMaterial		&convMat(UMaterial &mat);


// **********************
public:


	/// \name Object
	// @{
	CDriverUser (uint windowIcon = 0);
	virtual	~CDriverUser();
	// @}


	/// \name Window / driver management.
	// @{

	/// Enum All Fullscreen modes.
	virtual TModeList		enumModes();

	virtual void			disableHardwareVertexProgram();
	virtual void			disableHardwareVertexArrayAGP();
	virtual void			disableHardwareTextureShader();

	/// create the window.
	virtual	bool			setDisplay(const CMode &mode) ;
	/// Release the window.
	virtual	void			release() ;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void);
	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive();
	/// Return an OS dependent window handle. Under Win32, it is a HWND.
	virtual void			*getDisplay ();

	// @}


	/// \name Buffers.
	// @{
	/// This clear only the RGBA back buffer
	virtual	void			clearRGBABuffer(CRGBA col= CRGBA(255,255,255,255));
	/// This clear only the RGBA back buffer
	virtual	void			clearZBuffer();
	/// This clear the buffers (ALL the buffer :) )
	virtual	void			clearBuffers(CRGBA col= CRGBA(255,255,255,255));
	/// This swap the back and front buffer (ALL the buffer :) ).
	virtual	void			swapBuffers();

	virtual void			setSwapVBLInterval(uint interval);
	virtual uint			getSwapVBLInterval();

	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color);
	// @}

	/// \name Light support.
	// @{
	virtual void			setLight (uint8 num, const ULight& light);
	virtual void			enableLight (uint8 num, bool enable=true);
	virtual void			setAmbientColor (CRGBA color);
	// @}


	/// \name Scene gestion.
	// @{
	/// Create a new scene.
	virtual	UScene			*createScene() ;
	/// Delete a scene.
	virtual	void			deleteScene(UScene	*scene) ;
	// @}



	/// \name Components gestion for Interface 2D/3D.
	// @{

	/// create a new TextContext, for a given font.
	virtual	UTextContext	*createTextContext(const std::string fontFileName, const std::string fontExFileName = "") ;
	/// delete a TextContext.
	virtual	void			deleteTextContext(UTextContext	*textContext) ;
	/// Set the maxMemory used for the FontManager
	virtual	void			setFontManagerMaxMemory(uint maxMem) ;
	/// get cahce information.
	virtual		std::string getFontManagerCacheInformation() const ;


	/** Create a new texture file, searching in CPath.
	 * \param file filename, local to CPath paths.
	 */
	virtual	UTextureFile	*createTextureFile(const std::string &file) ;
	/// Delete a texture file. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureFile(UTextureFile *textfile) ;
	/// Create a new Raw texture, to be filled by user.
	virtual	UTextureRaw		*createTextureRaw() ;
	/// Delete a Raw texture. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureRaw(UTextureRaw *textraw) ;
	/// Create a new Material, to be filled by user.
	virtual	UMaterial		*createMaterial() ;
	/// Delete a Material.
	virtual	void			deleteMaterial(UMaterial *mat) ;
	// @}


	/// \name Matrix context for Interface 2D/3D.
	/** UScene ignore those function (use camera parameters instead), and do not disturb this active Matrix context.
	 * (after a scene rendering, the Matrix context for this interface is restored).
	 */
	// @{

	/** Set the active scissor for rendering. Default to fullscreen.
	 */
	virtual	void			setScissor(const CScissor &);
	virtual	CScissor		getScissor();
	/** Set the active viewport for rendering. Default to fullscreen.
	 */
	virtual	void			setViewport(const CViewport &);
	virtual	CViewport		getViewport();
	/** Set the active Frustum for rendering. 
	 */
	virtual	void			setFrustum(const CFrustum &frust) ;
	virtual	CFrustum		getFrustum() ;
	/** Set the active ViewMatrix for rendering.
	 * NB: this is the view matrix, which is the inverse of camera matrix.
	 */
	virtual	void			setViewMatrix(const CMatrix &mat) ;
	virtual	CMatrix			getViewMatrix() ;
	/** Set the active ModelMatrix for rendering. NB: UScene ignore this function (use camera parameters instead).
	 */
	virtual	void			setModelMatrix(const CMatrix &mat) ;
	virtual	CMatrix			getModelMatrix() ;


	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 2D.
	 * ModelMatrix is setup to identity. ViewMatrix is setup so that (x,y) of vectors maps to x,y screen!!!
	 */
	virtual	void			setMatrixMode2D(const CFrustum &frust) ;
	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 3D, using parameters of a UCamera.
	 * ModelMatrix setuped to identity. ViewMatrix setuped to the inverse of camera 's LocalMatrix.
	 * Frustum setuped to UCamera frustum.
	 */
	virtual	void			setMatrixMode3D(UCamera &camera) ;

	// @}


	/// \name Interface 2D/3D.
	/** All of those render primitives are unlit! You must use UScene to render lighted meshes.
	 * NB: If you set a texture to your material, the primitives are textured, even if no Uvs are provided. \n
	 * NB: All rendering are done in current viewport / current matrix context.
	 */
	// @{

	/// Draw the Line, taking color from material.
	virtual	void			drawLine(const NLMISC::CLine &tri, UMaterial &mat) ;
	/// Draw the Line, taking color from primitive.
	virtual	void			drawLine(const NLMISC::CLineColor &tri, UMaterial &mat) ;
	/// Draw the Line, taking color from material. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineUV &tri, UMaterial &mat) ;
	/// Draw the Line, taking color from primitive. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineColorUV &tri, UMaterial &mat) ;

	/// Draw the Triangle, taking color from material.
	virtual	void			drawTriangle(const NLMISC::CTriangle &tri, UMaterial &mat) ;
	/// Draw the Triangle, taking color from primitive.
	virtual	void			drawTriangle(const NLMISC::CTriangleColor &tri, UMaterial &mat) ;
	/// Draw the Triangle, taking color from material. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleUV &tri, UMaterial &mat) ;
	/// Draw the Triangle, taking color from primitive. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleColorUV &tri, UMaterial &mat) ;

	/// Draw the Quad, taking color from material.
	virtual	void			drawQuad(const NLMISC::CQuad &tri, UMaterial &mat) ;
	/// Draw the Quad, taking color from primitive.
	virtual	void			drawQuad(const NLMISC::CQuadColor &tri, UMaterial &mat) ;
	/// Draw the Quad, taking color from material. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadUV &tri, UMaterial &mat) ;
	/// Draw the Quad, taking color from primitive. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadColorUV &tri, UMaterial &mat) ;

	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV> &quad, UMaterial &mat) ;
	virtual	void			drawQuads(const std::vector<NLMISC::CQuadColorUV2> &quad, UMaterial &mat) ;
	virtual	void			drawQuads(const NLMISC::CQuadColorUV *quads, uint32 nbQuads, UMaterial &mat) ;
	virtual	void			drawQuads(const NLMISC::CQuadColorUV2 *quads, uint32 nbQuads, UMaterial &mat) ;
	
	// @}


	/// \name Tools for Interface 2D.
	/** For all those function, setMatrixMode2D*() should have been called (else strange results!!).
	 */
	// @{

	/// Draw a bitmap 2D. Warning: this is slow...
	virtual	void			drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend=true, CRGBA col= CRGBA(255,255,255,255)) ;
	/// Draw a line in 2D. Warning: this is slow...
	virtual	void			drawLine (float x0, float y0, float x1, float y1, CRGBA col= CRGBA(255,255,255,255)) ;
	/// Draw a Triangle in 2D. Warning: this is slow...
	virtual	void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float xcenter, float ycenter, float radius, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col) ;

	// @}



	/// \name Driver information/Queries
	// @{
	virtual uint32			getImplementationVersion () const;
	virtual const char*		getDriverInformation ();
	virtual const char*		getVideocardInformation ();
	virtual	sint			getNbTextureStages();
	virtual void			getWindowSize (uint32 &width, uint32 &height);
	virtual uint			getWindowWidth ();
	virtual uint			getWindowHeight ();
	virtual void			getWindowPos (uint32 &x, uint32 &y);
	virtual uint32			getAvailableVertexAGPMemory ();
	virtual uint32			getAvailableVertexVRAMMemory ();
	virtual void			getBuffer (CBitmap &bitmap) ;
	virtual void			getZBuffer (std::vector<float>  &zbuffer) ;
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) ;
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) ;
	virtual bool			fillBuffer (CBitmap &bitmap);
	// @}


	/// \name Mouse / Keyboards / Game devices
	// @{
	virtual NLMISC::IMouseDevice			*enableLowLevelMouse(bool enable, bool exclusive);
	//
	virtual NLMISC::IKeyboardDevice			*enableLowLevelKeyboard(bool enable) ;
	virtual NLMISC::IInputDeviceManager		*getLowLevelInputDeviceManager();
	virtual uint	getDoubleClickDelay(bool hardwareMouse);

	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor (bool b);
	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos (float x, float y);
	/// If true, capture the mouse to force it to stay under the window.
	virtual void			setCapture (bool b);
	// @}


	/// \name Misc.
	// @{

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
	

	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode);
	virtual U3dMouseListener*	create3dMouseListener ();
	virtual void delete3dMouseListener (U3dMouseListener *listener);
	virtual TPolygonMode 	getPolygonMode () ;
	virtual void			forceDXTCCompression(bool dxtcComp);
	virtual void			forceTextureResize(uint divisor);
	virtual bool			setMonitorColorProperties (const CMonitorColorProperties &properties);
	// @}

	/// \name Shape Bank
	// @{
	/// 
	virtual	UShapeBank*		getShapeBank()
	{
		NL3D_MEM_DRIVER
		return &_ShapeBank;
	}
	// @}


	/// \name Profiling.
	// @{

	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut);

	virtual	uint32			profileAllocatedTextureMemory();

	virtual	uint32			profileSetupedMaterials() const;

	virtual	uint32			profileSetupedModelMatrix() const;

	virtual void			enableUsedTextureMemorySum (bool enable);
	
	virtual uint32			getUsedTextureMemory() const;

	// @}


	/// \name Async Texture loading mgt
	// @{
	virtual void				setupAsyncTextureLod(uint baseLevel, uint maxLevel);
	virtual void				setupAsyncTextureMaxUploadPerFrame(uint maxup);
	virtual void				setupMaxTotalAsyncTextureSize(uint maxText);
	virtual void				setupMaxHLSColoringPerFrame(uint maxCol);
	virtual void				updateAsyncTexture();
	virtual	uint				getTotalAsyncTextureSizeAsked() const;
	virtual	uint				getLastAsyncTextureSizeGot() const;
	virtual void				loadHLSBank(const std::string &fileName);
	// @}


public:
	/// \name Accessor for CSeneUser.
	// @{
	IDriver		*getDriver()
	{
		NL3D_MEM_DRIVER
		return _Driver;
	}
	void		restoreMatrixContext()
	{
		NL3D_MEM_DRIVER
		setupMatrixContext();
	}
	// @}

};


} // NL3D


#endif // NL_DRIVER_USER_H

/* End of driver_user.h */
