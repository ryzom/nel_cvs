/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.205 2004/03/15 15:17:23 cado Exp $
 *
 * \todo ace: test the signal redirection on Unix
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdnet.h"

//
// Includes
//

#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	include <windows.h>
#	include <direct.h>
#elif defined NL_OS_UNIX
#	include <unistd.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mutex.h"
#include "nel/misc/window_displayer.h"
#include "nel/misc/gtk_displayer.h"
#include "nel/misc/win_displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/report.h"
#include "nel/misc/system_info.h"
#include "nel/misc/timeout_assertion_thread.h"

#include "nel/net/naming_client.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/net_manager.h"
#include "nel/net/net_displayer.h"
#include "nel/net/email.h"
#include "nel/net/varpath.h"
#include "nel/net/admin.h"

#include "nel/memory/memory_manager.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;

namespace NLNET
{


//
// Constants
//

static const sint Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

static const char *SignalName[]=
{
  "SIGABRT", "SIGFPE", "SIGILL", "SIGINT", "SIGSEGV", "SIGTERM"
};

static const char* NegFiltersNames[] =
{
   "NegFiltersDebug",
   "NegFiltersInfo",
   "NegFiltersWarning",
   "NegFiltersAssert",
   "NegFiltersError",
   0
};
	

//
// Variables
//

TUnifiedCallbackItem EmptyCallbackArray[1] = { { "", NULL } };

// class static member
IService	*IService::_Instance = NULL;

static sint ExitSignalAsked = 0;

// services stat
CVariable<sint32> UserSpeedLoop ("UserSpeedLoop", "duration of the last network loop (in ms)", 10, false);
CVariable<sint32> NetSpeedLoop ("NetSpeedLoop", "duration of the last user loop (in ms)", 10, false);


// this is the thread that initialized the signal redirection
// we'll ignore other thread signals
static uint SignalisedThread;

static CFileDisplayer fd;
static CNetDisplayer commandDisplayer(false);
//static CLog commandLog;

static string CompilationDate;
static uint32 LaunchingDate;

static uint32 NbUserUpdate = 0;

#ifdef NL_RELEASE_DEBUG
string CompilationMode = "NL_RELEASE_DEBUG";
#elif defined(NL_DEBUG_FAST)
string CompilationMode = "NL_DEBUG_FAST";
#elif defined(NL_DEBUG)
string CompilationMode = "NL_DEBUG";
#elif defined(NL_RELEASE)
string CompilationMode = "NL_RELEASE";
#else
string CompilationMode = "???";
#endif

//static bool Bench = false;

CVariable<bool> Bench ("Bench", "1 if benching 0 if not", 0, true);

// This produce an assertion in the thread if the update loop is too slow
static CTimeoutAssertionThread	MyTAT;
static void						UpdateAssertionThreadTimeoutCB(IVariable &var) { MyTAT.timeout(atoi(var.toString().c_str())); }
static CVariable<uint32>		UpdateAssertionThreadTimeout("UpdateAssertionThreadTimeout", "in millisecond, timeout before thread assertion", 0, 0, true, UpdateAssertionThreadTimeoutCB);


//
// Signals managing
//

// This function is called when a signal comes
static void sigHandler(int Sig)
{
	// redirect the signal for the next time
	signal(Sig, sigHandler);

	// find the signal
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		if (Sig == Signal[i])
		{
			if (getThreadId () != SignalisedThread)
			{
				nldebug ("SERVICE: Not the main thread received the signal (%s, %d), ignore it", SignalName[i],Sig);
				return;
			}
			else
			{
				nlinfo ("SERVICE: Signal %s (%d) received", SignalName[i], Sig);
				switch (Sig)
				{
				case SIGABRT :
				case SIGILL  :
				case SIGINT  :
				case SIGSEGV :
				case SIGTERM :
				// you should not call a function and system function like printf in a SigHandle because
				// signal-handler routines are usually called asynchronously when an interrupt occurs.
				if (ExitSignalAsked == 0)
				{
					nlinfo ("SERVICE: Receive a signal that said that i must exit");
					ExitSignalAsked = Sig;
					return;
				}
				else
				{
					nlinfo ("SERVICE: Signal already received, launch the brutal exit");
					exit (EXIT_FAILURE);
				}
				break;
				}
			}
		}
	}
	nlwarning ("SERVICE: Unknown signal received (%d)", Sig);
}

// Initialise the signal redirection
static void initSignal()
{
	SignalisedThread = getThreadId ();
#ifdef NL_DEBUG
	// in debug mode, we only trap the SIGINT signal
	signal(Signal[3], sigHandler);
	//nldebug("Signal : %s (%d) trapped", SignalName[3], Signal[3]);
#else
	// in release, redirect all signals
/* don't redirect now because to hard to debug...
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		signal(Signal[i], sigHandler);
		nldebug("Signal %s (%d) trapped", SignalName[i], Signal[i]);
	}
*/
#endif
}

void cbDirectoryChanged (IVariable &var)
{
	string vp = CPath::getFullPath(var.toString());
	nlinfo ("SERVICE: '%s' changed to '%s'", var.getName().c_str(), vp.c_str());
	var.fromString(vp);

	if (var.getName() == "RunningDirectory")
	{
#ifdef NL_OS_WINDOWS
		_chdir (vp.c_str());
#else
		chdir (vp.c_str());
#endif
	}
}


//
// Service built-in callbacks
//

void cbReceiveShardId (CMessage& msgin, const string &serviceName, uint16 serviceId)
{
	uint32	shardId;
	msgin.serial(shardId);

	if (serviceName != "WS")
	{
		nlwarning("SERVICE: received unauthorized R_SH_ID callback from service %s-%d asking to set ShardId to %d", serviceName.c_str(), serviceId, shardId);
		return;
	}

	nlinfo("SERVICE: set ShardId to %d", shardId);
	IService::getInstance()->_ShardId = shardId;
}

TUnifiedCallbackItem builtinServiceCallbacks [] =
{
	{ "R_SH_ID", cbReceiveShardId },
};



//
// Class implementation
//

// Ctor
IService::IService() :
	WindowDisplayer(0),
	WriteFilesDirectory("WriteFilesDirectory", "directory where to save generic shard information (packed_sheets for example)", ".", 0, true, cbDirectoryChanged),
	SaveFilesDirectory("SaveFilesDirectory", "directory where to save specific shard information (shard time for example)", ".", 0, true, cbDirectoryChanged),
	ListeningPort("ListeningPort", "listening port for this service", 0, 0, true),
	_RecordingState(CCallbackNetBase::Off),
	_UpdateTimeout(100),
	_SId(0),
	_Status(0),
	_Initialized(false),
	ConfigDirectory("ConfigDirectory", "directory where config files are", ".", 0, true, cbDirectoryChanged),
	LogDirectory("LogDirectory", "directory where the service is logging", ".", 0, true, cbDirectoryChanged),
	RunningDirectory("RunningDirectory", "directory where the service is running on", ".", 0, true, cbDirectoryChanged),
	Version("Version", "Version of the shard", ""),
	_CallbackArray (0),
	_CallbackArraySize (0),
	_DontUseNS(false),
	_DontUseAES(false),
	_ResetMeasures(false)
{
	// Singleton
	_Instance = this;
}



bool IService::haveArg (char argName)
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if (_Args[i][1] == argName)
			{
				return true;
			}
		}
	}
	return false;
}

string IService::getArg (char argName)
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if (_Args[i][1] == argName)
			{
				return _Args[i].substr(2);
			}
		}
	}
	throw Exception ("Parameter '-%c' is not found in command line", argName);
}


void IService::setArgs (const char *args)
{
	_Args.push_back ("<ProgramName>");

	string sargs (args);
	uint32 pos1 = 0, pos2 = 0;

	do
	{
		pos1 = sargs.find_first_not_of (" ", pos2);
		if (pos1 == string::npos) break;
		pos2 = sargs.find_first_of (" ", pos1);
		_Args.push_back (sargs.substr (pos1, pos2-pos1));
	}
	while (pos2 != string::npos);
}

void IService::setArgs (int argc, const char **argv)
{
	for (sint i = 0; i < argc; i++)
	{
		_Args.push_back (argv[i]);
	}
}


void cbLogFilter (CConfigFile::CVar &var)
{
	CLog *log = NULL;
	if (var.Name == "NegFiltersDebug")
	{
		log = DebugLog;
	}
	else if (var.Name == "NegFiltersInfo")
	{
		log = InfoLog;
	}
	else if (var.Name == "NegFiltersWarning")
	{
		log = WarningLog;
	}
	else if (var.Name == "NegFiltersAssert")
	{
		log = AssertLog;
	}
	else if (var.Name == "NegFiltersError")
	{
		log = ErrorLog;
	}
	else
	{
		nlstop;
	}

	nlinfo ("SERVICE: Updating %s from config file", var.Name.c_str());
	
	// remove all old filters from config file
	CConfigFile::CVar &oldvar = IService::getInstance()->ConfigFile.getVar (var.Name);
	for (sint j = 0; j < oldvar.size(); j++)
	{
		log->removeFilter (oldvar.asString(j).c_str());
	}

	// add all new filters from config file
	for (sint i = 0; i < var.size(); i++)
	{
		log->addNegativeFilter (var.asString(i).c_str());
	}
}

void cbExecuteCommands (CConfigFile::CVar &var)
{
	for (sint i = 0; i < var.size(); i++)
	{
		ICommand::execute (var.asString(i), IService::getInstance()->CommandLog);
	}
}



//
// The main function of the service
//

sint IService::main (const char *serviceShortName, const char *serviceLongName, uint16 servicePort, const char *configDir, const char *logDir, const char *compilationDate)
{
	bool userInitCalled = false;
//	bool resyncEvenly = false;
	CConfigFile::CVar *var = NULL;

	// a short name service can't be a number
	nlassert (atoi(serviceShortName) == 0);

	try
	{
		//
		// Init parameters
		//

		_ShortName = serviceShortName;
		CLog::setProcessName (_ShortName);
		
		// get the path where to run the service if any in the command line
		if (haveArg('A'))
			RunningDirectory = CPath::standardizePath(getArg('A'));

		ConfigDirectory = CPath::standardizePath(configDir);
		LogDirectory = CPath::standardizePath(logDir);
		_LongName = serviceLongName;

		CompilationDate = compilationDate;

		LaunchingDate = CTime::getSecondsSince1970();

		ListeningPort = servicePort;

		setReportEmailFunction ((void*)sendEmail);
		setDefaultEmailParams ("gw.nevrax.com", "", "lecroart@nevrax.com");


		//
		// Load the config file
		//
		
		// get the config file dir if any in the command line
		if (haveArg('C'))
			ConfigDirectory = CPath::standardizePath(getArg('C'));
		
		string cfn = ConfigDirectory.c_str() + _LongName + ".cfg";
		if (!CFile::fileExists(ConfigDirectory.c_str() + _LongName + ".cfg"))
		{
			// check if the default exists
			if (!CFile::fileExists(ConfigDirectory.c_str() + _LongName + "_default.cfg"))
			{
				nlerror ("SERVICE: The config file '%s' is not found, neither the default one, can't launch the service", cfn.c_str());
			}
			else
			{
				// create the basic .cfg that link the default one
				FILE *fp = fopen (cfn.c_str(), "w");
				if (fp == NULL)
				{
					nlerror ("SERVICE: Can't create config file '%s'", cfn.c_str());
				}
				fprintf(fp, "// link the default config file for %s\n", _LongName.c_str());
				fprintf(fp, "RootConfigFilename = \"%s_default.cfg\";\n", _LongName.c_str());
				fclose (fp);
			}
		}	
		
		ConfigFile.load (cfn);
		
		// setup variable with config file variable
		IVariable::init (ConfigFile);
		
		// we have to call this again because the config file can changed this variable but the cmd line is more prioritary
		if (haveArg('A'))
			RunningDirectory = CPath::standardizePath(getArg('A'));


		//
		// Init debug/log stuffs (must be first things otherwise we can't log if errors)
		//

		// get the log dir if any in the command line
		if (haveArg('L'))
			LogDirectory = CPath::standardizePath(getArg('L'));

		changeLogDirectory (LogDirectory);

		// we create the log with service name filename ("test_service.log" for example)
		fd.setParam (LogDirectory.c_str() + _LongName + ".log", false);

		DebugLog->addDisplayer (&fd);
		InfoLog->addDisplayer (&fd);
		WarningLog->addDisplayer (&fd);
		AssertLog->addDisplayer (&fd);
		ErrorLog->addDisplayer (&fd);
		CommandLog.addDisplayer (&fd, true);

		//
		// Init the hierarchical timer
		//

		CHTimer::startBench(false, true);
		CHTimer::endBench();


		//
		// Set the assert mode
		//

		if (ConfigFile.exists ("Assert"))
			setAssert (ConfigFile.getVar("Assert").asInt() == 1);

		//
		// Set the shard Id
		//

		if ((var = ConfigFile.getVarPtr("NoWSShardId")) != NULL)
		{
			_ShardId = var->asInt();
		}
		else
		{
			// something high enough as default
			_ShardId = 666;
		}

		//
		// Set the negative filter from the config file
		//

		for(const char **name = NegFiltersNames; *name; name++)
		{
			if ((var = ConfigFile.getVarPtr (*name)) != NULL)
			{
				ConfigFile.setCallback (*name, cbLogFilter);
				cbLogFilter(*var);
			}
		}

		ConfigFile.setCallback ("Commands", cbExecuteCommands);
		if ((var = ConfigFile.getVarPtr ("Commands")) != NULL)
		{
			cbExecuteCommands(*var);
		}
		
		
		//
		// Create the window if needed
		//

		if ((var = ConfigFile.getVarPtr ("WindowStyle")) != NULL)
		{
			string disp = var->asString ();
#ifdef NL_USE_GTK
			if (disp == "GTK")
			{
				WindowDisplayer = new CGtkDisplayer ("DEFAULT_WD");
			}
#endif // NL_USE_GTK

#ifdef NL_OS_WINDOWS
			if (disp == "WIN")
			{
				WindowDisplayer = new CWinDisplayer ("DEFAULT_WD");
			}
#endif // NL_OS_WINDOWS

			if (WindowDisplayer == NULL && disp != "NONE")
			{
				nlinfo ("SERVICE: Unknown value for the WindowStyle (should be GTK, WIN or NONE), use no window displayer");
			}
		}

		vector <pair<string,uint> > displayedVariables;
		//uint speedNetLabel, speedUsrLabel, rcvLabel, sndLabel, rcvQLabel, sndQLabel, scrollLabel;
		if (WindowDisplayer != NULL)
		{
			//
			// Init window param if necessary
			//

			sint x=-1, y=-1, w=-1, h=-1, fs=10, history=-1;
			bool iconified = false, ww = false;
			string fn;

			if ((var = ConfigFile.getVarPtr("XWinParam")) != NULL) x = var->asInt();
			if ((var = ConfigFile.getVarPtr("YWinParam")) != NULL) y = var->asInt();
			if ((var = ConfigFile.getVarPtr("WWinParam")) != NULL) w = var->asInt();
			if ((var = ConfigFile.getVarPtr("HWinParam")) != NULL) h = var->asInt();
			if ((var = ConfigFile.getVarPtr("HistoryWinParam")) != NULL) history = var->asInt();
			if ((var = ConfigFile.getVarPtr("IWinParam")) != NULL) iconified = var->asInt() == 1;
			if ((var = ConfigFile.getVarPtr("FontSize")) != NULL) fs = var->asInt();
			if ((var = ConfigFile.getVarPtr("FontName")) != NULL) fn = var->asString();
			if ((var = ConfigFile.getVarPtr("WordWrap")) != NULL) ww = var->asInt() == 1;
			
			if (haveArg('I')) iconified = true;

			WindowDisplayer->create (string("*INIT* ") + _ShortName + " " + _LongName, iconified, x, y, w, h, history, fs, fn, ww, &CommandLog);

			DebugLog->addDisplayer (WindowDisplayer);
			InfoLog->addDisplayer (WindowDisplayer);
			WarningLog->addDisplayer (WindowDisplayer);
			ErrorLog->addDisplayer (WindowDisplayer);
			AssertLog->addDisplayer (WindowDisplayer);
			CommandLog.addDisplayer(WindowDisplayer, true);

			// adding default displayed variables
			displayedVariables.push_back(make_pair(string("NetLop|NetSpeedLoop"), WindowDisplayer->createLabel ("NetLop")));
			displayedVariables.push_back(make_pair(string("UsrLop|UserSpeedLoop"), WindowDisplayer->createLabel ("UsrLop")));
//			displayedVariables.push_back(make_pair(string("Rcv|ReceivedBytes"), WindowDisplayer->createLabel ("Rcv")));
//			displayedVariables.push_back(make_pair(string("Snd|SentBytes"), WindowDisplayer->createLabel ("Snd")));
//			displayedVariables.push_back(make_pair(string("RcvQ|ReceivedQueueSize"), WindowDisplayer->createLabel ("RcvQ")));
//			displayedVariables.push_back(make_pair(string("SndQ|SentQueueSize"), WindowDisplayer->createLabel ("SndQ")));
			displayedVariables.push_back(make_pair(string("|Scroller"), WindowDisplayer->createLabel ("NeL Rulez")));
			
			CConfigFile::CVar *v = ConfigFile.getVarPtr("DisplayedVariables");
			if (v != NULL)
			{
				for (sint i = 0; i < v->size(); i++)
				{
					displayedVariables.push_back(make_pair(v->asString(i), WindowDisplayer->createLabel (v->asString(i).c_str())));
				}
			}
		}

		nlinfo ("SERVICE: Starting Service '%s' using NeL ("__DATE__" "__TIME__") compiled %s", _ShortName.c_str(), CompilationDate.c_str());
		nlinfo ("SERVICE: On OS: %s", CSystemInfo::getOS().c_str());
		
		setStatus (EXIT_SUCCESS);

		//
		// Redirect signal if needed (in release mode only)
		//

#ifdef NL_OS_WINDOWS
#ifdef NL_RELEASE
		initSignal();
#else
		// don't install signal is the application is started in debug mode
		if (IsDebuggerPresent ())
		{
			//nlinfo("Running with the debugger, don't redirect signals");
			initSignal();
		}
		else
		{
			//nlinfo("Running without the debugger, redirect SIGINT signal");
			initSignal();
		}
#endif
#else // NL_OS_UNIX
		initSignal();
#endif


		//
		// Ignore SIGPIPE (broken pipe) on unix system
		//

#ifdef NL_OS_UNIX
		// Ignore the SIGPIPE signal
		sigset_t SigList;
		bool IgnoredPipe = true;
		if (sigemptyset (&SigList) == -1)
		{
			perror("sigemptyset()");
			IgnoredPipe = false;
		}

		if (sigaddset (&SigList, SIGPIPE) == -1)
		{
			perror("sigaddset()");
			IgnoredPipe = false;
		}

		if (sigprocmask (SIG_BLOCK, &SigList, NULL) == -1)
		{
			perror("sigprocmask()");
			IgnoredPipe = false;
		}
		nldebug ("SERVICE: SIGPIPE %s", IgnoredPipe?"Ignored":"Not Ignored");
#endif // NL_OS_UNIX


		//
		// Initialize the network system
		//
		
		string localhost;
		try
		{
			// Initialize WSAStartup and network stuffs
			CSock::initNetwork();

			// Get the localhost name
			localhost = CInetAddress::localHost().hostName();
		}
		catch (NLNET::ESocket &)
		{
			localhost = "<UnknownHost>";
		}

		// Set the localhost name and service name to the logger
		CLog::setProcessName (localhost+"/"+_ShortName);
		nlinfo ("SERVICE: Host: %s", localhost.c_str());
		
		//
		// Initialize server parameters
		//

		// set the listen port if there are a port arg in the command line
		if (haveArg('P'))
		{
			ListeningPort = atoi(getArg('P').c_str());
		}

		// set the aliasname if is present in the command line
		if (haveArg('N'))
		{
			_AliasName = getArg('N');
		}

		// Load the recording state from the config file
		if ((var = ConfigFile.getVarPtr ("Rec")) != NULL)
		{
			string srecstate = var->asString();
			strupr( srecstate );
			if ( srecstate == "RECORD" )
			{
				_RecordingState = CCallbackNetBase::Record;
				nlinfo( "SERVICE: Service recording messages" );
			}
			else if ( srecstate == "REPLAY" )
			{
				_RecordingState = CCallbackNetBase::Replay;
				nlinfo( "SERVICE: Service replaying messages" );
			}
			else
			{
				_RecordingState = CCallbackNetBase::Off;
			}
		}
		else
		{
			// Not found
			_RecordingState = CCallbackNetBase::Off;
		}

		// Load the default stream format
		if ((var = ConfigFile.getVarPtr ("StringMsgFormat")) != NULL)
		{
			CMessage::setDefaultStringMode( var->asInt() == 1 );
		}
		else
		{
			// Not found => binary
			CMessage::setDefaultStringMode( false );
		}

/*
		//
		// Layer4 Startup (Connect to the Naming Service (except for the NS itself and Login Service))
		//

		if (IService::_ShortName != "NS" && IService::_ShortName != "LS" && IService::_ShortName != "AES" && IService::_ShortName != "AS")
		{
			bool ok = false;
			while (!ok)
			{
				// read the naming service address from the config file
				CInetAddress loc(ConfigFile.getVar("NSHost").asString(), ConfigFile.getVar("NSPort").asInt());
				try
				{
					CNetManager::init( &loc, _RecordingState );
					ok = true;
				}
				catch (ESocketConnectionFailed &)
				{
					nlwarning ("Could not connect to the Naming Service (%s). Retrying in a few seconds...", loc.asString().c_str());
					nlSleep (5000);
				}
			}
		}
		else
		{
			CNetManager::init( NULL, _RecordingState );
		}
*/

		///
		/// Layer5 Startup
		///

		// get the sid
		if ((var = ConfigFile.getVarPtr ("SId")) != NULL)
		{
			sint32 sid = var->asInt();
			if (sid<=0 || sid>255)
			{
				nlwarning("SERVICE: Bad SId value in the config file, %d is not in [0;255] range", sid);
				_SId = 0;
			}
			else
			{
				_SId = (uint8) sid;
			}
		}
		else
		{
			// ok, SId not found, use dynamic sid
			_SId = 0;
		}


		// look if we don't want to use NS
		if ((var = ConfigFile.getVarPtr ("DontUseNS")) != NULL)
		{
			// if we set the value in the config file, get it
			_DontUseNS = (var->asInt() == 1);
		}
		else
		{
			// if not, we use ns only if service is not ns, ls, aes, as
			_DontUseNS = false;
		}

		//
		// Register all network associations (must be before the CUnifiedNetwork::getInstance()->init)
		//

		if ((var = ConfigFile.getVarPtr ("Networks")) != NULL)
		{
			for (uint8 i = 0; i < var->size (); i++)
				CUnifiedNetwork::getInstance()->addNetworkAssociation (var->asString(i), i);
		}

		if ((var = ConfigFile.getVarPtr ("DefaultNetworks")) != NULL)
		{
			for (uint8 i = 0; i < var->size (); i++)
				CUnifiedNetwork::getInstance()->addDefaultNetwork(var->asString(i));
		}

		// normal setup for the common services
		if (!_DontUseNS)
		{
			bool ok = false;
			while (!ok)
			{
				string LSAddr;

				if (haveArg('B'))
				{
					// if the naming service address is set on the command line, get it (overwrite the cfg)
					LSAddr = getArg('B');
				}
				else
				{
					// else read the naming service address from the config file
					LSAddr = ConfigFile.getVar ("NSHost").asString();
				}

				// if there's no port to the NS, use the default one 50000
				if (LSAddr.find(":") == string::npos)
					LSAddr += ":50000";

				CInetAddress loc(LSAddr);
				try
				{
					if ( CUnifiedNetwork::getInstance()->init (&loc, _RecordingState, _ShortName, ListeningPort, _SId) )
					{
						ok = true;
					}
					else
					{
						nlinfo( "SERVICE: Exiting..." );
						beep( 880, 400 );
						beep( 440, 400 );
						beep( 220, 400 );
						return 10;
					}
				}
				catch (ESocketConnectionFailed &)
				{
					nlinfo ("SERVICE: Could not connect to the Naming Service (%s). Retrying in a few seconds...", loc.asString().c_str());
					nlSleep (5000);
				}
			}
		}
		else
		{
			CUnifiedNetwork::getInstance()->init(NULL, _RecordingState, _ShortName, ListeningPort, _SId);
		}

		// At this point, the _SId must be ok if we use the naming service.
		// If it's 0, it means that we don't use NS and we left the other side server to find a sid for your connection

		if(!_DontUseNS)
		{
			nlassert (_SId != 0);
		}

		//
		// Connect to the local AES and send identification
		//

		// look if we don't want to use NS
		if ((var = ConfigFile.getVarPtr ("DontUseAES")) != NULL)
		{
			// if we set the value in the config file, get it
			_DontUseAES = var->asInt() == 1;
		}
		else
		{
			// if not, we use aes only if service is not aes or as
			_DontUseAES = false;
		}

		initAdmin (_DontUseAES);
		

		//
		// Add callback array
		//

		// add inner service callback array
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(builtinServiceCallbacks, sizeof(builtinServiceCallbacks)/sizeof(builtinServiceCallbacks[0]));

		// add callback set in the NLNET_SERVICE_MAIN macro
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(_CallbackArray, _CallbackArraySize);

		//
		// Now we have the service id, we can set the entites id generator
		//

		NLMISC::CEntityId::setServiceId(_SId);

		// Set the localhost name and service name and the sid
		CLog::setProcessName (localhost+"/"+_ShortName+"-"+toString((uint16)_SId));


		//
		// Add default pathes
		//

		if ((var = ConfigFile.getVarPtr ("IgnoredFiles")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				CPath::addIgnoredDoubleFile (var->asString(i));
			}
		}

		if ((var = ConfigFile.getVarPtr ("Paths")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), true, false);
			}
		}

		if ((var = ConfigFile.getVarPtr ("PathsNoRecurse")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), false, false);
			}
		}
		
		// if we can, try to setup where to save files
		if (IService::getInstance()->haveArg('W'))
		{
			// use the command line param if set
			SaveFilesDirectory = IService::getInstance()->getArg('W');
		}


		//
		// Call the user service init
		//

		userInitCalled = true; // the bool must be put *before* the call to init()
		init ();


		//
		// Connects to the present services
		// WARNING: only after the user init() was called because the
		// addService may call up service callbacks.
		//

		CUnifiedNetwork::getInstance()->connect();

		//
		// Say to the AES that the service is ready
		//

		if (!_DontUseAES)
		{
			// send the ready message (service init finished)
			CMessage msgout ("SR");
			CUnifiedNetwork::getInstance()->send("AES", msgout);
		}


		_Initialized = true;

		nlinfo ("SERVICE: Service initialised, executing StartCommands");

		//
		// Call the user command from the config file if any
		//

		if ((var = ConfigFile.getVarPtr ("StartCommands")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				ICommand::execute (var->asString(i), CommandLog);
			}
		}

		string str;
		CLog logDisplayVars;
		CLightMemDisplayer mdDisplayVars;
		logDisplayVars.addDisplayer (&mdDisplayVars);

		//
		// Activate the timeout assertion thread
		//
		
		IThread::create(&MyTAT)->start();
		
		//
		// Set service ready
		//

		nlinfo ("SERVICE: Service ready");

		if (WindowDisplayer != NULL)
			WindowDisplayer->setTitleBar (_ShortName + " " + _LongName + " " + Version.c_str());

		//
		// Call the user service update each loop and check files and network activity
		//

		do
		{
			MyTAT.activate();

			if(Bench) CHTimer::startBench(false, true, false);

			// count the amount of time to manage internal system
			TTime bbefore = CTime::getLocalTime ();

			// call the user update and exit if the user update asks it
			{
				H_AUTO(NLNETServiceUpdate);
				if (!update ())
				{
					CHTimer::endBench();
					break;
				}
			}
			
			// if the launching mode is 'quit after the first update' we set the exit signal
			if (haveArg('Q'))
			{
				ExitSignalAsked = 1;
			}
			NbUserUpdate++;

			// count the amount of time to manage internal system
			TTime before = CTime::getLocalTime ();

			if (WindowDisplayer != NULL)
			{
				// update the window displayer and quit if asked
				if (!WindowDisplayer->update ())
				{
					nlinfo ("SERVICE: The window displayer was closed by user, need to quit");
					ExitSignalAsked = 1;
				}
			}

			// stop the loop if the exit signal asked
			if (ExitSignalAsked > 0)
			{
				CHTimer::endBench();
				break;
			}

			CConfigFile::checkConfigFiles ();

			updateAdmin ();

			CFile::checkFileChange();

			// get and manage layer 5 messages
			CUnifiedNetwork::getInstance()->update (_UpdateTimeout);

			// resync the clock every hours
//			if (resyncEvenly)
//			{
//				static TTime LastSyncTime = CTime::getLocalTime ();

				//---------------------------------------
				// To simulate Ctrl-C in the debugger... Exit after 1 min !
				/*if (CTime::getLocalTime () - LastSyncTime > 60 * 1000 )
				{
					ExitSignalAsked = 1;
				}*/
				//---------------------------------------
/*
				if (CTime::getLocalTime () - LastSyncTime > 60*60*1000)
				{
					CUniTime::syncUniTimeFromService ( _RecordingState );
					LastSyncTime = CTime::getLocalTime ();
				}
*/
//			}

			NetSpeedLoop = (sint32) (CTime::getLocalTime () - before);
			UserSpeedLoop = (sint32) (before - bbefore);

			if (WindowDisplayer != NULL)
			{
				uint64 rcv, snd, rcvq, sndq;
				rcv = CUnifiedNetwork::getInstance()->getBytesReceived ();
				snd = CUnifiedNetwork::getInstance()->getBytesSent ();
				rcvq = CUnifiedNetwork::getInstance()->getReceiveQueueSize ();
				sndq = CUnifiedNetwork::getInstance()->getSendQueueSize ();

				for (uint i = 0; i < displayedVariables.size(); i++)
				{
					// it s a separator, do nothing
					if (displayedVariables[i].first.empty())
						continue;

					// it s a command, do nothing
					if (displayedVariables[i].first[0] == '@')
						continue;

					string dispName = displayedVariables[i].first;
					string varName = dispName;
					uint32 pos = dispName.find("|");
					if (pos != string::npos)
					{
						varName = displayedVariables[i].first.substr(pos+1);
						dispName = displayedVariables[i].first.substr(0, pos);
					}

					if (dispName.empty())
						str = "";
					else
						str = dispName + ": ";
					
					mdDisplayVars.clear ();
					ICommand::execute(varName, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					if (strs.size()>0)
					{
						str += strs[0].substr(0,strs[0].size()-1);
						/*
						string s_ = strs[0];

						uint32 pos = strs[0].find("=");
						if(pos != string::npos && pos + 2 < strs[0].size())
						{
							uint32 pos2 = string::npos;
							if(strs[0][strs[0].size()-1] == '\n')
								pos2 = strs[0].size() - pos - 2 - 1;

							str += strs[0].substr (pos+2, pos2);
						}
						else
						{
							str += "???";
						}*/
					}
					else
					{
						str += "???";
					}
					mdDisplayVars.unlockStrings();
					WindowDisplayer->setLabel (displayedVariables[i].second, str);
				}

			}

//			nldebug ("SYNC: updatetimeout must be %d and is %d, sleep the rest of the time", _UpdateTimeout, delta);

			CHTimer::endBench();
			
			// Resetting the hierarchical timer must be done outside the top-level timer
			if ( _ResetMeasures )
			{
				CHTimer::clear();
				_ResetMeasures = false;
			}

			MyTAT.desactivate();
		}
		while (true);
	}
/*	catch (ETrapDebug &)
	{
		// we have to do that if we want to trap unhandled exception with the report message box
		setStatus (EXIT_FAILURE);
	}
*/	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}
	catch ( uint ) // SEH exceptions
	{
		ErrorLog->displayNL( "SERVICE: System exception" );
	}

#ifdef NL_RELEASE
/*	// in release mode, we catch everything to handle clean release.
	catch (Exception &e)
	{
		// Catch NeL exception to release the system cleanly
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: NeL Exception: Error running the service \"%s\": %s", _ShortName.c_str(), e.what());
	}
	catch (...)
	{
		// Catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: Unknown external exception");
	}
*/
#endif

	try
	{
		nlinfo ("SERVICE: Service starts releasing");

		//
		// Call the user service release() if the init() was called
		//

		if (userInitCalled)
			release ();

		//
		// Delete all network connection (naming client also)
		//

		CUnifiedNetwork::getInstance()->release ();

		CSock::releaseNetwork ();

		//
		// Remove the window displayer
		//

		if (WindowDisplayer != NULL)
		{
			DebugLog->removeDisplayer (WindowDisplayer);
			InfoLog->removeDisplayer (WindowDisplayer);
			WarningLog->removeDisplayer (WindowDisplayer);
			ErrorLog->removeDisplayer (WindowDisplayer);
			AssertLog->removeDisplayer (WindowDisplayer);
			CommandLog.removeDisplayer (WindowDisplayer);

			delete WindowDisplayer;
			WindowDisplayer = NULL;
		}

		nlinfo ("SERVICE: Service released succesfuly");
	}
/*	catch (ETrapDebug &)
	{
		// we have to do that if we want to trap unhandled exception with the report message box
		setStatus (EXIT_FAILURE);
	}
*/	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}

#ifdef NL_RELEASE
/*	// in release mode, we catch everything to handle clean release.
	catch (Exception &e)
	{
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: NeL Exception: Error releasing the service \"%s\": %s", _ShortName.c_str(), e.what());
	}
	catch (...)
	{
		// Catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: Unknown external exception");
	}
*/
#endif

	CHTimer::display();
	CHTimer::displayByExecutionPath ();
	CHTimer::displayHierarchical(&CommandLog, true, 64);
	CHTimer::displayHierarchicalByExecutionPathSorted (&CommandLog, CHTimer::TotalTime, true, 64);

	nlinfo ("SERVICE: Service ends");

	string name = getServiceLongName () + ".memory_report";
	NLMEMORY::StatisticsReport (name.c_str(), false);

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}

void IService::exit (sint code)
{
	nlinfo ("SERVICE: Somebody called IService::exit(), I have to quit");
	ExitSignalAsked = code;
}

/*
 * Require to reset the hierarchical timer
 */
void IService::requireResetMeasures()
{
	_ResetMeasures = true;
}


std::string IService::getServiceUnifiedName () const
{
	nlassert (!_ShortName.empty());
	string res;
	if (!_AliasName.empty())
	{
		res = _AliasName+"/";
	}
	res += _ShortName;
	if (_SId != 0)
	{
		res += "-";
		res += toString (_SId);
	}
	return res;
}


//
// Commands and Variables for controling all services
//

NLMISC_DYNVARIABLE(string, LaunchingDate, "date of the launching of the program")
{
	if (get) *pointer = asctime (localtime ((time_t*)&LaunchingDate));
}

NLMISC_DYNVARIABLE(string, Uptime, "time from the launching of the program")
{
	if (get)
	{
		if (human)
			*pointer = secondsToHumanReadable (CTime::getSecondsSince1970() - LaunchingDate);
		else
			*pointer = NLMISC::toString(CTime::getSecondsSince1970() - LaunchingDate);
	}
	else
	{
		LaunchingDate = CTime::getSecondsSince1970() - atoi ((*pointer).c_str());
	}
}

//NLMISC_VARIABLE(bool, Bench, "1 if benching 0 if not");

NLMISC_VARIABLE(string, CompilationDate, "date of the compilation");
NLMISC_VARIABLE(string, CompilationMode, "mode of the compilation");

NLMISC_VARIABLE(uint32, NbUserUpdate, "number of time the user IService::update() called");

/*NLMISC_DYNVARIABLE(uint32, ListeningPort, "default listening port for this service")
{
	if (get) *pointer = IService::getInstance()->getPort();
}
*/
NLMISC_DYNVARIABLE(string, Scroller, "current size in bytes of the sent queue size")
{
	if (get)
	{
		// display the scroll text
		static string foo =	"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! "
							"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! ";
		static int pos = 0;
		*pointer = foo.substr ((pos++)%(foo.size()/2), 10);
	}
}

NLMISC_COMMAND (quit, "exit the service", "")
{
	if(args.size() != 0) return false;

	log.displayNL("User ask me with a command to quit");
	ExitSignalAsked = 0xFFFF;

	return true;
}

NLMISC_COMMAND (brutalQuit, "exit the service brutally", "")
{
	if(args.size() != 0) return false;

	::exit (0xFFFFFFFF);

	return true;
}


#ifdef MUTEX_DEBUG
NLMISC_COMMAND (mutex, "display mutex values", "")
{
	if(args.size() != 0) return false;

	map<CFairMutex*,TMutexLocks>	acquiretimes = getNewAcquireTimes();

	map<CFairMutex*,TMutexLocks>::iterator im;
	for ( im=acquiretimes.begin(); im!=acquiretimes.end(); ++im )
	{
		log.displayNL( "%d %p %s: %.0f %.0f, called %u times th(%d, %d wait)%s", (*im).second.MutexNum, (*im).first, (*im).second.MutexName.c_str(),
			CTime::cpuCycleToSecond((*im).second.TimeToEnter)*1000.0, CTime::cpuCycleToSecond((*im).second.TimeInMutex)*1000.0,
			(*im).second.Nb, (*im).second.ThreadHavingTheMutex, (*im).second.WaitingMutex,
			(*im).second.Dead?" DEAD":"");
	}

	return true;
}
#endif // MUTEX_DEBUG

NLMISC_COMMAND (serviceInfo, "display information about this service", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Service %s '%s' using NeL ("__DATE__" "__TIME__")", IService::getInstance()->getServiceLongName().c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
	log.displayNL ("Service listening port: %d", IService::getInstance()->ListeningPort.get());
	log.displayNL ("Service running directory: '%s'", IService::getInstance()->RunningDirectory.c_str());
	log.displayNL ("Service log directory: '%s'", IService::getInstance()->LogDirectory.c_str());
	log.displayNL ("Service save files directory: '%s'", IService::getInstance()->SaveFilesDirectory.c_str());
	log.displayNL ("Service write files directory: '%s'", IService::getInstance()->WriteFilesDirectory.c_str());
	log.displayNL ("Service config directory: '%s' config filename: '%s.cfg'", IService::getInstance()->ConfigDirectory.c_str(), IService::getInstance()->_LongName.c_str());
	log.displayNL ("Service id: %d", IService::getInstance()->_SId);
	log.displayNL ("Service update timeout: %dms", IService::getInstance()->_UpdateTimeout);
	log.displayNL ("Service %suse naming service", IService::getInstance()->_DontUseNS?"don't ":"");
	log.displayNL ("Service %suse admin executor service", IService::getInstance()->_DontUseAES?"don't ":"");
	log.displayNL ("NeL is compiled in %s mode", CompilationMode.c_str());

	log.displayNL ("Services arguments: %d args", IService::getInstance()->_Args.size ());
	for (uint i = 0; i < IService::getInstance()->_Args.size (); i++)
	{
		log.displayNL ("  argv[%d] = '%s'", i, IService::getInstance()->_Args[i].c_str ());
	}

	log.displayNL ("Naming service info: %s", CNamingClient::info().c_str());

	ICommand::execute ("services", log);

	return true;
}

NLMISC_COMMAND(resetMeasures, "reset hierarchical timer", "")
{
	IService::getInstance()->requireResetMeasures();
	return true;
}

NLMISC_COMMAND(getWinDisplayerInfo, "display the info about the pos and size of the window displayer", "")
{
	uint32 x,y,w,h;
	IService::getInstance()->WindowDisplayer->getWindowPos (x,y,w,h);
	log.displayNL ("Window Displayer : XWinParam = %d; YWinParam = %d; WWinParam = %d; HWinParam = %d;", x, y, w, h);
	return true;
}

NLMISC_COMMAND(displayConfigFile, "display the variables of the default configfile", "")
{
	IService::getInstance()->ConfigFile.display (&log);
	return true;
}

NLMISC_COMMAND(getUnknownConfigFileVariables, "display the variables from config file that are called but not present", "")
{
	log.displayNL ("%d Variables not found in the configfile '%s'", IService::getInstance()->ConfigFile.UnknownVariables.size(), IService::getInstance()->ConfigFile.getFilename().c_str() );
	for (uint i = 0; i < IService::getInstance()->ConfigFile.UnknownVariables.size(); i++)
	{
		log.displayNL ("  %s", IService::getInstance()->ConfigFile.UnknownVariables[i].c_str());
	}
	return true;
}

// -1 = service is quitting
// 0 = service is not connected
// 1 = service is running
// 2 = service is launching
// 3 = service failed launching

NLMISC_DYNVARIABLE(string, State, "Set this value to 0 to shutdown the service and 1 to start the service")
{
	static string running = "Online";

	// read or write the variable
	if (get)
	{
		*pointer = running;
	}
	else
	{
		if (IService::getInstance()->getServiceShortName() == "AES" || IService::getInstance()->getServiceShortName() == "AS")
		{
			nlinfo ("SERVICE: I can't set State=0 because I'm the admin and I should never quit");
		}
		else if (*pointer == "0" || *pointer == "2")
		{
			// ok, we want to set the value to false, just quit
			nlinfo ("SERVICE: User ask me with a command to quit using the State variable");
			ExitSignalAsked = 0xFFFE;
			running = "Quitting";
		}
		else
		{
			nlwarning ("SERVICE: Unknown value for State '%s'", (*pointer).c_str());
		}
	}
}


NLMISC_DYNVARIABLE(uint32, ShardId, "Get value of shardId set for this particular service")
{
	// read or write the variable
	if (get)
	{
		*pointer = IService::getInstance()->getShardId();
	}
}


} //NLNET
