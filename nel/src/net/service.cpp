/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.16 2000/10/12 10:43:11 cado Exp $
 *
 * implementation of all debug functions
 *
 */

/// \todo ACE: test the signal redirection on unix

#include "nel/misc/types_nl.h"

#include <stdlib.h>
#include <signal.h>
#include <signal.h>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"

#include "nel/net/base_socket.h"
#include "nel/net/service.h"
#include "nel/net/inet_address.h"
#include "nel/net/naming_client.h"
#include "nel/net/msg_socket.h"

using namespace std;
using namespace NLMISC;

namespace NLNET
{

/* "Constants" */

static const int Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

static const char *SignalName[]=
{
  "SIGABRT", "SIGFPE", "SIGILL", "SIGINT", "SIGSEGV", "SIGTERM"
};

/* Variables */

static IService *Service = NULL;

/* Prototypes */

static void SigHandler (int Sig);
static void ExitFunc ();

/* Functions */

void InitSignal()
{
	// redirect all signals
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
		signal(Signal[i], SigHandler);
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
			nlinfo ("%s received", SignalName[i]);
			switch (Sig)
			{
			case SIGABRT :
			case SIGILL  :
			case SIGINT  :
			case SIGSEGV :
			case SIGTERM :
			// you should not call a function and system function like printf in a SigHandle because
			// signal-handler routines are usually called asynchronously when an interrupt occurs.
			ExitFunc ();
			exit (100+Sig);
			break;
			}
		}
	}
	nlwarning ("Signal #%d received", Sig);
}


///////////////////////////////

static void ExitFunc ()
{
	nldebug("** ExitFunc **");
	try
	{
		if (Service != NULL)
		{
			// release only one time
			IService *is = Service;
			Service = NULL;

			CNamingClient::finalize();

			nldebug("** ExitFunc Release **");
			is->release ();
		}
	}
	catch (Exception &e)
	{
		nlerror ("Error releasing service : %s", e.what());
		if (Service != NULL) Service->setStatus (EXIT_FAILURE);
	}
}


sint IService::main (int argc, char **argv)
{
	try
	{
		_Server = NULL;
		Service = this;
		atexit (ExitFunc);

		InitSignal();

		for (sint i = 0; i < argc; i++)
		{
			_Args.push_back (argv[i]);
		}

		setStatus (EXIT_SUCCESS);

		string localhost;
		try
		{
			// initialize WSAStartup and network stuffs
			CBaseSocket::init();

			// get the localhost name
			localhost = CInetAddress::localHost().hostName();
		}
		catch (NLNET::ESocket &)
		{
			localhost = "<UnknownHost>";
		}

		// set the localhost name and service name to the logger
		CLog::setLocalHostAndService ( localhost, _Name );

		// initialize debug stuffs, create displayers for rk* functions
		InitDebug();

		// user service init
		init ();

		// Register the name to the NS (except for the NS itself)
		if ( strcmp( IService::_Name, "NS" ) != 0 )
		{
			try
			{
				CNamingClient::registerService( IService::_Name, *(_Server->listenAddress()) );
			}
			catch ( ESocketConnectionFailed& )
			{
				nlwarning( "Could not connect to the Naming Service. The NS looks down" );
			}
			catch ( ESocket& )
			{
				nlwarning( "Could not register service into the Naming Service." );
			}
		}

		// user service update call each loop
		while ( update() )
		{
			CConfigFile::checkConfigFiles ();
		}
		ExitFunc ();
	}
	catch (Exception &e)
	{
		ExitFunc ();
		setStatus (EXIT_FAILURE);
		nlerror ("Error running the service \"%s\": %s", _Name, e.what());
	}
	catch (...)
	{
		ExitFunc ();
		setStatus (EXIT_FAILURE);
		nlerror ("Unknown external exception");
	}
	return getStatus ();
}

} //NLNET