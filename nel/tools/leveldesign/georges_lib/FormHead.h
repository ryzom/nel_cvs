// FormHead.h: interface for the CFormHead class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMHEAD_H__22D81E52_129F_4E7D_A3FC_AEB4388E2856__INCLUDED_)
#define AFX_FORMHEAD_H__22D81E52_129F_4E7D_A3FC_AEB4388E2856__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "StringEx.h"

// La classe CFormHead comprend trois strings: le nom de l'utilisateur, la date de dernière modif et la version.
class CFormHead  
{
protected:
	CStringEx	sxuser;	
	CStringEx	sxversion;	
	CStringEx	sxdate;	

public:
	CFormHead();
	CFormHead( const CFormHead& _fh );
	virtual ~CFormHead();
	void serial( NLMISC::IStream& s );
	CStringEx GetUser() const;
	CStringEx GetVersion() const;
	CStringEx GetDate() const;
	void SetUser( const CStringEx _sxuser );
	void SetVersion( const CStringEx _sxversion );
	void SetDate( const CStringEx _sxdate );

	CFormHead&	operator =( const CFormHead& _f );
};

#endif // !defined(AFX_FORMHEAD_H__22D81E52_129F_4E7D_A3FC_AEB4388E2856__INCLUDED_)
