// ItemElt.h: interface for the CItemElt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMELT_H__5AC5A067_71CC_4D98_904F_4877F99F4D9A__INCLUDED_)
#define AFX_ITEMELT_H__5AC5A067_71CC_4D98_904F_4877F99F4D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"
#include "Common.h"

class CLoader;
class CFormBodyElt;

class CItemElt  
{
protected:
	CLoader*	pl;
	unsigned int	infos;
	CStringEx		sxname;	
	CStringEx		sxparent;	
	CStringEx		sxcurrentresult;	
	CStringEx		sxcurrentvalue;	
	CStringEx		sxoldcurrentvalue;	
	CStringEx		sxparentresult;	
	CStringEx		sxparentvalue;	
	CStringEx		sxoldparentvalue;	

public:
	CItemElt( CLoader* const _pl );
	virtual ~CItemElt();

	void SetName( const CStringEx _sxname );

	unsigned int GetInfos() const;
	CStringEx GetName() const;
	CStringEx GetParent() const;
	CStringEx GetParentResult() const;
	CStringEx GetParentValue() const;
	CStringEx GetCurrentResult() const;
	CStringEx GetCurrentValue() const;

	virtual CStringEx GetFormula() const;
	virtual void SetParentValue( const CStringEx _sxparentvalue );
	virtual void SetCurrentValue( const CStringEx _sxcurrentvalue );
	virtual void FillParent( const CFormBodyElt* const _pfbe );
	virtual void FillCurrent(const  CFormBodyElt* const _pfbe );
	virtual CItemElt* Clone();
	virtual CFormBodyElt* BuildForm();

	virtual unsigned int GetNbElt() const;
	virtual CItemElt* GetElt( const unsigned int _index ) const;
	virtual CItemElt* GetElt( const CStringEx sxname ) const;
};

#endif // !defined(AFX_ITEMELT_H__5AC5A067_71CC_4D98_904F_4877F99F4D9A__INCLUDED_)
