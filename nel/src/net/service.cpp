/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.18 2000/10/12 16:15:31 cado Exp $
 *
 * \todo ACE: test the signal redirection on Unix
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
			// Release only once
			IService *is = Service;
			Service = NULL;

			// Unregister service
			CNamingClient::finalize();

			// Close server
			if ( is->_Server != NULL )
			{
				delete is->_Server;
			}

			is->release ();
			nlinfo( "Service stopped" );
		}
	}
	catch (Exception &e)
	{
		nlerror ("Error releasing service : %s", e.what());
		if (Service != NULL) Service->setStatus (EXIT_FAILURE);
	}
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

		// Initialize debug stuffs, create displayers for rk* functions
		InitDebug();

		// Initialize server parameters
		_Port = IService::_DefaultPort;
		_Timeout = IService::_DefaultTimeout;
		getCustomParams();

		// Server start-up
		_Server = new CMsgSocket( CallbackArray, CallbackArraySize, _Port );
		CMsgSocket::setTimeout( _Timeout );

		// User service init
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

		nlinfo( "Service ready" );

		// user service update call each loop
		while ( update() )
		{
			CConfigFile::checkConfigFiles ();
			CMsgSocket::update();
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