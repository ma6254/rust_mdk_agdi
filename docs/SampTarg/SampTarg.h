// SampTarg.h : main header file for the SAMPTARG DLL
//

#if !defined(AFX_SAMPTARG_H__9E4271D5_E521_11D3_87D6_000000000000__INCLUDED_)
#define AFX_SAMPTARG_H__9E4271D5_E521_11D3_87D6_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSampTargApp
// See SampTarg.cpp for the implementation of this class
//

class CSampTargApp : public CWinApp
{
public:
	CSampTargApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSampTargApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSampTargApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAMPTARG_H__9E4271D5_E521_11D3_87D6_000000000000__INCLUDED_)
