/* bit_set.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: bit_set.cpp,v 1.4 2000/10/19 12:32:06 corvazier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/bit_set.h"
#include "nel/misc/debug.h"
#include <algorithm>
using namespace std;


namespace	NLMISC
{

// must be defined elsewhere
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


// Size in bit of base word.
#define	BITLEN	(sizeof(uint)*8)


// ***************************************************************************
CBitSet::CBitSet()
{
	NumBits= 0;
	MaskLast= 0;
}
CBitSet::CBitSet(uint numBits)
{
	NumBits= 0;
	MaskLast= 0;
	resize(numBits);
}
CBitSet::CBitSet(const CBitSet &bs)
{
	NumBits= bs.NumBits;
	MaskLast= bs.MaskLast;
	Array= bs.Array;
}
CBitSet::~CBitSet()
{
}
CBitSet	&CBitSet::operator=(const CBitSet &bs)
{
	NumBits= bs.NumBits;
	MaskLast= bs.MaskLast;
	Array= bs.Array;

	return *this;
}


// ***************************************************************************
void	CBitSet::clear()
{
	Array.clear();
	NumBits= 0;
	MaskLast=0;
}
void	CBitSet::resize(uint numBits)
{
	if(numBits==0)
		clear();

	NumBits= numBits;
	Array.resize( (NumBits+BITLEN-1) / BITLEN );
	uint	nLastBits= NumBits & (BITLEN-1) ;
	// Generate the mask for the last word.
	if(nLastBits==0)
		MaskLast= ~((uint)0);
	else
		MaskLast= (1<< nLastBits) -1;

	// reset to 0.
	clearAll();
}
uint	CBitSet::size() const
{
	return NumBits;
}
void	CBitSet::set(sint bitNumber, bool value)
{
	nlassert(bitNumber>=0 && bitNumber<NumBits);

	uint	mask= bitNumber&(BITLEN-1);
	mask= 1<<mask;
	if(value)
		Array[bitNumber / BITLEN]|= mask ;
	else
		Array[bitNumber / BITLEN]&= ~mask;
}
bool	CBitSet::get(sint bitNumber) const
{
	nlassert(bitNumber>=0 && bitNumber<NumBits);

	uint	mask= bitNumber&(BITLEN-1);
	mask= 1<<mask;
	return (Array[bitNumber / BITLEN] & mask) != 0;
}
bool	CBitSet::operator[](sint bitNumber) const
{
	nlassert(bitNumber>=0 && bitNumber<NumBits);

	uint	mask= bitNumber&(BITLEN-1);
	mask= 1<<mask;
	return (Array[bitNumber / BITLEN] & mask) != 0;
}
void	CBitSet::setAll()
{
	fill_n(Array.begin(), Array.size(), ~((uint)0));

	Array[Array.size()-1]&= MaskLast;
}
void	CBitSet::clearAll()
{
	fill_n(Array.begin(), Array.size(), 0);
}


// ***************************************************************************
CBitSet	CBitSet::operator~() const
{
	CBitSet	ret;

	ret= *this;
	ret.flip();
	return ret;
}
CBitSet	CBitSet::operator&(const CBitSet &bs) const
{
	CBitSet	ret;

	ret= *this;
	ret&=bs;
	return ret;
}
CBitSet	CBitSet::operator|(const CBitSet &bs) const
{
	CBitSet	ret;

	ret= *this;
	ret|=bs;
	return ret;
}
CBitSet	CBitSet::operator^(const CBitSet &bs) const
{
	CBitSet	ret;

	ret= *this;
	ret^=bs;
	return ret;
}


// ***************************************************************************
void	CBitSet::flip()
{
	if(NumBits==0)
		return;

	for(sint i=0;i<(sint)Array.size();i++)
		Array[i]= ~Array[i];

	Array[Array.size()-1]&= MaskLast;
}
CBitSet	&CBitSet::operator&=(const CBitSet &bs)
{
	if(NumBits==0)
		return *this;

	sint	minSize= min(Array.size(), bs.Array.size());
	for(sint i=0;i<minSize;i++)
		Array[i]= Array[i] & bs.Array[i];
	for(i=minSize;i<(sint)Array.size();i++)
		Array[i]=0;

	Array[Array.size()-1]&= MaskLast;

	return *this;
}
CBitSet	&CBitSet::operator|=(const CBitSet &bs)
{
	if(NumBits==0)
		return *this;

	sint	minSize= min(Array.size(), bs.Array.size());
	for(sint i=0;i<minSize;i++)
		Array[i]= Array[i] | bs.Array[i];
	// Do nothing for bits word from minSize to Array.size().

	Array[Array.size()-1]&= MaskLast;

	return *this;
}
CBitSet	&CBitSet::operator^=(const CBitSet &bs)
{
	if(NumBits==0)
		return *this;

	sint	minSize= min(Array.size(), bs.Array.size());
	for(sint i=0;i<minSize;i++)
		Array[i]= Array[i] ^ bs.Array[i];
	// Do nothing for bits word from minSize to Array.size().

	Array[Array.size()-1]&= MaskLast;

	return *this;
}


// ***************************************************************************
bool	CBitSet::operator==(const CBitSet &bs) const
{
	if(NumBits!=bs.NumBits)
		return false;

	for(sint i=0;i<(sint)Array.size();i++)
	{
		if(Array[i]!=bs.Array[i])
			return false;
	}
	return true;
}
bool	CBitSet::operator!=(const CBitSet &bs) const
{
	return (!operator==(bs));
}
bool	CBitSet::compareRestrict(const CBitSet &bs) const
{
	sint	n=min(NumBits, bs.NumBits);
	if(n==0) return true;

	sint	nA= (n+BITLEN-1) / BITLEN;
	uint	mask;

	uint	nLastBits= n & (BITLEN-1) ;
	// Generate the mask for the last common word.
	if(nLastBits==0)
		mask= ~((uint)0);
	else
		mask= (1<< nLastBits) -1;


	for(sint i=0;i<nA-1;i++)
	{
		if(Array[i]!=bs.Array[i])
			return false;
	}
	if( (Array[nA-1]&mask) != (bs.Array[nA-1]&mask) )
		return false;


	return true;
}
bool	CBitSet::allSet()
{
	if(NumBits==0)
		return false;
	for(sint i=0;i<(sint)Array.size()-1;i++)
	{
		if( Array[i]!= (~((uint)0)) )
			return false;
	}
	if( Array[Array.size()-1]!= MaskLast )
		return false;
	return true;
}
bool	CBitSet::allCleared()
{
	if(NumBits==0)
		return false;
	for(sint i=0;i<(sint)Array.size();i++)
	{
		if( Array[i]!= 0 )
			return false;
	}
	return true;
}



void	CBitSet::serial(IStream &f)
{
	sint	ver=f.serialVersion(0);
	uint32	sz=0;
	vector<uint32>	array32;

	// Must support any size of uint.
	if(f.isReading())
	{
		f.serial(sz);
		resize(sz);

		f.serialCont(array32);
		for(sint i=0;i<(sint)sz;i++)
		{
			uint32	a=array32[i/32];
			a&= 1<<(i&31);
			set(i, a!=0);
		}
	}
	else
	{
		sz= size();
		f.serial(sz);

		array32.resize(sz/32);
		fill_n(array32.begin(), array32.size(), 0);
		for(sint i=0;i<(sint)sz;i++)
		{
			if(get(i))
				array32[i/32]|= 1<<(i&31);
		}
		f.serialCont(array32);
	}
}


}