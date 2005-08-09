/** \file module_message.h
 * module message definition
 *
 * $Id: module_message.h,v 1.3 2005/08/09 19:06:25 boucher Exp $
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
#include "nel/net/message.h"
#include "module_common.h"


namespace NLNET
{
	/** Module message header coder/decoder
	 *	Codec for module message header data.
	 */
	class CModuleMessageHeaderCodec
	{
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

		static void encode(CMessage &headerMessage, TMessageType msgType, TModuleId senderProxyId, TModuleId addresseeProxyId)
		{
			serial(headerMessage, msgType, senderProxyId, addresseeProxyId);
		}

		static void decode(const CMessage &headerMessage, TMessageType &msgType, TModuleId &senderProxyId, TModuleId &addresseeProxyId)
		{
			serial(const_cast<CMessage&>(headerMessage), msgType, senderProxyId, addresseeProxyId);
		}

	private:
		static void serial(CMessage &headerMessage, TMessageType &msgType, TModuleId &senderProxyId, TModuleId &addresseeProxyId)
		{
			uint8 mt;
			if (headerMessage.isReading())
			{
				headerMessage.serial(mt);
				msgType = CModuleMessageHeaderCodec::TMessageType(mt);
			}
			else
			{
				mt = msgType;
				headerMessage.serial(mt);
			}
			headerMessage.serial(senderProxyId);
			headerMessage.serial(addresseeProxyId);
		}
	};


//	class CModuleMessage : public NLMISC::CRefCount
//	{
//		friend class CModuleProxy;
//		friend class CModuleBase;
//		friend class CGatewayBase;
//	public:
//
//		enum TMessageType
//		{
//			/// Standard one way message
//			mt_oneway,
//			/// Two way request
//			mt_twoway_request,
//			/// Two way response
//			mt_twoway_response,
//
//
//			/// A special checking value
//			mt_num_types,
//			/// invalid flag
//			mt_invalid = mt_num_types
//
//		};
//		
//		/// The type of message
//		TMessageType		MessageType;
//		/// The id of the sender module proxy
//		TModuleId			SenderModuleId;
//		/// The id of the addressee module proxy
//		TModuleId			AddresseeModuleId;
//		/// The name of the operation (aka the method name)
////		std::string			_OperationName;
//
//		/** The message content (transmited to message handler)
//		 *	This already contain the operation name (in the 
//		 *	CMessage type (see CMessage::setType and CMessage::getName)
//		 */
////		CMessage			&MessageBody;
////		NLMISC::CMemStream	MessageBody;
//
//		///  Return the id of the addressee module
////		NLNET::TModuleId getAddresseeModuleProxyId() const { return _AddresseeModuleId;}
//
//		///  Return the id of the sender module
////		NLNET::TModuleId getSenderModuleProxyId() const { return _SenderModuleId;}
////
//		std::string getOperationName() const { return MessageBody.getName();}
//
//
//		/// Serialize the message
//		void serial(NLMISC::IStream &s);
//
//		/// Construct a module message, receive a reference to a message body.
//		CModuleMessage(const CMessage &messageBody);
//	};

} // namespace NLNET

#endif // NL_MODULE_MESSAGE_H
