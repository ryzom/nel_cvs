/** \file vegetablevb_allocator.cpp
 * <File description>
 *
 * $Id: vegetablevb_allocator.cpp,v 1.2 2001/11/05 16:26:45 berenguier Exp $
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


#include "3d/vegetablevb_allocator.h"
#include "3d/vegetable_def.h"


using namespace std;
using namespace NLMISC;

namespace NL3D 
{


/*
	Once a reallocation of a VBHard occurs, how many vertices we add to the re-allocation, to avoid 
	as possible reallocations.
*/
#define	NL3D_VEGETABLE_VERTEX_ALLOCATE_SECURITY		1024
/*
	The start size of the array.
*/
#define	NL3D_VEGETABLE_VERTEX_ALLOCATE_START		4048


#define	NL3D_VEGETABLE_VERTEX_FREE_MEMORY_RESERVE	1024


// 65000 is a maximum because of GeForce limitations.
#define	NL3D_VEGETABLE_VERTEX_MAX_VERTEX_VBHARD		60000


// ***************************************************************************
CVegetableVBAllocator::CVegetableVBAllocator()
{
	_Type= 0xFFFFFFFF;

	// Init free list
	_VertexFreeMemory.reserve(NL3D_VEGETABLE_VERTEX_FREE_MEMORY_RESERVE);
	_NumVerticesAllocated= 0;

	// Init vbhard
	_VBHardOk= false;
	_BufferLocked= false;
	_AGPBufferPtr= NULL;
}


// ***************************************************************************
void			CVegetableVBAllocator::init(uint type)
{
	nlassert(type<NL3D_VEGETABLE_NRDRPASS);
	_Type= type;

	// According to _Type, build VB format, and create VertexProgram
	setupVBFormatAndVertexProgram();
}


// ***************************************************************************
CVegetableVBAllocator::~CVegetableVBAllocator()
{
	clear();
	// delete the VP
	delete	_VertexProgram;
	_VertexProgram= NULL;
}

// ***************************************************************************
void			CVegetableVBAllocator::updateDriver(IDriver *driver)
{
	// test change of driver.
	nlassert(driver && !_BufferLocked);
	// If change of driver
	if( _Driver==NULL || driver!=_Driver )
	{
		// delete old VBHard.
		deleteVertexBufferHard();
		_Driver= driver;
		_VBHardOk= _Driver->supportVertexBufferHard();

		// Driver must support VP.
		nlassert(_Driver->isVertexProgramSupported());

		// must reallocate the VertexBuffer.
		if( _NumVerticesAllocated>0 )
			allocateVertexBufferAndFillVBHard(_NumVerticesAllocated);
	}
	else
	{
		// if VBHard possible, and if vbHardDeleted but space needed, reallocate.
		if( _VBHardOk && _VBHard==NULL && _NumVerticesAllocated>0 )
			allocateVertexBufferAndFillVBHard(_NumVerticesAllocated);
	}

}

// ***************************************************************************
void			CVegetableVBAllocator::clear()
{
	// clear list.
	_VertexFreeMemory.clear();
	_NumVerticesAllocated= 0;

	// delete the VB.
	deleteVertexBufferHard();
	// really delete the VB soft too
	_VB.deleteAllVertices();

	// clear other states.
	_Driver= NULL;
	_VBHardOk= false;
}


// ***************************************************************************
void			CVegetableVBAllocator::lockBuffer()
{
	// force unlock
	unlockBuffer();

	if(_VBHard)
	{
		_AGPBufferPtr= (uint8*)_VBHard->lock();
	}

	_BufferLocked= true;
}

// ***************************************************************************
void			CVegetableVBAllocator::unlockBuffer()
{
	if(_BufferLocked)
	{
		if(_VBHard)
			_VBHard->unlock();
		_BufferLocked= false;
		_AGPBufferPtr= NULL;
	}
}


// ***************************************************************************
uint			CVegetableVBAllocator::allocateVertex()
{
	// if no more free, allocate.
	if( _VertexFreeMemory.size()==0 )
	{
		// enlarge capacity.
		uint	newResize;
		if(_NumVerticesAllocated==0)
			newResize= NL3D_VEGETABLE_VERTEX_ALLOCATE_START;
		else
			newResize= NL3D_VEGETABLE_VERTEX_ALLOCATE_SECURITY;
		_NumVerticesAllocated+= newResize;
		// re-allocate VB.
		allocateVertexBufferAndFillVBHard(_NumVerticesAllocated);
		// resize infos on vertices.
		_VertexInfos.resize(_NumVerticesAllocated);

		// Fill list of free elements.
		for(uint i=0;i<newResize;i++)
		{
			// create a new entry which points to this vertex.
			// the list is made so allocation is in growing order.
			_VertexFreeMemory.push_back( _NumVerticesAllocated - (i+1) );

			// Mark as free the new vertices. (Debug).
			_VertexInfos[_NumVerticesAllocated - (i+1)].Free= true;
		}
	}

	// get a vertex (pop_back).
	uint	id= _VertexFreeMemory.back();
	// delete this vertex free entry.
	_VertexFreeMemory.pop_back();

	// check and Mark as not free the vertex. (Debug).
	nlassert(id<_NumVerticesAllocated);
	nlassert(_VertexInfos[id].Free);
	_VertexInfos[id].Free= false;


	return id;
}

// ***************************************************************************
void			CVegetableVBAllocator::deleteVertex(uint vid)
{
	// check and Mark as free the vertex. (Debug).
	nlassert(vid<_NumVerticesAllocated);
	nlassert(!_VertexInfos[vid].Free);
	_VertexInfos[vid].Free= true;

	// Add this vertex to the free list.
	// create a new entry which points to this vertex.
	_VertexFreeMemory.push_back( vid );
}

// ***************************************************************************
void			*CVegetableVBAllocator::getVertexPointer(uint i)
{
	return _VB.getVertexCoordPointer(i);
}

// ***************************************************************************
void			CVegetableVBAllocator::flushVertex(uint i)
{
	if(_VBHardOk)
	{
		nlassert(_VBHard && _BufferLocked);
		// copy the VB soft to the VBHard.
		void	*src= getVertexPointer(i);
		void	*dst= _AGPBufferPtr + i*_VB.getVertexSize();
		memcpy(dst, src, _VB.getVertexSize());
	}
}

// ***************************************************************************
void			CVegetableVBAllocator::activate()
{
	nlassert(_Driver);
	nlassert(!_BufferLocked);

	// activate Vertex program first.
	//nlinfo("\nSTARTVP\n%s\nENDVP\n", _VertexProgram->getProgram().c_str());
	nlverify(_Driver->activeVertexProgram(_VertexProgram));

	// Activate VB.
	if(_VBHard)
		_Driver->activeVertexBufferHard(_VBHard);
	else
		_Driver->activeVertexBuffer(_VB);
}


// ***************************************************************************
// ***************************************************************************
// Vertex Buffer hard.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void				CVegetableVBAllocator::deleteVertexBufferHard()
{
	// must unlock VBhard before.
	unlockBuffer();

	// test (refptr) if the object still exist in memory.
	if(_VBHard!=NULL)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// delete it from driver.
		_Driver->deleteVertexBufferHard(_VBHard);
		_VBHard= NULL;
	}

}

// ***************************************************************************
void				CVegetableVBAllocator::allocateVertexBufferAndFillVBHard(uint32 numVertices)
{
	// resize the Soft VB.
	_VB.setNumVertices(numVertices);

	// no allocation must be done if the Driver is not setuped, or if the driver has been deleted by refPtr.
	nlassert(_Driver);

	// must unlock VBhard before.
	bool	wasLocked= bufferLocked();
	unlockBuffer();

	// try to allocate a vbufferhard if possible.
	if( _VBHardOk )
	{
		// delete possible old _VBHard.
		if(_VBHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VBHard);
		}

		// try to create new one, in AGP Ram
		// If too many vertices wanted, abort VBHard.
		if(numVertices <= NL3D_VEGETABLE_VERTEX_MAX_VERTEX_VBHARD)
			_VBHard= _Driver->createVertexBufferHard(_VB.getVertexFormat(), _VB.getValueTypePointer(), numVertices, IDriver::VBHardAGP);
		else
			_VBHard= NULL;

		// If KO, never try again.
		if(_VBHard==NULL)
			_VBHardOk= false;
		else
		{
			// else, fill this AGP VBuffer Hard.
			// lock before the AGP buffer
			lockBuffer();

			// copy all the vertices to AGP.
			memcpy(_AGPBufferPtr, _VB.getVertexCoordPointer(0), _VB.getVertexSize() * numVertices);

			// If was not locked before, unlock this VB
			if(!wasLocked)
				unlockBuffer();
		}
	}

}


// ***************************************************************************
// ***************************************************************************
// Vertex Program.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/*
	Vegetable, without bend for now.

	Inputs
	--------
	v[0]  == Pos to Center of the vegetable in world space.
	v[10] == Center of the vegetable in world space.
	v[2]  == Normal (if lighted)
	v[3]  == Color (if unlit) or DiffuseColor (if lighted)
	v[4]  == SecondaryColor (==ambient if lighted or backFace color if Unlit 2Sided)
	v[8]  == Tex0 (xy) 
	v[9]  == BendInfo (xy) = {BendWeight*2, BendPhase}
		NB: *2 because compute a quaternion

	Constant:
	--------
	Setuped at beginning of CVegetableManager::render()
	c[0..3]= ModelViewProjection Matrix.
	c[4..7]= ModelView Matrix (for Fog).
	c[8]= {0, 1, 0.5, 2}
	c[9]= unit world space Directionnal light.
	NB: DiffuseColor and AmbientColor of vertex must have been pre-multiplied by lightColor

	// Bend:
	c[16]= quaternion axis. w==1, and z must be 0
	c[17]=	{ timeAnim (angle), WindPower, 0, 0 }
	c[18]=	High order Taylor cos coefficient: { -1/2, 1/24, -1/720, 1/40320 }
	c[19]=	Low order Taylor cos coefficient: { 1, -1/2, 1/24, -1/720 }
	c[20]=	Low order Taylor sin coefficient: { 1, -1/6, 1/120, -1/5040 }
	c[21]=	Special constant vector for quatToMatrix: { 0, 1, -1, 0 }
	c[22]=	{0.5, Pi, 2*Pi, 1/(2*Pi)}

	Fog Note:
	-----------
	Fog is computed on position R5.
	R5.w==1, and suppose that ModelViewMatrix has no Projection Part.
	Then Homogenous-coordinate == Non-Homogenous-coordinate.
	Hence we need only (ModelView*R1).z to get the FogC value.
	=> computed in just on instruction.
*/


// ***********************
/*
	Bend start program:
		Result: bend pos into R5, and R7,R8,R9 is the rotation matrix for possible normal lighting.

	TODO_VEGET_OPTIM: lot of ways to optimize this. And may not need rotation scheme if vegetables are not lighted...
*/
// ***********************
// Splitted in 2 parts because of the 2048 char limit
const char* NL3D_BendProgramP0=
"!!VP1.0																				\n\
	# compute time of animation: time + phase.											\n\
	ADD	R0.x, c[17].x, v[9].y;		# R0.x= time of animation							\n\
																						\n\
	# animation: f(x)= cos(x). compute a high precision cosinus							\n\
	MAD	R0.x, R0.x, c[22].w, c[22].x;	# transform to [-Pi, Pi]						\n\
	EXP	R0.y, R0.x;						# fract part=> R0.y= [0,1] <=> [-Pi, Pi]		\n\
	MAD	R0.x, R0.y, c[22].z, -c[22].y;	# R0.x= a= [-Pi, Pi]							\n\
	# R0 must get a2, a4, a6, a8														\n\
	MUL	R0.x, R0.x, R0.x;				# R0.x= a2										\n\
	MUL	R0.y, R0.x, R0.x;				# R0= a2, a4									\n\
	MUL	R0.zw, R0.y, R0.xxxy;			# R0= a2, a4, a6, a8							\n\
	# Taylor serie: cos(x)= 1 - (1/2) a2 + (1/24) a4 - (1/720) a6 + (1/40320) a8.		\n\
	DP4	R0.x, R0, c[18];				# R0.x= cos(x) - 1.								\n\
																						\n\
	# R0.x= [-2, 0]. And we want a result in [0, WindPower*BendWeight/2]				\n\
	MAD	R0.x, R0.x, c[8].z, c[8].y;		# R0.x= [0, 1]									\n\
	MUL	R0.x, R0.x, v[9].x;				# R0.x= angle [0, BendWeight/2]					\n\
	MUL	R0.x, R0.x, c[17].y;			# R0.x= angle [0, WindPower*BendWeight/2]		\n\
																						\n\
	# compute good precision sinus and cosinus, in R0.xy.								\n\
	# suppose that BendWeightMax*2== 2Pi/3 => do not need to fmod() nor					\n\
	# to have high order taylor serie													\n\
	DST	R1.xy, R0.x, R0.x;				# R1= 1, a2										\n\
	MUL	R1.z, R1.y, R1.y;				# R1= 1, a2, a4									\n\
	MUL	R1.w, R1.y, R1.z;				# R1= 1, a2, a4, a6 (cos serie)					\n\
	MUL	R2, R1, R0.x;					# R2= a, a3, a5, a7 (sin serie)					\n\
	DP4 R0.x, R1, c[19];				# R0.x= cos(a)									\n\
	DP4 R0.y, R2, c[20];				# R0.y= sin(a)									\n\
";
const char* NL3D_BendProgramP1=
"																						\n\
	# build our quaternion																\n\
	# multiply the angleAxis by sin(a) / cos(a), where a is actually a/2				\n\
	# remind: c[16].z== angleAxis.z== 0													\n\
	MUL	R0, c[16], R0.yyyx;				# R0= quaternion.xyzw							\n\
																						\n\
																						\n\
	# build	our matrix from this quaternion, into R7,R8,R9								\n\
	# Quaternion TO matrix 3x3 in 7 ope, with quat.z==0									\n\
	MUL	R1, R0, c[8].w;					# R1= quat2= 2*quat == 2*x, 2*y, 0, 2*w			\n\
	MUL R2, R1, R0.x;					# R2= quatX= xx, xy, 0, wx						\n\
	MUL R3, R1, R0.y;					# R3= quatY= xy, yy, 0, wy						\n\
	# NB: c[21]= {0, 1, -1, 0}															\n\
	MAD	R7.xyz, c[21].zyyw, R3.yxww, c[21].yxxw;										\n\
	# R7.x= a11 = 1.0f - (yy)															\n\
	# R7.y= a12 = xy																	\n\
	# R7.z= a13 = wy																	\n\
	# NB: c[21]= {0, 1, -1, 0}															\n\
	MAD	R8.xyz, c[21].yzzw, R2.yxww, c[21].xyxw;										\n\
	# R8.x= a21 = xy																	\n\
	# R8.y= a22 = 1.0f - (xx)															\n\
	# R8.z= a23 = - wx																	\n\
	# NB: c[21]= {0, 1, -1, 0}															\n\
	ADD	R9.xyz, R2.zwxw, R3.wzyw;		# a31= 0+wy, a32= wx+0, a33= xx + yy, because z==0	\n\
	MAD R9.xyz, R9.xyzw, c[21].zyzw, c[21].xxyw;										\n\
	# R9.x= a31 = - wy																	\n\
	# R9.y= a32 = wx																	\n\
	# R9.z= a33 = 1.0f - (xx + yy)														\n\
																						\n\
																						\n\
	# transform pos																		\n\
	DP3	R5.x, R7, v[0];																	\n\
	DP3	R5.y, R8, v[0];																	\n\
	DP3	R5.z, R9, v[0];				# R5= bended relative pos to center.				\n\
	MOV	R5.w, c[8].y;				# R5.w= 1.											\n\
																						\n\
																						\n\
	# add pos to center pos.															\n\
	ADD	R5.xyz, R5, v[10];			# R5= world pos. R5.w=1								\n\
																						\n\
";


// Concat the 2 strings
const string NL3D_BendProgram= string(NL3D_BendProgramP0) + string(NL3D_BendProgramP1);



// ***********************
/*
	Lighted start program:
		bend pos and normal, normalize and lit
*/
// ***********************
// Common start program.
const char* NL3D_LightedStartVegetableProgram=
"																						\n\
	# bend Pos into R5. Now do it for normal											\n\
	DP3	R0.x, R7, v[2];																	\n\
	DP3	R0.y, R8, v[2];																	\n\
	DP3	R0.z, R9, v[2];				# R0= matRot * normal.								\n\
	# Do the rot 2 times for normal (works fine)										\n\
	DP3	R6.x, R7, R0;																	\n\
	DP3	R6.y, R8, R0;																	\n\
	DP3	R6.z, R9, R0;				# R6= bended normal.								\n\
																						\n\
	# Normalize normal, and dot product, into R0.x										\n\
	DP3	R0.x, R6, R6;				# R0.x= R6.sqrnorm()								\n\
	RSQ R0.x, R0.x;					# R0.x= 1/norm()									\n\
	MUL	R6, R6, R0.x;				# R6= R6.normed()									\n\
	DP3	R0.x, R6, c[9];																	\n\
";


//	1Sided lighting.
const char* NL3D_LightedMiddle1SidedVegetableProgram=
"	#FrontFacing																		\n\
	MAX	R0.y, -R0.x, c[8].x;		# R0.y= diffFactor= max(0, -R6*LightDir)			\n\
	MUL	R1.xyz, R0.y, v[3];			# R7= diffFactor*DiffuseColor						\n\
	ADD	o[COL0].xyz, R1, v[4];		# col0.RGB= AmbientColor + diffFactor*DiffuseColor	\n\
";


//	2Sided lighting.
const char* NL3D_LightedMiddle2SidedVegetableProgram=
"	#FrontFacing																		\n\
	MAX	R0.y, -R0.x, c[8].x;		# R0.y= diffFactor= max(0, -R6*LightDir)			\n\
	MUL	R1.xyz, R0.y, v[3];			# R7= diffFactor*DiffuseColor						\n\
	ADD	o[COL0].xyz, R1, v[4];		# col0.RGB= AmbientColor + diffFactor*DiffuseColor	\n\
	# BackFacing.																		\n\
	MAX	R0.y, R0.x, c[8].x;			# R0.y= diffFactor= max(0, -(-R6)*LightDir)			\n\
	MUL	R1.xyz, R0.y, v[3];			# R7= diffFactor*DiffuseColor						\n\
	ADD	o[BFC0].xyz, R1, v[4];		# bfc0.RGB= AmbientColor + diffFactor*DiffuseColor	\n\
";


// ***********************
/*
	Unlit start program:
		bend pos into R5, and copy color(s)
*/
// ***********************


// Common start program.
// nothing to add.
const char* NL3D_UnlitStartVegetableProgram= 
"";


//	1Sided "lighting".
const char* NL3D_UnlitMiddle1SidedVegetableProgram=
"	MOV o[COL0].xyz, v[3];			# col.RGBA= vertex color							\n\
																						\n\
";


//	2Sided "lighting".
const char* NL3D_UnlitMiddle2SidedVegetableProgram=
"	MOV o[COL0].xyz, v[3];			# col.RGBA= vertex color							\n\
	MOV o[BFC0].xyz, v[4];			# bfc0.RGBA= bcf color								\n\
																						\n\
";


// ***********************
/*
	Common end of program: project, texture, and Fog. Take pos from R5
*/
// ***********************
const char* NL3D_CommonEndVegetableProgram=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R5;															\n\
	DP4 o[HPOS].y, c[1], R5;															\n\
	DP4 o[HPOS].z, c[2], R5;															\n\
	DP4 o[HPOS].w, c[3], R5;															\n\
	MOV o[TEX0].xy, v[8];																\n\
	DP4	o[FOGC].x, c[6], -R5;		# fogc>0 => fogc= - (ModelView*R5).z				\n\
	END																					\n\
";


// ***********************
/* 
	Speed test VP, No bend,no lighting.
*/
// ***********************
const char* NL3D_SimpleStartVegetableProgram=
"!!VP1.0																				\n\
	# compute in Projection space														\n\
	MOV	R5, v[0];	\n\
	ADD	R5.xyz, R5, v[10];	\n\
	MOV o[COL0], c[8].yyyy;	\n\
	MOV o[BFC0], c[8].xxyy;	\n\
";


// ***************************************************************************
void				CVegetableVBAllocator::setupVBFormatAndVertexProgram()
{
	// Build the Vertex Format.
	_VB.clearValueEx();
	_VB.addValueEx(NL3D_VEGETABLE_VPPOS_POS,		CVertexBuffer::Float3);		// v[0]
	// if lighted, need world space normal and AmbientColor for each vertex.
	if( _Type == NL3D_VEGETABLE_RDRPASS_LIGHTED || _Type == NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED )
	{
		_VB.addValueEx(NL3D_VEGETABLE_VPPOS_NORMAL, CVertexBuffer::Float3);		// v[2]
		_VB.addValueEx(NL3D_VEGETABLE_VPPOS_COLOR1, CVertexBuffer::UChar4);	// v[4]
	}
	// If 2Sided unlit, secondary color == backface color.
	else if( _Type == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED )
	{
		_VB.addValueEx(NL3D_VEGETABLE_VPPOS_COLOR1, CVertexBuffer::UChar4);	// v[4]
	}
	_VB.addValueEx(NL3D_VEGETABLE_VPPOS_COLOR0,		CVertexBuffer::UChar4);		// v[3]
	_VB.addValueEx(NL3D_VEGETABLE_VPPOS_TEX0,		CVertexBuffer::Float2);		// v[8]
	_VB.addValueEx(NL3D_VEGETABLE_VPPOS_BENDINFO,	CVertexBuffer::Float2);		// v[9]
	_VB.addValueEx(NL3D_VEGETABLE_VPPOS_CENTER,		CVertexBuffer::Float3);		// v[10]
	_VB.initEx();


	// Init the Vertex Program.
	string	vpgram;
	// start always with Bend.
	vpgram= NL3D_BendProgram;
	// combine the VP according to Type
	switch(_Type)
	{
	case NL3D_VEGETABLE_RDRPASS_LIGHTED:
		vpgram+= string(NL3D_LightedStartVegetableProgram);
		vpgram+= string(NL3D_LightedMiddle1SidedVegetableProgram);
		break;
	case NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED:		
		vpgram+= string(NL3D_LightedStartVegetableProgram);
		vpgram+= string(NL3D_LightedMiddle2SidedVegetableProgram);
		break;
	case NL3D_VEGETABLE_RDRPASS_UNLIT:		
		vpgram+= string(NL3D_UnlitStartVegetableProgram);
		vpgram+= string(NL3D_UnlitMiddle1SidedVegetableProgram);
		break;
	case NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED:		
		vpgram+= string(NL3D_UnlitStartVegetableProgram);
		vpgram+= string(NL3D_UnlitMiddle2SidedVegetableProgram);
		break;
	}

	// common end of VP
	vpgram+= string(NL3D_CommonEndVegetableProgram);

	// create VP.
	_VertexProgram= new CVertexProgram(vpgram.c_str());

}



} // NL3D
