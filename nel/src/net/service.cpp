/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.135 2002/07/03 09:55:37 lecroart Exp $
 *
 * \todo ace: test the signal redirection on Unix
 * \todo ace: add parsing command line (with CLAP?)
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

#ifdef NL_OS_WINDOWS

//
// Includes
//

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

#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mutex.h"
#include "nel/misc/window_displayer.h"
#include "nel/misc/gtk_displayer.h"
#include "nel/misc/win_displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/net/naming_client.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/net_manager.h"
#include "nel/net/net_displayer.h"

#include "nel/misc/hierarchical_timer.h"


//
// Namespace
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


//
// Variables
//


// class static member
IService	*IService::_Instance = NULL;

static sint ExitSignalAsked = 0;

// services stat
static sint32 NetSpeedLoop, UserSpeedLoop;

// this is the thread that initialized the signal redirection
// we ll ignore other thread signals
static uint SignalisedThread;

static CFileDisplayer fd;
static CNetDisplayer commandDisplayer(false);
static CLog commandLog;


//
// Callback managing
//

void AESConnection (const string &serviceName, TSockId from, void *arg)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	CMessage msgout (CNetManager::getSIDA ("AES"), "SID");
	msgout.serial (IService::getInstance()->_AliasName, IService::getInstance()->_ShortName, IService::getInstance()->_LongName);
	ICommand::serialCommands (msgout);
	CNetManager::send ("AES", msgout);

	if (IService::getInstance()->_Initialized)
	{
		CMessage msgout2 (CNetManager::getSIDA ("AES"), "SR");
		CNetManager::send ("AES", msgout2);
	}

	// add the displayer to the standard logger
	CCallbackClient *client = dynamic_cast<CCallbackClient *>(CNetManager::getNetBase("AES"));
	commandDisplayer.setLogServer (client);
	commandLog.addDisplayer (&commandDisplayer);
}

void AESConnection5 (const string &serviceName, uint16 sid, void *arg)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	CMessage msgout ("SID");
	msgout.serial (IService::getInstance()->_AliasName, IService::getInstance()->_ShortName, IService::getInstance()->_LongName);
	ICommand::serialCommands (msgout);
	CUnifiedNetwork::getInstance()->send("AES", msgout);

	if (IService::getInstance()->_Initialized)
	{
		CMessage msgout2 ("SR");
		CUnifiedNetwork::getInstance()->send("AES", msgout2);
	}

	// add the displayer to the standard logger
	TSockId			hid;
	CCallbackClient *client = dynamic_cast<CCallbackClient *>(CUnifiedNetwork::getInstance()->getNetBase("AES", hid));
	commandDisplayer.setLogServer (client);
	commandLog.addDisplayer (&commandDisplayer);
}

static void AESDisconnection (const string &serviceName, TSockId from, void *arg)
{
	commandLog.removeDisplayer (&commandDisplayer);
}


static void AESDisconnection5 (const std::string &serviceName, uint16 sid, void *arg)
{
	commandLog.removeDisplayer (&commandDisplayer);
}


static void cbExecCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string command;
	msgin.serial (command);

	ICommand::execute (command, commandLog);
}

static void cbExecCommand5 (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	string command;
	msgin.serial (command);

	ICommand::execute (command, commandLog);
}


// if we receive the stop service, we try to exit now
static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	ExitSignalAsked = 0xFFFF;
}

static void cbStopService5 (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	ExitSignalAsked = 0xFFFF;
}


// layer4
static TCallbackItem AESCallbackArray[] =
{
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
};

// layer 5
static TUnifiedCallbackItem AESCallbackArray5[] =
{
	{ "STOPS", cbStopService5 },
	{ "EXEC_COMMAND", cbExecCommand5 },
};

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
				nldebug ("Not the main thread received the signal (%s, %d), ignore it", SignalName[i],Sig);
				return;
			}
			else
			{
				nlinfo ("Signal %s (%d) received", SignalName[i], Sig);
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
					ExitSignalAsked = Sig;
					return;
				}
				else
				{
					nlinfo ("Signal already received, launch the brutal exit");
					exit (EXIT_FAILURE);
				}
				break;
				}
			}
		}
	}
	nlinfo ("Unknown signal received (%d)", Sig);
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

//
// Class implementation
//

// Ctor
IService::IService() :
	_Initialized(false), WindowDisplayer(NULL), _UpdateTimeout(100), _Port(0), _RecordingState(CCallbackNetBase::Off), _SId(0), _Status(0), _IsService5(false), _ResetMeasures(false)
{
	// Singleton
	nlassert( _Instance == NULL );

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
	sint pos1 = 0, pos2 = 0;

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

/*
 * Returns a pointer to the CCallbackServer object
 */
CCallbackServer *IService::getServer()
{
	if (isService5())
		return NULL;
	else
		return dynamic_cast<CCallbackServer*>(CNetManager::getNetBase(IService::_ShortName));
}



void cbLogFilter (CConfigFile::CVar &var)
{
	CLog *log = NULL;
	if (var.Name == "NegFiltersDebug")
	{
		log = DebugLog;
		nlinfo ("Updating negative filter on debug from config file");
	}
	else if (var.Name == "NegFiltersInfo")
	{
		log = InfoLog;
		nlinfo ("Updating negative filter on info from config file");
	}
	else
	{
		nlstop;
	}

	// remove all old filter from configfile
	CConfigFile::CVar &oldvar = IService::getInstance()->ConfigFile.getVar (var.Name);
	for (sint j = 0; j < oldvar.size(); j++)
	{
		log->removeFilter (oldvar.asString(j).c_str());
	}

	// add all new filter from configfile
	for (sint i = 0; i < var.size(); i++)
	{
		log->addNegativeFilter (var.asString(i).c_str());
	}
}








//
// The main function of the service
//

sint IService::main (const char *serviceShortName, const char *serviceLongName, uint16 servicePort, const char *configDir, const char *logDir)
{
	bool userInitCalled = false;
	bool resyncEvenly = false;
	CConfigFile::CVar *var = NULL;

	try
	{
		// get the path where to run the service if any in the command line
		if (haveArg('A'))
		{
			_RunningPath = CPath::standardizePath(getArg('A'));
#ifdef NL_OS_WINDOWS
			_chdir (_RunningPath.c_str());
#else
			chdir (_RunningPath.c_str());
#endif
		}

		//
		// Init parameters
		//

		_ConfigDir = CPath::standardizePath(configDir);
		_LogDir = CPath::standardizePath(logDir);
		_ShortName = serviceShortName;
		_LongName = serviceLongName;

		//
		// Init debug/log stuffs (must be first things otherwise we can't log if errors)
		//

		// get the log dir if any in the command line
		if (haveArg('L'))
			_LogDir = CPath::standardizePath(getArg('L'));

		// get the config file dir if any in the command line
		if (haveArg('C'))
			_ConfigDir = CPath::standardizePath(getArg('C'));

		createDebug (_LogDir.c_str(), false);

		DebugLog->addNegativeFilter ("NETL");

		// we create the log with service name filename ("test_service.log" for example)
		fd.setParam (_LogDir + _LongName + ".log", false);

		DebugLog->addDisplayer (&fd);
		InfoLog->addDisplayer (&fd);
		WarningLog->addDisplayer (&fd);
		AssertLog->addDisplayer (&fd);
		ErrorLog->addDisplayer (&fd);


		//
		// Init the hierarchical timer
		//

		CHTimer::startBench(false, true);


		//
		// Load the config file
		//

		ConfigFile.load (_ConfigDir + _LongName + ".cfg");


		//
		// Set the negatif filter from the config file
		//

		if ((var = ConfigFile.getVarPtr ("NegFiltersDebug")) != NULL)
		{
			ConfigFile.setCallback ("NegFiltersDebug", cbLogFilter);
			for (sint i = 0; i < var->size(); i++)
			{
				DebugLog->addNegativeFilter (var->asString(i).c_str());
			}
		}

		if ((var = ConfigFile.getVarPtr ("NegFiltersInfo")) != NULL)
		{
			ConfigFile.setCallback ("NegFiltersInfo", cbLogFilter);
			for (sint i = 0; i < var->size(); i++)
			{
				InfoLog->addNegativeFilter (var->asString(i).c_str());
			}
		}


		//
		// Create the window if neeeded
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
				nlwarning ("Unknown value for the WindowStyle (should be GTK, WIN or NONE), use no window displayer");
			}
		}

		vector <pair<string,uint> > displayedVariables;
		//uint speedNetLabel, speedUsrLabel, rcvLabel, sndLabel, rcvQLabel, sndQLabel, scrollLabel;
		if (WindowDisplayer != NULL)
		{
			//
			// Init window param if necessary
			//

			sint x=-1, y=-1, w=-1, h=-1;
			bool iconified = false;

			if ((var = ConfigFile.getVarPtr("XWinParam")) != NULL) x = var->asInt();
			if ((var = ConfigFile.getVarPtr("YWinParam")) != NULL) y = var->asInt();
			if ((var = ConfigFile.getVarPtr("WWinParam")) != NULL) w = var->asInt();
			if ((var = ConfigFile.getVarPtr("HWinParam")) != NULL) h = var->asInt();
			if ((var = ConfigFile.getVarPtr("HWinParam")) != NULL) iconified = var->asInt() == 1;

			if (haveArg('I')) iconified = true;

			if (w == -1 && h == -1)
				WindowDisplayer->create (string("*INIT* ") + _ShortName + " " + _LongName, iconified, x, y);
			else
				WindowDisplayer->create (string("*INIT* ") + _ShortName + " " + _LongName, iconified, x, y, w, h);

			DebugLog->addDisplayer (WindowDisplayer);
			InfoLog->addDisplayer (WindowDisplayer);
			WarningLog->addDisplayer (WindowDisplayer);
			ErrorLog->addDisplayer (WindowDisplayer);
			AssertLog->addDisplayer (WindowDisplayer);

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

		nlinfo ("Starting Service %d '%s' using NeL ("__DATE__" "__TIME__")", isService5()?5:4, _ShortName.c_str());

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
		nldebug ("SIGPIPE %s", IgnoredPipe?"Ignored":"Not Ignored");
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
		string processName = localhost;
		processName += '/';
		processName += _ShortName;
		CLog::setProcessName (processName);


		//
		// Initialize server parameters
		//

		// Get the port from config file or in the macro (overload the port set by the user init())
		if ((var = ConfigFile.getVarPtr("Port")) != NULL)
		{
			// set the listen port with the value in the config file if any
			_Port = var->asInt();
		}
		else
		{
			// set the listen port with the value in the NLNET_SERVICE_MAIN macro
			_Port = servicePort;
		}

		// set the listen port if there are a port arg in the command line
		if (haveArg('P'))
		{
			_Port = atoi(getArg('P').c_str());
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
				nlinfo( "Service recording messages" );
			}
			else if ( srecstate == "REPLAY" )
			{
				_RecordingState = CCallbackNetBase::Replay;
				nlinfo( "Service replaying messages" );
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
			if (sid<0 || sid>255)
			{
				nlwarning("Bad SId value in the config file, %d is not in [0;255] range", sid);
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
			_DontUseNS = var->asInt() == 1;
		}
		else
		{
			// if not, we use ns only if service is not ns, ls, aes, as
			_DontUseNS = (_ShortName == "NS" || _ShortName == "LS" || _ShortName == "AES" || _ShortName == "AS");
		}

		// normal setup for the common services
		if (!_DontUseNS)
		{
			bool ok = false;
			while (!ok)
			{
				// read the naming service address from the config file
				string LSAddr = ConfigFile.getVar ("NSHost").asString();
				
				// if there's no port to the NS, use the default one 50000
				if (LSAddr.find(":") == string::npos)
						LSAddr += ":50000";

				CInetAddress loc(LSAddr);
				try
				{
					if (isService5())
						CUnifiedNetwork::getInstance()->init (&loc, _RecordingState, _ShortName, _Port, _SId);
					else
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
			if (isService5())
				CUnifiedNetwork::getInstance()->init(NULL, _RecordingState, _ShortName, _Port, _SId);
			else
				CNetManager::init( NULL, _RecordingState );
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
			_DontUseAES = (_ShortName == "AES" || _ShortName == "AS");
		}

		if (!_DontUseAES)
		{
			if (isService5())
			{
				CUnifiedNetwork::getInstance()->setServiceUpCallback ("AES", AESConnection5, NULL);
				CUnifiedNetwork::getInstance()->setServiceDownCallback ("AES", AESDisconnection5, NULL);
				CUnifiedNetwork::getInstance()->addService ("AES", CInetAddress("localhost:49997"), false);
				CUnifiedNetwork::getInstance()->addCallbackArray (AESCallbackArray5, sizeof(AESCallbackArray5)/sizeof(AESCallbackArray5[0]));
			}
			else
			{
				CNetManager::setConnectionCallback ("AES", AESConnection, NULL);
				CNetManager::setDisconnectionCallback ("AES", AESDisconnection, NULL);
				CNetManager::addClient ("AES", "localhost:49997");
				CNetManager::addCallbackArray ("AES", AESCallbackArray, sizeof(AESCallbackArray)/sizeof(AESCallbackArray[0]));
			}
		}


		//
		// Add callback array
		//

		if (isService5())
		{
			// add callback set in the NLNET_SERVICE_MAIN macro
			NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(_CallbackArray5, _CallbackArraySize);
		}
		else
		{
			CNetManager::addServer (_ShortName, _Port, _SId);
			CNetManager::addCallbackArray (_ShortName, _CallbackArray, _CallbackArraySize);
		}

		//
		// Now we have the service id, we can set the entites id generator
		//

		_NextEntityId.setServiceId(_SId);


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

		if (isService5())
			CUnifiedNetwork::getInstance()->connect();


		//
		// On Unix system, the service fork itself to give back the hand to the shell
		//
		// note: we don't forking anymore because it doesn't work with thread system
		//

#ifdef NL_OS_UNIX
		/*
		nlinfo( "Forking the service" );

		int pid = fork();

		/// \todo ace: when we fork() on linux, the father process tries to release threads but it should not, so we have to find a solution

		if (pid == -1)
		{
			nlerror ("Couldn't fork the service");
		}
		else if (pid != 0)
		{
			// It's the father, return the hand to the shell.
			exit(EXIT_SUCCESS);
		}
		*/
#endif // NL_OS_UNIX


		//
		// Say to the AES that the service is ready
		//

		if (!_DontUseAES)
		{
			if (isService5())
			{
				// send the ready message (service init finished)
				CMessage msgout ("SR");
				CUnifiedNetwork::getInstance()->send("AES", msgout);
			}
			else
			{
				// send the ready message (service init finished)
				CMessage msgout (CNetManager::getSIDA ("AES"), "SR");
				CNetManager::send ("AES", msgout);
			}
		}


		//
		// Add default pathes
		//

		if ((var = ConfigFile.getVarPtr ("Paths")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i));
			}
		}


		_Initialized = true;

		nlinfo ("Service initialised");


		//
		// Call the user command from the config file if any
		//

		if ((var = ConfigFile.getVarPtr ("StartCommands")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				ICommand::execute (var->asString(i), *InfoLog);
			}
		}

		nlinfo ("Service ready");

		if (WindowDisplayer != NULL)
			WindowDisplayer->setTitleBar (_ShortName + " " + _LongName);


		//
		// Call the user service update each loop and check files and network activity
		//

		do
		{
			H_BEFORE(NLNETServiceLoop);

			// count the amount of time to manage internal system
			TTime bbefore = CTime::getLocalTime ();

			// call the user update and exit if the user update asks it
			H_BEFORE(NLNETServiceUpdate);
			if (!update ())
			{
				H_AFTER(NLNETServiceUpdate);
				H_AFTER(NLNETServiceLoop);
				break;
			}
			H_AFTER(NLNETServiceUpdate);
			
			// count the amount of time to manage internal system
			TTime before = CTime::getLocalTime ();

			if (WindowDisplayer != NULL)
			{
				// update the window displayer and quit if asked
				if (!WindowDisplayer->update ())
					ExitSignalAsked = true;
			}

			// stop the loop if the exit signal asked
			if (ExitSignalAsked)
			{
				H_AFTER(NLNETServiceLoop);
				break;
			}
	
			CConfigFile::checkConfigFiles ();

			CFile::checkFileChange();

			H_BEFORE(NLNETServiceManageMessages);
			if (isService5())
			{
				// get and manage layer 5 messages
				CUnifiedNetwork::getInstance()->update (_UpdateTimeout);
			}
			else
			{
				// get and manage layer 4 messages
				CNetManager::update (_UpdateTimeout);
			}
			H_AFTER(NLNETServiceManageMessages);
			
			// resync the clock every hours
			if (resyncEvenly)
			{
				static TTime LastSyncTime = CTime::getLocalTime ();

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
			}

			NetSpeedLoop = (sint32) (CTime::getLocalTime () - before);
			UserSpeedLoop = (sint32) (before - bbefore);

			if (WindowDisplayer != NULL)
			{
				uint64 rcv, snd, rcvq, sndq;
				if (isService5())
				{
					rcv = CUnifiedNetwork::getInstance()->getBytesReceived ();
					snd = CUnifiedNetwork::getInstance()->getBytesSent ();
					rcvq = CUnifiedNetwork::getInstance()->getReceiveQueueSize ();
					sndq = CUnifiedNetwork::getInstance()->getSendQueueSize ();
				}
				else
				{
					rcv = CNetManager::getBytesReceived ();
					snd = CNetManager::getBytesSent ();
					rcvq = CNetManager::getReceiveQueueSize ();
					sndq = CNetManager::getSendQueueSize ();
				}

				string str;
				CLog log;
				CMemDisplayer md;
				log.addDisplayer (&md);

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
					sint pos = dispName.find("|");
					if (pos != string::npos)
					{
						varName = displayedVariables[i].first.substr(pos+1);
						dispName = displayedVariables[i].first.substr(0, pos);
					}

					if (dispName.empty())
						str = "";
					else
						str = dispName + ": ";
					
					md.clear ();
					ICommand::execute(varName, log, true);
					const std::deque<std::string>	&strs = md.lockStrings();
					if (strs.size()>0)
					{
						sint pos = strs[0].find("=");
						if(pos != string::npos && pos + 2 < (sint)strs[0].size())
						{
							sint pos2 = string::npos;
							if(strs[0][strs[0].size()-1] == '\n')
								pos2 = strs[0].size() - pos - 2 - 1;

							str += strs[0].substr (pos+2, pos2);
						}
						else
						{
							str += "???";
						}
					}
					md.unlockStrings();
					WindowDisplayer->setLabel (displayedVariables[i].second, str);
				}

			}

//			nldebug ("SYNC: updatetimeout must be %d and is %d, sleep the rest of the time", _UpdateTimeout, delta);
			H_AFTER(NLNETServiceLoop);

			// Resetting the hierarchical timer must be done outside the top-level timer
			if ( _ResetMeasures )
			{
				CHTimer::clear();
				_ResetMeasures = false;
			}
		}
		while (true);
	}
	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}
	catch ( uint ) // SEH exceptions
	{
		ErrorLog->displayNL( "System exception" );
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
		nlinfo ("Service starts releasing");

		//
		// Call the user service release() if the init() was called
		//

		if (userInitCalled)
			release ();

		//
		// Delete all network connection (naming client also)
		//

		if (isService5())
			CUnifiedNetwork::getInstance()->release ();
		else
			CNetManager::release ();

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

			delete WindowDisplayer;
			WindowDisplayer = NULL;
		}

		nlinfo ("Service released succesfuly");
	}
	catch (EFatalError &)
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

	CHTimer::endBench();
	CHTimer::display();
	CHTimer::displayByExecutionPath ();
	CHTimer::displayHierarchical(InfoLog, true, 64);
	CHTimer::displayHierarchicalByExecutionPathSorted (InfoLog, CHTimer::TotalTime, true, 64);

	nlinfo ("Service ends");

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}


/*
 * Require to reset the hierarchical timer
 */
void IService::requireResetMeasures()
{
	_ResetMeasures = true;
}


//
// Commands and Variables for controling all services
//

NLMISC_VARIABLE(sint32, NetSpeedLoop, "duration of the last network loop (in ms)");
NLMISC_VARIABLE(sint32, UserSpeedLoop, "duration of the last user loop (in ms)");

NLMISC_DYNVARIABLE(uint64, ReceivedBytes, "total of bytes received by this service")
{
	// we can only read the value
	if (get)
		*pointer = IService::getInstance()->isService5()?CUnifiedNetwork::getInstance()->getBytesReceived ():CNetManager::getBytesReceived ();
}

NLMISC_DYNVARIABLE(uint64, SentBytes, "total of bytes sent by this service")
{
	// we can only read the value
	if (get)
		*pointer = IService::getInstance()->isService5()?CUnifiedNetwork::getInstance()->getBytesSent ():CNetManager::getBytesSent ();
}

NLMISC_DYNVARIABLE(uint64, ReceivedQueueSize, "current size in bytes of the received queue size")
{
	// we can only read the value
	if (get)
		*pointer = IService::getInstance()->isService5()?CUnifiedNetwork::getInstance()->getReceiveQueueSize ():CNetManager::getReceiveQueueSize ();
}

NLMISC_DYNVARIABLE(uint64, SentQueueSize, "current size in bytes of the sent queue size")
{
	// we can only read the value
	if (get)
		*pointer = IService::getInstance()->isService5()?CUnifiedNetwork::getInstance()->getSendQueueSize ():CNetManager::getSendQueueSize ();
}

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

	ExitSignalAsked = 0xFFFF;

	return true;
}

NLMISC_COMMAND (brutalQuit, "exit the service brutally", "")
{
	if(args.size() != 0) return false;

	exit (0xFFFFFFFF);

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
		nlinfo( "%d %p %s: %.0f %.0f, called %u times th(%d, %d wait)%s", (*im).second.MutexNum, (*im).first, (*im).second.MutexName.c_str(),
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

	log.displayNL ("Service %d '%s' '%s' '%s' using NeL ("__DATE__" "__TIME__")", IService::getInstance()->isService5()?5:4, IService::getInstance()->_ShortName.c_str(), IService::getInstance()->_LongName.c_str(), IService::getInstance()->_AliasName.c_str());
	log.displayNL ("Service listening port: %d", IService::getInstance()->_Port);
	log.displayNL ("Service running directory: '%s'", IService::getInstance()->_RunningPath.c_str());
	log.displayNL ("Service log directory: '%s'", IService::getInstance()->_LogDir.c_str());
	log.displayNL ("Service config directory: '%s' config filename: '%s.cfg'", IService::getInstance()->_ConfigDir.c_str(), IService::getInstance()->_LongName.c_str());
	log.displayNL ("Service id: %d", IService::getInstance()->_SId);
	log.displayNL ("Service update timeout: %dms", IService::getInstance()->_UpdateTimeout);
	log.displayNL ("Service %suse naming service", IService::getInstance()->_DontUseNS?"don't ":"");
	log.displayNL ("Service %suse admin executor service", IService::getInstance()->_DontUseAES?"don't ":"");
#ifdef NL_RELEASE_DEBUG
	string mode = "NL_RELEASE_DEBUG";
#elif defined(NL_DEBUG_FAST)
	string mode = "NL_DEBUG_FAST";
#elif defined(NL_DEBUG)
	string mode = "NL_DEBUG";
#elif defined(NL_RELEASE)
	string mode = "NL_RELEASE";
#else
	string mode = "???";
#endif
	log.displayNL ("NeL is compiled in %s mode", mode.c_str());

	nlinfo ("Services arguments: %d args", IService::getInstance()->_Args.size ());
	for (uint i = 0; i < IService::getInstance()->_Args.size (); i++)
	{
		nlinfo ("  argv[%d] = '%s'", i, IService::getInstance()->_Args[i].c_str ());
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

NLMISC_COMMAND(displayMeasures, "display hierarchical timer", "")
{
	CHTimer::display();
	CHTimer::displayHierarchicalByExecutionPathSorted (InfoLog, CHTimer::TotalTime, true, 64);
	return true;
}

NLMISC_COMMAND(getWinDisplayerInfo, "display the info about the pos and size of the window displayer", "")
{
	uint32 x,y,w,h;
	IService::getInstance()->WindowDisplayer->getWindowPos (x,y,w,h);
	log.displayNL ("Window Displayer : XWinParam = %d; YWinParam = %d; WWinParam = %d; HWinParam = %d;", x, y, w, h);
	return true;
}

NLMISC_COMMAND(printConfigFile, "display the variables of the default configfile", "")
{
	IService::getInstance()->ConfigFile.print(&log);
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

} //NLNET
