/** \file ia_exception.h
 * some ia exception class.
 *
 * $Id: ai_exception.h,v 1.11 2001/10/17 08:53:19 chafik Exp $
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
		virtual const IException *clone() const = 0;
		virtual ~IException()
		{
		}
	};

	class CExceptionContainer: public IException
	{
	private:
		IException *_Excep;
	public:
		CExceptionContainer(const CExceptionContainer &exc);
		CExceptionContainer(const IException &exc);/*:_Excep((IException *)exc.clone())
		{
		}*/
		
		const char *what() const throw() 
		{
			return _Excep->what();
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionContainer(*_Excep->clone());
		}*/
		virtual ~CExceptionContainer()
		{
			delete _Excep;
		}
	};

	class CExceptionIndexError: public IException
	{		
	public:
		
		CExceptionIndexError()
		{
		}
		
		const char *what() const throw() 
		{
			return "Depacement de résolution sur les index";
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionIndexError();
		}*/

		virtual ~CExceptionIndexError()
		{
		}
	};

	class CExceptionIndexHandeledError: public IException
	{
	
	public:
		
		CExceptionIndexHandeledError()
		{
		}
		
		const char *what() const throw() 
		{
			return "l'index ne pointe pas rien un agent";
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionIndexHandeledError();
		}*/

		virtual ~CExceptionIndexHandeledError()
		{
		}
	};

	class CExceptionUnRegisterClassError: public IException
	{
	private:	
		char *_ClassName;
	public:
		CExceptionUnRegisterClassError(const CExceptionUnRegisterClassError &e)
		{
			_ClassName = new char [strlen(e._ClassName) + 1];
			strcpy(_ClassName,e._ClassName);
		}
		CExceptionUnRegisterClassError(char *name)
		{
			_ClassName = new char [strlen(name) + 1];
			strcpy(_ClassName,name);
		}

		const char *what() const throw() 
		{
			return _ClassName;
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionUnRegisterClassError(_ClassName);
		}*/

		~CExceptionUnRegisterClassError()
		{
			delete _ClassName;
		}

	};

	class CExceptionObjectNotFoundError: public IException
	{
	private:	
		char *_ClassName;
	public:
		CExceptionObjectNotFoundError(const CExceptionObjectNotFoundError &e)
		{
			_ClassName = new char [strlen(e._ClassName) + 1];
			strcpy(_ClassName,e._ClassName);
		}
		CExceptionObjectNotFoundError(char *name)
		{
			_ClassName = new char [strlen(name) + 1];
			strcpy(_ClassName,name);
		}

		const char *what() const throw() 
		{
			return _ClassName;
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionObjectNotFoundError(_ClassName);
		}*/

		~CExceptionObjectNotFoundError()
		{
			delete _ClassName;
		}
	};

	class CExceptionNotImplemented: public IException
	{
	private:
		char *_Text;
	public:

		CExceptionNotImplemented(const CExceptionNotImplemented &e)
		{
			_Text = new char [strlen(e._Text) + 1];
			strcpy(_Text,e._Text);
		}
		
		CExceptionNotImplemented(const char *text)
		{
			_Text = new char [strlen(text) + 1];
			strcpy(_Text,text);
		}

		const char *what() const throw() 
		{
			return _Text;
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionNotImplemented(_Text);
		}*/

		virtual ~CExceptionNotImplemented()
		{
			delete _Text;
		}
	};

	class CExceptionUnReference: public IException
	{
	private:
		char *_Text;
	public:

		CExceptionUnReference(const CExceptionUnReference &e)
		{
			_Text = new char [strlen(e._Text) + 1];
			strcpy(_Text,e._Text);
		}
		
		CExceptionUnReference(char *text)
		{
			_Text = new char [strlen(text) + 1];
			strcpy(_Text,text);
		}

		const char *what() const throw() 
		{
			return _Text;
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionUnReference(_Text);
		}*/

		virtual ~CExceptionUnReference()
		{
			delete _Text;
		}
	};	

	class CExceptionAllReadyExist: public IException
	{
	private:
		char *_Text;
	public:

		CExceptionAllReadyExist(const CExceptionAllReadyExist &e)
		{
			_Text = new char [strlen(e._Text) + 1];
			strcpy(_Text,e._Text);
		}
		
		CExceptionAllReadyExist(char *text)
		{
			_Text = new char [strlen(text) + 1];
			strcpy(_Text,text);
		}

		const char *what() const throw() 
		{
			return _Text;
		}

		virtual const IException *clone() const;/*
		{
			return new CExceptionAllReadyExist(_Text);
		}*/

		virtual ~CExceptionAllReadyExist()
		{
			delete _Text;
		}
	};	
}
#endif
