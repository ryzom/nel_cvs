/** \file service.h
 * Base class for all network services
 *
 * $Id: service.h,v 1.20 2001/05/03 16:29:21 lecroart Exp $
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
#include "nel/misc/config_file.h"

#include "nel/net/callback_net_base.h"

#include <string>
#include <vector>

namespace NLNET
{

typedef uint8 TServiceId;


/**
 * The goal of this macro is to simplify the service creation, it creates the main body function.
 *
 * If you don't want to give a callback array, just put EmptyCallbacArray in the last argument
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
	// "NS" is the short service name and "namgin_service" is the long one.
	// The name of the config file is based on the long name!
	NLNET_SERVICE_MAIN(CFooService, "NS", "naming_service", 50000);
 *\endcode
 *
 * If you want the port to be auto-assigned by the naming service, set the port to 0. 
 */
#define NLNET_SERVICE_MAIN(__ServiceClassName, __ServiceShortName, __ServiceLongName, __ServicePort, __ServiceCallbackArray) \
 \
int main(int argc, char **argv) \
{ \
	__ServiceClassName *scn = new __ServiceClassName; \
	scn->setServiceName (__ServiceShortName, __ServiceLongName); \
	scn->setPort (__ServicePort); \
	if (__ServiceCallbackArray != NULL) \
		scn->setCallbackArray (__ServiceCallbackArray, sizeof(__ServiceCallbackArray)/sizeof(__ServiceCallbackArray[0])); \
	sint retval = scn->main (argc, argv); \
	delete scn; \
	return retval; \
}

static TCallbackItem EmptyCallbackArray[] = { { "", NULL } };


/**
 * Base class for all network services.
 * You must inherite from this class to create your own service. You must not
 * create ctor and dtor but implement init() and release() methods.
 * You have to create a global callback array called CallbackArray.
 *
 * \ref service_howto
 *
 * Temporary command line arguments :
 * \li Arg 1 : port number for listening
 * \li Arg 2 : (optional) timeout for select in millisecond
 * \author Vianney Lecroart
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class IService
{
public:

	/// Constructor
	IService();

	virtual ~IService() {}

	/// Initializes the service (must be called before the first call to update())
	virtual void		init () {}

	/// This function is called every "frame" (you must call init() before). It returns false if the service is stopped.
	virtual bool		update () { return true; }

	/// Finalization. Release the service. For example, this founction free all allocation made in the init() function.
	virtual void		release () {}

	/// Returns the current service name
	static std::string	serviceName () { return IService::_ShortName; };

	/// Returns the service identifier
	NLNET::TServiceId	serviceId() const { return _SId; }

	/// Returns the status
	sint				getStatus () { return _Status; }

	/// Set the status of the service, this status is return to the application. EXIT_SUCCESS is the default status
	/// You can set it to EXIT_FAILURE or any value you want. It's useful when you use the service in a script and you
	/// want to know the return value of the application to do the appropriate things.
	void				setStatus (sint status) { _Status = status; }

	/// User must just have to call this function in his main C function
	sint				main (int argc, char **argv);

	static void			setServiceName (const char *shortName, const char *longName) { _ShortName = shortName; _LongName = longName; }

	static void			setPort (uint16 Port) { _DefaultPort = Port; }

	/** Select timeout value in milliseconds for each update. If you set the value to 1000, the user update
	 * function will be call every second. By default the value is 1
	 */
	static void			setUpdateTimeout (uint32 timeout) { _UpdateTimeout = timeout; } 

	void setCallbackArray (TCallbackItem *callbackArray, uint16 callbackArraySize) { _CallbackArray=callbackArray; _CallbackArraySize=callbackArraySize; }

protected:

	/// Array of arguments
	std::vector<std::string>	_Args;

	/// Port for listening
	uint16						_Port;

	/// @name Static members that must be defined by the deriver class
	//@{

	/// Current service name. Is set by the actual service when declaring NLNET_SERVICE_MAIN
	static std::string			_ShortName;	// ex: "NS"
	static std::string			_LongName;	// ex: "naming_service"

	/// Current service default port. Must be set by the deriver class
	static uint16				_DefaultPort;

	//@}


	/// Process command line arguments for port and timeout
	void getCustomParams();

	/// Sets the service identifier
	void setServiceId (NLNET::TServiceId sid)	{ _SId = sid; }

	/// Singleton
	static IService				*Instance;

	static NLMISC::CConfigFile	_ConfigFile;

	friend void AESConnection (const std::string &serviceName, TSockId from, void *arg);

private:

	/// Select timeout value in milliseconds
	static uint32				_UpdateTimeout;

	TServiceId					_SId;

	sint						_Status;

	TCallbackItem				*_CallbackArray;
	uint16						_CallbackArraySize;
};

}; // NLNET

#endif // NL_SERVICE_H

/* End of service.h */
