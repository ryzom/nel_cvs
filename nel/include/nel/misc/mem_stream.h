/** \file mem_stream.h
 * From memory serialization implementation of IStream using ASCII format (look at stream.h)
 *
 * $Id: mem_stream.h,v 1.16 2002/01/30 10:09:01 lecroart Exp $
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

#ifndef NL_MEM_STREAM_H
#define NL_MEM_STREAM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include <vector>

namespace NLMISC
{

/// Exception class for CMemStream
struct EMemStream : public NLMISC::EStream
{
	EMemStream( const std::string& str ) : EStream( str ) {}
};

/*class EMemStreamTypeNbr : public EMemStream
{
	EMemStreamTypeNbr() : EMemStream( "Bad message type code" ) {}
};

class EMemStreamTypeStr : public EMemStream
{
	EMemStreamTypeStr() : EMemStream( "Bad message type name" ) {}
};*/

/// This exception is raised when someone tries to serialize in more than there is.
struct EStreamOverflow : public EMemStream
{
	EStreamOverflow() : EMemStream( "Stream Overflow Error" ) {}
};


/// Vector of uint8
typedef std::vector<uint8> CVector8;

/// Iterator on CVector8
typedef CVector8::iterator It8;



/**
 * MemStream memory stream (see also NLNET::CMessage).
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMemStream : public NLMISC::IStream
{
public:

	/// Initialization constructor
	CMemStream( bool inputStream=false, bool stringmode=false, uint32 defaultcapacity=0 );

	/// Copy constructor
	CMemStream( const CMemStream& other );

	/// Assignment operator
	CMemStream&		operator=( const CMemStream& other );

	/// Set string mode
	void			setStringMode( bool stringmode ) { _StringMode = stringmode; }

	/// Return string mode
	bool			stringMode() const { return _StringMode; }

	/// Method inherited from IStream
	virtual void	serialBuffer(uint8 *buf, uint len);

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit);

	/** 
	 * Moves the stream pointer to a specified location.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return true if seek sucessfull.
	 * \see ESeekNotSupported SeekOrigin getPos
	 */
	virtual bool	seek (sint32 offset, TSeekOrigin origin) throw(EStream);

	/** 
	 * Get the location of the stream pointer.
	 * 
	 * NB: If the stream doesn't support the seek fonctionnality, it throws ESeekNotSupported.
	 * Default implementation: 
	 * { throw ESeekNotSupported; }
	 * \param offset is the wanted offset from the origin.
	 * \param origin is the origin of the seek
	 * \return the new offset regarding from the origin.
	 * \see ESeekNotSupported SeekOrigin seek
	 */
	virtual sint32	getPos () throw(EStream);

	/// Clears the message
	virtual void	clear();

	/** Returns the length (size) of the message, in bytes.
	 * If isReading(), it is the number of bytes that can be read,
	 * otherwise it is the number of bytes that have been written.
	 */
	virtual uint32	length() const
	{
		if ( isReading() )
		{
			return lengthR();
		}
		else
		{
			return lengthS();
		}
	}

	/** Returns a pointer to the message buffer (read only)
	 * Returns NULL if the buffer is empty
	 */
	const uint8		*buffer() const
	{
		if ( _Buffer.empty() )
		{
			return NULL;
		}
		else
		{
			return &(*_Buffer.begin());
		}
	}

	/// Returns the message buffer (read only)
	const CVector8&	bufferAsVector() const
	{
		return _Buffer;
	}

	/// Returns the vector for external filling
	CVector8&	bufferAsVector()
	{
		return _Buffer;
	}

	// When you fill the buffer externaly (using bufferAsVector) you have to reset the BufPos calling this method
	void resetBufPos() { _BufPos = _Buffer.begin(); }

	/// Fills the message buffer, for reading
	void			fill( const uint8 *srcbuf, uint32 len );


	/** EXPERIMENTAL: Returns a pointer to the message buffer for filling by an external function (use at your own risk,
	 * you MUST fill the number of bytes you specify in "msgsize").
	 * This method prevents from doing one useless buffer copy, using fill().
	 */
	uint8			*bufferToFill( uint32 msgsize );

	/// Transforms the message from input to output or from output to input
	virtual void	invert();

	/// Force to reset the ptr table
	void			resetPtrTable() { IStream::resetPtrTable() ; }

	/// Template serialisation (should take the one from IStream)
    template<class T>
	void			serial(T &obj)							{ obj.serial(*this); }

	template<class T>
	void			serialCont(std::vector<T> &cont) 		{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 			{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 		{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 			{IStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 		{IStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 		{IStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{IStream::serialCont(cont);}

	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) {IStream::serialCont(cont);} 
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) {IStream::serialCont(cont);} 
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont) {IStream::serialCont(cont);} 



	template<class T0,class T1>
	void			serial(T0 &a, T1 &b) 
	{ serial(a); serial(b);}
	template<class T0,class T1,class T2>
	void			serial(T0 &a, T1 &b, T2 &c) 
	{ serial(a); serial(b); serial(c);}
	template<class T0,class T1,class T2,class T3>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d) 
	{ serial(a); serial(b); serial(c); serial(d);}
	template<class T0,class T1,class T2,class T3,class T4>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e) 
	{ serial(a); serial(b); serial(c); serial(d); serial(e);}
	template<class T0,class T1,class T2,class T3,class T4,class T5>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f) 
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}

	/** \name Base types serialisation, redefined for string mode
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{
	virtual void	serial(uint8 &b) ;
	virtual void	serial(sint8 &b) ;
	virtual void	serial(uint16 &b) ;
	virtual void	serial(sint16 &b) ;
	virtual void	serial(uint32 &b) ;
	virtual void	serial(sint32 &b) ;
	virtual void	serial(uint64 &b) ;
	virtual void	serial(sint64 &b) ;
	virtual void	serial(float &b) ;
	virtual void	serial(double &b) ;
	virtual void	serial(bool &b) ;
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b) ;
#endif
	virtual void	serial(std::string &b) ;
	virtual void	serial(ucstring &b) ;
	//@}


	///\name String-specific methods
	//@{

	/// Input: read len bytes at most from the stream until the next separator, and return the number of bytes read. The separator is then skipped.
	uint			serialSeparatedBufferIn( uint8 *buf, uint len );

	/// Output: writes len bytes from buf into the stream
	void			serialSeparatedBufferOut( uint8 *buf, uint len );

	/// Serialisation in hexadecimal
	virtual void	serialHex(uint32 &b);

	//@}

protected:

	/// Returns the serialized length (number of bytes written or read)
	uint32			lengthS() const
	{
		return _BufPos-_Buffer.begin();
	}

	/// Returns the "read" message size (number of bytes to read)
	uint32			lengthR() const
	{
		return _Buffer.size();
	}

	CVector8		_Buffer;
	It8				_BufPos;
	bool			_StringMode;

};

}

#endif // NL_MEM_STREAM_H

/* End of mem_stream.h */
