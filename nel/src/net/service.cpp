/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.8 2000/10/09 14:12:37 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/types_nl.h"

#include <stdlib.h>
#include <signal.h>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"

#include "nel/net/service.h"

#include <iostream>
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
			Service->release ();
	}
	catch (Exception& e)
	{
		cout << "Error releasing service : " << e.what() << endl;
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

		NLMISC_InitDebug();

		init ();
		while (update())
		{
			CConfigFile::checkConfigFiles ();
		}
		Service = NULL;
		release ();
	}
	catch (Exception &e)
	{
		cout << e.what() << endl;
		setStatus (EXIT_FAILURE);
		ExitFunc ();
	}
	catch (...)
	{
		cout << "Unknown exception\n" << endl;
	}
	return getStatus ();
}

} //NLNET