/** \file ps_size.cpp
 * <File description>
 *
 * $Id: ps_float.cpp,v 1.6 2001/09/13 14:22:21 vizerie Exp $
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

#include "3d/ps_float.h"


namespace NL3D {





float CPSFloatGradient::_DefaultGradient[] = { 0.1f, 1.0f } ;


/////////////////////////////////////
// CPSFloatGradient implementation //
/////////////////////////////////////

CPSFloatGradient::CPSFloatGradient(const float *floatTab, uint32 nbValues, uint32 nbStages, float nbCycles) 
				: CPSValueGradient<float>(nbCycles)
{
	_F.setValues(floatTab, nbValues, nbStages) ;
}


////////////////////////////////////////////
// CPSFloatBezierFunctor implementation   //
////////////////////////////////////////////

CPSFloatLagrangeFunctor::CPSFloatLagrangeFunctor() : _CoeffsTouched(true)
{
	_CtrlPoints[0] = std::make_pair(0, 0.5f);
	_CtrlPoints[1] = std::make_pair(0.25, 0.5f);
	_CtrlPoints[2] = std::make_pair(0.50, 0.5f);
	_CtrlPoints[3] = std::make_pair(0.75, 0.5f);
	updateTab();
}

void CPSFloatLagrangeFunctor::setControlPoint(uint index, float date, float value)
{
	nlassert(index < 4 && date >= 0.f && date <= 1.f)
	nlassert(index == 0 ? date == 0.f : true); // first index date must be zero
	nlassert(index == 3 ? date == 1.f : true); // last  index date must be one
	_CtrlPoints[index] = std::make_pair(date, value);
	touchCoeffs();
	updateTab();
}


const std::pair<float, float> & CPSFloatLagrangeFunctor::getControlPoint(uint index) const
{
	nlassert(index < 4);
	return _CtrlPoints[index];
}


void CPSFloatLagrangeFunctor::setNumSamples(uint32 numSamples)
{
	nlassert(numSamples != 0);
	_NumSamples = numSamples;
	updateTab();
}


uint32 CPSFloatLagrangeFunctor::getNumSamples(void) const
{
	return _NumSamples;
}


float CPSFloatLagrangeFunctor::getValue(float date) const
{
	nlassert(date >= 0 && date <= 1);
	updateCoeffs();
	const float date2 = date * date;
	return _Coeffs.x * date * date2 + _Coeffs.y * date2 + _Coeffs.z * date + _Coeffs.w;
	return 0;
}


void CPSFloatLagrangeFunctor::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	for (uint k = 0; k < 4; ++k)
	{
		f.serial(_CtrlPoints[k].first, _CtrlPoints[k].second);
		nlassert(_CtrlPoints[k].first >= 0 && _CtrlPoints[k].first >= 1);
	}
	if (f.isReading())
	{
		touchCoeffs();
		updateTab();
	}
}

void CPSFloatLagrangeFunctor::updateTab(void)
{
	nlassert(_NumSamples != 0);
	_Tab.resize(_NumSamples);
	float dateStep = 1.f / _NumSamples;
	float currDate = 0;
	for (uint k = 0; k < _NumSamples; ++k, currDate += dateStep)
	{
		_Tab[k] = getValue(currDate);
	}
}


void CPSFloatLagrangeFunctor::updateCoeffs(void) const
{
	if (!_CoeffsTouched) return;
	float mat[4][4];
	for (uint k = 0; k < 4; ++k)
	{
		mat[k][0] = _CtrlPoints[k].first * _CtrlPoints[k].first * _CtrlPoints[k].first; 
		mat[k][1] = _CtrlPoints[k].first * _CtrlPoints[k].first;
		mat[k][2] = _CtrlPoints[k].first;
		mat[k][3] = 1;
	}
	NLMISC::CMatrix m;
	m.set(&mat[0][0]);
	m.invert();
	_Coeffs = m * NLMISC::CVectorH(_CtrlPoints[0].second, _CtrlPoints[1].second, _CtrlPoints[2].second, _CtrlPoints[3].second);
	_CoeffsTouched = false;
}


class CPSFloatLagrange : public CPSAttribMakerT<float, CPSFloatLagrangeFunctor>
{
public:
	CPSFloatLagrange() : CPSAttribMakerT<float, CPSFloatLagrangeFunctor>(1) {}
	NLMISC_DECLARE_CLASS(CPSFloatLagrange);
	CPSAttribMakerBase *clone() const { return new CPSFloatLagrange(*this); }
};




} // NL3D
