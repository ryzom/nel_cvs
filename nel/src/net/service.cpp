/* service.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: service.cpp,v 1.13 2000/10/11 10:06:00 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/types_nl.h"

#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <signal.h>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"

#include "nel/net/base_socket.h"
#include "nel/net/service.h"
#include "nel/net/inet_address.h"

#include "nel/net/service.h"

using namespace std;
using namespace NLMISC;

namespace NLNET
{

static IService *Service = NULL;


///////////////////////////////

/* "Constants" */

static const int Signal[] = {
  SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

/* Variables */

static const char **SignalName;

/* Prototypes */

static void SigHandler (int Sig);

void InitSignal(void) {
  
  int I, SignalNb;
  
  SignalNb = 0;
  for (I = 0; I < (int)(sizeof(Signal)/sizeof(Signal[0])); I++) {
    if (Signal[I] >= SignalNb) SignalNb = Signal[I] + 1;
  }
  
  SignalName = (const char **) malloc((size_t)(SignalNb*sizeof(char *)));
  if (SignalName == NULL) exit(0);
  for (I = 0; I < SignalNb; I++) SignalName[I] = NULL;
  
  for (I = 0; I < (int)(sizeof(Signal)/sizeof(Signal[0])); I++) {
    switch (Signal[I]) {
    case SIGABRT : SignalName[SIGABRT] = "SIGABRT"; break;
    case SIGFPE  : SignalName[SIGFPE]  = "SIGFPE";  break;
    case SIGILL  : SignalName[SIGILL]  = "SIGILL";  break;
    case SIGINT  : SignalName[SIGINT]  = "SIGINT";  break;
    case SIGSEGV : SignalName[SIGSEGV] = "SIGSEGV"; break;
    case SIGTERM : SignalName[SIGTERM] = "SIGTERM"; break;
    }
    signal(Signal[I],SigHandler);
  }
}

/* SigHandler() */

static void SigHandler(int Sig) {
  
  signal(Sig,SigHandler);

  if (SignalName[Sig] != NULL) {
    printf("%s received\n", SignalName[Sig]);
  } else {
    printf("Signal #%d received\n",Sig);
  }
  
  switch (Sig) {
  case SIGABRT :
  case SIGILL  :
  case SIGINT  :
  case SIGSEGV :
  case SIGTERM :
    printf("exitcalled()\n");
    break;
  }
}


///////////////////////////////





void ExitFunc ()
{
	cout << "** ExitFunc **" << endl;
	try
	{
		if (Service != NULL)
		{
			// release only one time
			IService *is = Service;
			Service = NULL;

			cout << "** ExitFunc Release **" << endl;
			is->release ();
		}
	}
	catch (Exception &e)
	{
		nlerror ("Error releasing service : %s", e.what());
		if (Service != NULL) Service->setStatus (EXIT_FAILURE);
	}
}


sint IService::main (int argc, char **argv)
{
	try
	{
		Service = this;
		atexit (ExitFunc);

		InitSignal();

		for (sint i = 0; i < argc; i++)
		{
			_Args.push_back (argv[i]);
		}

		setStatus (EXIT_SUCCESS);

		string localhost;
		try
		{
			// initialize WSAStartup and network stuffs
			CBaseSocket::init();

			// get the localhost name
			localhost = CInetAddress::localHost().hostName();
		}
		catch (NLNET::ESocket &)
		{
			localhost = "<UnknownHost>";
		}

		// set the localhost name and service name to the logger
		CLog::setLocalHostAndService (localhost, _Name);

		// initialize debug stuffs, create displayers for rk* functions
		InitDebug();

		// user service init
		init ();

		// user service update call each loop
		while (update())
		{
			CConfigFile::checkConfigFiles ();
		}
		ExitFunc ();
	}
	catch (Exception &e)
	{
		ExitFunc ();
		setStatus (EXIT_FAILURE);
		nlerror ("Error running the service \"%s\": %s", _Name, e.what());
	}
	catch (...)
	{
		ExitFunc ();
		setStatus (EXIT_FAILURE);
		nlerror ("Unknown external exception");
	}
	return getStatus ();
}

} //NLNET