/** \file lod_character_manager.h
 * <File description>
 *
 * $Id: lod_character_manager.h,v 1.2 2002/05/13 16:45:56 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_LOD_CHARACTER_MANAGER_H
#define NL_LOD_CHARACTER_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "3d/material.h"


namespace NLMISC
{
class	CMatrix;
}


namespace NL3D 
{


using NLMISC::CRGBA;
using NLMISC::CMatrix;

class	IDriver;
class	CLodCharacterShapeBank;
class	CLodCharacterShape;


// ***************************************************************************
/**
 * A Manger used to display CLodCharacter instances.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterManager
{
public:

	/// Constructor
	CLodCharacterManager();
	~CLodCharacterManager();


	/// \name build process
	// @{

	/// reset the manager.
	void			reset();

	/** create a Shape Bank. NB: a vector of ShapeBank is maintained internally, hence, not so many shapeBank should be 
	 *	created at same Time.
	 *	\return	id of the shape Bank.
	 */
	uint32			createShapeBank();

	/// get a shape Bank. UseFull for serialisation for example. return NULL if not found
	const CLodCharacterShapeBank	*getShapeBank(uint32 bankId) const;

	/// get a shape Bank. UseFull for serialisation for example. return NULL if not found
	CLodCharacterShapeBank	*getShapeBank(uint32 bankId);

	/// delete a Shape Bank. No-op if bad id.
	void			deleteShapeBank(uint32 bankId);


	/** Get a shapeId by its name. -1 if not found.
	 *	Call valid only if compile() has been correctly called
	 */
	sint32			getShapeIdByName(const std::string &name) const;

	/// Get a const ref on a shape. Ptr not valid if shape Banks are modfied. NULL if not found
	const CLodCharacterShape	*getShape(uint32 shapeId) const;

	/** re-compile the shape map. This must be called after changing shape bank list.
	 *	It return false if same names have been found, but it is still correctly builded.
	 */
	bool			compile();

	// @}


	/// \name render process
	// @{

	/** set the max number of vertices the manager can render in one time. Default is 3000 vertices.
	 *	nlassert if isRendering()
	 */
	void			setMaxVertex(uint32 maxVertex);

	/// see setMaxVertex()
	uint32			getMaxVertex() const {return _MaxNumVertices;}

	/** Start the rendering process, freeing VBuffer.
	 *	nlassert if isRendering()
	 *	NB: VBhard is locked here, so you must call endRender to unlock him (even if 0 meshes are rendered)
	 *
	 *	\param managerPos is to help ZBuffer Precision (see IDriver::setupViewMatrixEx). This vector is removed from 
	 *	all instance worldMatrixes, and a IDriver::setupModelMatrix() will be done with this position.
	 *	Hence, whatever value you give, the result will be the same. But if you give a value near the camera position,
	 *	ZBuffer precision will be enhanced.
	 */
	void			beginRender(IDriver *driver, const CVector &managerPos);

	/** Add an instance to the render list.
	 *	nlassert if not isRendering()
	 *	\param shapeId is the id of the lod character shape to use. No-Op if not found.
	 *	\param animId is the anim to use for this shape. No-Op if not found.
	 *	\param time is the time of animation
	 *	\param wrapMode if true, the anim loop, else just clamp
	 *	\param worldMatrix is the world matrix, used to display the mesh
	 *	\param colorVertex is an array of color, must be same size of the shape number vertices, else the
	 *	whole mesh is supposed to be gray. see CLodCharacterShape::startBoneColor() for how to build this array
	 *	\param globalLighting is used to simulate the lighting on the lod. this color is modulated with colorVertex
	 *	\return false if the key can't be added to this pass BECAUSE OF TOO MANY VERTICES reason. If the shapeId or animId
	 *	are bad id, it return true!! You may call endRender(), then restart a block. Or you may just stop the process 
	 *	if you want.
	 */
	bool			addRenderCharacterKey(uint shapeId, uint animId, TGlobalAnimationTime time, bool wrapMode, 
		const CMatrix &worldMatrix, const std::vector<CRGBA> &colorVertex, CRGBA globalLighting);

	/**	compile the rendering process, effectively rendering into driver the lods.
	 *	nlassert if not isRendering().
	 *	The VBHard is unlocked here.
	 */
	void			endRender();

	/// tells if we are beetween a beginRender() and a endRender() 
	bool			isRendering() const {return _Rendering;}

	// @}


// ******************************
private:
	/// Map name To Id.
	typedef	std::map<std::string, uint32>	TStrIdMap;
	typedef	TStrIdMap::iterator				ItStrIdMap;
	typedef	TStrIdMap::const_iterator		CstItStrIdMap;


private:

	/// Array of shapeBank
	std::vector<CLodCharacterShapeBank*>	_ShapeBankArray;

	/// Map of shape id
	TStrIdMap						_ShapeMap;


	/// \name render process
	// @{

	CVector							_ManagerMatrixPos;

	CMaterial						_Material;

	uint							_CurrentVertexId;
	uint							_MaxNumVertices;
	CVertexBuffer					_VBuffer;
	NLMISC::CRefPtr<IDriver>		_Driver;
	NLMISC::CRefPtr<IVertexBufferHard>	_VBHard;
	bool							_VBHardOk;
	uint8							*_VertexData;
	uint							_VertexSize;
	bool							_Rendering;

	// list of triangles
	uint							_CurrentTriId;
	std::vector<uint32>				_Triangles;

	
	void			deleteVertexBuffer();

	// @}

};


} // NL3D


#endif // NL_LOD_CHARACTER_MANAGER_H

/* End of lod_character_manager.h */
