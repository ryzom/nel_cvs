/** \file login_client.cpp
 * CLoginClient is the interface used by the client to identifies itself to the login_sytem and
 * connects to the shard.
 *
 * $Id: login_client.cpp,v 1.15 2002/08/21 09:44:28 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "nel/misc/system_info.h"

#include "nel/net/callback_client.h"

#include "nel/net/login_cookie.h"
#include "nel/net/login_client.h"

#include "nel/net/udp_sock.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {

CLoginClient::TShardList CLoginClient::ShardList;

string CLoginClient::_GfxInfos;

CCallbackClient *CLoginClient::_CallbackClient = NULL;


// Callback for answer of the login password.
bool VerifyLoginPassword;
string VerifyLoginPasswordReason;
void cbVerifyLoginPassword (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S04: receive the "VLP" message from LS
	//

	uint8 ok;
	msgin.serial (ok);
	if (ok)
	{
		uint32 nbshard = 0;
		msgin.serial (nbshard);

		CLoginClient::ShardList.clear ();
		VerifyLoginPasswordReason = "";

		// get the shard list
		for (uint i = 0; i < nbshard; i++)
		{
			CLoginClient::CShardEntry se;
			msgin.serial (se.ShardName);
			msgin.serial (se.WSAddr);
			CLoginClient::ShardList.push_back (se);
		}		
	}
	else
	{
		msgin.serial (VerifyLoginPasswordReason);
	}
	VerifyLoginPassword = true;
}

// Callback for answer of the request shard
bool ShardChooseShard;
string ShardChooseShardReason;
string ShardChooseShardAddr;
CLoginCookie ShardChooseShardCookie;
void cbShardChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S11: receive "SCS" message from LS
	//

	msgin.serial (ShardChooseShardReason);
	
	if (ShardChooseShardReason.empty())
	{
		msgin.serial (ShardChooseShardCookie);
		msgin.serial (ShardChooseShardAddr);
	}
	ShardChooseShard = true;
}

static TCallbackItem CallbackArray[] =
{
	{ "VLP", cbVerifyLoginPassword },
	{ "SCS", cbShardChooseShard },
};

//

string CLoginClient::authenticate (const string &loginServiceAddr, const string &login, const string &password, uint32 clientVersion)
{
	//
	// S01: connect to the LS
	//
	try
	{
		nlassert (_CallbackClient == NULL);
		_CallbackClient = new CCallbackClient();
		_CallbackClient->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
		_CallbackClient->connect (loginServiceAddr);
	}
	catch (ESocket &e)
	{
		nlwarning ("Connection refused to LS (addr:%s): %s", loginServiceAddr.c_str(), e.what ());
		return "Connection refused to LS";
	}

	_CallbackClient->displayAllMyAssociations ();

	//
	// S02: create and send the "VLP" message
	//
	CMessage msgout (_CallbackClient->getSIDA (), "VLP");
	msgout.serial (const_cast<string&>(login));
	msgout.serial (const_cast<string&>(password));
	msgout.serial (clientVersion);

	string OS, Proc, Mem, Gfx;
	OS = CSystemInfo::getOS().c_str();
	Proc = CSystemInfo::getProc().c_str();
	Mem = CSystemInfo::getMem().c_str();
	msgout.serial (OS);
	msgout.serial (Proc);
	msgout.serial (Mem);
	msgout.serial (_GfxInfos);

	_CallbackClient->send (msgout);

	// wait the answer from the LS
	VerifyLoginPassword = false;
	while (_CallbackClient->connected() && !VerifyLoginPassword)
	{
		_CallbackClient->update ();
		nlSleep(10);
	}

	// have we received the answer?
	if (!VerifyLoginPassword)
	{
		delete _CallbackClient;
		_CallbackClient = NULL;
		return "CLoginClient::authenticate(): LS disconnects me";
	}

	if (!VerifyLoginPasswordReason.empty())
	{
		_CallbackClient->disconnect ();
		delete _CallbackClient;
		_CallbackClient = NULL;
	}

	return VerifyLoginPasswordReason;
}



// Callback for answer of the request shard
bool ShardValidate;
string ShardValidateReason;
void cbShardValidate (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S14: receive "SV" message from FES
	//

	msgin.serial (ShardValidateReason);
	ShardValidate = true;
}

static TCallbackItem FESCallbackArray[] =
{
	{ "SV", cbShardValidate },
};

string CLoginClient::confirmConnection (uint32 shardListIndex)
{
	nlassert (_CallbackClient != NULL && _CallbackClient->connected());
	nlassert (shardListIndex < ShardList.size());

	//
	// S05: send the client shard choice
	//

	// send CS
	CMessage msgout (_CallbackClient->getSIDA (), "CS");
	msgout.serial (ShardList[shardListIndex].WSAddr);
	_CallbackClient->send (msgout);

	// wait the answer
	ShardChooseShard = false;
	while (_CallbackClient->connected() && !ShardChooseShard)
	{
		_CallbackClient->update ();
		nlSleep(10);
	}

	// have we received the answer?
	if (!ShardChooseShard)
	{
		delete _CallbackClient;
		_CallbackClient = NULL;
		return "CLoginClient::connectToShard(): LS disconnects me";
	}
	else
	{
		_CallbackClient->disconnect ();
		delete _CallbackClient;
		_CallbackClient = NULL;
	}

	if (!ShardChooseShardReason.empty())
	{
		return ShardChooseShardReason;
	}

	// ok, we can try to connect to the good front end

	nlinfo("addr:%s cookie:%s", ShardChooseShardAddr.c_str(), ShardChooseShardCookie.toString().c_str());
	
	return "";
}

string CLoginClient::connectToShard (uint32 shardListIndex, CCallbackClient &cnx)
{
	nlassert (!cnx.connected());
	
	string res = confirmConnection (shardListIndex);
	if (!res.empty()) return res;

	try
	{

		//
		// S12: connect to the FES and send "SV" message to the FES
		//
		cnx.connect (CInetAddress(ShardChooseShardAddr));
		cnx.addCallbackArray (FESCallbackArray, sizeof(FESCallbackArray)/sizeof(FESCallbackArray[0]));

		cnx.displayAllMyAssociations ();

		// send the cookie
		CMessage msgout2 (cnx.getSIDA (), "SV");
		msgout2.serial (ShardChooseShardCookie);
		cnx.send (msgout2);

		// wait the answer of the connection
		ShardValidate = false;
		while (cnx.connected() && !ShardValidate)
		{
			cnx.update ();
			nlSleep(10);
		}
		
		// have we received the answer?
		if (!ShardValidate) return "FES disconnect me";
	}
	catch (ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::connectToShard (uint32 shardListIndex, CUdpSock &cnx, CLoginCookie &cookie)
{
	nlassert (!cnx.connected());
	
	string res = confirmConnection (shardListIndex);
	if (!res.empty()) return res;

	try
	{
		//
		// S12: connect to the FES. Note: In UDP mode, it's the user that have to send the cookie to the front end
		//
		cnx.connect (CInetAddress(ShardChooseShardAddr));

		cookie = ShardChooseShardCookie;
	}
	catch (ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}


} // NLNET

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/*
using namespace NLMISC;
using namespace NLNET;
using namespace std;

static uint32 ClientVersion = 1;

#include <time.h>

void main ()
{
	string res = CLoginClient::authenticate ("localhost:49999", "login", "password", ClientVersion);

	if (res.empty())
	{
		// ok
		nlinfo("auth OK");

		nlinfo("Shard list:");
		for (uint i = 0; i < CLoginClient::ShardList.size(); i++)
		{
			nlinfo(" > %s %s", CLoginClient::ShardList[i].ShardName.c_str(), CLoginClient::ShardList[i].WSAddr.c_str());
		}
		nlinfo("End shard list");

		CCallbackClient cnx;
		res = CLoginClient::connectToShard (0, cnx);

		if (res.empty())
		{
			// ok
			nlinfo("cnx OK");

			while (cnx.connected ())
			{
				cnx.update ();
			}
		}
		else
		{
			// not ok, and reason
			nlinfo("cnx FAILED: %s", res.c_str());
		}
	}
	else
	{
		// not ok, and reason
		nlinfo("auth FAILED: %s", res.c_str());
	}

	getchar ();
}
*/
