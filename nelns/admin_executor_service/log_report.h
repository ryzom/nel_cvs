/** \file log_report.h
 * <File description>
 *
 * $Id: log_report.h,v 1.1 2004/03/01 19:38:50 cado Exp $
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

	CMakeLogTask() : _Stopping(false), _Complete(false) {}

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

private:

	void	pleaseStop() { _Stopping = true; }

	volatile bool	_Stopping;
	volatile bool	_Complete;

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

	virtual void storeLine( const std::vector<std::string>& lineTokens ) = 0;

	virtual void report( NLMISC::CLog *targetLog ) = 0;

	virtual uint getNbDistinctLines() const = 0;

	virtual uint getNbTotalLines() const = 0;
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

	virtual uint	getNbTotalLines() const { return _TotalLines; };

	virtual void	storeLine( const std::vector<std::string>& lineTokens );

	virtual void	report( NLMISC::CLog *targetLog );

	uint			reportPart( uint beginIndex, uint maxNbLines, NLMISC::CLog *targetLog );

protected:

	typedef std::map< std::string, CLogLineInfo > CLogLineInfoMap;

	CLogLineInfoMap		_LogLineInfo;

	std::string			_Service;

	uint				_TotalLines;
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

	virtual void				storeLine( const std::vector<std::string>& lineTokens );

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

	virtual void	report( NLMISC::CLog *targetLog );

	virtual uint	getNbDistinctLines() const
	{
		uint n = 0;
		for ( std::vector<CLogReportLeaf*>::const_iterator it=_Children.begin(); it!=_Children.end(); ++it )
			n += (*it)->getNbDistinctLines();
		return n;
	}

	virtual uint	getNbTotalLines() const
	{
		uint n = 0;
		for ( std::vector<CLogReportLeaf*>::const_iterator it=_Children.begin(); it!=_Children.end(); ++it )
			n += (*it)->getNbTotalLines();
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
	///
	CLogReport() {}

	/// Clear all
	void	reset() { CLogReportNode::reset(); }

	/**
	 * Add a log line to the report tree.
	 * \param only Type of log to retain. If LOG_UNKNOWN, retain all.
	 */
	void	pushLine( const std::string& line, NLMISC::CLog::TLogType only=NLMISC::CLog::LOG_WARNING );

	/// Get results for a service
	void	reportByService( const std::string& service, NLMISC::CLog *targetLog );

	/// Get partial results (pageNum>=1)
	void	reportPage( uint pageNum, NLMISC::CLog *targetLog ) { CLogReportNode::reportPage( pageNum, targetLog ); }

	/// Get summary of results
	virtual void report( NLMISC::CLog *targetLog ) { CLogReportNode::report( targetLog ); }

};


extern CLogReport MainLogReport;


#endif // NL_LOG_REPORT_H

/* End of log_report.h */
