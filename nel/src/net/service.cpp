/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.10 2000/10/11 08:31:07 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/types_nl.h"

#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"

#include "nel/net/base_socket.h"
#include "nel/net/service.h"
#include "nel/net/inet_address.h"

#include "nel/net/service.h"

using namespace std;
using namespace NLMISC;

namespace NLNET
{

static IService *Service = NULL;

void ExitFunc ()
{
	try
	{
		if (Service != NULL)
		{
			// release only one time
			IService *is = Service;
			Service = NULL;

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
		Service = this;
		atexit (ExitFunc);

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
		CLog::setLocalHostAndService (localhost, _Name);

		// initialize debug stuffs, create displayers for rk* functions
		InitDebug();

		// user service init
		init ();

		// user service update call each loop
		while (update())
		{
			CConfigFile::checkConfigFiles ();
		}
		ExitFunc ();
	}
	catch (Exception &e)
	{
		nlerror ("Error running the service \"%s\": %s", _Name, e.what());
		setStatus (EXIT_FAILURE);
		ExitFunc ();
	}
	catch (...)
	{
		nlerror ("Unknown external exception");
		setStatus (EXIT_FAILURE);
		ExitFunc ();
	}
	return getStatus ();
}

} //NLNET