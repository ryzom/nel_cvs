/** \file buf_fifo.cpp
 * <File description>
 *
 * $Id: buf_fifo.cpp,v 1.2 2001/02/23 14:54:52 lecroart Exp $
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

/*
release
_BiggestBlock: 998
_SmallestBlock: 1
_BiggestBuffer: 2097152
_SmallestBuffer: 16
_Pushed : 2657068
_Poped: 2656608
_Resized: 18
_PushedTime: 10476272 3.942794
_PopedTime: 45561095 17.150101
_ResizedTime: 108373 6020.722222
--------------------------------------------
debug
_BiggestBlock: 998
_SmallestBlock: 1
_BiggestBuffer: 2097152
_SmallestBuffer: 16
_Pushed : 2657068
_Poped: 2656608
_Resized: 18
_PushedTime: 14303341 5.383129
_PopedTime: 1143916487 430.592879
_ResizedTime: 178167 9898.166667
*/

#include <vector>

#include "nel/misc/debug.h"
#include "nel/misc/time_nl.h"

#include "nel/misc/buf_fifo.h"

using namespace std;

namespace NLMISC {



//todo: kan tail = head ca veut pas dire que c est vide (ca peut etre full aussi) => mettre un bool

CBufFIFO::CBufFIFO() : _Buffer(NULL), _BufferSize(0), _Head(NULL), _Tail(NULL), _Empty(true), _Rewinder(NULL)
{
	_BiggestBlock = 0;
	_SmallestBlock = 999999999;
	_BiggestBuffer = 0;
	_SmallestBuffer = 999999999;
	_Pushed = 0;
	_Poped = 0;
	_Resized = 0;
	_PushedTime = 0;
	_PopedTime = 0;
	_ResizedTime = 0;
}

CBufFIFO::~CBufFIFO()
{
	if (_Buffer != NULL)
	{
		delete _Buffer;
		//nldebug("delete");
	}
}

void CBufFIFO::push(std::vector<uint8> &buffer)
{
	TTicks before = CTime::getPerformanceTime();

	//nldebug("push(%d)", size);

	nlassert(buffer.size() > 0 && buffer.size() < 1000);

	// stat code
	if (buffer.size() > _BiggestBlock) _BiggestBlock = buffer.size();
	if (buffer.size() < _SmallestBlock) _SmallestBlock = buffer.size();

	_Pushed++;

	while (!canFit (buffer.size() + sizeof (uint32)))
	{
		resize(_BufferSize * 2);
	}

	*(uint32 *)_Head = buffer.size();
	_Head += sizeof(uint32);

	memcpy(_Head, &(buffer[0]), buffer.size());

	_Head += buffer.size();

	_Empty = false;

	TTicks after = CTime::getPerformanceTime();

	_PushedTime += after - before;
}

void CBufFIFO::pop (vector<uint8> &buffer)
{
	TTicks before = CTime::getPerformanceTime();
	
	buffer.clear ();

	if (empty ())
	{
		nlwarning("Try to pop but the fifo is empty!");
		return;
	}

	_Poped++;
	
	if (_Rewinder != NULL && _Tail == _Rewinder)
	{
		//nldebug("rewind!");

		// need to rewind
		_Tail = _Buffer;
		_Rewinder = NULL;
	}

	uint32 size = *(uint32 *)_Tail;

	nlassert(size > 0 && size<1000);

	//nldebug("pop(%d)", size);

	_Tail += sizeof (uint32);

	buffer.resize (size);

	memcpy(&(buffer[0]), _Tail, size);

	memset(_Tail, '-', size);

	_Tail += size;

	if (_Tail == _Head) _Empty = true;

	TTicks after = CTime::getPerformanceTime();

	_PopedTime += after - before;
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

void CBufFIFO::resize (uint32 size)
{
	TTicks before = CTime::getPerformanceTime();

	//nldebug("resize(%d)", size);

	if (size > _BiggestBuffer) _BiggestBuffer = size;
	if (size < _SmallestBuffer) _SmallestBuffer = size;

	_Resized++;

	uint32 UsedSize;
	if (_Tail <= _Head)
	{
		UsedSize = _Head - _Tail;
	}
	else
	{
		nlassert (_Rewinder != NULL);
		UsedSize = _Rewinder - _Tail + _Head - _Buffer;
	}

	// creer un nouveau tableau et copie l ancien dans le nouveau.
	if (size < _BufferSize && UsedSize < size)
	{
		// probleme, on a pas assez de place pour caser les datas => on fait pas
		nlwarning("Can't resize the FIFO because there's not enough room in the new wanted buffer (%d byte needed at least)", UsedSize);
		return;
	}

	uint8 *NewBuffer = new uint8[size];
	memset (NewBuffer, '-', size);

	//nldebug("new %d bytes", size);

	// copy the old buffer to the new one
	// if _Tail == _Head => empty fifo, don't copy anything
	if (_Tail < _Head)
	{
		memcpy (NewBuffer, _Buffer, UsedSize);
	}
	else if (_Tail > _Head)
	{
		nlassert (_Rewinder != NULL);

		uint size1 = _Rewinder - _Tail;
		memcpy (NewBuffer, _Tail, size1);
		uint size2 = _Head - _Buffer;
		memcpy (NewBuffer + size1, _Buffer, size2);

		nlassert (size1+size2==UsedSize);
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
		//nldebug("delete");
	}

	// affect new buffer
	_Buffer = NewBuffer;
	_BufferSize = size;

	TTicks after = CTime::getPerformanceTime();

	_ResizedTime += after - before;
}

void CBufFIFO::displayStats ()
{
	printf ("_BiggestBlock: %d\n", _BiggestBlock);
	printf ("_SmallestBlock: %d\n", _SmallestBlock);
	printf ("_BiggestBuffer: %d\n", _BiggestBuffer);
	printf ("_SmallestBuffer: %d\n", _SmallestBuffer);
	printf ("_Pushed : %d\n", _Pushed );
	printf ("_Poped: %d\n", _Poped);
	printf ("_Resized: %d\n", _Resized);
	printf ("_PushedTime: %"NL_I64"d %f\n", _PushedTime, (double)(sint64)_PushedTime / (double)_Pushed);
	printf ("_PopedTime: %"NL_I64"d %f\n", _PopedTime, (double)(sint64)_PopedTime / (double)_Poped);
	printf ("_ResizedTime: %"NL_I64"d %f\n", _ResizedTime, (double)(sint64)_ResizedTime / (double)_Resized);
}

void CBufFIFO::display ()
{
	int size = 5000;
	int gran = size/30;

	printf("%p (%5d) %p %p %p ", _Buffer, _BufferSize, _Rewinder, _Tail, _Head);

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
					printf("*");
				}
				else
				{
					printf("@");
				}
			}
			else
			{
				printf("T");
			}
		}
		else if (_Head >= pos && _Head < pos + gran)
		{
			printf("H");
		}
		else if (_Rewinder != NULL && _Rewinder >= pos && _Rewinder < pos + gran)
		{
			printf("R");
		}
		else
		{
			printf("%c", *pos);
		}
	}

	for (; i < size; i+= gran)
	{
		printf(" ");
	}
#ifdef NL_DEBUG
	printf("\n");
#else
	printf("\r");
#endif
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
				//nldebug("reset tail and head");
				_Head = _Tail = _Buffer;
				return true;
			}
			else
			{
				// buffer not big enough
				//nldebug("buffer full buffersize<size");
				return false;
			}
		}
		else
		{
			// buffer full
			//nldebug("buffer full h=t");
			return false;
		}
	}
	else if (_Tail < _Head)
	{
		if (_Buffer + _BufferSize - _Head >= (sint32) size)
		{
			// can fit after _Head
			//nldebug("fit after");
			return true;
		}
		else if (_Tail - _Buffer >= (sint32) size)
		{
			// can fit at the beginning
			//nldebug("fit at beginning");
			_Rewinder = _Head;
			//nldebug("set the rewinder");
			_Head = _Buffer;
			return true;
		}
		else
		{
			// can't fit
			//nldebug("no room t<h");
			return false;
		}
	}
	else // the last case is : if (_Tail > _Head)
	{
		if (_Tail - _Head >= (sint32) size)
		{
			//nldebug("fit t>h");
			return true;
		}
		else
		{
			//nldebug("no room t>h");
			return false;
		}
	}
	nlstop;
}


} // NLMISC
