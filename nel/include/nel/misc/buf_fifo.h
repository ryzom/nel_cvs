/** \file buf_fifo.h
 * <File description>
 *
 * $Id: buf_fifo.h,v 1.2 2001/02/23 14:54:52 lecroart Exp $
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

#ifndef NL_BUF_FIFO_H
#define NL_BUF_FIFO_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"


namespace NLMISC {


/**
 * <Class description>
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CBufFIFO
{
public:

	CBufFIFO ();
	~CBufFIFO ();

	void	 push (std::vector<uint8> &buffer);

	void	 pop (std::vector<uint8> &buffer);

	void	 resize (uint32 size);

	bool	 empty ();

	void	 clear ();

	void	 display ();

	void	 displayStats ();

private:

	// pointer to the big buffer
	uint8	*_Buffer;
	// size of the big buffer
	uint32	 _BufferSize;

	// true if the bufffer is empty
	bool	 _Empty;

	// head of the FIFO
	uint8	*_Head;
	// tail of the FIFO
	uint8	*_Tail;

	// pointer to the real end of the buffer
	uint8	*_Rewinder;

	// return true if we can put size bytes on the buffer
	// return false if we have to resize
	bool	 canFit (uint32 size);


	// statisics on the fifo
	uint32 _BiggestBlock;
	uint32 _SmallestBlock;
	uint32 _BiggestBuffer;
	uint32 _SmallestBuffer;
	uint32 _Pushed ;
	uint32 _Poped;
	uint32 _Resized;
	TTicks _PushedTime;
	TTicks _PopedTime;
	TTicks _ResizedTime;
};


} // NLMISC


#endif // NL_BUF_FIFO_H

/* End of buf_fifo.h */
