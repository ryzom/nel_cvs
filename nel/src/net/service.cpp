/** \file service.cpp
 * Base class for all network services
 *
 * $Id: service.cpp,v 1.169 2003/02/21 15:52:35 lecroart Exp $
 *
 * \todo ace: test the signal redirection on Unix
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "stdnet.h"

//
// Includes
//

#ifdef NL_OS_WINDOWS
// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#	define _WIN32_WINDOWS	0x0410
#	define WINVER			0x0400
#	include <windows.h>
#	include <direct.h>
#elif defined NL_OS_UNIX
#	include <unistd.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/mutex.h"
#include "nel/misc/window_displayer.h"
#include "nel/misc/gtk_displayer.h"
#include "nel/misc/win_displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/report.h"
#include "nel/misc/system_info.h"

#include "nel/net/naming_client.h"
#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/net_manager.h"
#include "nel/net/net_displayer.h"
#include "nel/net/email.h"
#include "nel/net/varpath.h"
#include "nel/net/alarms.h"

#include "nel/memory/memory_manager.h"

#include "nel/misc/hierarchical_timer.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;

namespace NLNET
{


//
// Constants
//

static const sint Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

static const char *SignalName[]=
{
  "SIGABRT", "SIGFPE", "SIGILL", "SIGINT", "SIGSEGV", "SIGTERM"
};

static const char* NegFiltersNames[] =
{
   "NegFiltersDebug",
   "NegFiltersInfo",
   "NegFiltersWarning",
   "NegFiltersAssert",
   "NegFiltersError",
   0
};
	

//
// Variables
//


// class static member
IService	*IService::_Instance = NULL;

static sint ExitSignalAsked = 0;

// services stat
sint32 NetSpeedLoop, UserSpeedLoop;

// this is the thread that initialized the signal redirection
// we ll ignore other thread signals
static uint SignalisedThread;

static CFileDisplayer fd;
static CNetDisplayer commandDisplayer(false);
//static CLog commandLog;

static string CompilationDate;
static uint32 LaunchingDate;

static uint32 NbUserUpdate = 0;

#ifdef NL_RELEASE_DEBUG
string CompilationMode = "NL_RELEASE_DEBUG";
#elif defined(NL_DEBUG_FAST)
string CompilationMode = "NL_DEBUG_FAST";
#elif defined(NL_DEBUG)
string CompilationMode = "NL_DEBUG";
#elif defined(NL_RELEASE)
string CompilationMode = "NL_RELEASE";
#else
string CompilationMode = "???";
#endif

// use to display result of command (on file and windows displayer) **without** filter
static CLog CommandLog;

static bool Bench = false;

//
// Callback managing
//

bool isCommand (const string &str)
{
	if (str.empty())
		return false;
	
	return isupper(str[0]) == 0;
}

// this callback is used to create a view for the admin system
void serviceGetView (uint32 rid, const string &rawvarpath, vector<pair<vector<string>, vector<string> > > &answer)
{
	string str;
	CLog logDisplayVars;
	CLightMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);
	mdDisplayVars.setParam (1024);

	CVarPath varpath(rawvarpath);

	if (varpath.empty())
		return;

	if (varpath.isFinal())
	{
		vector<string> vara, vala;

		// add default row
		vara.push_back ("service");
		vala.push_back (IService::getInstance ()->getServiceUnifiedName());
		
		for (uint j = 0; j < varpath.Destination.size (); j++)
		{
			string cmd = varpath.Destination[j].first;

			// replace = with space to execute the command
			uint eqpos = cmd.find("=");
			if (eqpos != string::npos)
			{
				cmd[eqpos] = ' ';
				vara.push_back(cmd.substr(0, eqpos));
			}
			else
				vara.push_back(cmd);
			
			mdDisplayVars.clear ();
			ICommand::execute(cmd, logDisplayVars, !isCommand(cmd));
			const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

			if (isCommand(cmd))
			{
				// we want the log of the command
				if (j == 0)
				{
					vara.clear ();
					vara.push_back ("__log");
					vala.clear ();
				}
				
				vala.push_back ("----- Result from command: "+cmd+"\n");
				for (uint k = 0; k < strs.size(); k++)
				{
					vala.push_back (strs[k]);
				}
			}
			else
			{
				if (strs.size()>0)
				{
					uint32 pos = strs[0].find("=");
					if(pos != string::npos && pos + 2 < strs[0].size())
					{
						uint32 pos2 = string::npos;
						if(strs[0][strs[0].size()-1] == '\n')
							pos2 = strs[0].size() - pos - 2 - 1;
						
						str = strs[0].substr (pos+2, pos2);
						
						// replace all spaces into udnerscore because space is a reserved char
						for (uint i = 0; i < str.size(); i++) if (str[i] == ' ') str[i] = '_';
					}
					else
					{
						str = "???";
					}
				}
				else
				{
					str = "???";
				}
				vala.push_back (str);
				nlinfo ("Add to result view '%s' = '%s'", varpath.Destination[j].first.c_str(), str.c_str());
			}
			mdDisplayVars.unlockStrings();
		}

		answer.push_back (make_pair(vara, vala));
	}
	else
	{
		// there s an entity in the varpath, manage this case

		vector<string> *vara, *vala;
		
		// varpath.Destination		contains the entity number
		// subvarpath.Destination	contains the command name
		
		for (uint i = 0; i < varpath.Destination.size (); i++)
		{
			CVarPath subvarpath(varpath.Destination[i].second);
			
			for (uint j = 0; j < subvarpath.Destination.size (); j++)
			{
				// set the variable name
				string cmd = subvarpath.Destination[j].first;

				/*if (isCommand(cmd))
				{
					// it's a command with parameter, do it
					mdDisplayVars.clear ();
					cmd += " " + subvarpath.Destination[j].second;
					ICommand::execute(cmd, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					
					answer.push_back (make_pair(vector<string>(), vector<string>()));
					
					vara = &(answer[answer.size()-1].first);
					vala = &(answer[answer.size()-1].second);

					// we want the log of the command
					vara->clear ();
					vara->push_back ("__log");
					
					vala->push_back ("----- Result from command: "+cmd);
					for (uint k = 0; k < strs.size(); k++)
					{
						vala->push_back (strs[k]);
					}
				}
				else*/
				{
					// replace = with space to execute the command
					uint eqpos = cmd.find("=");
					if (eqpos != string::npos)
					{
						cmd[eqpos] = ' ';
						// add the entity
						cmd.insert(eqpos, " "+varpath.Destination[i].first);
					}
					else
					{
						// add the entity
						cmd += " "+varpath.Destination[i].first;
					}
					
					mdDisplayVars.clear ();
					ICommand::execute(cmd, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					for (uint k = 0; k < strs.size(); k++)
					{
						uint32 pos, pos2;

						const string &str = strs[k];

						string entity = "???";
						pos = str.find("Entity ");
						if(pos != string::npos)
						{
							pos2 = str.find(" ", pos+7);
							if(pos2 != string::npos)
								entity = str.substr(pos+7, pos2-pos-7);
						}

						// look in the array if we already have something about this entity

						uint y;
						for (y = 0; y < answer.size(); y++)
						{
							if (answer[y].second[1] == entity)
							{
								// ok we found it, just push_back new stuff
								vara = &(answer[y].first);
								vala = &(answer[y].second);
								break;
							}
						}
						if (y == answer.size ())
						{
							answer.push_back (make_pair(vector<string>(), vector<string>()));

							vara = &(answer[answer.size()-1].first);
							vala = &(answer[answer.size()-1].second);
							
							// don't add service if we want an entity
	// todo when we work on entity, we don't need service name and server so we should remove them and collapse all var for the same entity
							vara->push_back ("service");
							string name = IService::getInstance ()->getServiceUnifiedName();
							vala->push_back (name);
							
							// add default row
							vara->push_back ("entity");
							vala->push_back (entity);
						}

						vara->push_back(cmd.substr(0, cmd.find(" ")));

						pos = str.find("=");
						if(pos != string::npos && pos + 2 < str.size())
						{
							uint32 pos2 = string::npos;
							if(str[str.size()-1] == '\n')
								pos2 = str.size() - pos - 2 - 1;
							
							vala->push_back (strs[k].substr (pos+2, pos2));
						}
						else
						{
							vala->push_back ("???");
						}
						
						nlinfo ("Add to result view for entity '%s', '%s' = '%s'", varpath.Destination[i].first.c_str(), subvarpath.Destination[j].first.c_str(), str.c_str());
					}
					mdDisplayVars.unlockStrings();
				}
			}
		}
	}
}

void servcbGetView (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	uint32 rid;
	string rawvarpath;

	msgin.serial (rid);
	msgin.serial (rawvarpath);

	vector<pair<vector<string>, vector<string> > > answer;

	serviceGetView (rid, rawvarpath, answer);

	CMessage msgout("VIEW");
	msgout.serial(rid);
	
	for (uint i = 0; i < answer.size(); i++)
	{
		msgout.serialCont (answer[i].first);
		msgout.serialCont (answer[i].second);
	}
	
	CUnifiedNetwork::getInstance ()->send (sid, msgout);
	nlinfo ("Sent result view to service '%s-%hu'", serviceName.c_str(), sid);
}

void AESConnection (const string &serviceName, uint16 sid, void *arg)
{
	// established a connection to the AES, identify myself

	//
	// Sends the identification message with the name of the service and all commands available on this service
	//

	CMessage msgout ("SID");
	uint32 pid = getpid ();
	msgout.serial (IService::getInstance()->_AliasName, IService::getInstance()->_LongName, pid);
	ICommand::serialCommands (msgout);
	CUnifiedNetwork::getInstance()->send("AES", msgout);

	if (IService::getInstance()->_Initialized)
	{
		CMessage msgout2 ("SR");
		CUnifiedNetwork::getInstance()->send("AES", msgout2);
	}
}


static void AESDisconnection (const std::string &serviceName, uint16 sid, void *arg)
{
}


static void cbExecCommand (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	string command;
	msgin.serial (command);

	nlinfo ("Executing command from network : '%s'", command.c_str());
	ICommand::execute (command, CommandLog);
}


static void cbStopService (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	nlinfo ("Receive a stop from service %s-%d, need to quit", serviceName.c_str(), sid);
	ExitSignalAsked = 0xFFFF;
}



// layer 5
static TUnifiedCallbackItem AESCallbackArray[] =
{
	{ "STOPS", cbStopService },
	{ "EXEC_COMMAND", cbExecCommand },
	{ "GET_VIEW", servcbGetView },
};

//
// Signals managing
//

// This function is called when a signal comes
static void sigHandler(int Sig)
{
	// redirect the signal for the next time
	signal(Sig, sigHandler);

	// find the signal
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		if (Sig == Signal[i])
		{
			if (getThreadId () != SignalisedThread)
			{
				nldebug ("Not the main thread received the signal (%s, %d), ignore it", SignalName[i],Sig);
				return;
			}
			else
			{
				nlinfo ("Signal %s (%d) received", SignalName[i], Sig);
				switch (Sig)
				{
				case SIGABRT :
				case SIGILL  :
				case SIGINT  :
				case SIGSEGV :
				case SIGTERM :
				// you should not call a function and system function like printf in a SigHandle because
				// signal-handler routines are usually called asynchronously when an interrupt occurs.
				if (ExitSignalAsked == 0)
				{
					nlinfo ("Receive a signal that said that i must exit");
					ExitSignalAsked = Sig;
					return;
				}
				else
				{
					nlinfo ("Signal already received, launch the brutal exit");
					exit (EXIT_FAILURE);
				}
				break;
				}
			}
		}
	}
	nlinfo ("Unknown signal received (%d)", Sig);
}

// Initialise the signal redirection
static void initSignal()
{
	SignalisedThread = getThreadId ();
#ifdef NL_DEBUG
	// in debug mode, we only trap the SIGINT signal
	signal(Signal[3], sigHandler);
	//nldebug("Signal : %s (%d) trapped", SignalName[3], Signal[3]);
#else
	// in release, redirect all signals
/* don't redirect now because to hard to debug...
	for (int i = 0; i < (int)(sizeof(Signal)/sizeof(Signal[0])); i++)
	{
		signal(Signal[i], sigHandler);
		nldebug("Signal %s (%d) trapped", SignalName[i], Signal[i]);
	}
*/
#endif
}

//
// Class implementation
//

// Ctor
IService::IService() :
	WindowDisplayer(NULL),
	_Port(0),
	_RecordingState(CCallbackNetBase::Off),
	_UpdateTimeout(100),
	_SId(0),
	_Status(0),
	_Initialized(false),
	_ResetMeasures(false)
{
	// Singleton
	nlassert( _Instance == NULL );

	_Instance = this;
}



bool IService::haveArg (char argName)
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if (_Args[i][1] == argName)
			{
				return true;
			}
		}
	}
	return false;
}

string IService::getArg (char argName)
{
	for (uint32 i = 0; i < _Args.size(); i++)
	{
		if (_Args[i].size() >= 2 && _Args[i][0] == '-')
		{
			if (_Args[i][1] == argName)
			{
				return _Args[i].substr(2);
			}
		}
	}
	throw Exception ("Parameter '-%c' is not found in command line", argName);
}


void IService::setArgs (const char *args)
{
	_Args.push_back ("<ProgramName>");

	string sargs (args);
	uint32 pos1 = 0, pos2 = 0;

	do
	{
		pos1 = sargs.find_first_not_of (" ", pos2);
		if (pos1 == string::npos) break;
		pos2 = sargs.find_first_of (" ", pos1);
		_Args.push_back (sargs.substr (pos1, pos2-pos1));
	}
	while (pos2 != string::npos);
}

void IService::setArgs (int argc, const char **argv)
{
	for (sint i = 0; i < argc; i++)
	{
		_Args.push_back (argv[i]);
	}
}

/*
 * Returns a pointer to the CCallbackServer object
 */
CCallbackServer *IService::getServer()
{
	return NULL;
}



void cbLogFilter (CConfigFile::CVar &var)
{
	CLog *log = NULL;
	if (var.Name == "NegFiltersDebug")
	{
		log = DebugLog;
	}
	else if (var.Name == "NegFiltersInfo")
	{
		log = InfoLog;
	}
	else if (var.Name == "NegFiltersWarning")
	{
		log = WarningLog;
	}
	else if (var.Name == "NegFiltersAssert")
	{
		log = AssertLog;
	}
	else if (var.Name == "NegFiltersError")
	{
		log = ErrorLog;
	}
	else
	{
		nlstop;
	}

	nlinfo ("Updating %s from config file", var.Name.c_str());
	
	// remove all old filters from config file
	CConfigFile::CVar &oldvar = IService::getInstance()->ConfigFile.getVar (var.Name);
	for (sint j = 0; j < oldvar.size(); j++)
	{
		log->removeFilter (oldvar.asString(j).c_str());
	}

	// add all new filters from config file
	for (sint i = 0; i < var.size(); i++)
	{
		log->addNegativeFilter (var.asString(i).c_str());
	}
}








//
// The main function of the service
//

sint IService::main (const char *serviceShortName, const char *serviceLongName, uint16 servicePort, const char *configDir, const char *logDir, const char *compilationDate)
{
	bool userInitCalled = false;
	bool resyncEvenly = false;
	CConfigFile::CVar *var = NULL;
	
	// a short name service can't be a number
	nlassert (atoi(serviceShortName) == 0);

	try
	{
		//
		// Init parameters
		//
		
		_ConfigDir = CPath::standardizePath(configDir);
		_LogDir = CPath::standardizePath(logDir);
		_ShortName = serviceShortName;
		_LongName = serviceLongName;

		CompilationDate = compilationDate;

		LaunchingDate = CTime::getSecondsSince1970();

		// Set the process name
		CLog::setProcessName (_ShortName);

		setReportEmailFunction ((void*)sendEmail);
		setDefaultEmailParams ("gw.nevrax.com", "", "lecroart@nevrax.com");

		// get the path where to run the service if any in the command line
		if (haveArg('A'))
		{
			_RunningPath = CPath::standardizePath(getArg('A'));
#ifdef NL_OS_WINDOWS
			_chdir (_RunningPath.c_str());
#else
			chdir (_RunningPath.c_str());
#endif
		}

		//
		// Init debug/log stuffs (must be first things otherwise we can't log if errors)
		//

		// get the log dir if any in the command line
		if (haveArg('L'))
			_LogDir = CPath::standardizePath(getArg('L'));

		// get the config file dir if any in the command line
		if (haveArg('C'))
			_ConfigDir = CPath::standardizePath(getArg('C'));

		createDebug (_LogDir.c_str(), false);

		//DebugLog->addNegativeFilter ("NETL");

		// we create the log with service name filename ("test_service.log" for example)
		fd.setParam (_LogDir + _LongName + ".log", false);

		DebugLog->addDisplayer (&fd);
		InfoLog->addDisplayer (&fd);
		WarningLog->addDisplayer (&fd);
		AssertLog->addDisplayer (&fd);
		ErrorLog->addDisplayer (&fd);
		CommandLog.addDisplayer (&fd, true);

		//
		// Init the hierarchical timer
		//

		CHTimer::startBench(false, true);
		CHTimer::endBench();
		

		//
		// Load the config file
		//

		ConfigFile.load (_ConfigDir + _LongName + ".cfg");


		//
		// Set the negative filter from the config file
		//

		for(const char **name = NegFiltersNames; *name; name++)
		{
			if ((var = ConfigFile.getVarPtr (*name)) != NULL)
			{
				ConfigFile.setCallback (*name, cbLogFilter);
				cbLogFilter(*var);
			}
		}
		
		
		//
		// Create the window if needed
		//

		if ((var = ConfigFile.getVarPtr ("WindowStyle")) != NULL)
		{
			string disp = var->asString ();
#ifdef NL_USE_GTK
			if (disp == "GTK")
			{
				WindowDisplayer = new CGtkDisplayer ("DEFAULT_WD");
			}
#endif // NL_USE_GTK

#ifdef NL_OS_WINDOWS
			if (disp == "WIN")
			{
				WindowDisplayer = new CWinDisplayer ("DEFAULT_WD");
			}
#endif // NL_OS_WINDOWS

			if (WindowDisplayer == NULL && disp != "NONE")
			{
				nlwarning ("Unknown value for the WindowStyle (should be GTK, WIN or NONE), use no window displayer");
			}
		}

		vector <pair<string,uint> > displayedVariables;
		//uint speedNetLabel, speedUsrLabel, rcvLabel, sndLabel, rcvQLabel, sndQLabel, scrollLabel;
		if (WindowDisplayer != NULL)
		{
			//
			// Init window param if necessary
			//

			sint x=-1, y=-1, w=-1, h=-1, fs=10, history=-1;
			bool iconified = false, ww = false;
			string fn;

			if ((var = ConfigFile.getVarPtr("XWinParam")) != NULL) x = var->asInt();
			if ((var = ConfigFile.getVarPtr("YWinParam")) != NULL) y = var->asInt();
			if ((var = ConfigFile.getVarPtr("WWinParam")) != NULL) w = var->asInt();
			if ((var = ConfigFile.getVarPtr("HWinParam")) != NULL) h = var->asInt();
			if ((var = ConfigFile.getVarPtr("HistoryWinParam")) != NULL) history = var->asInt();
			if ((var = ConfigFile.getVarPtr("IWinParam")) != NULL) iconified = var->asInt() == 1;
			if ((var = ConfigFile.getVarPtr("FontSize")) != NULL) fs = var->asInt();
			if ((var = ConfigFile.getVarPtr("FontName")) != NULL) fn = var->asString();
			if ((var = ConfigFile.getVarPtr("WordWrap")) != NULL) ww = var->asInt() == 1;
			
			if (haveArg('I')) iconified = true;

			WindowDisplayer->create (string("*INIT* ") + _ShortName + " " + _LongName, iconified, x, y, w, h, history, fs, fn, ww, &CommandLog);

			DebugLog->addDisplayer (WindowDisplayer);
			InfoLog->addDisplayer (WindowDisplayer);
			WarningLog->addDisplayer (WindowDisplayer);
			ErrorLog->addDisplayer (WindowDisplayer);
			AssertLog->addDisplayer (WindowDisplayer);
			CommandLog.addDisplayer(WindowDisplayer, true);

			// adding default displayed variables
			displayedVariables.push_back(make_pair(string("NetLop|NetSpeedLoop"), WindowDisplayer->createLabel ("NetLop")));
			displayedVariables.push_back(make_pair(string("UsrLop|UserSpeedLoop"), WindowDisplayer->createLabel ("UsrLop")));
//			displayedVariables.push_back(make_pair(string("Rcv|ReceivedBytes"), WindowDisplayer->createLabel ("Rcv")));
//			displayedVariables.push_back(make_pair(string("Snd|SentBytes"), WindowDisplayer->createLabel ("Snd")));
//			displayedVariables.push_back(make_pair(string("RcvQ|ReceivedQueueSize"), WindowDisplayer->createLabel ("RcvQ")));
//			displayedVariables.push_back(make_pair(string("SndQ|SentQueueSize"), WindowDisplayer->createLabel ("SndQ")));
			displayedVariables.push_back(make_pair(string("|Scroller"), WindowDisplayer->createLabel ("NeL Rulez")));
			
			CConfigFile::CVar *v = ConfigFile.getVarPtr("DisplayedVariables");
			if (v != NULL)
			{
				for (sint i = 0; i < v->size(); i++)
				{
					displayedVariables.push_back(make_pair(v->asString(i), WindowDisplayer->createLabel (v->asString(i).c_str())));
				}
			}
		}

		nlinfo ("Starting Service '%s' using NeL ("__DATE__" "__TIME__") compiled %s", _ShortName.c_str(), CompilationDate.c_str());
		nlinfo ("On OS: %s", CSystemInfo::getOS().c_str());

		setStatus (EXIT_SUCCESS);

		//
		// Redirect signal if needed (in release mode only)
		//

#ifdef NL_OS_WINDOWS
#ifdef NL_RELEASE
		initSignal();
#else
		// don't install signal is the application is started in debug mode
		if (IsDebuggerPresent ())
		{
			//nlinfo("Running with the debugger, don't redirect signals");
			initSignal();
		}
		else
		{
			//nlinfo("Running without the debugger, redirect SIGINT signal");
			initSignal();
		}
#endif
#else // NL_OS_UNIX
		initSignal();
#endif


		//
		// Ignore SIGPIPE (broken pipe) on unix system
		//

#ifdef NL_OS_UNIX
		// Ignore the SIGPIPE signal
		sigset_t SigList;
		bool IgnoredPipe = true;
		if (sigemptyset (&SigList) == -1)
		{
			perror("sigemptyset()");
			IgnoredPipe = false;
		}

		if (sigaddset (&SigList, SIGPIPE) == -1)
		{
			perror("sigaddset()");
			IgnoredPipe = false;
		}

		if (sigprocmask (SIG_BLOCK, &SigList, NULL) == -1)
		{
			perror("sigprocmask()");
			IgnoredPipe = false;
		}
		nldebug ("SIGPIPE %s", IgnoredPipe?"Ignored":"Not Ignored");
#endif // NL_OS_UNIX


		//
		// Initialize the network system
		//
		
		string localhost;
		try
		{
			// Initialize WSAStartup and network stuffs
			CSock::initNetwork();

			// Get the localhost name
			localhost = CInetAddress::localHost().hostName();
		}
		catch (NLNET::ESocket &)
		{
			localhost = "<UnknownHost>";
		}

		// Set the localhost name and service name to the logger
		CLog::setProcessName (localhost+"/"+_ShortName);

		//
		// Initialize server parameters
		//

		// Get the port from config file or in the macro (overload the port set by the user init())
		if ((var = ConfigFile.getVarPtr("Port")) != NULL)
		{
			// set the listen port with the value in the config file if any
			_Port = var->asInt();
		}
		else
		{
			// set the listen port with the value in the NLNET_SERVICE_MAIN macro
			_Port = servicePort;
		}

		// set the listen port if there are a port arg in the command line
		if (haveArg('P'))
		{
			_Port = atoi(getArg('P').c_str());
		}

		// set the aliasname if is present in the command line
		if (haveArg('N'))
		{
			_AliasName = getArg('N');
		}

		// Load the recording state from the config file
		if ((var = ConfigFile.getVarPtr ("Rec")) != NULL)
		{
			string srecstate = var->asString();
			strupr( srecstate );
			if ( srecstate == "RECORD" )
			{
				_RecordingState = CCallbackNetBase::Record;
				nlinfo( "Service recording messages" );
			}
			else if ( srecstate == "REPLAY" )
			{
				_RecordingState = CCallbackNetBase::Replay;
				nlinfo( "Service replaying messages" );
			}
			else
			{
				_RecordingState = CCallbackNetBase::Off;
			}
		}
		else
		{
			// Not found
			_RecordingState = CCallbackNetBase::Off;
		}

		// Load the default stream format
		if ((var = ConfigFile.getVarPtr ("StringMsgFormat")) != NULL)
		{
			CMessage::setDefaultStringMode( var->asInt() == 1 );
		}
		else
		{
			// Not found => binary
			CMessage::setDefaultStringMode( false );
		}

/*
		//
		// Layer4 Startup (Connect to the Naming Service (except for the NS itself and Login Service))
		//

		if (IService::_ShortName != "NS" && IService::_ShortName != "LS" && IService::_ShortName != "AES" && IService::_ShortName != "AS")
		{
			bool ok = false;
			while (!ok)
			{
				// read the naming service address from the config file
				CInetAddress loc(ConfigFile.getVar("NSHost").asString(), ConfigFile.getVar("NSPort").asInt());
				try
				{
					CNetManager::init( &loc, _RecordingState );
					ok = true;
				}
				catch (ESocketConnectionFailed &)
				{
					nlwarning ("Could not connect to the Naming Service (%s). Retrying in a few seconds...", loc.asString().c_str());
					nlSleep (5000);
				}
			}
		}
		else
		{
			CNetManager::init( NULL, _RecordingState );
		}
*/

		///
		/// Layer5 Startup
		///

		// get the sid
		if ((var = ConfigFile.getVarPtr ("SId")) != NULL)
		{
			sint32 sid = var->asInt();
			if (sid<=0 || sid>255)
			{
				nlwarning("Bad SId value in the config file, %d is not in [0;255] range", sid);
				_SId = 0;
			}
			else
			{
				_SId = (uint8) sid;
			}
		}
		else
		{
			// ok, SId not found, use dynamic sid
			_SId = 0;
		}


		// look if we don't want to use NS
		if ((var = ConfigFile.getVarPtr ("DontUseNS")) != NULL)
		{
			// if we set the value in the config file, get it
			_DontUseNS = (var->asInt() == 1);
		}
		else
		{
			// if not, we use ns only if service is not ns, ls, aes, as
			_DontUseNS = false;
		}

		//
		// Register all network associations (must be before the CUnifiedNetwork::getInstance()->init)
		//

		if ((var = ConfigFile.getVarPtr ("Networks")) != NULL)
		{
			for (uint8 i = 0; i < var->size (); i++)
				CUnifiedNetwork::getInstance()->addNetworkAssociation (var->asString(i), i);
		}

		if ((var = ConfigFile.getVarPtr ("DefaultNetworks")) != NULL)
		{
			for (uint8 i = 0; i < var->size (); i++)
				CUnifiedNetwork::getInstance()->addDefaultNetwork(var->asString(i));
		}

		// normal setup for the common services
		if (!_DontUseNS)
		{
			bool ok = false;
			while (!ok)
			{
				// read the naming service address from the config file
				string LSAddr = ConfigFile.getVar ("NSHost").asString();
				
				// if there's no port to the NS, use the default one 50000
				if (LSAddr.find(":") == string::npos)
					LSAddr += ":50000";

				CInetAddress loc(LSAddr);
				try
				{
					CUnifiedNetwork::getInstance()->init (&loc, _RecordingState, _ShortName, _Port, _SId);

					ok = true;
				}
				catch (ESocketConnectionFailed &)
				{
					nlwarning ("Could not connect to the Naming Service (%s). Retrying in a few seconds...", loc.asString().c_str());
					nlSleep (5000);
				}
			}
		}
		else
		{
			CUnifiedNetwork::getInstance()->init(NULL, _RecordingState, _ShortName, _Port, _SId);
		}

		// At this point, the _SId must be ok if we use the naming service.
		// If it's 0, it means that we don't use NS and we left the other side server to find a sid for your connection

		if(!_DontUseNS)
		{
			nlassert (_SId != 0);
		}

		//
		// Connect to the local AES and send identification
		//

		// look if we don't want to use NS
		if ((var = ConfigFile.getVarPtr ("DontUseAES")) != NULL)
		{
			// if we set the value in the config file, get it
			_DontUseAES = var->asInt() == 1;
		}
		else
		{
			// if not, we use aes only if service is not aes or as
			_DontUseAES = false;
		}

		if (!_DontUseAES)
		{
			CUnifiedNetwork::getInstance()->setServiceUpCallback ("AES", AESConnection, NULL);
			CUnifiedNetwork::getInstance()->setServiceDownCallback ("AES", AESDisconnection, NULL);
			CUnifiedNetwork::getInstance()->addService ("AES", CInetAddress("localhost:49997"));
		}
		CUnifiedNetwork::getInstance()->addCallbackArray (AESCallbackArray, sizeof(AESCallbackArray)/sizeof(AESCallbackArray[0]));
		

		//
		// Add callback array
		//

		// add callback set in the NLNET_SERVICE_MAIN macro
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(_CallbackArray, _CallbackArraySize);

		//
		// Now we have the service id, we can set the entites id generator
		//

		_NextEntityId.setServiceId(_SId);

		// Set the localhost name and service name and the sid
		CLog::setProcessName (localhost+"/"+_ShortName+"-"+toString((uint16)_SId));


		//
		// Add default pathes
		//

		if ((var = ConfigFile.getVarPtr ("Paths")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), true, false);
			}
		}

		if ((var = ConfigFile.getVarPtr ("PathsNoRecurse")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), false, false);
			}
		}


		// if we can, try to setup where to write files
		if ((var = ConfigFile.getVarPtr ("WriteFilesDirectory")) != NULL)
		{
			WriteFilesDirectory = CPath::standardizePath(var->asString());
		}


		//
		// Call the user service init
		//

		userInitCalled = true; // the bool must be put *before* the call to init()
		init ();


		//
		// Connects to the present services
		// WARNING: only after the user init() was called because the
		// addService may call up service callbacks.
		//

		CUnifiedNetwork::getInstance()->connect();

		initAlarms ();

		//
		// Say to the AES that the service is ready
		//

		if (!_DontUseAES)
		{
			// send the ready message (service init finished)
			CMessage msgout ("SR");
			CUnifiedNetwork::getInstance()->send("AES", msgout);
		}


		_Initialized = true;

		nlinfo ("Service initialised, executing StartCommands");

		//
		// Call the user command from the config file if any
		//

		if ((var = ConfigFile.getVarPtr ("StartCommands")) != NULL)
		{
			for (sint i = 0; i < var->size(); i++)
			{
				ICommand::execute (var->asString(i), CommandLog);
			}
		}

		string str;
		CLog logDisplayVars;
		CLightMemDisplayer mdDisplayVars;
		logDisplayVars.addDisplayer (&mdDisplayVars);

		nlinfo ("Service ready");

		if (WindowDisplayer != NULL)
			WindowDisplayer->setTitleBar (_ShortName + " " + _LongName + " " + _Version);

		//
		// Call the user service update each loop and check files and network activity
		//

		do
		{
			if(Bench) CHTimer::startBench(false, true, false);

			// count the amount of time to manage internal system
			TTime bbefore = CTime::getLocalTime ();

			// call the user update and exit if the user update asks it
			H_BEFORE(NLNETServiceUpdate);
			if (!update ())
			{
				H_AFTER(NLNETServiceUpdate);
				CHTimer::endBench();
				break;
			}
			NbUserUpdate++;
			H_AFTER(NLNETServiceUpdate);
			
			// count the amount of time to manage internal system
			TTime before = CTime::getLocalTime ();

			if (WindowDisplayer != NULL)
			{
				// update the window displayer and quit if asked
				if (!WindowDisplayer->update ())
				{
					nlinfo ("The window displayer was closed by user, need to quit");
					ExitSignalAsked = 1;
				}
			}

			// stop the loop if the exit signal asked
			if (ExitSignalAsked > 0)
			{
				CHTimer::endBench();
				break;
			}
	
			CConfigFile::checkConfigFiles ();
	
			updateAlarms ();

			CFile::checkFileChange();

			H_BEFORE(NLNETManageMessages); // Not tick-wise
			// get and manage layer 5 messages
			CUnifiedNetwork::getInstance()->update (_UpdateTimeout);
			H_AFTER(NLNETManageMessages); // Not tick-wise
			
			// resync the clock every hours
			if (resyncEvenly)
			{
				static TTime LastSyncTime = CTime::getLocalTime ();

				//---------------------------------------
				// To simulate Ctrl-C in the debugger... Exit after 1 min !
				/*if (CTime::getLocalTime () - LastSyncTime > 60 * 1000 )
				{
					ExitSignalAsked = 1;
				}*/
				//---------------------------------------
/*
				if (CTime::getLocalTime () - LastSyncTime > 60*60*1000)
				{
					CUniTime::syncUniTimeFromService ( _RecordingState );
					LastSyncTime = CTime::getLocalTime ();
				}
*/
			}

			NetSpeedLoop = (sint32) (CTime::getLocalTime () - before);
			UserSpeedLoop = (sint32) (before - bbefore);

			if (WindowDisplayer != NULL)
			{
				uint64 rcv, snd, rcvq, sndq;
				rcv = CUnifiedNetwork::getInstance()->getBytesReceived ();
				snd = CUnifiedNetwork::getInstance()->getBytesSent ();
				rcvq = CUnifiedNetwork::getInstance()->getReceiveQueueSize ();
				sndq = CUnifiedNetwork::getInstance()->getSendQueueSize ();

				for (uint i = 0; i < displayedVariables.size(); i++)
				{
					// it s a separator, do nothing
					if (displayedVariables[i].first.empty())
						continue;

					// it s a command, do nothing
					if (displayedVariables[i].first[0] == '@')
						continue;

					string dispName = displayedVariables[i].first;
					string varName = dispName;
					uint32 pos = dispName.find("|");
					if (pos != string::npos)
					{
						varName = displayedVariables[i].first.substr(pos+1);
						dispName = displayedVariables[i].first.substr(0, pos);
					}

					if (dispName.empty())
						str = "";
					else
						str = dispName + ": ";
					
					mdDisplayVars.clear ();
					ICommand::execute(varName, logDisplayVars, true);
					const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();
					if (strs.size()>0)
					{
						
						string s_ = strs[0];

						uint32 pos = strs[0].find("=");
						if(pos != string::npos && pos + 2 < strs[0].size())
						{
							uint32 pos2 = string::npos;
							if(strs[0][strs[0].size()-1] == '\n')
								pos2 = strs[0].size() - pos - 2 - 1;

							str += strs[0].substr (pos+2, pos2);
						}
						else
						{
							str += "???";
						}
					}
					else
					{
						str += "???";
					}
					mdDisplayVars.unlockStrings();
					WindowDisplayer->setLabel (displayedVariables[i].second, str);
				}

			}

//			nldebug ("SYNC: updatetimeout must be %d and is %d, sleep the rest of the time", _UpdateTimeout, delta);

			CHTimer::endBench();
			
			// Resetting the hierarchical timer must be done outside the top-level timer
			if ( _ResetMeasures )
			{
				CHTimer::clear();
				_ResetMeasures = false;
			}
		}
		while (true);
	}
/*	catch (ETrapDebug &)
	{
		// we have to do that if we want to trap unhandled exception with the report message box
		setStatus (EXIT_FAILURE);
	}
*/	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}
	catch ( uint ) // SEH exceptions
	{
		ErrorLog->displayNL( "System exception" );
	}

#ifdef NL_RELEASE
/*	// in release mode, we catch everything to handle clean release.
	catch (Exception &e)
	{
		// Catch NeL exception to release the system cleanly
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: NeL Exception: Error running the service \"%s\": %s", _ShortName.c_str(), e.what());
	}
	catch (...)
	{
		// Catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: Unknown external exception");
	}
*/
#endif

	try
	{
		nlinfo ("Service starts releasing");

		//
		// Call the user service release() if the init() was called
		//

		if (userInitCalled)
			release ();

		//
		// Delete all network connection (naming client also)
		//

		CUnifiedNetwork::getInstance()->release ();

		CSock::releaseNetwork ();

		//
		// Remove the window displayer
		//

		if (WindowDisplayer != NULL)
		{
			DebugLog->removeDisplayer (WindowDisplayer);
			InfoLog->removeDisplayer (WindowDisplayer);
			WarningLog->removeDisplayer (WindowDisplayer);
			ErrorLog->removeDisplayer (WindowDisplayer);
			AssertLog->removeDisplayer (WindowDisplayer);
			CommandLog.removeDisplayer (WindowDisplayer);

			delete WindowDisplayer;
			WindowDisplayer = NULL;
		}

		nlinfo ("Service released succesfuly");
	}
/*	catch (ETrapDebug &)
	{
		// we have to do that if we want to trap unhandled exception with the report message box
		setStatus (EXIT_FAILURE);
	}
*/	catch (EFatalError &)
	{
		// Somebody call nlerror, so we have to quit now, the message already display
		// so we don't have to to anything
		setStatus (EXIT_FAILURE);
	}

#ifdef NL_RELEASE
/*	// in release mode, we catch everything to handle clean release.
	catch (Exception &e)
	{
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: NeL Exception: Error releasing the service \"%s\": %s", _ShortName.c_str(), e.what());
	}
	catch (...)
	{
		// Catch anything we can to release the system cleanly
		setStatus (EXIT_FAILURE);
		nlinfo ("ERROR: Unknown external exception");
	}
*/
#endif

	CHTimer::display();
	CHTimer::displayByExecutionPath ();
	CHTimer::displayHierarchical(&CommandLog, true, 64);
	CHTimer::displayHierarchicalByExecutionPathSorted (&CommandLog, CHTimer::TotalTime, true, 64);

	nlinfo ("Service ends");

	string name = getServiceLongName () + ".memory_report";
	NLMEMORY::StatisticsReport (name.c_str(), false);

	return ExitSignalAsked?100+ExitSignalAsked:getStatus ();
}

void IService::exit (sint code)
{
	nlinfo ("somebody called IService::exit(), I have to quit");
	ExitSignalAsked = code;
}

/*
 * Require to reset the hierarchical timer
 */
void IService::requireResetMeasures()
{
	_ResetMeasures = true;
}


std::string IService::getServiceUnifiedName () const
{
	nlassert (!_ShortName.empty());
	string res;
	if (!_AliasName.empty())
	{
		res = _AliasName+"/";
	}
	res += _ShortName;
	if (_SId != 0)
	{
		res += "-";
		res += toString (_SId);
	}
	return res;
}


//
// Commands and Variables for controling all services
//

NLMISC_DYNVARIABLE(string, LaunchingDate, "date of the launching of the program")
{
	if (get) *pointer = asctime (localtime ((time_t*)&LaunchingDate));
}

NLMISC_DYNVARIABLE(string, Uptime, "time from the launching of the program")
{
	if (get) *pointer = secondsToHumanReadable (CTime::getSecondsSince1970() - LaunchingDate);
}

NLMISC_VARIABLE(bool, Bench, "1 if benching 0 if not");

NLMISC_VARIABLE(string, CompilationDate, "date of the compilation");
NLMISC_VARIABLE(string, CompilationMode, "mode of the compilation");

NLMISC_VARIABLE(sint32, NetSpeedLoop, "duration of the last network loop (in ms)");
NLMISC_VARIABLE(sint32, UserSpeedLoop, "duration of the last user loop (in ms)");

NLMISC_VARIABLE(uint32, NbUserUpdate, "number of time the user IService::update() called");

NLMISC_DYNVARIABLE(uint32, ListeningPort, "default listening port for this service")
{
	if (get) *pointer = IService::getInstance()->getPort();
}

NLMISC_DYNVARIABLE(string, Version, "")
{
	if (get) *pointer = IService::getInstance()->_Version;
}

NLMISC_DYNVARIABLE(string, RunningDirectory, "path where the service is running")
{
	if (get) *pointer = IService::getInstance()->_RunningPath;
}

NLMISC_DYNVARIABLE(string, LogDirectory, "path where the service is logging")
{
	if (get) *pointer = IService::getInstance()->_LogDir;
}

NLMISC_DYNVARIABLE(string, ConfigDirectory, "path where the config file is")
{
	if (get) *pointer = IService::getInstance()->_ConfigDir + IService::getInstance()->_LongName + ".cfg";
}

NLMISC_DYNVARIABLE(string, Scroller, "current size in bytes of the sent queue size")
{
	if (get)
	{
		// display the scroll text
		static string foo =	"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! "
							"Welcome to NeL Service! This scroll is used to see the update frequency of the main function and to see if the service is frozen or not. Have a nice day and hope you'll like NeL!!! ";
		static int pos = 0;
		*pointer = foo.substr ((pos++)%(foo.size()/2), 10);
	}
}

NLMISC_COMMAND (quit, "exit the service", "")
{
	if(args.size() != 0) return false;

	nlinfo ("User ask me with a command to quit");
	ExitSignalAsked = 0xFFFF;

	return true;
}

NLMISC_COMMAND (brutalQuit, "exit the service brutally", "")
{
	if(args.size() != 0) return false;

	::exit (0xFFFFFFFF);

	return true;
}


#ifdef MUTEX_DEBUG
NLMISC_COMMAND (mutex, "display mutex values", "")
{
	if(args.size() != 0) return false;

	map<CFairMutex*,TMutexLocks>	acquiretimes = getNewAcquireTimes();

	map<CFairMutex*,TMutexLocks>::iterator im;
	for ( im=acquiretimes.begin(); im!=acquiretimes.end(); ++im )
	{
		nlinfo( "%d %p %s: %.0f %.0f, called %u times th(%d, %d wait)%s", (*im).second.MutexNum, (*im).first, (*im).second.MutexName.c_str(),
			CTime::cpuCycleToSecond((*im).second.TimeToEnter)*1000.0, CTime::cpuCycleToSecond((*im).second.TimeInMutex)*1000.0,
			(*im).second.Nb, (*im).second.ThreadHavingTheMutex, (*im).second.WaitingMutex,
			(*im).second.Dead?" DEAD":"");
	}

	return true;
}
#endif // MUTEX_DEBUG

NLMISC_COMMAND (serviceInfo, "display information about this service", "")
{
	if(args.size() != 0) return false;

	log.displayNL ("Service %s '%s' using NeL ("__DATE__" "__TIME__")", IService::getInstance()->getServiceLongName().c_str(), IService::getInstance()->getServiceUnifiedName().c_str());
	log.displayNL ("Service listening port: %d", IService::getInstance()->_Port);
	log.displayNL ("Service running directory: '%s'", IService::getInstance()->_RunningPath.c_str());
	log.displayNL ("Service log directory: '%s'", IService::getInstance()->_LogDir.c_str());
	log.displayNL ("Service config directory: '%s' config filename: '%s.cfg'", IService::getInstance()->_ConfigDir.c_str(), IService::getInstance()->_LongName.c_str());
	log.displayNL ("Service id: %d", IService::getInstance()->_SId);
	log.displayNL ("Service update timeout: %dms", IService::getInstance()->_UpdateTimeout);
	log.displayNL ("Service %suse naming service", IService::getInstance()->_DontUseNS?"don't ":"");
	log.displayNL ("Service %suse admin executor service", IService::getInstance()->_DontUseAES?"don't ":"");
	log.displayNL ("NeL is compiled in %s mode", CompilationMode.c_str());

	nlinfo ("Services arguments: %d args", IService::getInstance()->_Args.size ());
	for (uint i = 0; i < IService::getInstance()->_Args.size (); i++)
	{
		nlinfo ("  argv[%d] = '%s'", i, IService::getInstance()->_Args[i].c_str ());
	}

	log.displayNL ("Naming service info: %s", CNamingClient::info().c_str());

	ICommand::execute ("services", log);

	return true;
}

NLMISC_COMMAND(resetMeasures, "reset hierarchical timer", "")
{
	IService::getInstance()->requireResetMeasures();
	return true;
}

NLMISC_COMMAND(getWinDisplayerInfo, "display the info about the pos and size of the window displayer", "")
{
	uint32 x,y,w,h;
	IService::getInstance()->WindowDisplayer->getWindowPos (x,y,w,h);
	log.displayNL ("Window Displayer : XWinParam = %d; YWinParam = %d; WWinParam = %d; HWinParam = %d;", x, y, w, h);
	return true;
}

NLMISC_COMMAND(displayConfigFile, "display the variables of the default configfile", "")
{
	IService::getInstance()->ConfigFile.print(&log);
	return true;
}

NLMISC_COMMAND(getUnknownConfigFileVariables, "display the variables from config file that are called but not present", "")
{
	log.displayNL ("%d Variables not found in the configfile '%s'", IService::getInstance()->ConfigFile.UnknownVariables.size(), IService::getInstance()->ConfigFile.getFilename().c_str() );
	for (uint i = 0; i < IService::getInstance()->ConfigFile.UnknownVariables.size(); i++)
	{
		log.displayNL ("  %s", IService::getInstance()->ConfigFile.UnknownVariables[i].c_str());
	}
	return true;
}

NLMISC_COMMAND (freeze, "Freeze the service for N seconds (for debug purpose)", "<N>")
{
	if(args.size() != 1) return false;

	sint32 n = atoi (args[0].c_str());

	log.displayNL ("Freezing %d seconds", n);

	nlSleep(n * 1000);	
	return true;
}

/*
string foo = "205kb", bar = "2b";

NLMISC_VARIABLE(string, foo, "test the get view system");
NLMISC_VARIABLE(string, bar, "test the get view system");

vector<pair<uint32,uint32> > _Entities;

void selectEntities (const string entityName, vector <uint32> &entities)
{
	if (entityName.empty ())
		return;

	uint32 entity = atoi (entityName.c_str());

	if (entityName == "*")
	{
		// we want all entities
		for (uint i = 0; i < _Entities.size(); i++)
			entities.push_back (i);
	}
	else if (entityName.find ("-") != string::npos)
	{
		// it's a range
		uint ent2 = atoi(entityName.substr(entityName.find ("-")+1).c_str());
		for (uint i = entity; i <= ent2; i++)
			entities.push_back (i);
	}
	else
	{
		// we want a specific entity
		entities.push_back (entity);
	}
}



#define ENTITY_VARIABLE(__name,__help) \
struct __name##Class : public NLMISC::ICommand \
{ \
__name##Class () : NLMISC::ICommand(#__name, __help, "<entity> [<value>]") { Type = Variable; } \
	virtual bool execute(const std::vector<std::string> &args, NLMISC::CLog &log) \
	{ \
		if (args.size () != 1 && args.size () != 2) \
			return false; \
 \
		vector <uint32> entities; \
		selectEntities	(args[0], entities); \
 \
		for (uint i = 0; i < entities.size(); i++) \
		{ \
			string value; \
			if (args.size()==2) \
				value = args[1]; \
			else \
				value = "???"; \
			pointer (entities[i], (args.size()==1), value); \
			log.displayNL ("Entity %d Variable %s = %s", entities[i], _CommandName.c_str(), value.c_str()); \
		} \
		return true; \
	} \
	void pointer(uint32 entity, bool get, std::string &value); \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(uint32 entity, bool get, std::string &value)

ENTITY_VARIABLE(test, "test")
{
	if (get)
	{
		// get the value if available
		if(entity < _Entities.size())
			value = toString(_Entities[entity].first);
	}
	else
	{
		// set the variable with the new value
		if(entity >= _Entities.size())
			_Entities.resize(entity+1);

		_Entities[entity].first = atoi(value.c_str());
	}
}

ENTITY_VARIABLE(test2, "test2")
{
	if (get)
	{
		// get the value if available
		if(entity < _Entities.size())
			value = toString(_Entities[entity].second);
	}
	else
	{
		// set the variable with the new value
		if(entity >= _Entities.size())
			_Entities.resize(entity+1);
		
		_Entities[entity].second = atoi(value.c_str());
	}
}
*/




// -1 = service is quitting
// 0 = service is not connected
// 1 = service is running
// 2 = service is launching
// 3 = service failed launching

NLMISC_DYNVARIABLE(string, State, "Set this value to 0 to shutdown the service and 1 to start the service")
{
	static string running = "Online";

	// read or write the variable
	if (get)
	{
		*pointer = running;
	}
	else
	{
		if (IService::getInstance()->getServiceShortName() == "AES" || IService::getInstance()->getServiceShortName() == "AS")
		{
			nlinfo ("I can't set State=0 because I'm the admin and I should never quit");
		}
		else if (*pointer == "0" || *pointer == "2")
		{
			// ok, we want to set the value to false, just quit
			nlinfo ("User ask me with a command to quit using the State variable");
			ExitSignalAsked = 0xFFFE;
			running = "Quitting";
		}
		else
		{
			nlwarning ("Unknown value for State '%s'", (*pointer).c_str());
		}
	}
}


NLMISC_COMMAND (getView, "send a view and receive an array as result", "<varpath>")
{
	if(args.size() != 1) return false;
	
	vector<pair<vector<string>, vector<string> > > answer;
	serviceGetView (0, args[0], answer);

	log.displayNL("have %d answer", answer.size());
	for (uint i = 0; i < answer.size(); i++)
	{
		log.displayNL("  have %d value", answer[i].first.size());

		nlassert (answer[i].first.size() == answer[i].second.size());

		for (uint j = 0; j < answer[i].first.size(); j++)
		{
			log.displayNL("    %s -> %s", answer[i].first[j].c_str(), answer[i].second[j].c_str());
		}
	}

	return true;
}



} //NLNET
