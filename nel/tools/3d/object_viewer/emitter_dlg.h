#if !defined(AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_)
#define AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "3d/ps_particle.h"
#include "3d/ps_emitter.h"

namespace NL3D
{
	class NL3D::CPSEmitter ;
}

#include "attrib_dlg.h"

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg dialog

class CEmitterDlg : public CDialog
{
// Construction
public:
	CEmitterDlg(NL3D::CPSEmitter *emitter);   // standard constructor

	~CEmitterDlg() ;


	void init(CWnd* pParent = NULL) ;


// Dialog Data
	//{{AFX_DATA(CEmitterDlg)
	enum { IDD = IDD_EMITTER_DIALOG };
	CComboBox	m_EmissionTypeCtrl;
	CComboBox	m_EmittedTypeCtrl;
	BOOL	m_UseSpeedBasis;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmitterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the emitter being edited
	NL3D::CPSEmitter *_Emitter ;

	// the period for emission
	CAttribDlgFloat  *_PeriodDlg ;

	// the number of particles being emitted at a time
	CAttribDlgUInt  *_GenNbDlg ;

	// dlg for speed of emittee
	CAttribDlgFloat  *_StrenghtModulateDlg ;

	// dlg for speed inheritance factor
	CEditableRangeFloat  *_SpeedInheritanceFactorDlg ;


	// Generated message map functions
	//{{AFX_MSG(CEmitterDlg)
	afx_msg void OnSelchangeEmittedType();
	afx_msg void OnSelchangeTypeOfEmission();
	virtual BOOL OnInitDialog();
	afx_msg void OnUseSpeedBasis();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	/// disable / enable the period edition dialog if it is needed
	void updatePeriodDlg(void) ;

	//////////////////////////////////////////////
	//	WRAPPERS TO EDIT THE EMITTER PROPERTIES //
	//////////////////////////////////////////////



		////////////////////////
		// period of emission //
		////////////////////////

			struct tagPeriodWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSEmitter *E ;
			   float get(void) const { return E->getPeriod() ; }
			   void set(const float &v) { E->setPeriod(v) ; }
			   scheme_type *getScheme(void) const { return E->getPeriodScheme() ; }
			   void setScheme(scheme_type *s) { E->setPeriodScheme(s) ; }
			} _PeriodWrapper ;

		//////////////////////////////////////////////
		// number of particle to generate each time //
		//////////////////////////////////////////////

			struct tagGenNbWrapper : public IPSWrapperUInt, IPSSchemeWrapperUInt 
			{
			   NL3D::CPSEmitter *E ;
			   uint32 get(void) const { return E->getGenNb() ; }
			   void set(const uint32 &v) { E->setGenNb(v) ; }
			   scheme_type *getScheme(void) const { return E->getGenNbScheme() ; }
			   void setScheme(scheme_type *s) { E->setGenNbScheme(s) ; }
			} _GenNbWrapper ;

		////////////////////////////////////////////////////////
		// wrappers to emitters that have strenght modulation //
		////////////////////////////////////////////////////////

			struct tagModulateStrenghtWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSModulatedEmitter *E ;
			   float get(void) const { return E->getEmitteeSpeed() ; }
			   void set(const float &v) { E->setEmitteeSpeed(v) ; }
			   scheme_type *getScheme(void) const { return E->getEmitteeSpeedScheme() ; }
			   void setScheme(scheme_type *s) { E->setEmitteeSpeedScheme(s) ; }
			} _ModulatedStrenghtWrapper ;

		//////////////////////////////////////////////////
		// wrappers to set the speed inheritance factor //
		//////////////////////////////////////////////////

			struct tagSpeedInheritanceFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitter *E ;
			   float get(void) const { return E->getSpeedInheritanceFactor() ; }
			   void set(const float &f) { E->setSpeedInheritanceFactor(f) ; }	

			} _SpeedInheritanceFactorWrapper ;




	// contains pointers to the located
	std::vector<NL3D::CPSLocated *> _LocatedList ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_)
