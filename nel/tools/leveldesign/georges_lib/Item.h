// Item.h: interface for the CItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEM_H__74DCC45F_FA98_4428_8C37_BA81F6B53601__INCLUDED_)
#define AFX_ITEM_H__74DCC45F_FA98_4428_8C37_BA81F6B53601__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"

class CLoader;
class CItemElt;
class CItemEltAtom;
class CItemEltStruct;
class CItemEltList;

class CItem  
{
protected:
	CLoader* pl;
	CStringEx sxlocalworkdirectory;
	CItemEltStruct* pitemes;
	CItemEltList* pitemelparents;
	CItemEltAtom* pitemeacomments;

public:
	CItem();
	virtual ~CItem();

	void Clear();
	void SetLoader( CLoader* const _pl );
	void New( const CStringEx& _sxdfnfilename );
	void Load( const CStringEx& _sxfullname );
	void Load( const CStringEx& _sxfullname, const CStringEx _sxdate ); 
	void Load( const CStringEx& _sxfullname, CStringEx& _sxlocalworkdirectory );
	void Load( const CStringEx& _sxfullname, const CStringEx& _sxlocalworkdirectory, const CStringEx _sxdate ); 
	void Save( const CStringEx& _sxfullname );

	void SetCurrentValue( const unsigned int _index, const CStringEx s );
	unsigned int GetNbElt() const;
	unsigned int GetInfos( const unsigned int _index ) const;
	CStringEx GetName( const unsigned int _index ) const;
	CStringEx GetCurrentResult( const unsigned int _index ) const;
	CStringEx GetCurrentValue( const unsigned int _index ) const;
	CStringEx GetFormula( const unsigned int _index ) const;

	CItemElt* GetElt( const unsigned int _index ) const;
	CItemElt* GetElt( const CStringEx _sxname ) const;
};

#endif // !defined(AFX_ITEM_H__74DCC45F_FA98_4428_8C37_BA81F6B53601__INCLUDED_)
