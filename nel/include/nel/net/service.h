/* service.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.h,v 1.7 2000/10/06 10:27:36 lecroart Exp $
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
 * Base class for all network services.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IService
{
public:

	/// Initializes the service (must be called before the first call to update())
	virtual void		init () {}

	/// This function is called every "frame" (you must call init() before). It returns false if the service is stopped.
	virtual bool		update () { return false; }

	/// Finalization. Release the service. For example, this founction free all allocation made in the init() function.
	virtual void		release () {}

	/// Returns the current service name
	static std::string	serviceName ()
	{
		return IService::_Name;
	};

	/// Returns the status
	sint				getStatus () { return _Status; }

	/// Set the status of the service, this status is return to the application. EXIT_SUCCESS is the default status
	/// You can set it to EXIT_FAILURE or any value you want. It's useful when you use the service in a script and you
	/// want to know the return value of the application to do the appropriate things.
	void				setStatus (sint status) { _Status = status; }

	/// User must just have to call this function in his main C function
	sint				main (int argc, char **argv);

protected:

	/// Current service name. Must be set by the deriver class
	static const  char			_Name [];

	/// Array of arguments
	std::vector<std::string>	_Args;

private:

	sint	_Status;
};

}; // NLNET

#endif // NL_SERVICE_H

/* End of service.h */
