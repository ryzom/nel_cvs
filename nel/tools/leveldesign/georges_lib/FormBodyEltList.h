// FormBodyEltList.h: interface for the CFormBodyEltList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMBODYELTLIST_H__48FB2EE8_0215_41F4_9591_7835BC6FA80A__INCLUDED_)
#define AFX_FORMBODYELTLIST_H__48FB2EE8_0215_41F4_9591_7835BC6FA80A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "FormBodyElt.h"

class CFormBodyEltList : public CFormBodyElt  
{
protected:
	std::vector< CFormBodyElt* > vpbodyelt;
	std::vector< CFormBodyElt* >::iterator Find( const CStringEx _sxname );
	std::vector< CFormBodyElt* >::const_iterator Find( const CStringEx _sxname ) const;

public:
	NLMISC_DECLARE_CLASS( CFormBodyEltList );
	CFormBodyEltList();
	CFormBodyEltList( const CFormBodyEltList& _fbel );
	virtual ~CFormBodyEltList();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void AddElt( CFormBodyElt* const pfbe );
	void Clear();
};

#endif // !defined(AFX_FORMBODYELTLIST_H__48FB2EE8_0215_41F4_9591_7835BC6FA80A__INCLUDED_)
