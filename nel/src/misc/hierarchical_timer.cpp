/** \file hierarchical_timer.cpp
 * Hierarchical timer
 *
 * $Id: hierarchical_timer.cpp,v 1.1 2002/03/13 13:52:28 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdmisc.h"

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/debug.h"

#ifdef ALLOW_TIMING_MEASURES

std::vector<CHTimer*>		CHTimer::_Stack;
std::vector<CHTimer*>		CHTimer::_Roots;
double						CHTimer::_TicksPerTest;
double						CHTimer::_MsPerTick;


void	CHTimer::bench()
{
	clear();
	uint	i;
	CHTimer		t("Test");
	CHTimer		g("Global");

	NLMISC::TTicks	st = NLMISC::CTime::getPerformanceTime();
	g.before();
	for (i=0; i<10000; ++i)
	{
		t.before();
		t.after();
	}
	NLMISC::TTicks	et = NLMISC::CTime::getPerformanceTime();
	g.after();

	_TicksPerTest = (double)t._TotalTicks / (double)i;
	_MsPerTick = NLMISC::CTime::ticksToSecond(et-st)*1000.0 / (double)g._TotalTicks;
	clear();
}

void	CHTimer::display()
{
	uint	i;
	nlinfo("FEHTIMER> _TicksPerTest = %.2f", _TicksPerTest);
	nlinfo("FEHTIMER> _MsPerTick = %g (%.0f MHz)", _MsPerTick, 2.0/(1000.0*_MsPerTick));
	for (i=0; i<_Roots.size(); ++i)
		_Roots[i]->displayNode(0, _Roots[i]);
}

void	CHTimer::clear()
{
	_Stack.clear();
	uint	i;
	for (i=0; i<_Roots.size(); ++i)
		_Roots[i]->clearNode();
	_Roots.clear();
}

void	CHTimer::adjust()
{
	uint	i;
	for (i=0; i<_Roots.size(); ++i)
		_Roots[i]->adjustNode();
}

//

uint32	CHTimer::adjustNode()
{
	uint32	totalTests = _NumTests;

	uint	i;
	for (i=0; i<_Children.size(); ++i)
		totalTests += _Children[i]->adjustNode();

	_AdjustedTicks = _TotalTicks - (sint64)(totalTests*_TicksPerTest);
	return totalTests;
}

//

void	CHTimer::displayNode(uint level, CHTimer *root)
{
	static char	str[256];

	uint	i;

	str[0] = 0;
	for (i=0; i<level; ++i)
		strcat(str, "  ");

	strcat(str, _Name);

	if (level == 0)
	{
		nlinfo("FEHTIMER> %s %.3fms %d loops", str, _AdjustedTicks*_MsPerTick/(double)_NumTests, _NumTests);
	}
	else if (level == 1)
	{
//		nlinfo("> %s%s: %.2f global", str, _Name, 100.0*(double)_TotalTicks/(double)root->_TotalTicks);
		nlinfo("FEHTIMER> %-32s%5.2f global               %8.3fms/loop, mean=%8.3fms min=%8.3fms max=%8.3fms ticks=%-11"NL_I64"d tests=%-9d", str, 100.0*(double)_AdjustedTicks/(double)root->_AdjustedTicks, _AdjustedTicks*_MsPerTick/(double)root->_NumTests,  _AdjustedTicks*_MsPerTick/(double)_NumTests, _MsPerTick*_MinTicks, _MsPerTick*_MaxTicks, (sint64)_AdjustedTicks, _NumTests);
	}
	else
	{
//		nlinfo("> %s%s: %.2f global, %.2f local", str, _Name, 100.0*(double)_TotalTicks/(double)root->_TotalTicks, 100.0*(double)_TotalTicks/(double)_Parent->_TotalTicks);
		nlinfo("FEHTIMER> %-32s%5.2f global, %5.2f local, %8.3fms/loop, mean=%8.3fms min=%8.3fms max=%8.3fms ticks=%-11"NL_I64"d tests=%-9d", str, 100.0*(double)_AdjustedTicks/(double)root->_AdjustedTicks, 100.0*(double)_AdjustedTicks/(double)_Parent->_AdjustedTicks, _AdjustedTicks*_MsPerTick/(double)root->_NumTests, _AdjustedTicks*_MsPerTick/(double)_NumTests, _MsPerTick*_MinTicks, _MsPerTick*_MaxTicks, (sint64)_AdjustedTicks, _NumTests);
	}

	if (_Children.size() != 0)
	{
		sint64	totalChildren = 0;
		for (i=0; i<_Children.size(); ++i)
			totalChildren += _Children[i]->_AdjustedTicks;

		str[0] = 0;
		for (i=0; i<level+1; ++i)
			strcat(str, "  ");

		strcat(str, "[unmeasured]");

		sint64	totalLeft = _AdjustedTicks - totalChildren;
		nlinfo("FEHTIMER> %-32s%5.2f global, %5.2f local,                                                                ticks=%-11"NL_I64"d", str, 100.0*(double)totalLeft/(double)root->_AdjustedTicks, 100.0*(double)totalLeft/(double)_AdjustedTicks, (sint64)totalLeft);
	}

	for (i=0; i<_Children.size(); ++i)
		_Children[i]->displayNode(level+1, root);
}

#endif // ALLOW_TIMING_MEASURES
