/** \file service.h
 * Base class for all network services
 *
 * $Id: service.h,v 1.15 2000/11/22 15:56:47 cado Exp $
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


#ifndef NL_SERVICE_H
#define NL_SERVICE_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>

namespace NLNET
{


class CMsgSocket;


/**
 * The goal of this macro is to simplify the creation of a service, it creates the main body function.
 *
 * Example:
 *\code
	// Create the Foo Service class
	class CFooService : public IService
	{
	public:
		void init () { nlinfo("init()"); }
		bool update () { nlinfo ("update();"); return true; }
		void release () { nlinfo("release()"); }
	};
	// Create the main() function that create a foo service instance and execute it.
	// ("FS" is the short service name)
	NLNET_SERVICE_MAIN(CFooService, "FS", 41200);
 *\endcode
 *
 * If you want the port to be auto-assigned by the naming service, set the port to 0. 
 */
#define NLNET_SERVICE_MAIN(ServiceClassName, ServiceName, ServicePort) \
const char IService::_Name[] = ServiceName; \
const uint16 IService::_DefaultPort = ServicePort; \
sint16 CallbackArraySize = sizeof(CallbackArray)/sizeof(TCallbackItem); \
int main(int argc, char **argv) { \
ServiceClassName *scn = new ServiceClassName; \
sint retval = scn->main (argc, argv); \
delete scn; \
return retval; \
}


/**
 * Base class for all network services.
 * You must inherite from this class to create your own service. You must not
 * create ctor and dtor but implement init() and release() methods.
 * You have to create a global callback array called CallbackArray.
 *
 * \ref new_service_howto
 *
 * Temporary command line arguments :
 * \li Arg 1 : port number for listening
 * \li Arg 2 : (optional) timeout for select in millisecond
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
	virtual bool		update () { return true; }

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

	/// Array of arguments
	std::vector<std::string>	_Args;

	/// Port for listening
	uint16						_Port;

	/// Select timeout value in milliseconds
	uint32						_Timeout;

	/// Server socket
	NLNET::CMsgSocket			*_Server;

	/// Default select timeout value in milliseconds (common to all services)
	static const uint32			_DefaultTimeout;

	/// @name Static members that must be defined by the deriver class
	//@{

	/// Current service name. Is set by the actual service when declaring NLNET_SERVICE_MAIN
	static const char			_Name [];

	/// Current service default port. Must be set by the deriver class
	static const uint16			_DefaultPort;

	//@}


	/// Process command line arguments for port and timeout
	void getCustomParams();

private:

	sint	_Status;
};

}; // NLNET

#endif // NL_SERVICE_H

/* End of service.h */
