/** \file service.h
 * Base class for all network services
 *
 * $Id: service.h,v 1.64 2003/07/22 16:25:58 coutelas Exp $
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

//
// Includes
//

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



//
// Macros
//


/**
 * The goal of this macro is to simplify the service creation, it creates the main body function.
 *
 * If you don't want to give a callback array, just put EmptyCallbackArray in the last argument
 *
 * Example:
 *\code
	// Create the Test Service class
	class CTestService : public IService
	{
	public:
		void init () { nlinfo("init()"); }
		bool update () { nlinfo ("update();"); return true; }
		void release () { nlinfo("release()"); }
	};
	// Create the main() function that create a test service instance and execute it.
	// "TS" is the short service name and "test_service" is the long one.
	// The name of the config file is based on the long name!
	// EmptyCallbackArray means that you don't provide right now the callback
	// the last 2 path are where is the config file is (directory) and where to log info (directory)
	NLNET_SERVICE_MAIN(CTestService, "TS", "test_service", 0, EmptyCallbackArray, "", "");
 *\endcode
 *
 * If you want the port to not be auto-assigned by the naming service, set the port to a number different than 0.
 *
 * Args used by service are always in lower case:
 *
 * -A followed by the path where to execute the service (it uses chdir())
 * -C followed by the directory where we can find the config file
 * -D followed by the listend address for the login system
 * -I to start the service iconified
 * -L followed by the directory where we have to log
 * -N followed by the alias name (used by the admin system)
 * -P followed by the listen port (ListenAddress)
 * -S followed by the shard Id (sint32) for WS only
 * -W followed by the directory where to save all shard data (SaveFilesDirectory)
 * -Q to make the service quit immediately after the first update
 *
 */


#if defined(NL_OS_WINDOWS) && defined(_WINDOWS)
#define NLNET_SERVICE_MAIN(__ServiceClassName, __ServiceShortName, __ServiceLongName, __ServicePort, __ServiceCallbackArray, __ConfigDir, __LogDir) \
 \
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) \
{ \
	__ServiceClassName *scn = new __ServiceClassName; \
	scn->setArgs (lpCmdLine); \
	scn->setCallbackArray (__ServiceCallbackArray, sizeof(__ServiceCallbackArray)/sizeof(__ServiceCallbackArray[0])); \
    sint retval = scn->main (__ServiceShortName, __ServiceLongName, __ServicePort, __ConfigDir, __LogDir, __DATE__" "__TIME__); \
	delete scn; \
	return retval; \
}

#else
#define NLNET_SERVICE_MAIN(__ServiceClassName, __ServiceShortName, __ServiceLongName, __ServicePort, __ServiceCallbackArray, __ConfigDir, __LogDir) \
 \
int main(int argc, const char **argv) \
{ \
	__ServiceClassName *scn = new __ServiceClassName; \
	scn->setArgs (argc, argv); \
	scn->setCallbackArray (__ServiceCallbackArray, sizeof(__ServiceCallbackArray)/sizeof(__ServiceCallbackArray[0])); \
	sint retval = scn->main (__ServiceShortName, __ServiceLongName, __ServicePort, __ConfigDir, __LogDir, __DATE__" "__TIME__); \
	delete scn; \
	return retval; \
}

#endif


//
// Typedefs
//

typedef uint8 TServiceId;


//
// Variables provided to application and unused in the NeL library itself.
//

static TUnifiedCallbackItem EmptyCallbackArray[] = { { "", NULL } };

extern sint32 NetSpeedLoop, UserSpeedLoop;

//
// Classes
//

/**
 * Base class for all network services.
 * You must inherite from this class to create your own service. You must not
 * create ctor and dtor but implement init() and release() methods.
 * You have to create a global callback array called CallbackArray.
 *
 * \ref service_howto
 *
 * Temporary command line arguments :
 * \li -n<AliasName>
 *
 * \author Vianney Lecroart
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class IService
{
public:

	/// \name User overload methods. These methods can be overload by the user do handle init, update and release operation.
	// @{

	/// Initializes the service (must be called before the first call to update())
	virtual void			init () {}

	/// This function is called every "frame" (you must call init() before). It returns false if the service is stopped.
	virtual bool			update () { return true; }

	/// Finalization. Release the service. For example, this founction free all allocation made in the init() function.
	virtual void			release () {}

	//@}


	/// \name get methods. These methods provide a way to read internal service variables.
	// @{

	/// Returns the instance of the service to access to methods/variables class
	static IService					*IService::getInstance () { nlassert (_Instance != NULL); return _Instance; }

	/// Returns the current service short name (ie: TS)
	const std::string				&getServiceShortName () const { return _ShortName; };

	/// Returns the current service long name (ie: test_serivce)
	const std::string				&getServiceLongName () const { return _LongName; };

	/// Returns the current service alias name setted by AES
	const std::string				&getServiceAliasName () const { return _AliasName; };

	/// Returns the current service unified name that is   alias/short-id or short-id if alias is empty
	std::string						getServiceUnifiedName () const;

	/// Returns the service identifier
	TServiceId						getServiceId () const { return _SId; }

	/// Returns the status
	sint							getStatus () const { return _Status; }

	/// Returns arguments of the program pass from the user to the program using parameters (ie: "myprog param1 param2")
	const std::vector<std::string>	&getArgs () const { return _Args; }

	/// Returns true if the argument if present in the command line (ie: haveArg('p') will return true if -p is in the command line)
	bool							haveArg (char argName);

	/** Returns the parameter linked to an option (ie: getArg('p') will return "toto" if -ptoto is in the command line)
	 * It'll thrown an Exception if the argName is not found
	 */
	std::string						getArg (char argName);

	/// Returns an uniq id for an entities on this service.
	uint64							getEntityId (uint8 Type)
	{
		NLMISC::CEntityId id = _NextEntityId++;
		id.setType( Type );
		return id.getRawId ();
	}
	
	/// Returns a pointer to the CCallbackServer object
	CCallbackServer					*getServer();

	/// Returns the recording state (don't needed if you use layer5)
	CCallbackNetBase::TRecordingState	getRecordingState() const { return _RecordingState; }


	//@}


	/// \name set methods. These methods provide a way to modify internal service variables.
	// @{

	/** Sets the status of the service, this status is return to the application. EXIT_SUCCESS is the default status
	 * You can set it to EXIT_FAILURE or any value you want. It's useful when you use the service in a script and you
	 * want to know the return value of the application to do the appropriate things.
	 */
	void					setStatus (sint status) { _Status = status; }

	/** Call this function if you want the service quits next loop. The code will be returned outside of the application.
	 * \warning If you set the code to 0, it ll NOT exit the service */
	void					exit (sint code = 0x10);

	/** Selects timeout value in seconds for each update. You are absolutely certain that your update()
	 * function will not be called before this amount of time you set.
	 * If you set the update timeout value higher than 0, all messages in queues will be process until the time greater than the timeout user update().
	 * If you set the update timeout value to 0, all messages in queues will be process one time before calling the user update().
	 *
	 * The default value is 100 (100ms)
	 */
	void					setUpdateTimeout (NLMISC::TTime timeout) { /*if (timeout>1.0) nlerror ("IServer::setUpdateTimeout is now a double in SECOND and not ms");*/ _UpdateTimeout = timeout; } 


	/// \name variables. These variables can be read/modified by the user.
	// @{

	NLMISC::CConfigFile	ConfigFile;

	//@}


	/// \name private methods. These methods are used by internal system but can't be put in private, don't use them.
	// @{

	/// This main is called by the macro (service5 says if we have to use layer5 or not)
	sint main (const char *serviceShortName, const char *serviceLongName, uint16 servicePort, const char *configDir, const char *logDir, const char *compilationDate);

	/// Sets the command line and init _Args variable. You must call this before calling main()
	void setArgs (int argc, const char **argv);

	/// Sets the command line and init _Args variable. You must call this before calling main()
	void setArgs (const char *args);

	/// Sets the default callback array given from the macro
	void setCallbackArray (TUnifiedCallbackItem *array, uint nbelem) { _CallbackArray = array; _CallbackArraySize = nbelem; }

	/// Require to reset the hierarchical timer
	void requireResetMeasures();

	/// Ctor. You must not inherit ctor but overload init() function
	IService ();

	/// Dtor. You must not inherit dtor but overload release() function
	virtual ~IService () {}

	//@}

	/// The window displayer instance
	NLMISC::CWindowDisplayer			*WindowDisplayer;

	/// Directory where to store files that the services will write but are the same for all shard instance (for example: packet_sheets)
	std::string							WriteFilesDirectory;

	/// Directory where to store files that the services will write during the exploitation of the game (for example: player backup, string cache)
	std::string							SaveFilesDirectory;
	
	void								setVersion (const std::string &version) { _Version = version; }

	uint32								getPort() { return _Port; }

private:

	/// \name methods. These methods are used by internal system.
	// @{

	/// Changes the recording state (use if you know what you are doing)
	void								setRecordingState( CCallbackNetBase::TRecordingState rec ) { _RecordingState = rec; }

	//@}


	/// \name variables. These variables are used by the internal system.
	// @{

	/// Array of arguments pass from the command line
	std::vector<std::string>			_Args;

	/// Listening port of this service
	uint16								_Port;

	/// Recording state
	CCallbackNetBase::TRecordingState	_RecordingState;

	/// Current service name sets by the actual service when declaring NLNET_SERVICE_MAIN
	std::string							_ShortName;		// ie: "NS"
	std::string							_LongName;		// ie: "naming_service"
	std::string							_AliasName;		// this name is initialized by the admin executor service via the args

	/// Instance of this service (singleton)
	static IService						*_Instance;

	/// Select timeout value in milliseconds between to call of user update()
	NLMISC::TTime						_UpdateTimeout;

	/// the service id of this sevice
	TServiceId							_SId;

	/// the status of this service (the status is give to the at the release time)
	sint								_Status;

	/// true if the service initialisation is passed
	bool								_Initialized;

	/// This variable is used to generate uniq id for entities on this service.
	NLMISC::CEntityId					_NextEntityId;

	/// The directory where the configfile is
	std::string							_ConfigDir;

	/// The directory where the logfiles are
	std::string							_LogDir;

	/// The directory where the service is running
	std::string							_RunningPath;

	std::string							_Version;

	TUnifiedCallbackItem				*_CallbackArray;
	uint								_CallbackArraySize;

	/// true if the service don't use the naming service
	bool								_DontUseNS;
	/// true if the service don't use the admin executor service
	bool								_DontUseAES;

	/// Require to reset the hierarchical timer
	bool								_ResetMeasures;

	//@}

	friend void serviceGetView (uint32 rid, const std::string &rawvarpath, std::vector<std::string> &vara, std::vector<std::string> &vala);
	friend void AESConnection (const std::string &serviceName, uint16 sid, void *arg);
	friend struct serviceInfoClass;
	friend struct getWinDisplayerInfoClass;
	friend class RunningDirectoryClass;
	friend class LogDirectoryClass;
	friend class ConfigDirectoryClass;
	friend class VersionClass;
};


}; // NLNET

#endif // NL_SERVICE_H

/* End of service.h */
