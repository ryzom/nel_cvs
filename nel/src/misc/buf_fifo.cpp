/** \file buf_fifo.cpp
 * Implementation for CBufFIFO
 *
 * $Id: buf_fifo.cpp,v 1.11 2001/03/07 16:19:22 cado Exp $
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

#include <vector>

#include "nel/misc/debug.h"
#include "nel/misc/time_nl.h"

#include "nel/misc/buf_fifo.h"

using namespace std;

#define DEBUG_FIFO 1

namespace NLMISC {


CBufFIFO::CBufFIFO() : _Buffer(NULL), _BufferSize(0), _Head(NULL), _Tail(NULL), _Empty(true), _Rewinder(NULL)
{
	_BiggestBlock = 0;
	_SmallestBlock = 999999999;
	_BiggestBuffer = 0;
	_SmallestBuffer = 999999999;
	_Pushed = 0;
	_Fronted = 0;
	_Resized = 0;
	_PushedTime = 0;
	_FrontedTime = 0;
	_ResizedTime = 0;
}

CBufFIFO::~CBufFIFO()
{
	if (_Buffer != NULL)
	{
		delete _Buffer;
#if DEBUG_FIFO
		nldebug("%p delete", this);
#endif
	}
}

void CBufFIFO::push(const std::vector<uint8> &buffer)
{
	TTicks before = CTime::getPerformanceTime();

	uint32 size = buffer.size();

#if DEBUG_FIFO
	nldebug("%p push(%d)", this, size);
#endif

	nlassert(size > 0 && size < 1000);

	// stat code
	if (size > _BiggestBlock) _BiggestBlock = size;
	if (size < _SmallestBlock) _SmallestBlock = size;
	_Pushed++;


	while (!canFit (size + sizeof (uint32)))
	{
		resize(_BufferSize * 2);
	}

	*(uint32 *)_Head = size;
	_Head += sizeof(uint32);

	memcpy(_Head, &(buffer[0]), size);

	_Head += size;

	_Empty = false;

	// stat code
	TTicks after = CTime::getPerformanceTime();
	_PushedTime += after - before;
}

void CBufFIFO::push(const std::vector<uint8> &buffer1, const std::vector<uint8> &buffer2)
{
	TTicks before = CTime::getPerformanceTime();

	uint32 size = buffer1.size() + buffer2.size ();

#if DEBUG_FIFO
	nldebug("%p push2(%d)", this, size);
#endif

	nlassert(size > 0 && size < 1000);

	// stat code
	if (size > _BiggestBlock) _BiggestBlock = size;
	if (size < _SmallestBlock) _SmallestBlock = size;

	_Pushed++;

	// resize while the buffer is enough big to accept the block
	while (!canFit (size + sizeof (uint32)))
	{
		resize(_BufferSize * 2);
	}

	// store the size of the block
	*(uint32 *)_Head = size;
	_Head += sizeof(uint32);

	// store the block itself
	memcpy(_Head, &(buffer1[0]), buffer1.size ());
	memcpy(_Head + buffer1.size(), &(buffer2[0]), buffer2.size ());
	_Head += size;

	_Empty = false;

	// stat code
	TTicks after = CTime::getPerformanceTime();
	_PushedTime += after - before;
}

void CBufFIFO::pop ()
{
	if (empty ())
	{
		nlwarning("Try to pop an empty fifo!");
		return;
	}

	if (_Rewinder != NULL && _Tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p pop rewind!", this);
#endif

		// need to rewind
		_Tail = _Buffer;
		_Rewinder = NULL;
	}

	uint32 size = *(uint32 *)_Tail;

	nlassert(size > 0 && size < 1000);

#if DEBUG_FIFO
	nldebug("%p pop(%d)", this, size);
#endif

#ifdef NL_DEBUG
	// clear the message to be sure user doesn't use it anymore
	memset (_Tail, '-', size + sizeof (uint32));
#endif

	_Tail += size + sizeof (uint32);

	if (_Tail == _Head) _Empty = true;
}
	
void CBufFIFO::front (vector<uint8> &buffer)
{
	TTicks before = CTime::getPerformanceTime ();

	uint8	*tail = _Tail;
	
	buffer.clear ();

	if (empty ())
	{
		nlwarning("Try to get the front of an empty fifo!");
		return;
	}

	_Fronted++;
	
	if (_Rewinder != NULL && tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p front rewind!", this);
#endif

		// need to rewind
		tail = _Buffer;
	}

	uint32 size = *(uint32 *)tail;

	nlassert (size > 0 && size < 1000);

#if DEBUG_FIFO
	nldebug("%p front(%d)", this, size);
#endif

	tail += sizeof (uint32);

	buffer.resize (size);

	memcpy (&(buffer[0]), tail, size);

	// stat code
	TTicks after = CTime::getPerformanceTime ();
	_FrontedTime += after - before;
}


bool CBufFIFO::empty ()
{
	return _Empty;
}

void CBufFIFO::clear ()
{
	_Tail = _Head = _Buffer;
	_Rewinder = NULL;
	_Empty = true;
}

uint32 CBufFIFO::size ()
{
	if (empty ())
	{
		return 0;
	}
	else if (_Head == _Tail)
	{
		// buffer is full
		return _BufferSize;
	}
	else if (_Head > _Tail)
	{
		return _Head - _Tail;
	}
	else if (_Head < _Tail)
	{
		nlassert (_Rewinder != NULL);
		return (_Rewinder - _Tail) + (_Head - _Buffer);
	}
	nlstop;
	return 0;
}

void CBufFIFO::resize (uint32 size)
{
	TTicks before = CTime::getPerformanceTime();

	if (size == 0) size = 100;

#if DEBUG_FIFO
	nldebug("%p resize(%d)", this, size);
#endif

	if (size > _BiggestBuffer) _BiggestBuffer = size;
	if (size < _SmallestBuffer) _SmallestBuffer = size;

	_Resized++;

	uint32 UsedSize = CBufFIFO::size();

	// creer un nouveau tableau et copie l ancien dans le nouveau.
	if (size < _BufferSize && UsedSize < size)
	{
		// probleme, on a pas assez de place pour caser les datas => on fait pas
		nlwarning("Can't resize the FIFO because there's not enough room in the new wanted buffer (%d byte needed at least)", UsedSize);
		return;
	}

	uint8 *NewBuffer = new uint8[size];
	memset (NewBuffer, '-', size);

#if DEBUG_FIFO
	nldebug("%p new %d bytes", this, size);
#endif

	// copy the old buffer to the new one
	// if _Tail == _Head => empty fifo, don't copy anything
	if (!empty())
	{
		if (_Tail < _Head)
		{
			memcpy (NewBuffer, _Buffer, UsedSize);
		}
		else if (_Tail >= _Head)
		{
			nlassert (_Rewinder != NULL);

			uint size1 = _Rewinder - _Tail;
			memcpy (NewBuffer, _Tail, size1);
			uint size2 = _Head - _Buffer;
			memcpy (NewBuffer + size1, _Buffer, size2);

			nlassert (size1+size2==UsedSize);
		}
	}

	// resync the circular pointer
	// Warning: don't invert these 2 lines position or it ll not work
	_Tail = NewBuffer;
	_Head = NewBuffer + UsedSize;
	_Rewinder = NULL;

	// delete old buffer if needed
	if (_Buffer != NULL)
	{
		delete _Buffer;
#if DEBUG_FIFO
		nldebug ("delete", this);
#endif
	}

	// affect new buffer
	_Buffer = NewBuffer;
	_BufferSize = size;

	TTicks after = CTime::getPerformanceTime();

	_ResizedTime += after - before;
}

void CBufFIFO::displayStats ()
{
	nldebug ("_BiggestBlock: %d\n", this, _BiggestBlock);
	nldebug ("_SmallestBlock: %d\n", this, _SmallestBlock);
	nldebug ("_BiggestBuffer: %d\n", this, _BiggestBuffer);
	nldebug ("_SmallestBuffer: %d\n", this, _SmallestBuffer);
	nldebug ("_Pushed : %d\n", this, _Pushed );
	nldebug ("_Fronted: %d\n", this, _Fronted);
	nldebug ("_Resized: %d\n", this, _Resized);
	nldebug ("_PushedTime: %"NL_I64"d %f\n", this, _PushedTime, (double)(sint64)_PushedTime / (double)_Pushed);
	nldebug ("_FrontedTime: %"NL_I64"d %f\n", this, _FrontedTime, (double)(sint64)_FrontedTime / (double)_Fronted);
	nldebug ("_ResizedTime: %"NL_I64"d %f\n", this, _ResizedTime, (double)(sint64)_ResizedTime / (double)_Resized);
}

void CBufFIFO::display ()
{
	int size = 500;
	int gran = size/30;

	char str[1024];

	smprintf(str, 1024, "%p %p (%5d %5d) %p %p %p ", this, _Buffer, _BufferSize, CBufFIFO::size(), _Rewinder, _Tail, _Head);

	int i;
	for (i = 0; i < (sint32) _BufferSize; i+= gran)
	{
		uint8 *pos = _Buffer + i;
		if (_Tail >= pos && _Tail < pos + gran)
		{
			if (_Head >= pos && _Head < pos + gran)
			{
				if (_Rewinder != NULL && _Rewinder >= pos && _Rewinder < pos + gran)
				{
					strncat (str, "*", 1024);
				}
				else
				{
					strncat (str, "@", 1024);
				}
			}
			else
			{
				strncat (str, "T", 1024);
			}
		}
		else if (_Head >= pos && _Head < pos + gran)
		{
			strncat (str, "H", 1024);
		}
		else if (_Rewinder != NULL && _Rewinder >= pos && _Rewinder < pos + gran)
		{
			strncat (str, "R", 1024);
		}
		else
		{
			if (strlen(str) < 1023)
			{
				uint32 p = strlen(str);
				if (isprint(*pos))
					str[p] = *pos;
				else
					str[p] = '$';

				str[p+1] = '\0';
			}
		}
	}

	for (; i < size; i+= gran)
	{
		strncat (str, " ", 1024);
	}
#ifdef NL_DEBUG
	strncat (str, "\n", 1024);
#else
	strncat (str, "\r", 1024);
#endif
	DebugLog.display (str);
}

bool CBufFIFO::canFit (uint32 size)
{
	if (_Tail == _Head)
	{
		if (empty())
		{
			// is the buffer large enough?
			if (_BufferSize >= size)
			{
				// reset the pointer
#if DEBUG_FIFO
				nldebug("%p reset tail and head", this);
#endif
				_Head = _Tail = _Buffer;
				return true;
			}
			else
			{
				// buffer not big enough
#if DEBUG_FIFO
				nldebug("%p buffer full buffersize<size", this);
#endif
				return false;
			}
		}
		else
		{
			// buffer full
#if DEBUG_FIFO
			nldebug("%p buffer full h=t", this);
#endif
			return false;
		}
	}
	else if (_Tail < _Head)
	{
		if (_Buffer + _BufferSize - _Head >= (sint32) size)
		{
			// can fit after _Head
#if DEBUG_FIFO
			nldebug("%p fit after", this);
#endif
			return true;
		}
		else if (_Tail - _Buffer >= (sint32) size)
		{
			// can fit at the beginning
#if DEBUG_FIFO
			nldebug("%p fit at beginning", this);
#endif
			_Rewinder = _Head;
#if DEBUG_FIFO
			nldebug("%p set the rewinder", this);
#endif
			_Head = _Buffer;
			return true;
		}
		else
		{
			// can't fit
#if DEBUG_FIFO
			nldebug("%p no room t<h", this);
#endif
			return false;
		}
	}
	else // the last case is : if (_Tail > _Head)
	{
		if (_Tail - _Head >= (sint32) size)
		{
#if DEBUG_FIFO
			nldebug("%p fit t>h", this);
#endif
			return true;
		}
		else
		{
#if DEBUG_FIFO
			nldebug("%p no room t>h", this);
#endif
			return false;
		}
	}
	nlstop;
}


} // NLMISC
