/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.71 2001/06/18 09:09:20 cado Exp $
 *
 * \todo ace: test the signal redirection on Unix
 * \todo ace: add parsing command line (with CLAP?)
 */

/* Copyright, 2000 Nevrax Ltd.
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


#include "nel/misc/types_nl.h"
#include "nel/net/unitime.h"

#include <stdlib.h>
#include <signal.h>

#ifdef NL_OS_WINDOWS

// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#define _WIN32_WINDOWS	0x0410
#define WINVER			0x0400
#include <windows.h>

#include "nel/misc/win_displayer.h"

#elif defined NL_OS_UNIX

#include <unistd.h>

#endif

#include "nel/misc/common.h"
#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"

#include "nel/net/naming_client.h"
#include "nel/net/service.h"
#include "nel/net/net_displayer.h"
#include "nel/net/net_log.h"
#include "nel/net/unitime.h"

#include "nel/net/callback_server.h"
#include "nel/net/net_manager.h"

#include <sstream>

using namespace std;
using namespace NLMISC;


namespace NLNET
{

string IService::_ShortName = "";
string IService::_LongName = "";
string IService::_AliasName= "";
uint16 IService::_DefaultPort = 0;

sint32 IService::_UpdateTimeout = 0;


CConfigFile IService::ConfigFile;

IService	 *IService::Instance = NULL;


// Constants

static const sint Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

static const char *SignalName[]=
{
  "SIGABRT", "SIGFPE", "SIGILL", "SIGINT", "SIGSEGV", "SIGTERM"
};

// Variables

static sint ExitSignalAsked = 0;
static CStdDisplayer sd;

// Prototypes

static void sigHandler (int Sig);

// Functions

static void initSignal()
{

#ifdef NL_DEBUG
	// in debug mode, we only trap the SIGINT signal
	signal(Signal[3], sigHandler);
	nldebug("Signal : %s (%d) trapped", SignalName[3], Signal[3]);
#else
	// in release, redirect all signals
/* don't redirect now because to hard to debug...

	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		signal(Signal[i], sigHandler);
		nldebug("Signal %s (%d) trapped", SignalName[i], Signal[i]);
	}
*/#endif
}

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
	nlinfo ("Unknown signal received (%d)", Sig);
}



/*
 * Constructor
 */
IService::IService()
{
	// Singleton
	nlassert( IService::Instance == NULL );
	IService::Instance = this;
	_Initialized = false;
}


/*
 * Process command line arguments for port and timeout (commented till the new argument param management)
 */
void IService::getCustomParams()
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i][0] == '-')
		{
			if (_Args[i][1] == 'n')
			{
				_AliasName = _Args[i].substr (2);
			}
			else
			{
				nlwarning ("Unknown parameter: %s", _Args[i].c_str());
			}
		}
	}

/*	// At the moment we don't have a processing system yet
	if ( _Args.size() > 1 )
	{
		stringstream ss( _Args[1] );
		uint port;
		ss >> port;
		_Port = (uint16)port; 
		return;

		if ( _Args.size() > 2 )
		{
			stringstream ss( _Args[2] );
			uint timeout;
			ss >> timeout;
			_UpdateTimeout = timeout;
			return;
		}
	}
*/
}


NLMISC_COMMAND (hello, "hello", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Hello World!");
	return true;
}

CLog commandLog;
CNetDisplayer commandDisplayer(false);

void AESConnection (const string &serviceName, TSockId from, void *arg)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	CMessage msgout (CNetManager::getSIDA ("AES"), "SID");
	msgout.serial (IService::_AliasName, IService::_ShortName, IService::_LongName);
	vector<string> commands;
	ICommand::getCommands (commands);
	msgout.serialCont (commands);
	CNetManager::send ("AES", msgout);

	if (IService::Instance->_Initialized)
	{
		CMessage msgout2 (CNetManager::getSIDA ("AES"), "SR");
		CNetManager::send ("AES", msgout2);
	}

	// add the displayer to the standard logger
	CCallbackClient *client = dynamic_cast<CCallbackClient *>(CNetManager::getNetBase("AES"));
	commandDisplayer.setLogServer (client);
	commandLog.addDisplayer (&commandDisplayer);
}


void AESDisconnection (const string &serviceName, TSockId from, void *arg)
{
	commandLog.removeDisplayer (&commandDisplayer);
}

static void cbExecCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string command;
	msgin.serial (command);

	ICommand::execute (command, commandLog);
}

// if we receive the stop service, we try to exit now
static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	ExitSignalAsked = true;
}


TCallbackItem AESCallbackArray[] =
{
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
};

/*
 * Returns a pointer to the CCallbackServer object
 */
CCallbackServer *IService::getServer()
{
	return dynamic_cast<CCallbackServer*>(CNetManager::getNetBase(IService::_ShortName));
}

// The main function of the service
sint IService::main (int argc, char **argv, void *wd)
{
	bool userInitCalled = false;
	bool resyncEvenly = false;

	try
	{
		createDebug ();
#if defined (NL_OS_WINDOWS)
		if (wd != NULL)
		{
			DebugLog->addDisplayer ((CWinDisplayer *)wd);
			InfoLog->addDisplayer ((CWinDisplayer *)wd);
			WarningLog->addDisplayer ((CWinDisplayer *)wd);
			ErrorLog->addDisplayer ((CWinDisplayer *)wd);
			AssertLog->addDisplayer ((CWinDisplayer *)wd);
		}
#endif

		nlinfo ("Starting Service '%s' using NeL ("__DATE__" "__TIME__")", _ShortName.c_str());
		DebugLog->addNegativeFilter ("L3NB_ASSOC:");
		DebugLog->addNegativeFilter ("L3NB_CB:");


		//
		// Parse argc argv into easy to use format
		//

		for (sint i = 0; i < argc; i++)
		{
			_Args.push_back (argv[i]);
		}

		setStatus (EXIT_SUCCESS);


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
		// Redirect signal if needed (in release mode only)
		//

#ifdef NL_OS_WINDOWS
#ifdef NL_RELEASE
		initSignal();
#else
		// don't install signal is the application is started in debug mode
		if (IsDebuggerPresent ())
		{
			nlinfo("Running with the debugger, don't redirect signals");
			initSignal();
		}
		else
		{
			nlinfo("Running without the debugger, redirect SIGINT signal");
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
		// Load the config file
		//

		ConfigFile.load (_LongName + ".cfg");

		//
		// Initialize server parameters
		//

		_Port = IService::_DefaultPort;

		getCustomParams();

		// Load the recording state from the config file
		try
		{
			string srecstate = ConfigFile.getVar("Rec").asString();
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
		catch ( EUnknownVar& )
		{
			// Not found
			_RecordingState = CCallbackNetBase::Off;
		}

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

		//
		// Connect to the local AES and send identification
		//

		if (_ShortName != "AES" && _ShortName != "AS")
		{
			CNetManager::setConnectionCallback ("AES", AESConnection, NULL);
			CNetManager::setDisconnectionCallback ("AES", AESDisconnection, NULL);
			CNetManager::addClient ("AES", "localhost:49997");
			CNetManager::addCallbackArray ("AES", AESCallbackArray, sizeof(AESCallbackArray)/sizeof(AESCallbackArray[0]));
		}

		//
		// Add the server of this service
		//

		try
		{
			sint32 sid = ConfigFile.getVar("SId").asInt();
			if (sid<0 || sid>255)
			{
				nlwarning("Bad SId in the config file, %d is not in [0;255] range", sid);
				_SId = 0;
			}
			else
			{
				_SId = (uint8) sid;
			}
		}
		catch(EUnknownVar&)
		{
			// ok, SId not found, use dynamic sid
			_SId = 0;
		}

		CNetManager::addServer (_ShortName, _Port, _SId);
		CNetManager::addCallbackArray (_ShortName, _CallbackArray, _CallbackArraySize);

		//
		// Call the user service init
		//

		userInitCalled = true; // the bool must be put *before* the call to init()
		init ();

		//
		// Register the name to the NS (except for the NS itself)
		//

		if (IService::_ShortName != "NS" && IService::_ShortName != "LS" && IService::_ShortName != "AES" && IService::_ShortName != "AS")
		{

			//
			// Setup Net Displayer
			//
/* todo: mettre le netloger kan on aura reussi a virer le probleme des log recursif
   todo: ne pas oublier de deleter nd a la fin du programme
			if (IService::_Name != "LOGS")
			{
				CNetDisplayer *nd = new CNetDisplayer();
				if ( nd->connected() )
				{
					NetLog.addDisplayer( nd );

					// Add the net displayer for all debug information
					ErrorLog->addDisplayer (nd);
					WarningLog->addDisplayer (nd);
					InfoLog->addDisplayer (nd);
#ifdef NL_DEBUG
					DebugLog->addDisplayer (nd);
					AssertLog->addDisplayer (nd);
#endif
				}
			}
*/

			//
			// Get the universal time (useful for debugging)
			//

			// Don't call the sync if it's the Time Service or the Naming Service
			if ( IService::_ShortName != "TS" )
			{
				if ( _RecordingState == CCallbackNetBase::Replay )
				{
					CUniTime::simulate(); 
				}
				CUniTime::syncUniTimeFromService ( _RecordingState );
				resyncEvenly = true;
			}

			//
			// Talk with the NS to get the port if necessary and register the service
			//
/*
			bool registered = false;
			while (!registered)
			{
				try
				{
					if (_Port == 0)
					{
						// Auto-assign port, ask it to the naming service
						_Port = CNamingClient::queryServicePort ();
					}


					Server->init (_Port);

					// Register service
					setServiceId (CNamingClient::registerService (IService::_Name, Server->listenAddress ()));
					registered = true;
				}
				catch ( ESocket& e )
				{
					nlwarning( "Could not register service into the Naming Service : %s", e.what() );
				}
				if ( ! registered )
				{
					// wait 5s before retrying
					nlSleep (5000);
				}
			}
		}
		else
		{
			Server->init (_Port);
		*/
		}


		//
		// On Unix system, the service fork itself to give back the hand to the shell
		//
		// note: we don't forking anymore because it doesn't work with thread system
		//

#ifdef NL_OS_UNIX
		/*
		nlinfo( "Forking the service" );

		int pid = fork();

		// todo ace: probleme kan on fork sous linux car le process pere essaie de liberer les thread => torche

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

		if (_ShortName != "AES" && _ShortName != "AS")
		{
			// send the ready message (service init finished)
			CMessage msgout (CNetManager::getSIDA ("AES"), "SR");
			CNetManager::send ("AES", msgout);
		}

		_Initialized = true;
		
		nlinfo ("Service ready");

		//
		// Call the user service update each loop and check files and network activity
		//

		do
		{
			// call the user update and exit if the user update asks it
			if (!update ()) break;
			
			// stop the loop if the exit signal asked
			if (ExitSignalAsked) break;

#ifdef NL_OS_WINDOWS
			// update the window displayer
			((CWinDisplayer *)wd)->update ();
#endif // NL_OS_WINDOWS

			// count the amount of time to manage internal system
			TTime before = CTime::getLocalTime ();
	
			CConfigFile::checkConfigFiles ();

			// get and manage layer 4 messages
			CNetManager::update (_UpdateTimeout);
			
			// TEMP: always sleep one millisecond for multitasking
//			nlSleep (1);

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

				if (CTime::getLocalTime () - LastSyncTime > 60*60*1000)
				{
					CUniTime::syncUniTimeFromService ( _RecordingState );
					LastSyncTime = CTime::getLocalTime ();
				}
			}

			sint32 delta = (sint32)(CTime::getLocalTime () - before);

//			nldebug ("SYNC: updatetimeout must be %d and is %d, sleep the rest of the time", _UpdateTimeout, delta);

			// now, sleep the rest of the time if needed
//			if (_UpdateTimeout > 0 && delta <= _UpdateTimeout)
//				nlSleep (_UpdateTimeout - delta);
		}
		while (true);
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
*/#endif

	try
	{
		nlinfo ("Service starts releasing");

		//
		// Call the user service release() if the init() was called
		//

		if (userInitCalled)
			release ();


		//
		// Disconnect from the Naming Service, if necessary
		//

		CNetManager::release ();

		CSock::releaseNetwork();

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

	nlinfo ("Service ends");

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}

} //NLNET
