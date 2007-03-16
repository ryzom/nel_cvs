/** \file effect_context.cpp
 * Allow the binding of effect' context parameters
 *
 * $Id: effect_context.cpp,v 1.1.2.1 2007/03/16 11:11:11 legallo Exp $
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

#include "effect_context.h"
#include "driver.h"

using namespace std;

namespace NL3D 
{

CTypeParameter::TTypeParameterMap CTypeParameter::_ConverterTypeStrToEnum;

//-------------------------------------------------------------------------------
CEffectContext::CEffectContext()
{
	init();
}

//-------------------------------------------------------------------------------
CEffectContext::~CEffectContext()
{
}

//-------------------------------------------------------------------------------
void CEffectContext::init()
{
	_TypeParameterMap[ModelViewMatrix] = CTypeParameter(CTypeParameter::Float, 4, 4);
	_TypeParameterMap[InverseModelView] = CTypeParameter(CTypeParameter::Float, 4, 4);
	_TypeParameterMap[ProjMatrix] = CTypeParameter(CTypeParameter::Float, 4, 4);
	_TypeParameterMap[ModelViewProjection] = CTypeParameter(CTypeParameter::Float, 4, 4);
	_TypeParameterMap[ViewPosition] = CTypeParameter(CTypeParameter::Float, 4);
}

//-------------------------------------------------------------------------------
inline void 
setDriverProgramConstantMatrix(IDriver * driver, CEffectContextParameter::TProgramType progType, 
							   uint index, IDriver::TMatrix matrix, IDriver::TTransform transform)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstantMatrix(index, matrix, transform);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstantMatrix(index, matrix, transform);
		break;
	}
}

//-------------------------------------------------------------------------------
inline void 
setDriverProgramConstant(IDriver * driver, CEffectContextParameter::TProgramType progType, 
						 uint index, uint num, float * src)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstant(index, num, src);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstant(index, num, src);
		break;
	}
}

//-------------------------------------------------------------------------------
inline void 
setDriverProgramConstant(IDriver * driver, CEffectContextParameter::TProgramType progType, 
						 uint index, uint num, double * src)
{
	switch(progType)
	{
	case CEffectUserParameter::VertexProgram:
		driver->setConstant(index, num, src);
		break;
	case CEffectUserParameter::PixelProgram:
		driver->setPixelProgramConstant(index, num, src);
		break;
	}
}

//-------------------------------------------------------------------------------
bool CEffectContext::setContextParameterValue(IDriver * driver, CEffectContextParameter * contextParam)
{
	switch(contextParam->getContextEnum())
	{
	case ModelViewMatrix:
	{
		setDriverProgramConstantMatrix(driver, contextParam->getProgramType(), contextParam->getRegisterNb(), 
									   IDriver::ModelView, IDriver::Identity);
		
		break;
	}
	case InverseModelView:
	{
		setDriverProgramConstantMatrix(driver, contextParam->getProgramType(), contextParam->getRegisterNb(), 
									   IDriver::ModelView, IDriver::Inverse);
		break;
	}
	case ProjMatrix:
	{
		setDriverProgramConstantMatrix(driver, contextParam->getProgramType(), contextParam->getRegisterNb(), 
									   IDriver::Projection, IDriver::Identity);
		break;
	}
	case ModelViewProjection:
	{
		setDriverProgramConstantMatrix(driver, contextParam->getProgramType(), contextParam->getRegisterNb(), 
									   IDriver::ModelViewProjection, IDriver::Identity);
		break;
	}
	case ViewPosition:
		{
			CVector pos;
			driver->getViewMatrix().getPos(pos);
			float position[4] = {pos.x, pos.y, pos.z, 1.f};
			setDriverProgramConstant(driver, contextParam->getProgramType(), contextParam->getRegisterNb(), 
				1, position);
			break;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------
bool CEffectContext::getContextParameter(CTypeParameter * param, const std::string & contextType, 
										 TContextParameter & contextEnum)
{
	contextEnum = getContextParameter(contextType);
	if(contextEnum==Unknow)
	{
		nlwarning("Unable to match '%s' with a context parameter type enum", contextType.c_str());
		return false;
	}
	
	TContextTypeParameterMap::iterator it;
	if((it=_TypeParameterMap.find(contextEnum))!=_TypeParameterMap.end())
	{
		CTypeParameter & compTypeParam = it->second;
		return  (compTypeParam==(*param));
	}
	else
	{
		nlwarning("Unable to find context parameter '%s' in map", contextType.c_str());
		return false;
	}
}

} // NL3D
