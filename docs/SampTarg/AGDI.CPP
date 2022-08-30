#include "stdafx.h"
#include "SampTarg.h"

#define _IN_TARG_                   // define if used within target
#include "Agdi.h"
#include "Bom.h"
#include "ComTyp.h"
#include "Collect.h"
#include <stdio.h>
#include "resource.h"



RG51                REG51;          // 8051 Registers
BYTE             initflag;
BYTE             bootflag;
BYTE             PlayDead;          // Disables the driver after comm breaks down.
BYTE          RegUpToDate;          // Flag: mark driver is 'dead' (disconnected)
BYTE            FlashProg;          // Debugger started in Flash Programming mode

static OIL    ProgressBar;          // Progress-Bar data
int               NumRecs;          // number of trace records

/*
 * AGDI-Data
 */

       SUPP             supp;       // supported features
       UC8              iRun;       // target is currently executing
       UC8           StopRun;       // Flag: stop execution
       UC8            GoMode;
       AG_BP        **pBhead;
       UL32          *pCURPC;
       UL32            curPC;

       AG_BP            *pBX;       // used for temporary Breakpoint
       UL32          abreaks;       // number of code-address Bp's
       UL32          cbreaks;       // number of conditional Bp's
       UL32          wbreaks;       // number of access Bp's
       UL32      SetBrkCount;
static const char szNBP[] = "*** AGDI-Error: can't setup break at 0x%08X\n";


       UL32           Uv2Msg;       // Uv2-registered Message token
       pCBF          pCbFunc;       // call-back function of s166
extern DYMENU         Menu[];       // forward

// if an extension modeless dialog is activated, then '*pHwnd' *MUST*
// receive the HWND (m_hWnd in MFC) of the dialog.  If the dialog
// looses focus or is closed, then '*pHwnd' *MUST* be set to NULL.
// This is to ensure the proper message chain within Uv2.

       HWND          *pHwnd;
       HWND           hMfrm;        // parent handle (CMainFrame)
       HMODULE        hInst;        // this DLL's instance handle

extern void InitRegs (void);        // forward
int   FlashLoad (void);             // forward

/*
 * Initialize uVision2 Progress bar
 */

static void InitProgress (char *msg)  {
  ProgressBar.Job   = PROGRESS_INIT;
  ProgressBar.pos   = 0;
  ProgressBar.low   = 0;
  ProgressBar.hig   = 100;
  ProgressBar.label = (SC8 *)msg;
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);
}

/*
 * Kill uVision2 Progress bar
 */

static void StopProgress (void)  {
  ProgressBar.Job = PROGRESS_KILL;
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);
}

/*
 * Set Progress bar
 */

static void SetProgress (DWORD pos )  {
  if (pos > 100) pos = 100;
  ProgressBar.Job = PROGRESS_SETPOS;
  ProgressBar.pos = pos;
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);
}


//--- Memory cache handling.
//--- Simulate the (non-existing) target memory
// Memory caching is used to increase debugger performance. 
// When uVision2 requests data, it is taken out from cache memory buffer
// instead of reading it from the target hardware. This performance
// is increased slightly if fast interface is used (USB, Ethernet,...) 

struct MMM  mslots [520];           // cache memory slots

/*
 * Free all allocated memory by this driver before driver shutdown.
 */

static void FreeCache (void)  {
  struct MMM *s;
  int         i;

  for ( i = 0 ; i < (sizeof (mslots) / sizeof (mslots[0])) ; ++i )  {
    s = &mslots [i];                 // a16-a23
    if (s->mem)  {
      free (s->mem);        
      free (s->atr);
      s->mem = NULL;
      s->atr = NULL;
    }
  }
//---TODO: free additional memory resources, if allocated
}


/*
 * Map full address to Cache-Slot-number:
 */

static DWORD SlotNo (DWORD n)  {
  DWORD   z;

  switch (n >> 24)  {                 // high byte of address holds mSpace.
    case amDATA:
      if ((n & 0xFF) >= 0x80)  {      // SFR-address
        n = 0x200;                    // sfr slot at 0x200
        break;
      }
    case amIDATA:
      n = 0x201;                      // idata slot at 0x201
      break;
    case amPDATA:
    case amXDATA:
      n = 0x100 + ((n >> 16) & 0xFF); // xdata slots start at 0x0100
      break;
    default:                          // code   
      z = (n >> 16) & 0xFF;
      if (z >= amBANK0 && z <= amBANK31)  {
        n = z;                        // banking: slots 0x80...0x9F
      }
      else  {                         // code slot (0x00...0xFF)
        n = z;
      }
      break;
  }
  return (n);
}


/*
 * Cache data
 */

void CacheData (BYTE *pB, DWORD nAdr, DWORD nCnt)  {
  U16       *pA;
  BYTE      *pM;
  DWORD   nSlot, aMask;

  if (pB == NULL || nCnt == 0) return;
  for ( ; nCnt != 0 ; --nCnt, ++nAdr, ++pB )  {
    nSlot = SlotNo (nAdr);    
    switch (nAdr >> 24)  {
      case amDATA:
      case amPDATA:
      case amIDATA:
        aMask = 0xFF;
        break;
      default:
        aMask = 0xFFFF;
        break;
    }
    pA = mslots [nSlot].atr;
    pM = mslots [nSlot].mem;
    if (!pA || !pM) continue;            // both of them need to be valid
    pA [nAdr & aMask] |= AG_ATR_UPTD;    // 'up to date' attribute
    pM [nAdr & aMask]  = *pB;            // the data
  }
}


/*
 * clear 'AG_ATR_UPDT' for the given range
 */

void ClearCaR (DWORD nAdr, DWORD nCnt)  {
  U16       *pA;
  DWORD   nSlot, aMask;

  for ( ; nCnt != 0 ; --nCnt, ++nAdr )  {
    nSlot = SlotNo (nAdr);    
    switch (nAdr >> 24)  {
      case amDATA:
      case amPDATA:
      case amIDATA:
        aMask = 0xFF;
        break;
      default:
        aMask = 0xFFFF;
        break;
    }
    pA = mslots [nSlot].atr;
    if (pA != NULL)  {                     // attributes are valid.
      pA [nAdr & aMask] &= ~AG_ATR_UPTD;   // clear 'up to date' attribute
    }
  }
}



/*
 * Check if chached data is valid.
 */

int CacheValid (BYTE *pB, DWORD nAdr, DWORD nCnt)  {
  int        n1, n2;
  U16       *pA;
  BYTE      *pM;
  DWORD   nSlot,aMask;

  if (pB == NULL) return (0);

  switch (nAdr >> 24) {
    case amDATA:
      if ((nAdr & 0xFF) < 0x80) break;  // Cache internal DATA
      return (0);                       // SFRs are not cached

    case amIDATA:                       // Cache DATA, IDATA
      if (MonConf.Opt & CACHE_DATA) break;
      return (0);

    case amXDATA:                       // Cache XDATA ram
      if (MonConf.Opt & CACHE_XDATA) break;
      return (0);

    case amCODE:                        // Cache CODE mem.
      if (MonConf.Opt & CACHE_CODE) break;
      return (0);
  }    
    
  n1 = nCnt;
  for ( n2 = 0 ; n1 != 0 ; ++n2, --n1, ++nAdr )  {
    nSlot = SlotNo (nAdr);    
    switch (nAdr >> 24)  {
      case amDATA:
      case amPDATA:
      case amIDATA:
        aMask = 0xFF;
        break;
      default:
        aMask = 0xFFFF;
        break;
    }
    pA = mslots [nSlot].atr;
    pM = mslots [nSlot].mem;
    if (!pA || !pM) return (0);          // both of them need to be valid
    if (!(pA [nAdr & aMask] & AG_ATR_UPTD))  {
      return (0);                        // cache data invalid.
    }
    pB [n2] = pM [nAdr & aMask];         // copy data to user buffer
  }
  return (1);                            // cached data valid.
}


/*
 * Map Memory
 */

static void MapSeg (DWORD nSeg, DWORD nSof, DWORD nLen, WORD16 nAtr)  {
  struct MMM   *s;
  WORD16      *pA;
  DWORD         n;

  if (nLen == 0) return;
  n = SlotNo (nSeg);
  s = &mslots [n];
  if (s->mem == NULL)  {                 // if not already mapped...
    s->mem = (UC8 *) calloc (_MSGM, 1);       // allocate 64k block
    s->atr = (U16 *) calloc (_ASGM, 1);       // and associated attributes
    if (s->mem == NULL || s->atr == NULL)  {  // malloc failed.
      s->mem = NULL;
      s->atr = NULL;
      return;
    }
  }

  pA = &s->atr [nSof];
  for ( n = 0 ; n < nLen ; ++n, ++pA )  {     // initialize attribs
    *pA &= (AG_ATR_READ | AG_ATR_WRITE | AG_ATR_EXEC | AG_ATR_EXTR | AG_ATR_UPTD);
    *pA |= nAtr;
  }
}


/*
 * Map a cache segment descriptor.
 */

DWORD MMapMem (UL32 nAtr, UL32 nAdr, UL32 nLen)  {
  if (nAtr == 0)  {
    nAtr = AG_ATR_READ | AG_ATR_WRITE | AG_ATR_EXEC;
  }
  MapSeg (nAdr, nAdr & 0xFFFFFF, nLen, (U16) nAtr);
  return (0);
}


/*
 * Get Pointer to Attribute Segment for address 'nAdr'
 */

U16 *MGetAttrSeg (UL32 nAdr)  {
  U16     *pA;
  DWORD     n;

  n  = SlotNo (nAdr);           // map address to cache slot
  pA = mslots [n].atr;          // and return the attribute segment.
  return (pA);
}


/*
 * Get Pointer to Attributes for address 'nAdr'
 */

U16 *MGetAttr (UL32 nAdr)  {
  U16     *pA;

  pA = MGetAttrSeg (nAdr);      // get attribute segment
  if (pA != NULL)  {
    pA = &pA [nAdr & 0xFFFF];   // address of attributes for address 'nAdr'
  }
  return (pA);
}


void WriteAttr (UL32 nAdr, UL32 aMask, UL32 oMask, UL32 nMany)  {
  U16    *pA;

  for ( ; nMany != 0 ; --nMany, ++nAdr )  {
    pA = MGetAttrSeg (nAdr);
    if (pA == NULL) continue;
    *pA &= ~aMask;
    *pA |=  oMask;
  }
}


static void InitMem (void)  {
  MapSeg (amDATA,    0x00,   0x100, AG_ATR_READ | AG_ATR_WRITE);
  MapSeg (amIDATA,   0x80,    0x80, AG_ATR_READ | AG_ATR_WRITE);
  MapSeg (amXDATA, 0x0000, 0x10000, AG_ATR_READ | AG_ATR_WRITE);
  MapSeg (amCODE,  0x0000, 0x10000, AG_ATR_READ | AG_ATR_EXEC);
//---TODO:
//---add cache buffer for additional memory space here if needed
}

//--- End of c/i-chache


/*
 * Output a message line into uVision2's command window in debug mode
 * and to build window in Flash download mode
 */

void txtout (char *fmt, ...)  {
  va_list   marker;
  char      txtbuf [2048];

  va_start (marker, fmt);
  vsprintf (&txtbuf[0], fmt, marker);
  SendMessage (hMfrm,                // for Flash Download send message to Build window 
               Uv2Msg, FlashProg ? MSG_UV2_TRNLINE : MSG_UV2_CMDLINE,
              (LPARAM) &txtbuf[0]);
}




/*
 * This function is required for Remote-Setup in uVision2 Debug mode
 */

static void ConfDisp (DYMENU *pM)  {
  struct MonConf oCnf;
  int     i;

  pM;
  oCnf = MonConf;              // save current setup
  i    = DoDlgSetup();         // start setup dialog

  if (i == IDOK)  {
    if (memcmp (&MonConf, &oCnf, sizeof (oCnf)) != 0)  {   // configuration has changed
      i = MessageBox (hMfrm,"Configuration has been changed, take new values ?",
                             "Target Monitor Notification",
                             MB_OKCANCEL | MB_ICONWARNING); 
      if (i == IDCANCEL)  {
        MonConf = oCnf;         // restore previous configuration
      }
      if (i == IDOK)  {         // take new configuration
        if (ReInitTarget())  {  // failed...
          StopTarget();         // final shutdown
          PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
        }
      }
    }
  }
  else  {                       // cancelled: restore previous config
    MonConf = oCnf;
  }
}




/*
 * Extension Menues and Dialogs
 */


//--- this relates to TestDlg.cpp: ------------
extern void   MdUpdate (void);       // Dialog Update function
extern void     MdKill (DIAD *pM);   // Dialog Kill function
extern void     MdShow (DYMENU *pM); // Show/Hide modeless Dialog

DIAD ModDlg  = { 0, NULL, NULL, { -1,-1,-1,-1, }, MdUpdate, MdKill };
//---------------------------------------------

static DYMENU Menu[] =  {
 {  1, "Target Settings", ConfDisp, 0, 0, 0       },  // modal dialog
 {  1, "Modeless Dialog",   MdShow, 0, 0, &ModDlg },  // modeless dialog
 { -1,            /* End of menu list */          },
};



/*
 * uVision2 want's to update all modeless dialogs.
 */

void DlgUpdate (void)  {        // Update all modeless extension dialogs
  DYMENU    *pM;

  pM = &Menu[0];
  while (pM->nDelim != -1)  {             // while not end of Menu-list
    if (pM->pDlg && pM->pDlg->Update)  {  // if dialog has been created
      pM->pDlg->Update();                 // then call it's update function
    }
    ++pM;                                 // next menu entry.
  }
}


/*
 * Need to close all currently open modeless dialogs
 */

void CloseAllDlg (void)  {
  DYMENU    *pM;

  pM = &Menu[0];
  while (pM->nDelim != -1)  {
    if (pM->pDlg && pM->pDlg->hw)  {        // Dialog is visible
      pM->pDlg->Kill (pM->pDlg);            // update contents
    }
    ++pM;
  }
}




//--- Interface functions between AGDI and target follow
//------------------------------------------------------


/*
 * Read 'nMany' bytes out of data space into buffer 'pB'
 *  return: 0:=Ok, else 'ErrorAddress | (amDATA << 24)'
 */

UL32 ReadData (BYTE *pB, DWORD nAdr, DWORD nMany)  {
  if (CacheValid (pB, nAdr, nMany))  {   // use cache, if possible
    return (0);
  }
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amDATA << 24);   // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Read 'nMany' bytes out of the idata space into buffer 'pB'
 *  return: 0:=Ok, else 'ErrorAddress | (amIDATA << 24)'
 */

UL32 ReadIdata (BYTE *pB, DWORD nAdr, DWORD nMany)  {
  if (CacheValid (pB, nAdr, nMany))  {   // use cache, if possible
    return (0);
  }
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amIDATA << 24);  // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Read 'nMany' sfr values out of sfr-data space into buffer 'pB'
 *  return: 0:=Ok, else 'ErrorAddress | (amDATA << 24)'
 */

UL32 ReadSFR (BYTE *pB, DWORD nAdr, DWORD nMany)  {
  if (CacheValid (pB, nAdr, nMany))  {   // use cache, if possible
    return (0);
  }

//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amDATA << 24);   // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Read 'nMany' bytes out of the xdata space into buffer 'pB'
 *  return: 0:=Ok, else 'ErrorAddress | (amXDATA << 24)'
 */

UL32 ReadXdata (BYTE *pB, DWORD nAdr, DWORD nMany)  {
  if (CacheValid (pB, nAdr, nMany))  {   // use cache, if possible
    return (0);
  }
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amIDATA << 24);  // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Read 'nMany' bytes out of the code space into buffer 'pB'
 *  return: 0:=Ok, else 'ErrorAddress | (amCODE << 24)'
 */

UL32 ReadCode (BYTE *pB, DWORD nAdr, DWORD nMany)  {
  if (CacheValid (pB, nAdr, nMany))  {   // use cache, if possible
    return (0);
  }
  else  {
    memset (pB, 0, nMany);    // for now, we return 0 bytes (NOP's)
  }

//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amCODE << 24);   // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Write 'nMany' bytes of code into target
 *  return: 0:=Ok, else 'ErrorAddress | (amCODE << 24)'
 */

UL32 WriteCode (BYTE *pB, DWORD nAdr, DWORD nMany)  {
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amCODE << 24);   // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Write 'nMany' bytes of xdata into target
 *  return: 0:=Ok, else 'ErrorAddress | (amXDATA << 24)'
 */

UL32 WriteXdata (BYTE *pB, DWORD nAdr, DWORD nMany)  {
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amXDATA << 24);  // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Write 'nMany' bytes of idata into target
 *  return: 0:=Ok, else 'ErrorAddress | (amIDATA << 24)'
 */

UL32 WriteIdata (BYTE *pB, DWORD nAdr, DWORD nMany)  {
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amIDATA << 24);  // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Write 'nMany' bytes of data into target
 *  return: 0:=Ok, else 'ErrorAddress | (amDATA << 24)'
 */

UL32 WriteData (BYTE *pB, DWORD nAdr, DWORD nMany)  {
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amDATA << 24);   // make address uVision2 conforming
  return (0);                 // say Ok.
}

/*
 * Write 'nMany' sfr's to target
 *  return: 0:=Ok, else 'ErrorAddress | (amDATA << 24)'
 */

UL32 WriteSFR (BYTE *pB, DWORD nAdr, DWORD nMany)  {
//---TODO:
//---if Ok, then return 0, else error-address
//   adr |= (amDATA << 24);   // make address uVision2 conforming
  return (0);                 // say Ok.
}



/*
 * Initialize your target communication
 * Return 0 if Ok, 1 if failed.
 */

U32 InitTarget (void)  {
//---TODO:
  return (0);            // say 'Ok.'
}


/*
 * Target settings have changed (Baudrate or ComPort for example)
 * Return 0 if Ok, 1 if failed.
 */

U32 ReInitTarget (void)  {
//---TODO: shutdown target
//---      reinit communication with current 'MonConf' values
  return (0);            // say 'Ok.'
}

/*
 * Stop your target communication
 * Free all resources (dynamic memory, ...)
 */

void StopTarget (void)  {
  FreeCache();                 // free allocated cache memory.
//---TODO: add code for cleanup
}

/*
 * Reset your target. Set PC to whatever required value
 */

void ResetTarget (void)  {
//---TODO: add code for target reset
}


/*
 * Read PC out of target
 */

UL32 ReadPC (void)  {
  UL32   nPC;

//---TODO: read PC out of target

  nPC  = REG51.nPC & 0xFFFF;     // we use shadow PC for now...
  nPC |= (amCODE << 24);         // C:0xnnnn
  return (nPC);
}


/*
 * Write PC out to target
 */

void WritePC (UL32 nPC)  {
//---TODO: write PC to target
  REG51.nPC = nPC;                // keep in shadow registers
}



/*
 * Read all registers out of target and fill the REG51 structure
 */

void GetRegs (void)  {
  if (RegUpToDate) return;        // already up to date
  if (iRun) return;               // currently executing, can't access
  curPC = REG51.nPC = ReadPC();
 
//---TODO: fetch regs from target and put them into REG51
//---if connection is broken for some reason, then set PlayDead:=1 !
}


/*
 * Write all registers to target
 */

void SetRegs (RG51 *pR)  {
//--- write pR->Rn[0] ... pR->Rn[7]
//--- write pR->sp, pR->psw, pR->b, pR->acc, pR->dpl, pR->dph, pR->nPC
  WritePC (pR->nPC);
  REG51 = *pR;
//---if connection is broken for some reason, then set PlayDead:=1 !
}


/*
 * Stop execution of user program
 */

U32 StopExec (void)  {
  if (iRun)  {                  // if currently executing
//---TODO: Stop exec
//---if successful, return (1), else (0)
  }
  return (1);                   // 'Stopped'
}

/*
 * Invalidate everything which may be invalid after Go or Step
 *  (Registers, Caches, etc.)
 */

void Invalidate (void)  {
  RegUpToDate = 0;                       // invalidate Registers
  ClearCaR ((amDATA << 24)  | 0x00, 0);  // invalidate data-cache
  ClearCaR ((amIDATA << 24) | 0x80, 128);// invalidate idata-cache
  ClearCaR ((amXDATA << 24) | 0x0000, 0x10000);// invalidate xdata-cache
}


/*
 * Execute a single instruction, return 1 if ok, else 0
 */

UL32 Step (void)  {
//--TODO: 
  return (1);
}


/*
 * Start execution.  Stop when a Bp is reached
 */

void GoCmd (void)  {
//--TODO:
}


/*
 * Set/Clear Breakpoint at address pB->Adr
 */

int SetClrBp (int set, AG_BP *pB)  {
  int       nR;

  nR = 1;
  if (set)  {                   // set address break
//---TODO: give target a message about Set-Bp at address 'pB->Adr'
//---      if ok, then return (1), else (0)
//  if (failed)  {
//    MessageBeep (MB_OK);
//    txtout ((char *) szNBP, pB->Adr);
//    nR = 0;
//  }
  }
  else  {                       // clear address break
//---TODO: give target a message about Clear-Bp at address 'pB->Adr'
//---      if ok, then return (1), else (0)
  }
  return (nR);
}


//--- Flash Download interface functions follow
//------------------------------------------------------

/*
 * Initialize Flash programmer, download programmer firmware.
 * Return 0 if Ok, 1 if failed.
 */

int InitProgrammer (void)
{
  FlashProg = 1;         // we have started the Flash Programmer   
//---TODO: Add your initialization code here
  return (0);            // say 'Ok.'
}


/*
 * Stop Flash programmer, disconnect device power
 * Return 0 if Ok, 1 if failed.
 */

int ExitProgrammer (void)
{
  if (!FlashProg) return (0); // if Already done, just return
//---TODO: Add your deinitialization code here
//---If insertion socket is used for target Flash, make sure that all address,
//---data and control pins are held low and target VCC is disconnected here.
  FlashProg = 0;
  return (0);            // say 'Ok.'
}


/*
 * Enter programming mode for the flash device
 * Return 0 if Ok, 1 if failed.
 */

int EnterProgMode (void)
{
//---TODO: Add code for activating programming mode or just return if not needed
  return (0);            // say 'Ok.'
}


/*
 * Check device ID
 * Return 0 if Ok, 1 if failed.
 */

int CheckDevID (void)
{
//---TODO: Add code to Read device ID and verify device type
  return (0);            // say 'Ok.'
}


/*
 * Erase the flash device
 * Return 0 if Ok, 1 if failed.
 */

int EraseFlash (void)
{
//---TODO: Add Flash Erase code here
  return (0);            // say 'Ok.'
}


/*
 * Write data to Flash device
 * Return 0 if Ok, 1 if failed.
 */

int WriteToFlash (DWORD nAdr, BYTE *pb, DWORD nCnt, DWORD *eAdr)
{
//---TODO: Add Flash Write Code here
//---Note: nAdr has uVision2 address convention
//---if write fails, set write error addres and return 1
  *eAdr = 0;
  return (0);            // say 'Ok.'
}


/*
 * Verify the content of Flash device
 * Return 0 if Ok, 1 if failed.
 */

int VerifyFlash (DWORD nAdr, BYTE *pb, DWORD nCnt, DWORD *eAdr)
{
//---TODO: Verify Flash Code here
//---Note: nAdr has uVision2 address convention
//---if verify fails, set verify error addres and return 1
  *eAdr = 0;
  return (0);            // say 'Ok.'
}


/*
 * Send text Error message to Build window
 */

static void PgmError (char *pCmd)  {
  txtout ("*** Error: %s\n", pCmd);
}

/*
 * Initialize target Flash device.
 * Return 0 if Ok, 1 if failed.
 */

int InitDevFlash (void)
{
  int nE;

//---We use 'EnterProgMode()' function to activate the programming mode of the device
//---when a special procedure to activate the programming mode is required.
//---If this is not required, we can skip this function.
  nE = EnterProgMode ();
  if (nE) {
    PgmError ("Programming failed. Device in programming socket does not response.");
    return (1);                         // failed
  }

//---We use 'CheckDevID()' function to verify if right target device is attached.
//---If this is not required, we can skip this function.
  nE = CheckDevID ();
  if (nE) {
    PgmError ("Programming failed. This device is not supported");
    return (1);                         // failed
  }

  if (!(MonConf.Opt & FLASH_ERASE)) {   // check if Download Function-Erase Flash enabled
    return (0);
  }
  
//---We erase the target flash here and prepare for flash writing.
  nE = EraseFlash ();
  if (nE) {
    PgmError ("Flash Erase failed.");
    return (1);                         // failed Flash not Erased
  }
  txtout ("Flash Erase Done.\n");
  return (0);                           // Ok.
}


/*
 * Write data for Flash Memory to Target
 *
 * If target hardware has installed SRAM that is big enough to hold the
 * complete Flash image data, it is a good practice to use it as a buffer.
 * This would speedup the flash verify process a lot if a big flash with
 * big amount of data is programmed using 'slow RS232' interface.
 */

int FlashLoad (void)  {
  FLASHPARM  *pF;
  DWORD  bCnt,errAdr;
  char szMsg[80];
  int nE = 0;

  if (!(MonConf.Opt & FLASH_PROGRAM)) goto skipProg;
//---Write Flash memory if enabled
  InitProgress("Loading...");
//---Get Flash Parameters from S8051.DLL
  pF = (FLASHPARM *)pCbFunc (AG_CB_GETFLASHPARAM, NULL);
  bCnt = 0;
  while (pF->many != 0)  {                      // while more chunks..
    nE = WriteToFlash (pF->start, pF->image, pF->many, &errAdr);
    if (nE) {                                   // failed, send message
      sprintf (szMsg,"Flash Write Failed at %04X", errAdr);
      PgmError (szMsg);                         
      goto x;
    }
    bCnt += pF->many;
    SetProgress (bCnt*100/pF->ActSize);
    pF = (FLASHPARM *)pCbFunc (AG_CB_GETFLASHPARAM, pF);// Get next parameters
  }
  SetProgress (100);
  txtout ("Flash Write Done: %d bytes programmed.\n", bCnt);
  StopProgress ();

skipProg:
  if (!(MonConf.Opt & FLASH_VERIFY)) goto x;
//---Verify Flash memory if enabled
  InitProgress("Verifying...");
//---Get Flash Parameters from S8051.DLL
  pF = (FLASHPARM *)pCbFunc (AG_CB_GETFLASHPARAM, NULL);
  bCnt = 0;
  while (pF->many != 0)  {                      // while more chunks..
    nE = VerifyFlash (pF->start, pF->image, pF->many, &errAdr);
    if (nE) {                                   // failed, send message
      sprintf (szMsg,"Flash Verify Failed at %04X", errAdr);
      PgmError (szMsg);                         
      goto x;
    }
    bCnt += pF->many;
    SetProgress (bCnt*100/pF->ActSize);
    pF = (FLASHPARM *)pCbFunc (AG_CB_GETFLASHPARAM, pF);// Get next parameters
  }
  SetProgress (100);
  txtout ("Flash Verify Done: %d bytes verified.\n", bCnt);
x:StopProgress ();
  ExitProgrammer ();
  return (nE);
}

/*
 *----------- AGDI Basic Functions -----------------
 */


/*
 * AGDI-Init Function
 */

U32 _EXPO_ AG_Init (U16 nCode, void *vp)  {
  U32     nE;

  nE = 0;
  switch (nCode & 0xFF00)  {
    case AG_INITFEATURES:     // Initialize & start the target
      PlayDead = 0;           // clear some variables...

//---We don't have special features here, so all variables are cleared.
//---uVision2 will query these features lateron.
//---Note: the 'supp' structure is defined in 'AGDI.H'

      supp.MemAccR  = 0;      // memory-access while running
      supp.RegAccR  = 0;      // register-access while running
      supp.hTrace   = 0;      // trace support
      supp.hCover   = 0;      // code coverage support
      supp.hPaLyze  = 0;      // Performance-Analyzer support
      supp.hMemMap  = 0;      // Memory-Map support
      supp.ResetR   = 0;      // Reset while running support

//---Note: if InitTarget() fails, then set nE=1, else nE=0.
//         if 1 is returned, then uVision will cancel using this driver

      FlashProg = 0; 
      InitMem();                 // init Cache memory

      bootflag = 1;
      nE       = InitTarget();   // Initialize your target...
      bootflag = 0;
      if (nE == 0)  {            // initialization was Ok.
        InitRegs();              // define register layout for RegWindow
      }

      if (nE == 0)  {            // everything is ok so far
        pCbFunc (AG_CB_EXECCMD, "U $\n");  // force disassemble @curPC
      }
      NumRecs = 0;                // clear 'number of trace records'
      break;

    case AG_INITITEM:             // init item
      switch (nCode & 0x00FF)  {
        case AG_INITMENU:         // init extension menu
          *((DYMENU **) vp) = (DYMENU *) Menu;
          break;
        case AG_INITEXTDLGUPD:    // init modeless extesion dlg update function
          *((UC8 **) vp) = (UC8 *) DlgUpdate;
          break;
        case AG_INITMHANDLEP:     // setup ptr to HWND of active modeless dlg
          pHwnd = (HWND *) vp;
          break;
        case AG_INITPHANDLEP:     // pointer to parent handle (MainFrame)
          hMfrm = (HWND) vp;
          break;
        case AG_INITINSTHANDLE:   // pointer to Agdi-instance handle
          hInst = (HMODULE) vp;
          break;
        case AG_INITBPHEAD:       // pointer to head of Bp-List
          pBhead = (AG_BP **) vp;
          break;
        case AG_INITCURPC:        // pointer to program counter
          pCURPC = (UL32 *) vp;
          break;
        case AG_INITDOEVENTS:     // DoEvents function pointer
//--- this is no longer relevant due to uVision's threading.
          break;
        case AG_INITUSRMSG:       // Registered Message for SendMessage
          Uv2Msg = (DWORD) vp;    // (Serial-Window, TextOut messages)
          break;
        case AG_INITCALLBACK:     // pointer to callback function
          pCbFunc = (pCBF) vp;    // call-back function of s166
          break;
        case AG_INITFLASHLOAD:    // Prepare for Flash Download
          nE = InitProgrammer ();
          if (nE) break;
          nE = InitDevFlash ();   // Initialize Flash, Erase
          break;
        case AG_STARTFLASHLOAD:   // S8051 says 'Ready for Flash DownLoad'
          nE = FlashLoad();       // Flash-Download, write, verify
          break;
      }
      break;

    case AG_GETFEATURE:           // uVision2 want's details about features...
      switch (nCode & 0x00FF)  {
        case AG_F_MEMACCR:    nE = supp.MemAccR; break;
        case AG_F_REGACCR:    nE = supp.RegAccR; break;
        case AG_F_TRACE:      nE = supp.hTrace;  break;
        case AG_F_COVERAGE:   nE = supp.hCover;  break;
        case AG_F_PALYZE:     nE = supp.hPaLyze; break;
        case AG_F_MEMMAP:     nE = supp.hMemMap; break;
        case AG_F_RESETR:     nE = supp.ResetR;  break;
      }
      break;

    case AG_EXECITEM:             // execute various commands
      switch (nCode & 0x00FF)  {
        case AG_UNINIT:           // Clean up target system settings
          ExitProgrammer ();      // Stop Programmer
          PlayDead = 1;           // mark target as disconnected.
          CloseAllDlg();          // close all open dialogs
          StopTarget();           // shutdown target & communication
          WriteMonParms (pdbg->TargArgs);  // update argument string
          break;

        case AG_RESET:            // perform a reset on the target system
          if (PlayDead || hMfrm == NULL) break;  // target is disconnected
          ResetTarget();          // reset the target
          pCbFunc (AG_CB_EXECCMD, "U $\n");      // dasm $
          NumRecs = 0;            // clear 'number of trace records'
          break;
      }
      break;
  }

  if (PlayDead)  {       // driver is disconnected
    StopTarget();        // shut down driver
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);  // unload this driver
  }

  return (nE);
}



/*
 * Memory Interface functions
 */

static DWORD ReadMem (DWORD nAdr, BYTE *pB, DWORD nMany)  {
  DWORD     n1, n, ErrAdr;
  DWORD     nA, nM;
  BYTE     *p1;

  nA = nAdr;                    // save values for caching...
  p1 = pB;
  nM = nMany;

  while (nMany != 0)  {
    n = (nMany > 256) ? 256 : nMany;
    nMany -= n;
    switch (nAdr >> 24)  {      // extract mSpace from address.
      case amDATA:
        if ((nAdr & 0xFF) >= 0x80)  {  // Sfr-area: D:0x80...D:0xFF
          if (((nAdr & 0xFF) + n) > 0x100) n1 = 0x100 - (nAdr & 0xFF);
          else                             n1 = n;
          ErrAdr = ReadSFR (pB, nAdr & 0xFF0000FF, n1);
        }
        else  {                     // Data-area: D:0x00...D:0x7F
          if (((nAdr & 0xFF) + n) > 0x100) n1 = 0x100 - (nAdr & 0xFF);
          else                             n1 = n;
          ErrAdr = ReadData (pB, nAdr & 0xFF0000FF, n1);
        }
        break;

      case amIDATA:                 // Idata
        if (((nAdr & 0xFF) + n) > 0x100) n1 = 0x100 - (nAdr & 0xFF);
        else                             n1 = n;
        ErrAdr = ReadIdata (pB, nAdr & 0xFF0000FF, n1);
        break;

      case amXDATA:                 // Xdata
      case amPDATA:
        ErrAdr = ReadXdata (pB, nAdr, n);
        break;

//    case amCODE:                  // Code
      default:                      // anything else assumed to be c-address
        ErrAdr = ReadCode (pB, nAdr, n);
        break;
    }
    pB   += n;
    nAdr += n;
  }

  return (ErrAdr);
}


static DWORD WriteMem (DWORD nAdr, BYTE *pB, DWORD nMany)  {
  DWORD     n1, n, ErrAdr;
  BYTE     *p1;
  DWORD     nA, nM;

  nA = nAdr;                           // save values for caching...
  p1 = pB;
  nM = nMany;

  n = nAdr >> 24;
  if (n == amDATA || n == amIDATA)  {  // write to data/idata
    if ((nAdr & 0xFF) < 0x20)  {       // register bank area
      RegUpToDate = 0;                 // regs might be written...
    }
  }

  while (nMany != 0)  {
    n = (nMany > 256) ? 256 : nMany;
    nMany -= n;
    switch (nAdr >> 24)  {
      case amDATA:
        if ((nAdr & 0xFF) >= 0x80)  {
          if (((nAdr & 0xFF) + n) > 0x100) n1 = 0x100 - (nAdr & 0xFF);
          else                             n1 = n;
          ErrAdr = WriteSFR (pB, nAdr & 0xFF0000FF, n1);
        }
        else  {
          if (((nAdr & 0xFF) + n) > 0x100) n1 = 0x100 - (nAdr & 0xFF);
          else                             n1 = n;
          ErrAdr = WriteData (pB, nAdr & 0xFF0000FF, n1);
        }
        break;

      case amIDATA:          
        if (((nAdr & 0xFF) + n) > 0x100) n1 = 0x100 - (nAdr & 0xFF);
        else                             n1 = n;
        ErrAdr = WriteIdata (pB, nAdr & 0xFF0000FF, n1);
        break;

//    case amCODE:
      default:
        ErrAdr = WriteCode (pB, nAdr, n);
        break;
    }
    pB   += n;
    nAdr += n;
  }

  if (ErrAdr == 0)  {                 // write was Ok.
    CacheData (p1, nA, nM);           // put write data into cache
  }
  else  {                             // write failed...
    ClearCaR (nA, nM);                // invalidate cache range
  }

  return (ErrAdr);
}


/*
 * Access target memory
 */

U32 _EXPO_ AG_MemAcc (U16 nCode, UC8 *pB, GADR *pA, UL32 nMany)  {
  U16     nErr;
  UL32      nA;

  if (iRun && !supp.MemAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access register
  }
  if (PlayDead)  {                  // driver disconnected.
    return (AG_NOACCESS);
  }

  nErr = 0;
  if (hMfrm == NULL) return (nErr);

  nA = pA->Adr;
  switch (nCode)  {
    case AG_READ:                   // need 'read' permission
      pA->ErrAdr = ReadMem (nA, pB, nMany);
      if (pA->ErrAdr) nErr = AG_RDFAILED;
      break;

    case AG_WRITE:                  // need 'write' permission
      pA->ErrAdr = WriteMem (nA, pB, nMany);
      if (pA->ErrAdr) nErr = AG_WRFAILED;
      break;
                                    // used for Program download
    case AG_WROPC:                  // need 'Read/Execute' permissions
      nA |= (amCODE << 24);
      pA->ErrAdr = WriteMem (nA, pB, nMany);
      if (pA->ErrAdr) nErr = AG_WRFAILED;
      break;
                                    // used for disassembly etc.
    case AG_RDOPC:                  // need 'Read/Execute' permissions
      nA |= (amCODE << 24);
      pA->ErrAdr = ReadMem (nA, pB, nMany);
      if (pA->ErrAdr) nErr = AG_RDFAILED;
      break;

    default:
      nErr = AG_INVALOP;            // invalid Operation
      break;
  }

  if (PlayDead == 1) {   // disconnected...
    StopTarget();        // shut down driver
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nErr);
}





/*
 * Read/Write Registers
 */

U32 _EXPO_ AG_AllReg (U16 nCode, void *vp)  {
  U16     nErr;

  if (iRun && !supp.RegAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access register
  }
  if (PlayDead) return (AG_NOACCESS);

  nErr = 0;
  switch (nCode)  {                 // which Opcode
    case AG_READ:                   // read register
      GetRegs();
      break;

    case AG_WRITE:
      SetRegs (&REG51);             // write Regs + PC
      RegUpToDate = 1;              // mark regs as 'up to date'
      break;
   
    default:
      nErr = AG_INVALOP;            // invalid Operation
      break;
  }
  if (PlayDead == 1)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nErr);
}


/*
 * Read/Write a single Register
 */

U32 _EXPO_ AG_RegAcc (U16 nCode, U32 nReg, GVAL *pV)  {
  U16    nErr;

  if (iRun && !supp.RegAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access register
  }
  if (PlayDead) return (AG_NOACCESS);  // driver is disconnected.

  nErr = 0;
  switch (nCode)  {                 // which Opcode
    case AG_READ:                   // read register
      GetRegs();                    // make sure REG51 is up to date
      switch (nReg)  {
        case nnR0: case nnR1: case nnR2: case nnR3:
        case nnR4: case nnR5: case nnR6: case nnR7:
          pV->uc = REG51.Rn [nReg & 0x07];
          break;
        case nrA:
          pV->uc = REG51.acc;
          break;
        case nrDPTR:
          pV->u16 = (((WORD16) REG51.dph) << 8) | (WORD16) REG51.dpl;
          break;
        case mPC:
          pV->u32 = REG51.nPC;      // Note: requires 'amCODE << 24' selector.
          break;
      }
      break;

    case AG_WRITE:
      switch (nReg)  {
        case nnR0: case nnR1: case nnR2: case nnR3:
        case nnR4: case nnR5: case nnR6: case nnR7:
          REG51.Rn [nReg & 0x07] = pV->uc;
x:        SetRegs (&REG51);
          break;
        case nrA:
          REG51.acc = pV->uc;
          goto x;
        case nrDPTR:
          REG51.dpl = (BYTE) (pV->u16 & 0xFF);
          REG51.dph = (BYTE) ((pV->u16 >> 8) & 0xFF);
          goto x;
        case mPC:
          curPC = pV->u32;
          WritePC (curPC);          // to target
          break;
      }
      break;

    default:
      nErr = AG_INVALOP;            // invalid Operation
      break;
  }
  if (PlayDead == 1)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nErr);
}


/*
 * Memory Attribute access
 */

U32 _EXPO_ AG_MemAtt (U16 nCode, UL32 nAttr, GADR *pA)  {
  U16     nErr;

  if (iRun && !supp.MemAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access memory
  }
  if (PlayDead) return (AG_NOACCESS);

  nErr = 0;
  switch (nCode)  {
    case AG_MEMMAP:                 // map memory address range
      pA->ErrAdr = MMapMem (nAttr, pA->Adr, pA->nLen);
      break;

    case AG_GETMEMATT:              // Attribute descriptor is requested
      pA->Adr = (UL32) MGetAttr (pA->Adr);  // so return it in pA->Adr.
      break;

    case AG_SETMEMATT:              // not used by uVision.
      break;
  }
  if (PlayDead)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);  // terminate driver
  }
  return (nErr);
}




#define __BPQX (AG_ATR_BREAK | AG_ATR_BPDIS | AG_ATR_EXECD)

static UL32 MBpInfo (UL32 nAdr, U16 nCode)  {
  struct MMM  acc;
  U16         *pB;
  UL32         nA, nSlot;
  AG_BP       *pK, *pN;

  switch (nCode)  {
    case AG_BPDISALL:                  // disable all Exec-Bp's
    case AG_BPKILLALL:                 // kill all Exec-Bp's
      break;
    default:
      nSlot = SlotNo (nAdr);           // requested code address
      acc   = mslots [nSlot];
      if (acc.mem == NULL || acc.atr == NULL)  {
        return (0x80000000);           // failed: unmapped address
      }
      pB = &acc.atr [nAdr & 0xFFFF];
      break;
  }

  nA = 0;
  switch (nCode)  {                                 // Function code
    case AG_BPQUERY:                                // Query Break
    case AG_BPEXQUERY:                   // Query for 'executed' attribute
      nA = (UL32) (*pB & __BPQX);        // Attributes are similar to uVision
      break;

    case AG_BPENABLE:                               // enable breakpoint
      if (*pB & AG_ATR_BPDIS)  {                    // is Bp disabled ?
        *pB = (*pB & ~AG_ATR_BPDIS) | AG_ATR_BREAK; // enable it
        break;
      }
      break;

    case AG_BPDISABLE:                              // disable breakpoint
      if (*pB & AG_ATR_BREAK)  {                    // is Bp enabled ?
        *pB = (*pB & ~AG_ATR_BREAK) | AG_ATR_BPDIS; // disable it
        break;
      }
      break;

    case AG_BPKILL:                                 // kill breakpoint
      *pB = *pB & ~(AG_ATR_BREAK | AG_ATR_BPDIS);
      break;

    case AG_BPSET:                                  // Set breakpoint
      *pB = (*pB & ~AG_ATR_BPDIS) | AG_ATR_BREAK;
      break;

    case AG_BPDISALL:                  // disable all Bp's
      pK = *pBhead;
      for ( ; pK ; pK = pK->next )  {
        if (pK->type == AG_ABREAK && pK->enabled)  {
          ++nA;
          MBpInfo (pK->Adr, AG_BPDISABLE);
        }
      }
      break;

    case AG_BPKILLALL:                 // kill all Bp's
      pK = *pBhead;
      while (pK)  {
        pN = pK->next;
        if (pK->type == AG_ABREAK)  {
          ++nA;
          MBpInfo (pK->Adr, AG_BPKILL);
        }
        pK = pN;
      }
      break;
  }
  return (nA);
}



int SaCBreaks (DWORD set, DWORD nA)  {  // 1:=set, 0:=clear
  AG_BP    *pB;
  int       nR;

  abreaks = 0;                    // clear number of address Bp's
  cbreaks = 0;                    // clear number of conditional Bp's
  wbreaks = 0;                    // clear number of access Bp's

  for ( pB = *pBhead ; pB ; pB = pB->next )  {
    if (!pB->enabled) continue;
    switch (pB->type)  {
      case AG_ABREAK: ++abreaks; break;  // increase Nr. of AddressBreaks
      case AG_CBREAK: ++cbreaks; break;  // increase Nr. of CondBreaks
      case AG_WBREAK: ++wbreaks; break;  // increase Nr. of WatchBreaks
    }
  }

  if (set == 0)  {                    // Reset Breaks
    if (SetBrkCount == 0) return (1);
    --SetBrkCount;
  }
  else  {                             // Set Breaks
    if (SetBrkCount != 0) return (1);
    ++SetBrkCount;
  }

//--- when CondBreaks or WatchBreaks are active, skip ExecBreaks
  if (cbreaks || wbreaks) return (1);

  nR = 1;
  for ( pB = *pBhead ; pB ; pB = pB->next )  {
    if (!pB->enabled) continue;       // Break is disabled
    switch (pB->type)  {
      case AG_ABREAK:                 // Addess-Break
        if (pB->Adr == nA)  {         // g,main address
          pBX = pB;
        }
        if (!SetClrBp (set, pB))  {
          return (0);
        }
        break;
      case AG_CBREAK:                 // Conditional-Break
        break;
      case AG_WBREAK:                 // Access-Break
        break;
    }
  }
  return (nR);
}



/*
 * Is there a Breakpoint at address 'nAdr'
 */

DWORD IsBreak (DWORD nAdr)  {
  AG_BP   *pB;
  
  for ( pB = *pBhead ; pB ; pB = pB->next )  {
    if (pB->enabled) {
      if (pB->type == AG_ABREAK)  { 
        if (pB->Adr == nAdr)  {
          return (1);               // yes, 'nAdr' has a Bp on it
        }
      }
    }
  }
  return (0);
}


/*
 * Search for Breakpoint at address 'nAdr'
 */
  
DWORD BrkWalk (DWORD nAdr)  {
  AG_BP   *pB;

  for ( pB = *pBhead ; pB ; pB = pB->next )  {
    if (!pB->enabled) continue;         // this Break is disabled.
    switch (pB->type)  {
      case AG_ABREAK:                 // Address Break
        if (nAdr == pB->Adr)  {       // match
          if (pB->rcount == 1)  {
            if (pB->cmd)  {           // is command defined
              pCbFunc (AG_CB_EXECCMD, pB->cmd);  // execute command
              return (0);             // continue app when command is defined
            }
            return (1);               // don't continue app
          }
          --pB->rcount;               // decrease counter
          return (0);                 // continue app
        }
        break;

      case AG_CBREAK:                 // Conditional Break, not supported
        break;

      case AG_WBREAK:                 // Watchpoint-Break, not supported
        break;
    }
  }
  return (2);                         // no break reason found
}

/*
 * Breakpoint Management
 */

#define __BPQX (AG_ATR_BREAK | AG_ATR_BPDIS | AG_ATR_EXECD)

U32 _EXPO_ AG_BpInfo (U16 nCode, void *vp)  {
  U32     nA, nAdr;
  U16    *pB;
  AG_BP  *pK;

  if (PlayDead) return (0);
  switch (nCode)  {
    case AG_BPDISALL:                  // disable all Exec-Bp's
    case AG_BPKILLALL:                 // kill all Exec-Bp's
      break;
    default:                           // need attribute pointer anyway, set it up here.
      nAdr = ((GADR *) vp)->Adr;       // the referred address
      pB   = MGetAttr (nAdr);          // get attribute location for 'nAdr'
      if (!pB) return (0x80000000);    // failed: unmapped address
      break;
  }

  nA = 0;
  switch (nCode)  {                    // Function code
    case AG_BPQUERY:                   // Query Break
    case AG_BPEXQUERY:                 // Query for 'executed' attribute
      nA = (UL32) (*pB & __BPQX);      // Attributes are similar to uVision
      break;

    case AG_BPENABLE:                               // enable breakpoint
      if (*pB & AG_ATR_BPDIS)  {                    // is Bp disabled ?
        *pB = (*pB & ~AG_ATR_BPDIS) | AG_ATR_BREAK; // enable it
      }
      break;

    case AG_BPDISABLE:                              // disable breakpoint
      if (*pB & AG_ATR_BREAK)  {                    // is Bp enabled ?
        *pB = (*pB & ~AG_ATR_BREAK) | AG_ATR_BPDIS; // disable it
        break;
      }
      break;

    case AG_BPKILL:                                 // kill breakpoint
      *pB = *pB & ~(AG_ATR_BREAK | AG_ATR_BPDIS);
      break;

    case AG_BPSET:                                  // Set breakpoint
      *pB = (*pB & ~AG_ATR_BPDIS) | AG_ATR_BREAK;
      break;

    case AG_BPDISALL:                  // disable all Bp's
      pK = *pBhead;                    // head of breakpoint list
      for ( ; pK ; pK = pK->next )  {
        if (pK->type != AG_ABREAK || !pK->enabled)  {
          continue;
        }
        ++nA;                          // count number of changed Bp's
        pB = MGetAttr (pK->Adr);             // get attribute location
        if (pB && (*pB & AG_ATR_BREAK))  {   // is Bp enabled ?
          *pB = (*pB & ~AG_ATR_BREAK) | AG_ATR_BPDIS; // disable it
        }
      }
      break;

    case AG_BPKILLALL:                 // kill all Bp's
      pK = *pBhead;                    // head of breakpoint list
      for ( ; pK ; pK = pK->next )  {
        if (pK->type != AG_ABREAK) continue;
        ++nA;                          // count number of changed Bp's
        pB = MGetAttr (pK->Adr);       // get attribute location
        if (pB)  {
          *pB = *pB & ~(AG_ATR_BREAK | AG_ATR_BPDIS);
        }
      }
      break;
  }

  if (PlayDead == 1)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nA);
}


_EXPO_ AG_BP *AG_BreakFunc (U16 nCode, U16 n1, GADR *pA, AG_BP *pBp)  {
  U16         *pB;

  if (PlayDead) return (NULL);       // driver is disconnected

  if (nCode != 5)  {                 // not BpAccept function
    pB = MGetAttr (pA->Adr);         // get attribute location
    if (!pB) return (NULL);          // invalid, cancel
  }

  switch (nCode)  {
    case 2:                          // Notification: 'pB' will be unlinked
      *pB &= ~(AG_ATR_BREAK | AG_ATR_BPDIS);
      break;
    case 3:                          // not used.
    case 6:                          // not used.
    case 7:                          // not used.
    case 8:                          // not used.
      break;
    case 1:                          // Notification: 'pB' will be linked
    case 4:                          // 'pB->enabled' may have changed
      if (pBp->enabled)  {
        *pB = (*pB & ~AG_ATR_BPDIS) | AG_ATR_BREAK; // enable it
      }
      else  {
        *pB = (*pB & ~AG_ATR_BREAK) | AG_ATR_BPDIS; // disable it
      }
      break;

    case 5:                          // Bp-accept function
      if (pBp->type == AG_WBREAK)  { // read/write access Bp's are not supported here.
        return (NULL);
      }
      break;
  }

  if (PlayDead == 1)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (pBp);
}



static const char szGWW[] =
    "Application does not run in real time because " \
    "of conditional breakpoint(s) !\n";


/*
 * Execute until 'nAdr' is reached or some other Bp fires.
 */

void GoUntil (DWORD nAdr)  {     // 0xFFFFFFFF := go forever
  AG_BP   abp;

  pBX = NULL;                     // clear temporary Bp pointer.
  if (StopRun)  {                 // Stop-Button pressed
    StopRun = 0;
    return;
  }

  if (cbreaks || wbreaks)  {      // cond/watch break(s): need to single step
    txtout ((char *) szGWW);      // give the user a note about it.
  }
  else  {                         // Go, setup Bp's first
    if (IsBreak (curPC))  {       // Bp at current PC, step over it.
      Step();
    }
    if (!SaCBreaks (1, nAdr))  {  // set breakpoints (1:=set, 0:=clear)
      StopRun = 0;                // write breakcode(s) failed.
      return;                     // cancel Go
    }
  }

  while (!StopRun)  {
    if (IsBreak (nAdr) || cbreaks || wbreaks)  {
      Step();                    // One Step, then ReadPC()
      Invalidate();              // registers, d/i-caches
      if (BrkWalk (curPC) == 0x01) break;
      if (IsBreak (curPC)) continue;
    }
    if (cbreaks || wbreaks)  {
      continue;                  // continue stepping with c/wbreaks
    }

    GoMode = 1;
    abp.enabled = 0;
    if (pBX == NULL)  {               // setup a temporary Bp
      if (nAdr != 0xFFFFFFFF)  {      // example: g,main
        pBX = &abp;
        abp.type    = AG_ABREAK;
        abp.enabled = 1;
        abp.rcount  = 1;
        abp.ocount  = 1;
        abp.Adr     = nAdr;
        if (!SetClrBp (1, pBX))  {    // setup temporary break
          return;
        }
      }
    }

    GoCmd();

    if (abp.enabled)  {          // clear temporary break
      SetClrBp (0, &abp);
      pBX = NULL;
    }

    GoMode = 0;

    if (PlayDead) return;
    curPC = ReadPC();
    if (BrkWalk (curPC)) break;
  }

  SaCBreaks (0, nAdr);           // set breakpoints (1:=set, 0:=clear)
}



/*
 * Go/Step/Stop commands
 */

_EXPO_ U32 AG_GoStep (U16 nCode, U32 nSteps, GADR *pA)  {
  U32     nE;

  if (PlayDead) return (0);           // driver is disconnected.

  nE = 0;                             // clear error code
  switch (nCode)  {
    case AG_STOPRUN:                  // Stop Go/Step.
      StopRun = 1;
      nE = StopExec();                // nE:  1=Stopped, 0=executing
      if (nE == 1) iRun = 0;          // stopped.
      if (!iRun)  {
        Invalidate();                 // registers, caches, etc.
      }
      NumRecs = 0;                    // clear 'number of trace records'
      break;

    case AG_NSTEP:                    // execute 'nSteps' instruction steps
      iRun = 1;                       // 'executing'
      for ( ; nSteps != 0 ; --nSteps )  {
        if (StopRun) break;           // Stop-Button was pressed
        if (Step() != 0x01) break;    // 0x01 means 'Ok.'
      }
      StopRun = 0;                    // clear Stop-Button flag.
      Invalidate();                   // registers, caches, etc.
      iRun = 0;                       // clear 'executing' flag
      NumRecs = 0;                    // clear 'number of trace records'
      break;

    case AG_GOTILADR:                 // run til 'pA->Adr' or some Bp,
      iRun = 1;                       // whichever comes first
      GoUntil (pA->Adr);
      StopRun = 0;
      Invalidate();                   // registers, caches, etc.
      iRun = 0;                       // clear 'executing' flag
      NumRecs = 0;                    // clear 'number of trace records'
      break;

    case AG_GOFORBRK:                 // run forever or till some Bp reached.
      iRun = 1;
      GoUntil (0xFFFFFFFF);           // means 'go forever'
      StopRun = 0;
      Invalidate();                   // registers, caches, etc.
      iRun = 0;                       // clear 'executing' flag
      NumRecs = 0;                    // clear 'number of trace records'
      break;
  }

  if (PlayDead)  {                    // target not connected
    StopTarget();                     // shut it down
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0); // terminate ourselves
  }
  else  {
    GetRegs();                        // get target registers
    *pCURPC = REG51.nPC;              // let uVision2 know about PC...
  }
  return (nE);
}

/*
 * Serial Window read/write
 */

_EXPO_ U32 AG_Serial (U16 nCode, U32 nSerNo, U32 nMany, void *vp)  {
  struct SerAS  va;

  if (PlayDead) return (0);    // driver is disconnected.
  va.n1 = nSerNo;              // 0:=Serial #1,  1:=Serial #2
  va.n2 = nMany;               // number of items to write

  switch (nCode)  {
    case AG_SERBOUT:           // write nMany bytes to Serial #1 Window of uVision2
      va.v.pS = (SC8 *) vp;    // content
      SendMessage (hMfrm, Uv2Msg, MSG_UV2_SERBOUT, (LPARAM) &va);
      break;

    case AG_SERWOUT:           // write nMany words to Serial #1 Window
//    va.v.pW = (U16 *) vp;    // content
//    SendMessage (hMfrm, Uv2Msg, MSG_UV2_SERWOUT, (LPARAM) &va);
      break;

    case AG_SERXIN:            // Key was pressed in Serial Window #1 or #2
//    WriteToSerPort (nSerNo, (char *) vp, nMany); // Output to serial interface
//    nSerno := Window number (#0 or #1)
//    vp     := &character(s) to send
//    nMany  := number of characters to send
      break;
  }
  return (0);
}


#if 0  // Example on how to output a string to uVision's serial window #1:
  char szSerTxt[] = "this should appear in serial window #1";
  AG_Serial (AG_SERBOUT, 0, sizeof (szSerTxt) - 1, (void *) szSerTxt);
#endif



/*
 * Calculate Trace History
 */
 
#define MAXTRC   256                // assume 1024 (use a power of two value)

static DWORD TracePC [MAXTRC];      // trace code-address buffer


static int CalcNumRecords (void)  {
  if (NumRecs == 0)  {              // seems to be invalidated

//---just a test vector:
    TracePC [0] = 0x0000000;        // Slot[0] not used.
    TracePC [1] = 0x0000047;
    TracePC [2] = 0x0000049;
    TracePC [3] = 0x000004B;
    TracePC [4] = 0x000004C;
    TracePC [5] = 0x0000050;
    TracePC [6] = 0x0000050;
    TracePC [7] = 0x0000050;
    TracePC [8] = 0x0000050;
    TracePC [9] = 0x0000050;
    TracePC[10] = 0x0000050;
    NumRecs = 10;                   // assume 10 entries

//  Calc:
//  TracePC [0] := unused
//  TracePC [1] := newest code address
//    ..
//  TracePC [n] := oldest code address  ( n >= 0, n < MAXTRC )
  }
  return (NumRecs);
}


/*
 * Trace-History access function:
 *   - the first call is always nCode 3 for 'query number of records'
 *   - then nCode 0 / 1 are activated.
 * Note: Emulator hardware must support tracing
 */

_EXPO_ U32 AG_HistFunc (U32 nCode, I32 indx, I32 dir, void *vp)  {
  U32     nR = 0;

  if (PlayDead) return (0);

  switch (nCode)  {
    case 0:                   // Function #0: get Hist index
      if (NumRecs == 0) break;
      if (dir)  {             // dir, 1:=towards older entries
        if (indx < NumRecs)  {
          nR = indx + 1;
        }
      }
      else  {                 // dir, 0:=towards newer entries
        if (indx > 0) nR = indx - 1;
        else          nR = 0;
      }
      break;

    case 1:                   // Function #1: GetTracePC
      if (indx >= 0 && indx < MAXTRC)  {
        nR  = TracePC [indx]; // get PC of trace record 'indx' 
        nR |= amCODE << 24;
      }
      break;

    case 2:                   // Function #2: GetHistRegs
// this function should be ignored since regs are not recorded anyway.
      break;                  // get recorded regs of record 'indx'

    case 3:                   // Function #3: get Nr. of entries
      if (iRun) return (0);   // block view-trace while running

//   - Find out the number of instructions based on the TRACE-Sfrs up to,
//     but not including the current PC
//***NOTE: make sure that CalcNumRecords() is as fast as possible since
//         it is called many times due to Billy's OnUpdate() handling.
      NumRecs = CalcNumRecords();
      nR      = NumRecs;      // return-value.
      break;
  }

  if (PlayDead == 1)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nR);
}


/*
 * Register View Management
 */

struct rGroup rGroups[] =  {
  { 0x00, 0x01, "Regs",   },      // Group 0, show expanded
  { 0x00, 0x01, "Sys",    },      // Group 1, show expanded
};

struct rItem rItems[] =  {
//--desc-nGi-nItem-szReg[]--isPC-cc-iHig--
  { 0x01, 0, 0x00, "r0",      0, 1, 0,  },
  { 0x01, 0, 0x01, "r1",      0, 1, 0,  },
  { 0x01, 0, 0x02, "r2",      0, 1, 0,  },
  { 0x01, 0, 0x03, "r3",      0, 1, 0,  },
  { 0x01, 0, 0x04, "r4",      0, 1, 0,  },
  { 0x01, 0, 0x05, "r5",      0, 1, 0,  },
  { 0x01, 0, 0x06, "r6",      0, 1, 0,  },
  { 0x01, 0, 0x07, "r7",      0, 1, 0,  },

  { 0x01, 1, 0x10, "a",       0, 1, 0,  },
  { 0x01, 1, 0x11, "b",       0, 1, 0,  },
  { 0x01, 1, 0x12, "sp",      0, 1, 0,  },
  { 0x01, 1, 0x13, "dptr",    0, 1, 0,  },
  { 0x01, 1, 0x14, "PC  $",   1, 1, 0,  },
//{ 0x01, 1, 0x15, "states",  0, 0, 0,  },
//{ 0x01, 1, 0x16, "sec",     0, 0, 0,  },

  { 0x01, 1, 0x100,"psw",     0, 1, 0,  },
  { 0x01, 1, 0x101,"p",       0, 1, 0,  },  // child-0
  { 0x01, 1, 0x102,"f1",      0, 1, 0,  },  // child-1
  { 0x01, 1, 0x103,"ov",      0, 1, 0,  },
  { 0x01, 1, 0x104,"rs",      0, 1, 0,  },
  { 0x01, 1, 0x105,"f0",      0, 1, 0,  },
  { 0x01, 1, 0x106,"ac",      0, 1, 0,  },
  { 0x01, 1, 0x107,"cy",      0, 1, 0,  },
  { 0x01, 1, 0x108,"",        0, 0, 0,  },  // End of 'psw' children.
};


struct bmsk  {
  U16       mask;
  I32       shft;
};

static const struct bmsk mx51[] = {
  { 0x0001,  0, },   // P
  { 0x0002,  1, },   // F1
  { 0x0004,  2, },   // OV
  { 0x0018,  3, },   // RS0, RS1
  { 0x0020,  5, },   // F0
  { 0x0040,  6, },   // AC
  { 0x0080,  7, },   // CY
};

static BYTE           nRmode;
static BYTE           nRstat;
static RG51              uRg;         // shadow registers
static char        szV [128];         // temporary buffer


/*
 * uVision2 want's to display some registers value...
 */

static void RegGet (RITEM *vp, int nR)  {
  GVAL      v;
  U32       n, z;

  if (PlayDead) return;                    // target is disconnected...

  switch (nR & 0xF0000000)  {
    case UPR_NORMAL:                       // Setup Normal Regs
      GetRegs();                           // update our local regs
      uRg    = REG51;                      // capture current regs
      nRmode = 0;                          // normal mode
      *pCURPC = uRg.nPC;                   // C:0xnnnn

      if (PlayDead)  {                     // target died...
        StopTarget();
        PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);  // Bye
      }
      return;

    case UPR_HIST:                         // Setup History Regs
//--- currently not supported !
//    v.i32 = nR & 0xFFFF;                 // History index (max. 64K entries)
//    if (ioc.numtra != MAXTRC) v.i32 = (ioc.numtra - v.i32) & (MAXTRC - 1);
//    else                      v.i32 = (ioc.curtra - v.i32) & (MAXTRC - 1);
//    uRg    = *(((REG51 *) ioc.trap) + v.i32);  // HistRegs
//    nRstat = 1;                          // mark 'PSW' as current.
//    nRmode = 1;                          // history mode
      return;
  }

  v.u64     = 0;
  vp->iDraw = 0;

  switch (vp->nItem)  {
    case 0x00:  case 0x01:  case 0x02:  case 0x03:
    case 0x04:  case 0x05:  case 0x06:  case 0x07:
      v.uc = uRg.Rn [vp->nItem & 0x07];         // current Rn
x8:   if (v.uc != vp->v.uc || vp->szVal[0] == 0)  {
        sprintf (vp->szVal, "0x%02x", v.u32);
      }
      break;
    case 0x10: v.uc = uRg.acc;  goto x8;  // A
    case 0x11: v.uc = uRg.b;    goto x8;  // B
    case 0x12: v.uc = uRg.sp;   goto x8;  // SP
    case 0x13:                            // DPTR
      v.u16 = ((U16) uRg.dph) << 8 | (U16) uRg.dpl;
      if (v.u16 != vp->v.u16 || vp->szVal[0] == 0)  {
        sprintf (vp->szVal, "0x%04x", v.u32);
      }
      break;

    case 0x14:                            // 'PC  $'
      if (uRg.nPC != vp->v.u32 || vp->szVal[0] == 0)  {
        sprintf (vp->szVal, "0x%04x", uRg.nPC & 0xFFFF);
        vp->iDraw = 1;
      }
      vp->v.u32 = uRg.nPC;
      vp->iHigh = 0;                      // never highlighted
      return;

    case 0x15:                            // States
    case 0x16:                            // Seconds
// not relevant in target mode
      return;

    case 0x100:                           // PSW
    case 0x101:                           // P
    case 0x102:                           // F1
    case 0x103:                           // OV
    case 0x104:                           // RS
    case 0x105:                           // F0
    case 0x106:                           // AC
    case 0x107:                           // Cy
      if (vp->nItem == 0x100)  {          // PSW
        v.uc = uRg.psw;                   // want PSW only
        goto x8;
      }
//--- want some bit from PSW:
      n = vp->nItem - 0x101;              // use item-number as index
      z = uRg.psw & mx51[n].mask;         // masked value, not normalized
      v.u32 = z >> mx51[n].shft;          // normalize
      if (v.u32 != vp->v.u32 || vp->szVal[0] == 0)  {  // value changed
        sprintf (vp->szVal, "%d       ", v.u32);       // spaces to ease mouse hit.
      }
      break;
  }

  if (v.u32 == vp->v.u32)  {              // value unchanged
    if (vp->iHigh)  {                     // item is currently highlighted
      vp->iHigh = 0;                      // clear highlight color
      vp->iDraw = 1;                      // need repaint.
    }
  }
  else  {                                 // value changed
    vp->iHigh = 1;                        // highlight
    vp->iDraw = 1;                        // need repaint
  }
  vp->v.u64 = v.u64;                      // update 'On Screen' value
}


/*
 * uVision2 want's to change some registers value...
 */

static I32 RegSet (RITEM *vp, GVAL *pV)  {
  I32       n;
  U16     val;

  if (nRmode == 1) return (0);       // Error: can't change HistRegs !
  if (PlayDead)    return (0);       // cancel: driver is disconnected...
  if (iRun)        return (0);       // cancel: currently running...

  switch (vp->nItem)  {
    default:    return (0);
    case 0x00:  case 0x01:  case 0x02:  case 0x03:   // R0...R7
    case 0x04:  case 0x05:  case 0x06:  case 0x07:
      uRg.Rn [vp->nItem & 0x07] = pV->uc;
x:    SetRegs (&uRg);
      break;
    case 0x10: uRg.acc = pV->uc;  goto x;            // A
    case 0x11: uRg.b   = pV->uc;  goto x;            // B
    case 0x12: uRg.sp  = pV->uc;  goto x;            // SP
    case 0x13:
      uRg.dpl = (BYTE) pV->u16;
      uRg.dph = (BYTE) (pV->u16 >> 8);
      goto x;

    case 0x14:
      if (iRun) return (0);     // $ PC: can't change while running...
      uRg.nPC = (pV->ul & 0xFFFF) | (amCODE << 24);
      goto x;
//  case 0x15:                                // states, can't change
//  case 0x16:                                // sec, can't change
//    break;

    case 0x100:                               // PSW
      uRg.psw = pV->uc;
      goto x;

    case 0x101:                           // P
    case 0x102:                           // F1
    case 0x103:                           // OV
    case 0x104:                           // RS
    case 0x105:                           // F0
    case 0x106:                           // AC
    case 0x107:                           // Cy
      val = pV->uc;
      n   = vp->nItem - 0x101;            // use Item-number for index
      uRg.psw &= ~mx51[n].mask;           // clear value
      val = (val << mx51[n].shft) & mx51[n].mask;
      uRg.psw |= (BYTE) val;
      nRstat   = 1;                       // mark 'PSW' as current.
      goto x;
  }

  if (PlayDead == 1)  {           // target died for some reason...
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0); // Asta la vista...
  }
  return (1);                                 // Ok.
}


/*
 * Initialize Register View
 */

static void InitRegs (void)  {
  REGDSC    dsc;

  dsc.nGitems = sizeof (rGroups) / sizeof (rGroups[0]);
  dsc.nRitems = sizeof (rItems) / sizeof (rItems[0]);
  dsc.GrpArr  = rGroups;
  dsc.RegArr  = rItems;
  dsc.RegGet  = RegGet;                 // get function
  dsc.RegSet  = RegSet;                 // set function
  pCbFunc (AG_CB_INITREGV, &dsc);       // Install RegView in uVision2
}
