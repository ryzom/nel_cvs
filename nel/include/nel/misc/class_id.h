/* class_id.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: class_id.h,v 1.1 2000/10/02 12:55:43 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_CLASS_ID_H
#define NL_CLASS_ID_H


#include "nel/misc/types_nl.h"


namespace	NLMISC
{

// ***************************************************************************
/**
 * A unique id to specify Object by a uint64.
 * The Deriver should use a Max-like Id generator, to identify his own object.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CClassId
{
	uint64	Uid;

public:
	static const	CClassId	Null;

public:
	CClassId() {Uid=0;}
	CClassId(uint32 a, uint32 b) {Uid= ((uint64)a<<32) | b;}
	CClassId(uint64 a) {Uid=a;}
	bool	operator==(const CClassId &o) const {return Uid==o.Uid;}
	bool	operator!=(const CClassId &o) const {return Uid!=o.Uid;}
	bool	operator<=(const CClassId &o) const {return Uid<=o.Uid;}
	bool	operator>=(const CClassId &o) const {return Uid>=o.Uid;}
	bool	operator<(const CClassId &o) const {return Uid<o.Uid;}
	bool	operator>(const CClassId &o) const {return Uid>o.Uid;}
	operator uint64() const {return Uid;}

};


}


#endif // NL_CLASS_ID_H

/* End of class_id.h */
