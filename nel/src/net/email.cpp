/** \file email.cpp
 * send email
 *
 * $Id: email.cpp,v 1.1 2002/08/23 12:17:47 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdnet.h"

#include "nel/misc/report.h"

#include "nel/net/tcp_sock.h"
#include "nel/net/email.h"

using namespace std;
using namespace NLMISC;


namespace NLNET {

static string DefaultSMTPServer, DefaultFrom, DefaultTo;

bool sendEmail (const string &smtpServer, const string &from, const string &to, const string &subject, const string &body, bool onlyCheck)
{	
	bool ok  = false;
	CTcpSock sock;
	string buffer;

	string formatedBody;
	string formatedFrom;
	string formatedTo;

	sint pos, nb250=0;
	string res;
	bool end = false;
	uint32 size, i;

	string formatedSMTPServer;
	if (smtpServer.empty())
	{
		if(DefaultSMTPServer.empty())
		{
			nlwarning ("Can't send email because no SMTPServer was provided");
			goto end;
		}
		else
		{
			formatedSMTPServer = DefaultSMTPServer;
		}
	}
	else
	{
		formatedSMTPServer = smtpServer;
	}

	sock.connect(CInetAddress(formatedSMTPServer, 25));

	if (!sock.connected())
	{
		nlwarning ("Can't connect to email server %s", formatedSMTPServer.c_str());
		goto end;
	}

	if (to.empty())
	{
		if(DefaultTo.empty())
		{
			nlwarning ("Can't send email because no To was provided");
			goto end;
		}
		else
		{
			formatedTo = DefaultTo;
		}
	}
	else
	{
		formatedTo = to;
	}

	if(from.empty())
	{
		if (DefaultFrom.empty())
		{
			formatedFrom = CInetAddress::localHost().hostName();
			formatedFrom += "@gnu.org";
		}
		else
		{
			formatedFrom = DefaultFrom;
		}
	}

	// replace \n with \r\n
	for (i = 0; i < body.size(); i++)
	{
		if (body[i] == '\n' && i > 0 && body[i-1] != '\r')
		{
			formatedBody += '\r';
		}
		formatedBody += body[i];
	}

	if(onlyCheck)
	{
		buffer =
			"HELO localhost\r\n"
			"MAIL FROM: " + formatedFrom + "\r\n"
			"RCPT TO: " + formatedTo + "\r\n"
			"QUIT\r\n";
	}
	else
	{
		buffer =
			"HELO localhost\r\n"
			"MAIL FROM: " + formatedFrom + "\r\n"
			"RCPT TO: " + formatedTo + "\r\n"
			"DATA\r\n"
			"From: " + formatedFrom + "\r\n"
			"To: " + formatedTo + "\r\n"
			"Subject: " + subject + "\r\n"
			"\r\n"
			+ formatedBody + "\r\n"
			".\r\n"
			"QUIT\r\n";
	}

	size = buffer.size();
	if (sock.send ((uint8 *)buffer.c_str(), size) != CSock::Ok)
	{
		nlwarning ("Can't send data to the server");
		goto end;
	}
	
	while (!end)
	{
		char c;
		uint32 size = 1;

		if (sock.receive((uint8*)&c, size, false) == CSock::Ok)
		{
			res += c;
		}
		else
		{
			end = true;
		}
	}

	pos = 0;
	do
	{
		pos = res.find ("250", pos+1);
		if (pos == string::npos)
			break;
		nb250++;
	}
	while (true);

	if (!onlyCheck && nb250 != 4 || onlyCheck && nb250 != 3)
	{
		nlwarning("Not enough 250 ok code, send mail failed.\nSent------------------\n%s\nReceived------------------\n%s\n", buffer.c_str(), res.c_str());
		goto end;
	}
	nldebug ("%s", res.c_str());

	ok = true;

end:
	if (sock.connected())
		sock.close ();

	return ok;
}

void setDefaultEmailParams (const std::string &smtpServer, const std::string &from, const std::string &to)
{
	DefaultSMTPServer = smtpServer;
	DefaultFrom = from;
	DefaultTo = to;
}

} // NLNET

