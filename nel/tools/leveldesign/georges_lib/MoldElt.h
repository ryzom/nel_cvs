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
	bool		benum;
	bool		blist;
	CStringEx	sxname;
	CStringEx	sxfullname;
	
public:
	CMoldElt( CLoader* const _pl );
	virtual ~CMoldElt();

	CStringEx	GetName() const;													
	void SetName( const CStringEx& _sxname );
	bool IsEnum() const;
	bool IsList() const;
/*
	virtual CStringEx	GetName() const;													
	virtual void SetName( const CStringEx& _sxname );
	virtual bool IsEnum() const;
	virtual bool IsList() const;
*/
	virtual	CStringEx GetFormula();												
	virtual void Load( const CStringEx _sxfullname );
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
	virtual CMoldElt* GetMold(); 
};

#endif // !defined(AFX_MOLDELT_H__154B6055_F21A_4DCF_8B71_9DFD531A8D70__INCLUDED_)
