// MoldElt.h: interface for the CMoldElt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDELT_H__154B6055_F21A_4DCF_8B71_9DFD531A8D70__INCLUDED_)
#define AFX_MOLDELT_H__154B6055_F21A_4DCF_8B71_9DFD531A8D70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"

class CLoader;

class CMoldElt  
{
protected:
	CLoader*	pl;
	bool		bliste;
	CStringEx	sxname;

public:
	CMoldElt( CLoader* const _pl );
	virtual ~CMoldElt();

	CStringEx	GetName();													
	void SetList( const bool _b );
	bool IsList() const;

	virtual	CStringEx GetFormula();												
	virtual void Load( const CStringEx _sxfilename );
	virtual void Load( const CStringEx _sxfilename, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
};

#endif // !defined(AFX_MOLDELT_H__154B6055_F21A_4DCF_8B71_9DFD531A8D70__INCLUDED_)
