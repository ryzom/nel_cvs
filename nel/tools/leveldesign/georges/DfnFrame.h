// DfnFrame.h : interface of the CDfnFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DFNFRAME_H__
#define __DFNFRAME_H__


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
class CDfnFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CDfnFrame)
public:
	CDfnFrame();

// Attributes
//protected:
public:
	/// the splitter window in the view
	CSplitterWnd m_wndSplitter;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDfnFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage (MSG* pMsg);

	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDfnFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CDfnFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __DFNFRAME_H__
