#if !defined(AFX_SETUPT_H__9E4271DD_E521_11D3_87D6_000000000000__INCLUDED_)
#define AFX_SETUPT_H__9E4271DD_E521_11D3_87D6_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetupT.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetupT dialog

class CSetupT : public CDialog
{
// Construction
public:
	CSetupT(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetupT)
	enum { IDD = IDD_TSETUP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetupT)
protected:
//	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetupT)
	afx_msg void OnSelchangeBaudrate();
	afx_msg void OnSelchangeComport();
	afx_msg void OnCachemem();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETUPT_H__9E4271DD_E521_11D3_87D6_000000000000__INCLUDED_)
