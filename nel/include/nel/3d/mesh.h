/** \file mesh.h
 * <File description>
 *
 * $Id: mesh.h,v 1.4 2000/12/18 09:45:20 corvazier Exp $
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

#ifndef NL_MESH_H
#define NL_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/3d/shape.h"
#include "nel/3d/driver.h"
#include "nel/3d/aabbox.h"
#include "nel/3d/uv.h"
#include <set>


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CPlane;
using	NLMISC::CMatrix;


// ***************************************************************************
/**
 * An instanciable mesh.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMesh : public IShape
{
public:

	/// \name Structures for building a mesh.
	//@{

	/// A corner of a face.
	struct	CCorner
	{
		sint		Vertex;		/// The vertex Id.
		CVector		Normal;
		CUV			Uvs[IDRV_VF_MAXSTAGES];
		CRGBA		Color;
		CRGBA		Specular;
		float		Weights[IDRV_VF_MAXW];

		// Setup all to 0, but Color (to white)... Important for good corner comparison.
		// This is slow but doesn't matter since used at mesh building....
		CCorner();

		bool		operator<(const CCorner &c) const;

	private:
		// The result of the compression.
		mutable sint	VBId;
		// The flags to know what to compare.
		static	sint	Flags;
		friend class CMesh;
	};

	/// A Triangle face.
	struct	CFace
	{
		CCorner		Corner[3];
		sint		MaterialId;
	};

	/// A mesh information.
	struct	CMeshBuild
	{
		/** the IDRV_VF* flags which tells what vertices data are used. See IDriver::setVertexFormat() for 
		 * more information. NB: IDRV_VF_XYZ is always considered to true.
		 */
		sint					VertexFlags;
		std::vector<CMaterial>	Materials;
		std::vector<CVector>	Vertices;
		std::vector<CFace>		Faces;
	};
	//@}


public:
	/// Constructor
	CMesh();

	/// Build a mesh, replacing old.
	void			build(const CMeshBuild &mbuild);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid);

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMesh);


// ************************
private:
	class	CRdrPass
	{
	public:
		CMaterial			Material;
		CPrimitiveBlock		PBlock;

		// Serialize a rdrpass.
		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(0);

			f.serial(Material);
			f.serial(PBlock);
		}
	};

private:
	/// The only one VBuffer of the mesh.
	CVertexBuffer			_VBuffer;
	/// The face.
	std::vector<CRdrPass>	_RdrPass;
	/// For clipping.
	CAABBoxExt				_BBox;


private:
	// Locals, for build.
	class	CCornerPred
	{
	public:
		bool operator()(const CCorner *x, const CCorner *y) const
		{
			return (*x<*y);
		}
	};
	typedef		std::set<CCorner*, CCornerPred>	TCornerSet;
	typedef		TCornerSet::iterator ItCornerSet;

	// Find and fill the VBuffer.
	void	findVBId(TCornerSet  &corners, const CCorner *corn, sint &currentVBIndex, const CVector &vert)
	{
		ItCornerSet  it= corners.find(const_cast<CCorner *>(corn));
		if(it!=corners.end())
			corn->VBId= (*it)->VBId;
		else
		{
			sint	i;
			corn->VBId= currentVBIndex++;
			// Fill the VBuffer.
			_VBuffer.setNumVertices(currentVBIndex);
			sint	id= currentVBIndex-1;
			// XYZ.
			_VBuffer.setVertexCoord(id, vert);
			// Normal
			if(CCorner::Flags & IDRV_VF_NORMAL)
				_VBuffer.setNormalCoord(id, corn->Normal);
			// Uvs.
			for(i=0;i<IDRV_VF_MAXSTAGES;i++)
			{
				if(CCorner::Flags & IDRV_VF_UV[i])
					_VBuffer.setTexCoord(id, i, corn->Uvs[i].U, corn->Uvs[i].V);
			}
			// Color.
			if(CCorner::Flags & IDRV_VF_COLOR)
				_VBuffer.setColor(id, corn->Color);
			// Specular.
			if(CCorner::Flags & IDRV_VF_SPECULAR)
				_VBuffer.setSpecular(id, corn->Specular);
			// Weights
			for(i=0;i<IDRV_VF_MAXW;i++)
			{
				if(CCorner::Flags & IDRV_VF_W[i])
					_VBuffer.setWeight(id, i, corn->Weights[i]);
			}
		}
	}

};


} // NL3D


#endif // NL_MESH_H

/* End of mesh.h */
