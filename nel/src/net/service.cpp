/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.110 2002/03/20 12:43:49 lecroart Exp $
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

#include "stdnet.h"

#ifdef NL_OS_WINDOWS

// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	include <windows.h>

#elif defined NL_OS_UNIX

#	include <unistd.h>

#endif

#include <signal.h>

#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mutex.h"
#include "nel/misc/window_displayer.h"
#include "nel/misc/gtk_displayer.h"
#include "nel/misc/win_displayer.h"

#include "nel/net/naming_client.h"
#include "nel/net/service.h"
#include "nel/net/net_displayer.h"
#include "nel/net/net_log.h"
#include "nel/net/unitime.h"
#include "nel/net/callback_server.h"
#include "nel/net/net_manager.h"

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

static sint ExitSignalAsked = 0;
static CStdDisplayer sd;

// services stat
static sint32  _NetSpeedLoop, _UserSpeedLoop;

// class static member

string		 IService::_ShortName;
string		 IService::_LongName;
string		 IService::_AliasName;
string		 IService::_ConfigDir;
string		 IService::_LogDir;

uint16		 IService::_DefaultPort		= 0;
TTime		 IService::_UpdateTimeout	= 100;
CEntityId	 IService::_NextEntityId;

IService	*IService::Instance			= NULL;
CConfigFile  IService::ConfigFile;


//
// Prototypes
//

static void sigHandler (int Sig);

//
// Functions
//

// this is the thread that initialized the signal redirection
// we ll ignore other thread signals
static uint SignalisedThread;

static void initSignal()
{
	SignalisedThread = getThreadId ();
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
*/
#endif
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



/*
 * Constructor
 */
IService::IService()
{
	// Singleton
	nlassert( IService::Instance == NULL );
	IService::Instance = this;
	_Initialized = false;
	_WindowDisplayer = NULL;
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

CLog _commandLog;
CNetDisplayer _commandDisplayer(false);

void AESConnection (const string &serviceName, TSockId from, void *arg)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	CMessage msgout (CNetManager::getSIDA ("AES"), "SID");
	msgout.serial (IService::_AliasName, IService::_ShortName, IService::_LongName);
	ICommand::serialCommands (msgout);
	CNetManager::send ("AES", msgout);

	if (IService::Instance->_Initialized)
	{
		CMessage msgout2 (CNetManager::getSIDA ("AES"), "SR");
		CNetManager::send ("AES", msgout2);
	}

	// add the displayer to the standard logger
	CCallbackClient *client = dynamic_cast<CCallbackClient *>(CNetManager::getNetBase("AES"));
	_commandDisplayer.setLogServer (client);
	_commandLog.addDisplayer (&_commandDisplayer);
}


void AESDisconnection (const string &serviceName, TSockId from, void *arg)
{
	_commandLog.removeDisplayer (&_commandDisplayer);
}

static void cbExecCommand (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	string command;
	msgin.serial (command);

	ICommand::execute (command, _commandLog);
}

// if we receive the stop service, we try to exit now
static void cbStopService (CMessage& msgin, TSockId from, CCallbackNetBase &netbase)
{
	ExitSignalAsked = 0xFFFF;
}


static TCallbackItem AESCallbackArray[] =
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

static CFileDisplayer fd;

void IService::setServiceName (const char *shortName, const char *longName)
{
	_ShortName = shortName;
	_LongName = longName;

	// now we have the service name, we create the log with this service

	createDebug ();

	fd.setParam (getLogDir() + _LongName + ".log", false);

	DebugLog->addDisplayer (&fd);
	InfoLog->addDisplayer (&fd);
	WarningLog->addDisplayer (&fd);
	AssertLog->addDisplayer (&fd);
	ErrorLog->addDisplayer (&fd);
}


sint IService::main (char *args)
{
	_Args.push_back ("program name");

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

	return main ();
}

sint IService::main (int argc, char **argv)
{
	for (sint i = 0; i < argc; i++)
	{
		_Args.push_back (argv[i]);
	}
	return main ();
}

// The main function of the service
sint IService::main ()
{
	bool userInitCalled = false;
	bool resyncEvenly = false;

	try
	{
		//
		// init debug stuffs
		//

		createDebug ();

		DebugLog->addNegativeFilter ("NETL");

		//
		// Load the config file
		//

		ConfigFile.load (getConfigDir() + _LongName + ".cfg");

		try
		{
			string disp = ConfigFile.getVar ("WindowStyle").asString ();
#ifdef NL_USE_GTK
			if (disp == "GTK")
			{
				_WindowDisplayer = new CGtkDisplayer ("DEFAULT_WD");
			}
#endif // NL_USE_GTK

#ifdef NL_OS_WINDOWS
			if (disp == "WIN")
			{
				_WindowDisplayer = new CWinDisplayer ("DEFAULT_WD");
			}
#endif // NL_OS_WINDOWS

			if (_WindowDisplayer == NULL && disp != "NONE")
			{
				nlwarning ("Unknown value for the WindowStyle (should be GTK, WIN or NONE), use no window displayer");
			}
		}
		catch (EUnknownVar&)
		{
			// no WindowStyle variable, no displayer
		}

		uint speedNetLabel, speedUsrLabel, rcvLabel, sndLabel, rcvQLabel, sndQLabel, scrollLabel;
		if (_WindowDisplayer != NULL)
		{
			//
			// Init window param if necessary
			//

			sint x=-1, y=-1, w=-1, h=-1;

			try { x = ConfigFile.getVar("XWinParam").asInt(); } catch (EUnknownVar&) { }
			try { y = ConfigFile.getVar("YWinParam").asInt(); } catch (EUnknownVar&) { }
			try { w = ConfigFile.getVar("WWinParam").asInt(); } catch (EUnknownVar&) { }
			try { h = ConfigFile.getVar("HWinParam").asInt(); } catch (EUnknownVar&) { }

			if (w == -1 && h == -1)
				_WindowDisplayer->create (_ShortName + " " + _LongName, x, y);
			else
				_WindowDisplayer->create (_ShortName + " " + _LongName, x, y, w, h);

			DebugLog->addDisplayer (_WindowDisplayer);
			InfoLog->addDisplayer (_WindowDisplayer);
			WarningLog->addDisplayer (_WindowDisplayer);
			ErrorLog->addDisplayer (_WindowDisplayer);
			AssertLog->addDisplayer (_WindowDisplayer);
			speedNetLabel = _WindowDisplayer->createLabel ("");
			speedUsrLabel = _WindowDisplayer->createLabel ("");
			rcvLabel = _WindowDisplayer->createLabel ("");
			sndLabel = _WindowDisplayer->createLabel ("");
			rcvQLabel = _WindowDisplayer->createLabel ("");
			sndQLabel = _WindowDisplayer->createLabel ("");
			scrollLabel = _WindowDisplayer->createLabel ("");
		}

		nlinfo ("Starting Service '%s' using NeL ("__DATE__" "__TIME__")", _ShortName.c_str());

		//
		// Display command line arguments
		//

		nlinfo ("args = %d", _Args.size ());
		for (uint i = 0; i < _Args.size (); i++)
		{
			nlinfo ("argv[%d] = '%s'", i, _Args[i].c_str ());
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
		// Initialize server parameters
		//

		// 1: get the port from the service application
		_Port = IService::_DefaultPort;

		// 2: get the port from config file if in it
		try
		{
			_Port = ConfigFile.getVar("Port").asInt();
		}
		catch ( EUnknownVar& )
		{
		}

		
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

		// Load the default stream format
		try
		{
			CMessage::setDefaultStringMode( ConfigFile.getVar("StringMsgFormat").asInt() == 1 );
		}
		catch ( EUnknownVar& )
		{
			// Not found => binary
			CMessage::setDefaultStringMode( false );
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
		// Now we have the service id, we can set the entites id generator
		//

		_NextEntityId.setServiceId(_SId);

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
/// \todo ace: activate the netloger when we resolved the recursive logging bug
/// \todo ace: don't forget to delete the netloger at the end of the program
/*			if (IService::_Name != "LOGS")
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
/*
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
*/
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
			// count the amount of time to manage internal system
			TTime bbefore = CTime::getLocalTime ();

			// call the user update and exit if the user update asks it
			if (!update ()) break;
			
			// count the amount of time to manage internal system
			TTime before = CTime::getLocalTime ();

			if (_WindowDisplayer != NULL)
			{
				// update the window displayer and quit if asked
				if (!_WindowDisplayer->update ())
					ExitSignalAsked = true;
			}

			// stop the loop if the exit signal asked
			if (ExitSignalAsked) break;
	
			CConfigFile::checkConfigFiles ();

			// get and manage layer 4 messages
			CNetManager::update (_UpdateTimeout);
			
			// TEMP: always sleep one millisecond for multitasking
//			nlSleep (1);
/*
			// resync the clock every hours
			if (resyncEvenly)
			{
				static TTime LastSyncTime = CTime::getLocalTime ();
*/
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
			}
*/

			_NetSpeedLoop = (sint32) (CTime::getLocalTime () - before);
			_UserSpeedLoop = (sint32) (before - bbefore);

			if (_WindowDisplayer != NULL)
			{
				string str;
				str = "NetLop: ";
				str += toString (_NetSpeedLoop);
				_WindowDisplayer->setLabel (speedNetLabel, str);
				str = "UsrLop: ";
				str += toString (_UserSpeedLoop);
				_WindowDisplayer->setLabel (speedUsrLabel, str);
				str = "Rcv: ";
				str += toString (CNetManager::getBytesReceived ());
				_WindowDisplayer->setLabel (rcvLabel, str);
				str = "Snd: ";
				str += toString (CNetManager::getBytesSent ());
				_WindowDisplayer->setLabel (sndLabel, str);
				str = "RcvQ: ";
				str += toString (CNetManager::getReceiveQueueSize ());
				_WindowDisplayer->setLabel (rcvQLabel, str);
				str = "SndQ: ";
				str += toString (CNetManager::getSendQueueSize ());
				_WindowDisplayer->setLabel (sndQLabel, str);

				// display the scroll text
				static string foo =	"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! "
									"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! ";
				static int pos = 0;
				_WindowDisplayer->setLabel (scrollLabel, foo.substr (pos%(foo.size()/2), 10));
				pos++;
			}

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

		CSock::releaseNetwork ();

		if (_WindowDisplayer != NULL)
		{
			DebugLog->removeDisplayer (_WindowDisplayer);
			InfoLog->removeDisplayer (_WindowDisplayer);
			WarningLog->removeDisplayer (_WindowDisplayer);
			ErrorLog->removeDisplayer (_WindowDisplayer);
			AssertLog->removeDisplayer (_WindowDisplayer);

			// Never delete the windows displayer because another thread could use it
			//delete _WindowDisplayer;
			//_WindowDisplayer = NULL;
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

	nlinfo ("Service ends");

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}


//
// Commands and Variables for controling all services
//

NLMISC_VARIABLE(sint32, _NetSpeedLoop, "duration of the last network loop (in ms)");
NLMISC_VARIABLE(sint32, _UserSpeedLoop, "duration of the last user loop (in ms)");

NLMISC_DYNVARIABLE(uint64, _ReceivedBytes, "total of bytes received by this service")
{
	// we can only read the value
	if (get) *pointer = CNetManager::getBytesReceived ();
}

NLMISC_DYNVARIABLE(uint64, _SentBytes, "total of bytes sended by this service")
{
	// we can only read the value
	if (get) *pointer = CNetManager::getBytesSent ();
}

NLMISC_DYNVARIABLE(uint64, _ReceivedQueueSize, "current size in bytes of the received queue size")
{
	// we can only read the value
	if (get) *pointer = CNetManager::getReceiveQueueSize ();
}

NLMISC_DYNVARIABLE(uint64, _SentQueueSize, "current size in bytes of the sended queue size")
{
	// we can only read the value
	if (get) *pointer = CNetManager::getSendQueueSize ();
}

NLMISC_COMMAND (_quit, "exit the service", "")
{
	if(args.size() != 0) return false;

	ExitSignalAsked = 0xFFFF;

	return true;
}

NLMISC_COMMAND (_brutal_quit, "exit the service brutally", "")
{
	if(args.size() != 0) return false;

	exit (0xFFFFFFFF);

	return true;
}

#ifdef MUTEX_DEBUG
NLMISC_COMMAND (_mutex, "display mutex values", "")
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

} //NLNET
