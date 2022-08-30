// SetupFD.cpp : implementation file
//

#include "stdafx.h"
#include "SampTarg.h"
#include "SetupFD.h"

#include "Collect.h"        // our target local definitions



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupFD dialog


CSetupFD::CSetupFD(CWnd* pParent /*=NULL*/)
	: CDialog(CSetupFD::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetupFD)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


#if 0      // not required.
void CSetupFD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupFD)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
#endif

BEGIN_MESSAGE_MAP(CSetupFD, CDialog)
	//{{AFX_MSG_MAP(CSetupFD)
	ON_BN_CLICKED(IDC_FLASH_ERASE, OnFlashErase)
	ON_BN_CLICKED(IDC_FLASH_PROGRAM, OnFlashProgram)
	ON_BN_CLICKED(IDC_FLASH_VERIFY, OnFlashVerify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupFD message handlers

void CSetupFD::OnFlashErase() 
{
  MonConf.Opt &= ~FLASH_ERASE;
  if (IsDlgButtonChecked (IDC_FLASH_ERASE))  {
    MonConf.Opt |= FLASH_ERASE;
  }
}


void CSetupFD::OnFlashProgram() 
{
  MonConf.Opt &= ~FLASH_PROGRAM;
  if (IsDlgButtonChecked (IDC_FLASH_PROGRAM))  {
    MonConf.Opt |= FLASH_PROGRAM;
  }
}


void CSetupFD::OnFlashVerify() 
{
  MonConf.Opt &= ~FLASH_VERIFY;
  if (IsDlgButtonChecked (IDC_FLASH_VERIFY))  {
    MonConf.Opt |= FLASH_VERIFY;
  }
}


void CSetupFD::OnOK() {
  CDialog::OnOK();
}


void CSetupFD::OnCancel() {
  CDialog::OnCancel();
}


BOOL CSetupFD::OnInitDialog() 
{
  CDialog::OnInitDialog();
	
//--- Initialize CheckBox controls:
  CheckDlgButton (IDC_FLASH_ERASE,   (MonConf.Opt & FLASH_ERASE) ? 1 : 0);
  CheckDlgButton (IDC_FLASH_PROGRAM, (MonConf.Opt & FLASH_PROGRAM) ? 1 : 0);
  CheckDlgButton (IDC_FLASH_VERIFY,  (MonConf.Opt & FLASH_VERIFY) ? 1 : 0);
	
  return (TRUE); 
}

