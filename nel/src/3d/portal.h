/** \file portal.h
 * Definition of a cluster/portal visibility
 *
 * $Id: portal.h,v 1.3 2002/06/04 14:49:48 vizerie Exp $
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

#ifndef NL_PORTAL_H
#define NL_PORTAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/quat.h"
#include <vector>

namespace NLMISC
{
class CRGBA;
class IStream;
struct EStream;
class CMatrix;
}

namespace NL3D {

class CCluster;

/**
  * CPortal
  *
  * \author Matthieu Besson
  * \author Nevrax France
  * \date 2001
  */
class CPortal
{

public:

	CPortal();

	/**
	 * Clip the view pyramid (all planes pass through observer point) against the portal
	 * \return false if the pyramid is completly clipped
	 */
	bool clipPyramid (NLMISC::CVector &observer, std::vector<NLMISC::CPlane> &pyramid);

	/// Return true if the vertex v is in front of the portal
	bool isInFront (NLMISC::CVector &v);

	/// Accessors
	/// ---------

	/// set all link to clusters to NULL.
	void resetClusterLinks();
	
	/// return false if more than 2 clusters added
	bool setCluster (CCluster *cluster);

	/// return the number of clusters linked to the portal
	uint8 getNbCluster ();

	/// return the cluster linked to the portal
	CCluster* getCluster (uint pos) { return _Clusters[pos]; }

	/// return false if the polygon is not convex
	bool setPoly (const std::vector<NLMISC::CVector> &poly);

	/// get this cluster poly in local coordinates
	void getPoly (std::vector<NLMISC::CVector> &dest) const;

	/// Serial
	void serial (NLMISC::IStream& f);

	void setWorldMatrix (const NLMISC::CMatrix &WM);
	

	void setName (std::string &name) { _Name = name; }

	std::string getName () { return _Name; }

	void open (bool opened) { _Opened = opened; }
	bool isOpened () { return _Opened; }

private:

	CCluster* _Clusters[2];

	std::string _Name;
	bool _Opened;

	std::vector<NLMISC::CVector> _LocalPoly;
	std::vector<NLMISC::CVector> _Poly;

	/// Friend class
	friend class CInstanceGroup;
};

// ***************************************************************************



} // NL3D


#endif // NL_PORTAL_H

/* End of portal.h */
