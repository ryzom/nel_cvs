/** \file hierarchical_timer.cpp
 * Hierarchical timer
 *
 * $Id: hierarchical_timer.cpp,v 1.6 2002/05/30 16:17:33 vizerie Exp $
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
#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include <map>

namespace NLMISC
{

     
bool   CSimpleClock::_InitDone = false;
uint64 CSimpleClock::_StartStopNumTicks = 0;


// root node for all execution paths
CHTimer::CNode  CHTimer::_RootNode;
CHTimer::CNode *CHTimer::_CurrNode = &_RootNode;
CSimpleClock	CHTimer::_PreambuleClock;
CHTimer			CHTimer::_RootTimer("root");
bool			CHTimer::_Benching = false;
bool			CHTimer::_BenchStartedOnce = false;
double			CHTimer::_MsPerTick;
bool			CHTimer::_WantStandardDeviation = false;
CHTimer		   *CHTimer::_CurrTimer = &_RootTimer;







//=================================================================
void CSimpleClock::init()
{	
	const uint numSamples = 10000;

	CSimpleClock observedClock;
	CSimpleClock measuringClock;
	
	measuringClock.start();
	for(uint l = 0; l < numSamples; ++l)
	{		
		observedClock.start();
		observedClock.stop();
	}
	measuringClock.stop();

	_StartStopNumTicks = (measuringClock.getNumTicks() >> 1) / numSamples;	
	_InitDone = true;
}



//=================================================================
/** Do simple statistics on a list of values (mean value, standard deviation) 
  */ 
/*static void PerformStatistics(const std::vector<double> &values, double &standardDeviation)
{
	nlassert(!values.empty());
	double total = 0;	
	double variance = 0;
	uint k;
	for(k = 0; k < values.size(); ++k)
	{
		total += (double) values[k];		
	}
	meanValue = total / values.size();
	if (values.size() <= 1)
	{
		standardDeviation = 0.f;
		return;
	}
	for(k = 0; k < values.size(); ++k)
	{
		variance += NLMISC::sqr((values[k] - meanValue));
	}
	standardDeviation = ::sqrt(variance / values.size() - 1);
}*/



//=================================================================
CHTimer::CNode::~CNode()
{
	releaseSons();
	for(uint k = 0; k < Sons.size(); ++k)
		delete Sons[k];
}

//=================================================================
void CHTimer::CNode::releaseSons()
{
	for(uint k = 0; k < Sons.size(); ++k)
		delete Sons[k];
	Sons.clear();
}

//=================================================================
void CHTimer::CNode::displayPath() const
{
	std::string path;
	getPath(path);
	nlinfo(("FEHTIMER> " + path).c_str());
}

//=================================================================
void CHTimer::CNode::getPath(std::string &path) const
{
	path.clear();
	const CNode *currNode = this;
	do
	{
		path = path.empty() ? currNode->Owner->getName() 
			: currNode->Owner->getName() + std::string("::") + path;
		currNode = currNode->Parent;
	}
	while (currNode);
}


//=================================================================
uint CHTimer::CNode::getNumNodes() const
{
	uint sum = 1;
	for(uint k = 0; k < Sons.size(); ++k)
	{
		sum += Sons[k]->getNumNodes();
	}
	return sum;
}


//=================================================================
void CHTimer::walkTreeToCurrent()
{
	if (_CurrNode->Owner == this) return;
	bool found = false;
	for(uint k = 0; k < _CurrNode->Sons.size(); ++k)
	{
		if (_CurrNode->Sons[k]->Owner == this)
		{
			_CurrNode = _CurrNode->Sons[k];
			found = true;
			break;
		}
	}
	if (!found)
	{
		// no node for this execution path : create a new one
		_CurrNode->Sons.push_back(new CNode(this, _CurrNode));
		_CurrNode->Sons.back()->Parent = _CurrNode;
		_CurrNode = _CurrNode->Sons.back();
	}
}

#ifdef	NL_CPU_INTEL
//=================================================================
uint64 CHTimer::getProcessorFrequency()
{
	static uint64 freq;
	static bool freqComputed = false;	
	if (freqComputed) return freq;

	TTicks bestNumTicks   = 0;
	uint64 bestNumCycles;
	uint64 numCycles;
	const uint numSamples = 5;
	const uint numLoops   = 50000000;
	
	volatile uint k; // prevent optimisation for the loop
	volatile dummy = 0;
	for(uint l = 0; l < numSamples; ++l)
	{	
		TTicks startTick = NLMISC::CTime::getPerformanceTime();
		uint64 startCycle = rdtsc();
		uint dummy = 0;
		for(k = 0; k < numLoops; ++k)
		{		
			++ dummy;
		}		
		numCycles = rdtsc() - startCycle;
		TTicks numTicks = NLMISC::CTime::getPerformanceTime() - startTick;
		if (numTicks > bestNumTicks)
		{		
			bestNumTicks  = numTicks;
			bestNumCycles = numCycles;
		}
	}
	freqComputed = true;
	freq = (uint64) ((double) bestNumCycles * 1 / CTime::ticksToSecond(bestNumTicks));
	return freq;
	nlassert(0);
	return 0;
}
#endif


//=================================================================
void	CHTimer::startBench(bool wantStandardDeviation /*= false*/)
{
	nlassert(!_Benching)
	clear();
	_Benching = true;
	_BenchStartedOnce = true;
	_RootNode.Owner = &_RootTimer;
#	ifdef NL_CPU_INTEL
		double freq = (double) getProcessorFrequency();
		_MsPerTick = 1000 / (double) freq;
#	else
		_MsPerTick = ticksToSecond(1000);
#	endif
	CSimpleClock::init();
	_RootNode.Owner = &_RootTimer;
	_WantStandardDeviation = wantStandardDeviation;
	_RootTimer.before();
}

//=================================================================
void	CHTimer::endBench()
{
	nlassert(_Benching);
	if (_CurrNode == &_RootNode)
	{
		_RootTimer.after();
	}
	else
	{
		nlwarning("FEHTIMER> Stopping the bench inside a benched functions !");
	}
	_Benching = false;
}

//=================================================================
void	CHTimer::display(TSortCriterion criterion, bool displayInline /*= true*/, bool displayEx)
{	
	CSimpleClock	benchClock;
	benchClock.start();
	nlinfo("=========================================================================");
	nlinfo("Bench cumuled results");
	nlassert(_BenchStartedOnce); // should have done at least one bench	
	typedef std::map<CHTimer *, TNodeVect> TNodeMap;
	TNodeMap nodeMap;
	TNodeVect nodeLeft;	
	nodeLeft.push_back(&_RootNode);
	/// 1 ) walk the tree to build the node map (well, in a not very optimal way..)		  
	while (!nodeLeft.empty())
	{	
		CNode *currNode = nodeLeft.back();
		nodeMap[currNode->Owner].push_back(currNode);
		nodeLeft.pop_back();
		nodeLeft.insert(nodeLeft.end(), currNode->Sons.begin(), currNode->Sons.end());

	}
	//	
	// 2 ) build statistics	
	typedef std::vector<CTimerStat> TTimerStatVect;
	typedef std::vector<CTimerStat *> TTimerStatPtrVect;
	TTimerStatVect		stats(nodeMap.size());
	TTimerStatPtrVect	statsPtr(stats.size());
	//
	uint k = 0;
	for(TNodeMap::iterator it = nodeMap.begin(); it != nodeMap.end(); ++it)
	{
		statsPtr[k] = &stats[k];
		stats[k].Timer = it->first;
		stats[k].buildFromNodes(&(it->second[0]), it->second.size(), _MsPerTick);
		++k;
	}
	// 3 ) sort statistics
	if (criterion != NoSort)
	{
		CStatSorter sorter(criterion);
		std::sort(statsPtr.begin(), statsPtr.end(), sorter);		
	}
	// 4 ) display statistics

	uint maxNodeLenght = 0;
	std::string format;
	if (displayInline)
	{
		for(TTimerStatPtrVect::iterator statIt = statsPtr.begin(); statIt != statsPtr.end(); ++statIt)
		{
			maxNodeLenght = std::max(maxNodeLenght, strlen((*statIt)->Timer->_Name));
		}
		format = "FEHTIMER> %-" + NLMISC::toString(maxNodeLenght + 1) + "s %s";
	}
	std::string statsInline;
	for(TTimerStatPtrVect::iterator statIt = statsPtr.begin(); statIt != statsPtr.end(); ++statIt)
	{
		if (!displayInline)
		{		
			nlinfo("FEHTIMER> =================================");
			nlinfo("FEHTIMER> Node %s", (*statIt)->Timer->_Name);		
			(*statIt)->display(displayEx, _WantStandardDeviation);
		}
		else
		{
			(*statIt)->getStats(statsInline, displayEx, _WantStandardDeviation);
			char out[4096];
			NLMISC::smprintf(out, 2048, format.c_str(), (*statIt)->Timer->_Name, statsInline.c_str());
			nlinfo(out);					
		}
	}	
	benchClock.stop();
	_CurrNode->SonsPreambule += benchClock.getNumTicks();
}

//================================================================================================
void		CHTimer::displayByExecutionPath(TSortCriterion criterion, bool displayInline, bool alignPaths, bool displayEx)
{	
	CSimpleClock	benchClock;
	benchClock.start();
	nlinfo("=========================================================================");
	nlinfo("Bench by execution path");
	nlassert(_BenchStartedOnce); // should have done at least one bench	
	bool wasBenching = _Benching;	
	//
	typedef std::vector<CNodeStat>   TNodeStatVect;
	typedef std::vector<CNodeStat *> TNodeStatPtrVect;

	TNodeStatVect nodeStats;
	nodeStats.reserve(_RootNode.getNumNodes());
	TNodeVect nodeLeft;	
	nodeLeft.push_back(&_RootNode);
	/// 1 ) walk the tree to build the node map (well, in a not very optimal way..)		  
	while (!nodeLeft.empty())
	{	
		CNode *currNode = nodeLeft.back();
		
		nodeStats.push_back();
		nodeStats.back().buildFromNodes(&currNode, 1, _MsPerTick);
		nodeStats.back().Node = currNode;

		nodeLeft.pop_back();
		nodeLeft.insert(nodeLeft.end(), currNode->Sons.begin(), currNode->Sons.end());

	}

	/// 2 ) sort statistics
	// create a pointer list
	TNodeStatPtrVect nodeStatsPtrs(nodeStats.size());
	for(uint k = 0; k < nodeStats.size(); ++k)
	{
		nodeStatsPtrs[k] = &nodeStats[k];
	}

	// 3 ) sort statistics
	if (criterion != NoSort)
	{
		CStatSorter sorter(criterion);
		std::sort(nodeStatsPtrs.begin(), nodeStatsPtrs.end(), sorter);
	}

	// 4 ) display statistics
	std::string statsInline;
	std::string nodePath;

	std::string format;
	if (displayInline)
	{
		if (alignPaths)
		{
			uint maxSize = 0;
			std::string nodePath;
			for(TNodeStatPtrVect::iterator it = nodeStatsPtrs.begin(); it != nodeStatsPtrs.end(); ++it)
			{
				(*it)->Node->getPath(nodePath);
				maxSize = std::max(maxSize, nodePath.size());
			}
			format = "FEHTIMER> %-" + NLMISC::toString(maxSize) +"s %s";
		}
		else
		{
			format = "FEHTIMER> %s %s";
		}
	}

	for(TNodeStatPtrVect::iterator it = nodeStatsPtrs.begin(); it != nodeStatsPtrs.end(); ++it)
	{
		if (!displayInline)
		{		
			nlinfo("FEHTIMER> =================================");
			(*it)->Node->displayPath();
			(*it)->display(displayEx, _WantStandardDeviation);
		}
		else
		{
			(*it)->getStats(statsInline, displayEx, _WantStandardDeviation);
			(*it)->Node->getPath(nodePath);

			char out[2048];
			NLMISC::smprintf(out, 2048, format.c_str(), nodePath.c_str(), statsInline.c_str());
			nlinfo(out);
		}
	}
	benchClock.stop();
	_CurrNode->SonsPreambule += benchClock.getNumTicks();
}

//=================================================================
/*static*/ void CHTimer::displayHierarchical(bool displayEx /*=true*/,uint labelNumChar /*=32*/, uint indentationStep /*= 2*/)
{
	CSimpleClock	benchClock;
	benchClock.start();
	nlinfo("=========================================================================");
	nlinfo("Hierarchical display of bench");
	nlassert(_BenchStartedOnce); // should have done at least one bench
	bool wasBenching = _Benching;	
	typedef std::map<CHTimer *, TNodeVect> TNodeMap;
	TNodeMap nodeMap;
	TNodeVect nodeLeft;	
	nodeLeft.push_back(&_RootNode);
	/// 1 ) walk the execution tree to build the node map (well, in a not very optimal way..)		  
	while (!nodeLeft.empty())
	{	
		CNode *currNode = nodeLeft.back();
		nodeMap[currNode->Owner].push_back(currNode);
		nodeLeft.pop_back();
		nodeLeft.insert(nodeLeft.end(), currNode->Sons.begin(), currNode->Sons.end());

	}
	/// 2 ) walk the timers tree and display infos (cumulate infos of nodes of each execution path)
	CStats	currNodeStats;
	std::vector<uint> sonsIndex;
	uint depth = 0;
	CHTimer *currTimer = &_RootTimer;
	sonsIndex.push_back(0);
	bool displayStat = true;
	std::string resultName;
	std::string resultStats;
	while (!sonsIndex.empty())
	{		
		if (displayStat)
		{
			resultName.resize(labelNumChar);
			std::fill(resultName.begin(), resultName.end(), '.');
			uint startIndex = depth * indentationStep;
			uint endIndex = std::min(startIndex + ::strlen(currTimer->_Name), labelNumChar);			
			if ((sint) (endIndex - startIndex) >= 1)
			{
				std::copy(currTimer->_Name, currTimer->_Name + (endIndex - startIndex), resultName.begin() + startIndex);
			}
			TNodeVect &execNodes = nodeMap[currTimer];
			currNodeStats.buildFromNodes(&execNodes[0], execNodes.size(), _MsPerTick);			
			currNodeStats.getStats(resultStats, displayEx, _WantStandardDeviation);
			nlinfo((resultName + resultStats).c_str());
		}
		if (sonsIndex.back() == currTimer->_Sons.size())
		{
			sonsIndex.pop_back();
			currTimer = currTimer->_Parent;
			displayStat = false;
			-- depth;
		}
		else
		{
			currTimer = currTimer->_Sons[sonsIndex.back()];
			++ sonsIndex.back();
			sonsIndex.push_back(0);			
			displayStat = true;
			++ depth;
		}
	}	
	benchClock.stop();
	_CurrNode->SonsPreambule += benchClock.getNumTicks();
}

//=================================================================
void	CHTimer::clear()
{
	// should not be benching !
	nlassert(_CurrNode == &_RootNode)
	_RootNode.releaseSons();
	_CurrNode = &_RootNode;
	_RootNode.reset();	
}

//=================================================================
void CHTimer::CStats::buildFromNodes(CNode **nodes, uint numNodes, double msPerTick)
{
	TotalTime = 0;	
	TotalTimeWithoutSons = 0;	
	NumVisits = 0;
	
	uint64 minTime = (uint64) -1;
	uint64 maxTime = 0;
	
	uint k, l;
	for(k = 0; k < numNodes; ++k)
	{		
		TotalTime += nodes[k]->TotalTime * msPerTick;
		TotalTimeWithoutSons += (nodes[k]->TotalTime -  nodes[k]->LastSonsTotalTime) * msPerTick;
		NumVisits += nodes[k]->NumVisits;
		minTime = std::min(minTime, nodes[k]->MinTime);
		maxTime = std::max(maxTime, nodes[k]->MaxTime);				
	}
	if (minTime == (uint64) -1) 
	{
		minTime = 0;
	}
	MinTime  = minTime * msPerTick;
	MaxTime  = maxTime * msPerTick;
	MeanTime = TotalTime / NumVisits;

	// compute standard deviation
	double varianceSum = 0;
	uint   numMeasures = 0;
	for(k = 0; k < numNodes; ++k)
	{
		numMeasures += nodes[k]->Measures.size();
		for(l = 0; l < nodes[k]->Measures.size(); ++l)
		{
			varianceSum += NLMISC::sqr(nodes[k]->Measures[l] - MeanTime);
		}		
	}
	TimeStandardDeviation = numMeasures == 0 ? 0
											 : ::sqrt(varianceSum / (numMeasures +1));
}

//=================================================================
void CHTimer::CStats::display(bool displayEx, bool wantStandardDeviation /* = false*/)
{
	nlinfo("FEHTIMER> Total time                = %.3f ms", (float) TotalTime);
	nlinfo("FEHTIMER> Total time without sons   = %.3f ms", (float) TotalTimeWithoutSons);
	nlinfo(("FEHTIMER> Num visits                = " + NLMISC::toString(NumVisits)).c_str());	
	if (displayEx)
	{
			nlinfo("FEHTIMER> Min time                  = %.3f ms", (float) MinTime);
			nlinfo("FEHTIMER> Max time                  = %.3f ms", (float) MaxTime);
			nlinfo("FEHTIMER> Mean time                 = %.3f ms", (float) MeanTime);
			if (wantStandardDeviation)
			{
			nlinfo("FEHTIMER> Standard deviation        = %.3f ms", (float) TimeStandardDeviation);
			}
			//nlinfo("Time standard deviation	= %.3f ms", (float) TimeStandardDeviation);
	}
}


//=================================================================
void CHTimer::CStats::getStats(std::string &dest, bool statEx, bool wantStandardDeviation /*= false*/)
{
	char buf[1024];
	if (!wantStandardDeviation)
	{	
		if (!statEx)
		{	
			NLMISC::smprintf(buf, 1024, " | total  %5.3f  | local  %5.3f | visits  %12s ", (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str());
		}
		else
		{
			NLMISC::smprintf(buf, 1024, " | total  %5.3f  | local  %5.3f | visits  %12s | min %5.3f | max %5.3f | mean %5.3f",
					  (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str(), 
					  (float) MinTime, (float) MaxTime, (float) MeanTime
					 );
		}
	}
	else
	{
		if (!statEx)
		{	
			NLMISC::smprintf(buf, 1024, " | total  %5.3f  | local  %5.3f | visits  %12s | std deviation %5.3f", (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str(), (float) TimeStandardDeviation);
		}
		else
		{
			NLMISC::smprintf(buf, 1024, " | total  %5.3f  | local  %5.3f | visits  %12s | min %12.3f | max %5.3f | mean %5.3f | std deviation %5.3f",
							  (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str(), 
							  (float) MinTime, (float) MaxTime, (float) MeanTime,
							  (float) TimeStandardDeviation
							);
		}
	}
	dest = buf;
}


//=================================================================
bool CHTimer::CStatSorter::operator()(const CHTimer::CStats *lhs, const CHTimer::CStats *rhs)
{
	switch(Criterion)
	{
		case CHTimer::TotalTime:				return lhs->TotalTime >= rhs->TotalTime;
		case CHTimer::TotalTimeWithoutSons:		return lhs->TotalTimeWithoutSons >= rhs->TotalTimeWithoutSons;
		case CHTimer::MeanTime:					return lhs->MeanTime >= rhs->MeanTime;
		case CHTimer::NumVisits:				return lhs->NumVisits >= rhs->NumVisits;
		case CHTimer::MaxTime:					return lhs->MaxTime >= rhs->MaxTime;
		case CHTimer::MinTime:					return lhs->MinTime < rhs->MinTime;	
		default:
			nlassert(0); // not a valid criterion
		break;
	}	
}



} // NLMISC

