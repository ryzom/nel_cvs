/** \file log_report.cpp
 * <File description>
 *
 * $Id: log_report.cpp,v 1.1 2004/03/01 19:38:50 cado Exp $
 */

/* Copyright, 2000-2004 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "log_report.h"
#include "nel/misc/common.h"
#include "nel/misc/displayer.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/variable.h"

using namespace NLMISC;
using namespace std;


CVariable<string> LogPath( "LogPath", "Path of the log files", ".", 0, true );

const uint MAX_LOG_LINE_SIZE = 1024;
//nlctassert(MAX_LOG_LINE_SIZE>0);

CLogReport MainLogReport;

enum TLogLineHeader { LHDate, LHTime, LHType, LHThread, LHService, LHCodeFile, LHCodeLine, LHSeparator, LH_NB_FIELDS };


///
bool isLogFile( const std::string& filename )
{
	return (filename.find( ".log" ) != string::npos);
}

///
inline bool	isNumberChar( char c )
{
	return (c >= '0') && (c <= '9');
}

///
void sortLogFiles( vector<std::string>& filenames )
{
	uint i;
	for ( i=0; i!=filenames.size(); ++i )
	{
		// Ensure that a log file without number comes *after* the ones with a number
		string name = string(filenames[i]);
		unsigned int dotpos = name.find_last_of('.');
		if ( (dotpos!=string::npos) && (dotpos > 2) )
		{
			if ( ! (isNumberChar(name[dotpos-1]) && isNumberChar(name[dotpos-2]) && isNumberChar(name[dotpos-3])) )
			{
				name = name.substr( 0, dotpos ) + "ZZZ" + name.substr( dotpos );
				filenames[i] = name.c_str();
			}
		}
	}
	sort( filenames.begin(), filenames.end() );
	for ( i=0; i!=filenames.size(); ++i )
	{
		// Set the original names back
		string name = filenames[i];
		unsigned int tokenpos = name.find( "ZZZ." );
		if ( tokenpos != string::npos )
		{
			name = name.substr( 0, tokenpos ) + name.substr( tokenpos + 3 );
			filenames[i] = name.c_str();
		}
	}
}


/*
 *
 */
void	CMakeLogTask::run()
{
	// Get log files and sort them
	string path = LogPath.get();
	if ( (! path.empty()) && (path[path.size()-1]!='/') )
		path += "/";
	vector<string> filenames;
	CPath::getPathContent( path, false, false, true, filenames, NULL, true );
	vector<string>::iterator ilf = partition( filenames.begin(), filenames.end(), isLogFile );
	filenames.erase( ilf, filenames.end() );
	sortLogFiles( filenames );

	// Analyse log files
	MainLogReport.reset();
	uint nbLines = 0;
	char line [MAX_LOG_LINE_SIZE];

	uint skippedLogDotLog = 0;
	for ( ilf=filenames.begin(); ilf!=filenames.end(); ++ilf )
	{
		string shortname = CFile::getFilename( *ilf );
		if ( (shortname == "log.log") ||
			 ((shortname.size() == 10) &&
			  (shortname.substr( 0, 3 ) == "log") &&
			  isNumberChar(shortname[3]) && isNumberChar(shortname[4]) && isNumberChar(shortname[5]) &&
			  (shortname.substr( 6, 4 ) == ".log")) )
		{
			++skippedLogDotLog;
			continue;
		}
		nlinfo( "Processing %s (%u/%u)", (*ilf).c_str(), ilf-filenames.begin(), filenames.size() );
		CIFile logfile;
		if ( logfile.open( *ilf, true ) )
		{
			while ( ! logfile.eof() )
			{
				logfile.getline( line, MAX_LOG_LINE_SIZE );
				line[MAX_LOG_LINE_SIZE-1] = '\0'; // force valid end of line
				MainLogReport.pushLine( line );
				++nbLines;

				if ( isStopping() )
					return;
			}
		}
	}
	nlinfo( "%u lines processed", nbLines );
	if ( skippedLogDotLog != 0 )
		nlinfo( "%u log[NNN].log files skipped", skippedLogDotLog );
	_Complete = true;
}


/*
 * Add a log line to the report tree
 */
void	CLogReport::pushLine( const std::string& line, NLMISC::CLog::TLogType only )
{
	// Ignore session title
	if ( (line.size() > 14) && (line.substr( 0, 14 ) == "Log Starting [") )
		return;

	// Decode standard log line
	vector<string> lineTokens;
	explode( line, " ", lineTokens );
	if ( lineTokens.size() < LH_NB_FIELDS )
		return;

	// Filter log type
	if ( only != CLog::LOG_UNKNOWN )
	{
		if ( lineTokens[LHType] != IDisplayer::logTypeToString( only ) )
			return;
	}

	// Store
	storeLine( lineTokens );
}


/*
 *
 */
void	CLogReportNode::storeLine( const std::vector<std::string>& lineTokens )
{
	// Get service name from "[machine/]serviceName-serviceId"
	string service = lineTokens[LHService];
	string::size_type p = service.find( '/' );
	if ( p != string::npos )
		service = service.substr( p+1 );
	p = service.find( '-' );
	if ( p != string::npos )
		service = service.substr( 0, p );

	// Store to appropriate child
	CLogReportLeaf *child = getChild( service );
	if ( ! child )
		child = addChild( service );
	child->storeLine( lineTokens );
}


/*
 *
 */
void	CLogReportLeaf::storeLine( const std::vector<std::string>& lineTokens )
{
	// Build key from "codeFile codeLine"
	string key = lineTokens[LHCodeFile] + ":" + lineTokens[LHCodeLine];
	_LogLineInfo[key].addAnOccurence( lineTokens );
	++_TotalLines;
}


/*
 *
 */
void	CLogLineInfo::addAnOccurence( const std::vector<std::string>& lineTokens )
{
	if ( NbOccurences == 0 )
	{
		for ( uint i=LH_NB_FIELDS; i<lineTokens.size(); ++i )
		{
			if ( i != LH_NB_FIELDS )
				SampleLogText += " ";
			SampleLogText += lineTokens[i];
		}
	}
	++NbOccurences;
}


/*
 * Get results for a service
 */
void	CLogReport::reportByService( const std::string& service, NLMISC::CLog *targetLog )
{
	ILogReport *child = getChild( service );
	if ( child )
	{
		child->report( targetLog );
	}
	else
	{
		targetLog->displayNL( "Nothing found for service %s", service.c_str() );
	}
}


/*
 * Get results for a service (all distinct lines, sorted by occurence)
 */
void	CLogReportLeaf::report( NLMISC::CLog *targetLog )
{
	// Sort it
	typedef multimap< uint, pair< string, const CLogLineInfo * >, greater<uint> > CSortedByOccurenceLogLineInfoMap;
	CSortedByOccurenceLogLineInfoMap sortedByOccurence;
	for ( CLogLineInfoMap::const_iterator it=_LogLineInfo.begin(); it!=_LogLineInfo.end(); ++it )
	{
		const string &key = (*it).first;
		const CLogLineInfo& info = (*it).second;
		sortedByOccurence.insert( make_pair( info.NbOccurences, make_pair( key, &info ) ) );
	}

	// Display it
	for ( CSortedByOccurenceLogLineInfoMap::const_iterator iso=sortedByOccurence.begin(); iso!=sortedByOccurence.end(); ++iso )
	{
		const string &key = (*iso).second.first;
		const CLogLineInfo& info = *((*iso).second.second);
		targetLog->displayRawNL( "%s %6u %s : %s", _Service.c_str(), info.NbOccurences, key.c_str(), info.SampleLogText.c_str() );
	}
}


/*
 *
 */
uint	CLogReportLeaf::reportPart( uint beginIndex, uint maxNbLines, NLMISC::CLog *targetLog )
{
	uint i = 0;
	CLogLineInfoMap::const_iterator it;
	for ( it=_LogLineInfo.begin(); it!=_LogLineInfo.end(); ++it )
	{
		if ( i >= beginIndex )
		{
			if ( i >= maxNbLines )
				return i - beginIndex;

			const string &key = (*it).first;
			const CLogLineInfo& info = (*it).second;
			targetLog->displayRawNL( "%s %6u %s : %s", _Service.c_str(), info.NbOccurences, key.c_str(), info.SampleLogText.c_str() );
		}
		++i;
	}
	return i - beginIndex;
}


/*
 * Get summary of results
 */
void	CLogReportNode::report( NLMISC::CLog *targetLog )
{
	uint nb1Sum=0, nb2Sum=0;
	for ( std::vector<CLogReportLeaf*>::const_iterator it=_Children.begin(); it!=_Children.end(); ++it )
	{
		CLogReportLeaf *pt = (*it);
		uint nb1 = pt->getNbDistinctLines();
		nb1Sum += nb1;
		uint nb2 = pt->getNbTotalLines();
		targetLog->displayRawNL( "%s: %u distinct, %u total", pt->service().c_str(), nb1, nb2 );
		nb2Sum += nb2;
	}
	targetLog->displayRawNL( "=> %u distinct, %u total (%u pages)", nb1Sum, nb2Sum, nb1Sum / NB_LINES_PER_PAGE + 1 );
}


/*
 * Get partial results (pageNum>=1)
 */
void	CLogReportNode::reportPage( uint pageNum, NLMISC::CLog *targetLog )
{
	if ( _Children.empty() )
	{
		targetLog->displayRawNL( "[END OF LOG]" );
		return;
	}

	uint beginIndex = pageNum * NB_LINES_PER_PAGE;
	uint lineCounter = 0, prevLineCounter;
	for ( std::vector<CLogReportLeaf*>::const_iterator it=_Children.begin(); it!=_Children.end(); ++it )
	{
		CLogReportLeaf *pt = (*it);
		prevLineCounter = lineCounter;
		lineCounter += pt->getNbDistinctLines();
		if ( lineCounter >= beginIndex )
		{
			uint remainingLines = pageNum - (lineCounter - beginIndex );
			pt->reportPart( beginIndex - prevLineCounter, remainingLines, targetLog ); //
			while ( remainingLines != 0 )
			{
				++it;
				if ( it == _Children.end() )
				{
					targetLog->displayRawNL( "[END OF LOG]" );
					return;
				}
				pt = (*it);
				remainingLines -= pt->reportPart( 0, remainingLines, targetLog );
			}
			return;
		}
	}	
}
