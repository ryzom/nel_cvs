/* service.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.h,v 1.1 2000/09/14 16:18:47 lecroart Exp $
 *
 * Base class for all network services
 */

#ifndef NL_SERVICE_H
#define NL_SERVICE_H

#include <vector>

using namespace std;

#include "nel/misc/types_nl.h"

namespace NLNET
{

/**
 * Base class for all network services
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IService
{
public:

	/// Constructor. Use argc/argv old style format to generate _Args, vector of string new format easy to use
	IService(int argc, char **argv);

protected:

	/// Array of arguments
	vector<string> _Args;
};

}; // NLNET

#endif // NL_SERVICE_H

/* End of service.h */
