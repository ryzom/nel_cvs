/** \file interf.h
 *
 *
 * $Id: interf.h,v 1.1 2001/05/18 16:51:49 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_INTERF_H
#define NL_INTERF_H

#include "datas.h"

// Functions

void initInterf ();
void runInterf ();

void interfAddAS (CAdminService *as);
void interfAddAES (CAdminService *as, CAdminExecutorService *aes);
void interfAddS (CAdminExecutorService *aes, CService *s);

void interfUpdateAES (CAdminExecutorService *aes);
void interfUpdateS (CService *aes);

void interfRemoveS (CService *s);
void interfRemoveAES (CAdminExecutorService *aes);
void interfRemoveAS (CAdminService *as);

void setBitmap (const std::string &bitmapName, void *&bitmap);
void removeSubTree (CAdminService *as);

void removeServiceAliasPopup (CAdminExecutorService *aes);
void addServiceAliasPopup (CAdminExecutorService *aes);


#endif // NL_INTERF_H

/* End of interf.h */
