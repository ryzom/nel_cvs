// FormBodyEltStruct.h: interface for the CFormBodyEltStruct class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMBODYELTSTRUCT_H__B6DFBDB3_61B8_46DD_89D1_F6D7CAC48393__INCLUDED_)
#define AFX_FORMBODYELTSTRUCT_H__B6DFBDB3_61B8_46DD_89D1_F6D7CAC48393__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "FormBodyElt.h"

#define SXCOMMENT "Comment"
#define SXPARENTS "Parents"

// La classe CFormBodyEltStruct permet d'obtenir un ensemble de CFormBodyElt. 
// Dans l'opérateur +=, les nouveaux élément remplacent les anciens, les anciens non-écrasés restent
class CFormBodyEltStruct : public CFormBodyElt  
{
protected:
	std::vector< CFormBodyElt* > vpbodyelt;
	std::vector< CFormBodyElt* >::iterator Find( const CStringEx _sxname );
	std::vector< CFormBodyElt* >::const_iterator Find( const CStringEx _sxname ) const;
	void Clear();

public:
	NLMISC_DECLARE_CLASS( CFormBodyEltStruct );
	CFormBodyEltStruct();
	CFormBodyEltStruct( const CFormBodyEltStruct& _fbes );
	virtual ~CFormBodyEltStruct();
	virtual void serial( NLMISC::IStream& s );

			CFormBodyElt&	operator  =( const CFormBodyEltStruct& _fbes );
	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	CStringEx GetComment() const;
	void SetComment( const CStringEx _sxcomment );
	CStringEx GetParent( unsigned int _index ) const;
	CStringEx GetActivity( unsigned int _index ) const;

	uint32 GetNbElt () const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void AddElt( CFormBodyElt* const pfbe );
};

#endif // !defined(AFX_FORMBODYELTSTRUCT_H__B6DFBDB3_61B8_46DD_89D1_F6D7CAC48393__INCLUDED_)

