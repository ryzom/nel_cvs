/** \file ps_mesh.h
 * Particle meshs
 *
 * $Id: ps_mesh.h,v 1.12 2003/07/03 16:16:45 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_PS_MESH_H
#define NL_PS_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/class_registry.h"
#include "3d/ps_particle_basic.h"
#include "3d/ps_attrib.h"
#include "3d/ps_plane_basis.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/shape.h"



#include <string>
#include <vector>
#include <queue>

namespace NLMISC
{
	class IStream;
	struct EStream;
}


namespace NL3D {


class CPSLocated;
class CTransformShape;
class CShapeBank;
class CMesh;



const uint ConstraintMeshMaxNumVerts			= 512; // the maximum number of vertices for a constraint mesh
const uint ConstraintMeshBufSize				= 64;  // number of meshs to be processed at once...
const uint ConstraintMeshMaxNumPrerotatedModels = 32;  // maximum number of meshs that can be prerotated

/** This class is for mesh handling. It operates with any mesh, but it must insert them in the scene...
 *  It is not very adapted for lots of little meshs..
 *  To create the mesh basis, we use CPlaneBasis here. It give us the I and J vector of the basis for each mesh 
 *  and compute K ( K =  I ^ J)
 */

class CPSMesh : public  CPSParticle,
			    public CPSSizedParticle,
				public CPSRotated3DPlaneParticle,
				public CPSRotated2DParticle,
				public CPSShapeParticle
{
public:
	/// construct the system by using the given shape for mesh
	CPSMesh(const std::string &shape = "") : _Invalidated(false)
	{
		_Shape = shape;
		_Name = std::string("Mesh");
	}

	/// set a new shape for that kind of particles
	void setShape(const std::string &shape) { _Shape = shape; }

	/// get the shape used for those particles	
	std::string getShape(void) const { return _Shape; }

		/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	virtual ~CPSMesh();

	NLMISC_DECLARE_CLASS(CPSMesh);


	/** invalidate the transformShapes that were inserted in the scene, so they need to be rebuilt
	 *  during the next rendering. This is useful for clipping, or when the system has been loaded
	 */

	void invalidate() 
	{ 
		_Invalidated = true; 
		_Instances.clear();
	}

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool		hasLightableFaces();

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getMaxNumFaces(void) const;

protected:
	/**	Generate a new element for this bindable. They are generated according to the properties of the class		 
	 */
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index);

	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt);

	/// in fact we don't draw the meshs, we just update their pos...
	virtual void updatePos();

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size);	

	//CSmartPtr<IShape> _Shape;

	std::string _Shape;

	// a container for mesh instances
	typedef CPSAttrib<CTransformShape *> TInstanceCont;

	TInstanceCont _Instances;

	// this is set to true when the transformed shape have to be recerated

	bool _Invalidated;

	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
	virtual CPSLocated *getPlaneBasisOwner(void) { return _Owner; }
}; 


/** This class is for mesh that have very simple geometry. The constraint is that they can only have one matrix block. 
 *  They got a hint for constant rotation scheme. With little meshs, this is the best to draw a maximum of them
 */

class CPSConstraintMesh : public CPSParticle,
						  public CPSSizedParticle,
						  public CPSRotated3DPlaneParticle,
						  public CPSHintParticleRotateTheSame,
						  public CPSShapeParticle,
						  public CPSColoredParticle
{
public:	
	/// ctor
	CPSConstraintMesh();

	virtual ~CPSConstraintMesh();

	/** Construct the mesh by using the given mesh shape file.
	  * No morphing is applied. The mesh is used 'as it'.
	  */
	void				setShape(const std::string &meshFileName);

	/// Get the shape used for those particles. (must use no morphing or an assertion is raised)
	std::string			getShape(void) const;



	/** Setup the mesh for morphing use. There are several restrictions : 	 
	  * - All meshs must have the same number of vertices
	  * - All meshes must have the same vertex format
	  * If these conditions are not met, a 'dummy' mesh will be used instead.
	  * If there's only one mesh, no morphing is performed.
	  * NB : Morphing not supported with precomputed rotations. First mesh is used instead
	  * \param shapesNames A tab of string containing the names of the shapes
	  * \param numShapes 
	  */
	void						setShapes(const std::string *shapesNames, uint numShapes);


	/// Set a shape by its index
	void						setShape(uint index, const std::string &shapeName);

	/// Get a shape name by its index
	const std::string          &getShape(uint index) const;

	/// Get the number of shapes used
	uint						getNumShapes() const;

	/** Retrieve the names of the shapes
	  * \param shapesNames :A tab of shapes with enough spaces to store the names
	  */
	void						getShapesNames(std::string *shapesNames) const;

	/// Use a constant value for morphing. This discard any scheme for the morph value. The value must range from 0 to numberOfShapes
	void						setMorphValue(float value);

	/// Get the value used for morphing
	float						getMorphValue() const;

	/// Set a morphing scheme. The scheme is then owned by this object
	void						setMorphScheme(CPSAttribMaker<float> *scheme);

	/// Get the current morphing scheme or NULL if no one was set
	CPSAttribMaker<float>		*getMorphScheme();

	/// Get the current morphing scheme or NULL if no one was set. Const version
	const CPSAttribMaker<float>	*getMorphScheme() const;
	  

	
	

	/** Tells that all meshs are turning in the same manner, and only have a rotationnal bias
	 *  This is a lot faster then other method. Any previous set scheme for 3d rotation is kept.
	 *	\param: the number of rotation configuration we have. The more high it is, the slower it'll be
	 *          If this is too low, a lot of particles will have the same orientation	           	 
	 *          If it is 0, then the hint is disabled.
	 *          This can't be higher than ConstraintMeshMaxNumPrerotatedModels
 	 *  \param  minAngularVelocity : the maximum angular velocity for particle rotation	 
	 *  \param  maxAngularVelocity : the maximum angular velocity for particle rotation	 
	 *  \see    CPSRotated3dPlaneParticle
	 */
	void				hintRotateTheSame(uint32 nbConfiguration,
										  float minAngularVelocity = NLMISC::Pi,
										  float maxAngularVelocity = NLMISC::Pi
										 );

	/** disable the hint 'hintRotateTheSame'
	 *  The previous set scheme for roation is used
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	void				disableHintRotateTheSame(void)
	{
		hintRotateTheSame(0);
	}

	/** check wether a call to hintRotateTheSame was performed
	 *  \return 0 if the hint is disabled, the number of configurations else
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */

	uint32				checkHintRotateTheSame(float &min, float &max) const
	{
		min = _MinAngularVelocity;
		max = _MaxAngularVelocity;
		return _PrecompBasis.size(); 
	}


	/// serialisation. Derivers must override this, and call their parent version
	virtual void		serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	
	NLMISC_DECLARE_CLASS(CPSConstraintMesh);
	
	/// return true if there are transparent faces in the object
	virtual bool		hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool		hasOpaqueFaces(void);

	/// from CPSParticle : return true if there are lightable faces in the object
	virtual bool		hasLightableFaces();

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32		getMaxNumFaces(void) const;


	/** Force the n-th stage of all material to be modulated by the mesh color. This allow to put colors on meshs
	  * that haven't got material that allow them.
	  * \param stage The stage the modulation applies on. Range from 0 to IDRV_MAT_MAXTEXTURES - 1.
	  * \param force True enable modulation, false disable it.
	  */
	void				forceStageModulationByColor(uint stage, bool force);
	
	/// Test if the i-th stage of all materials is forced to be modulated with the mesh color
	bool				isStageModulationForced(uint stage) const;

	/// force all material to use vertex color lighting
	void				forceVertexColorLighting(bool force = true) { _VertexColorLightingForced = force; }

	/// test wether vertex color lighting is forced.
	bool				isVertexColorLightingForced() const { return _VertexColorLightingForced; }

	/// Setup the buffers used with prerotated meshs. Must be called during initialization.
	static	void		initPrerotVB();

	//\name Texture animation
	//@{
		/// The type of animation that is used with meshs textures. 
		enum TTexAnimType { NoAnim = 0, GlobalAnim, /*Local, */ Last};

		/// Set the type of texture animation to use. None is the default. Setting a new value discard the previous change.
		void	setTexAnimType(TTexAnimType type);

		/// Get the the type of texture animation
		TTexAnimType getTexAnimType() const;
	//@}

	//\name Global texture animation. Calls to these method are only valid if texture animation is global.
	//@{
		/// Properties of global texture animation
		struct CGlobalTexAnim
		{
			NLMISC::CVector2f TransOffset; /* = (0, 0) */
			NLMISC::CVector2f TransSpeed; /* = (0, 0) */
			NLMISC::CVector2f TransAccel; /* = (0, 0) */
			NLMISC::CVector2f ScaleStart; /* = (1, 1) */
			NLMISC::CVector2f ScaleSpeed; /* = (0, 0) */
			NLMISC::CVector2f ScaleAccel; /* = (0, 0) */
			float			  WRotSpeed;  /* = 0 */
			float			  WRotAccel;  /* = 0 */
			CGlobalTexAnim();
			void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
			/// Build a texture matrix from a date and this obj.
			void    buildMatrix(TAnimationTime &date, NLMISC::CMatrix &dest);
		};

		/// Set the properties of texture animation for a texture stage. Global animation should have been activated.
		void			setGlobalTexAnim(uint stage, const CGlobalTexAnim &properties);

		/// Get the properties of texture animation.Global animation should have been activated.
		const CGlobalTexAnim &getGlobalTexAnim(uint stage) const;

		/// Force the time counter for global anim to be reseted when a new mesh is created.
		void  forceGlobalAnimTimeResetOnNewElement(bool force = true) { _ReinitGlobalAnimTimeOnNewElement = force; }
		bool  isGlobalAnimTimeResetOnNewElementForced()  const { return _ReinitGlobalAnimTimeOnNewElement != 0; }

	//@}

	

protected:
	friend class CPSConstraintMeshHelper;
	// inherited from CPSColoredParticle
	virtual CPSLocated *getColorOwner(void) { return _Owner; }

	// inherited from CPSColoredParticle
	virtual void updateMatAndVbForColor(void);

	/**	Generate a new element.
	 */
	virtual void		newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	
	/** Delete an element by its index	 
	 */
	virtual void		deleteElement(uint32 index);

	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt);
	/** called by the system when particles must be drawn
	  * \param opaque true if we are dealing with the opaque pass, false for transparent faces
	  */
	void		draw(bool opaque, TAnimationTime ellapsedTime);

	/// draw for pre-rotated meshs
	void				drawPreRotatedMeshs(bool opaque, TAnimationTime ellapsedTime);
	

	/// release the shapes used by this particle
	void				releaseShapes();


	/** Compute (optionnal) mesh colors.
	  * \param outVB		The destination VB.
	  * \param inVB			the vb of the current shape
	  * \param startIndex   Index of the mesh being processed
	  * \param toProcess    Number of meshs to process
	  */
	void	computeColors(CVertexBuffer &outVB, const CVertexBuffer &inVB, uint startIndex, uint toProcess, uint32 srcStep);

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void		resize(uint32 size);	
	
	/** Build the mesh data, if the 'touch' flag is set.
	  * \return true if the mesh could be found and match the requirement
	  */
	bool				update(void);

	/// make a vb for the prerotated mesh from a source vb
	CVertexBuffer	    &makePrerotatedVb(const CVertexBuffer &inVB, TAnimationTime ellapsedTime);
	
	/** A rendering pass. The pb block contains several duplication of the primitives of the original mesh, in order
      * to draw sevral of them at once
	  */
	struct CRdrPass
	{
		CMaterial			Mat;
		CMaterial			SourceMat;
		CPrimitiveBlock		Pb;		
	};

	/// A set of rendering pass.	
	typedef std::vector<CRdrPass> TRdrPassSet;

	/// a set of rendering pass, and the associated vertex buffer
	struct CMeshDisplay
	{
		TRdrPassSet   RdrPasses;
		CVertexBuffer VB;
	};
	
	void restoreMaterials();

	/// Setup a set of rendering passes.
	void	CPSConstraintMesh::setupRenderPasses(float date, TRdrPassSet &rdrPasses, bool opaque);

	/// Perform a set of rendering passes. The VB must have been activated in the driver before to call this
	void				doRenderPasses(IDriver *driver, uint numObj, TRdrPassSet &rdrPasses, bool opaque);	

	
	typedef NLMISC::CSmartPtr<IShape>		  PShape;
	typedef std::vector<std::string>		  TShapeNameVect;
	typedef std::vector<PShape>				  TShapeVect;	

	// name of the shapes
	TShapeNameVect _MeshShapeFileName;
	TShapeVect	   _Shapes;

	// caches the number of faces (for load balacing)
	uint _NumFaces;

	// the shape bank containing the shape
	CShapeBank  *_ModelBank;	

	
	/** This class manage sharing between several mesh displays.
	  * There can be a limited number of them at a given time.
	  */
	class CMeshDisplayShare
	{
		public:
			/// ctor giving the max number of CDipslayMesh structures to be kept simulaneously.
			CMeshDisplayShare(uint maxNumMD) : _MaxNumMD(maxNumMD) {}

			/// dtor
			~CMeshDisplayShare();

			/** Retrieve a display share from the given vertex format and shape			  
			  * \param format The format used with the vb. It must be the same than the input mesh, but it can also add a color component (for color fading)
			  */
			CMeshDisplay &getMeshDisplay(IShape *shape, uint32 format);
			
		protected:
			uint     _MaxNumMD;
			typedef  NLMISC::CSmartPtr<IShape> PShape;
			struct CKey
			{
				~CKey();
				PShape Shape;
				uint32 Format;
				bool operator == (const CKey &key) const { return Shape == key.Shape && Format == key.Format; }
				bool operator != (const CKey &key) const { return ! (*this == key); }
				bool operator <  (const CKey &key) const { return Shape < key.Shape || (Shape == key.Shape && Format < key.Format); }
			};				
			typedef std::map<CKey, CMeshDisplay *> TMDMap; // vb  sorted by their formats
			typedef std::queue<CKey> TMDQueue; // vb sorted by creation date
			TMDQueue MDQueue;
			TMDMap   MDMap;
			/// build a set of render pass from a mesh
			static void buildRdrPassSet(TRdrPassSet &dest, const IShape *src);
			/// Build a vb from a shape. The format can add an additionnal color
			static void buildVB(uint32 destFormat, CVertexBuffer &dest, const IShape *src); 
	};

	friend class CMeshDisplayShare;

	
	/// manage vertex buffers and primitive blocks used for rendering
	static CMeshDisplayShare		_MeshDisplayShare;	

	/// vertex buffer used with prerotated meshs
	static CVertexBuffer			_PreRotatedMeshVB;			  // mesh has no normals
	static CVertexBuffer			_PreRotatedMeshVBWithNormal;  // mesh has normals

	
	// we must store them for serialization
	float _MinAngularVelocity;
	float _MaxAngularVelocity;


	// use for rotation of precomputed meshs
	struct CPlaneBasisPair
	{		
		CPlaneBasis Basis;
		CVector		Axis; // an axis for rotation
		float		AngularVelocity; // an angular velocity
	};

	/// a set of precomp basis, before and after transfomation in world space, used if the hint 'RotateTheSame' has been called
	std::vector< CPlaneBasisPair > _PrecompBasis;

	/// this contain an index in _PrecompBasis for each particle
	std::vector<uint32> _IndexInPrecompBasis;

	/// fill _IndexInPrecompBasis with index in the range [0.. nb configurations[
	void fillIndexesInPrecompBasis(void);
	
	// release the model shape (dtor, or before loading)
	void clean(void);

	virtual CPSLocated *getSizeOwner(void) { return _Owner; }	
	virtual CPSLocated *getPlaneBasisOwner(void) { return _Owner; }

	/** Setup material so that global or per mesh color is taken in account. Useful if material hasn't been setup correctly in the export
	  */
	void setupMaterialColor(CMaterial &destMat, CMaterial &srcMat);

	/// A 'bitfield' to force some stage to be modulated with the primary color
	uint8   _ModulatedStages;

	// A new mesh has been set, so we must reconstruct it when needed	
	uint8	_Touched : 1;	
	// flags that indicate wether the object has transparent faces. When the 'touch' flag is set, it is undefined, until the next update() call.
	uint8	_HasTransparentFaces : 1;
	// flags that indicate wether the object has opaques faces. When the 'touch' flag is set, it is undefined, until the next update() call.
	uint8	_HasOpaqueFaces                   : 1;
	uint8   _VertexColorLightingForced        : 1;
	uint8   _GlobalAnimationEnabled           : 1;
	uint8   _ReinitGlobalAnimTimeOnNewElement : 1;
	uint8   _HasLightableFaces                : 1;

	
	/// Infos for global texture animation
	struct CGlobalTexAnims
	{
		CGlobalTexAnim		Anims[IDRV_MAT_MAXTEXTURES];		
		void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);		
	};

	typedef std::auto_ptr<CGlobalTexAnims> PGlobalTexAnims;
	PGlobalTexAnims						   _GlobalTexAnims;
	float								   _GlobalAnimDate;


	/// \name morphing
	//@{
		float					_MorphValue;
		CPSAttribMaker<float>	*_MorphScheme;
	//@}
private:
	CPSConstraintMesh(const CPSConstraintMesh &) { nlassert(0); /* not supported */ }
	CPSConstraintMesh &operator = (const CPSConstraintMesh &other) { nlassert(0); return *this; /* not supported */ }
}; 



} // NL3D


#endif // NL_PS_MESH_H

/* End of ps_mesh.h */
