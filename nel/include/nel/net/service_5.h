/** \file service_5.h
 * Base class for all network services
 *
 * $Id: service_5.h,v 1.14 2002/03/20 12:43:59 lecroart Exp $
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


#ifndef NL_SERVICE_5_H
#define NL_SERVICE_5_H

#include "nel/misc/types_nl.h"

#if defined(NL_OS_WINDOWS) && defined(_WINDOWS)
#	include <windows.h>
#	undef min
#	undef max
#endif

#include "nel/misc/config_file.h"
#include "nel/misc/entity_id.h"

#include "nel/net/unified_network.h"

#include <string>
#include <vector>

namespace NLMISC
{
	class CWindowDisplayer;
}

namespace NLNET
{

class CCallbackServer;

typedef uint8 TServiceId;


/**
 * The goal of this macro is to simplify the service creation, it creates the main body function.
 *
 * If you don't want to give a callback array, just put EmptyCallbackArray in the last argument
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
	NLNET_SERVICE_MAIN(CFooService, "NS", "naming_service", 50000, EmptyCallbackArray);
 *\endcode
 *
 * If you want the port to be auto-assigned by the naming service, set the port to 0. 
 */

#if defined(NL_OS_WINDOWS) && defined(_WINDOWS)

#define NLNET_SERVICE_MAIN(__ServiceClassName, __ServiceShortName, __ServiceLongName, __ServicePort, __ServiceCallbackArray, __ConfigDir, __LogDir) \
 \
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) \
{ \
	IService5::setConfigDir (__ConfigDir); \
	IService5::setLogDir (__LogDir); \
	__ServiceClassName *scn = new __ServiceClassName; \
	scn->setServiceName (__ServiceShortName, __ServiceLongName); \
	scn->setPort (__ServicePort); \
	(NLNET::CUnifiedNetwork::getInstance())->addCallbackArray(__ServiceCallbackArray, sizeof(__ServiceCallbackArray)/sizeof(__ServiceCallbackArray[0])); \
    sint retval = scn->main (lpCmdLine); \
	delete scn; \
	return retval; \
}

#else

#define NLNET_SERVICE_MAIN(__ServiceClassName, __ServiceShortName, __ServiceLongName, __ServicePort, __ServiceCallbackArray, __ConfigDir, __LogDir) \
 \
int main(int argc, char **argv) \
{ \
	IService5::setConfigDir (__ConfigDir); \
	IService5::setLogDir (__LogDir); \
	__ServiceClassName *scn = new __ServiceClassName; \
	scn->setServiceName (__ServiceShortName, __ServiceLongName); \
	scn->setPort (__ServicePort); \
	(NLNET::CUnifiedNetwork::getInstance())->addCallbackArray(__ServiceCallbackArray, sizeof(__ServiceCallbackArray)/sizeof(__ServiceCallbackArray[0])); \
	sint retval = scn->main (argc, argv); \
	delete scn; \
	return retval; \
}

#endif

static TUnifiedCallbackItem EmptyCallbackArray[] = { { "", NULL } };


/**
 * Base class for all network services.
 * You must inherite from this class to create your own service. You must not
 * create ctor and dtor but implement init() and release() methods.
 * You have to create a global callback array called CallbackArray.
 *
 * \ref service_howto
 *
 * Temporary command line arguments :
 * \li -n<alias_name>
 * \author Vianney Lecroart
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class IService5
{
public:

	/// Constructor
	IService5();

	virtual ~IService5() {}

	/// Initializes the service (must be called before the first call to update())
	virtual void		init () {}

	/// This function is called every "frame" (you must call init() before). It returns false if the service is stopped.
	virtual bool		update () { return true; }

	/// Finalization. Release the service. For example, this founction free all allocation made in the init() function.
	virtual void		release () {}

	/// Returns the current service name
	static std::string	serviceName () { return IService5::_ShortName; };

	/// Returns the service identifier
	NLNET::TServiceId	serviceId() const { return _SId; }

	/// Returns the status
	sint				getStatus () { return _Status; }

	/// Set the status of the service, this status is return to the application. EXIT_SUCCESS is the default status
	/// You can set it to EXIT_FAILURE or any value you want. It's useful when you use the service in a script and you
	/// want to know the return value of the application to do the appropriate things.
	void				setStatus (sint status) { _Status = status; }

	sint				main (int argc, char **argv);
	sint				main (char *args);

	static void					 setServiceName (const char *shortName, const char *longName);

	static void					 setConfigDir (const char *configDir) { _ConfigDir = configDir; }
	static const std::string	&getConfigDir () { return _ConfigDir; }

	static void					 setLogDir (const char *logDir) { _LogDir = logDir; }
	static const std::string	&getLogDir () { return _LogDir; }

	static void					 setPort (uint16 Port) { _DefaultPort = Port; }

	/** Select timeout value in seconds for each update. You are absolutely certain that your update()
	 * function will not be called before this amount of time you set.
	 * If you set the update timeout value higher than 0, all messages in queues will be process until the time greater than the timeout user update().
	 * If you set the update timeout value to 0, all messages in queues will be process one time before calling the user update().
	 *
	 * The default value is 100ms
	 */
	static void			setUpdateTimeout (NLMISC::TTime  timeout) { /*if (timeout>1.0) nlerror ("IServer5::setUpdateTimeout is now a double in SECOND and not ms");*/ _UpdateTimeout = timeout; } 

	/// Changes the recording state (use if you know what you are doing)
	void				setRecordingState( CCallbackNetBase::TRecordingState rec ) { _RecordingState = rec; }

	/// Returns an uniq id for an entities on this service.
	static uint64 getEntityId (uint8 Type)
	{
		NLMISC::CEntityId id = _NextEntityId++;
		id.Type = Type;
		return id.getRawId ();
	}

	/// Returns the recording state
	static CCallbackNetBase::TRecordingState recordingState() { return Instance->_RecordingState; }

	static NLMISC::CConfigFile	ConfigFile;

	/// Singleton
	static IService5				*Instance;

protected:

	/// Array of arguments
	std::vector<std::string>	_Args;

	/// Port for listening
	uint16						_Port;

	/// Recording state
	CCallbackNetBase::TRecordingState _RecordingState;

	/// @name Static members that must be defined by the deriver class
	//@{

	/// Current service name. Is set by the actual service when declaring NLNET_SERVICE_MAIN
	static std::string			_ShortName;	// ex: "NS"
	static std::string			_LongName;	// ex: "naming_service"
	static std::string			_AliasName;	// this name is initialized by the admin executor service via the arg list

	/// Current service default port. Must be set by the deriver class
	static uint16				_DefaultPort;

	//@}


	/// Process command line arguments for port and timeout
	void getCustomParams();

	/// Sets the service identifier
	void setServiceId (NLNET::TServiceId sid)	{ _SId = sid; }

	NLMISC::CWindowDisplayer	*_WindowDisplayer;

	friend void AESConnection (const std::string &serviceName, uint16 sid, void *arg);

private:

	// this main is called by other main, the command line must be processing before calling this function
	sint				main ();

	/// Select timeout value in milliseconds
	static NLMISC::TTime		_UpdateTimeout;

	TServiceId					_SId;

	sint						_Status;

	bool						_Initialized;	// true if the init part of the service is passed

	/// This variable is used to generate uniq id for entities on this service.
	static NLMISC::CEntityId	_NextEntityId;

	static std::string			_ConfigDir;
	static std::string			_LogDir;
};

}; // NLNET

#endif // NL_SERVICE_5_H

/* End of service_5.h */
