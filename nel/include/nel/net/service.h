/* service.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.h,v 1.3 2000/10/04 09:38:41 lecroart Exp $
 *
 * Base class for all network services
 */

#ifndef NL_SERVICE_H
#define NL_SERVICE_H

#include <string>
#include <vector>

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

	/// Initialize the service. Allocating buffers, creating connections, etc...
	virtual void	init () {};

	/// Launch the service main loop
	virtual void	run () {};

	/// Release the service. Freeing buffers, closing connections, etc...
	virtual void	release () {};

	sint	getStatus () { return _Status; }

protected:

	/// Array of arguments
	std::vector<std::string> _Args;

	/// Set the status of the service, this status is return to the application. EXIT_SUCCESS is the default status
	/// You can set it to EXIT_FAILURE or any value you want. It's useful when you use the service in a script and you
	/// want to know the return value of the application to do the appropriate things.
	void	setStatus (sint status) { _Status = status; }

private:

	sint	_Status;
};

}; // NLNET

#endif // NL_SERVICE_H

/* End of service.h */
