/** \file water_shape.cpp
 * <File description>
 *
 * $Id: water_shape.cpp,v 1.1 2001/10/26 08:21:57 vizerie Exp $
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

#include "3d/water_shape.h"
#include "3d/water_model.h"
#include "3d/vertex_buffer.h"
#include "3d/texture_bump.h"
#include "3d/scene.h"
#include <memory>


namespace NL3D {




// globals
static std::auto_ptr<CVertexProgram> WaterVP(NULL);
const char *WaterVpCode = "!!VP1.0\n\
					  ADD R1, c[7], -v[0];\n\
					  DP3 R2, R1, R1;\n\
					  RSQ R2, R2.x;\n\
					  MUL R3, R2, c[4].y;\n\
					  MIN R3, c[4].x, R3;\n\
					  MUL R0,   R3, v[8];\n\
					  MOV R0.z,  c[4].x;\n\
					  DP3 R3.x, R0, R0;\n\
					  RSQ R3.x,  R3.x;\n\
					  MUL R0,  R0, R3.x;\n\
					  DP3 o[COL0], R0, c[6];\n\
					  DP4 o[HPOS].x, c[0], v[0];\n\
					  DP4 o[HPOS].y, c[1], v[0];\n\
					  DP4 o[HPOS].z, c[2], v[0];\n\
					  DP4 o[HPOS].w, c[3], v[0];\n\
					  MUL R3, v[0], c[12];\n\
					  ADD o[TEX0].xy, R3, c[11];\n\
					  MUL R3, v[0], c[14];\n\
					  ADD o[TEX1].xy, R3, c[13];\n\
					  MUL R1, R1, R2.x;\n\
					  DP3 R2.x, R1, R0;\n\
					  MUL R0, R0, R2.x;\n\
					  ADD R2, R0, R0;\n\
					  ADD R0, R2, -R1;\n\
					  MAD o[TEX2].xy, R0, c[8], c[8];\n\
					  END\
					  ";



// static members

uint32									CWaterShape::_XScreenGridSize = 55;
uint32									CWaterShape::_YScreenGridSize = 55;
CVertexBuffer							CWaterShape::_VB;
std::vector<uint32>						CWaterShape::_IB;
NLMISC::CSmartPtr<IDriver>				CWaterShape::_Driver;
bool									CWaterShape::_GridSizeTouched = true;
std::auto_ptr<CVertexProgram>	CWaterShape::_VertexProgram;





/*
 * Constructor
 */
CWaterShape::CWaterShape() :  _WaterPoolID(0)
{
	for (sint k = 0; k < 2; ++k)
	{
		_HeightMapScale[k].set(1, 1);
		_HeightMapSpeed[k].set(0, 0);
	}
	_ColorMapOffset.set(0, 0);
	_ColorMapScale.set(1, 1);


}

void CWaterShape::initVertexProgram()
{	
	_VertexProgram = std::auto_ptr<CVertexProgram>(new CVertexProgram(WaterVpCode));	
}



void CWaterShape::setupVertexBuffer()
{
	_VB.clearValueEx();
	_VB.addValueEx (WATER_VB_POS, CVertexBuffer::Float3);
	_VB.addValueEx (WATER_VB_DX, CVertexBuffer::Float2);	

	_VB.initEx();
	_VB.setNumVertices((_XScreenGridSize + 1) * (_YScreenGridSize + 1));
	_IB.resize(_XScreenGridSize * _YScreenGridSize * 6);

	// setup the IB
	uint x, y;
	for (y = 0; y < _YScreenGridSize; ++y)
	{
		for (x = 0; x < _XScreenGridSize; ++x)
		{
			_IB [ 6 * (x + _XScreenGridSize * y)    ] = x + 1 + (_XScreenGridSize + 1) * y;
			_IB [ 6 * (x + _XScreenGridSize * y) + 1] = x +   + (_XScreenGridSize + 1) * y;
			_IB [ 6 * (x + _XScreenGridSize * y) + 2] = x     + (_XScreenGridSize + 1) * (y + 1);

			_IB [ 6 * (x + _XScreenGridSize * y) + 3] = x + 1 + (_XScreenGridSize + 1) * (y + 1);
			_IB [ 6 * (x + _XScreenGridSize * y) + 4] = x + 1 + (_XScreenGridSize + 1) * y;
			_IB [ 6 * (x + _XScreenGridSize * y) + 5] = x     + (_XScreenGridSize + 1) * (y + 1);			
		}
	}
	_GridSizeTouched = false;
}

CTransformShape		*CWaterShape::createInstance(CScene &scene)
{
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(scene.createModel(WaterModelClassId) );
	wm->Shape = this;
	return wm;
}




float CWaterShape::getNumTriangles (float distance)
{
	// TODO
	return 0;
}

void CWaterShape::flushTextures (IDriver &driver)
{
	for (uint k = 0; k < 2; ++k)
	{
		if (_BumpMap[k] != NULL)
			_BumpMap[k]->generate();
	}
}

void	CWaterShape::setScreenGridSize(uint32 x, uint32 y)
{
	nlassert(x > 0 && y > 0);
	_XScreenGridSize = x;	
	_YScreenGridSize = y;
}



void CWaterShape::setShape(const NLMISC::CPolygon &poly)
{
	nlassert(poly.Vertices.size() != 0); // empty poly not allowed
	_Poly = poly;
	computeBBox();
}

void CWaterShape::computeBBox()
{
	nlassert(_Poly.Vertices.size() != 0);
	NLMISC::CVector min, max;
	min = max = _Poly.Vertices[0];
	for (uint k = 1; k < _Poly.Vertices.size(); ++k)
	{
		min.minof(min, _Poly.Vertices[k]);
		max.maxof(max, _Poly.Vertices[k]);
	}
	_BBox.setMinMax(min, max);
}


void				CWaterShape::setHeightMap(uint k, ITexture *hm)
{
	nlassert(k < 2);	
	if (!_BumpMap[k])
	{
		_BumpMap[k] = new CTextureBump;
	}
	((CTextureBump *) (ITexture *) _BumpMap[k])->setHeightMap(hm);
}

ITexture			*CWaterShape::getHeightMap(uint k)
{
	nlassert(k < 2);
	return ((CTextureBump *) (ITexture *) _BumpMap[k] )->getHeightMap();
}

const ITexture		*CWaterShape::getHeightMap(uint k) const
{
	nlassert(k < 2);
	return ((CTextureBump *) (ITexture *) _BumpMap[k] )->getHeightMap();
}

void CWaterShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(0);
	f.serial(_Poly);
	f.serial(_WaterPoolID);
	//serial maps
	ITexture *map = NULL;	
	if (f.isReading())
	{
		f.serialPolyPtr(map); _EnvMap = map;
		f.serialPolyPtr(map); _BumpMap[0] = map;
		f.serialPolyPtr(map); _BumpMap[1] = map;
		f.serialPolyPtr(map); _ColorMap = map;
	}
	else
	{
		map = _EnvMap; f.serialPolyPtr(map);
		map = _BumpMap[0]; f.serialPolyPtr(map);
		map = _BumpMap[1]; f.serialPolyPtr(map);
		map = _ColorMap; f.serialPolyPtr(map);
		computeBBox();
	}

	f.serial(_HeightMapScale[0], _HeightMapScale[1],
			 _HeightMapSpeed[0], _HeightMapSpeed[1]);
	f.serial(_ColorMapOffset, _ColorMapScale);
}


bool CWaterShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{	
	for (uint k = 0; k < pyramid.size(); ++k)
	{
		if (! _BBox.clipBack(pyramid[k] * worldMatrix)) return false;
	}	
	return true;
}

void				CWaterShape::setHeightMapScale(uint k, const NLMISC::CVector2f &scale)
{
	nlassert(k < 2);
	_HeightMapScale[k] = scale;
}
NLMISC::CVector2f	CWaterShape::getHeightMapScale(uint k) const
{
	nlassert(k < 2);
	return _HeightMapScale[k];
}
void			    CWaterShape::setHeightMapSpeed(uint k, const NLMISC::CVector2f &speed)
{
	nlassert(k < 2);
	_HeightMapSpeed[k] = speed;
}
NLMISC::CVector2f   CWaterShape::getHeightMapSpeed(uint k) const
{
	nlassert(k < 2);
	return _HeightMapSpeed[k];
}

void	CWaterShape::setColorMapPos(uint k, const NLMISC::CVector2f &scale, const NLMISC::CVector2f &offset)
{
	_ColorMapScale = scale;
	_ColorMapOffset = offset;

}

} // NL3D
