#if !defined(AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_)
#define AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// texture_chooser.h : header file
//


#include "nel/misc/smart_ptr.h"
#include "3d/texture.h"

#include "ps_wrapper.h"

using NLMISC::CSmartPtr ;




/////////////////////////////////////////////////////////////////////////////
// CTextureChooser dialog

class CTextureChooser : public CDialog
{
// Construction
public:
	// construct the object with the given texture
	CTextureChooser();   // standard constructor

	~CTextureChooser();

	void init(uint x, uint y, CWnd *pParent = NULL) ;



	// set a wrapper to get the datas
	void setWrapper(IPSWrapperTexture *wrapper) { _Wrapper = wrapper ; }

// Dialog Data
	//{{AFX_DATA(CTextureChooser)
	enum { IDD = IDD_TEXTURE_CHOOSER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureChooser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IPSWrapperTexture *_Wrapper ;

	// handle to the current bitmap being displayed
	HBITMAP _CurrBitmap ;

	// update the current bitmap
	void CTextureChooser::textureToBitmap() ;

	// the current texture
	CSmartPtr<NL3D::ITexture> _Texture ;

	// Generated message map functions
	//{{AFX_MSG(CTextureChooser)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseTexture();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_)
