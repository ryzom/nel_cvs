/** \file landscapevb_info.cpp
 * <File description>
 *
 * $Id: landscapevb_info.cpp,v 1.2 2002/02/28 12:59:49 besson Exp $
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

#include "3d/landscapevb_info.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "3d/landscapevb_allocator.h"


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// VertexBufferInfo.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CFarVertexBufferInfo::setupNullPointers()
{
	VertexCoordPointer= NULL;
	TexCoordPointer0= NULL;
	ColorPointer= NULL;
	GeomInfoPointer= NULL;
	DeltaPosPointer= NULL;
	AlphaInfoPointer= NULL;
}


// ***************************************************************************
void		CFarVertexBufferInfo::setupPointersForVertexProgram()
{
	// see CLandscapeVBAllocator for program definition.
	uint8	*vcoord= (uint8*)VertexCoordPointer;

	TexCoordPointer0= vcoord + TexCoordOff0;
	GeomInfoPointer= vcoord + GeomInfoOff;			
	DeltaPosPointer= vcoord + DeltaPosOff;
	AlphaInfoPointer= vcoord + AlphaInfoOff;
}


// ***************************************************************************
void		CFarVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb, bool forVertexProgram)
{
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	if(NumVertices==0)
	{
		setupNullPointers();
		return;
	}

	VertexCoordPointer= vb.getVertexCoordPointer();

	if(forVertexProgram)
	{
		// With VertexCoordPointer setuped, init for VP.
		TexCoordOff0= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX0);				// v[8]= Tex0.
		GeomInfoOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO);			// v[10]= GeomInfos.
		DeltaPosOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS);			// v[11]= EndPos-StartPos
		// Init Alpha Infos only if enabled (enabled if Value 5 are).
		AlphaInfoOff= 0;
		if( vb.getVertexFormat() & (1<<NL3D_LANDSCAPE_VPPOS_ALPHAINFO) )
			AlphaInfoOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_ALPHAINFO);		// v[12]= AlphaInfos

		// update Ptrs.
		setupPointersForVertexProgram();
	}
	else
	{
		TexCoordOff0= vb.getTexCoordOff(0);
		TexCoordPointer0= vb.getTexCoordPointer(0, 0);

		// In Far0, we don't have Color component.
		if(VertexFormat & CVertexBuffer::PrimaryColorFlag)
		{
			ColorOff= vb.getColorOff();
			ColorPointer= vb.getColorPointer();
		}
		else
		{
			ColorOff= 0;
			ColorPointer= NULL;
		}
	}

}
// ***************************************************************************
void		CFarVertexBufferInfo::setupVertexBufferHard(IVertexBufferHard &vb, void *vcoord, bool forVertexProgram)
{
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	if(NumVertices==0)
	{
		setupNullPointers();
		return;
	}

	VertexCoordPointer= vcoord;

	if(forVertexProgram)
	{
		// With VertexCoordPointer setuped, init for VP.
		TexCoordOff0= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_TEX0);				// v[8]= Tex0.
		GeomInfoOff= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_GEOMINFO);				// v[10]= GeomInfos.
		DeltaPosOff= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_DELTAPOS);				// v[11]= EndPos-StartPos
		// Init Alpha Infos only if enabled (enabled if Value 5 are).
		AlphaInfoOff= 0;
		if( vb.getVertexFormat() & (1<<NL3D_LANDSCAPE_VPPOS_ALPHAINFO) )
			AlphaInfoOff= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_ALPHAINFO);		// v[12]= AlphaInfos

		// update Ptrs.
		setupPointersForVertexProgram();
	}
	else
	{
		TexCoordOff0= vb.getValueOff (CVertexBuffer::TexCoord0);
		TexCoordPointer0= (uint8*)vcoord + TexCoordOff0;

		// In Far0, we don't have Color component.
		if(VertexFormat & CVertexBuffer::PrimaryColorFlag)
		{
			ColorOff= vb.getValueOff (CVertexBuffer::PrimaryColor);
			ColorPointer= (uint8*)vcoord + ColorOff;
		}
		else
		{
			ColorOff= 0;
			ColorPointer= NULL;
		}
	}
}


// ***************************************************************************
void		CNearVertexBufferInfo::setupNullPointers()
{
	VertexCoordPointer= NULL;
	TexCoordPointer0= NULL;
	TexCoordPointer1= NULL;
	GeomInfoPointer= NULL;
	DeltaPosPointer= NULL;
}


// ***************************************************************************
void		CNearVertexBufferInfo::setupPointersForVertexProgram()
{
	// see CLandscapeVBAllocator for program definition.
	uint8	*vcoord= (uint8*)VertexCoordPointer;

	TexCoordPointer0= vcoord + TexCoordOff0;
	TexCoordPointer1= vcoord + TexCoordOff1;
	GeomInfoPointer= vcoord + GeomInfoOff;			
	DeltaPosPointer= vcoord + DeltaPosOff;

}


// ***************************************************************************
void		CNearVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb, bool forVertexProgram)
{
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	if(NumVertices==0)
	{
		setupNullPointers();
		return;
	}

	VertexCoordPointer= vb.getVertexCoordPointer();

	if(forVertexProgram)
	{
		// With VertexCoordPointer setuped, init for VP.
		TexCoordOff0= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX0);				// v[8]= Tex0.
		TexCoordOff1= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_TEX1);				// v[9]= Tex1.
		GeomInfoOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_GEOMINFO);			// v[10]= GeomInfos.
		DeltaPosOff= vb.getValueOffEx(NL3D_LANDSCAPE_VPPOS_DELTAPOS);			// v[11]= EndPos-StartPos

		// update Ptrs.
		setupPointersForVertexProgram();
	}
	else
	{
		TexCoordPointer0= vb.getTexCoordPointer(0, 0);
		TexCoordPointer1= vb.getTexCoordPointer(0, 1);

		TexCoordOff0= vb.getTexCoordOff(0);
		TexCoordOff1= vb.getTexCoordOff(1);
	}
}
// ***************************************************************************
void		CNearVertexBufferInfo::setupVertexBufferHard(IVertexBufferHard &vb, void *vcoord, bool forVertexProgram)
{
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	if(NumVertices==0)
	{
		setupNullPointers();
		return;
	}

	VertexCoordPointer= vcoord;

	if(forVertexProgram)
	{
		// With VertexCoordPointer setuped, init for VP.
		TexCoordOff0= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_TEX0);				// v[8]= Tex0.
		TexCoordOff1= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_TEX1);				// v[9]= Tex1.
		GeomInfoOff= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_GEOMINFO);				// v[10]= GeomInfos.
		DeltaPosOff= vb.getValueOff(NL3D_LANDSCAPE_VPPOS_DELTAPOS);				// v[11]= EndPos-StartPos

		// update Ptrs.
		setupPointersForVertexProgram();
	}
	else
	{
		TexCoordPointer0= (uint8*)vcoord + vb.getValueOff (CVertexBuffer::TexCoord0);
		TexCoordPointer1= (uint8*)vcoord + vb.getValueOff (CVertexBuffer::TexCoord1);

		TexCoordOff0= vb.getValueOff (CVertexBuffer::TexCoord0);
		TexCoordOff1= vb.getValueOff (CVertexBuffer::TexCoord1);
	}
}


} // NL3D
