#if !defined(AFX_SETUPFD_H__3E23C71A_C7BB_4277_8E31_7689C73C0411__INCLUDED_)
#define AFX_SETUPFD_H__3E23C71A_C7BB_4277_8E31_7689C73C0411__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetupFD.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetupFD dialog

class CSetupFD : public CDialog
{
// Construction
public:
	CSetupFD(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetupFD)
	enum { IDD = IDD_FDSETUP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetupFD)
	protected:
//	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetupFD)
	afx_msg void OnErase();
	afx_msg void OnFlashErase();
	afx_msg void OnFlashProgram();
	afx_msg void OnFlashVerify();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETUPFD_H__3E23C71A_C7BB_4277_8E31_7689C73C0411__INCLUDED_)
