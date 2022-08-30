// TestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SampTarg.h"

#include "Agdi.h"
#include "TestDlg.h"

#include "Collect.h"          // need AGDI's 'pHwnd'



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CTestDlg   *pTestDlg;


/*
 * AGDI needs the following 3 functions to link and handle
 * the modeless dialog within uVision2:
 */

void MdUpdate (void)  {               // (1) Dialog Update function
  if (pTestDlg) pTestDlg->Update();
}

void MdKill (DIAD *pM)  {             // (2) Dialog Kill function
  if (pTestDlg != NULL)  {            // Dialog is still active
    pTestDlg->SendMessage (WM_CLOSE);
    pTestDlg  = NULL;                 // clear global object pointer
    pM->iOpen = 0;                    // clear AGDI's dlg variables
    pM->hw    = NULL;
  }
}

void MdShow (DYMENU *pM)  {           // (3) Dialog Show/Hide function
  if (pM->pDlg->hw != NULL)  {           // Dialog is already open
    MdKill (pM->pDlg);                   // close it.
  }
  else  {                                // Dialog is not created
    pTestDlg = new CTestDlg (pM, NULL);  // create it
    if (pTestDlg != NULL)  {             // construction was Ok.
      pM->pDlg->hw = pTestDlg->m_hWnd;
    }
  }
}



/////////////////////////////////////////////////////////////////////////////
// CTestDlg dialog


#if 0  // standard modal constructor can't be used.
CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
#else

//--- Special constructor for AGDI's modeless dialogs:
CTestDlg::CTestDlg (DYMENU *pMenu, CWnd *pWnd)  {
  pM = pMenu;                   // save DYM descriptor in our dialog object.
  Create (IDD_MODELESS, pWnd);  // create the dialog
  pTestDlg = this;              //
}

#endif


#if 0    // not required.
void CTestDlg::DoDataExchange(CDataExchange* pDX)  {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
#endif


BEGIN_MESSAGE_MAP(CTestDlg, CDialog)
	//{{AFX_MSG_MAP(CTestDlg)
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDlg message handlers

void CTestDlg::PostNcDestroy()  {
  delete this;                    // delete the new'ed object
}


/*
 * Dialog will be closed
 */

void CTestDlg::OnClose()  {
  GetWindowRect (&pM->pDlg->rc);  // save Window position
  pM->pDlg->hw = NULL;            // clear m_hWnd
  DestroyWindow();                // ends up with PostNcDestroy().
}


/*
 * Dialog controls need to be update
 */

void CTestDlg::Update (void)  {
//---TODO: update all dialog controls to reflect the 'current' values.
}


/*
 * OnActivate is required to link the modeless dialog correctly
 * into the uVision2 chain for proper message pumping.
 */

void CTestDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)  {
  CDialog::OnActivate(nState, pWndOther, bMinimized);
	
  switch (nState)  {
    case WA_INACTIVE:
      *pHwnd = NULL;             // clear active modeless handle
      break;
    case WA_ACTIVE:
    case WA_CLICKACTIVE:
      *pHwnd = m_hWnd;           // set modeless dialog handle
      break;
  }
}



//
// Important Note:
//   Make sure that the 'Visible' style is set for the dialog resource !
//   This is required for modeless dialogs. Otherwise the dialog will
//   always be invisible !
//

BOOL CTestDlg::OnInitDialog()  {
  CDialog::OnInitDialog();

//--- restore previous window position if possible, otherwise center only.
  RECT rc = pM->pDlg->rc;
  if (rc.left != -1)  {                        // restore Window position
    MoveWindow (rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);;
  }
  else CenterWindow();	
//---
	
// TODO: Add extra initialization here
	
  return (TRUE);
}
