/** \file module_message.h
 * module message definition
 *
 * $Id: module_message.h,v 1.1 2005/06/23 16:33:49 boucher Exp $
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


#ifndef NL_MODULE_MESSAGE_H
#define NL_MODULE_MESSAGE_H

#include "nel/misc/enum_bitset.h"
#include "module_common.h"


namespace NLNET
{
	/** Module message definition
	 *	Module contains informations about module sender, module addresse,
	 *	type of message (for future extension).
	 */
	class CModuleMessage : public NLMISC::CRefCount
	{
		friend class CModuleProxy;
		friend class CModuleBase;
		friend class CGatewayBase;
	public:

		enum TMessageType
		{
			/// Standard one way message
			mt_oneway,
			/// Two way request
			mt_twoway_request,
			/// Two way response
			mt_twoway_response,


			/// A special checking value
			mt_num_types,
			/// invalid flag
			mt_invalid = mt_num_types

		};
		
	private:
		/// The type of message
		TMessageType		_MessageType;
		/// The id of the sender module
		TModuleId			_SenderModuleId;
		/// The id of the addressee module
		TModuleId			_AddresseeModuleId;
		/// The name of the operation (aka the method name)
		std::string			_OperationName;
	public:

		/// The message content (transmited to message handler)
		NLMISC::CMemStream	MessageBody;

		/// Serialize the message
		void serial(NLMISC::IStream &s);

		CModuleMessage(const std::string &operationName);
	};

} // namespace NLNET

#endif // NL_MODULE_MESSAGE_H
