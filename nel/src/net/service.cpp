/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.6 2000/10/04 15:09:30 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include <stdlib.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"

#include "nel/net/service.h"

#include <iostream>
using namespace std;

using namespace NLMISC;

namespace NLNET
{

sint IService::main (int argc, char **argv)
{
	try
	{
		for (sint i = 0; i < argc; i++)
		{
			_Args.push_back (argv[i]);
		}

		setStatus (EXIT_SUCCESS);

		init ();
		while (update())
		{
			CConfigFile::checkConfigFiles ();
		}
		release ();
	}
	catch (Exception &e)
	{
		cout << e.what() << endl;
		setStatus (EXIT_FAILURE);
		try
		{
			release ();
		}
		catch (Exception& e)
		{
			cout << "Error releasing service : " << e.what() << endl;
		}
	}
	return getStatus ();
}

} //NLNET