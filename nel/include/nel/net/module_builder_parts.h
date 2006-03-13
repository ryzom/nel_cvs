/** \file module_builder_parts.h
 * Template module building blocks.
 *
 * $Id: module_builder_parts.h,v 1.4.4.4 2006/03/13 17:43:24 boucher Exp $
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
		std::string					buildModuleManifest() const
		{
			return "";
		}
		virtual void				onModuleUp(IModuleProxy *moduleProxy)
		{}
		virtual void				onModuleDown(IModuleProxy *moduleProxy)
		{}
		virtual void				onModuleSecurityChange(IModuleProxy *moduleProxy)
		{}
		virtual bool				onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
		{	return false; }
	};

	template <class T>
	class CEmptySocketBehav : public T
	{
	public:
		virtual void	onModuleSocketEvent(IModuleSocket *moduleSocket, IModule::TModuleSocketEvent eventType)
		{
		}
	};

	/** Forwarded interceptor callback.
	 *	this class work in conjunction with CInterceptorForwarder.
	 *	Derive the class that implement the forwarded call
	 *	from this interface.
	 */
//	class IForwardedInterceptor
//	{
//	public:
//		std::string			fwdBuildModuleManifest() const
//		{
//			return std::string();
//		}
//		virtual void				fwdOnModuleUp(IModuleProxy *moduleProxy)
//		{
//		}
//		virtual void				fwdOnModuleDown(IModuleProxy *moduleProxy)
//		{
//		}
//		virtual bool				fwdOnProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
//		{
//			return false;
//		}
//		virtual void				fwdOnModuleSecurityChange(IModuleProxy *moduleProxy)
//		{
//		}
//	};

	/** Interceptor forwarder
	 *	The trick is that if you build a module interceptor class 
	 *	and then you want to inherit this class in a module definition, then
	 *	the virtual callbacks are received by the module instead of by your 
	 *	interceptor (because the base module is also an interceptor and
	 *	it eventualy overides the calls).
	 *	The workaround consist of having the interceptor implemented in
	 *	an inner class with method forwarded to you class with a different 
	 *	interface.
	 */
	template <class ParentClass>
	class CInterceptorForwarder : public IModuleInterceptable
	{
		ParentClass		*_Parent;
	public:
		CInterceptorForwarder()
			:	_Parent(NULL)
		{}

		void init(ParentClass *parent, IModule *module)
		{
			nlassert(parent != NULL);
			nlassert(module != NULL);

			_Parent = parent;
			registerInterceptor(module);
		}

		virtual std::string			buildModuleManifest() const
		{
			return _Parent->fwdBuildModuleManifest();
		}
		virtual void				onModuleUp(IModuleProxy *moduleProxy)
		{
			_Parent->fwdOnModuleUp(moduleProxy);
		}
		virtual void				onModuleDown(IModuleProxy *moduleProxy)
		{
			_Parent->fwdOnModuleDown(moduleProxy);
		}
		virtual bool				onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
		{
			return _Parent->fwdOnProcessModuleMessage(senderModuleProxy, message);
		}
		virtual void				onModuleSecurityChange(IModuleProxy *moduleProxy)
		{
			_Parent->fwdOnModuleSecurityChange(moduleProxy);
		}
	};

} // namespace NLNET

#endif // MODULE_BUILDER_PARTS_H

