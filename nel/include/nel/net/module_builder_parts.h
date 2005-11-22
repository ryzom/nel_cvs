/** \file module_builder_parts.h
 * Template module building blocks.
 *
 * $Id: module_builder_parts.h,v 1.4.4.1 2005/11/22 18:46:20 boucher Exp $
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

#include "module.h"

#ifndef MODULE_BUILDER_PARTS_H
#define MODULE_BUILDER_PARTS_H

namespace NLNET
{

	template <class T>
	class CEmptyModuleServiceBehav : public T
	{
	public:
		virtual void				onServiceUp(const std::string &serviceName, uint16 serviceId)
		{}
		virtual void				onServiceDown(const std::string &serviceName, uint16 serviceId)
		{}
		virtual void				onModuleUpdate()
		{}
		virtual void				onApplicationExit()
		{}
	};

	template <class T>
	class CEmptyModuleCommBehav : public T
	{
	public:
		virtual void				onModuleUp(IModuleProxy *moduleProxy)
		{}
		virtual void				onModuleDown(IModuleProxy *moduleProxy)
		{}
		virtual void				onModuleSecurityChange(IModuleProxy *moduleProxy)
		{}
		virtual void				onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
		{}
	};

	template <class T>
	class CEmptySocketBehav : public T
	{
	public:
		virtual void	onModuleSocketEvent(IModuleSocket *moduleSocket, IModule::TModuleSocketEvent eventType)
		{
		}
	};

} // namespace NLNET

#endif // MODULE_BUILDER_PARTS_H

