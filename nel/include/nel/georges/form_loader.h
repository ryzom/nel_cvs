/** \file form_loader.h
 * Georges system files
 *
 * $Id: form_loader.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_FORM_LOADER_H
#define NLGEORGES_FORM_LOADER_H

#include "nel/georges/string_ex.h"
#include "nel/georges/form_file.h"

namespace NLGEORGES
{

// La classe CFormLoader est le point d'entrée des classes CForm pour charger une fiche.
// Il y a deux fonctions load:
//    Sans date: donne directement la dernière fiche historiquement parlant
//    Avec date: donne une fiche composée de la dernière additionnés des historiques postérieurs ou égaux à la date.
class CFormLoader  
{
protected:

public:
	CFormLoader();
	virtual ~CFormLoader();

	void LoadForm( CForm& _f, const CStringEx& _sxfilename );
	void LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ); 
	void SaveForm( CForm& _f, const CStringEx& _sxfilename );
};

} // NLGEORGES

#endif // NLGEORGES_FORM_LOADER_H
