/** \file service/chat_service.cpp
 * example of the IService class
 *
 * $Id: chat_service.cpp,v 1.1 2002/04/17 08:08:32 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <string>

// contains the service base class
#include "nel/net/service.h"

using namespace std;
using namespace NLNET;

// a service is a process, a program. the goal is to automatically use
// common initialization and features (like command line, file check,
// debug features, signal redirection and more).

// a service has a listen socket for external connection. when a message
// comes on the socket, the message is automatically updated and the
// associated callback is called.

// to create a service, you have to inherit from IService and to implement
// a few functions (init, update, release). you are not forced to implement
// these functions if you have nothing to do in them.

// to launch a service, the naming_service must run.


// this stupid example creates a chat service. it's a 1 person chat, it
// means that you can send strings to the service and it sends them back to you
// (and only to you) so you can't see strings from other people :)
// (i warned you! it's really stupid!)

class CChatService : public IService
{
public:

	void init ()
	{
		// this function is called after all standard service initialization.
		// put here your code that inits your application.

		nlinfo ("init() was called");
	}

	bool update ()
	{
		// this function is called every "loop". you return true if you want
		// to continue or return false if you want to exit the service.
		// the loop is called evenly (by default, at least one time per second).

		nlinfo ("update() was called");

		return true;
	}

	void release ()
	{
		// this function is called before all standard service release code.
		// put here your code that releases your application.

		nlinfo ("release() was called");
	}
};


// each time the message CHAT is received, this function is called.
// the first param contains parameters of the message. the second one is the
// identifier of who sent this message
static void cbChat (CMessage &msgin, const std::string &serviceName, uint16 sid)
{
	// get the chat string of the client
	string chat;
	msgin.serial (chat);

	// create the message to send to the other
	CMessage msgout ("CHAT");
	msgout.serial (chat);

	// send it back to the sender
	CUnifiedNetwork::getInstance()->send (sid, msgout);
}


// this array contains all callback functions. it associates the callbackname (messagename),
// with a callback
TUnifiedCallbackItem CallbackArray[] =
{
	{ "CHAT", cbChat }
};


// this macro is the "main". the first param is the class name inherited from IService.
// the second one is the name of the service used to register and find the service
// using the naming service. the third one is the port where the listen socket will
// be created. If you put 0, the system automatically finds a port.
NLNET_SERVICE_MAIN (CChatService, "CS", "chat_service", 0, CallbackArray, "", "");
