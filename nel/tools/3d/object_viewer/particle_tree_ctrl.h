/** \file particle_tree_ctrl.h
 * shows the structure of a particle system
 *
 * $Id: particle_tree_ctrl.h,v 1.11 2002/04/25 12:22:54 vizerie Exp $
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


#if !defined(AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_)
#define AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include "nel/misc/matrix.h"

#include "3d/ps_particle.h"
#include "3d/ps_edit.h"
#include "3d/ps_located.h"


#include <algorithm>
#include <memory>

class CParticleDlg ;



namespace NL3D
{
	class CParticleSystem ;
	class CParticleSystemModel ;
}




/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl window

class CParticleTreeCtrl : public CTreeCtrl
{

// Construction
public:
	CParticleTreeCtrl(CParticleDlg *);
	virtual ~CParticleTreeCtrl();
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleTreeCtrl)	
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL


	/// build a portion of the tree using the given particle system
	void					buildTreeFromPS(NL3D::CParticleSystem *ps, NL3D::CParticleSystemModel *psm) ;
	/// Add a node from the given lcoated
	void					createNodeFromLocated(NL3D::CPSLocated *loc, HTREEITEM rootHandle);
	/// Add a node from the given located bindable
	void					createNodeFromLocatedBindable(NL3D::CPSLocatedBindable *lb, HTREEITEM rootHandle);
	// rebuild the located instance in the tree (after loading for example)
	void					rebuildLocatedInstance(void) ;	  
	/// suppress located instance item, so that they don't have higher index than the new size
	void					suppressLocatedInstanceNbItem(uint32 newSize) ;
	//
	void					init(void) ;
	// move the current element by using the given matrix
	void					moveElement(const NLMISC::CMatrix &mat) ;
	// get the matrix of the current element being selected, or identity if there's none
	NLMISC::CMatrix			getElementMatrix(void) const ;
	// reset the list of node in the tree (but don't delete the tree)
	void					reset();

protected:
	//{{AFX_MSG(CParticleTreeCtrl)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);	
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	CImageList _ImageList ;  // the image list containing the icons


	// the dialog that contain us
	CParticleDlg *_ParticleDlg ;

public:
	/** this struct is used to identify the type of each node	
	 */
	struct CNodeType
	{	
		enum { located, particleSystem, locatedBindable, locatedInstance  } Type ;
		union
		{
			NL3D::CPSLocated *Loc ;
			NL3D::CPSLocatedBindable *Bind ;
			NL3D::CParticleSystem *PS ;		
		} ;

		
		// if Type = particleSystem, it gives the model of this system
		NL3D::CParticleSystemModel *PSModel ;

		// for the located instance type, this is the index of the instance
		uint32 LocatedInstanceIndex ;
		
		// a located
		CNodeType(NL3D::CPSLocated *loc) { Loc = loc ; Type = located ; }

		// an instance of a located
		CNodeType(NL3D::CPSLocated *loc, uint32 index) 
		{ 
			Loc = loc ; 
			Type = locatedInstance ; 
			LocatedInstanceIndex = index ; 

				
		}
		CNodeType(NL3D::CParticleSystem *ps, NL3D::CParticleSystemModel *psModel) 
		{ 			
			Type = particleSystem ; 
			PS = ps ; 
		
			PSModel = psModel ;
		}
		CNodeType(NL3D::CPSLocatedBindable *lb) { Bind = lb ; Type = locatedBindable ; }

	} ;

private:
	// instanciate a located in the given system , and return its nodetype and htreeitem
	std::pair<CNodeType *, HTREEITEM> CParticleTreeCtrl::createLocated(NL3D::CParticleSystem *ps, HTREEITEM headItem);

	// the last ps that had a selected instance in it
	NL3D::CParticleSystem *_LastClickedPS ;
	// node that we allocated
	std::vector<CNodeType *> _NodeTypes ;

	std::auto_ptr<NL3D::CPSLocated>			_LocatedCopy;
	std::auto_ptr<NL3D::CPSLocatedBindable> _LocatedBindableCopy;


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_)
