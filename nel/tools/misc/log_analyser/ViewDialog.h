/** \file ViewDialog.h
 * header file
 *
 * $Id: ViewDialog.h,v 1.1 2002/10/18 12:04:56 cado Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#include <vector>

#if !defined(AFX_VIEWDIALOG_H__FDD49815_5955_4204_8D1C_2839AE39DDB3__INCLUDED_)
#define AFX_VIEWDIALOG_H__FDD49815_5955_4204_8D1C_2839AE39DDB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewDialog dialog

class CViewDialog : public CDialog
{
// Construction
public:
	CViewDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewDialog)
	enum { IDD = IDD_View };
	CListCtrl	m_ListCtrl;
	CString	m_Caption;
	//}}AFX_DATA

	///	 Load, using the current filters
	void		reload();

	/// Load trace
	void		reloadTrace();

	/// Returns true if the string must be logged, according to the current filters
	bool		passFilter( const char *filter ) const;

	/// Resize
	void		resizeView( int index, int nbViews, int top );

	/// Clear
	void		clear();

	/// Return the nb of lines
	int			getNbLines() const;

	/// Return the nb of visible lines
	int			getNbVisibleLines() const;

	/// Set redraw state
	void		setRedraw( bool redraw ) { m_ListCtrl.SetRedraw( redraw ); }

	/// Fill from getNbLines() to maxNbLines with blank lines
	void		fillGaps( int maxNbLines );

	/// Add one line
	void		addLine( const CString &line ) { Buffer.push_back( line ); }

	/// Commit the lines previously added
	void		commitAddedLines();

	/// Scroll
	void		scrollTo( int index );

	/// Return the index of the top of the listbox
	int			getScrollIndex() const;

	/// Display string
	void		displayString();

	int						Index;
	CString					Filename;
	std::vector<CString>	PosFilter;
	std::vector<CString>	NegFilter;
	CString					LogSessionStartDate;
	bool					SessionDatePassed;
	std::vector<CString>	Buffer;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CViewDialog)
	afx_msg void OnButtonFilter();
	virtual BOOL OnInitDialog();
	afx_msg void OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWDIALOG_H__FDD49815_5955_4204_8D1C_2839AE39DDB3__INCLUDED_)
