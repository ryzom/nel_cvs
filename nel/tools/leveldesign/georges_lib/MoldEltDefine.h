// MoldEltDefine.h: interface for the CMoldEltDefine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDELTDEFINE_H__EE46B754_D44F_463B_A2B0_B53C46687397__INCLUDED_)
#define AFX_MOLDELTDEFINE_H__EE46B754_D44F_463B_A2B0_B53C46687397__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MoldElt.h"
#include <vector>

class CLoader;

class CMoldEltDefine : public CMoldElt  
{
protected:
	std::vector< std::pair< CStringEx, CMoldElt* > > vpair;

public:
	CMoldEltDefine( CLoader* const _pl );
	virtual ~CMoldEltDefine();

	virtual CMoldElt* GetMold(); 
	virtual void Load( const CStringEx _sxfilename );
	virtual void Load( const CStringEx _sxfilename, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
};

#endif // !defined(AFX_MOLDELTDEFINE_H__EE46B754_D44F_463B_A2B0_B53C46687397__INCLUDED_)
