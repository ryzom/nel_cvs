// Form.h: interface for the CForm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORM_H__2E1EC870_75E2_4FB6_AC69_513BC146D386__INCLUDED_)
#define AFX_FORM_H__2E1EC870_75E2_4FB6_AC69_513BC146D386__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "FormHead.h"
#include "FormBodyEltStruct.h"

class CFormBodyElt;

class CForm  
{
protected:
	CFormHead			head;
	CFormBodyEltStruct	body;
	bool				bmodified;

public:
	CForm();
	CForm( const CForm& _f );
	virtual ~CForm();

	void serial( NLMISC::IStream& s );
	
	CStringEx GetDate() const;
	CStringEx GetVersion() const;
	CStringEx GetUser() const;
	CStringEx GetComment() const;
	CStringEx GetParent( unsigned int _index ) const;
	CFormBodyElt* GetElt( const CStringEx _sxname ) const;
	CFormBodyEltStruct* GetBody() const;
	bool GetModified() const;
	void SetModified( const bool _b );
	
	CForm&	operator  =( const CForm& _f );
	CForm&	operator +=( const CForm& _f );
	CForm&	operator -=( const CForm& _f );
	bool	operator ==( const CForm& _f ) const;
};

#endif // !defined(AFX_FORM_H__2E1EC870_75E2_4FB6_AC69_513BC146D386__INCLUDED_)
