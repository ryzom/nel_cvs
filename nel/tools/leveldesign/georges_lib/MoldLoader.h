// MoldLoader.h: interface for the CMoldLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDLOADER_H__5E812FFC_E18D_45A4_A955_703EB2F8DFC3__INCLUDED_)
#define AFX_MOLDLOADER_H__5E812FFC_E18D_45A4_A955_703EB2F8DFC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"
#include < map >

class CMoldElt;
class CLoader;

class CMoldLoader  
{
protected:
//	std::map< CStringEx, CMoldElt* >;
	CLoader* pl;	

public:
	CMoldLoader();
	virtual ~CMoldLoader();

	void SetLoader( CLoader* const _pl );
	CMoldElt* LoadMold( const CStringEx _sxfilename );
	CMoldElt* LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ); 
	// Vérifie si la fiche ne serait pas déja chargé...
	// Plus tard, lock le fichier, ect...
};

#endif // !defined(AFX_MOLDLOADER_H__5E812FFC_E18D_45A4_A955_703EB2F8DFC3__INCLUDED_)
