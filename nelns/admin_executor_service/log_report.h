/** \file log_report.h
 * <File description>
 *
 * $Id: log_report.h,v 1.3.22.1 2005/02/21 13:46:08 lancon Exp $
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

#ifndef NL_LOG_REPORT_H
#define NL_LOG_REPORT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/log.h"
#include "nel/misc/thread.h"
#include <string>
#include <vector>
#include <map>


/*
 *
 */
class CMakeLogTask : public NLMISC::IRunnable
{
public:

	/// Constructor
	CMakeLogTask() : _Stopping(false), _Complete(false) {}

	/// Destructor
	~CMakeLogTask()
	{
		if ( _Thread )
		{
			if ( ! _Complete )
			{
				pleaseStop();
				_Thread->wait();
			}
			delete _Thread;
			_Thread = NULL;
		}
	}

	/// Start (called in main thread)
	void	start()
	{
		if ( _Thread && (!_Complete) )
			return;
		_Stopping = false;
		_Complete = false;
		_Thread = NLMISC::IThread::create( this );
		_Thread->start();
	}

	/// Ask for stop and wait until terminated (called in main thread)
	void	terminateTask()
	{
		pleaseStop();
		_Thread->wait();
		delete _Thread;
		_Thread = NULL;
	}

	///
	bool	isRunning() const { return (_Thread != NULL) && (!_Complete); }

	///
	bool	isStopping() const { return _Stopping; }

	///
	bool	isComplete() const { return _Complete; }

	virtual void run();


	/// Set the path of logfile directory. 
	void setLogPath(const std::string & logPath);

	/// Set the path of logfile directory to default value
	void setLogPathToDefault();




private:

	void	pleaseStop() { _Stopping = true; }

	volatile bool	_Stopping;
	volatile bool	_Complete;
	std::string _DefaultLogPath;

	NLMISC::IThread	*_Thread;
};


const uint NB_LINES_PER_PAGE = 100;


/**
 *
 */
class CLogLineInfo
{
public:
	
	CLogLineInfo() : NbOccurences(0) {}

	void				addAnOccurence( const std::vector<std::string>& lineTokens );

	uint				NbOccurences;

	std::string			SampleLogText;
};


/**
 *
 */
class ILogReport
{
public:

	virtual void storeLine( const std::vector<std::string>& lineTokens, bool mainCountOnly ) = 0;

	virtual void report( NLMISC::CLog *targetLog, bool detailed ) = 0;

	virtual uint getNbDistinctLines() const = 0;

	virtual uint getNbTotalLines( NLMISC::CLog::TLogType logType ) = 0;

	virtual ~ILogReport() {}
};


/**
 * For one service (service name, not service instance), store info about log lines
 */
class CLogReportLeaf : public ILogReport
{
public:

	/// Constructor
	CLogReportLeaf( const std::string& service ) : _Service(service), _TotalLines(0) {}

	std::string		service() { return _Service; }

	virtual uint	getNbDistinctLines() const { return _LogLineInfo.size(); }

	virtual uint	getNbTotalLines( NLMISC::CLog::TLogType logType );

	virtual void	storeLine( const std::vector<std::string>& lineTokens, bool mainCountOnly );

	virtual void	report( NLMISC::CLog *targetLog, bool detailed );

	uint			reportPart( uint beginIndex, uint maxNbLines, NLMISC::CLog *targetLog );

protected:

	typedef std::map< std::string, CLogLineInfo > CLogLineInfoMap;

	std::string						_Service;

	CLogLineInfoMap					_LogLineInfo;

	std::map< std::string, uint >	_Counts; // indexed by log type string
	
	uint							_TotalLines;
};


/**
 * Store info about log lines for several services
 */
class CLogReportNode : public ILogReport
{
public:

	/// Constructor
	CLogReportNode() {}

	/// Destructor
	~CLogReportNode() { reset(); }

	/// Clear all
	void	reset()
	{
		for ( std::vector<CLogReportLeaf*>::iterator it=_Children.begin(); it!=_Children.end(); ++it )
			delete (*it);
		_Children.clear();
	}

protected:

	virtual void				storeLine( const std::vector<std::string>& lineTokens, bool mainCountOnly=false );

	CLogReportLeaf*				getChild( const std::string& service )
	{
		for ( std::vector<CLogReportLeaf*>::iterator it=_Children.begin(); it!=_Children.end(); ++it )
		{
			if ( (*it)->service() == service )
				return (*it);
		}
		return NULL;
	}

	CLogReportLeaf*				addChild( const std::string& service )
	{
		CLogReportLeaf *child = new CLogReportLeaf( service );
		_Children.push_back( child );
		return child;
	}

	virtual void	report( NLMISC::CLog *targetLog, bool displayDetailsPerService );

	virtual uint	getNbDistinctLines() const
	{
		uint n = 0;
		for ( std::vector<CLogReportLeaf*>::const_iterator it=_Children.begin(); it!=_Children.end(); ++it )
			n += (*it)->getNbDistinctLines();
		return n;
	}

	virtual uint	getNbTotalLines( NLMISC::CLog::TLogType logType=NLMISC::CLog::LOG_UNKNOWN )
	{
		uint n = 0;
		for ( std::vector<CLogReportLeaf*>::const_iterator it=_Children.begin(); it!=_Children.end(); ++it )
			n += (*it)->getNbTotalLines( logType );
		return n;
	}

	void			reportPage( uint pageNum, NLMISC::CLog *targetLog );

private:

	std::vector<CLogReportLeaf*>	_Children;
};


/*
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2004
 */
class CLogReport : public CLogReportNode
{
public:

	/// Constructor
	CLogReport() : _CurrentFile(~0), _TotalFiles(~0) {}

	/// Clear all
	void	reset() { CLogReportNode::reset(); }

	/**
	 * Add a log line to the report tree.
	 * \param onlyType Type of log to study. If LOG_UNKNOWN, study all.
	 * \param countOtherTypes If true and onlyType not LOG_UNKNOWN, count the number of lines of each other type found.
	 */
	void	pushLine( const std::string& line, NLMISC::CLog::TLogType onlyType=NLMISC::CLog::LOG_WARNING, bool countOtherTypes=true );

	/// Set the current progress
	void	setProgress( uint currentFile, uint totalFiles ) { _CurrentFile = currentFile; _TotalFiles = totalFiles; }

	/// Get the current progress
	void	getProgress( uint& currentFile, uint& totalFiles ) { currentFile = _CurrentFile; totalFiles = _TotalFiles; }

	/// Get results for a service
	void	reportByService( const std::string& service, NLMISC::CLog *targetLog );

	/// Get partial results (pageNum>=1)
	void	reportPage( uint pageNum, NLMISC::CLog *targetLog ) { CLogReportNode::reportPage( pageNum, targetLog ); }

	/// Get summary of results
	virtual void report( NLMISC::CLog *targetLog, bool displayDetailsPerService=false ) { CLogReportNode::report( targetLog, displayDetailsPerService ); }

private:

	uint	_CurrentFile;
	uint	_TotalFiles;
};


extern CLogReport MainLogReport;


#endif // NL_LOG_REPORT_H

/* End of log_report.h */
