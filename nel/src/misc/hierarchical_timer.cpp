/** \file hierarchical_timer.cpp
 * Hierarchical timer
 *
 * $Id: hierarchical_timer.cpp,v 1.30 2003/10/20 16:10:17 lecroart Exp $
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
#include "nel/misc/command.h"
#include "nel/misc/system_info.h"

#ifdef NL_CPU_INTEL
#include "nel/misc/time_nl.h"
#endif

#include <map>

namespace NLMISC
{

bool   CSimpleClock::_InitDone = false;
uint64 CSimpleClock::_StartStopNumTicks = 0;


// root node for all execution paths
CHTimer::CNode  CHTimer::_RootNode;
CHTimer::CNode *CHTimer::_CurrNode = &_RootNode;
CSimpleClock	CHTimer::_PreambuleClock;
CHTimer			CHTimer::_RootTimer("root", true);
bool			CHTimer::_Benching = false;
bool			CHTimer::_BenchStartedOnce = false;
double			CHTimer::_MsPerTick;
bool			CHTimer::_WantStandardDeviation = false;
CHTimer		   *CHTimer::_CurrTimer = &_RootTimer;







//=================================================================
void CSimpleClock::init()
{	
	if (_InitDone) return;
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
void CHTimer::CNode::displayPath(CLog *log) const
{
	std::string path;
	getPath(path);
	log->displayRawNL(("HTIMER: " + path).c_str());
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
	if (_IsRoot) return;	
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



//=================================================================
void	CHTimer::startBench(bool wantStandardDeviation /*= false*/, bool quick, bool reset)
{
	nlassert(!_Benching);

	if(reset)
		clear();

	_Benching = true;
	_BenchStartedOnce = true;
	_RootNode.Owner = &_RootTimer;

	if(reset)
	{
#ifdef NL_CPU_INTEL
		double freq = (double) CSystemInfo::getProcessorFrequency(quick);
		_MsPerTick = 1000 / (double) freq;
#else
		_MsPerTick = CTime::ticksToSecond(1000);
#endif
		CSimpleClock::init();
	}

	_RootNode.Owner = &_RootTimer;
	_WantStandardDeviation = wantStandardDeviation;
	_RootTimer.before();
}

//=================================================================
void	CHTimer::endBench()
{
	if (!_Benching)
		return;

	if (_CurrNode == &_RootNode)
	{
		_RootTimer.after();
	}
	else
	{
		nlwarning("HTIMER: Stopping the bench inside a benched functions !");
	}
	_Benching = false;
}

//=================================================================
void	CHTimer::display(CLog *log, TSortCriterion criterion, bool displayInline /*= true*/, bool displayEx)
{	
	CSimpleClock	benchClock;
	benchClock.start();
	if(!_BenchStartedOnce) // should have done at least one bench	
	{
		benchClock.stop();
		_CurrNode->SonsPreambule += benchClock.getNumTicks();
		return;
	}
	log->displayNL("HTIMER: =========================================================================");
	log->displayRawNL("HTIMER: Bench cumuled results");
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

	// 4 ) get root total time.
	CStats	rootStats;
	rootStats.buildFromNode( &_RootNode, _MsPerTick);

	// 5 ) display statistics
	uint maxNodeLenght = 0;
	std::string format;
	if (displayInline)
	{
		for(TTimerStatPtrVect::iterator statIt = statsPtr.begin(); statIt != statsPtr.end(); ++statIt)
		{
			maxNodeLenght = std::max(maxNodeLenght, strlen((*statIt)->Timer->_Name));
		}
		format = "HTIMER: %-" + NLMISC::toString(maxNodeLenght + 1) + "s %s";
	}
	std::string statsInline;

	log->displayRawNL(format.c_str(), "", " |      total |      local |       visits |  loc%/ glb% |       min |       max |      mean");

	for(TTimerStatPtrVect::iterator statIt = statsPtr.begin(); statIt != statsPtr.end(); ++statIt)
	{
		if (!displayInline)
		{		
			log->displayRawNL("HTIMER: =================================");
			log->displayRawNL("HTIMER: Node %s", (*statIt)->Timer->_Name);		
			(*statIt)->display(log, displayEx, _WantStandardDeviation);
		}
		else
		{
			(*statIt)->getStats(statsInline, displayEx, rootStats.TotalTime, _WantStandardDeviation);
			char out[4096];
			NLMISC::smprintf(out, 2048, format.c_str(), (*statIt)->Timer->_Name, statsInline.c_str());
			log->displayRawNL(out);					
		}
	}	
	benchClock.stop();
	_CurrNode->SonsPreambule += benchClock.getNumTicks();
}

//================================================================================================
void		CHTimer::displayByExecutionPath(CLog *log, TSortCriterion criterion, bool displayInline, bool alignPaths, bool displayEx)
{	
	CSimpleClock	benchClock;
	benchClock.start();
	log->displayRawNL("HTIMER: =========================================================================");
	log->displayRawNL("HTIMER: Bench by execution path");
	nlassert(_BenchStartedOnce); // should have done at least one bench	
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
		
		nodeStats.push_back(CNodeStat());
		nodeStats.back().buildFromNode(currNode, _MsPerTick);
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

	// 4 ) get root total time.
	CStats	rootStats;
	rootStats.buildFromNode(&_RootNode, _MsPerTick);

	// 5 ) display statistics
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
			format = "HTIMER: %-" + NLMISC::toString(maxSize) +"s %s";
		}
		else
		{
			format = "HTIMER: %s %s";
		}
	}

	log->displayRawNL(format.c_str(), "", " |      total |      local |       visits |  loc%/ glb% |       min |       max |      mean");

	for(TNodeStatPtrVect::iterator it = nodeStatsPtrs.begin(); it != nodeStatsPtrs.end(); ++it)
	{
		if (!displayInline)
		{		
			log->displayRawNL("HTIMER: =================================");
			(*it)->Node->displayPath(log);
			(*it)->display(log, displayEx, _WantStandardDeviation);
		}
		else
		{
			(*it)->getStats(statsInline, displayEx, rootStats.TotalTime, _WantStandardDeviation);
			(*it)->Node->getPath(nodePath);

			char out[2048];
			NLMISC::smprintf(out, 2048, format.c_str(), nodePath.c_str(), statsInline.c_str());
			log->displayRawNL(out);
		}
	}
	benchClock.stop();
	_CurrNode->SonsPreambule += benchClock.getNumTicks();
}

//=================================================================
/*static*/ void CHTimer::displayHierarchical(CLog *log, bool displayEx /*=true*/,uint labelNumChar /*=32*/, uint indentationStep /*= 2*/)
{
	CSimpleClock	benchClock;
	benchClock.start();
	log->displayNL("HTIMER: =========================================================================");
	log->displayRawNL("HTIMER: Hierarchical display of bench");
	nlassert(_BenchStartedOnce); // should have done at least one bench
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
	log->displayRawNL("HTIMER: %*s |      total |      local |       visits |  loc%%/ glb%% |       min |       max |      mean", labelNumChar, "");

	/// 2 ) get root total time.
	CStats	rootStats;
	rootStats.buildFromNode(&_RootNode, _MsPerTick);

	/// 3 ) walk the timers tree and display infos (cumulate infos of nodes of each execution path)
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
			if (execNodes.size() > 0)
			{
				currNodeStats.buildFromNodes(&execNodes[0], execNodes.size(), _MsPerTick);			
				currNodeStats.getStats(resultStats, displayEx, rootStats.TotalTime, _WantStandardDeviation);
				log->displayRawNL("HTIMER: %s", (resultName + resultStats).c_str());
			}
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
/*static*/ void		CHTimer::displayHierarchicalByExecutionPath(CLog *log, bool displayEx, uint labelNumChar, uint indentationStep)
{
	displayHierarchicalByExecutionPathSorted(log, NoSort, displayEx, labelNumChar, indentationStep);
}


//=================================================================
/*static*/ void		CHTimer::displayHierarchicalByExecutionPathSorted(CLog *log, TSortCriterion criterion, bool displayEx, uint labelNumChar, uint indentationStep)
{

	CSimpleClock	benchClock;
	benchClock.start();
	nlassert(_BenchStartedOnce); // should have done at least one bench

	// get root total time.
	CStats	rootStats;
	rootStats.buildFromNode(&_RootNode, _MsPerTick);


	// display header.
	CLog::TDisplayInfo	dummyDspInfo;
	log->displayNL("HTIMER: =========================================================================");
	log->displayRawNL("HTIMER: Hierarchical display of bench by execution path");
	log->displayRawNL("HTIMER: %*s |      total |      local |       visits |  loc%%/ glb%% |       min |       max |      mean", labelNumChar, "");


	// use list because vector of vector is bad.
	std::list< CExamStackEntry >	examStack;

	// Add the root to the stack.
	examStack.push_back( CExamStackEntry( &_RootNode ) );
	CStats		currNodeStats;
	std::string resultName;
	std::string resultStats;

	while (!examStack.empty())
	{
		CNode				*node = examStack.back().Node;
		std::vector<CNode*>	&children= examStack.back().Children;
		uint				child = examStack.back().CurrentChild;

		// If child 0, then must first build children info and display me.
		if (child == 0)
		{
			// Build Sons Infos.
			// ==============
			
			// resize array
			children.resize(node->Sons.size());

			// If no sort, easy.
			if(criterion == NoSort)
			{
				children= node->Sons;
			}
			// else, Sort them with criterion.
			else
			{
				std::vector<CNodeStat>		stats;
				std::vector<CNodeStat *>	ptrStats;
				stats.resize(children.size());
				ptrStats.resize(children.size());

				// build stats.
				uint	i;
				for(i=0; i<children.size(); i++)
				{
					CNode	*childNode= node->Sons[i];
					stats[i].buildFromNode(childNode, _MsPerTick);
					stats[i].Node = childNode;
					ptrStats[i]= &stats[i];
				}

				// sort.
				CStatSorter	sorter;
				sorter.Criterion= criterion;
				std::sort(ptrStats.begin(), ptrStats.end(), sorter);		

				// fill children.
				for(i=0; i<children.size(); i++)
				{
					children[i]= ptrStats[i]->Node;
				}
			}


			// Display our infos
			// ==============
			// build the indented node name.
			resultName.resize(labelNumChar);
			std::fill(resultName.begin(), resultName.end(), '.');
			uint startIndex = (examStack.size()-1) * indentationStep;
			uint endIndex = std::min(startIndex + ::strlen(node->Owner->_Name), labelNumChar);			
			if ((sint) (endIndex - startIndex) >= 1)
			{
				std::copy(node->Owner->_Name, node->Owner->_Name + (endIndex - startIndex), resultName.begin() + startIndex);
			}

			// build the stats string.
			currNodeStats.buildFromNode(node, _MsPerTick);			
			currNodeStats.getStats(resultStats, displayEx, rootStats.TotalTime, _WantStandardDeviation);

			// display
			log->displayRawNL("HTIMER: %s", (resultName + resultStats).c_str());
		}

		// End of sons?? stop.
		if (child >= children.size())
		{
			examStack.pop_back();
			continue;
		}

		// next son.
		++(examStack.back().CurrentChild);

		// process the current son.
		examStack.push_back( CExamStackEntry( children[child] ) );
	}

	//
	benchClock.stop();
	_CurrNode->SonsPreambule += benchClock.getNumTicks();
}

//=================================================================
void	CHTimer::clear()
{
	// should not be benching !
	nlassert(_CurrNode == &_RootNode);
	_RootNode.releaseSons();
	_CurrNode = &_RootNode;
	_RootNode.reset();	
}

//=================================================================
void CHTimer::CStats::buildFromNode(CNode *node, double msPerTick)
{
	buildFromNodes(&node, 1, msPerTick);
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
	if (NumVisits > 0)
		MeanTime = TotalTime / NumVisits;
	else
		MeanTime = 0.0;

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
void CHTimer::CStats::display(CLog *log, bool displayEx, bool wantStandardDeviation /* = false*/)
{
	log->displayRawNL("HTIMER: Total time                = %.3f ms", (float) TotalTime);
	log->displayRawNL("HTIMER: Total time without sons   = %.3f ms", (float) TotalTimeWithoutSons);
	log->displayRawNL(("HTIMER: Num visits                = " + NLMISC::toString(NumVisits)).c_str());	
	if (displayEx)
	{
			log->displayRawNL("HTIMER: Min time                  = %.3f ms", (float) MinTime);
			log->displayRawNL("HTIMER: Max time                  = %.3f ms", (float) MaxTime);
			log->displayRawNL("HTIMER: Mean time                 = %.3f ms", (float) MeanTime);
			if (wantStandardDeviation)
			{
			log->displayRawNL("HTIMER: Standard deviation        = %.3f ms", (float) TimeStandardDeviation);
			}
			//log->displayRawNL("Time standard deviation	= %.3f ms", (float) TimeStandardDeviation);
	}
}


//=================================================================
void CHTimer::CStats::getStats(std::string &dest, bool statEx, double rootTotalTime, bool wantStandardDeviation /*= false*/)
{
	char buf[1024];
	if (!wantStandardDeviation)
	{	
		if (!statEx)
		{	
			NLMISC::smprintf(buf, 1024, " | %10.3f | %10.3f | %12s ", (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str());
		}
		else
		{
			NLMISC::smprintf(buf, 1024, " | %10.3f | %10.3f | %12s | %5.1f/%5.1f | %9.3f | %9.3f | %9.3f",
					  (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str(), 
					  float(100*TotalTimeWithoutSons/rootTotalTime), float(100*TotalTime/rootTotalTime), 
					  (float) MinTime, (float) MaxTime, (float) MeanTime
					 );
		}
	}
	else
	{
		if (!statEx)
		{	
			NLMISC::smprintf(buf, 1024, " | %10.3f | %10.3f | %12s | std deviation %9.3f", (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str(), (float) TimeStandardDeviation);
		}
		else
		{
			NLMISC::smprintf(buf, 1024, " | %10.3f | %10.3f | %12s | %5.1f/%5.1f | %9.3f | %9.3f | %9.3f | std deviation %9.3f",
							  (float) TotalTime, (float) TotalTimeWithoutSons, toString(NumVisits).c_str(), 
							  float(100*TotalTimeWithoutSons/rootTotalTime), float(100*TotalTime/rootTotalTime), 
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
	return false;
}


//===============================================
void	CHTimer::before()
{	
	if (!_Benching) return;
	_PreambuleClock.start();	
	walkTreeToCurrent();			
	++ _CurrNode->NumVisits;
	_CurrNode->SonsPreambule = 0;
	if (!_Parent && _CurrTimer != this)
	{
		_Parent = _CurrTimer;
		// register as a son of the parent
		_Parent->_Sons.push_back(this); 
	}
	_CurrTimer = this;
	_PreambuleClock.stop();
	if (_CurrNode->Parent)
	{	
		_CurrNode->Parent->SonsPreambule += _PreambuleClock.getNumTicks();
	}
	_CurrNode->Clock.start();
}

//===============================================
void	CHTimer::after(bool displayAfter /*= false*/)
{
	if (!_Benching) return;
	_CurrNode->Clock.stop();		
	_PreambuleClock.start();
	//		
	//nlinfo((std::string("clock ") + _Name + std::string(" time = ") + NLMISC::toString(_CurrNode->Clock.getNumTicks())).c_str());
	sint64 numTicks = _CurrNode->Clock.getNumTicks()  - _CurrNode->SonsPreambule - (CSimpleClock::getStartStopNumTicks() << 1);
	numTicks= std::max((sint64)0, numTicks);

	_CurrNode->TotalTime += numTicks;		
	_CurrNode->MinTime = std::min(_CurrNode->MinTime, (uint64)numTicks);
	_CurrNode->MaxTime = std::max(_CurrNode->MaxTime, (uint64)numTicks);
	_CurrNode->LastSonsTotalTime = _CurrNode->SonsTotalTime;

	if (displayAfter)
	{		
		nlinfo("HTIMER: %s %.3fms loop number %d", _Name, numTicks * _MsPerTick, _CurrNode->NumVisits);
	}
	//
	if (_WantStandardDeviation)
	{
		_CurrNode->Measures.push_back(numTicks * _MsPerTick);
	}
	//
	if (_Parent)
	{
		_CurrTimer = _Parent;
	}	
	//
	if (_CurrNode->Parent)
	{
		_PreambuleClock.stop();
		_CurrNode = _CurrNode->Parent;
		_CurrNode->SonsTotalTime += numTicks;
		_CurrNode->SonsPreambule += _PreambuleClock.getNumTicks();
	}
	else
	{
		_PreambuleClock.stop();
	}
}


//
// Commands
//

NLMISC_COMMAND(displayMeasures, "display hierarchical timer", "")
{
	CHTimer::display(&log);
	CHTimer::displayHierarchicalByExecutionPathSorted (&log, CHTimer::TotalTime, true, 64);
	return true;
}

} // NLMISC

