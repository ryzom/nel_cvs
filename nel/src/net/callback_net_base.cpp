/** \file callback_net_base.cpp
 * Network engine, layer 3, base
 *
 * $Id: callback_net_base.cpp,v 1.15 2001/06/12 15:41:11 lecroart Exp $
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
#include <vector>
#include <set>

#include "nel/misc/string_id_array.h"
#include "nel/misc/time_nl.h"

#include "nel/net/buf_sock.h"
#include "nel/net/callback_net_base.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {


void cbnbMessageRecvAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// receive a new message association

	CStringIdArray::TStringId size;
	msgin.serial (size);

	nldebug ("L3NB_ASSOC: The other side gave me %d association strings", size);

	for (CStringIdArray::TStringId i = 0; i < size; i++)
	{
		std::string name;
		CStringIdArray::TStringId id;

		msgin.serial (name);
		msgin.serial (id);

		// if id == -1, it means that there are no callback associated to this message
		// it should not happen, it mean that one side send a message that the other side
		// can't manage in his callbackarray.
		// to resolve the problem, add the callback in the callbackarray in the other side
		// and put NULL if you don't want to manage this message
		nlassert (id != -1);

		nldebug ("L3NB_ASSOC:  association '%s' -> %d", name.c_str (), id);
		netbase.getSIDA().addString (name, id);
	}
}


// the other side want to know some of my association, send them!
void cbnbMessageAskAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	CMessage msgout (netbase.getSIDA(), "RA");
	CStringIdArray::TStringId size;
	msgin.serial (size);

	nldebug ("L3NB_ASSOC: The other side want %d string associations", size);

	msgout.serial (size);

	for (sint i = 0; i < size; i++)
	{
		string name;
		msgin.serial (name);
		nldebug ("L3NB_ASSOC:  sending association '%s' -> %d", name.c_str (), netbase._OutputSIDA.getId(name));

		// if this assert occurs, it means that the other side ask an unknown message
		// or that there are different types of client (with differents callbackarray) and one of the client doesn't have this callback
		nlassert(netbase._OutputSIDA.getId(name) != -1);

		msgout.serial (name);

		CStringIdArray::TStringId id = netbase._OutputSIDA.getId (name);
		msgout.serial (id);
	}
	// send the message to the other side
	netbase.send (msgout, from);
}

static TCallbackItem cbnbMessageAssociationArray[] =
{
	{ "AA", cbnbMessageAskAssociations },
	{ "RA", cbnbMessageRecvAssociations },
};

void cbnbNewDisconnection (TSockId from, void *data)
{
	nlassert (data != NULL);
	CCallbackNetBase *base = (CCallbackNetBase *)data;

	nldebug("L3NB: cbnbNewDisconnection()");

	// call the client callback if necessary
	if (base->_DisconnectionCallback != NULL)
		base->_DisconnectionCallback (from, base->_DisconnectionCbArg);
}

CCallbackNetBase::CCallbackNetBase () : _FirstUpdate (true), _DisconnectionCallback(NULL), _DisconnectionCbArg(NULL)
{
	_NewDisconnectionCallback = cbnbNewDisconnection;

	nldebug ("disable display layer 3 association message");
	DebugLog->addNegativeFilter ("L3NB_ASSOC");

	// add the callback needed to associate messages with id
	addCallbackArray (cbnbMessageAssociationArray, sizeof (cbnbMessageAssociationArray) / sizeof (cbnbMessageAssociationArray[0]));
}

/*
 *	Append callback array with the specified array
 */
void CCallbackNetBase::addCallbackArray (const TCallbackItem *callbackarray, CStringIdArray::TStringId arraysize)
{
	// be sure that the 2 array have the same size
	nlassert (_CallbackArray.size () == (uint)_OutputSIDA.size ());

	if (arraysize == 1 && callbackarray[0].Callback == NULL && string("") == callbackarray[0].Key)
	{
		// it's an empty array, ignore it
		return;
	}

	// resize the array
	sint oldsize = _CallbackArray.size();

	_CallbackArray.resize (oldsize + arraysize);
	_OutputSIDA.resize (oldsize + arraysize);

//TOO MUCH MESSAGE	nldebug ("L3NB_CB: Adding %d callback to the array", arraysize);

	for (sint i = 0; i < arraysize; i++)
	{
		CStringIdArray::TStringId ni = oldsize + i;
//TOO MUCH MESSAGE		nldebug ("L3NB_CB: Adding callback to message '%s', id '%d'", callbackarray[i].Key, ni);
		// copy callback value
		
		_CallbackArray[ni] = callbackarray[i];
		// add the string to the string id array
		_OutputSIDA.addString (callbackarray[i].Key, ni);

	}
	nldebug ("L3NB_CB: Added %d callback Now, there's %d callback associated with message type", arraysize, _CallbackArray.size ());
}

void CCallbackNetBase::processOneMessage ()
{
	CMessage msgin (_OutputSIDA, "", true);
	TSockId tsid;
	receive (msgin, &tsid);

	nldebug ("L3NB: Received a message %s from %s", msgin.toString().c_str(), tsid->asString().c_str());
	
	// now, we have to call the good callback
	NLMISC::CStringIdArray::TStringId pos = -1;
	if (msgin.TypeHasAnId)
	{
		pos = msgin.getId ();
	}
	else
	{
		std::string name = msgin.getName ();
		sint16 i;
		for (i = 0; i < (sint16) _CallbackArray.size (); i++)
		{
			if (name == _CallbackArray[i].Key)
			{
				pos = i;
				break;
			}
		}
	}

	if (pos < 0 || pos >= (sint16) _CallbackArray.size ())
	{
		nlerror ("L3NB_CB: Callback %s not found in _CallbackArray", msgin.toString().c_str());
	}
	else
	{
		TSockId realid = getSockId (tsid);

		if (!realid->AuthorizedCallback.empty() && msgin.getName() != realid->AuthorizedCallback)
		{
			nlwarning ("L3NB_CB: %s try to call the callback %s but only %s is authorized. Disconnect him!", tsid->asString().c_str(), msgin.toString().c_str(), tsid->AuthorizedCallback.c_str());
			disconnect (tsid);
		}
		else if (_CallbackArray[pos].Callback == NULL)
		{
			nlwarning ("L3NB_CB: Callback %s is NULL, can't call it", msgin.toString().c_str());
		}
		else
		{
			nldebug ("L3NB_CB: Calling callback (%s)", _CallbackArray[pos].Key);
			_CallbackArray[pos].Callback (msgin, realid, *this);
		}
	}
}


void CCallbackNetBase::baseUpdate (sint32 timeout)
{
	nlassert( timeout >= -1 );
	TTime t0 = CTime::getLocalTime();

	//
	// The first time, we init time counters
	//
	if (_FirstUpdate)
	{
		nldebug("L3NB: First update()");
		_FirstUpdate = false;
		_LastUpdateTime = CTime::getLocalTime ();
		_LastMovedStringArray = CTime::getLocalTime ();
	}

	//
	// Every 1 seconds if we have new unknown association, we ask them to the other side
	//
	if (_LastUpdateTime + 1000 < CTime::getLocalTime ())
	{
//		nldebug("L3NB: baseUpdate()");
		_LastUpdateTime = CTime::getLocalTime ();

		const set<string> &sa = _InputSIDA.getNeedToAskedStringArray ();
		if (!sa.empty ())
		{
			CMessage msgout (_InputSIDA, "AA");
			nlassert (sa.size () < 65536);
			CStringIdArray::TStringId size = sa.size ();
			nldebug ("L3NB_ASSOC: I need %d string association, ask them to the other side", size);
			msgout.serial (size);
			for (set<string>::iterator it = sa.begin(); it != sa.end(); it++)
			{
				nldebug ("L3NB_ASSOC:  what is the id of '%s'?", (*it).c_str ());
				string str(*it);
				msgout.serial (str);
			}
			// send the message to the other side
			send (msgout, 0);
			_InputSIDA.moveNeedToAskToAskedStringArray();
			_LastMovedStringArray = CTime::getLocalTime ();
		}
	}

	//
	// Every 60 seconds if we have not answered association, we ask again to get them!
	//
	if (!_InputSIDA.getAskedStringArray().empty() && _LastMovedStringArray + 60000 < CTime::getLocalTime ())
	{
		// we didn't have an answer for the association, resend them
		const set<string> sa = _InputSIDA.getAskedStringArray ();
		CMessage msgout (_InputSIDA, "AA");
		nlassert (sa.size () < 65536);
		CStringIdArray::TStringId size = sa.size ();
		nldebug ("L3NB_ASSOC: client didn't answer my asked association, retry! I need %d string association, ask them to the other side", size);
		msgout.serial (size);
		for (set<string>::iterator it = sa.begin(); it != sa.end(); it++)
		{
			nldebug ("L3NB_ASSOC:  what is the id of '%s'?", (*it).c_str ());
			string str(*it);
			msgout.serial (str);
		}
		// sends the message to the other side
		send (msgout, 0);
		_LastMovedStringArray = CTime::getLocalTime ();
	}


	/*
	 * timeout -1    =>  read one message in the queue
	 * timeout 0     =>  read all messages in the queue
	 * timeout other =>  read all messages in the queue until timeout expired (at least all one time)
	 */

	bool exit = false;

	while (!exit)
	{
		// process all messages in the queue
		while (dataAvailable ())
		{
			processOneMessage ();
			if (timeout == -1)
			{
				break;
			}
		}

		// enable multithreading on windows :-/
		nlSleep (1);

		// need to exit?
		if (timeout == -1 || timeout == 0 || (sint32)(CTime::getLocalTime()-t0) > timeout)
		{
			exit = true;
		}
	}

/* old message processing
	while (dataAvailable ()) // can be interrupted by "break"
	{
		processOneMessage ();

		// Test if we read more data
		if ( timeout == -1 )
		{
			break; // only one read already done => exit
		}
		else if ( timeout != -1 )
		{
			if ( (sint32)(CTime::getLocalTime()-t0) > timeout )
			{
				break; // the timeout has expired
			}
		}
	}
*/
}


const	CInetAddress& CCallbackNetBase::hostAddress (TSockId hostid)
{
	// should never called
	nlstop;
	static CInetAddress tmp;
	return tmp;
}

void	CCallbackNetBase::setOtherSideAssociations (const char **associationarray, NLMISC::CStringIdArray::TStringId arraysize)
{
	nldebug ("L3NB_ASSOC: setOtherSideAssociations() sets %d association strings", arraysize);

	for (sint i = 0; i < arraysize; i++)
	{
		nldebug ("L3NB_ASSOC:  association '%s' -> %d", associationarray[i], i);
		getSIDA().addString (associationarray[i], i);
	}
}

void	CCallbackNetBase::displayAllMyAssociations ()
{
	_OutputSIDA.display ();
}

void	CCallbackNetBase::authorizeOnly (const char *callbackName, TSockId hostid)
{
	nldebug ("L3NB: authorizeOnly (%s, %s)", callbackName, hostid->asString().c_str());

	hostid = getSockId (hostid);
	
	nlassert (hostid != NULL);
	if (callbackName == NULL)
		hostid->AuthorizedCallback = "";
	else
		hostid->AuthorizedCallback = callbackName;
}

} // NLNET
