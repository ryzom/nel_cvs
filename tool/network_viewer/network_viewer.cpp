/** \file network_viewer.cpp
 * network_viewer prototype
 *
 * $Id: network_viewer.cpp,v 1.11 2001/04/17 15:59:02 lecroart Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/net/msg_socket.h"
#include "nel/net/unitime.h"
#include "nel/net/net_displayer.h"

#include "nel/3d/driver.h"
#include "nel/3d/camera.h"
#include "nel/3d/nelu.h"
#include "nel/3d/dru.h"
#include "nel/3d/font_manager.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/computed_string.h"

#include <string>
#include <vector>

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace NL3D;

const sint32	Width = 800, Height = 600;

// store specific user information
NLMISC::CFileDisplayer Fd ("network_viewer.log");

#ifdef NL_OS_WINDOWS
CFontGenerator fontGen("\\\\server\\code\\fonts\\arialuni.ttf");
#else
CFontGenerator fontGen("arialuni.ttf");
#endif

CScene *scene = &CNELU::Scene;
CFontManager fontManager;

double ZoomX = 1.0, PosX = 0.0, ZoomY = 1.0, FontZoom = 1.0;
sint64 PosY;
bool Automat = true;

CMsgSocket *client;

static const double HostFontSize = 30.0;
static const double MsgFontSize = 10.0;

sint32 SelectedLine = -1;

#include "nel/net/service.h"

const char NLNET::IService::_Name[] = "toto";
const uint16 NLNET::IService::_DefaultPort = 12345;
sint16 CallbackArraySize = 0;
NLNET::TCallbackItem CallbackArray [] = { NULL, };

/*

Send:
@@DATE@HOTE_SRC(IP/PORT)@NUM@PROCESS(SERVICE/PID)@HOTE_DEST(IP/PORT)@NOM_MESSAGE@TAILLE_MSG@

Receive:
##DATE#HOTE_SRC(IP/PORT)#NUM#PROCESS(SERVICE/PID)#HOTE_DEST(IP/PORT)#

*/

struct CMsg
{
	CMsg()
	{
		cnxsrc = cnxdst = -1;
		procsrc = procdst = -1;
		hostsrc = hostdst = -1;
		timesrc = timedst = 0;
		num = -1;
		name = "?";
		cs = false;
		size = 0;
	}

	sint32	cnxsrc, cnxdst;
	sint32	procsrc, procdst;
	sint32	hostsrc, hostdst;
	TTime	timesrc, timedst;
	sint32	num;
	string	name;
	CComputedString csname;
	bool	cs;

	sint32	size;

	void	drawArrow ();
};

struct CObject
{
	CObject(string name, sint32 pos) : name(name), cs(false), pos(pos) { }

	string	name;
	CComputedString csname;
	bool	cs;
	
	sint32	pos;
};

vector<CMsg>	Msgs;
vector<CObject>	Cnxs;
vector<CObject>	Procs;
vector<CObject>	Hosts;

vector<CObject>	*View = &Cnxs;
sint32 SelectedView = 0;

void switchView ()
{
	SelectedView = (SelectedView+1)%3;
	SelectedLine = -1;
	switch (SelectedView)
	{
	case 0: View = &Cnxs; break;
	case 1: View = &Procs; break;
	case 2: View = &Hosts; break;
	}
}

sint32 findMax (vector<CObject> &v)
{
	sint32 maxpos = 0;
	for (sint32 i = 0; i < (sint32) v.size (); i++)
	{
		if (v[i].pos > maxpos) maxpos = v[i].pos;
	}
	return maxpos;
}

CMsg &findOrAddMsg (sint32 cnxsrc, sint32 cnxdst, sint32 num, bool send)
{
	for (sint32 i = 0; i < (sint32) Msgs.size (); i++)
	{
		if (Msgs[i].cnxsrc == cnxsrc && Msgs[i].cnxdst == cnxdst && Msgs[i].num == num)
		{
			if (send && Msgs[i].timesrc != 0)
			{
				// found it, return it
				return Msgs[i];
			}
			else if (!send && Msgs[i].timedst != 0)
			{
				// found it, return it
				return Msgs[i];
			}
		}
	}
	Msgs.push_back (CMsg());
	// add it and return it
	return Msgs[Msgs.size () - 1];
}

sint32 findOrAddCnx (string cnxsrcname)
{
	sint32 maxpos = 0;
	for (sint32 i = 0; i < (sint32) Cnxs.size (); i++)
	{
		if (Cnxs[i].name == cnxsrcname)
		{
			// found it, return it
			return i;
		}
		if (Cnxs[i].pos > maxpos) maxpos = Cnxs[i].pos;
	}
	Cnxs.push_back (CObject(cnxsrcname, maxpos+1));
	// add it and return it
	return Cnxs.size () - 1;
}

sint32 findOrAddProc (string procsrcname)
{
	sint32 maxpos = 0;
	for (sint32 i = 0; i < (sint32) Procs.size (); i++)
	{
		if (Procs[i].name == procsrcname)
		{
			// found it, return it
			return i;
		}
		if (Procs[i].pos > maxpos) maxpos = Procs[i].pos;
	}
	Procs.push_back (CObject(procsrcname, maxpos+1));
	// add it and return it
	return Procs.size () - 1;
}

sint32 findOrAddHost (string hostsrcname)
{
	sint32 maxpos = 0;
	for (sint32 i = 0; i < (sint32) Hosts.size (); i++)
	{
		if (Hosts[i].name == hostsrcname)
		{
			// found it, return it
			return i;
		}
		if (Hosts[i].pos > maxpos) maxpos = Hosts[i].pos;
	}
	Hosts.push_back (CObject(hostsrcname, maxpos+1));
	// add it and return it
	return Hosts.size () - 1;
}

void finalize (CMsg &m)
{
	nlinfo ("* %"NL_I64"d '%s' --'%s'--> '%s' %"NL_I64"d", m.timesrc, Cnxs[m.cnxsrc].name.c_str(), m.name.c_str(), Cnxs[m.cnxdst].name.c_str(), m.timedst);
	for (sint32 i = 0; i < (sint32) Msgs.size (); i++)
	{
		CMsg &m2 = Msgs[i];
		if (&m != &m2)
		{
			if (m.timesrc == m2.timesrc && m.timedst == m2.timedst && m.cnxsrc == m2.cnxsrc && m.cnxdst == m2.cnxdst)
			{
				m2.name += ", " + m.name;
				m2.cs = false;
				Msgs.erase (Msgs.begin()+i);
				return;
			}
		}
	}
}

// set a send message (@)
void setMessage (TTime timesrc, string cnxsrcname, sint32 num, string procsrcname, string cnxdstname, string name, sint32 size)
{
	sint32 cnxsrc = findOrAddCnx (cnxsrcname);
	sint32 cnxdst = findOrAddCnx (cnxdstname);
	CMsg &m = findOrAddMsg(cnxsrc, cnxdst, num, false);
	m.timesrc = timesrc;
	m.cnxsrc = cnxsrc;
	m.num = num;
	m.procsrc = findOrAddProc (procsrcname);
	m.cnxdst = cnxdst;
	m.name = name;
	m.cs = false;
	m.size = size;

	string hostname;
	sint32 i;
	for (i=0, hostname=""; cnxsrcname[i]!='/'; hostname+=cnxsrcname[i++])	;
	m.hostsrc = findOrAddHost (hostname);

	for (i=0, hostname=""; cnxdstname[i]!='/'; hostname+=cnxdstname[i++])	;
	m.hostdst = findOrAddHost (hostname);

	if (m.timedst != 0) finalize (m);
}

// set a receive message (#)
void setMessage (TTime timedst, string cnxsrcname, sint32 num, string procdstname, string cnxdstname)
{
	sint32 cnxsrc = findOrAddCnx (cnxsrcname);
	sint32 cnxdst = findOrAddCnx (cnxdstname);
	CMsg &m = findOrAddMsg(cnxsrc, cnxdst, num, true);
	m.timedst = timedst;
	m.cnxsrc = cnxsrc;
	m.num = num;
	m.procdst = findOrAddProc (procdstname);
	m.cnxdst = cnxdst;

	string hostname;
	sint32 i;
	for (i=0, hostname=""; cnxsrcname[i]!='/'; hostname+=cnxsrcname[i++])	;
	m.hostsrc = findOrAddHost (hostname);

	for (i=0, hostname=""; cnxdstname[i]!='/'; hostname+=cnxdstname[i++])	;
	m.hostdst = findOrAddHost (hostname);

	if (m.timesrc != 0) finalize (m);
}

void RecomputeFont()
{
	// invalidate all strings
	sint32 i;
	for (i = 0; i < (sint32) Msgs.size (); i++)
	{
		Msgs[i].cs = false;
	}

	for (i = 0; i < (sint32) Cnxs.size (); i++)
	{
		Cnxs[i].cs = false;
	}

	for (i = 0; i < (sint32) Procs.size (); i++)
	{
		Procs[i].cs = false;
	}

	for (i = 0; i < (sint32) Hosts.size (); i++)
	{
		Hosts[i].cs = false;
	}
}

double vx2sx (double px)
{
	// utiliser maxpos au lieu de size
	sint32 nb = findMax (*View);
	if (nb > 6) nb = 6;
	double dx = 1.0 / ((double) nb+1);
	double cx = px - (double) nb/2.0;
	return 0.5 + cx * dx * ZoomX + PosX;
}

double sx2vx (double px)
{
	// utiliser maxpos au lieu de size
	sint32 nb = findMax (*View);
	if (nb > 6) nb = 6;
	double dx = 1.0 / ((double) nb+1);
	return (px - 0.5 - PosX) / (dx * ZoomX) + (double) nb/2.0;
}

double vy2sy (TTime py)
{
	return (sint64)(py-PosY)/(-5000.0/ZoomY);
}

sint64 sy2vy (double py)
{
	sint64 t = (sint64)(py * -5000.0/ZoomY);
	return (sint64)(t);
}

sint32 findLine (double x, double y)
{
	sint32 neededpos = (sint32)(sx2vx (x) + 0.5);
	for (sint32 i = 0; i < (sint32) (*View).size (); i++)
	{
		if ((*View)[i].pos == neededpos) return i;
	}
	return -1;
}


void CMsg::drawArrow()
{
	CVector v0;
	CVector v1;

	sint32 viewsrc, viewdst;

	// manage if some info needed (when the @ and # are not receive)
	switch (SelectedView)
	{
	case 0: viewsrc = cnxsrc; viewdst = cnxdst; break;
	case 1: viewsrc = procsrc; viewdst = procdst; break;
	case 2: viewsrc = hostsrc; viewdst = hostdst; break;
	}

	if (viewsrc != -1)
	{
		v0.x = (float)(vx2sx((*View)[viewsrc].pos)*1.33);
	}
	else
	{
		v0.x = (float)(vx2sx((*View)[viewdst].pos+0.3)*1.33);
	}

	TTime ut = CUniTime::getUniTime ();
	if (timesrc != 0)
	{
		v0.y = (float)vy2sy(timesrc);
	}
	else
	{
		if (ut > timedst + 2000)
			v0.y = (float)vy2sy(timedst);
		else
			v0.y = (float)vy2sy(ut);
	}

	if (viewdst != -1)
	{
		v1.x = (float)(vx2sx((*View)[viewdst].pos)*1.33);
	}
	else
	{
		v1.x = (float)(vx2sx((*View)[viewsrc].pos+0.3)*1.33);
	}

	if (timedst != 0)
	{
		v1.y = (float)vy2sy(timedst);
	}
	else
	{
		if (ut > timesrc + 2000)
			v1.y = (float)vy2sy(timesrc);
		else
			v1.y = (float)vy2sy(ut);
	}

	v0.z = v1.z = 0.0f;


	// clip on the window
	double minx = (v0.x < v1.x) ? v0.x : v1.x;
	double maxx = (v0.x > v1.x) ? v0.x : v1.x;
	double miny = (v0.y < v1.y) ? v0.y : v1.y;
	double maxy = (v0.y > v1.y) ? v0.y : v1.y;
	if (maxx<0.0 || minx>1.33 || maxy<0.0 || miny>1.0) return;

	CVector dv = v1 - v0;
	CVector v2, v3, dv1, dv2;

	CMatrix mat;
	mat.identity();
	mat.rotateZ((float) (20.0*Pi/180.0));
	dv1= mat*dv;
	dv1/=15;
	mat.identity();
	mat.rotateZ((float) (-20.0*Pi/180.0));
	dv2= mat*dv;
	dv2/=15;
	v2= v1-dv1;
	v3= v1-dv2;
	v0.x/=1.33f;
	v1.x/=1.33f;
	v2.x/=1.33f;
	v3.x/=1.33f;

	CRGBA col;
	if(viewdst == -1) col = CRGBA (255,0,0,0);
	else if(timesrc == 0) col = CRGBA (0,0,255,0);
	else if(timedst == 0) col = CRGBA (0,255,0,0);
	else col = CRGBA (255,255,255,0);

	CDRU::drawLine (v0.x, v0.y, v1.x, v1.y, *CNELU::Driver, col);
	CDRU::drawLine (v2.x, v2.y, v1.x, v1.y, *CNELU::Driver, col);
	CDRU::drawLine (v3.x, v3.y, v1.x, v1.y, *CNELU::Driver, col);

	v2= v0+(v1-v0)/5;
	if (!cs)
	{
		cs = true;
		fontManager.computeString(name, &fontGen, CRGBA(255,128,75), (uint32) (MsgFontSize*ZoomX), CNELU::Driver, csname);
	}

	csname.render2D(*scene->getDriver(), v2.x, v2.y, CComputedString::MiddleMiddle);
}

void render ()
{
	for (sint32 i = 0; i < (sint32) (*View).size (); i++)
	{
		double pos = vx2sx((*View)[i].pos);
		CDRU::drawLine ((float) pos, 0.0, (float) pos, 1.0, *CNELU::Driver);

		if (!(*View)[i].cs)
		{
			fontManager.computeString((*View)[i].name, &fontGen, CRGBA(255,255,128), (uint32)(HostFontSize*FontZoom), CNELU::Driver, (*View)[i].csname);
			(*View)[i].cs = true;
		}

		if (SelectedLine == i)
		{
			(*View)[i].csname.render2D(*scene->getDriver(), (float) pos, 1.0, CComputedString::BottomRight, 1.5, 1.5, (float) (-90.0*Pi/180.0));
		}
		else
		{
			(*View)[i].csname.render2D(*scene->getDriver(), (float) pos, 1.0, CComputedString::BottomRight, 1.0, 1.0, (float) (-90.0*Pi/180.0));
		}
	}

	for (sint32 j = 0; j < (sint32) Msgs.size (); j++)
	{
		Msgs[j].drawArrow ();
	}
}

void cbProcessReceivedMsg( CMessage& message, TSenderId from );


class CWCallback : public IEventListener
{
	virtual void operator ()(const CEvent& event)
	{
		CEventMouseWheel &ec = (CEventMouseWheel &) event;
		if (ec.Direction)
		{
			ZoomX += 0.1;
			ZoomY *= 1.5;
			FontZoom = ZoomX;
			if (FontZoom > 1.0) FontZoom = 1.0;
			else if (FontZoom < 0.1) FontZoom = 0.1;
		}
		else
		{
			if (ZoomX > 0.15)
			{
				ZoomX -= 0.1;
				ZoomY /= 1.5;
				FontZoom = ZoomX;
				if (FontZoom > 1.0) FontZoom = 1.0;
				else if (FontZoom < 0.1) FontZoom = 0.1;
			}
		}
		RecomputeFont ();
	}
};

class CKCallback : public IEventListener
{
	virtual void operator ()(const CEvent& event)
	{
		CEventKeyDown &ec = (CEventKeyDown &) event;

		static int nums=0, numr=0;
		static int nums2=100, numr2=100;

		if (ec.Key == KeySPACE)
			Automat = !Automat;
		else if (ec.Key == KeyP)
			switchView ();
		else if (ec.Key == KeyA)
		{
			// send fake
			CMessage msgout;
			char str2[1024];
			nldebug("'%"NL_I64"d'", CUniTime::getUniTime());
		     
			sprintf(str2, "@@%"NL_I64"d@321.321.321.321/1000@%d@server@123.123.123.123/2001@TEST@10@\n", CUniTime::getUniTime (), nums++);
			string str = str2;
			msgout.serial (str);
			CMessage msgin( "C", true );
			msgin.fill( msgout.buffer(), msgout.length() );
			cbProcessReceivedMsg (msgin, 0);
		}
		else if (ec.Key == KeyB)
		{
			// send fake
			CMessage msgout;
			char str2[1024];
			sprintf(str2, "##%"NL_I64"d#321.321.321.321/1000#%d#client1#123.123.123.123/2001#\n", CUniTime::getUniTime (), numr++);
			string str = str2;
			msgout.serial (str);
			CMessage msgin( "C", true );
			msgin.fill( msgout.buffer(), msgout.length() );
			cbProcessReceivedMsg (msgin, 0);
		}
		else if (ec.Key == KeyC)
		{
			// send fake
			CMessage msgout;
			char str2[1024];
			sprintf(str2, "@@%"NL_I64"d@321.321.321.321/1000@%d@server@123.123.123.123/2002@TEST2@10@\n", CUniTime::getUniTime (), nums2++);
			string str = str2;
			msgout.serial (str);
			CMessage msgin( "C", true );
			msgin.fill( msgout.buffer(), msgout.length() );
			cbProcessReceivedMsg (msgin, 0);
		}
		else if (ec.Key == KeyD)
		{
			// send fake
			CMessage msgout;
			char str2[1024];
			sprintf(str2, "##%"NL_I64"d#321.321.321.321/1000#%d#client2#123.123.123.123/2002#\n", CUniTime::getUniTime (), numr2++);
			string str = str2;
			msgout.serial (str);
			CMessage msgin( "C", true );
			msgin.fill( msgout.buffer(), msgout.length() );
			cbProcessReceivedMsg (msgin, 0);
		}
		else if (ec.Key == KeyLEFT)
		{
			if (SelectedLine != -1)
			{
				(*View)[SelectedLine].pos--;
			}
		}
		else if (ec.Key == KeyRIGHT)
		{
			if (SelectedLine != -1)
			{
				(*View)[SelectedLine].pos++;
			}
		}
		else if (ec.Key == KeyUP)
		{
			PosY += sy2vy (0.1);
		}
		else if (ec.Key == KeyDOWN)
		{
			PosY -= sy2vy (0.1);
		}
		else if (ec.Key == KeyEND)
		{
			// put the selected line to the end
			if (SelectedLine != -1)
			{
				sint32 maxpos = findMax (*View);
				(*View)[SelectedLine].pos = maxpos+1;
			}
		}
		else if (ec.Key == KeyDELETE)
		{
			// put the selected line to the end
			if (SelectedLine != -1)
			{
				for (sint32 i = 0; i < (sint32) Msgs.size (); i++)
				{
					switch (SelectedView)
					{
					case 0:
						if (Msgs[i].cnxsrc == SelectedLine) Msgs[i].cnxsrc = -1;
						if (Msgs[i].cnxdst == SelectedLine) Msgs[i].cnxdst = -1;
						break;
					case 1:
						if (Msgs[i].procsrc == SelectedLine) Msgs[i].procsrc = -1;
						if (Msgs[i].procdst == SelectedLine) Msgs[i].procdst = -1;
						break;
					case 2:
						if (Msgs[i].hostsrc == SelectedLine) Msgs[i].hostsrc = -1;
						if (Msgs[i].hostdst == SelectedLine) Msgs[i].hostdst = -1;
						break;
					}
				}
				(*View).erase ((*View).begin()+SelectedLine);
				SelectedLine = -1;
			}
		}
		else if (ec.Key == KeyADD)
		{
			ZoomY *= 1.5;
		}
		else if (ec.Key == KeySUBTRACT)
		{
			ZoomY /= 1.5;
		}
		else if (ec.Key == KeyF || ec.Key == KeyG)
		{
		  if (ec.Key==KeyF)
		    {
		      ZoomX += 0.1;
		      ZoomY *= 1.5;
		      FontZoom = ZoomX;
		      if (FontZoom > 1.0) FontZoom = 1.0;
		      else if (FontZoom < 0.1) FontZoom = 0.1;
		    }
		  else
		    {
		      if (ZoomX > 0.15)
			{
			  ZoomX -= 0.1;
			  ZoomY /= 1.5;
			  FontZoom = ZoomX;
			  if (FontZoom > 1.0) FontZoom = 1.0;
			  else if (FontZoom < 0.1) FontZoom = 0.1;
			}
		    }
		  RecomputeFont ();
		}
	}
};

class CCCallback : public IEventListener
{
	virtual void operator ()(const CEvent& event)
	{
		static double mx, my;
		static bool down = false;

		if (event==EventMouseDownId)
		{
			CEventMouseDown &ec = (CEventMouseDown &) event;
			mx = ec.X;
			my = ec.Y;
			down = true;
		}
		else if (event==EventMouseUpId)
		{
			CEventMouseUp &ec = (CEventMouseUp &) event;
			if (ec.Button == 1 && down && mx == ec.X && my == ec.Y)
			{
				SelectedLine = findLine (mx, my);
				down = false;	
			}
			else if (ec.Button == 4)
			{
				SelectedLine = -1;
			}
		}
	}
};


class CMCallback : public IEventListener
{
	virtual void operator ()(const CEvent& event)
	{
		static bool firstclick = false;
		static double mx, my;

		CEventMouseMove &ec = (CEventMouseMove &) event;

		if (ec.Button == 1)
		{
			if (!firstclick)
			{
				firstclick = true;
				mx = ec.X;
				my = ec.Y;
			}
			else
			{
				PosX += -mx+ec.X;
				mx = ec.X;
				if (!Automat)
				{
					PosY += sy2vy (+my-ec.Y);
					my = ec.Y;
				}
			}
		}
		else
		{
			firstclick = false;
		}
	}
};


sint64 str2sint64(string &str)
{
	sint64 num = 0;
	if (str.size() == 0) return 0;
	sint i = 0;
	bool neg = false;

	if (str[i] == '-') 
	{
		neg = true;
		i++;
	}
	else if (str[i] == '+')
	{
		i++;
	}

	for (; i < (sint) str.size (); i++)
	{
		nlassert (str[i] >= '0' && str[i] <= '9');
		num *= 10;
		num += str[i] - '0';
	}
	if (neg) num = -num;
	return num;
}


/**
 * Callback for a log message
 *
 * Message expected:
 * - Log string (string)
 */
void cbProcessReceivedMsg( CMessage& message, TSenderId from )
{
	// Process received message
	string logstr;
	message.serial( logstr );

	// @@DATE@HOTE_SRC(IP/PORT)@NUM@PROCESS(SERVICE/PID)@HOTE_DEST(IP/PORT)@NOM_MESSAGE@TAILLE_MSG@
	string tok = "@@";
	uint res = logstr.find (tok);
	if (res != logstr.npos)
	{
		NLMISC::InfoLog.displayRaw( "S%s", logstr.c_str() );

		
		res += tok.size ();

		string timesrcname;
		while (logstr[res] != '@') timesrcname += logstr[res++];
		res++;

		string cnxsrcname;
		while (logstr[res] != '@') cnxsrcname += logstr[res++];
		res++;

		string numname;
		while (logstr[res] != '@') numname += logstr[res++];
		res++;

		string procsrcname;
		while (logstr[res] != '@') procsrcname += logstr[res++];
		res++;

		string cnxdstname;
		while (logstr[res] != '@') cnxdstname += logstr[res++];
		res++;

		string name;
		while (logstr[res] != '@') name += logstr[res++];
		res++;

		string sizename;
		while (logstr[res] != '@') sizename += logstr[res++];
		res++;

		sint64 timesrc = str2sint64 (timesrcname);
		if (timesrc < 0)
		{
			nlwarning ("Received a negative time!!!");
			timesrc = 0;
		}

		if (name == "") name = "<Empty>";
		if (timesrc == 0) timesrc = CUniTime::getUniTime ();
		setMessage (timesrc, cnxsrcname, atoi(numname.c_str()), procsrcname, cnxdstname, name, atoi(sizename.c_str()));
	}

	string tok2 = "##";
	res = logstr.find (tok2);
	if (res != logstr.npos)
	{
		NLMISC::InfoLog.displayRaw( "R%s", logstr.c_str() );

		res += tok.size ();

		string timedstname;
		while (logstr[res] != '#') timedstname += logstr[res++];
		res++;

		string cnxsrcname;
		while (logstr[res] != '#') cnxsrcname += logstr[res++];
		res++;

		string numname;
		while (logstr[res] != '#') numname += logstr[res++];
		res++;

		string procdstname;
		while (logstr[res] != '#') procdstname += logstr[res++];
		res++;

		string cnxdstname;
		while (logstr[res] != '#') cnxdstname += logstr[res++];
		res++;

		sint64 timedst = str2sint64 (timedstname);
		if (timedst < 0)
		{
			nlwarning ("Received a negative time!!!");
			timedst = 0;
		}

		if (timedst == 0) timedst = CUniTime::getUniTime ();
		setMessage (timedst, cnxsrcname, atoi(numname.c_str()), procdstname, cnxdstname);
	}
}

// Callback array
TCallbackItem ClientCallbacks [] =
{
	{ "LOG", cbProcessReceivedMsg },
};


/****************************************************************\
							MAIN
\****************************************************************/
void main()
{
  	try
	{
	  InitDebug();

		// init scene
		uint w = 800;
		uint h = 600;
		uint bpp = 32;
		bool windowed = true;
		NL3D::CNELU::init(w, h, CViewport(), bpp, windowed); 
		/*
		// Events management
		CNELU::EventServer.addEmitter(scene->getDriver()->getEventEmitter());
		CNELU::AsyncListener.addToServer(CNELU::EventServer);
		*/

		NLMISC::InfoLog.addDisplayer (&Fd);
		NLMISC::InfoLog.setLongInfo (true);
		NLMISC::InfoLog.setLocalHostAndService ("", "");

		// Positioning camera
		CMatrix	camera;
		camera.identity();
		camera.setPos(CVector(0,-20,0));
		CNELU::Camera->setMatrix(camera);

		// Connect to the family service

		client = new CMsgSocket ( ClientCallbacks, sizeof(ClientCallbacks)/sizeof(TCallbackItem), "LOGS" );

		// Add us to the logger forward list
		CMessage msgout ("FW");
		client->send( msgout );

		CUniTime::syncUniTimeFromService ();

		// Events management
		CNELU::EventServer.addEmitter(scene->getDriver()->getEventEmitter());
		CNELU::AsyncListener.addToServer(CNELU::EventServer);

		CWCallback cb;
		CNELU::EventServer.addListener (EventMouseWheelId, &cb);

		CMCallback cb2;
		CNELU::EventServer.addListener (EventMouseMoveId, &cb2);

		CKCallback cb3;
		CNELU::EventServer.addListener (EventKeyDownId, &cb3);

		CCCallback cb4;
		CNELU::EventServer.addListener (EventMouseDownId, &cb4);
		CNELU::EventServer.addListener (EventMouseUpId, &cb4);

		CComputedString autoString, cnxString, procString, hostString;
		fontManager.computeString("AutoScroll", &fontGen, CRGBA(255,0,0), 20, CNELU::Driver, autoString);
		fontManager.computeString("Connexion", &fontGen, CRGBA(255,128,255), 20, CNELU::Driver, cnxString);
		fontManager.computeString("Process", &fontGen, CRGBA(255,128,255), 20, CNELU::Driver, procString);
		fontManager.computeString("Host", &fontGen, CRGBA(255,128,255), 20, CNELU::Driver, hostString);

		do
		{
			CNELU::EventServer.pump();
		
			CNELU::clearBuffers(CRGBA(0,0,0));

			switch(SelectedView)
			{
			case 0: cnxString.render2D(*scene->getDriver(), 0.0f, 0.0f, CComputedString::BottomLeft); break;
			case 1: procString.render2D(*scene->getDriver(), 0.0f, 0.0f, CComputedString::BottomLeft); break;
			case 2: hostString.render2D(*scene->getDriver(), 0.0f, 0.0f, CComputedString::BottomLeft); break;
			}

			if (Automat)
			{
				autoString.render2D(*scene->getDriver(), 1.0f, 0.0f, CComputedString::BottomRight);
				PosY = CUniTime::getUniTime ();
			}

			render ();

			scene->render();
			CNELU::swapBuffers();

			client->update();
		}
		while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));

		CNELU::AsyncListener.removeFromServer(CNELU::EventServer);
		/*
		do {
		  CNELU::EventServer.pump();
		}
		while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));
		*/
		// release nelu
		NL3D::CNELU::release();
		
	}
	catch(Exception &e)
	{
	  try
	    {
		nlerror ("main(): Exception trapped: %s", e.what ());
	    }
	  catch (Exception &)
	    {
	    }
	}
}
