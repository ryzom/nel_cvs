#if !defined(AFX_TEXTURE_ANIM_DLG_H__4A689FB0_93B9_4F5A_8075_8006D8FD19B2__INCLUDED_)
#define AFX_TEXTURE_ANIM_DLG_H__4A689FB0_93B9_4F5A_8075_8006D8FD19B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "ps_wrapper.h"
#include "value_gradient_dlg.h"

namespace NL3D
{
	class CPSTexturedParticle ;
}

class CTextureChooser ;
class CAttribDlgInt ;
class CValueGradientDlg ;

/////////////////////////////////////////////////////////////////////////////
// CTextureAnimDlg dialog

class CTextureAnimDlg : public CDialog
{
// Construction
public:
	CTextureAnimDlg(NL3D::CPSTexturedParticle *p);   // standard constructor
	~CTextureAnimDlg() ;

	void init(sint x, sint y, CWnd *pParent) ;
// Dialog Data
	//{{AFX_DATA(CTextureAnimDlg)
	enum { IDD = IDD_TEXTURE_ANIM };
	CButton	m_ChooseTextures;
	BOOL	m_EnableTextureAnim;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSTexturedParticle *_EditedParticle ;

	// dialog to choose a constant texture
	CTextureChooser *_TextureChooser ;

	// dialog to have a theme or constant value for texture id
	CAttribDlgInt *_TextureIndexDialog ;

	// Generated message map functions
	//{{AFX_MSG(CTextureAnimDlg)
	afx_msg void OnChooseTextures();
	afx_msg void OnEnableTextureAnim();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    // delete created controls
	void cleanCtrl(void) ;

	// create the right controls, depending on the fact that a an,imated texture is used or not
	void setupCtrl(void) ;


	//////////////////////////////////
	//   wrapper for single texture //
	//////////////////////////////////
			
		struct CTextureWrapper : public IPSWrapperTexture
		{
			NL3D::CPSTexturedParticle *P ;
			NL3D::ITexture *get(void)  { return P->getTexture() ; }
			void set(NL3D::ITexture *t) { P->setTexture(t) ; }
		} _TextureWrapper ;

	///////////////////////////////////////
	// wrapper for texture anim sequence //
	///////////////////////////////////////

		struct CTextureIndexWrapper : public IPSWrapper<sint32>, IPSSchemeWrapper<sint32>
		{
		   NL3D::CPSTexturedParticle *P ;		   
		   sint32 get(void) const { return P->getTextureIndex() ; }
		   void set(const sint32 &v) { P->setTextureIndex(v) ; }
		   scheme_type *getScheme(void) const { return P->getTextureIndexScheme() ; }
		   void setScheme(scheme_type *s) { P->setTextureIndexScheme(s) ; }
		} _TextureIndexWrapper ;

	/**
	 * the implementation of this struct tells the gradient dialog bow how to edit a texture list
	 */

	struct CGradientInterface : public IValueGradientDlgClient
	{	
		// the particle being edited
		CValueGradientDlg *Dlg ;
		NL3D::CPSTexturedParticle *P ;	
		// all method inherited from IValueGradientDlgClient
		virtual CEditAttribDlg *createDialog(uint index, CValueGradientDlg *grad) ;			
		virtual void modifyGradient(TAction, uint index) ;	
		virtual void displayValue(CDC *dc, uint index, sint x, sint y) ;	
		virtual uint32 getSchemeSize(void) const ;	
		virtual uint32 getNbSteps(void) const ;	
		virtual void setNbSteps(uint32 value) ;

		/////////////////////////////////////////////////
		// wrapper for the texture chooser             //
		// that allows to choose a texture in the list //
		/////////////////////////////////////////////////

		struct CTextureWrapper : public IPSWrapperTexture
		{
			CValueGradientDlg *Dlg ;
			NL3D::CPSTexturedParticle *P ;			
			// index of the particle in the list
			uint32 Index ;
			NL3D::ITexture *get(void)  ;
			void set(NL3D::ITexture *t) ;
		} _TextureWrapper ;

	} _GradientInterface ;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURE_ANIM_DLG_H__4A689FB0_93B9_4F5A_8075_8006D8FD19B2__INCLUDED_)
