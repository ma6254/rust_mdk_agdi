#if !defined(AFX_TESTDLG_H__13FE54B1_E837_11D3_87DA_000000000000__INCLUDED_)
#define AFX_TESTDLG_H__13FE54B1_E837_11D3_87DA_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TestDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CTestDlg dialog

class CTestDlg : public CDialog  {
// Construction
public:

#if 0  //---Note: standard constructor can't be used !
	CTestDlg(CWnd* pParent = NULL);   // standard constructor
#else  // need this special modeless constructor:
    CTestDlg(DYMENU *pMen, CWnd* pParent = NULL);
#endif

//--- the following 2 functions must be present:
    void        Update (void);        // Required: Update dialog function
    DYMENU         *pM;               // Required: AGDI dialog struct
//---


// Dialog Data
	//{{AFX_DATA(CTestDlg)
	enum { IDD = IDD_MODELESS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDlg)
protected:
//	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
//---we don't need DoDataExchange-----
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
extern CTestDlg *pTestDlg;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTDLG_H__13FE54B1_E837_11D3_87DA_000000000000__INCLUDED_)
