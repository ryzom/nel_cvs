/** \file ai_exception.h
 * some ai exception class.
 *
 * $Id: ai_exception.h,v 1.20 2002/10/04 09:59:01 coutelas Exp $
 *
 * Available constantes:
 * - NL_OS_WINDOWS		: windows operating system (32bits)
 * - NL_OS_UNIX			: linux operating system
 *
 * - NL_BIG_ENDIAN		: other processor
 * - NL_LITTLE_ENDIAN	: x86 processor
 *
 * - NL_DEBUG			: no optimization, full debug information, all log for the client
 * - NL_RELEASE			: full optimization, no debug information, no log for the client
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

#ifndef NL_IA_EXCEPTION_H
#define NL_IA_EXCEPTION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"

//#include "nel/ai/e/nel_exception_def.h"

namespace NLAIE
{		
	class IException: public NLMISC::Exception
	{	

	private:
	public:		
		IException()
		{
		}		

		IException(const IException &e):NLMISC::Exception(e.what())
		{
		}		
		IException(const std::string &reason): NLMISC::Exception(reason)
		{
		}		
		virtual const IException *clone() const = 0;
		virtual ~IException() throw()
		{
		}
	};

	class CExceptionContainer: public IException
	{
	
	public:
		CExceptionContainer(const CExceptionContainer &exc);		
		CExceptionContainer(const std::string &name);

		virtual const IException *clone() const;		
		virtual ~CExceptionContainer() throw()
		{			
		}
	};

	class CExceptionIndexError: public IException
	{		
	public:
		
		CExceptionIndexError(): IException("Depacement de résolution sur les index")
		{
		}

		virtual const IException *clone() const;

		virtual ~CExceptionIndexError() throw()
		{
		}
	};

	class CExceptionIndexHandeledError: public IException
	{
	
	public:
		
		CExceptionIndexHandeledError(): IException("l'index ne pointe pas rien un agent")
		{
		}
			
		virtual const IException *clone() const;

		virtual ~CExceptionIndexHandeledError() throw()
		{
		}
	};

	class CExceptionUnRegisterClassError: public IException
	{
	private:			
	public:
		CExceptionUnRegisterClassError(const CExceptionUnRegisterClassError &e);		
		CExceptionUnRegisterClassError(const std::string &name);

		virtual const IException *clone() const;

		~CExceptionUnRegisterClassError() throw()
		{			
		}

	};

	class CExceptionObjectNotFoundError: public IException
	{	
	public:
		CExceptionObjectNotFoundError(const CExceptionObjectNotFoundError &e):IException(e)
		{			
		}
		CExceptionObjectNotFoundError(const std::string &name):IException(name)
		{			
		}		

		virtual const IException *clone() const;

		~CExceptionObjectNotFoundError() throw()
		{		
		}
	};

	class CExceptionNotImplemented: public IException
	{	
	public:

		CExceptionNotImplemented(const CExceptionNotImplemented &e);		
		
		CExceptionNotImplemented(const std::string &text);				

		virtual const IException *clone() const;

		virtual ~CExceptionNotImplemented() throw()
		{			
		}
	};

	class CExceptionUnReference: public IException
	{	
	public:

		CExceptionUnReference(const CExceptionUnReference &e);
		CExceptionUnReference(const std::string &text);
		CExceptionUnReference(char *text);		

		virtual const IException *clone() const;

		virtual ~CExceptionUnReference() throw()
		{			
		}
	};	

	class CExceptionAllReadyExist: public IException
	{
	
	public:

		CExceptionAllReadyExist(const CExceptionAllReadyExist &e):IException(e)
		{			
		}
		
		CExceptionAllReadyExist(const std::string &name):IException(name)
		{		
		}
		
		virtual const IException *clone() const;

		virtual ~CExceptionAllReadyExist() throw()
		{		
		}
	};	
}
#endif
