// SampTarg.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SampTarg.h"
#include "SetupT.h"         // Target Setup Dialog

#include "Bom.h"
#include "Agdi.h"
#include "Collect.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




struct dbgblk     *pdbg;      // startup values
struct MonConf  MonConf;      // holds target-setup values 
BYTE           NotInMon;      // 1:=Remote Setup Mode via Options-Debug

/////////////////////////////////////////////////////////////////////////////
// CSampTargApp

BEGIN_MESSAGE_MAP(CSampTargApp, CWinApp)
	//{{AFX_MSG_MAP(CSampTargApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSampTargApp construction

CSampTargApp::CSampTargApp()  {
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSampTargApp object

CSampTargApp theApp;


/*
 * Analyze the command arguments 'pArg' and initialize 'MonConf'
 */

void AnalyzeMonParms (char *pPath, char *pArgs)  {
  int    i, tmp;

  strcpy (MonConf.MonPath, pPath);      // copy path
  MonConf.comnr    = 1;                 // initialize default settings
  MonConf.baudrate = 115200;            // use appropriate speed...
  MonConf.Opt      = 0x00;

  for ( i = 0 ; pArgs[i] ; )  {         // analyze arguments
    while (pArgs[i] && pArgs[i] != '-')  {   // skip character until next '-'
      ++i;
    }
    if (!pArgs[i] || pArgs[i] != '-') break;
    ++i;                                     // skip '-'
    switch (pArgs[i]) {
      case 'S':                              // ComPort number
        if (sscanf (&pArgs [i + 1], "%d", &tmp) == 1)  {
          MonConf.comnr = tmp;
        }
        break;

      case 'B':                              // BaudRate
        if (sscanf (&pArgs [i + 1], "%d", &tmp) == 1)  {
          MonConf.baudrate = tmp;
        }
        break;

      case 'O':                              // some Options
        if (sscanf (&pArgs [i + 1], "%d", &tmp) == 1)  {
          MonConf.Opt = tmp;
        }
        break;
    }
  }
}


/*
 * Create an argument string out of current 'MonConf'.  This string
 * will be registered with the current project.
 * Note: the maximum length of the string must not exceed 'ValSize-1'
 *       characters. ValSize is a member of QDLL (in Bom.h). The current
 *       maximum is 1024 characters including the zero terminator.
 */

void WriteMonParms (char *pArgs)  {
  sprintf (pArgs, "-S%d -B%d -O%d", MonConf.comnr, MonConf.baudrate, MonConf.Opt);
}




/*
 * Start the Configuration-Parameter Setup Dialog.
 * Note: this dialog *MUST* be modal, modeless is not allowed !
 */

int DoDlgSetup (void)  {
  int          i;
  CSetupT    dlg;

  i = dlg.DoModal();             // run the setup dialog...
  return (i);                    // IDOK or IDCANCEL
}


#ifdef __cplusplus
 extern "C"  {  // must avoid C++ mangled names here !
#endif

/*
 * Checker Entry for Target-Dll loader
 *  Note: When the uVision2 debugger is startet in target mode, this
 *        function is called first to check the validity of the DLL.
 *
 * The name of this function depends on architecture:
 *        80167: EnumUv3167()
 *        8051:  EnumUv351()
 *        80251: EnumUv3251()
 *        ARM7 : EnumUvARM7()   
 */

int _EXPO_ EnumUvARM7 (void *p, DWORD nCode)  {
  switch (nCode)  {
    case 0:          
      break;
    case 1:          // not used
      break;
    case 2:          // register debug block
      pdbg = (struct dbgblk *) p;  // defined in ComTyp.h
      AnalyzeMonParms (pdbg->TargDll, pdbg->TargArgs);  
      return (7);                        // return 7 for the ARM7 
  }
  return (0);                            // Ok.
}



/*
 * This function is required for Remote-Setup by Target-Options-Debug Sheet.
 */

int _EXPO_ DllUv3Cap (DWORD nCode, void *p)  {
  QDLL    *pQ;
  int       i;
  
  switch (nCode)  {
    case 2:                          // match family
      return (7);                    // ARM7  
      break;

    case 1:                          // Cpu/Target-DLL Settings
      NotInMon = 1;                  // we are just doing remote setup
      pQ = (QDLL *) p;               // refer to Bom.H for 'QDLL'
      AnalyzeMonParms (pQ->pathUv3, pQ->value);
      i = DoDlgSetup();              // start Target Setup Dialog...
      if (i == IDOK)  {              // take the values...
        WriteMonParms (pQ->value);   // create an ASCII string out of MonConf
        i = 1;
      }
      else i = 0;                    // don't register string in project.
      NotInMon = 0;
      break;
  }
  return (i);                        // return value
}


#ifdef __cplusplus
 }              // End 'extern "C"'
#endif
