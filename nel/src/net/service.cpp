/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.2 2000/09/14 16:40:53 cado Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/types_nl.h"

#include "nel/net/service.h"

namespace NLNET
{

/*
 * Constructor
 */
IService::IService(int argc, char **argv)
{
	for (sint i = 0; i < argc; i++)
	{
		_Args.push_back (argv[i]);
	}
}

} //NLNET