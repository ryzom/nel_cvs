/** \file pick_sound.cpp
 * Dialog used to select a sound in the sound bank.
 *
 * $Id: pick_sound.cpp,v 1.3 2001/12/18 18:39:37 vizerie Exp $
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


#include "std_afx.h"
#include "object_viewer.h"
#include "pick_sound.h"
#include "sound_system.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPickSound dialog


CPickSound::CPickSound(TNameVect &names, CWnd* pParent /*=NULL*/)
	: CDialog(CPickSound::IDD, pParent), _Names(names)
{
	
	//{{AFX_DATA_INIT(CPickSound)
	//}}AFX_DATA_INIT
}


void CPickSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickSound)
	DDX_Control(pDX, IDC_LIST1, m_NameList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPickSound, CDialog)
	//{{AFX_MSG_MAP(CPickSound)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchange)
	ON_BN_CLICKED(IDC_BUTTON1, OnPlaySound)
	ON_BN_CLICKED(IDC_PLAY_SOUND, OnPlaySound)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPickSound message handlers

BOOL CPickSound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData();
	
	for (TNameVect::const_iterator it = _Names.begin(); it	!= _Names.end(); ++it)
	{
		m_NameList.AddString(*it);
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPickSound::OnSelchange() 
{
	UpdateData();
	char str[1024];
	nlassert(m_NameList.GetTextLen(m_NameList.GetCurSel()) < 1024);
	
	m_NameList.GetText(m_NameList.GetCurSel(), str);
	_CurrName = str; 
	
}

void CPickSound::OnPlaySound() 
{
	int curSel = m_NameList.GetCurSel();
	if (curSel == LB_ERR) return;
	CString sName;
	m_NameList.GetText(curSel, sName);
	CSoundSystem::play(std::string( (LPCTSTR) sName));	
}
