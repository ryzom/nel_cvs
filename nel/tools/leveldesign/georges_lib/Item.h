// Item.h: interface for the CItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEM_H__74DCC45F_FA98_4428_8C37_BA81F6B53601__INCLUDED_)
#define AFX_ITEM_H__74DCC45F_FA98_4428_8C37_BA81F6B53601__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"
#include "form.h"

class CLoader;
class CItemElt;
class CItemEltAtom;
class CItemEltStruct;
class CItemEltList;

class CItem  
{
protected:
	CLoader* pl;
	CItemEltStruct* pitemes;
//	CItemEltList* pitemelparents;
//	CItemEltAtom* pitemeacomments;
	std::vector< std::pair< CStringEx, CStringEx > > vsxparents;
	CStringEx moldfilename;

public:
	CItem();
	virtual ~CItem();

	void Clear();
	void SetLoader( CLoader* const _pl );
	void New( const CStringEx& _sxdfnfilename );
	void Load( const CStringEx& _sxfilename );
	void Load( const CStringEx& _sxfilename, const CStringEx _sxdate ); 
	void Save( const CStringEx& _sxfilename );
	bool Update();

	// Convert CItem to a CForm (in is this)
	void MakeForm (CForm &out);
	// Convert CForm to CItem (out is this)
	void MakeItem (CForm &in);

	void SetCurrentValue( const unsigned int _index, const CStringEx s );
	unsigned int GetNbElt() const;
	unsigned int GetNbParents() const;
	unsigned int GetNbElt( const unsigned int _index ) const;
	unsigned int GetInfos( const unsigned int _index ) const;
	CStringEx GetName( const unsigned int _index ) const;
	CStringEx GetCurrentResult( const unsigned int _index ) const;
	CStringEx GetCurrentValue( const unsigned int _index ) const;
	CStringEx GetFormula( const unsigned int _index ) const;
	bool IsEnum( const unsigned int _index ) const; 
	bool IsPredef( const unsigned int _index ) const; 
	bool CanEdit( const unsigned int _index ) const; 
	void GetListPredef( const unsigned int _index, std::vector< CStringEx >& _vsx ) const;

	CItemElt* GetElt( const unsigned int _index ) const;
	CItemElt* GetElt( const CStringEx _sxname ) const;

	void AddList( const unsigned int _index ) const;
	void DelListChild( const unsigned int _index ) const;
	void VirtualSaveLoad();

	void AddParent( const unsigned int _index );
	void DelParent( const unsigned int _index );
	CStringEx GetParent( const unsigned int _index ) const;
	void SetParent( const unsigned int _index, const CStringEx _sx );
	CStringEx GetActivity( const unsigned int _index ) const;
	void SetActivity( const unsigned int _index, const CStringEx _sx );
};

#endif // !defined(AFX_ITEM_H__74DCC45F_FA98_4428_8C37_BA81F6B53601__INCLUDED_)
