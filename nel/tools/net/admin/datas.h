/** \file datas.h
 *
 *
 * $Id: datas.h,v 1.3 2001/05/31 16:41:59 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_DATAS_H
#define NL_DATAS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include "nel/net/buf_net_base.h"

#include <list>
#include <string>

// Structures
struct CAdminExecutorService;


struct CService
{
	CService () : Id(0xFFFFFFFF), Ready(false), Connected(false), InConfig(false), Unknown(true), RootTreeItem(NULL) { }

	uint32		Id;				/// uint32 to identify the service
	std::string	AliasName;		/// alias of the service used in the AES and AS to find him (unique per AES)
	std::string	ShortName;		/// name of the service in short format ("NS" for example)
	std::string	LongName;		/// name of the service in long format ("naming_service")
	bool		Ready;			/// true if the service is ready
	bool		Connected;		/// true if the service is connected to the AES
	bool		InConfig;		/// true if the service is in the configuration
	bool		Unknown;		/// true if the aes is not connected

	CAdminExecutorService *AES;

	void setValues (const CService &t)
	{
		// copy all except gtk stuffs
		Id = t.Id;
		AliasName = t.AliasName;
		ShortName = t.ShortName;
		LongName = t.LongName;
		Ready = t.Ready;
		Connected = t.Connected;
		InConfig = t.InConfig;
		Unknown = t.Unknown;
	}

	// used by gtk
	void	*RootTreeItem;
	void	*RootSubTree;
	void	*Bitmap, *Label;
};

typedef std::list<CService> TServices;
typedef std::list<CService>::iterator SIT;

struct CAdminService;

struct CAdminExecutorService
{
	CAdminExecutorService () : Id(0xFFFFFFFF), Connected(false), RootTreeItem(NULL) { }

	uint32		Id;				/// uint32 to identify the AES where the service is running

	std::string	ServerAlias;	/// name of the layer4 connection, used to send message to this AES
	std::string	ServerAddr;		/// address in a string format (only the ip)
	bool		Connected;		/// true if the AES is connected

	TServices	Services;

	CAdminService *AS;

	std::vector<std::string>	ServiceAliasList;	/// contains all service aliases that this AES can run

	void setValues (const CAdminExecutorService &t)
	{
		// copy all except gtk stuffs
		Id = t.Id;
		ServerAlias = t.ServerAlias;
		ServerAddr = t.ServerAddr;
		Connected = t.Connected;
	}

	SIT findService (uint32 sid, bool asrt = true)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).Id == sid)
				break;

		if (asrt)
			nlassert (sit != Services.end());
		return sit;
	}

	SIT findService (const std::string &alias, bool asrt = true)
	{
		SIT sit;
		for (sit = Services.begin(); sit != Services.end(); sit++)
			if ((*sit).AliasName == alias)
				break;

		if (asrt)
			nlassert (sit != Services.end());
		return sit;
	}

	// used by gtk
	void	*RootTreeItem;
	void	*RootSubTree;
	void	*Bitmap, *Label;
	void	*ItemFactory;
};

typedef std::list<CAdminExecutorService> TAdminExecutorServices;
typedef std::list<CAdminExecutorService>::iterator AESIT;


struct CAdminService
{
	CAdminService () : Connected(false), SockId(NULL), Id(NextId++), RootTreeItem(NULL) { }
	
	NLNET::TSockId	SockId;			/// connection to the AS
	uint32			Id;				/// uint32 to identify the AS

	std::string		ASAddr;			/// name of the layer4 connection, used to send message to this AS (AS address and service name are the same)
	std::string		ASName;			/// name of the shard in a human form (ex: "San Fresco Shard")
	bool			Connected;

	TAdminExecutorServices AdminExecutorServices;

	AESIT findAdminExecutorService (uint32 aesid, bool asrt=true)
	{
		AESIT aesit;
		for (aesit = AdminExecutorServices.begin(); aesit != AdminExecutorServices.end(); aesit++)
			if ((*aesit).Id == aesid)
				break;

		if (asrt)
			nlassert (aesit != AdminExecutorServices.end());
		return aesit;
	}

	// used by gtk
	void	*RootTreeItem;
	void	*RootSubTree;
	void	*Bitmap, *Label;

private:
	static uint32 NextId;
};

typedef std::list<CAdminService> TAdminServices;
typedef std::list<CAdminService>::iterator ASIT;

// Variables

extern TAdminServices AdminServices;

// Functions

ASIT findAdminService (uint32 asid, bool asrt = true);

void displayServices ();

#endif // NL_DATAS_H

/* End of datas.h */
