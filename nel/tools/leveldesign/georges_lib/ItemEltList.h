// ItemEltList.h: interface for the CItemEltList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMELTLIST_H__69BC62E0_FAC1_4D3E_90ED_B34BD22790DE__INCLUDED_)
#define AFX_ITEMELTLIST_H__69BC62E0_FAC1_4D3E_90ED_B34BD22790DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "ItemElt.h"

class CItemEltAtom;
class CItemEltStruct;

class CItemEltList : public CItemElt  
{
protected:
	CItemElt* piemodel;
	std::vector< CItemElt* > vpie;
	void Clear();

public:
	CItemEltList( CLoader* const _pl );
	virtual ~CItemEltList();

	void BuildItem( CItemElt* const _pie );

	virtual CStringEx GetFormula() const;
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	virtual unsigned int GetNbElt() const;
	virtual CItemElt* GetElt( const unsigned int _index ) const;
	virtual CItemElt* GetElt( const CStringEx sxname ) const;

	void NewElt();
	void AddElt( const CItemElt* const _pie );
	void DelElt( CItemElt* const _pie );
	void VerifyName();
};

#endif // !defined(AFX_ITEMELTLIST_H__69BC62E0_FAC1_4D3E_90ED_B34BD22790DE__INCLUDED_)
