/** \file pacs.cpp
 * pacs management
 *
 * $Id: pacs.cpp,v 1.3 2001/07/12 17:06:58 legros Exp $
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


#include <nel/pacs/u_retriever_bank.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_move_container.h>

#include <nel/3d/u_scene.h>
#include <nel/3d/u_visual_collision_manager.h>

#include "client.h"
#include "landscape.h"
#include "pacs.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;


URetrieverBank			*RetrieverBank;
UGlobalRetriever		*GlobalRetriever;
UMoveContainer			*MoveContainer;

UVisualCollisionManager	*VisualCollisionManager;


void	initPACS()
{
	// init the global retriever and the retriever bank
	RetrieverBank = URetrieverBank::createRetrieverBank(ConfigFile.getVar("RetrieverBankName").asString().c_str());
	GlobalRetriever = UGlobalRetriever::createGlobalRetriever(ConfigFile.getVar("GlobalRetrieverName").asString().c_str(), RetrieverBank);

	// create the move primitive
	MoveContainer = UMoveContainer::createMoveContainer(GlobalRetriever, 100, 100, 2.0);

	// create a visual collision manager
	// this should not be in pacs, but this is too close to pacs to be put elsewhere
	VisualCollisionManager = Scene->createVisualCollisionManager();
	VisualCollisionManager->setLandscape(Landscape);
}

void	releasePACS()
{
	// delete all allocated objects
	UGlobalRetriever::deleteGlobalRetriever(GlobalRetriever);
	URetrieverBank::deleteRetrieverBank(RetrieverBank);
	UMoveContainer::deleteMoveContainer(MoveContainer);

	// delete the visual collision manager
	Scene->deleteVisualCollisionManager(VisualCollisionManager);
}


