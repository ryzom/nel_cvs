/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.23 2000/11/24 11:22:13 cado Exp $
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

#include <stdlib.h>
#include <signal.h>

#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on wiindows 95
#define _WIN32_WINDOWS	0x0410
#define WINVER			0x0400
#include <windows.h>

#elif defined NL_OS_UNIX
#include <unistd.h>

#endif

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"

#include "nel/net/base_socket.h"
#include "nel/net/service.h"
#include "nel/net/inet_address.h"
#include "nel/net/naming_client.h"
#include "nel/net/msg_socket.h"
#include "nel/net/pt_callback_item.h"

#include <sstream>

using namespace std;
using namespace NLMISC;


extern NLNET::TCallbackItem CallbackArray [];
extern sint16				CallbackArraySize;


namespace NLNET
{

const uint32 IService::_DefaultTimeout = 1000;


/* "Constants" */

static const sint Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

static const char *SignalName[]=
{
  "SIGABRT", "SIGFPE", "SIGILL", "SIGINT", "SIGSEGV", "SIGTERM"
};

/* Variables */

static sint ExitSignalAsked = 0;

/* Prototypes */

static void SigHandler (int Sig);

/* Functions */

void InitSignal()
{

#ifdef NL_DEBUG
	// in debug mode, we only trap the SIGINT signal
	signal(Signal[3], SigHandler);
#else
	// in release, redirect all signals
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		signal(Signal[i], SigHandler);
	}
#endif
}

// This function is called when a signal comes
static void SigHandler(int Sig)
{
	// redirect the signal for the next time
	signal(Sig, SigHandler);

	// find the signal
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		if (Sig == Signal[i])
		{
			nlinfo ("%s received (%d)", SignalName[i], Sig);
			switch (Sig)
			{
			case SIGABRT :
			case SIGILL  :
			case SIGINT  :
			case SIGSEGV :
			case SIGTERM :
			// you should not call a function and system function like printf in a SigHandle because
			// signal-handler routines are usually called asynchronously when an interrupt occurs.
				ExitSignalAsked = Sig;
				return;
				break;
			}
		}
	}
	nlinfo ("unknown signal received (%d)", Sig);
}


/*
 * Process command line arguments for port and timeout
 */
void IService::getCustomParams()
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


/*
 * main
 */
sint IService::main (int argc, char **argv)
{
	try
	{
		_Server = NULL;

		// Initialize debug stuffs, create displayers for nl* functions
		InitDebug();

		for (sint i = 0; i < argc; i++)
		{
			_Args.push_back (argv[i]);
		}

		setStatus (EXIT_SUCCESS);

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
		CLog::setLocalHostAndService ( localhost, _Name );

#ifdef NL_OS_WINDOWS
#ifdef NL_RELEASE
		InitSignal();
#else
		// don't install signal is the application is started in debug mode
		if (IsDebuggerPresent ())
		{
			nlinfo("Running with the debugger");
		}
		else
		{
			nlinfo("Running without the debugger");
			InitSignal();
		}
#endif
#endif

		// Initialize server parameters
		_Port = IService::_DefaultPort;
		_Timeout = IService::_DefaultTimeout;
		getCustomParams();

		// Register the name to the NS (except for the NS itself)
		if ( strcmp( IService::_Name, "NS" ) != 0 )
		{
			bool registered = false;
			while ( ! registered )
			{
				try
				{
					CNamingClient::open();
					if ( _Port == 0 )
					{
						// Auto-assign port
						_Port = CNamingClient::queryServicePort( IService::_Name, CInetAddress::localHost() );
					}
					// Server start-up
					_Server = new CMsgSocket( CallbackArray, CallbackArraySize, _Port );
					CMsgSocket::setTimeout( _Timeout );
			
					// Register service
					nlassert( _Server->listenAddress() != NULL );
					CNamingClient::registerService( IService::_Name, *(_Server->listenAddress()) );
					CNamingClient::close();
					registered = true;
				}
				catch ( ESocketConnectionFailed& )
				{
					nlwarning( "Could not connect to the Naming Service. Retrying in a few seconds..." );
					CNamingClient::close();

				}
				catch ( ESocket& )
				{
					nlwarning( "Could not register service into the Naming Service." );
					CNamingClient::close();
				}
				if ( ! registered )
				{
#ifdef NL_OS_WINDOWS
					Sleep( 5000 ); // wait 5 seconds
#elif define NL_OS_UNIX
					sleep( 5 ); // wait 5 seconds
#endif
				}
			}
		}
		else
		{
			// Server start-up
			_Server = new CMsgSocket( CallbackArray, CallbackArraySize, _Port );
			CMsgSocket::setTimeout( _Timeout );
		}

		// User service init
		init ();

		nlinfo( "Service ready" );

		// user service update call each loop
		while ( update() && !ExitSignalAsked)
		{
			CConfigFile::checkConfigFiles ();
			CMsgSocket::update();
		}
	}
	catch (EFatalError &)
	{
		setStatus (EXIT_FAILURE);

		// somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
	}
	catch (Exception &e)
	{
		setStatus (EXIT_FAILURE);

		// we don't use nlerror macro because we don't want to generate an exit exception
		nlError ("Error running the service \"%s\": %s", _Name, e.what());
	}
#ifdef NL_RELEASE
	catch (...)
	{
		// in release mode, we catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);

		// we don't use nlerror macro because we don't want to generate an exit exception
		nlError ("Unknown external exception");
	}
#endif
	try
	{
		release ();
		
		// Unregister service
		CNamingClient::finalize();

		// Close server
		if (_Server != NULL )
		{
			delete _Server;
			_Server = NULL;
		}
		nlinfo ("Service stopped");
	}
	catch (Exception &e)
	{
		nlerror ("Error releasing service : %s", e.what());
		setStatus (EXIT_FAILURE);
	}

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}

} //NLNET
