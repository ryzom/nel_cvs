// MoldEltDefine.h: interface for the CMoldEltDefine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDELTDEFINELIST_H__EE46B754_D44F_463B_A2B0_B53C46687397__INCLUDED_)
#define AFX_MOLDELTDEFINELIST_H__EE46B754_D44F_463B_A2B0_B53C46687397__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MoldEltDefine.h"
#include <vector>

class CLoader;

class CMoldEltDefineList : public CMoldEltDefine  
{
protected:
	CMoldEltDefine* pmed;

public:
	CMoldEltDefineList( CLoader* const _pl, CMoldEltDefine* const _pmed );
	virtual ~CMoldEltDefineList();

	virtual CMoldElt* GetMold(); 
	virtual void Load( const CStringEx _sxfullname );
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
};

#endif // !defined(AFX_MOLDELTDEFINELIST_H__EE46B754_D44F_463B_A2B0_B53C46687397__INCLUDED_)
