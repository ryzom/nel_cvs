/** \file ai_exception.h
 * some ai exception class.
 *
 * $Id: ai_exception.h,v 1.21 2003/02/05 14:47:23 chafik Exp $
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
	/**
	Class abstract to support several standard exception.
	*/
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

	/**
	Class allow to spread an ather exepton, it is used in multible block try/catch when user want to spread the original exeption.
	*/
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

	/**
	This exception is emit when an index error is occure.
	*/
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

	/**
	This exception is emit when an handel is'nt refered. For exemple with an ident of entity bad refered.
	*/
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

	/**
	This exception is emit when user want to access to a class not in the class factory.
	*/
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

	/**	
	This exception is emit when user want to access to an object not mounted in the class factory.	
	*/
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

	/**	
	This exception is emit when user call an undefined method.
	*/
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

	/**
	This exception is emit when user made a bad reference, for exemple when user want to call a script method not defined.
	*/
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

	/**	
	This exception is emit when user want mount a class in the class factory but this call is all ready mounted.	
	*/
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
