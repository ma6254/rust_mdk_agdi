// SetupT.cpp : implementation file
//

#include "stdafx.h"
#include "SampTarg.h"
#include "SetupT.h"

#include "Collect.h"        // our target local definitions



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupT dialog


CSetupT::CSetupT (CWnd *pParent) : CDialog (CSetupT::IDD, pParent)  {
	//{{AFX_DATA_INIT(CSetupT)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


#if 0      // not required.
void CSetupT::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupT)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
#endif


BEGIN_MESSAGE_MAP(CSetupT, CDialog)
	//{{AFX_MSG_MAP(CSetupT)
	ON_CBN_SELCHANGE(IDC_BAUDRATE, OnSelchangeBaudrate)
	ON_CBN_SELCHANGE(IDC_COMPORT, OnSelchangeComport)
	ON_BN_CLICKED(IDC_CACHEMEM, OnCachemem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupT message handlers

const char acomm[][7] = { "Com 1", "Com 2", "Com 3", "Com 4", };
const char arate[][7] = {
    "300",   "600",   "1200",  "2400",  "4800",  "9600", "10473",
  "11520", "12800",  "14400", "16457", "19200", "23040", "28800",
  "38400", "57600", "115200",
};
const DWORD brval[] = {
      300,     600,     1200,    2400,    4800,    9600,   10473,
    11520,   12800,    14400,   16457,   19200,   23040,   28800,
    38400,   57600,   115200,
};



void CSetupT::OnSelchangeBaudrate()  {
  CComboBox *pC;
  int         i;

  pC = (CComboBox *) GetDlgItem (IDC_BAUDRATE);
  i  = pC->GetCurSel();
  MonConf.baudrate = brval [i];
}


void CSetupT::OnSelchangeComport()  {
  CComboBox *pC;

  pC = (CComboBox *) GetDlgItem (IDC_COMPORT);
  MonConf.comnr = pC->GetCurSel() + 1;
}

void CSetupT::OnCachemem()  {
  MonConf.Opt &= ~CACHE_MEM;
  if (IsDlgButtonChecked (IDC_CACHEMEM))  {
    MonConf.Opt |= CACHE_MEM;
  }
}


void CSetupT::OnOK()  {
  CDialog::OnOK();
}

void CSetupT::OnCancel()  {
  CDialog::OnCancel();
}


BOOL CSetupT::OnInitDialog()  {
  int         i;
  CComboBox *pC;

  CDialog::OnInitDialog();

//--- Initialize COMPORT combo:
  pC = (CComboBox *) GetDlgItem (IDC_COMPORT);
  for ( i = 0 ; i < (sizeof (acomm) / sizeof (acomm[0])) ; ++i )  {
    pC->AddString (acomm [i]);                // list of Com-Ports
  }
  if (MonConf.comnr == 0) i = -1;
  else                    i = MonConf.comnr - 1;
  pC->SetCurSel (i);                          // set last used item

//--- Initialize BAUDRATE combo:
  pC = (CComboBox *) GetDlgItem (IDC_BAUDRATE);
  for ( i = 0 ; i < (sizeof (arate) / sizeof (arate[0])) ; ++i )  {
    pC->AddString (arate [i]);                // add BaudRate value strings
  }
  for ( i = 0 ; i < (sizeof (brval) / sizeof (brval[0])); i++)  {
    if (brval[i] == MonConf.baudrate) break;  // calc CurSel
  }
  pC->SetCurSel (i);                          // set last used baudrate

//--- Initialize CheckBox controls:
//CheckDlgButton (IDC_IGNCODE,  (MonConf.Opt & CACHE_CODE) ? 1 : 0);
  CheckDlgButton (IDC_CACHEMEM, (MonConf.Opt &  CACHE_MEM) ? 1 : 0);
//CheckDlgButton (IDC_CACHESFR, (MonConf.Opt &  CACHE_SFR) ? 1 : 0);

  return (TRUE);
}
