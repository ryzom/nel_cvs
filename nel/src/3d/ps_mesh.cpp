/** \file ps_mesh.cpp
 * Particle meshs
 *
 * $Id: ps_mesh.cpp,v 1.22 2003/03/17 16:52:55 vizerie Exp $
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

#include "std3d.h"

#include "3d/ps_mesh.h"
#include "3d/ps_macro.h"
#include "3d/shape.h"
#include "3d/mesh.h"
#include "3d/transform_shape.h"
#include "3d/shape_bank.h"
#include "3d/texture_mem.h"
#include "3d/scene.h"
#include "3d/ps_located.h"
#include "3d/particle_system.h"
#include "3d/particle_system_shape.h"
#include "3d/ps_iterator.h"
#include "nel/misc/stream.h"
#include "nel/misc/path.h"

#include <memory>





namespace NL3D 
{

////////////////////
// static members //
////////////////////




CPSConstraintMesh::CMeshDisplayShare		CPSConstraintMesh::_MeshDisplayShare(16);	
CVertexBuffer								CPSConstraintMesh::_PreRotatedMeshVB;			  // mesh has no normals
CVertexBuffer								CPSConstraintMesh::_PreRotatedMeshVBWithNormal;  // mesh has normals



// this produce a random unit vector
static CVector MakeRandomUnitVect(void)	
{
	CVector v((float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  );
	v.normalize();
	return v;
}


////////////////////////////
// CPSMesh implementation //
////////////////////////////


//====================================================================================




const std::string DummyShapeName("dummy mesh shape");

/** a private function that create a dummy mesh :a cube with dummy textures
 */ 

static CMesh *CreateDummyShape(void)
{
	CMesh::CMeshBuild mb;
	CMeshBase::CMeshBaseBuild mbb;

	mb.VertexFlags = CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag;
	mb.Vertices.push_back(CVector(-.5f, -.5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, -.5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, -.5f, .5f));
	mb.Vertices.push_back(CVector(-.5f, -.5f, .5f));

	mb.Vertices.push_back(CVector(-.5f, .5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, .5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, .5f, .5f));
	mb.Vertices.push_back(CVector(-.5f, .5f, .5f));

	// index for each face
	uint32 tab[] = { 4, 1, 0,
					 4, 5, 1,
					 5, 2, 1,
					 5, 6, 2,
					 6, 3, 2,
					 6, 7, 3,
					 7, 0, 3,
					 7, 4, 0,
					 7, 5, 4,
					 7, 6, 5,
					 2, 0, 1,
					 2, 3, 0
					};

	for (uint k = 0; k < 6; ++k)
	{
		CMesh::CFace f;
		f.Corner[0].Vertex = tab[6 * k];
		f.Corner[0].Uvws[0] = NLMISC::CUVW(0, 0, 0);

		f.Corner[1].Vertex = tab[6 * k + 1];
		f.Corner[1].Uvws[0] = NLMISC::CUVW(1, 1, 0);

		f.Corner[2].Vertex = tab[6 * k + 2];
		f.Corner[2].Uvws[0] = NLMISC::CUVW(0, 1, 0);

		f.MaterialId = 0;

		mb.Faces.push_back(f);

		f.Corner[0].Vertex = tab[6 * k + 3];
		f.Corner[0].Uvws[0] = NLMISC::CUVW(0, 0, 0);

		f.Corner[1].Vertex = tab[6 * k + 4];
		f.Corner[1].Uvws[0] = NLMISC::CUVW(1, 0, 0);

		f.Corner[2].Vertex = tab[6 * k + 5];
		f.Corner[2].Uvws[0] = NLMISC::CUVW(1, 1, 0);

		f.MaterialId = 0;
		mb.Faces.push_back(f);		
	}

	CMaterial mat;
	CTextureMem *tex = new CTextureMem;
	tex->makeDummy();
	mat.setTexture(0, tex);
	mat.setLighting(false);
	mat.setColor(CRGBA::White);
	mbb.Materials.push_back(mat);
	CMesh *m = new CMesh;
	m->build(mbb, mb);
	return m;
} 


//====================================================================================
void CPSMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	(void)f.serialVersion(3);	
	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial(_Shape);
	if (f.isReading())
	{
		invalidate();
	}
}


//====================================================================================
uint32 CPSMesh::getMaxNumFaces(void) const
{
	/// we don't draw any face ! (the meshs are drawn by the scene)
	return 0;
}

//====================================================================================
bool CPSMesh::hasTransparentFaces(void)
{
	/// we don't draw any tri ! (the meshs are drawn by the scene)
	return false;
}

//====================================================================================
bool CPSMesh::hasOpaqueFaces(void)
{
	/// we don't draw any tri ! (the meshs are drawn by the scene)
	return false;
}

//====================================================================================
void CPSMesh::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newPlaneBasisElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);

	nlassert(_Owner);
	nlassert(_Owner->getOwner());

	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called
	//CTransformShape *instance = _Shape->createInstance(*scene);

	CTransformShape *instance = scene->createInstance(_Shape);

	if (!instance)
	{
		
		// mesh not found ...
		IShape *is = CreateDummyShape();
		scene->getShapeBank()->add(DummyShapeName, is);
		instance = scene->createInstance(DummyShapeName);
		nlassert(instance);
	}


	instance->setTransformMode(CTransform::DirectMatrix);

	instance->hide(); // the object hasn't the right matrix yet so we hide it. It'll be shown once it is computed
	nlassert(instance);

	_Instances.insert(instance);
}

//====================================================================================	
void CPSMesh::deleteElement(uint32 index)
{	
	deleteSizeElement(index);
	deleteAngle2DElement(index);
	deletePlaneBasisElement(index);

	// check wether CTransformShape have been instanciated
	if (_Invalidated) return;

	nlassert(_Owner);
	nlassert(_Owner->getOwner());

	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called

	scene->deleteInstance(_Instances[index]);
	_Instances.remove(index);
}

//====================================================================================
void CPSMesh::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt)
{
		if (pass == PSSolidRender)			
		{
			updatePos();
		}
		else 
		if (pass == PSToolRender) // edition mode only
		{			
			showTool();
		}

}

//====================================================================================
void CPSMesh::updatePos()
{
	const uint MeshBufSize = 512;
	PARTICLES_CHECK_MEM;
	nlassert(_Owner);
	const uint32 size = _Owner->getSize();
	if (!size) return;


	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


	if (_Invalidated)
	{
		// need to rebuild all the transform shapes
		nlassert(_Owner);
		nlassert(_Owner->getOwner());

		CScene *scene = _Owner->getScene();
		nlassert(scene); // the setScene method of the particle system should have been called
	

		resize(_Owner->getMaxSize());

		for (uint k = 0; k < size; ++k)
		{
			CTransformShape *instance = scene->createInstance(_Shape);
			instance->setTransformMode(CTransform::DirectMatrix);
			instance->hide();
			_Instances.insert(instance);
		}

		_Invalidated = false;
	}
	
	float sizes[MeshBufSize];
	float angles[MeshBufSize];
	static CPlaneBasis planeBasis[MeshBufSize];

	uint32 leftToDo = size, toProcess;


	float *ptCurrSize;
	const uint  ptCurrSizeIncrement = _SizeScheme ? 1 : 0;

	float *ptCurrAngle;
	const uint  ptCurrAngleIncrement = _Angle2DScheme ? 1 : 0;

	CPlaneBasis *ptBasis;
	const uint  ptCurrPlaneBasisIncrement = _PlaneBasisScheme ? 1 : 0;

	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt;


	TInstanceCont::iterator instanceIt = _Instances.begin();

	do
	{
		toProcess = leftToDo < MeshBufSize ? leftToDo : MeshBufSize;

		if (_SizeScheme)
		{
			ptCurrSize  = (float *) (_SizeScheme->make(_Owner, size - leftToDo, &sizes[0], sizeof(float), toProcess, true));			
		}
		else
		{
			ptCurrSize =& _ParticleSize;
		}

		if (_Angle2DScheme)
		{
			ptCurrAngle  = (float *) (_Angle2DScheme->make(_Owner, size - leftToDo, &angles[0], sizeof(float), toProcess, true));			
		}
		else
		{
			ptCurrAngle =& _Angle2D;
		}


		if (_PlaneBasisScheme)
		{
			ptBasis  = (CPlaneBasis *) (_PlaneBasisScheme->make(_Owner, size - leftToDo, &planeBasis[0], sizeof(CPlaneBasis), toProcess, true));			
		}
		else
		{
			ptBasis = &_PlaneBasis;
		}

		endPosIt = posIt + toProcess;		
		CMatrix mat, tmat;		

		// the matrix used to get in the right basis
		const CMatrix &transfo = _Owner->isInSystemBasis() ? /*_Owner->getOwner()->*/getSysMat() : CMatrix::Identity;
		do
		{
			(*instanceIt)->show();

			tmat.identity();
			mat.identity();

			tmat.translate(*posIt);

			

			mat.setRot( ptBasis->X * CPSUtil::getCos((sint32) *ptCurrAngle) + ptBasis->Y * CPSUtil::getSin((sint32) *ptCurrAngle)
						, ptBasis->X * CPSUtil::getCos((sint32) *ptCurrAngle + 64) + ptBasis->Y * CPSUtil::getSin((sint32) *ptCurrAngle + 64)
						, ptBasis->X ^ ptBasis->Y
					  );

			mat.scale(*ptCurrSize);			
			
			(*instanceIt)->setMatrix(transfo * tmat * mat);			

			++instanceIt;
			++posIt;
			ptCurrSize += ptCurrSizeIncrement;
			ptCurrAngle += ptCurrAngleIncrement;
			ptBasis += ptCurrPlaneBasisIncrement;
		}
		while (posIt != endPosIt);
		leftToDo -= toProcess;
	}
	while (leftToDo);

	PARTICLES_CHECK_MEM;
}

//====================================================================================
void CPSMesh::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	resizeSize(size);
	resizeAngle2D(size);
	resizePlaneBasis(size);
	_Instances.resize(size);
}


//====================================================================================
CPSMesh::~CPSMesh()
{	
	if (_Owner && _Owner->getOwner())
	{	
		CScene *scene = _Owner->getScene();
		nlassert(scene); // the setScene method of the particle system should have been called

		for (TInstanceCont::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
		{
			scene->deleteInstance(*it);
		}
	}
}

//////////////////////////////////////
// CPSConstraintMesh implementation //
//////////////////////////////////////

/// private : eval the number of triangles in a mesh
static uint getMeshNumTri(const CMesh &m)
{	
	uint numFaces = 0;
	for (uint k = 0; k < m.getNbMatrixBlock(); ++k)
	{
		for (uint l = 0; l  < m.getNbRdrPass(k); ++l)
		{
			const CPrimitiveBlock pb = m.getRdrPassPrimitiveBlock(k, l);
			numFaces += (pb.getNumLine() << 1) + pb.getNumTri() + (pb.getNumQuad() << 1);

		}
	}
	return numFaces;
}


//====================================================================================
/// private use : check if there are transparent and / or opaque faces in a mesh
static void CheckForOpaqueAndTransparentFacesInMesh(const CMesh &m, bool &hasTransparentFaces, bool &hasOpaqueFaces)
{
	hasTransparentFaces = false;
	hasOpaqueFaces = false;
	
	for (uint k = 0; k < m.getNbRdrPass(0); ++k)
	{
		const CMaterial &currMat = m.getMaterial(m.getRdrPassMaterial(0, k));
		if (!currMat.getZWrite())
		{
			hasTransparentFaces = true;
		}
		else // z-buffer write or no blending -> the face is opaque
		{
			hasOpaqueFaces = true;
		}
	}	 
}



/** Well, we could have put a method template in CPSConstraintMesh, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSConstraintMeshHelper
{
public:
	template <class T>	
	static void drawMeshs(T posIt, CPSConstraintMesh &m, uint size, uint32 srcStep, bool opaque)
	{
		CMesh				  &mesh	= * NLMISC::safe_cast<CMesh *>((IShape *) m._Shapes[0]);	
		const CVertexBuffer   &modelVb = mesh.getVertexBuffer();

		// size for model vertices
		const uint inVSize	  = modelVb.getVertexSize(); // vertex size				

		// driver setup
		IDriver *driver = m.getDriver();
		m.setupDriverModelMatrix();	

		// buffer to compute sizes
		float			sizes[ConstraintMeshBufSize];
		
		float *ptCurrSize;
		uint ptCurrSizeIncrement = m._SizeScheme ? 1 : 0;

		T endPosIt;
		uint leftToDo = size, toProcess;			
		
		/// get a vb in which to write. It has the same format than the input mesh, but can also have a color flag added
		CPSConstraintMesh::CMeshDisplay  &md= m._MeshDisplayShare.getMeshDisplay(m._Shapes[0], modelVb.getVertexFormat() 
																| (m._ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));

		m.setupRenderPasses((float) m._Owner->getOwner()->getSystemDate() - m._GlobalAnimDate, md.RdrPasses, opaque);

		CVertexBuffer &outVb = md.VB;
		const uint outVSize = outVb.getVertexSize();
		
		driver->activeVertexBuffer(outVb);	

		// we don't have precomputed mesh there ... so each mesh must be transformed, which is the worst case	
		CPlaneBasis planeBasis[ConstraintMeshBufSize];
		CPlaneBasis *ptBasis;
		uint ptBasisIncrement = m._PlaneBasisScheme ? 1 : 0;

		const uint nbVerticesInSource	= modelVb.getNumVertices();

		sint inNormalOff=0;
		sint outNormalOff=0;
		if (modelVb.getVertexFormat() & CVertexBuffer::NormalFlag)
		{	
			inNormalOff  =  modelVb.getNormalOff();
			outNormalOff =  outVb.getNormalOff();	
		}
		
		do
		{
			uint8 *outVertex = (uint8 *) outVb.getVertexCoordPointer();		

			toProcess = std::min(leftToDo, ConstraintMeshBufSize);

			if (m._SizeScheme)
			{
				ptCurrSize  = (float *) (m._SizeScheme->make(m._Owner, size -leftToDo, &sizes[0], sizeof(float), toProcess, true, srcStep));				
			}
			else
			{
				ptCurrSize = &m._ParticleSize;
			}

			if (m._PlaneBasisScheme)
			{
				ptBasis = (CPlaneBasis *) (m._PlaneBasisScheme->make(m._Owner, size -leftToDo, &planeBasis[0], sizeof(CPlaneBasis), toProcess, true, srcStep));
			}
			else
			{
				ptBasis = &m._PlaneBasis;
			}
			

			endPosIt = posIt + toProcess;
			// transfo matrix & scaled transfo matrix;
			CMatrix  M, sM;

		
			if (m._Shapes.size() == 1)
			{
				/// unmorphed case
				do
				{

					uint8 *inVertex = (uint8 *) modelVb.getVertexCoordPointer();
					uint k = nbVerticesInSource;

					// do we need a normal ?
					if (modelVb.getVertexFormat() & CVertexBuffer::NormalFlag)
					{
						M.identity();
						M.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
						sM = M;
						sM.scale(*ptCurrSize);

						// offset of normals in the prerotated mesh				
						do
						{
							CHECK_VERTEX_BUFFER(modelVb, inVertex);	
							CHECK_VERTEX_BUFFER(outVb,	  outVertex);	
							CHECK_VERTEX_BUFFER(modelVb, inVertex + inNormalOff);	
							CHECK_VERTEX_BUFFER(outVb,	  outVertex + outNormalOff);	

							// translate and resize the vertex (relatively to the mesh origin)
							*(CVector *) outVertex = *posIt + sM * *(CVector *) inVertex;										
							// copy the normal
							*(CVector *) (outVertex + outNormalOff) = M * *(CVector *) (inVertex + inNormalOff);
							

							inVertex  += inVSize;
							outVertex += outVSize;
						}
						while (--k);
					}
					else
					{
						// no normal to transform
						sM.identity();
						sM.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
						sM.scale(*ptCurrSize);

						do
						{
							CHECK_VERTEX_BUFFER(modelVb, inVertex);
							CHECK_VERTEX_BUFFER(outVb, outVertex);

							// translate and resize the vertex (relatively to the mesh origin)
							*(CVector *) outVertex = *posIt + sM * *(CVector *) inVertex;				

							inVertex  += inVSize;
							outVertex += outVSize;
						}
						while (--k);
					}

					
					++posIt;
					ptCurrSize += ptCurrSizeIncrement;
					ptBasis += ptBasisIncrement;
				}
				while (posIt != endPosIt);
			}
			else
			{
				// morphed case
				
				// first, compute the morph value for each mesh
				float	morphValues[ConstraintMeshBufSize];
				float	*currMorphValue;
				uint	morphValueIncr;

				if (m._MorphScheme) // variable case
				{
					currMorphValue = (float *) m._MorphScheme->make(m._Owner, size - leftToDo, &morphValues[0], sizeof(float), toProcess, true, srcStep);
					morphValueIncr  = 1;
				}
				else /// constant case
				{
					currMorphValue = &m._MorphValue;
					morphValueIncr  = 0;
				}

				do
				{
					const uint numShapes = m._Shapes.size();
					const uint8 *m0, *m1;
					float lambda;
					float opLambda;
					const CVertexBuffer *inVB0, *inVB1;
					if (*currMorphValue >= numShapes - 1)
					{
						lambda = 0.f;
						opLambda = 1.f;
						inVB0 = inVB1 = &NLMISC::safe_cast<CMesh *>((IShape *) m._Shapes[numShapes - 1])->getVertexBuffer();
					}
					else if (*currMorphValue <= 0)
					{
						lambda = 0.f;
						opLambda = 1.f;
						inVB0 = inVB1 = &NLMISC::safe_cast<CMesh *>((IShape *) m._Shapes[0])->getVertexBuffer();
					}
					else
					{
						uint iMeshIndex = (uint) *currMorphValue;
						lambda = *currMorphValue - iMeshIndex;
						opLambda = 1.f - lambda;
						inVB0 = &NLMISC::safe_cast<CMesh *>((IShape *) m._Shapes[iMeshIndex])->getVertexBuffer();
						inVB1 = &NLMISC::safe_cast<CMesh *>((IShape *) m._Shapes[iMeshIndex + 1])->getVertexBuffer();
					}

					m0 = (uint8 *) inVB0->getVertexCoordPointer();
					m1 = (uint8 *) inVB1->getVertexCoordPointer();

							
					uint k = nbVerticesInSource;
					// do we need a normal ?
					if (modelVb.getVertexFormat() & CVertexBuffer::NormalFlag)
					{
						M.identity();
						M.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
						sM = M;
						sM.scale(*ptCurrSize);

						// offset of normals in the prerotated mesh				
						do
						{
							CHECK_VERTEX_BUFFER((*inVB0),	  m0);							
							CHECK_VERTEX_BUFFER((*inVB1),	  m1);	
							CHECK_VERTEX_BUFFER((*inVB0),	  m0 + inNormalOff);			
							CHECK_VERTEX_BUFFER((*inVB1),	  m1 + inNormalOff);
							CHECK_VERTEX_BUFFER(outVb,	  outVertex);							
							CHECK_VERTEX_BUFFER(outVb,	  outVertex + outNormalOff);	

							// morph, and transform the vertex
							*(CVector *) outVertex = *posIt + sM * (opLambda * *(CVector *) m0 + lambda * *(CVector *) m1);
							// morph, and transform the normal
							*(CVector *) (outVertex + outNormalOff) = M * (opLambda * *(CVector *) (m0 + inNormalOff)
																		  + lambda * *(CVector *) (m1 + inNormalOff)).normed();
							

							m0  += inVSize;
							m1  += inVSize;
							outVertex += outVSize;
						}
						while (--k);
					}
					else
					{
						// no normal to transform
						sM.identity();
						sM.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
						sM.scale(*ptCurrSize);

						do
						{			
							CHECK_VERTEX_BUFFER((*inVB0),	  m0);							
							CHECK_VERTEX_BUFFER((*inVB1),	  m1);		
							CHECK_VERTEX_BUFFER(outVb, outVertex);
							// morph, and transform the vertex
							*(CVector *) outVertex = *posIt + sM * (opLambda * *(CVector *) m0 + opLambda * *(CVector *) m1);

							m0  += inVSize;
							m1  += inVSize;
							outVertex += outVSize;
						}
						while (--k);
					}

					
					++posIt;
					ptCurrSize += ptCurrSizeIncrement;
					ptBasis += ptBasisIncrement;
					currMorphValue += morphValueIncr;
				}
				while (posIt != endPosIt);
			}		

			// compute colors if needed
			if (m._ColorScheme)
			{
				m.computeColors(outVb, modelVb, size - leftToDo, toProcess, srcStep);
			}
			
			// render meshs
			m.doRenderPasses(driver, toProcess, md.RdrPasses, opaque);
			leftToDo -= toProcess;

		}
		while (leftToDo);
	}


	template <class T, class U>	
	static void drawPrerotatedMeshs(T posIt,
								    U indexIt,
									CPSConstraintMesh &m,
									uint size,
									uint32 srcStep,
									bool opaque,
									TAnimationTime ellapsedTime)
	{
		// get the vb from the original mesh
		CMesh				  &mesh	= * NLMISC::safe_cast<CMesh *>((IShape *) m._Shapes[0]);	
		const CVertexBuffer   &modelVb = mesh.getVertexBuffer();

		/// precompute rotation in a VB from the src mesh
		CVertexBuffer &prerotVb  = m.makePrerotatedVb(modelVb, ellapsedTime);	

		// driver setup
		IDriver *driver = m.getDriver();
		m.setupDriverModelMatrix();	

		// renderPasses setup
		nlassert(m._Owner)	

		// storage for sizes of meshs
		float sizes[ConstraintMeshBufSize];
		
		// point the size for the current mesh
		float *ptCurrSize;
		uint ptCurrSizeIncrement = m._SizeScheme ? 1 : 0;

		T endPosIt;
		uint leftToDo = size, toProcess;
		const uint nbVerticesInSource = modelVb.getNumVertices();
		


		// size of a complete prerotated model
		const uint prerotatedModelSize = prerotVb.getVertexSize() * modelVb.getNumVertices();

		/// get a mesh display struct on this shape, with eventually a primary color added.
		CPSConstraintMesh::CMeshDisplay  &md    = m._MeshDisplayShare.getMeshDisplay(m._Shapes[0], modelVb.getVertexFormat() 
																| (m._ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));


		m.setupRenderPasses((float) m._Owner->getOwner()->getSystemDate() - m._GlobalAnimDate, md.RdrPasses, opaque);

		CVertexBuffer &outVb = md.VB;

		driver->activeVertexBuffer(outVb);


		// size of vertices in prerotated model
		const uint inVSize = prerotVb.getVertexSize();

		// size ofr vertices in dest vb
		const uint outVSize = outVb.getVertexSize();

		// offset of normals in vertices of the prerotated model, and source model		
		uint normalOff=0;
		uint pNormalOff=0;
		if (prerotVb.getVertexFormat() & CVertexBuffer::NormalFlag) 
		{
			normalOff  =  outVb.getNormalOff();
			pNormalOff =  prerotVb.getNormalOff();			
		}
					
		do
		{			
			toProcess = std::min(leftToDo, ConstraintMeshBufSize);

			if (m._SizeScheme)
			{
				// compute size
				ptCurrSize = (float *) (m._SizeScheme->make(m._Owner, size - leftToDo, &sizes[0], sizeof(float), toProcess, true, srcStep));				
			}
			else
			{
				// pointer on constant size
				ptCurrSize = &m._ParticleSize;
			}

			endPosIt = posIt + toProcess;
			uint8 *outVertex  = (uint8 *) outVb.getVertexCoordPointer();
			/// copy datas for several mesh
			do
			{
				uint8 *inVertex = (uint8 *) prerotVb.getVertexCoordPointer() + prerotatedModelSize * *indexIt; // prerotated vertex			
				uint k = nbVerticesInSource;
				
				if (prerotVb.getVertexFormat() & CVertexBuffer::NormalFlag) // has it a normal ?
				{
					do
					{
						CHECK_VERTEX_BUFFER(outVb, outVertex);
						CHECK_VERTEX_BUFFER(prerotVb, inVertex);
						CHECK_VERTEX_BUFFER(outVb, outVertex + normalOff);
						CHECK_VERTEX_BUFFER(prerotVb, inVertex + pNormalOff);


						// translate and resize the vertex (relatively to the mesh origin)
						*(CVector *)  outVertex						 = *posIt + *ptCurrSize * *(CVector *) inVertex;
						// copy the normal
						*(CVector *)  (outVertex + normalOff ) = *(CVector *) (inVertex + pNormalOff);
						inVertex  += inVSize;
						outVertex += outVSize;
					}
					while (--k);
				}
				else
				{
					do
					{					
						// translate and resize the vertex (relatively to the mesh origin)
						CHECK_VERTEX_BUFFER(outVb, outVertex);
						CHECK_VERTEX_BUFFER(prerotVb, inVertex);
						*(CVector *)  outVertex = *posIt + *ptCurrSize * *(CVector *) inVertex;													
						inVertex  += inVSize;
						outVertex += outVSize;
					}
					while (--k);
				}
				
				++indexIt;
				++posIt;
				ptCurrSize += ptCurrSizeIncrement;
			}
			while (posIt != endPosIt);

			// compute colors if needed
			if (m._ColorScheme)
			{
				m.computeColors(outVb, modelVb, size - leftToDo, toProcess, srcStep);
			}
					

			/// render the result
			m.doRenderPasses(driver, toProcess, md.RdrPasses, opaque);
			leftToDo -= toProcess;

		}
		while (leftToDo);
		PARTICLES_CHECK_MEM
	}
};

CPSConstraintMesh::CPSConstraintMesh() : _NumFaces(0),
										 _ModelBank(NULL),
										 _ModulatedStages(0),
										 _Touched(1),
										 _VertexColorLightingForced(false),
										 _GlobalAnimationEnabled(0),
										 _ReinitGlobalAnimTimeOnNewElement(0),
										 _MorphValue(0),
										 _MorphScheme(NULL)
{		
	_Name = std::string("ConstraintMesh");
}

//====================================================================================
uint32 CPSConstraintMesh::getMaxNumFaces(void) const
{
//	nlassert(_ModelVb);
	return _NumFaces * _Owner->getMaxSize();
	
}


//====================================================================================
bool CPSConstraintMesh::hasTransparentFaces(void)
{
	if (!_Touched) return _HasTransparentFaces != 0;
	/// we must update the mesh to know wether it has transparent faces
	update();
	return _HasTransparentFaces != 0;
}

//====================================================================================
bool CPSConstraintMesh::hasOpaqueFaces(void)
{
	if (!_Touched) return _HasOpaqueFaces != 0;	
	update();
	return _HasOpaqueFaces != 0;
}

//====================================================================================
void CPSConstraintMesh::setShape(const std::string &meshFileName)
{		
	_MeshShapeFileName.resize(1);	
	_MeshShapeFileName[0] = meshFileName;
	_Touched = 1;
}


//===========================================================================
std::string			CPSConstraintMesh::getShape(void) const
{
	if (_Touched) 
	{	
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	nlassert(_MeshShapeFileName.size() == 1);
	return _MeshShapeFileName[0];
}

//====================================================================================
void		CPSConstraintMesh::setShapes(const std::string *shapesNames, uint numShapes)
{
	_MeshShapeFileName.resize(numShapes);
	std::copy(shapesNames, shapesNames + numShapes, _MeshShapeFileName.begin());
	_Touched = 1;
}

//====================================================================================
uint	    CPSConstraintMesh::getNumShapes() const
{
	if (_Touched)
	{	
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	return _MeshShapeFileName.size();
}

//====================================================================================
void	CPSConstraintMesh::getShapesNames(std::string *shapesNames) const
{
	if (_Touched) 
	{	
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	std::copy(_MeshShapeFileName.begin(), _MeshShapeFileName.end(), shapesNames);
}



//====================================================================================
void		CPSConstraintMesh::setShape(uint index, const std::string &shapeName)
{
	nlassert(index < _MeshShapeFileName.size());
	_MeshShapeFileName[index] = shapeName;
	_Touched = 1;
}

	
//====================================================================================
const std::string          &CPSConstraintMesh::getShape(uint index) const
{
	if (_Touched) 
	{	
		const_cast<CPSConstraintMesh *>(this)->update();
	}
	nlassert(index < _MeshShapeFileName.size());
	return _MeshShapeFileName[index];
}



//====================================================================================
void	CPSConstraintMesh::setMorphValue(float value)
{
	delete _MorphScheme;
	_MorphScheme = NULL;
	_MorphValue = value;
}


//====================================================================================
float	CPSConstraintMesh::getMorphValue() const
{
	return _MorphValue;
}

//====================================================================================
void	CPSConstraintMesh::setMorphScheme(CPSAttribMaker<float> *scheme)
{
	delete _MorphScheme;
	_MorphScheme = scheme;
	if (_MorphScheme->hasMemory()) _MorphScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
}

//====================================================================================
CPSAttribMaker<float>		*CPSConstraintMesh::getMorphScheme()
{
	return _MorphScheme;
}

//====================================================================================
const CPSAttribMaker<float>	*CPSConstraintMesh::getMorphScheme() const
{
	return _MorphScheme;
}


//====================================================================================
static IShape *GetDummyShapeFromBank(CShapeBank &sb)
{	
	static const std::string dummyMeshName("dummy constraint mesh shape");
	if (sb.isPresent(dummyMeshName) == CShapeBank::Present)
	{				
		return sb.addRef(dummyMeshName);
	}
	else
	{
		// no dummy shape created -> add one to the bank
		IShape *is = CreateDummyShape();
		sb.add(std::string("dummy constraint mesh shape"), is);
		return is;
	}
}

//====================================================================================
bool CPSConstraintMesh::update(void)
{		
	bool ok = true;
	if (!_Touched) return ok;

	clean();
	
	nlassert(_Owner->getScene());

	CScene *scene = _Owner->getScene();
	CShapeBank *sb = scene->getShapeBank();
	IShape *is;


	uint32 vFormat=0;
	uint   numVerts=0;	

	if (_MeshShapeFileName.size() == 0)
	{
		_MeshShapeFileName.resize(1);
		_MeshShapeFileName[0] = DummyShapeName;		
	}
	

	_Shapes.resize(_MeshShapeFileName.size());	
	for (uint k = 0; k < _MeshShapeFileName.size(); ++k)
	{
		if (sb->isPresent(_MeshShapeFileName[k]) == CShapeBank::Present)
		{
			_Shapes[k] = sb->addRef(_MeshShapeFileName[k]);

			/// get  the mesh format, or check that is was the same that previous shapes ' one
			const CMesh &m  = * NLMISC::safe_cast<CMesh *>((IShape *) _Shapes[k]); // only mesh shape's can be used with this class!
			if (k == 0)
			{			
				vFormat = m.getVertexBuffer().getVertexFormat();
				numVerts =  m.getVertexBuffer().getNumVertices();
			}
			else
			{
				if (vFormat != m.getVertexBuffer().getVertexFormat()
					|| numVerts != m.getVertexBuffer().getNumVertices())
				{
					ok = false;
				}
			}
		}
		else
		{
			try
			{
				sb->load(_MeshShapeFileName[k]);
			}	
			catch (NLMISC::EPathNotFound &)
			{
				nlwarning("mesh not found : %s; used as a constraint mesh particle", _MeshShapeFileName[k].c_str());
				// shape not found, so not present in the shape bank -> we create a dummy shape
			}

			if (sb->isPresent(_MeshShapeFileName[k]) != CShapeBank::Present)
			{					
				ok = false;
			}
			else
			{
				is = sb->addRef(_MeshShapeFileName[k]);
				if (!dynamic_cast<CMesh *>(is)) // is it a mesh
				{
					nlwarning("Tried to bind a shape that is not a mesh to a mesh particle : %s", _MeshShapeFileName[k].c_str());
					sb->release(is);					
					ok = false;
				}
				else
				{
					const CMesh &m  = * NLMISC::safe_cast<CMesh *>(is);
					/// make sure there are not too many vertices
					if (m.getVertexBuffer().getNumVertices() > ConstraintMeshMaxNumVerts)
					{
						nlwarning("Tried to bind a mesh that has more than %d vertices to a particle mesh: %s", (int) ConstraintMeshMaxNumVerts, _MeshShapeFileName[k].c_str());
						sb->release(is);						
						ok = false;
					}
					else
					{
						_Shapes[k] = is;
						/// get  the mesh format, or check that is was the same that previous shapes ' one
						const CMesh &m  = * NLMISC::safe_cast<CMesh *>((IShape *) _Shapes[k]); // only mesh shape's can be used with this class!
						if (k == 0)
						{			
							vFormat = m.getVertexBuffer().getVertexFormat();
							numVerts =  m.getVertexBuffer().getNumVertices();
						}
						else
						{
							if (vFormat != m.getVertexBuffer().getVertexFormat()
								|| numVerts != m.getVertexBuffer().getNumVertices())
							{
								ok = false;
							}
						}
					}
				}
			}
		}
		
		if (!ok)
		{
			releaseShapes();
			_Shapes.resize(1);
			_Shapes[0] = GetDummyShapeFromBank(*sb);
			break;
		}
	}


	const CMesh &m  = * NLMISC::safe_cast<CMesh *>((IShape *) _Shapes[0]); // only mesh shape's can be used with this class!

	/// update the number of faces
	_NumFaces = getMeshNumTri(m);
	notifyOwnerMaxNumFacesChanged();
	
	/// update opacity / transparency state
	bool hasTransparentFaces, hasOpaqueFaces;
	CheckForOpaqueAndTransparentFacesInMesh(m, hasTransparentFaces, hasOpaqueFaces);
	_HasTransparentFaces = hasTransparentFaces;	
	_HasOpaqueFaces = hasOpaqueFaces;
	_ModelBank = sb;	
	_GlobalAnimDate = _Owner->getOwner()->getSystemDate();
	_Touched = 0;
	nlassert(_Shapes.size() > 0);

	#ifdef NL_DEBUG
		for (uint j = 0; j < _Shapes.size(); ++j)
		{
			nlassert(dynamic_cast<CMesh *>((IShape *) _Shapes[j]));
		}
	#endif

	return ok;
	
}



//====================================================================================
void CPSConstraintMesh::hintRotateTheSame(uint32 nbConfiguration,
										  float minAngularVelocity,
										  float maxAngularVelocity
										)
{
	nlassert(nbConfiguration <= ConstraintMeshMaxNumPrerotatedModels);

	// TODO : avoid code duplication with CPSFace ...
	_MinAngularVelocity = minAngularVelocity;
	_MaxAngularVelocity = maxAngularVelocity;



	_PrecompBasis.resize(nbConfiguration);

	if (nbConfiguration)
	{
		// each precomp basis is created randomly;
		for (uint k = 0; k < nbConfiguration; ++k)
		{
			 CVector v = MakeRandomUnitVect();
			_PrecompBasis[k].Basis = CPlaneBasis(v);
			_PrecompBasis[k].Axis = MakeRandomUnitVect();
			_PrecompBasis[k].AngularVelocity = minAngularVelocity 
											   + (rand() % 20000) / 20000.f * (maxAngularVelocity - minAngularVelocity);

		}	

		// we need to do this because nbConfs may have changed
		fillIndexesInPrecompBasis();
	}	
}


//====================================================================================
void CPSConstraintMesh::fillIndexesInPrecompBasis(void)
{
	// TODO : avoid code duplication with CPSFace ...
	const uint32 nbConf = _PrecompBasis.size();
	if (_Owner)
	{
		_IndexInPrecompBasis.resize( _Owner->getMaxSize() );
	}	
	for (std::vector<uint32>::iterator it = _IndexInPrecompBasis.begin(); it != _IndexInPrecompBasis.end(); ++it)
	{
		*it = rand() % nbConf;
	}
}

//====================================================================================	
/// serialisation. Derivers must override this, and call their parent version
void CPSConstraintMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	sint ver = f.serialVersion(4);
	if (f.isReading())
	{
		clean();
	}

	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);

	// prerotations ...

	if (f.isReading())
	{
		uint32 nbConfigurations;
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
		hintRotateTheSame(nbConfigurations, _MinAngularVelocity, _MaxAngularVelocity);	
	}
	else	
	{				
		uint32 nbConfigurations = _PrecompBasis.size();
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
	}

	// saves the model file name, or an empty string if nothing has been set	
	static std::string emptyStr;

	if (ver < 4) // early version : no morphing support
	{
		if (!f.isReading())
		{
			if (_MeshShapeFileName.size() > 0)
			{
				f.serial(_MeshShapeFileName[0]);			
			}
			else
			{
				f.serial(emptyStr); 
			}
		}
		else
		{	
			_MeshShapeFileName.resize(1);				
			f.serial(_MeshShapeFileName[0]);
			_Touched = true;			
		}
	}

	if (ver > 1)
	{
		CPSColoredParticle::serialColorScheme(f);
		f.serial(_ModulatedStages);
		if (f.isReading())
		{
			bool vcEnabled;
			f.serial(vcEnabled);
			_VertexColorLightingForced = vcEnabled;
		}
		else
		{
			bool vcEnabled = (_VertexColorLightingForced != 0);
			f.serial(vcEnabled);
		}						
	}

	if (ver > 2) // texture animation
	{
		if (f.isReading())
		{
			bool gaEnabled;
			f.serial(gaEnabled);
			_GlobalAnimationEnabled = gaEnabled;
			if (gaEnabled)
			{
				PGlobalTexAnims newPtr(new CGlobalTexAnims); // create new
				std::swap(_GlobalTexAnims, newPtr);			 // replace old
				f.serial(*_GlobalTexAnims);
			}

			bool rgt;
			f.serial(rgt);
			_ReinitGlobalAnimTimeOnNewElement = rgt;
		}
		else
		{
			bool gaEnabled = (_GlobalAnimationEnabled != 0);
			f.serial(gaEnabled);
			if (gaEnabled)
			{
				f.serial(*_GlobalTexAnims);
			}

			bool rgt = _ReinitGlobalAnimTimeOnNewElement != 0;
			f.serial(rgt);			
		}
	}

	if (ver > 3) // mesh morphing
	{
		f.serialCont(_MeshShapeFileName);
		bool useScheme;
		if (f.isReading())
		{		
			delete _MorphScheme;
		}
		else
		{
			useScheme = _MorphScheme != NULL;
		}
		f.serial(useScheme);
		if (useScheme)
		{				
			f.serialPolyPtr(_MorphScheme);
		}
		else
		{
			f.serial(_MorphValue);
		}		
	}
}

//====================================================================================
CPSConstraintMesh::~CPSConstraintMesh() 
{
	clean();
	delete _MorphScheme;
}



//====================================================================================
void CPSConstraintMesh::releaseShapes()
{
	for (TShapeVect::iterator it = _Shapes.begin(); it != _Shapes.end(); ++it)
	{
		if (*it)
		{
			_ModelBank->release(*it);
		}
	}
	_Shapes.clear();
}

//====================================================================================
void CPSConstraintMesh::clean(void)
{	
	if (_ModelBank)
	{	
		releaseShapes();		
	}
}


//====================================================================================
CVertexBuffer &CPSConstraintMesh::makePrerotatedVb(const CVertexBuffer &inVb, TAnimationTime ellapsedTime)
{
	// get a VB that has positions and eventually normals
	CVertexBuffer &prerotatedVb = inVb.getVertexFormat() & CVertexBuffer::NormalFlag ? _PreRotatedMeshVBWithNormal : _PreRotatedMeshVB;

	// size of vertices for source VB
	const uint vSize = inVb.getVertexSize();

	// size for vertices in prerotated model
	const uint vpSize = prerotatedVb.getVertexSize();
	

	// offset of normals in vertices of the prerotated model, and source model		
	uint normalOff=0;
	uint pNormalOff=0;
	if (prerotatedVb.getVertexFormat() & CVertexBuffer::NormalFlag) 
	{
		normalOff  =  inVb.getNormalOff();
		pNormalOff =  prerotatedVb.getNormalOff();			
	}

	const uint nbVerticesInSource	= inVb.getNumVertices();
		

	// rotate basis
	// and compute the set of prerotated meshs that will then duplicated (with scale and translation) to create the Vb of what must be drawn
	uint8 *outVertex = (uint8 *) prerotatedVb.getVertexCoordPointer();
	for (std::vector< CPlaneBasisPair >::iterator it = _PrecompBasis.begin(); it != _PrecompBasis.end(); ++it)
	{
		// not optimized at all, but this will apply to very few elements anyway...
		CMatrix mat;
		mat.rotate(CQuat(it->Axis, ellapsedTime * it->AngularVelocity));
		CVector n = mat * it->Basis.getNormal();
		it->Basis = CPlaneBasis(n);
	
		mat.identity();
		mat.setRot(it->Basis.X, it->Basis.Y, it->Basis.X ^ it->Basis.Y);

		uint8 *inVertex = (uint8 *) inVb.getVertexCoordPointer();

		uint k = nbVerticesInSource;

		// check wether we need to rotate normals as well...
		if (inVb.getVertexFormat() & CVertexBuffer::NormalFlag)
		{
		
			do
			{
				CHECK_VERTEX_BUFFER(inVb, inVertex);
				CHECK_VERTEX_BUFFER(inVb, inVertex + normalOff);
				CHECK_VERTEX_BUFFER(prerotatedVb, outVertex);
				CHECK_VERTEX_BUFFER(prerotatedVb, outVertex + pNormalOff);

				* (CVector *) outVertex =  mat.mulVector(* (CVector *) inVertex);
				* (CVector *) (outVertex + normalOff) =  mat.mulVector(* (CVector *) (inVertex + pNormalOff) );
				outVertex += vpSize;
				inVertex  += vSize;
				
			}
			while (--k);		
		}
		else
		{
			// no normal included
			do
			{	
				
				CHECK_VERTEX_BUFFER(prerotatedVb, outVertex);	
				CHECK_VERTEX_BUFFER(inVb, inVertex);					

				* (CVector *) outVertex =  mat.mulVector(* (CVector *) inVertex);
				outVertex += vpSize;
				inVertex += vSize;
			}
			while (--k);	

		}
	}
	return prerotatedVb;
}


//====================================================================================
void CPSConstraintMesh::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt)
{
		if (
			(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
			)
		{
			draw(pass == PSSolidRender, ellapsedTime);
		}
		else 
		if (pass == PSToolRender) // edition mode only
		{			
			showTool();
		}
}

//====================================================================================
void CPSConstraintMesh::draw(bool opaque, TAnimationTime ellapsedTime)
{
	PARTICLES_CHECK_MEM;
	nlassert(_Owner);	
	
	update(); // update mesh datas if needed
	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;
	_Owner->incrementNbDrawnParticles(numToProcess); // for benchmark purpose	

	
	if (_PrecompBasis.size() == 0) /// do we deal with prerotated meshs ?
	{
		if (step == (1 << 16))
		{
			CPSConstraintMeshHelper::drawMeshs(_Owner->getPos().begin(),					   
											  *this,
						                      numToProcess,
											  step,
											  opaque
											 );
		}
		else
		{	
			CPSConstraintMeshHelper::drawMeshs(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
											  *this,
						                      numToProcess,
											  step,
											  opaque
											 );		
		}
	}
	else
	{
		if (step == (1 << 16))
		{
			CPSConstraintMeshHelper::drawPrerotatedMeshs(_Owner->getPos().begin(),
													     _IndexInPrecompBasis.begin(),
														 *this,
														 numToProcess,
														 step,
														 opaque,
														 ellapsedTime
													    );
		}
		else
		{	
			typedef CAdvance1616Iterator<std::vector<uint32>::const_iterator, uint32> TIndexIterator;
			CPSConstraintMeshHelper::drawPrerotatedMeshs(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
														 TIndexIterator(_IndexInPrecompBasis.begin(), 0, step),
														 *this,
														 numToProcess,
														 step,
														 opaque,
														 ellapsedTime
													    );		
		}
	}
	
		
}

//====================================================================================
// Private func used to force modulation on a material and to store the preious state
static inline void ForceMaterialModulation(CMaterial &destMat, CMaterial &srcMat, uint8 modulatedStages)
{
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{		
		if (modulatedStages & (1 << k))
		{
			destMat.texEnvArg0RGB(k, CMaterial::Previous, CMaterial::SrcColor);
			destMat.texEnvArg0Alpha(k, CMaterial::Previous, CMaterial::SrcAlpha);
			destMat.texEnvArg1RGB(k, CMaterial::Constant, CMaterial::SrcColor);
			destMat.texEnvArg1Alpha(k, CMaterial::Constant, CMaterial::SrcAlpha);
			destMat.texEnvOpRGB(k, CMaterial::Modulate);
			destMat.texEnvOpAlpha(k, CMaterial::Modulate);			
		}
		else // restore from source material
		{
			destMat.setTexEnvMode(k, srcMat.getTexEnvMode(k));
		}
	}	
}


//====================================================================================
void	CPSConstraintMesh::setupRenderPasses(float date, TRdrPassSet &rdrPasses, bool opaque)
{
	// render meshs : we process each rendering pass
	for (TRdrPassSet::iterator rdrPassIt = rdrPasses.begin() 
		; rdrPassIt != rdrPasses.end(); ++rdrPassIt)
	{

		CMaterial &Mat = rdrPassIt->Mat;
		CMaterial &SourceMat = rdrPassIt->SourceMat;


		/// check wether this material has to be rendered
		if ((opaque && Mat.getZWrite()) || (!opaque && ! Mat.getZWrite()))
		{		

	
			// has to setup material constant color ?
			// global color not supported for mesh
		/*	CParticleSystem &ps = *(_Owner->getOwner());
			if (!_ColorScheme) 
			{								
				NLMISC::CRGBA col;
				col.modulateFromColor(SourceMat.getColor(), _Color);
				if (ps.getColorAttenuationScheme() == NULL)
				{
					col.modulateFromColor(col, ps.getGlobalColor());
				}
				Mat.setColor(col);				
			}
			else
			{				
				Mat.setColor(ps.getGlobalColor());
			}*/

			/// force modulation for some stages
			ForceMaterialModulation(Mat, SourceMat, _ModulatedStages);

			/// force vertex lighting
			bool forceVertexcolorLighting = _VertexColorLightingForced != 0 ? true : SourceMat.getLightedVertexColor();
			if (forceVertexcolorLighting != Mat.getLightedVertexColor()) // avoid to touch mat if not needed
			{
				Mat.setLightedVertexColor(forceVertexcolorLighting);
			}

			///global texture animation
			if (_GlobalAnimationEnabled != 0)
			{
				for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
				{
					if (Mat.getTexture(k) != NULL)
					{
						Mat.enableUserTexMat(k, true);
						CMatrix mat;
						_GlobalTexAnims->Anims[k].buildMatrix(date, mat);
						Mat.setUserTexMat(k ,mat);
					}
				}
			}		
		}			
	}

}

//====================================================================================
void	CPSConstraintMesh::doRenderPasses(IDriver *driver, uint numObj, TRdrPassSet &rdrPasses, bool opaque)
{		
	// render meshs : we process each rendering pass
	for (TRdrPassSet::iterator rdrPassIt = rdrPasses.begin() 
		; rdrPassIt != rdrPasses.end(); ++rdrPassIt)
	{	
		CMaterial &Mat = rdrPassIt->Mat;
		if ((opaque && Mat.getZWrite()) || (!opaque && ! Mat.getZWrite()))
		{
			/// setup number of primitives to be rendered
			rdrPassIt->Pb.setNumTri(rdrPassIt->Pb.capacityTri()   * numObj / ConstraintMeshBufSize);
			rdrPassIt->Pb.setNumQuad(rdrPassIt->Pb.capacityQuad() * numObj / ConstraintMeshBufSize);
			rdrPassIt->Pb.setNumLine(rdrPassIt->Pb.capacityLine() * numObj / ConstraintMeshBufSize);

			/// render the primitives
			driver->render(rdrPassIt->Pb, rdrPassIt->Mat);
		}
	}

}


//====================================================================================
void	CPSConstraintMesh::computeColors(CVertexBuffer &outVB, const CVertexBuffer &inVB, uint startIndex, uint toProcess, uint32 srcStep)
{	
	nlassert(_ColorScheme);
	// there are 2 case : 1 - the source mesh has colors, which are modulated with the current color
	//					  2 - the source mesh has no colors : colors are directly copied into the dest vb

	if (inVB.getVertexFormat() & CVertexBuffer::PrimaryColorFlag) // case 1
	{
		// TODO: optimisation : avoid to duplicate colors...
		_ColorScheme->makeN(_Owner, startIndex, outVB.getColorPointer(), outVB.getVertexSize(), toProcess, inVB.getNumVertices(), srcStep);
		// modulate from the source mesh
		uint8 *vDest  = (uint8 *) outVB.getColorPointer();
		uint8 *vSrc   = (uint8 *) inVB.getColorPointer();
		const uint vSize = outVB.getVertexSize();
		const uint numVerts = inVB.getNumVertices();
		uint  meshSize = vSize * numVerts;		
		for (uint k = 0; k < toProcess; ++k)
		{
			NLMISC::CRGBA::modulateColors((CRGBA *) vDest, (CRGBA *) vSrc, (CRGBA *) vDest, numVerts, vSize, vSize);
			vDest += meshSize;
		}
	}
	else // case 2
	{
		_ColorScheme->makeN(_Owner, startIndex, outVB.getColorPointer(), outVB.getVertexSize(), toProcess, inVB.getNumVertices(), srcStep);
	}
}


//====================================================================================
void CPSConstraintMesh::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newSizeElement(emitterLocated, emitterIndex);
	newPlaneBasisElement(emitterLocated, emitterIndex);
	// TODO : avoid code cuplication with CPSFace ...
	const uint32 nbConf = _PrecompBasis.size();
	if (nbConf) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis[_Owner->getNewElementIndex()] = rand() % nbConf;
	}
	newColorElement(emitterLocated, emitterIndex);
	if (_GlobalAnimationEnabled && _ReinitGlobalAnimTimeOnNewElement)
	{		
		_GlobalAnimDate = _Owner->getOwner()->getSystemDate();
	}
	if (_MorphScheme && _MorphScheme->hasMemory()) _MorphScheme->newElement(emitterLocated, emitterIndex);
}
	
//====================================================================================	
void CPSConstraintMesh::deleteElement(uint32 index)
{
	deleteSizeElement(index);
	deletePlaneBasisElement(index);
	// TODO : avoid code cuplication with CPSFace ...
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		// replace ourself by the last element...
		_IndexInPrecompBasis[index] = _IndexInPrecompBasis[_Owner->getSize() - 1];
	}
	deleteColorElement(index);
	if (_MorphScheme && _MorphScheme->hasMemory()) _MorphScheme->deleteElement(index);
}
	
//====================================================================================
void CPSConstraintMesh::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	resizeSize(size);
	resizePlaneBasis(size);
	// TODO : avoid code cuplication with CPSFace ...
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis.resize(size);
	}
	resizeColor(size);
	if (_MorphScheme && _MorphScheme->hasMemory()) _MorphScheme->resize(size, _Owner->getSize());
}	

//====================================================================================
void CPSConstraintMesh::updateMatAndVbForColor(void)
{
	// nothing to do for us...
}

//====================================================================================
void	CPSConstraintMesh::forceStageModulationByColor(uint stage, bool force)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	if (force)
	{
		_ModulatedStages |= 1 << stage;
	}
	else
	{
		_ModulatedStages &= ~(1 << stage);
	}
}

//====================================================================================
bool	CPSConstraintMesh::isStageModulationForced(uint stage) const
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return (_ModulatedStages & (1 << stage)) != 0;
}

//====================================================================================

/** This duplicate a primitive block n time in the destination primitive block
 *  This is used to draw several mesh at once
 *  For each duplication, vertices indices are shifted from the given offset (number of vertices in the mesh)
 */

static void DuplicatePrimitiveBlock(const CPrimitiveBlock &srcBlock, CPrimitiveBlock &destBlock, uint nbReplicate, uint vertOffset)
{
	PARTICLES_CHECK_MEM;

	// this must be update each time a new primitive is added
	
	// loop counters, and index of the current primitive in the dest pb
	uint k, l, index;

	// the current vertex offset.
	uint currVertOffset;


	// duplicate triangles
	uint numTri = srcBlock.getNumTri();
	destBlock.reserveTri(numTri * nbReplicate);
	
	index = 0;
	currVertOffset = 0;

	const uint32 *triPtr = srcBlock.getTriPointer();
	const uint32 *currTriPtr; // current Tri
	for (k = 0; k < nbReplicate; ++k)
	{
		currTriPtr = triPtr;
		for (l = 0; l < numTri; ++l)
		{
			destBlock.setTri(index, currTriPtr[0] + currVertOffset, currTriPtr[1] + currVertOffset, currTriPtr[2] + currVertOffset);
			currTriPtr += 3;
			++ index;
		}
		currVertOffset += vertOffset;
	}


	// duplicate quads
	uint numQuad = srcBlock.getNumQuad();
	destBlock.reserveQuad(numQuad * nbReplicate);
	
	index = 0;
	currVertOffset = 0;

	const uint32 *QuadPtr = srcBlock.getQuadPointer();
	const uint32 *currQuadPtr; // current Quad
	for (k = 0; k < nbReplicate; ++k)
	{
		currQuadPtr = QuadPtr;
		for (l = 0; l < numQuad; ++l)
		{
			destBlock.setQuad(index, currQuadPtr[0] + currVertOffset, currQuadPtr[1] + currVertOffset, currQuadPtr[2] + currVertOffset, currQuadPtr[3] + currVertOffset);
			currQuadPtr += 4;
			++ index;
		}
		currVertOffset += vertOffset;
	}

	// duplicate lines
	uint numLine = srcBlock.getNumLine();
	destBlock.reserveLine(numLine * nbReplicate);
	
	index = 0;
	currVertOffset = 0;

	const uint32 *LinePtr = srcBlock.getLinePointer();
	const uint32 *currLinePtr; // current Line
	for (k = 0; k < nbReplicate; ++k)
	{
		currLinePtr = LinePtr;
		for (l = 0; l < numLine; ++l)
		{
			destBlock.setLine(index, currLinePtr[0] + currVertOffset, currLinePtr[1] + currVertOffset);
			currLinePtr += 4;
			++ index;
		}
		currVertOffset += vertOffset;
	}	


	// TODO quad / strips duplication : (unimplemented in primitive blocks for now)

	PARTICLES_CHECK_MEM;
}

//====================================================================================
void CPSConstraintMesh::initPrerotVB()
{
	// position, no normals
	_PreRotatedMeshVB.setVertexFormat(CVertexBuffer::PositionFlag);
	_PreRotatedMeshVB.setNumVertices(ConstraintMeshMaxNumPrerotatedModels * ConstraintMeshMaxNumVerts);

	// position & normals
	_PreRotatedMeshVBWithNormal.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag);
	_PreRotatedMeshVBWithNormal.setNumVertices(ConstraintMeshMaxNumPrerotatedModels * ConstraintMeshMaxNumVerts);
}

//====================================================================================
CPSConstraintMesh::CMeshDisplay &CPSConstraintMesh::CMeshDisplayShare::getMeshDisplay(IShape *shape, uint32 format)
{
	CKey key;
	key.Shape = shape;
	key.Format = format;
	if (MDMap.count(key)) // already exists ?
	{
		nlassert(MDMap[key]);
		return *MDMap[key];
	}
	else
	{
		if (MDQueue.size() == _MaxNumMD) // is there room left?
		{
			// no, destroy the least recent entry
			nlassert(MDMap.count(MDQueue.front())); // make sure it is also in the map
			MDMap.erase(MDQueue.front());
			MDQueue.pop();
		}
		std::auto_ptr<CMeshDisplay> MD(new CMeshDisplay);
	
		// setup rdr passes & primitive blocks
		buildRdrPassSet(MD->RdrPasses, shape);
		
		// setup vb
		buildVB(format, MD->VB, shape);
		
		MDQueue.push(key);
		MDMap[key] = MD.get();
		return *(MD.release());
	}
}


//====================================================================================
CPSConstraintMesh::CMeshDisplayShare::~CMeshDisplayShare()
{
	for (TMDMap::iterator it = MDMap.begin(); it != MDMap.end(); ++it)
	{
		delete it->second;
	}
}

//====================================================================================
void CPSConstraintMesh::CMeshDisplayShare::buildRdrPassSet(TRdrPassSet &dest, const IShape *shape)
{	
	const CMesh &m = *NLMISC::safe_cast<const CMesh *>(shape);
	// we don't support skinning for mesh particles, so there must be only one matrix block	
	nlassert(m.getNbMatrixBlock() == 1);  // SKINNING UNSUPPORTED
	
	dest.resize(m.getNbRdrPass(0));
	const CVertexBuffer &srcVb = m.getVertexBuffer();
	
	for (uint k = 0; k < m.getNbRdrPass(0); ++k)
	{
		dest[k].Mat = m.getMaterial(m.getRdrPassMaterial(0, k));
		dest[k].SourceMat = dest[k].Mat;
		DuplicatePrimitiveBlock(m.getRdrPassPrimitiveBlock(0, k), dest[k].Pb, ConstraintMeshBufSize, srcVb.getNumVertices() );		
	}	
}

//====================================================================================
void CPSConstraintMesh::CMeshDisplayShare::buildVB(uint32 destFormat, CVertexBuffer &dest, const IShape *shape)
{
	/// we duplicate the original mesh data's 'ConstraintMeshBufSize' times, eventually adding a color
	nlassert(shape);
	const CMesh &m = *NLMISC::safe_cast<const CMesh *>(shape);
	const CVertexBuffer &meshVb = m.getVertexBuffer();
	nlassert(destFormat == meshVb.getVertexFormat() || destFormat == (meshVb.getVertexFormat() | (uint32) CVertexBuffer::PrimaryColorFlag) );
	dest.setVertexFormat(destFormat);
	dest.setNumVertices(ConstraintMeshBufSize * meshVb.getNumVertices());

	uint8 *outPtr = (uint8 *) dest.getVertexCoordPointer();
	uint8 *inPtr = (uint8 *)  meshVb.getVertexCoordPointer();
	uint  meshSize  = dest.getVertexSize() * meshVb.getNumVertices();

	if (destFormat == meshVb.getVertexFormat()) // no color added
	{		
		for (uint k = 0; k < ConstraintMeshBufSize; ++k)
		{
			::memcpy((void *) (outPtr + k * meshSize), (void *) inPtr, meshSize);			
		}
	}
	else // color added, but not available in src 
	{
		sint colorOff = dest.getColorOff();
		uint inVSize    = meshVb.getVertexSize();
		uint outVSize   = dest.getVertexSize();		
		for (uint k = 0; k < ConstraintMeshBufSize; ++k)
		{
			for (uint v = 0; v < meshVb.getNumVertices(); ++v)
			{
				// copy until color
				::memcpy((void *) (outPtr + k * meshSize + v * outVSize), (void *) (inPtr + v * inVSize), colorOff);
				// copy datas after color
				::memcpy((void *) (outPtr + k * meshSize + v * outVSize + colorOff + sizeof(uint8[4])), (void *) (inPtr + v * inVSize + colorOff), inVSize - colorOff);
			}
		}
	}
}

//=====================================================================================
CPSConstraintMesh::CMeshDisplayShare::CKey::~CKey()
{
}

//=====================================================================================	
CPSConstraintMesh::CGlobalTexAnim::CGlobalTexAnim() : TransSpeed(NLMISC::CVector2f::Null),
							 TransAccel(NLMISC::CVector2f::Null),
							 ScaleStart(1 ,1),
							 ScaleSpeed(NLMISC::CVector2f::Null),
							 ScaleAccel(NLMISC::CVector2f::Null),
							 WRotSpeed(0),
							 WRotAccel(0)
{
}

//=====================================================================================
void	CPSConstraintMesh::CGlobalTexAnim::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	f.serial(TransSpeed, TransAccel, ScaleStart, ScaleSpeed, ScaleAccel);
	f.serial(WRotSpeed, WRotAccel);
}

//=====================================================================================
void CPSConstraintMesh::CGlobalTexAnim::buildMatrix(float &date, NLMISC::CMatrix &dest)
{
	float fDate = (float) date;
	float halfDateSquared   = 0.5f * fDate * fDate;
	NLMISC::CVector2f pos   = fDate * TransSpeed + halfDateSquared * fDate * TransAccel;
	NLMISC::CVector2f scale = ScaleStart + fDate * ScaleSpeed + halfDateSquared * fDate * ScaleAccel;
	float rot = fDate * WRotSpeed + halfDateSquared * WRotAccel;
	
			
	float fCos, fSin;
	if (rot != 0.f)
	{
		fCos = ::cosf(- rot);
		fSin = ::sinf(- rot);
	}
	else
	{
		fCos = 1.f;
		fSin = 0.f;
	}

	NLMISC::CVector I(fCos, fSin, 0);
	NLMISC::CVector J(-fSin, fCos, 0);				
	dest.setRot(scale.x * I, scale.y * J, NLMISC::CVector::K);	
	NLMISC::CVector center(-0.5f, -0.5f, 0.f);
	NLMISC::CVector t(pos.x, pos.y, 0);
	dest.setPos(t + dest.mulVector(center) - center);	
}

//=====================================================================================
void	CPSConstraintMesh::setGlobalTexAnim(uint stage, const CGlobalTexAnim &properties)
{
	nlassert(_GlobalAnimationEnabled != 0);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(_GlobalTexAnims.get());
	_GlobalTexAnims->Anims[stage] = properties;
}

//=====================================================================================
const CPSConstraintMesh::CGlobalTexAnim &CPSConstraintMesh::getGlobalTexAnim(uint stage) const
{
	nlassert(_GlobalAnimationEnabled != 0);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(_GlobalTexAnims.get());
	return _GlobalTexAnims->Anims[stage];
}


//=====================================================================================
CPSConstraintMesh::TTexAnimType CPSConstraintMesh::getTexAnimType() const
{
	return (TTexAnimType) (_GlobalAnimationEnabled != 0 ? GlobalAnim : NoAnim);	
}

//=====================================================================================
void  CPSConstraintMesh::setTexAnimType(TTexAnimType type)
{
	nlassert(type < Last);
	if (type == getTexAnimType()) return; // does the type of animation change ?
	switch (type)
	{
		case NoAnim:
			_GlobalTexAnims.reset();
			restoreMaterials();
			_GlobalAnimationEnabled = 0;
		break;
		case GlobalAnim:
		{
			PGlobalTexAnims newPtr(new CGlobalTexAnims);
			std::swap(_GlobalTexAnims, newPtr);			
			_GlobalAnimationEnabled = 1;
		}
		break;
		default: break;
	}
}

//=====================================================================================
void	CPSConstraintMesh::CGlobalTexAnims::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		f.serial(Anims[k]);
	}
}

//=====================================================================================
void CPSConstraintMesh::restoreMaterials()
{
	update();
	CMesh				  &mesh	= * NLMISC::safe_cast<CMesh *>((IShape *) _Shapes[0]);
	const CVertexBuffer   &modelVb = mesh.getVertexBuffer();
	CMeshDisplay  &md= _MeshDisplayShare.getMeshDisplay(_Shapes[0], modelVb.getVertexFormat() 
															| (_ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));

	TRdrPassSet rdrPasses = md.RdrPasses;
		// render meshs : we process each rendering pass
	for (TRdrPassSet::iterator rdrPassIt = rdrPasses.begin() 
		; rdrPassIt != rdrPasses.end(); ++rdrPassIt)
	{	
		rdrPassIt->Mat = rdrPassIt->SourceMat;		
	}
}

} // NL3D
