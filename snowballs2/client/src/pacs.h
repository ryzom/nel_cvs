/** \file pacs.h
 * 
 *
 * $Id: pacs.h,v 1.2 2001/07/12 17:06:58 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef PACS_H
#define PACS_H

namespace NLPACS
{
	class URetrieverBank;
	class UGlobalRetriever;
	class UMoveContainer;
};

namespace NL3D
{
	class UVisualCollisionManager;
};

extern NLPACS::URetrieverBank			*RetrieverBank;
extern NLPACS::UGlobalRetriever			*GlobalRetriever;
extern NLPACS::UMoveContainer			*MoveContainer;

extern NL3D::UVisualCollisionManager	*VisualCollisionManager;

void	initPACS();

void	releasePACS();

#endif // PACS_H

/* End of pacs.h */
