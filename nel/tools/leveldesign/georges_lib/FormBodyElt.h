// FormBodyElt.h: interface for the CFormBodyElt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMBODYELT_H__022AE626_4DC7_4F7F_8C0B_C3AEA136F264__INCLUDED_)
#define AFX_FORMBODYELT_H__022AE626_4DC7_4F7F_8C0B_C3AEA136F264__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "StringEx.h"

// La classe CFormBodyElt est la classe de base pour le polymorphisme des CFormBodyEltAtom, CFormBodyEltList, CFormBodyEltStruct
// Elle comprend le nom de la branche ou de la feuille.
class CFormBodyElt : public NLMISC::IStreamable  
{
protected:
	CStringEx	sxname;

public:
	NLMISC_DECLARE_CLASS( CFormBodyElt );
	CFormBodyElt();
	CFormBodyElt( const CFormBodyElt& _fbe );
	virtual ~CFormBodyElt();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;

	CStringEx GetName() const;
	virtual CStringEx GetValue() const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void SetName( const CStringEx _sxname );
};

#endif // !defined(AFX_FORMBODYELT_H__022AE626_4DC7_4F7F_8C0B_C3AEA136F264__INCLUDED_)
