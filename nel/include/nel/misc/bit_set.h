/* bit_set.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: bit_set.h,v 1.2 2000/10/06 10:41:08 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_BIT_SET_H
#define NL_BIT_SET_H


#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"


namespace	NLMISC
{


// ***************************************************************************
/**
 * A BitSet, to test / set flags quickly.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CBitSet
{
public:
	/// \name Object.
	//@{
	CBitSet();
	CBitSet(uint numBits);
	CBitSet(const CBitSet &bs);
	~CBitSet();
	CBitSet	&operator=(const CBitSet &bs);
	//@}

	/// \name Basics.
	//@{
	/// Resize the bit array. All Bits are reseted.
	void	resize(uint numBits);
	/// Clear the bitarray so size() return 0.
	void	clear();
	/// Return size of the bit array.
	uint	size() const;
	/// Set a bit to 0 or 1.
	void	set(sint bitNumber, bool value);
	/// Get the value of a bit.
	bool	get(sint bitNumber) const;
	/// Get the value of a bit.
	bool	operator[](sint bitNumber) const;
	/// Set a bit to 1.
	void	set(sint bitNumber) {set(bitNumber, true);}
	/// Set a bit to 0.
	void	clear(sint bitNumber) {set(bitNumber, false);}
	/// Set all bits to 1.
	void	setAll();
	/// Set all bits to 0.
	void	clearAll();
	//@}


	/// \name Bit operations.
	//@{
	/// Return The bitarray NOTed.
	CBitSet	operator~() const;
	/**
	 * Return this ANDed with bs.
	 * The result BitSet is of size of \c *this. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	operator&(const CBitSet &bs) const;
	/**
	 * Return this ORed with bs.
	 * The result BitSet is of size of \c *this. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	operator|(const CBitSet &bs) const;
	/**
	 * Return this XORed with bs.
	 * The result BitSet is of size of \c *this. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	operator^(const CBitSet &bs) const;

	/// NOT the BitArray.
	void	flip();
	/**
	 * AND the bitArray with bs.
	 * The bitset size is not changed. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	&operator&=(const CBitSet &bs);
	/**
	 * OR the bitArray with bs.
	 * The bitset size is not changed. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	&operator|=(const CBitSet &bs);
	/**
	 * XOR the bitArray with bs.
	 * The bitset size is not changed. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	&operator^=(const CBitSet &bs);
	//@}


	/// \name Bit comparisons.
	//@{
	/** 
	 * Compare two BitSet not necessarely of same size. The comparison is done on N bits, where N=min(this->size(), bs.size())
	 * \return true if the N common bits of this and bs are the same. false otherwise.
	 */
	bool	compareRestrict(const CBitSet &bs) const;
	/// Compare two BitSet. If not of same size, return false.
	bool	operator==(const CBitSet &bs) const;
	/// operator!=.
	bool	operator!=(const CBitSet &bs) const;
	/// Return true if all bits are set. false if size()==0.
	bool	allSet();
	/// Return true if all bits are cleared. false if size()==0.
	bool	allCleared();
	//@}


	void	serial(NLMISC::IStream &f);

private:
	std::vector<uint>	Array;
	sint			NumBits;
	uint			MaskLast;	// Mask for the last uint.
};


}



#endif // NL_BIT_SET_H

/* End of bit_set.h */
