/** \file callback_net_base.cpp
 * Network engine, layer 3, base
 *
 * $Id: callback_net_base.cpp,v 1.39 2002/08/22 15:04:21 lecroart Exp $
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

#include "stdnet.h"

#include "nel/misc/string_id_array.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/net/buf_sock.h"
#include "nel/net/callback_net_base.h"

#ifdef USE_MESSAGE_RECORDER
#ifdef NL_OS_WINDOWS
#pragma message ( "NeL Net layer 3: message recorder enabled" )
#endif // NL_OS_WINDOWS
#include "nel/net/message_recorder.h"
#else
#ifdef NL_OS_WINDOWS
#pragma message ( "NeL Net layer 3: message recorder disabled" )
#endif // NL_OS_WINDOWS
#endif


using namespace std;
using namespace NLMISC;

namespace NLNET {


/*
 *
 */
void cbnbMessageRecvAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// receive a new message association

	CStringIdArray::TStringId size;
	msgin.serial (size);

	nldebug ("LNETL3NB_ASSOC: The other side gave me %d association strings", size);

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

		nldebug ("LNETL3NB_ASSOC:  association '%s' -> %d", name.c_str (), id);
		netbase.getSIDA().addString (name, id);
	}
}


/*
 * the other side want to know some of my association, send them!
 */
void cbnbMessageAskAssociations (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	CMessage msgout (netbase.getSIDA(), "RA");
	CStringIdArray::TStringId size;
	msgin.serial (size);

	nldebug ("LNETL3NB_ASSOC: The other side want %d string associations", size);

	msgout.serial (size);

	for (sint i = 0; i < size; i++)
	{
		string name;
		msgin.serial (name);
		nldebug ("LNETL3NB_ASSOC:  sending association '%s' -> %d", name.c_str (), netbase._OutputSIDA.getId(name));

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


/*
 * Disconnection callback
 */
void cbnbNewDisconnection (TSockId from, void *data)
{
	nlassert (data != NULL);
	CCallbackNetBase *base = (CCallbackNetBase *)data;

	nldebug("LNETL3NB: cbnbNewDisconnection()");

#ifdef USE_MESSAGE_RECORDER
	// Record or replay disconnection
	base->noticeDisconnection( from );
#endif
	
	// Call the client callback if necessary
	if (base->_DisconnectionCallback != NULL)
		base->_DisconnectionCallback (from, base->_DisconnectionCbArg);
}


/*
 * Constructor
 */
CCallbackNetBase::CCallbackNetBase(  TRecordingState rec, const string& recfilename, bool recordall ) :
	_FirstUpdate (true), _DisconnectionCallback(NULL), _DisconnectionCbArg(NULL)
#ifdef USE_MESSAGE_RECORDER
	, _MR_RecordingState(rec), _MR_UpdateCounter(0)
#endif
{
	_ThreadId = getThreadId ();
	_NewDisconnectionCallback = cbnbNewDisconnection;

	_BytesSent = 0;
	_BytesReceived = 0;

	createDebug(); // for addNegativeFilter to work even in release and releasedebug modes

	// add the callback needed to associate messages with id
	addCallbackArray (cbnbMessageAssociationArray, sizeof (cbnbMessageAssociationArray) / sizeof (cbnbMessageAssociationArray[0]));

#ifdef USE_MESSAGE_RECORDER
	switch ( _MR_RecordingState )
	{
	case Record :
		_MR_Recorder.startRecord( recfilename, recordall );
		break;
	case Replay :
		_MR_Recorder.startReplay( recfilename );
		break;
	default:;
		// No recording
	}
#endif
}


/*
 *	Append callback array with the specified array
 */
void CCallbackNetBase::addCallbackArray (const TCallbackItem *callbackarray, CStringIdArray::TStringId arraysize)
{
	checkThreadId ();

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
//	nldebug ("LNETL3NB_CB: Added %d callback Now, there's %d callback associated with message type", arraysize, _CallbackArray.size ());
}


/*
 * processOneMessage()
 */
void CCallbackNetBase::processOneMessage ()
{
	checkThreadId ();

	H_AUTO (CCallbackNetBase_processOneMessage);

	CMessage msgin (_OutputSIDA, "", true);
	TSockId tsid;
	receive (msgin, &tsid);

	_BytesReceived += msgin.length ();
	
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

	TMsgCallback	cb = NULL;
	if (pos < 0 || pos >= (sint16) _CallbackArray.size ())
	{
		if (_DefaultCallback == NULL)
		{
			nlwarning ("LNETL3NB_CB: Callback %s not found in _CallbackArray", msgin.toString().c_str());
		}
		else
		{
			cb = _DefaultCallback;
		}
	}
	else
	{
		cb = _CallbackArray[pos].Callback;
	}

	TSockId realid = getSockId (tsid);

	if (!realid->AuthorizedCallback.empty() && msgin.getName() != realid->AuthorizedCallback)
	{
		nlwarning ("LNETL3NB_CB: %s try to call the callback %s but only %s is authorized. Disconnect him!", tsid->asString().c_str(), msgin.toString().c_str(), tsid->AuthorizedCallback.c_str());
		disconnect (tsid);
	}
	else if (cb == NULL)
	{
		nlwarning ("LNETL3NB_CB: Callback %s is NULL, can't call it", msgin.toString().c_str());
	}
	else
	{
		nldebug ("LNETL3NB_CB: Calling callback (%s)%s", msgin.getName().c_str(), (cb==_DefaultCallback)?" DEFAULT_CB":"");
		cb(msgin, realid, *this);
	}
	
/*
	if (pos < 0 || pos >= (sint16) _CallbackArray.size ())
	{
		if (_DefaultCallback == NULL)
			nlwarning ("LNETL3NB_CB: Callback %s not found in _CallbackArray", msgin.toString().c_str());
		else
		{
			// ...
		}
	}
	else
	{
		TSockId realid = getSockId (tsid);

		if (!realid->AuthorizedCallback.empty() && msgin.getName() != realid->AuthorizedCallback)
		{
			nlwarning ("LNETL3NB_CB: %s try to call the callback %s but only %s is authorized. Disconnect him!", tsid->asString().c_str(), msgin.toString().c_str(), tsid->AuthorizedCallback.c_str());
			disconnect (tsid);
		}
		else if (_CallbackArray[pos].Callback == NULL)
		{
			nlwarning ("LNETL3NB_CB: Callback %s is NULL, can't call it", msgin.toString().c_str());
		}
		else
		{
			nldebug ("LNETL3NB_CB: Calling callback (%s)", _CallbackArray[pos].Key);
			_CallbackArray[pos].Callback (msgin, realid, *this);
		}
	}
*/
}



/*
 * baseUpdate
 * Recorded : YES
 * Replayed : YES
 */
void CCallbackNetBase::baseUpdate (sint32 timeout)
{
	checkThreadId ();

//	H_AUTO (CCallbackNetBase_baseUpdate);

	nlassert( timeout >= -1 );
	TTime t0 = CTime::getLocalTime();

	//
	// The first time, we init time counters
	//
	if (_FirstUpdate)
	{
//		nldebug("LNETL3NB: First update()");
		_FirstUpdate = false;
		_LastUpdateTime = t0;
		_LastMovedStringArray = t0;
	}

	//
	// Every 1 seconds if we have new unknown association, we ask them to the other side
	//
	if (t0 - _LastUpdateTime >  1000)
	{
//		nldebug("LNETL3NB: baseUpdate()");
		_LastUpdateTime = t0;

		const set<string> &sa = _InputSIDA.getNeedToAskedStringArray ();
		if (!sa.empty ())
		{
			CMessage msgout (_InputSIDA, "AA");
			//nlassert (sa.size () < 65536); // no size limit anymore
			CStringIdArray::TStringId size = sa.size ();
			nldebug ("LNETL3NB_ASSOC: I need %d string association, ask them to the other side", size);
			msgout.serial (size);
			for (set<string>::iterator it = sa.begin(); it != sa.end(); it++)
			{
				nldebug ("LNETL3NB_ASSOC:  what is the id of '%s'?", (*it).c_str ());
				string str(*it);
				msgout.serial (str);
			}
			// send the message to the other side
			send (msgout, 0);
			_InputSIDA.moveNeedToAskToAskedStringArray();
			_LastMovedStringArray = t0;
		}
	}

	//
	// Every 60 seconds if we have not answered association, we ask again to get them!
	//
	if (!_InputSIDA.getAskedStringArray().empty() && t0 - _LastMovedStringArray > 60000)
	{
		// we didn't have an answer for the association, resend them
		const set<string> sa = _InputSIDA.getAskedStringArray ();
		CMessage msgout (_InputSIDA, "AA");
		//nlassert (sa.size () < 65536); // no size limit anymore
		CStringIdArray::TStringId size = sa.size ();
		nldebug ("LNETL3NB_ASSOC: client didn't answer my asked association, retry! I need %d string association, ask them to the other side", size);
		msgout.serial (size);
		for (set<string>::iterator it = sa.begin(); it != sa.end(); it++)
		{
			nldebug ("LNETL3NB_ASSOC:  what is the id of '%s'?", (*it).c_str ());
			string str(*it);
			msgout.serial (str);
		}
		// sends the message to the other side
		send (msgout, 0);
		_LastMovedStringArray = t0;
	}

	/*
	 * timeout -1    =>  read one message in the queue or nothing if no message in queue
	 * timeout 0     =>  read all messages in the queue
	 * timeout other =>  read all messages in the queue until timeout expired (at least all one time)
	 */

	bool exit = false;

	while (!exit)
	{
		nlwarning ("avant dataava");
		// process all messages in the queue
		while (dataAvailable ())
		{
			processOneMessage ();
			if (timeout == -1)
			{
				exit = true;
				break;
			}
		}
		nlwarning ("apres dataava");

		// need to exit?
		if (timeout == 0 || (sint32)(CTime::getLocalTime() - t0) > timeout)
		{
			exit = true;
		}
		else
		{
			// enable multithreading on windows :-/
			H_AUTO (CCallbackNetBase_baseUpdate_nlSleep);
			nlSleep (10);
		}
	}

#ifdef USE_MESSAGE_RECORDER
	_MR_UpdateCounter++;
#endif

}


const	CInetAddress& CCallbackNetBase::hostAddress (TSockId hostid)
{
	// should never be called
	nlstop;
	static CInetAddress tmp;
	return tmp;
}

void	CCallbackNetBase::setOtherSideAssociations (const char **associationarray, NLMISC::CStringIdArray::TStringId arraysize)
{
	checkThreadId ();

	nldebug ("LNETL3NB_ASSOC: setOtherSideAssociations() sets %d association strings", arraysize);

	for (sint i = 0; i < arraysize; i++)
	{
		nldebug ("LNETL3NB_ASSOC:  association '%s' -> %d", associationarray[i], i);
		getSIDA().addString (associationarray[i], i);
	}
}

void	CCallbackNetBase::displayAllMyAssociations ()
{
	checkThreadId ();

	_OutputSIDA.display ();
}

void	CCallbackNetBase::authorizeOnly (const char *callbackName, TSockId hostid)
{
	checkThreadId ();

	nldebug ("LNETL3NB: authorizeOnly (%s, %s)", callbackName, hostid->asString().c_str());

	hostid = getSockId (hostid);
	
	nlassert (hostid != InvalidSockId);

	hostid->AuthorizedCallback = (callbackName == NULL)?"":callbackName;
}


#ifdef USE_MESSAGE_RECORDER

/*
 * Replay dataAvailable() in replay mode
 */
bool CCallbackNetBase::replayDataAvailable()
{
	nlassert( _MR_RecordingState == Replay );

	if ( _MR_Recorder.ReceivedMessages.empty() )
	{
		// Fill the queue of received messages related to the present update
		_MR_Recorder.replayNextDataAvailable( _MR_UpdateCounter );
	}

	return replaySystemCallbacks();
}


/*
 * Record or replay disconnection
 */
void CCallbackNetBase::noticeDisconnection( TSockId hostid )
{
	nlassert (hostid != InvalidSockId);	// invalid hostid
	if ( _MR_RecordingState != Replay )
	{
		if ( _MR_RecordingState == Record )
		{
			// Record disconnection
			CMessage emptymsg;
			_MR_Recorder.recordNext( _MR_UpdateCounter, Disconnecting, hostid, emptymsg );
		}
	}
	else
	{
		// Replay disconnection
		hostid->disconnect( false );
	}
}

#endif // USE_MESSAGE_RECORDER



/*
 * checkThreadId
 */
void CCallbackNetBase::checkThreadId () const
{
/*	some people use this class in different thread but with a mutex to be sure to have
	no concurent access
	if (getThreadId () != _ThreadId)
	{
		nlerror ("You try to access to the same CCallbackClient or CCallbackServer with 2 differents thread (%d and %d)", _ThreadId, getThreadId());
	}
*/
}


} // NLNET

