/** \file transport_class.cpp
 * <File description>
 *
 * $Id: transport_class.cpp,v 1.16 2003/05/14 10:24:11 lecroart Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

/*
 * Limitations: Not threadsafe, not reentrant.
 */


//
// Includes
//

#include "stdnet.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"

#include "nel/net/unified_network.h"

#include "nel/net/transport_class.h"

//
// Namespace
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace NLNET {

//
// Variables
//

uint CTransportClass::Mode = 0;	// 0=nothing 1=read 2=write 3=register

map<string, CTransportClass::CRegisteredClass>	CTransportClass::LocalRegisteredClass;	// registered class that are in my program

CTransportClass::CRegisteredClass	CTransportClass::TempRegisteredClass;

NLNET::CMessage	CTransportClass::TempMessage;

vector<CTransportClass::CRegisteredBaseProp *> CTransportClass::DummyProp;

bool CTransportClass::Init = false;


//
// Functions
//

string typeToString (CTransportClass::TProp type)
{
	string conv[] = {
		"PropUInt8", "PropUInt16", "PropUInt32", "PropUInt64",
		"PropSInt8", "PropSInt16", "PropSInt32", "PropSInt64",
		"PropBool", "PropFloat", "PropDouble", "PropString", "PropDataSetRow", "PropSheetId", "PropUKN" };
//		"PropBool", "PropFloat", "PropDouble", "PropString", "PropDataSetRow", "PropEntityId", "PropSheetId", "PropUKN" };
		
	if (type > CTransportClass::PropUKN)
		return "<InvalidType>";
	return conv[type];
}

void CTransportClass::displayDifferentClass (uint8 sid, const string &className, const vector<CRegisteredBaseProp> &otherClass, const vector<CRegisteredBaseProp *> &myClass)
{
	nlinfo ("Service with sid %u send me the TransportClass '%s' with differents properties:", sid, className.c_str());
	uint i;
	nlinfo ("  My local TransportClass is:");
	for (i = 0; i < myClass.size(); i++)
	{
		nlinfo ("    Property: %d Name: '%s' type: '%s'", i, myClass[i]->Name.c_str(), typeToString(myClass[i]->Type).c_str());
	}

	nlinfo ("  The other side TransportClass is:");
	for (i = 0; i < otherClass.size(); i++)
	{
		nlinfo ("    Property: %d Name: '%s' type: '%s'", i, otherClass[i].Name.c_str(), typeToString(otherClass[i].Type).c_str());
	}
}

void CTransportClass::registerOtherSideClass (uint8 sid, TOtherSideRegisteredClass &osrc)
{
	for (TOtherSideRegisteredClass::iterator it = osrc.begin(); it != osrc.end (); it++)
	{
		// find the class name in the map

		TRegisteredClass::iterator res = LocalRegisteredClass.find ((*it).first);
		if (res == LocalRegisteredClass.end ())
		{
			// it s a class that the other side have but not me, can't send this class
			nlwarning ("CTransportClass::registerOtherSideClass(): the other side class '%s' is not registered in my system, skip it", (*it).first.c_str());
			continue;
		}

		if (sid >= (*res).second.Instance->States.size ())
			(*res).second.Instance->States.resize (sid+1);

		(*res).second.Instance->States[sid].clear ();

		for (sint j = 0; j < (sint)(*it).second.size (); j++)
		{
			// check each prop to see the correspondance

			// try to find the prop name in the array
			uint k;
			for (k = 0; k < (*res).second.Instance->Prop.size(); k++)
			{
				if ((*it).second[j].Name == (*res).second.Instance->Prop[k]->Name)
				{
					if ((*it).second[j].Type != (*res).second.Instance->Prop[k]->Type)
					{
						nlwarning ("Property '%s' of the class '%s' have not the same type in the 2 sides (%s %s)", (*it).second[j].Name.c_str(), (*it).first.c_str(), typeToString((*it).second[j].Type).c_str(), typeToString((*res).second.Instance->Prop[k]->Type).c_str());
					}
					break;
				}
			}
			if (k == (*res).second.Instance->Prop.size())
			{
				// not found, put -1
				(*res).second.Instance->States[sid].push_back (make_pair (-1, (*it).second[j].Type));
			}
			else
			{
				// same, store the index
				(*res).second.Instance->States[sid].push_back (make_pair (k, PropUKN));
			}
		}

		// check if the version are the same
		if ((*it).second.size () != (*res).second.Instance->Prop.size ())
		{
			// 2 class don't have the same number of prop => different class => display class
			displayDifferentClass (sid, (*it).first.c_str(), (*it).second, (*res).second.Instance->Prop);
		}
		else
		{
			// check if the prop are same
			for (uint i = 0; i < (*res).second.Instance->Prop.size (); i++)
			{
				if ((*res).second.Instance->Prop[i]->Name != (*it).second[i].Name)
				{
					// different name => different class => display class
					displayDifferentClass (sid, (*it).first.c_str(), (*it).second, (*res).second.Instance->Prop);
					break;
				}
				else if ((*res).second.Instance->Prop[i]->Type != (*it).second[i].Type)
				{
					// different type => different class => display class
					displayDifferentClass (sid, (*it).first.c_str(), (*it).second, (*res).second.Instance->Prop);
					break;
				}
			}
		}
	}

	displayLocalRegisteredClass ();
}


void CTransportClass::registerClass (CTransportClass &instance)
{
	nlassert (Init);
	nlassert (Mode == 0);

	// set the mode to register
	Mode = 3;
	
	// clear the current class
	TempRegisteredClass.clear ();

	// set the instance pointer
	TempRegisteredClass.Instance = &instance;

	// fill name and props
	TempRegisteredClass.Instance->description ();

	// add the new registered class in the array
	LocalRegisteredClass[TempRegisteredClass.Instance->Name] = TempRegisteredClass;

	// set to mode none
	Mode = 0;
}

void CTransportClass::unregisterClass ()
{
	for (TRegisteredClass::iterator it = LocalRegisteredClass.begin(); it != LocalRegisteredClass.end (); it++)
	{
		for (uint j = 0; j < (*it).second.Instance->Prop.size (); j++)
		{
			delete (*it).second.Instance->Prop[j];
		}
		(*it).second.Instance->Prop.clear ();
		(*it).second.Instance = NULL;
	}
	LocalRegisteredClass.clear ();
}

void CTransportClass::displayLocalRegisteredClass (CRegisteredClass &c)
{
	nldebug ("NETTC:  > %s", c.Instance->Name.c_str());
	for (uint j = 0; j < c.Instance->Prop.size (); j++)
	{
		nldebug ("NETTC:    > %s %s", c.Instance->Prop[j]->Name.c_str(), typeToString(c.Instance->Prop[j]->Type).c_str());
	}

	for (uint l = 0; l < c.Instance->States.size (); l++)
	{
		if (c.Instance->States[l].size () != 0)
		{
			nldebug ("NETTC:      > sid: %u", l);
			for (uint k = 0; k < c.Instance->States[l].size (); k++)
			{
				nldebug ("NETTC:      - %d type : %s", c.Instance->States[l][k].first, typeToString(c.Instance->States[l][k].second).c_str());
			}
		}
	}
}

void CTransportClass::displayLocalRegisteredClass ()
{
	nldebug ("NETTC:> LocalRegisteredClass:");
	for (TRegisteredClass::iterator it = LocalRegisteredClass.begin(); it != LocalRegisteredClass.end (); it++)
	{
		displayLocalRegisteredClass ((*it).second);
	}
}

void cbTCReceiveMessage (CMessage &msgin, const string &name, uint16 sid)
{
	nldebug ("NETTC: cbReceiveMessage");

	CTransportClass::TempMessage = msgin;

	string className;
	CTransportClass::TempMessage.serial (className);

	CTransportClass::TRegisteredClass::iterator it = CTransportClass::LocalRegisteredClass.find (className);
	if (it == CTransportClass::LocalRegisteredClass.end ())
	{
		nlwarning ("cbTCReceiveMessage(): Receive unknown transport class '%s' received from %s-%hu", className.c_str(), name.c_str(), sid);
		return;
	}

	nlassert ((*it).second.Instance != NULL);
	
	if (!(*it).second.Instance->read (name, (uint8)sid))
	{
		nlwarning ("cbTCReceiveMessage(): Can't read the transportclass '%s' received from %s-%hu", className.c_str(), name.c_str(), sid);
	}
}

void cbTCReceiveOtherSideClass (CMessage &msgin, const string &name, uint16 sid)
{
	nldebug ("NETTC: cbReceiveOtherSideClass");

	CTransportClass::TOtherSideRegisteredClass osrc;

	uint32 nbClass;
	msgin.serial (nbClass);

	nldebug ("NETTC: %d class", nbClass);

	for (uint i = 0; i < nbClass; i++)
	{
		string className;
		msgin.serial (className);

		osrc.push_back(make_pair (className, vector<CTransportClass::CRegisteredBaseProp>()));

		uint32 nbProp;
		msgin.serial (nbProp);

		nldebug ("NETTC:   %s (%d prop)", className.c_str(), nbProp);

		for (uint j = 0; j < nbProp; j++)
		{
			CTransportClass::CRegisteredBaseProp prop;
			msgin.serial (prop.Name);
			msgin.serialEnum (prop.Type);
			nldebug ("NETTC:     %s %s", prop.Name.c_str(), typeToString(prop.Type).c_str());
			osrc[osrc.size()-1].second.push_back (prop);
		}
	}

	// we have the good structure
	CTransportClass::registerOtherSideClass ((uint8)sid, osrc);
}

static TUnifiedCallbackItem CallbackArray[] =
{
	{ "CT_LRC", cbTCReceiveOtherSideClass },
	{ "CT_MSG", cbTCReceiveMessage },
};

void cbTCUpService (const std::string &serviceName, uint16 sid, void *arg)
{
	nldebug ("NETTC: CTransportClass Service %s %d is up", serviceName.c_str(), sid);
	if (sid >= 256)
		return;
	CTransportClass::sendLocalRegisteredClass ((uint8)sid);
}

void CTransportClass::init ()
{
	// this isn't an error!
	if (Init) return;

	// filter all my debug stuffs
	DebugLog->addNegativeFilter ("NETTC");


	CUnifiedNetwork::getInstance()->addCallbackArray (CallbackArray, sizeof (CallbackArray) / sizeof (CallbackArray[0]));

	// create an instance of all d'ifferent prop types

	DummyProp.resize (PropUKN);

	nlassert (PropUInt8 < PropUKN); DummyProp[PropUInt8] = new CTransportClass::CRegisteredProp<uint8>;
	nlassert (PropUInt16 < PropUKN); DummyProp[PropUInt16] = new CTransportClass::CRegisteredProp<uint16>;
	nlassert (PropUInt32 < PropUKN); DummyProp[PropUInt32] = new CTransportClass::CRegisteredProp<uint32>;
	nlassert (PropUInt64 < PropUKN); DummyProp[PropUInt64] = new CTransportClass::CRegisteredProp<uint64>;
	nlassert (PropSInt8 < PropUKN); DummyProp[PropSInt8] = new CTransportClass::CRegisteredProp<sint8>;
	nlassert (PropSInt16 < PropUKN); DummyProp[PropSInt16] = new CTransportClass::CRegisteredProp<sint16>;
	nlassert (PropSInt32 < PropUKN); DummyProp[PropSInt32] = new CTransportClass::CRegisteredProp<sint32>;
	nlassert (PropSInt64 < PropUKN); DummyProp[PropSInt64] = new CTransportClass::CRegisteredProp<sint64>;
	nlassert (PropBool < PropUKN); DummyProp[PropBool] = new CTransportClass::CRegisteredProp<bool>;
	nlassert (PropFloat < PropUKN); DummyProp[PropFloat] = new CTransportClass::CRegisteredProp<float>;
	nlassert (PropDouble < PropUKN); DummyProp[PropDouble] = new CTransportClass::CRegisteredProp<double>;
	nlassert (PropString < PropUKN); DummyProp[PropString] = new CTransportClass::CRegisteredProp<string>;
//	nlassert (PropDataSetRow < PropUKN); DummyProp[PropDataSetRow] = new CTransportClass::CRegisteredProp<TDataSetRow>;
//	nlassert (PropEntityId < PropUKN); DummyProp[PropEntityId] = new CTransportClass::CRegisteredProp<CEntityId>;
	nlassert (PropSheetId < PropUKN); DummyProp[PropSheetId] = new CTransportClass::CRegisteredProp<CSheetId>;

	// we have to know when a service comes, so add callback (put the callback before all other one because we have to send this message first)
	CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbTCUpService, NULL, false);

	Init = true;
}

void CTransportClass::release ()
{
	unregisterClass ();

	for (uint i = 0; i < DummyProp.size (); i++)
	{
		delete DummyProp[i];
	}
	DummyProp.clear ();
}

void CTransportClass::createLocalRegisteredClassMessage ()
{
	TempMessage.clear ();
	if (TempMessage.isReading())
		TempMessage.invert();
	TempMessage.setType ("CT_LRC");

	uint32 nbClass = LocalRegisteredClass.size ();
	TempMessage.serial (nbClass);

	for (TRegisteredClass::iterator it = LocalRegisteredClass.begin(); it != LocalRegisteredClass.end (); it++)
	{
		nlassert ((*it).first == (*it).second.Instance->Name);

		TempMessage.serial ((*it).second.Instance->Name);

		uint32 nbProp = (*it).second.Instance->Prop.size ();
		TempMessage.serial (nbProp);

		for (uint j = 0; j < (*it).second.Instance->Prop.size (); j++)
		{
			// send the name and the type of the prop
			TempMessage.serial ((*it).second.Instance->Prop[j]->Name);
			TempMessage.serialEnum ((*it).second.Instance->Prop[j]->Type);
		}
	}
}

} // NLNET
