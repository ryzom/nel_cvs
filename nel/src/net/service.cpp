/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.4 2000/10/04 13:58:22 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include <stdlib.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"

#include "nel/net/service.h"

using namespace NLMISC;

namespace NLNET
{

sint IService::main (int argc, char **argv)
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
	return getStatus ();
}

} //NLNET