// ItemEltStruct.h: interface for the CItemEltStruct class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMELTSTRUCT_H__22F905D3_A344_4622_8161_A9DC849AD70D__INCLUDED_)
#define AFX_ITEMELTSTRUCT_H__22F905D3_A344_4622_8161_A9DC849AD70D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ItemElt.h"

class CMoldElt;
class CMoldEltDefine;
class CFormBodyEltStruct;

class CItemEltStruct : public CItemElt  
{
protected:
	CMoldEltDefine*	pmed;
	std::vector< CItemElt* > vpie;
	void Clear();

public:
	CItemEltStruct( CLoader* const _pl );
	virtual ~CItemEltStruct();
	
	void BuildItem( CMoldElt* const _pme );
	
	virtual CStringEx GetFormula() const;
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();
	void BuildForm( CFormBodyEltStruct* pfbes );
	virtual unsigned int GetNbElt() const;
	virtual CItemElt* GetElt( const unsigned int _index ) const;
	virtual CItemElt* GetElt( const CStringEx sxname ) const;
};

#endif // !defined(AFX_ITEMELTSTRUCT_H__22F905D3_A344_4622_8161_A9DC849AD70D__INCLUDED_)
