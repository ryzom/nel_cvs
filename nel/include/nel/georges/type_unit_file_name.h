/** \file type_unit_file_name.h
 * Georges system files
 *
 * $Id: type_unit_file_name.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_TYPE_UNIT_FILE_NAME_H
#define NLGEORGES_TYPE_UNIT_FILE_NAME_H

#include "nel/georges/type_unit.h"

namespace NLGEORGES
{

class CTypeUnitFileName : public CTypeUnit  
{
protected:
	unsigned short int	usihighlimit;

public:
	CTypeUnitFileName( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnitFileName();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
	virtual	void	SetDefaultValue( const CStringEx _sxdv );
	virtual	void	SetLowLimit( const CStringEx _sxll );
	virtual	void	SetHighLimit( const CStringEx _sxhl );
};

} // NLGEORGES

#endif // NLGEORGES_TYPE_UNIT_FILE_NAME_H
