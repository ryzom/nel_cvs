/** \file service.cpp
 * Base class for all network services
 *
 * $Id: new_service.cpp,v 1.7 2001/03/15 15:09:18 coutelas Exp $
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

#elif defined NL_OS_UNIX

#include <unistd.h>

#endif

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"

#include "nel/net/new_service.h"

#include "nel/net/base_socket.h"
#include "nel/net/net_displayer.h"
#include "nel/net/inet_address.h"
#include "nel/net/naming_client.h"
#include "nel/net/net_log.h"

#include "nel/net/callback_server.h"

#include <sstream>

using namespace std;
using namespace NLMISC;


namespace NLNET
{

string INewService::_Name = "";
uint16 INewService::_DefaultPort = 0;

const uint32 INewService::_DefaultTimeout = 1000;

INewService	 *INewService::Instance = NULL;


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
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		signal(Signal[i], sigHandler);
		nldebug("Signal %s (%d) trapped", SignalName[i], Signal[i]);
	}
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
INewService::INewService()
{
	// Singleton
	nlassert( INewService::Instance == NULL );
	INewService::Instance = this;
}


/*
 * Process command line arguments for port and timeout
 */
void INewService::getCustomParams()
{
	// At the moment we don't have a processing system yet
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
			_Timeout = timeout;
			return;
		}
	}

}

void INewService::setServiceName (const char *ServiceName)
{
	_Name = ServiceName;
}

void INewService::setPort (uint16 Port)
{
	_DefaultPort = Port;
}

// The main function of the service
sint INewService::main (int argc, char **argv)
{
	try
	{
		_Server = NULL;

		//
		// Initialize debug stuffs, create displayers for nl* functions
		//

		initDebug();
#ifdef NL_RELEASE
		ErrorLog.addDisplayer (&sd);
		WarningLog.addDisplayer (&sd);
		InfoLog.addDisplayer (&sd);
#endif // NL_RELEASE

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
			CBaseSocket::init();

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
		processName += _Name;
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

		_Port = INewService::_DefaultPort;
		_Timeout = INewService::_DefaultTimeout;
		getCustomParams();

		//
		// Server start-up
		//

		_Server = new CCallbackServer;
		//
		// Call the user service init
		//

		init ();

		_Server->setTimeout(_Timeout);

		//
		// Register the name to the NS (except for the NS itself)
		//

		if ( INewService::_Name != "NS" )
		{
			// Setup Net Displayer
			CNetDisplayer *nd = new CNetDisplayer();
			if ( nd->connected() )
			{
				NetLog.addDisplayer( nd );

				// Add the net displayer for all debug information
				ErrorLog.addDisplayer (nd);
				WarningLog.addDisplayer (nd);
				InfoLog.addDisplayer (nd);
#ifdef NL_DEBUG
				DebugLog.addDisplayer (nd);
				AssertLog.addDisplayer (nd);
#endif
			}


			// Get the universal time (useful for debugging)
			if ( INewService::_Name != "TS" )
			{
				// Don't call the sync if it's the Time Service and Naming Service
				CUniTime::syncUniTimeFromService ();
			}


			if ( INewService::_Name != "LS" ) // The Login Service must not register itself
			{
				// Talk with the NS
				bool registered = false;
				while ( ! registered )
				{
					try
					{
						// Connect to the NS and keep connection (to detect unexpected service closure)
						CNamingClient::open();

						if ( _Port == 0 )
						{
							// Auto-assign port
							_Port = CNamingClient::queryServicePort( INewService::_Name, CInetAddress::localHost() );
						}

						_Server->init (_Port);

						// Register service
						setServiceId( CNamingClient::registerService( INewService::_Name, _Server->listenAddress() ) );
						registered = true;
					}
					catch ( ESocketConnectionFailed& )
					{
						nlwarning( "Could not connect to the Naming Service. Retrying in a few seconds..." );
						CNamingClient::close();

					}
					catch ( ESocket& e )
					{
						nlwarning( "Could not register service into the Naming Service : %s", e.what() );
						CNamingClient::close();
					}
					if ( ! registered )
					{
	#ifdef NL_OS_WINDOWS
						Sleep( 5000 ); // wait 5 seconds
	#elif defined NL_OS_UNIX
						sleep( 5 ); // wait 5 seconds
	#endif
					}
				}
			}
			else
			{
				_Server->init (_Port);
			}
		}
		else
		{
			_Server->init (_Port);
		}

		//
		// On unix system, the service fork itself to give back the hand to the shell
		//

#ifdef NL_OS_UNIX

		nlinfo( "Forking the service" );

		int pid = fork();

		if (pid == -1)
		{
			nlerror ("Couldn't fork the service");
		}
		else if (pid != 0)
		{
			// It's the father, return the hand to the shell.
			exit(EXIT_SUCCESS);
		}

#endif // NL_OS_UNIX

		nlinfo( "Service ready" );
		//
		// Call the user service update each loop and check files and network activity
		//

		while ( update() && !ExitSignalAsked)
		{
			CConfigFile::checkConfigFiles ();
			_Server->update();
		}
	}
	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}
	catch (Exception &e)
	{
		// Catch NeL exception to release the system cleanly
		setStatus (EXIT_FAILURE);

		try
		{
			nlerror ("NeL Exception: Error running the service \"%s\": %s", _Name.c_str(), e.what());
		}
		catch (EFatalError &)
		{
			// Ignore the fatalerror
		}
	}
	catch (...)
	{
		// Catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);

		try
		{
			nlerror ("Unknown external exception");
		}
		catch (EFatalError &)
		{
			// Ignore the fatalerror
		}
	}

	try
	{
		//
		// Call the user service release
		//

		release ();

		//
		// Unregister the service if needed
		//

		if ( ( INewService::_Name != "NS") && ( INewService::_Name != "LS") )
		{
			CNamingClient::finalize();
			CNamingClient::close(); // close connection to the NS
		}

		//
		// Close server network
		//

		if (_Server != NULL )
		{
			delete _Server;
			_Server = NULL;
		}
		nlinfo ("Service stopped");
	}
	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}
	catch (Exception &e)
	{
		setStatus (EXIT_FAILURE);
		try
		{
			nlerror ("NeL Exception: Error releasing the service \"%s\": %s", _Name.c_str(), e.what());
		}
		catch (EFatalError &)
		{
			// Ignore the fatalerror
		}
	}
	catch (...)
	{
		// Catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);

		try
		{
			nlerror ("Unknown external exception");
		}
		catch (EFatalError &)
		{
			// Ignore the fatalerror
		}
	}

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}

} //NLNET
