// ItemEltAtom.h: interface for the CItemEltAtom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMELTATOM_H__429CFC50_85B1_43E5_9208_B75FEF85705B__INCLUDED_)
#define AFX_ITEMELTATOM_H__429CFC50_85B1_43E5_9208_B75FEF85705B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ItemElt.h"

class CMoldElt;
class CMoldEltType;

class CItemEltAtom : public CItemElt  
{
protected:
	CMoldEltType*	pmet;

public:
	CItemEltAtom( CLoader* const _pl );
	virtual ~CItemEltAtom();
	
	void BuildItem( CMoldElt* const _pme );

	virtual CStringEx GetFormula() const;
	virtual void SetParentValue( const CStringEx _sxparentvalue );
	virtual void SetCurrentValue( const CStringEx _sxcurrentvalue );
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	virtual unsigned int GetNbElt() const;
	
	CMoldEltType* GetMoldType() const;
};

#endif // !defined(AFX_ITEMELTATOM_H__429CFC50_85B1_43E5_9208_B75FEF85705B__INCLUDED_)
