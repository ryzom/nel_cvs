/** \file pick_sound.h
 * Dialog used to select a sound in the sound bank.
 *
 * $Id: pick_sound.h,v 1.5 2002/11/04 15:40:45 boucher Exp $
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


#if !defined(AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_)
#define AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// pick_sound.h : header file
//

#include <vector>
#include <string>

/////////////////////////////////////////////////////////////////////////////
// CPickSound dialog

class CPickSound : public CDialog
{
// Construction
public:
	typedef std::vector<const char *> TNameVect;
	CPickSound(const std::vector<std::string> &names, CWnd* pParent = NULL);   // standard constructor


	const std::string &getName(void) const { return _CurrName; }

// Dialog Data
	//{{AFX_DATA(CPickSound)
	enum { IDD = IDD_PICK_SOUND };
	CListBox	m_NameList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickSound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//TNameVect _Names;
	std::vector<std::string>	_Names;
	std::string _CurrName;

	UINT _Timer;

	NLMISC::CVector _BackupVel;
	float _BackupGain;

	// Generated message map functions
	//{{AFX_MSG(CPickSound)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchange();
	afx_msg void OnPlaySound();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_)
