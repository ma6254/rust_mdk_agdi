; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
ClassCount=4
Class1=CSampTargApp
LastClass=CSetupT
NewFileInclude2=#include "SampTarg.h"
ResourceCount=5
NewFileInclude1=#include "stdafx.h"
Class2=CSetupT
LastTemplate=CDialog
Resource1=IDD_TSETUP
Class3=CTestDlg
Resource2=IDD_MODELESS
Resource3=IDD_TSETUP (English (U.S.))
Resource4=IDD_MODELESS (English (U.S.))
Class4=CSetupFD
Resource5=IDD_FDSETUP (English (U.S.))

[CLS:CSampTargApp]
Type=0
HeaderFile=SampTarg.h
ImplementationFile=SampTarg.cpp
Filter=N
LastObject=CSampTargApp

[DLG:IDD_TSETUP]
Type=1
Class=CSetupT
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_COMPORT,combobox,1344339970
Control4=IDC_BAUDRATE,combobox,1344339970
Control5=IDC_IGNCODE,button,1208156163
Control6=IDC_CACHEMEM,button,1342242819
Control7=IDC_CACHESFR,button,1208025091
Control8=IDC_STATIC,static,1342308354
Control9=IDC_STATIC,static,1342308354
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,button,1342177287

[CLS:CSetupT]
Type=0
HeaderFile=SetupT.h
ImplementationFile=SetupT.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_CACHE_XDATA

[DLG:IDD_MODELESS]
Type=1
Class=CTestDlg
ControlCount=1
Control1=IDC_STATIC,static,1342308353

[CLS:CTestDlg]
Type=0
HeaderFile=TestDlg.h
ImplementationFile=TestDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CTestDlg

[DLG:IDD_MODELESS (English (U.S.))]
Type=1
Class=CTestDlg
ControlCount=1
Control1=IDC_STATIC,static,1342308353

[DLG:IDD_TSETUP (English (U.S.))]
Type=1
Class=CSetupT
ControlCount=11
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_COMPORT,combobox,1344339970
Control4=IDC_BAUDRATE,combobox,1344339970
Control5=IDC_CACHE_DATA,button,1342242819
Control6=IDC_STATIC,static,1342308354
Control7=IDC_STATIC,static,1342308354
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,button,1342177287
Control10=IDC_CACHE_XDATA,button,1342242819
Control11=IDC_CACHE_CODE,button,1342242819

[DLG:IDD_FDSETUP (English (U.S.))]
Type=1
Class=CSetupFD
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342177294
Control5=IDC_FLASH_ERASE,button,1342242819
Control6=IDC_FLASH_PROGRAM,button,1342242819
Control7=IDC_FLASH_VERIFY,button,1342242819

[CLS:CSetupFD]
Type=0
HeaderFile=SetupFD.h
ImplementationFile=SetupFD.cpp
BaseClass=CDialog
Filter=D
LastObject=IDCANCEL
VirtualFilter=dWC

