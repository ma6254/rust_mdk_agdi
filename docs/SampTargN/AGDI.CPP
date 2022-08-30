#include "stdafx.h"
#include "SampTarg.h"

#define _IN_TARG_               // define if used within target
#include "Agdi.h"
#include "Bom.h"
#include "ComTyp.h"
#include "Collect.h"
#include <stdio.h>
#include "resource.h"

#include "math.h"



RgARM              REGARM;  // Register structure for ARM7
BYTE             initflag;
BYTE             bootflag;
BYTE             PlayDead;  // Disables the driver after comm breaks down.
BYTE          RegUpToDate;  // Flag: mark driver is 'dead' (disconnected)

static UC8 comm_buffer[0x1010]; // common buffer for data or opcode 
UL32 CacheMode = READ_CACHE | READ_CODE_CACHE;  // Initialize the Cache mode
#define DSWAP16(a) ((((a)>>8) &0xFF)|(((a)<<8)&0xFF00)) 
#define DSWAP32(a) ((((a)>>24)&0xFF)|(((a)>>8)&0xFF00) | (((a)<<8)&0xFF0000) | (((a)<<24)&0xFF000000))

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
       struct bom      *pBom;
       AG_BP            *pBX;       // used for temporary Breakpoint
       UL32          abreaks;       // number of code-address Bp's
       UL32          cbreaks;       // number of conditional Bp's
       UL32          wbreaks;       // number of access Bp's
       UL32      SetBrkCount;

       LOADPARMS      lParms;       // LOAD-Parameters


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




/*
 * Memory-allocation and clustering Functions
 */
#define MCC      16

typedef struct mcu  {
  struct mcu *next;                 // Link to next Cluster
  DWORD       aval;                 // available bytes in Cluster
  BYTE      b[MCC];                 // buffer
} MCU;

//struct menv  {
//  MCU      *head;
//  MCU      *tail;
//};

static MCU    *mHead;
static MCU    *mTail;


/*
 * Free all Memory-Clusters starting at mHead.
 */
static void FreeClusters (void)  {
  MCU       *pMem, *pNext;

  if (mHead == NULL)  {             // Clusters are already cleaned up.
    return;
  }
  pMem = mHead;
  for ( ; pMem ; )  {
    pNext = pMem->next;             // save Next
    free (pMem);                    // free
    pMem = pNext;
  }
  mHead = NULL;
  mTail = NULL;
}


static void OutOfMem (void)  {
//---TODO: out of memory occurred !
  while (1);
}


/*
 * Allocate memory for Segment/Attributes chunks
 * Note: GetMem is intended for the big memory blocks which live
 *       until this DLL is shut down.
 */
static void *GetMem (DWORD many)  {
  MCU      *tmp;

  tmp = (MCU *) calloc (many + 16, sizeof (UC8));
  if (tmp == NULL)  {             // out of memory !!!
    OutOfMem();                   //
  }
  if (!mHead)       mHead = tmp;
  else        mTail->next = tmp;
  mTail = tmp;
  mTail->aval = many;
  return ((void *) mTail->b);     // ptr to free Memory
}






//--- Memory cache handling.
//--- Simulate the (non-existing) target memory

static struct EMM (*slots[256])[256] = {
  /*        0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
  /* 00 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 10 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 20 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 30 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 40 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 50 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 60 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 70 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 80 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* 90 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* A0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* B0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* C0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* D0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* E0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  /* F0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};


/*
 * Map Memory
 */
static void MapSeg (DWORD nSeg, DWORD nSof, DWORD nLen, DWORD nAtr)  {
  AMAP        adr;
  struct EMM (*s) [256];
  struct EMM  acc;
  DWORD       *pA;
  DWORD         n;

  if (nLen == 0) return;
  adr.a32 = (nSeg << 16) + nSof;
  s = slots[adr.ub[3]];
  if (s == NULL)  {                             // if slot is not yet allocated
    s = (struct EMM (*)[256]) GetMem (sizeof(struct EMM) * 256);      // 256 ptr's EMM
    slots[adr.ub[3]] = s;
  }
  acc = (*s)[adr.ub[2]];                        // slot available?
  if (acc.mem == NULL)  {
    acc.mem = (UC8 *)  GetMem (_MSGM);          // allocate memory block
    acc.atr = (UL32 *) GetMem (_MSGM);          // allocate corresponding attributes
//  if (nAtr & ATRX_EXEC) {                     // profiling...
//    acc.ep = ioc.GetMem (4*_MSGM);            // map 256K + 16
//  }
    (*s)[adr.ub[2]] = acc;
  }

  nLen = (nLen + 3) >> 2;
  pA = &acc.atr [adr.w16 >> 2];
  for ( n = 0 ; n < nLen ; ++n, ++pA )  {       // initialize attribs
    if (nAtr == 0)  {                           // this is 'map clear'
      *pA &= ~(ATRX_READ | ATRX_WRITE | ATRX_EXEC | ATRX_NOINST | ATRX_THUMB | ATRX_ARM);
    }
    else  {
      *pA |= nAtr;                              // Note: can't use memset() here !
    }
  }
}






/*
 * Invalidate memory
 */

void InvalidateMemory (void) {   // clears the ATRX_UPTD attribute bit in all mapped memory areas
  UL32 i, j, z;
  struct EMM  (*s)[256];
  struct EMM acc;

  RegUpToDate = 0;               // invalidate Registers
  for ( i = 0 ; i < 256 ; i++ )  {
    s = slots[i]; 
    if (!s) continue;  

    for ( j = 0 ; j < 256 ; ++j )  {
      acc = (*s)[j];    
      if (!acc.atr) continue;
      for ( z = 0 ; z < (_MSGM / sizeof (acc.atr[0])); ++z )  {
        acc.atr[z] &= ~ATRX_UPTD;  // clear 'valid' bit in Attributes.
      }
    }
  }
}



/*
 *  Cache data or opcode
 */

void WriteToCache (UL32 Adr, UC8 *pB, UL32 nMany) {   // writing opcodes and variables is idential
  MAMAP         x;
  struct EMM  (*s)[256];
  struct EMM   acc;

  x.a32 = Adr;                                    // requested address
  for ( ; nMany != 0 ; --nMany, ++x.a32 )  {
    s = slots [x.ub[3]];                          // a24-a31
    if (!s) {
      s = (struct EMM (*)[256]) GetMem (sizeof(struct EMM) * 256);      // 256 ptr's EMM
      slots[x.ub[3]] = s;
    }
    acc = (*s)[x.ub[2]];                          // slot available?
    if (acc.mem == NULL)  {
      acc.mem = (UC8 *)  GetMem (_MSGM);          // allocate memory block
    }
    if (acc.atr == NULL) { 
      acc.atr = (UL32 *) GetMem (_MSGM);          // allocate corresponding attributes
    }
    (*s)[x.ub[2]] = acc;

    acc.mem[x.w16] = *pB++;  
    ((BYTE *) acc.atr) [x.w16] |= ATRX_UPTD;      // mark address content as 'valid'
  }
}





/*
 * Map a cache segment descriptor.
 */

UL32 MMapMem (UL32 nAtr, UL32 nAdr, UL32 nLen)  {
  DWORD    nSof, nSeg;

  nSeg = nAdr >> 16;                     // Start-Segment
  nSof = nAdr & 0xFFFF;
  while ((nSof + nLen) > 0x10000)  {
    MapSeg (nSeg, nSof, 0x10000 - nSof, nAtr);
    ++nSeg;
    nLen -= (0x10000 - nSof);
    nSof  = 0;
  }
  if (nLen != 0)  {
    MapSeg (nSeg, nSof, nLen, nAtr);     // Segments are never > than 64K !
  }
  return (0);
}



/*
 * Get Pointer to Attributes for addresss 'nAdr'
 */
UL32 *MGetAttr (UL32 nAdr)  {
  MAMAP        adr;
  struct EMM (*s) [256];
  struct EMM  acc;
  DWORD       *pA;

  pA = NULL;
  adr.a32 = nAdr;
  s = slots [adr.ub[3]];              // a24-a31
  if (s)  {
    acc = (*s) [adr.ub[2]];
    if (acc.atr)  {
      pA = &acc.atr [adr.w16 >> 2];   // attributes for 'nAdr'
    }
  }
  return (pA);
}




/*
 * free any allocated memory before driver shutdown.
 */
static void FreeCache (void)  {
//---TODO: free memory resources, if any
  FreeClusters();                 // free our Memory-Cluster chain.
  memset (slots, 0, sizeof (slots));
}

static void InitCache (void)  {
  mHead = NULL;
  mTail = NULL;
  memset (slots, 0, sizeof (slots));
}

//--- End of c/i-chache





/*
 * Output a message line into uVision2's command window
 */

void txtout (char *fmt, ...)  {
  va_list   marker;
  char      txtbuf [2048];

  va_start (marker, fmt);
  vsprintf (&txtbuf[0], fmt, marker);
  SendMessage (hMfrm, Uv2Msg, MSG_UV2_CMDLINE, (LPARAM) &txtbuf[0]);
}




/*
 * Show target setup dialog
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
 {  1, "Target Settings", ConfDisp, 0, 0,         },  // modal dialog
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
 *  Read 'len' bytes out of data space into buffer 'pB'
 *  return: 0:=Ok
 */

UL32 read_block (UL32 staddr, UL32 len) {
//---TODO: read from Addr = staddr len bytes
//---if Ok, then return 0, else error-address
  return(0);   // say ok 
}


/*
 *  Write 'len' bytes into Target memory at adress staddr
 *  return: 0:=Ok
 */

UL32 write_block (UL32 staddr, UL32 len) {
//---TODO: write len bytes at Adress = staddr
//---if Ok, then return 0, else error-address
  return(0);   // say ok 
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
 * Write Register value
 * initregnum :=  number of the Register
 * bytes      :=  size of the reg
 * value      :=  pointer to the value to write
 * Return    0 = Ok
 */

int WriteRegister(unsigned int initregnum,unsigned char bytes,unsigned long *value) {
//---TODO----
  return 0;
}

/*
 * Write PC out to target
 */

void WritePC (UL32 nPC)  {
//---TODO: write PC to target
  REGARM.cur[15] = nPC;                // keep in shadow registers
}

/*
 * Reset your target. Set PC to whatever required value
 */

void ResetTarget (void)  {
//---TODO: add code for target reset

#if 0
  DWORD val;
  int   i;

  REGARM.cur[15] = 0;
  *pCURPC = 0;
  WritePC(0);  // Set IP to 0 if not already done via reset

  REGARM.cpsr = 0x000000D3;
  pBom->Thumb = (REGARM.cpsr & 0x20) ? 1 : 0;
  val = DSWAP32(REGARM.cpsr);
  WriteRegister(DSWAP16(CPSR),4, &val);
  switch (REGARM.cpsr & 0x1F) {
    case 0x1F:
    case 0x10:
      for (i=8;i<15;i++) REGARM.cur[i] = REGARM.usr[i-8];
      break;
    case 0x11:
      for (i=8;i<15;i++) REGARM.cur[i] = REGARM.fiq[i-8];
      REGARM.spsr = REGARM.fiq[7];
      break;
    case 0x12:
      for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
      REGARM.cur[13] = REGARM.irq[0];
	    REGARM.cur[14] = REGARM.irq[1];
	    REGARM.spsr    = REGARM.irq[2];
      break;
    case 0x13:
      for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
      REGARM.cur[13] = REGARM.svc[0];
	    REGARM.cur[14] = REGARM.svc[1];
	    REGARM.spsr    = REGARM.svc[2];
	    break;
    case 0x17:
	    for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
	    REGARM.cur[13] = REGARM.abt[0];
	    REGARM.cur[14] = REGARM.abt[1];
	    REGARM.spsr    = REGARM.abt[2];
	    break;
    case 0x1B:
	    for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
	    REGARM.cur[13] = REGARM.und[0];
	    REGARM.cur[14] = REGARM.und[1];
	    REGARM.spsr    = REGARM.und[2];
	    break;
  }
#endif  
}


/*
 * Read PC out of target
 */

UL32 ReadPC (void)  {
  UL32   nPC = 0;

//---TODO: read PC out of target

  REGARM.cur[15] = nPC;         // we use shadow PC for now...
  return (nPC);
}


/*
 * Read all registers out of target and fill the REG51 structure
 */

void GetRegs (void)  {
  if (RegUpToDate) return;        // already up to date
  if (iRun) return;               // currently executing, can't access
  curPC = REGARM.cur[15] = ReadPC();
 
//---TODO: fetch regs from target and put them into REGARM
//---if connection is broken for some reason, then set PlayDead:=1 !
}



/*
 * Write all registers to target
 */

void SetRegs (RgARM *pR)  { 
//--- write pR->cur[0] ... pR->cur[14]
  WritePC (pR->cur[15]);
  REGARM = *pR;
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
  RegUpToDate = 0;                    // invalidate Registers
  InvalidateMemory();                // invalidate memory
}


/*
 * Execute a single instruction, return 1 if ok, else 0
 */
UL32 Step (void)  {
  return (1);
}



/*
 * Start execution.  Stop when a Bp is reached
 */
void GoCmd (void)  {
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




/*
 *----------- AGDI Basic Functions -----------------
 */

static AG_SCOPE   ac;

static DWORD Find_Line (DWORD nAddr)  {
  DWORD       n;
  AG_LINE   *pL;

  pL = ac.pLines;
  for ( n = 0 ; n < ac.nLines ; ++n, ++pL )  {
    if (pL->addr == nAddr)  {
      return (pL->line);
    }
  }
  return (0);                     // no line number found for address 'nAddr'
}


/*
 * Application: get Scope-Info (Function-address range, line-numbers and full-path-src)
 */
static void TestScope (DWORD nAddr)  {
  U32    nE, line;

  ac.AddrIn = nAddr;              // Input-Parameter: Address to get Scope-Info for.
  nE = pCbFunc (AG_CB_GETSCOPEINFO, (void *) &ac);    // send data to uVision
  if (nE == 0)  {                 // Scope-Mapping failed...
    return;
  }
//---Now, ag_scope has the following info:
// 
//  ac.pModule    := Name of Module
//  ac.pFunc      := Name of Function
//  ac.lowPC      := start address of function
//  ac.higPC      := last valid address of function
//  ac.nLines     := number of lines in Module 'pModule'
//  ac.curLine    := nAddr maps to line 'ac.curLine' in File 'ac.szPath[]'
//  ac.pLines     := Array[ac.nLines] of Linenumber/address pairs for module 'ac.pModule' 
//  ac.szPath     := full path name of associated source file

  line = Find_Line (nAddr);
  if (line != 0)  {               // found line
//---TODO:  access file 'ac.szPath', line-number = line and fetch the source-code line.
  }
}



static AG_BLOCK  ab;

static void TestEnumScopes (void)  {
  UL32     nE;
  char    szB[512];

  memset (&ab, 0, sizeof (ab));             // clear -->  ***REQUIRED!!!***
  nE = pCbFunc (AG_CB_ENUMFUNCTIONS, (void *) &ab);    // initialize for enumeration
  if (nE == 0)  {                                      // failed...
    return;
  }
  while (1)  {                              // while more enumerations...
    nE = pCbFunc (AG_CB_ENUMFUNCTIONS, (void *) &ab);    // send data to uVision
    if (nE == 0) break;                     // Enumeration finished.
    sprintf (szB, "AGDI: \\\\%s\\%s\\%s(): lowPC=0x%08X, higPC=0x%08X\n",
                    ab.pApp, ab.pModule, ab.pFunc, ab.lowPC, ab.higPC);
    txtout (szB);                           // write result into command-output-Window.
  }
}





/*
 * Logic-Analyzer Helper Functions
 */
static DWORD SaveAdr;

I32 QueryLaSig (AGDI_LA *pLA)  {
  DWORD     nAdr;
  DWORD    nSize;

  nAdr  = (U32) pLA->nAdr;         // Memory-Address
  nSize = (U32) pLA->nSize;        // Size in Bytes
SaveAdr = nAdr;

//---TODO:
//if (cannot_set tracking-range nAdr...nAdr+nSize-1)  {
//  strcpy (pLA->szError, "Error-Text");   // Reason for 'not supported'
//  return (AGDI_LA_NOTSUPPORTED);
//}


//---OK:
  return (AGDI_LA_OK);             // LA-Signal accepted
}


/*
 * Given LA-Signal has been killed.
 */
U32 KilledLaSig (AGDI_LA *pLA)  {
//---TODO: whatever cleanup is required.
  return (AGDI_LA_OK);
}


/*
 * Send a Data-Record to uVision
 *  uVision will look for the matching LA-Signal based on the write-address
 *  and assign the data record to that La-signal.
 */
U32 SendLaDataRecord (AGDI_LAREC *pLA)  {
  U32    nE;

  nE = pCbFunc (AG_CB_LAREC_DATA, (void *) pLA);  // send data to uVision
  switch (nE)  {
    default:
    case 0:                       // could not find a matching Signal (pLA->nAdr)
      break;
    case AGDI_LA_OK:
      break;
  }
  return (nE);
}


static AGDI_LAREC  la;

static void SineSig (void)  {
  int       i;
  double    volts, frequ, offs, dur, val;

  memset (&la, 0, sizeof (la));
  la.nAdr   = SaveAdr;
  la.tStamp  = 10;
  la.totTime = 10;

  volts =  2.0;         // peak-to-peak voltage
  offs  =  2.5;         // voltage offset
  frequ = 1000;         // frequency
  dur   =  0.2;         // duration in seconds

  for ( i = 0 ; i < (dur * 100000) ; ++i )  {
    val = (float) sin (frequ * (((double) la.totTime) / 33e6) * 2 * 3.14159);
    la.v.f32  = (float) ((val * volts) + offs);
    SendLaDataRecord (&la);                    // send data to uVision
    la.totTime += 10;
    la.tStamp  += 10;
  }
}



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

      supp.MemAccR  = 0;         // memory-access while running
      supp.RegAccR  = 0;         // register-access while running
      supp.hTrace   = 0;         // trace support
      supp.hCover   = 0;         // code coverage support
      supp.hPaLyze  = 0;         // Performance-Analyzer support
      supp.hMemMap  = 0;         // Memory-Map support
      supp.ResetR   = 0;         // Reset while running support

      supp.LaSupp   = 1;         // Logic-Analyzer support


//---Note: if InitTarget() fails, then set nE=1, else nE=0.
//         if 1 is returned, then uVision will cancel using this driver

      InitCache();               // init Cache

      bootflag = 1;
      nE       = InitTarget();   // Initialize your target...
      bootflag = 0;
      if (nE == 0)  {            // initialization was Ok.
        InitRegs();              // define register layout for RegWindow
      }

      if (nE == 0)  {            // everything is ok so far
        pCbFunc (AG_CB_EXECCMD, "U $\n");  // force disassemble @curPC
      }
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

        case AG_INITSTARTLOAD:    // about to start 'load file'
          if (vp != NULL)  {
            lParms = *((LOADPARMS *) vp);
// lParms.szFile[]    : full path name of App to load
// lParms.Incremental : 1:= incremental load
// lParms.NoCode      : 1:= load debug info only, no code
          }
          break;

        case AG_INITENDLOAD:      // Load is now completed.
          break;

      }
      break;

    case AG_GETFEATURE:           // uVision2 want's details about features...
      switch (nCode & 0x00FF)  {
        case AG_F_MEMACCR:    nE = supp.MemAccR;  break;
        case AG_F_REGACCR:    nE = supp.RegAccR;  break;
        case AG_F_TRACE:      nE = supp.hTrace;   break;
        case AG_F_COVERAGE:   nE = supp.hCover;   break;
        case AG_F_PALYZE:     nE = supp.hPaLyze;  break;
        case AG_F_MEMMAP:     nE = supp.hMemMap;  break;
        case AG_F_RESETR:     nE = supp.ResetR;   break;
        case AG_F_LANALYZER:  nE = supp.LaSupp;   break;      // Logic-Analyzer support
      }
      break;

    case AG_EXECITEM:             // execute various commands
      switch (nCode & 0x00FF)  {

        case AG_QUERY_LASIG:      // is LA-Signal acceptable ?
          if (vp == NULL)  {      // NOTE: just for Test...
//---NOTE: vp == NULL is just for test only, it is not a real case !!!
            SineSig();            // generate a sine Wave for test data
            break;                // LA should show the sine wave...
          }

          nE = QueryLaSig ((AGDI_LA *) vp);
          break;

        case AG_KILLED_LASIG:     // LA-Signal was killed
          nE = KilledLaSig ((AGDI_LA *) vp);
          break;
         
        case AG_UNINIT:           // Clean up target system settings
          PlayDead = 1;           // mark target as disconnected.
          CloseAllDlg();          // close all open dialogs
          StopTarget();           // shutdown target & communication
          WriteMonParms (pdbg->TargArgs);  // update argument string
          FreeCache();                     // free memory clusters.
          break;

        case AG_RESET:            // perform a reset on the target system
          if (PlayDead || hMfrm == NULL) break;  // target is disconnected
          ResetTarget();          // reset the target
          pCbFunc (AG_CB_EXECCMD, "U $\n");      // dasm $

#if 1
          TestScope (0x1000B1C);
          TestEnumScopes();
#endif
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

UL32 ReadMemory (UL32 Adr, UC8 *pB, UL32 nMany) {    // reads memory
  UL32       len, temp;
  U16                i;
  struct EMM (*s)[256];
  struct EMM       acc;
  MAMAP              x;

  x.a32 = Adr;                       // requested address
  while (nMany) {
    s = slots [x.ub[3]];                        // a24 ... a31
    if (s) acc = (*s)[x.ub[2]];

#if 0      // could map memory here...
    if (s == NULL || acc.mem == NULL) {         // accessing unmapped memory !
      MapSeg (x.a32 >> 16, x.a32 & 0xFFFF, 0x10000 - (x.a32 & 0xFFFF), 0);   // map a segment
    }
#endif

    if (s) acc = (*s)[x.ub[2]];
    if (s == NULL || acc.mem == NULL) {         // accessing unmapped memory !
      return (x.a32);                           // return error-address
    }

//---fetch data out of cache, if content is valid:
    if ((acc.atr [x.w16 >> 2] & ATRX_UPTD) && (CacheMode & READ_CACHE) && !iRun)  {
      *pB = acc.mem [x.w16];
      ++pB;                   // advance receive pointer
      --nMany;                // decrease 'many'
      ++x.a32;                // increase address
      continue;
    }
//---

    len = nMany;
    if (x.a32 & 0x01) {       // JTAG cannot handle odd addresses
      x.a32--;                // decrease address by one to make it even
      len++;                  // increase len by 1 to get all requested bytes
    }
    if (len > 0x1000) len = 0x1000;    // can't handle more than 4096 bytes because of buffer size
    if (len < 0x0040) len = 0x40;//0x001f;    // avoid too small memory transfers
    if ((0x00010000 - (UL32)x.w16) < len) len = (0x00010000 - x.w16);  // do not write across segment boundaries
    if (PlayDead) return(0);                  // Disables the driver after the communication breaks down.
    while (read_block (x.a32, len));
      
    memcpy (&acc.mem [x.w16], &comm_buffer[0], len);  // copy bytes from communication buffer into memory
    temp = len;
    for ( i = x.w16 >> 2 ; temp ; temp--, i++)  {
      acc.atr[i] |= ATRX_UPTD;                        // set 'cache valid' bit for address
    }
    if (len > nMany) len = nMany;      // if more bytes have been requested as needed to return
    memcpy (pB, &acc.mem[x.w16], len); // copy bytes from communication buffer
  
    nMany -= len;
    x.a32 += len;
    pB    += len;
  }
  return(0);
}



UL32 WriteOpcode (UL32 Adr, UC8 *pB, UL32 nMany) {   // writing opcodes and variables is idential
  UL32        len;

  WriteToCache (Adr, pB, nMany);
  if (!(CacheMode & WRITE_BEHIND_CACHE)) {
    RegUpToDate = 0;
    while(nMany) {      // write new data to target system immediately
      if (PlayDead) return(0);
      len = nMany;
      if (len > 0x1000) len = 0x1000;
      memcpy(&comm_buffer[0], pB, len);  // copy bytes into communication buffer
      while(write_block(Adr, len));
      nMany -= len;
      Adr   += len;
      pB    += len;
    }
  }
  return(0);
}



UL32 ReadOpcode (UL32 Adr, UC8 *pB, UL32 nMany) {    // reads opcodes (does not care about the updated flag when mapped as code)
  UL32        len, temp;
  U16           i;
  struct EMM  (*s)[256];
  struct EMM      acc;
  MAMAP         x;
 
  x.a32 = Adr;                                   // requested address
  while (nMany) {
    s = slots [x.ub[3]];                         // a24-a31

#if 0      // could map memory here...
    if (s == NULL || acc.mem == NULL) {          // accessing unmapped memory !
      MapSeg (x.a32 >> 16, x.a32 & 0xFFFF, 0x10000 - (x.a32 & 0xFFFF), 0);   // map a segment
    }
#endif

    if (s) acc = (*s)[x.ub[2]];
    if (s == NULL || acc.mem == NULL) {          // accessing unmapped memory !
      return (x.a32);                            // access violation address
    }
    
//---fetch data out of cache, if content is valid:
    if (((acc.atr [x.w16>>2] & ATRX_UPTD) && (CacheMode & READ_CACHE))  ||
        ((acc.atr [x.w16>>2] & ATRX_EXEC) && !(acc.atr [x.w16>>2] & ATRX_WRITE) && (CacheMode & READ_CODE_CACHE)))  {
      *pB = acc.mem [x.w16];  
      pB++;
      nMany--;
      x.a32++;
      continue;
    }

    len = nMany;
    if (x.a32 & 0x01) {       // JTAG cannot handle odd addresses
      x.a32--;                // decrease address by one to make it even
      len++;                  // increase len by 1 to get all requested bytes
    }
    if (len > 0x1000) len = 0x1000;    // can't handle more than 4096 bytes because of buffer size
    if (len < 0x0040) len = 0x0040;    // avoid too small memory transfers
    if ((0x00010000 - (UL32)x.w16) < len) len = (0x00010000 - x.w16);  // do not write across segment boundaries
    if (PlayDead) return(0);
    while(read_block(x.a32, (UC8)len));
   
    memcpy(&acc.mem [x.w16], &comm_buffer[0], len);  // copy bytes from communication buffer into memory
    temp = len;
    for (i = x.w16>>2; temp; temp--, i++) {
      acc.atr [i] |= ATRX_UPTD;
    }
    if (len > nMany) len = nMany;      // if more bytes have been requested as needed to return
    memcpy(pB, &acc.mem[x.w16], len);  // copy bytes from communication buffer
   
    nMany -= len;
    x.a32 += len;
    pB    += len;
  }
  return(0);
}



/*
 * Access target memory
 */


U32 _EXPO_ AG_MemAcc (U16 nCode, UC8 *pB, GADR *pA, UL32 nMany)  {
  U16     nErr;
 
  if (iRun && !supp.MemAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access register
  }
  if (PlayDead) return(AG_NOACCESS);
  nErr = 0;
  if (hMfrm == NULL) {
    memset (pB, 0, nMany);          
    return(nErr);
  }
  switch (nCode)  {
    case AG_READ:                   // need 'read' permission
      pA->ErrAdr = ReadMemory (pA->Adr, pB, nMany); 
      break;

    case AG_WRITE:                  // need 'write' permission
      pA->ErrAdr = WriteOpcode (pA->Adr, pB, nMany); 
      if (pA->ErrAdr != 0)  {
        nErr = AG_WRFAILED;
      }
      break;

    case AG_F_WRITE:                // Flash-Write
//    pA->ErrAdr = WriteFlash (pA->Adr, pB, nMany);
      break;
                                    // used for Program download
    case AG_WROPC:                  // need 'Read/Execute' permissions
      pA->ErrAdr = WriteOpcode (pA->Adr, pB, nMany);
      if (pA->ErrAdr != 0)  {
        nErr = AG_WRFAILED;
      }
      break;
                                    // used for disassembly etc.
    case AG_RDOPC:                  // need 'Read/Execute' permissions
      pA->ErrAdr = ReadOpcode (pA->Adr, pB, nMany);
      if (pA->ErrAdr != 0)  {
        nErr = AG_RDFAILED;
      }
      break;

    default:
      nErr = AG_INVALOP;            // invalid Operation
      break;
  }

  if (PlayDead == 1)  {             // Disables the driver after the communication breaks down.
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
  if (PlayDead) return(AG_NOACCESS);

  nErr = 0;
  switch (nCode)  {                 // which Opcode
    case AG_READ:                   // read register
      if (!RegUpToDate) GetRegs();
      memcpy (vp, &REGARM, sizeof (RgARM));
      break;

    case AG_WRITE:
      memcpy (&REGARM, vp, sizeof (RgARM));
      SetRegs(&REGARM);             // write Regs + PC     
      *pCURPC = REGARM.cur[15];
      WritePC(REGARM.cur[15]);
      
      break;
   
    default:
      nErr = AG_INVALOP;            // invalid Operation
      break;
  }
  
  if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nErr);
}



/*
 * Read/Write a single Register (ARM7)
 */



U32 _EXPO_ AG_RegAcc (U16 nCode, U32 nReg, GVAL *pV)  {
  U16    nErr;
  DWORD  val;
  int    i;
 

  if (iRun && !supp.RegAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access register
  }
  if (PlayDead) return(AG_NOACCESS);

  nErr = 0;
  switch (nCode)  {                 // which Opcode
    case AG_READ:                   // read register
      if (!RegUpToDate) GetRegs();
      switch (nReg)  {
        case nR0:   case nR1:  case nR2:  case nR3:
        case nR4:   case nR5:  case nR6:  case nR7:
        case nR8:   case nR9:  case nR10: case nR11:
        case nR12:  case nR13: case nR14: case nR15:
          pV->u32 = REGARM.cur[nReg];
          break;
        case mPC:                   // current PC
          pV->u32 = REGARM.cur[15];
          break;
        case nCPSR:
          pV->u32 = REGARM.cpsr;
          break;
        case nSPSR:
          pV->u32 = REGARM.spsr;
          break;
          
        default:
          nErr = AG_INVALOP;        // invalid Operation
          break;
      }
      break;

    case AG_WRITE:
      switch (nReg)  {
        case nR0:   case nR1:  case nR2:  case nR3:
        case nR4:   case nR5:  case nR6:  case nR7:
          REGARM.cur[nReg] = pV->u32;
          val = DSWAP32(pV->u32); 
//          WriteRegister(DSWAP16(R0 + nReg), 4, &val);
          break;
        case nR8:   case nR9:  case nR10: case nR11:
        case nR12:
          REGARM.cur[nReg] = pV->u32;
		      val = DSWAP32(pV->u32);
          switch (REGARM.cpsr & 0x1F) {
            case 0x1F:
            case 0x10:
            case 0x12:
            case 0x13:
            case 0x17:
            case 0x1B:
              REGARM.usr[nReg - 0x08] = pV->u32;
//              WriteRegister(DSWAP16(R0 + nReg), 4, &val);
		      break;
            case 0x11:
              REGARM.fiq[nReg - 0x08] = pV->u32;
//              WriteRegister(DSWAP16(FIQ_R8 + (nReg - 0x08)), 4, &val);
		      break;
		  }
          break;
        case nR13: case nR14:
          REGARM.cur[nReg] = pV->u32;
		  val = DSWAP32(pV->u32);
          switch (REGARM.cpsr & 0x1F) {
            case 0x1F:
            case 0x10:
              REGARM.usr [nReg - 0x08] = pV->u32;
//              WriteRegister(DSWAP16(R0 + nReg), 4, &val);
		      break;
            case 0x11:
              REGARM.fiq [nReg - 0x08] = pV->u32;
//              WriteRegister(DSWAP16(FIQ_R8 + (nReg - 0x08)), 4, &val);
		      break;
            case 0x12:
              REGARM.irq [nReg - 0x0D] = pV->u32;
//              WriteRegister(DSWAP16(IRQ_R13 + (nReg - 0x0D)), 4, &val);
		      break;
            case 0x13:
              REGARM.svc [nReg - 0x0D] = pV->u32;
//              WriteRegister(DSWAP16(SVC_R13 + (nReg - 0x0D)), 4, &val);
		      break;
            case 0x17:
              REGARM.abt [nReg - 0x0D] = pV->u32;
//              WriteRegister(DSWAP16(ABT_R13 + (nReg - 0x0D)), 4, &val);
		      break;
            case 0x1B:
              REGARM.und [nReg - 0x0D] = pV->u32;
//              WriteRegister(DSWAP16(UND_R13 + (nReg - 0x0D)), 4, &val);
		      break;
		  }
          break;
		case nR15:
        case mPC:                   // current PC
          REGARM.cur[15] = pV->u32;
          WritePC(pV->u32);
          break;
        case nCPSR:
          REGARM.cpsr = pV->u32;
          pBom->Thumb = (REGARM.cpsr & 0x20) ? 1 : 0;
		      val = DSWAP32(pV->u32);
//          WriteRegister(DSWAP16(CPSR),4, &val);
          switch (REGARM.cpsr & 0x1F) {
            case 0x1F:
            case 0x10:
	          for (i=8;i<15;i++) REGARM.cur[i] = REGARM.usr[i-8];
	          break;
            case 0x11:
	          for (i=8;i<15;i++) REGARM.cur[i] = REGARM.fiq[i-8];
			      REGARM.spsr = REGARM.fiq[7];
	          break;
            case 0x12:
	          for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
			      REGARM.cur[13] = REGARM.irq[0];
			      REGARM.cur[14] = REGARM.irq[1];
			      REGARM.spsr    = REGARM.irq[2];
	          break;
            case 0x13:
	          for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
			      REGARM.cur[13] = REGARM.svc[0];
			      REGARM.cur[14] = REGARM.svc[1];
			      REGARM.spsr    = REGARM.svc[2];
	          break;
            case 0x17:
	          for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
			      REGARM.cur[13] = REGARM.abt[0];
			      REGARM.cur[14] = REGARM.abt[1];
			      REGARM.spsr    = REGARM.abt[2];
	          break;
            case 0x1B:
	          for (i=8;i<13;i++) REGARM.cur[i] = REGARM.usr[i-8];
			      REGARM.cur[13] = REGARM.und[0];
			      REGARM.cur[14] = REGARM.und[1];
			      REGARM.spsr    = REGARM.und[2];
	          break;
		  }
          break;
        case nSPSR:
          REGARM.spsr = pV->u32;
		      val = DSWAP32(pV->u32);
          switch (REGARM.cpsr & 0x1F) {
            case 0x1F:
            case 0x10:
		      break;
            case 0x11:
              REGARM.fiq [7] = pV->u32;
//              WriteRegister(DSWAP16(FIQ_SPSR), 4, &val);
		      break;
            case 0x12:
              REGARM.irq [2] = pV->u32;
//              WriteRegister(DSWAP16(IRQ_SPSR), 4, &val);
		      break;
            case 0x13:
              REGARM.svc [2] = pV->u32;
//              WriteRegister(DSWAP16(SVC_SPSR), 4, &val);
		      break;
            case 0x17:
              REGARM.abt [2] = pV->u32;
//              WriteRegister(DSWAP16(ABT_SPSR), 4, &val);
		      break;
            case 0x1B:
              REGARM.und [2] = pV->u32;
//              WriteRegister(DSWAP16(UND_SPSR), 4, &val);
		      break;
		  }
          break;
        default:
          nErr = AG_INVALOP;        // invalid Operation
          break;
      }
      break;

    default:
      nErr = AG_INVALOP;            // invalid Operation
      break;
  }
  
  if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nErr);
}



/*
 * Set Memory-Attribute for address 'nAdr'
 *  After the File has been loaded by SARM.DLL, it 'colors' in
 *  the memory attributes such as ATRX_THUMB, ATRX_ARM or ATRX_NOINST.
 *  This allows for correct interpretation of the memory content
 *  (ARM vs. THUMB vs. Not an Instruction).
 *
 *  The typical attributes used here (also in combination) are:
 *    ATRX_EXEC    0x00000001       // 'executable' Attribute
 *    ATRX_READ    0x00000002       // 'readable' Attribute
 *    ATRX_WRITE   0x00000004       // 'writable' Attribute
 *    ATRX_THUMB   0x00000080       // Location contains 'Thumb' code
 *    ATRX_ARM     0x00008000       // Location contains 'ARM' code
 *    ATRX_NOINST  0x00080000       // not an instructuction, some constant or data location
 */

static void SetMemAttribute (UL32 nAdr, DWORD nAttr)  {
  UL32    *pAtr;

  pAtr = MGetAttr (nAdr);           // get pointer to Attributes for 'nAdr'
  if (pAtr != NULL)  {              // pointer is valid
    if (nAttr & (AG_ATR_THUMB | AG_ATR_ARM))  {
      *pAtr &= ~AG_ATR_NOINST;      // clear 'not an instruction' attribute
    }
    *pAtr |= nAttr;                 // or in new attribute
  }
}




#if 1     // an Example to find out the type of content of the memory

#define NO_INST     0               // not an instruction
#define THUMB_INST  1               // a thumb instruction
#define ARM_INST    2               // an arm instruction

int GetInstMode (UL32 nAdr)  {
  UL32    *pAtr;
  
  pAtr = MGetAttr (nAdr);           // get pointer to Attributes for 'nAdr'
  if (pAtr != NULL)  {              // pointer is valid
    if (*pAtr & AG_ATR_NOINST)  {   // not an instruction
      return (NO_INST);
    }
    if (*pAtr & AG_ATR_THUMB)  {    // a thumb instruction
      return (THUMB_INST);
    }
    if (*pAtr & AG_ATR_ARM)  {      // an arm instruction
      return (THUMB_INST);
    }
  }
  return (NO_INST);                 // unmapped address, treat as NO_INST
}

#endif



U32 _EXPO_ AG_MemAtt (U16 nCode, UL32 nAttr, GADR *pA)  {
  U16     nErr;

  if (iRun && !supp.MemAccR)  {     // currently running, can't access.
    return (AG_NOACCESS);           // error: can't access register
  }
  if (PlayDead) return(AG_NOACCESS);

  nErr = 0;
  switch (nCode)  {
    case AG_MEMMAP:                 // map memory address range
      pA->ErrAdr = MMapMem (nAttr, pA->Adr, pA->nLen);
      break;

    case AG_GETMEMATT:              // Attribute descriptor is requested
      pA->Adr = (UL32) MGetAttr (pA->Adr);
      break;

    case AG_SETMEMATT:              // Set Memory Attribute
      SetMemAttribute (pA->Adr, nAttr);
      break;
  }
  
  if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);  // terminate driver
  }
  return (nErr);
}



/*
 *  Analyze the type of breakpoint and if it is enabled or disabled
 */

UL32 MBpInfo (UL32 nAdr, U16 nCode)  {
  MAMAP         x;
  struct EMM  (*s)[256];
  struct EMM      acc;
  U16         *pB;
  UL32         nA;
  AG_BP       *pK, *pN;

  switch (nCode)  {
    case AG_BPDISALL:                  // disable all Exec-Bp's
    case AG_BPKILLALL:                 // kill all Exec-Bp's
      break;
    default:
      x.a32 = nAdr;                    // requested code address
      s =  slots [x.ub[3]];            // a24-a31

      if (s == NULL)  {
        MapSeg (x.a32 >> 16, x.a32 & 0xFFFF, 0x10000 - (x.a32 & 0xFFFF), 0);   // map a segment
        s = slots [x.ub[3]];
      }
      acc = (*s)[x.ub[2]];                         // slot available?
      if (acc.mem == NULL || acc.atr == NULL)  {
        return (0x80000000);                       // failed: unmapped address
      }
      pB = &((U16 *) acc.atr) [x.w16 >> 1];        // attributes


#if 0
      acc = (*s)[x.ub[2]];                         // slot available?
      if (acc.mem == NULL)  {
        acc.mem = (BYTE *) calloc(_MSGM, 1);       // map 64K + 4
        acc.atr = (DWORD *) calloc(_MSGM, 1);      // map 64K + 4
        (*s)[x.ub[2]] = acc;
     
        s =  slots [x.ub[3]];                      // slot available?
        if (s == NULL)  {
          return (0x80000000);                     // failed: unmapped address
        }
        acc = (*s)[x.ub[2]];                       // slot available?
        if (acc.mem == NULL || acc.atr == NULL)  { // exit if automap failed
          return (0x80000000);                     // failed: unmapped address
        }
      }
      pB = &((U16 *) acc.atr) [x.w16 >> 1];
#endif
      break;
  }

  nA = 0;
  switch (nCode)  {                                 // Function code
    case AG_BPQUERY:                                // Query Break
    case AG_BPEXQUERY:                              // Query for 'executed' attribute
      nA = (UL32)*pB & (ATRX_BREAK | ATRX_BPDIS);   // Attributes are similar to uVision
      break;

    case AG_BPENABLE:                               // enable breakpoint
      if (*pB & ATRX_BPDIS)  {                      // is Bp disabled ?
        *pB = (*pB & ~ATRX_BPDIS) | ATRX_BREAK;     // enable it
        break;
      }

    case AG_BPDISABLE:                              // disable breakpoint
      if (*pB & ATRX_BREAK)  {                      // is Bp enabled ?
        *pB = (*pB & ~ATRX_BREAK) | ATRX_BPDIS;     // disable it
        break;
      }
      break;

    case AG_BPKILL:                                 // kill breakpoint
      *pB = *pB & ~(ATRX_BREAK | ATRX_BPDIS);       // kill it
      break;

    case AG_BPSET:                                  // Set breakpoint
      *pB = (*pB & ~ATRX_BPDIS) | ATRX_BREAK;       // set breakpoint
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



/*
 * Get BreakPoints Infos
 */

U32 _EXPO_ AG_BpInfo (U16 nCode, void *vp)  {
  U32     n;

  n = 0;
  if (PlayDead) return(0);

  n = MBpInfo (((GADR *) vp)->Adr, nCode);
  
  if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (n);
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


AG_BP *MBreakFunc (U16 nCode, U16 n1, UL32 nAdr, AG_BP *pBp)  {
  MAMAP         x;
  struct EMM  (*s)[256];
  struct EMM      acc;
  U16         *pB;
  AG_BP       *bptemp;

  if (nCode != 5)  {
    x.a32 = nAdr;                    // requested code address
    s =  slots [x.ub[3]];            // a24-a31
    
    if (!s) {
      s = (struct EMM (*)[256])calloc(sizeof(struct EMM) * 256,1);
      slots[x.ub[3]] = s;
    }
    acc = (*s)[x.ub[2]];                       // slot available?
    if (acc.mem == NULL)  {
      acc.mem = (BYTE *) calloc(_MSGM, 1);      // map 64K + 4
      acc.atr = (DWORD *) calloc(_MSGM, 1);     // map 64K + 4
      (*s)[x.ub[2]] = acc;
   
      s =  slots [x.ub[3]];            // slot available?
      if (!s) return (NULL);
      acc = (*s)[x.ub[2]];      // slot available?
      if (acc.mem == NULL || acc.atr == NULL) // exit if automap failed
        return (NULL);               // failed: unmapped address
    }
    pB = &((WORD *)acc.atr)[x.w16>>1];  
  }

  switch (nCode)  {
    case 2:                          // Notification: 'pB' will be unlinked
      *pB &= ~(ATRX_BREAK | ATRX_BPDIS);
      break;

    case 3:                          // not sent
    case 6:                          // not sent
    case 7:                          // not sent
    case 8:                          // not sent
      break;

    case 1:                          // Notification: 'pB' will be linked
    case 4:                          // 'pB->enabled' may have changed
      if (pBp->enabled) {
        *pB = (*pB & ~ATRX_BPDIS) | ATRX_BREAK; // enable it
      } else {
        *pB = (*pB & ~ATRX_BREAK) | ATRX_BPDIS; // disable it
      }
      break;           

    case 5:                          // Bp-accept function
      abreaks = cbreaks = wbreaks = 0;       /* Assume no Conditional-Breaks */
      for ( bptemp = *pBhead ; bptemp ; bptemp = bptemp->next )  {
        if (bptemp->enabled) {
          switch (bptemp->type)  {
            case AG_ABREAK: ++abreaks; break;     /* increase Nr. of AddressBreaks */
            case AG_CBREAK: ++cbreaks; break;     /* increase Nr. of CondBreaks */
            case AG_WBREAK: ++wbreaks; break;     /* increase Nr. of WatchBreaks */
          }
        }
      }
      if (cbreaks) pBp = NULL;                   // monitor only supports ABREAKS

      break;                                    // TODO: set Bp to NULL if unsupported.
  }
    return (pBp);
}


_EXPO_ AG_BP *AG_BreakFunc (U16 nCode, U16 n1, GADR *pA, AG_BP *pB)  {

  if (PlayDead) return(NULL);

  pB = MBreakFunc (nCode, n1, pA->Adr, pB);
  
  if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (pB);
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
      break;

    case AG_GOTILADR:                 // run til 'pA->Adr' or some Bp,
      iRun = 1;                       // whichever comes first
      GoUntil (pA->Adr);
      StopRun = 0;
      Invalidate();                   // registers, caches, etc.
      iRun = 0;                       // clear 'executing' flag
      break;

    case AG_GOFORBRK:                 // run forever or till some Bp reached.
      iRun = 1;
      GoUntil (0xFFFFFFFF);           // means 'go forever'
      StopRun = 0;
      Invalidate();                   // registers, caches, etc.
      iRun = 0;                       // clear 'executing' flag
      break;
  }

  if (PlayDead)  {                    // target not connected
    StopTarget();                     // shut it down
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0); // terminate ourselves
  }
  else  {
    GetRegs();                        // get target registers
    *pCURPC = REGARM.cur[15];              // let uVision2 know about PC...
  }
  return (nE);
}




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
 * Trace-History access function (future extension)
 * Note: Trace is currently not supported when uVision2 is used with a target driver !
 */

_EXPO_ U32 AG_HistFunc (U32 nCode, I32 indx, I32 dir, void *vp)  {
  U32     nR = 0;

  if (PlayDead) return (0);
#if 0                  // TODO: Trace Recording Stuff
  switch (nCode)  {
    case 0:                              // Function #0: get Hist index
      break;       // dir:=1 forward, dir:=0 backward
    case 1:                              // Function #1: GetTracePC
      break;       // get PC of trace record 'indx'
    case 2:                              // Function #2: GetHistRegs
      break;       // get recorded regs of record 'indx'
    case 3:                              // Function #3: get Nr. of entries
      break;       // get number of instructions recorded so far
  }
  nR = HistFunc1 (nCode, indx, dir, (UC8 *) vp);
  return (nR);
#endif
  if (PlayDead == 1)  {
    StopTarget();
    PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
  }
  return (nR);
}


/*
 * New Register-Manager
 */

struct rGroup rGroups[] =  {
  { 0x00, 0x01, "Current",         },        // Group 0 (Current), show expanded
  { 0x00, 0x00, "User/System",     },        // Group 1 (User & System), collapsed
  { 0x00, 0x00, "Fast Interrupt",  },        // Group 2 (Fast Interrupt), collapsed
  { 0x00, 0x00, "Interrupt",       },        // Group 3 (Interrupt), collapsed
  { 0x00, 0x00, "Supervisor",      },        // Group 4 (Supervisor), collapsed
  { 0x00, 0x00, "Abort",           },        // Group 5 (Abort), collapsed
  { 0x00, 0x00, "Undefined",       },        // Group 6 (Undefined), collapsed
  { 0x00, 0x01, "Internal",        },        // Group 7 (Internal), show expanded
};

struct rItem rItems[] =  {
//--desc-nGi-nItem-szReg[]--isPC-cc-iHig--
  { 0x01, 0, 0x00, "R0",      0, 1, 0,  },
  { 0x01, 0, 0x01, "R1",      0, 1, 0,  },
  { 0x01, 0, 0x02, "R2",      0, 1, 0,  },
  { 0x01, 0, 0x03, "R3",      0, 1, 0,  },
  { 0x01, 0, 0x04, "R4",      0, 1, 0,  },
  { 0x01, 0, 0x05, "R5",      0, 1, 0,  },
  { 0x01, 0, 0x06, "R6",      0, 1, 0,  },
  { 0x01, 0, 0x07, "R7",      0, 1, 0,  },
  { 0x01, 0, 0x08, "R8",      0, 1, 0,  },
  { 0x01, 0, 0x09, "R9",      0, 1, 0,  },
  { 0x01, 0, 0x0A, "R10",     0, 1, 0,  },
  { 0x01, 0, 0x0B, "R11",     0, 1, 0,  },
  { 0x01, 0, 0x0C, "R12",     0, 1, 0,  },
  { 0x01, 0, 0x0D, "R13 (SP)",0, 1, 0,  },
  { 0x01, 0, 0x0E, "R14 (LR)",0, 1, 0,  },
  { 0x01, 0, 0x0F, "R15 (PC)",0, 1, 0,  },
  { 0x01, 0, 0x800,"CPSR",    0, 1, 0,  },
  { 0x01, 0, 0x801,"N",       0, 1, 0,  },
  { 0x01, 0, 0x802,"Z",       0, 1, 0,  },
  { 0x01, 0, 0x803,"C",       0, 1, 0,  },
  { 0x01, 0, 0x804,"V",       0, 1, 0,  },
  { 0x01, 0, 0x805,"I",       0, 1, 0,  },
  { 0x01, 0, 0x806,"F",       0, 1, 0,  },
  { 0x01, 0, 0x807,"T",       0, 1, 0,  },
  { 0x01, 0, 0x808,"M",       0, 1, 0,  },
  { 0x01, 0, 0x809,"",        0, 0, 0,  },  // Delimiter
  { 0x01, 0, 0x900,"SPSR",    0, 1, 0,  },
  { 0x01, 0, 0x901,"N",       0, 1, 0,  },
  { 0x01, 0, 0x902,"Z",       0, 1, 0,  },
  { 0x01, 0, 0x903,"C",       0, 1, 0,  },
  { 0x01, 0, 0x904,"V",       0, 1, 0,  },
  { 0x01, 0, 0x905,"I",       0, 1, 0,  },
  { 0x01, 0, 0x906,"F",       0, 1, 0,  },
  { 0x01, 0, 0x907,"T",       0, 1, 0,  },
  { 0x01, 0, 0x908,"M",       0, 1, 0,  },
  { 0x01, 0, 0x909,"",        0, 0, 0,  },  // Delimiter

  { 0x01, 1, 0x10, "R8",      0, 1, 0,  },
  { 0x01, 1, 0x11, "R9",      0, 1, 0,  },
  { 0x01, 1, 0x12, "R10",     0, 1, 0,  },
  { 0x01, 1, 0x13, "R11",     0, 1, 0,  },
  { 0x01, 1, 0x14, "R12",     0, 1, 0,  },
  { 0x01, 1, 0x15, "R13 (SP)",0, 1, 0,  },
  { 0x01, 1, 0x16, "R14 (LR)",0, 1, 0,  },

  { 0x01, 2, 0x20, "R8",      0, 1, 0,  },
  { 0x01, 2, 0x21, "R9",      0, 1, 0,  },
  { 0x01, 2, 0x22, "R10",     0, 1, 0,  },
  { 0x01, 2, 0x23, "R11",     0, 1, 0,  },
  { 0x01, 2, 0x24, "R12",     0, 1, 0,  },
  { 0x01, 2, 0x25, "R13 (SP)",0, 1, 0,  },
  { 0x01, 2, 0x26, "R14 (LR)",0, 1, 0,  },
  { 0x01, 2, 0x200,"SPSR",    0, 1, 0,  },
  { 0x01, 2, 0x201,"N",       0, 1, 0,  },
  { 0x01, 2, 0x202,"Z",       0, 1, 0,  },
  { 0x01, 2, 0x203,"C",       0, 1, 0,  },
  { 0x01, 2, 0x204,"V",       0, 1, 0,  },
  { 0x01, 2, 0x205,"I",       0, 1, 0,  },
  { 0x01, 2, 0x206,"F",       0, 1, 0,  },
  { 0x01, 2, 0x207,"T",       0, 1, 0,  },
  { 0x01, 2, 0x208,"M",       0, 1, 0,  },
  { 0x01, 2, 0x209,"",        0, 0, 0,  },  // Delimiter

  { 0x01, 3, 0x30, "R13 (SP)",0, 1, 0,  },
  { 0x01, 3, 0x31, "R14 (LR)",0, 1, 0,  },
  { 0x01, 3, 0x300,"SPSR",    0, 1, 0,  },
  { 0x01, 3, 0x301,"N",       0, 1, 0,  },
  { 0x01, 3, 0x302,"Z",       0, 1, 0,  },
  { 0x01, 3, 0x303,"C",       0, 1, 0,  },
  { 0x01, 3, 0x304,"V",       0, 1, 0,  },
  { 0x01, 3, 0x305,"I",       0, 1, 0,  },
  { 0x01, 3, 0x306,"F",       0, 1, 0,  },
  { 0x01, 3, 0x307,"T",       0, 1, 0,  },
  { 0x01, 3, 0x308,"M",       0, 1, 0,  },
  { 0x01, 3, 0x309,"",        0, 0, 0,  },  // Delimiter

  { 0x01, 4, 0x40, "R13 (SP)",0, 1, 0,  },
  { 0x01, 4, 0x41, "R14 (LR)",0, 1, 0,  },
  { 0x01, 4, 0x400,"SPSR",    0, 1, 0,  },
  { 0x01, 4, 0x401,"N",       0, 1, 0,  },
  { 0x01, 4, 0x402,"Z",       0, 1, 0,  },
  { 0x01, 4, 0x403,"C",       0, 1, 0,  },
  { 0x01, 4, 0x404,"V",       0, 1, 0,  },
  { 0x01, 4, 0x405,"I",       0, 1, 0,  },
  { 0x01, 4, 0x406,"F",       0, 1, 0,  },
  { 0x01, 4, 0x407,"T",       0, 1, 0,  },
  { 0x01, 4, 0x408,"M",       0, 1, 0,  },
  { 0x01, 4, 0x409,"",        0, 0, 0,  },  // Delimiter

  { 0x01, 5, 0x50, "R13 (SP)",0, 1, 0,  },
  { 0x01, 5, 0x51, "R14 (LR)",0, 1, 0,  },
  { 0x01, 5, 0x500,"SPSR",    0, 1, 0,  },
  { 0x01, 5, 0x501,"N",       0, 1, 0,  },
  { 0x01, 5, 0x502,"Z",       0, 1, 0,  },
  { 0x01, 5, 0x503,"C",       0, 1, 0,  },
  { 0x01, 5, 0x504,"V",       0, 1, 0,  },
  { 0x01, 5, 0x505,"I",       0, 1, 0,  },
  { 0x01, 5, 0x506,"F",       0, 1, 0,  },
  { 0x01, 5, 0x507,"T",       0, 1, 0,  },
  { 0x01, 5, 0x508,"M",       0, 1, 0,  },
  { 0x01, 5, 0x509,"",        0, 0, 0,  },  // Delimiter

  { 0x01, 6, 0x60, "R13 (SP)",0, 1, 0,  },
  { 0x01, 6, 0x61, "R14 (LR)",0, 1, 0,  },
  { 0x01, 6, 0x600,"SPSR",    0, 1, 0,  },
  { 0x01, 6, 0x601,"N",       0, 1, 0,  },
  { 0x01, 6, 0x602,"Z",       0, 1, 0,  },
  { 0x01, 6, 0x603,"C",       0, 1, 0,  },
  { 0x01, 6, 0x604,"V",       0, 1, 0,  },
  { 0x01, 6, 0x605,"I",       0, 1, 0,  },
  { 0x01, 6, 0x606,"F",       0, 1, 0,  },
  { 0x01, 6, 0x607,"T",       0, 1, 0,  },
  { 0x01, 6, 0x608,"M",       0, 1, 0,  },
  { 0x01, 6, 0x609,"",        0, 0, 0,  },  // Delimiter

  { 0x01, 7, 0x70, "PC  $",   1, 1, 0,  },
  { 0x01, 7, 0x71, "Mode",    0, 0, 0,  },
  { 0x01, 7, 0x72, "States",  0, 0, 0,  },
  { 0x01, 7, 0x73, "Sec",     0, 0, 0,  },

};



struct bmsk  {
  U32       mask;
  I32       shft;
};

static const struct bmsk bPSR[] = {          // PSR
  { 0x80000000, 31, },   // N
  { 0x40000000, 30, },   // Z
  { 0x20000000, 29, },   // C
  { 0x10000000, 28, },   // V
  { 0x00000080,  7, },   // I
  { 0x00000040,  6, },   // F
  { 0x00000020,  5, },   // T
  { 0x0000001F,  0, },   // M4..M0
};

static BYTE           nRmode;
static BYTE           nRstat;
static RgARM             uRg;         // shadow registers
static char        szV [128];         // temporary buffer
static DWORD             nPC;         // temporary PC

/*
 * uVision2 want's to display some registers value...
 */
static void RegGet (RITEM *vp, int nR)  {
  GVAL      v;
  U32       n, z;

  switch (nR & 0xF0000000)  {
    case UPR_NORMAL:                       // Setup Normal Regs
      if (!RegUpToDate && !iRun) GetRegs();
      uRg = REGARM;  
      for (n = 1; n < 7; n++) rGroups[n].ShEx &= ~0x02;
	  switch (uRg.cpsr & 0x1F) {
        case 0x1F:  // System Mode
        case 0x10:  // User Mode
          rGroups[1].ShEx |= 0x02;         // User & System Bold
		  break;
        case 0x11:  // FIQ Mode
          rGroups[2].ShEx |= 0x02;         // Fast Interrupt Bold
	      break;
        case 0x12:  // IRQ Mode
          rGroups[3].ShEx |= 0x02;         // Interrupt Bold   
	      break;
        case 0x13:  // Supervisor Mode
          rGroups[4].ShEx |= 0x02;         // Supervisor Bold
	      break;
        case 0x17:  // Abort Mode
          rGroups[5].ShEx |= 0x02;         // Abort Bold        
	      break;
        case 0x1B:  // Undefined
          rGroups[6].ShEx |= 0x02;         // Undefined Bold        
	      break;
	  }
      nRmode = 0;                          // normal mode
      if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
      // TODO frees all allocated memory and closes the serial connection
        PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
      }

      return;

    case UPR_HIST:                         // Setup History Regs
#if 0
      v.i32 = nR & 0xFFFF;                 // History index (max. 64K entries)
      if (ioc.numtra != MAXTRC) v.i32 = (ioc.numtra - v.i32) & (MAXTRC - 1);
      else                      v.i32 = (ioc.curtra - v.i32) & (MAXTRC - 1);
      uRg    = *(((struct RgARM *) ioc.trap) + v.i32);  // HistRegs
      nRstat = 1;                          // mark CPSR as current.
      nRmode = 1;                          // history mode
#endif 
     return;
  }

  v.u64 = 0;
  vp->iDraw = 0;

  switch (vp->nItem)  {

    case 0x00:  case 0x01:  case 0x02:  case 0x03:
    case 0x04:  case 0x05:  case 0x06:  case 0x07:
    case 0x08:  case 0x09:  case 0x0A:  case 0x0B:
    case 0x0C:  case 0x0D:  case 0x0E:  case 0x0F:
	  v.u32 = uRg.cur [vp->nItem];                         // Current Mode
x32:  if (v.u32 != vp->v.u32 || vp->szVal[0] == 0)  {      // value changed
        sprintf (vp->szVal, "0x%08x", v.u32);
      }
      break;

    case 0x800:
	  v.u32 = uRg.cpsr;                                    // CPSR
	  goto x32;
    case 0x801:                                            // N
    case 0x802:                                            // Z
    case 0x803:                                            // C
    case 0x804:                                            // V
    case 0x805:                                            // I
    case 0x806:                                            // F
    case 0x807:                                            // T
      n = vp->nItem - 0x801;                               // use item-number as index
      z = uRg.cpsr;
xbit: v.u32 = (z & bPSR[n].mask) >> bPSR[n].shft;          // masked normalized value
      if (v.u32 != vp->v.u32 || vp->szVal[0] == 0)  {      // value changed
        sprintf (vp->szVal, "%d       ", v.u32);           // spaces to ease mouse hit.
      }
      break;
	case 0x808:                                            // M4..M0
	  z = uRg.cpsr;
xm:   v.u32 = z & 0x1F;
	  if (v.u32 != vp->v.u32 || vp->szVal[0] == 0)  {
		sprintf (vp->szVal, "0x%02x", v.u32);
	  }
	  break;

    case 0x900:
	  v.u32 = uRg.spsr;                                    // Current SPSR
	  goto x32;
    case 0x901:                                            // N
    case 0x902:                                            // Z
    case 0x903:                                            // C
    case 0x904:                                            // V
    case 0x905:                                            // I
    case 0x906:                                            // F
    case 0x907:                                            // T
      n = vp->nItem - 0x901;                               // use item-number as index
      z = uRg.spsr;
	  goto xbit;
	case 0x908:                                            // M4..M0
      z = uRg.spsr;
	  goto xm;

    case 0x10:  case 0x11:  case 0x12:  case 0x13:
    case 0x14:  case 0x15:  case 0x16:
	  v.u32 = uRg.usr [vp->nItem & 0x0F];                  // User & System
	  goto x32;

    case 0x20:  case 0x21:  case 0x22:  case 0x23:
    case 0x24:  case 0x25:  case 0x26:
	  v.u32 = uRg.fiq [vp->nItem & 0x0F];                  // Fast Interrupt
	  goto x32;

    case 0x200:
	  v.u32 = uRg.fiq [7];                                 // SPSR_fiq
	  goto x32;
    case 0x201:                                            // N
    case 0x202:                                            // Z
    case 0x203:                                            // C
    case 0x204:                                            // V
    case 0x205:                                            // I
    case 0x206:                                            // F
    case 0x207:                                            // T
      n = vp->nItem - 0x201;                               // use item-number as index
      z = uRg.fiq [7];
	  goto xbit;
	case 0x208:                                            // M4..M0
	  z = uRg.fiq [7];
	  goto xm;

    case 0x30:  case 0x31:
	  v.u32 = uRg.irq [vp->nItem & 0x0F];                  // Interrupt
	  goto x32;

    case 0x300:
	  v.u32 = uRg.irq [2];                                 // SPSR_irq
	  goto x32;
    case 0x301:                                            // N
    case 0x302:                                            // Z
    case 0x303:                                            // C
    case 0x304:                                            // V
    case 0x305:                                            // I
    case 0x306:                                            // F
    case 0x307:                                            // T
      n = vp->nItem - 0x301;                               // use item-number as index
      z = uRg.irq [2];
	  goto xbit;
	case 0x308:                                            // M4..M0
	  z = uRg.irq [2];
	  goto xm;

    case 0x40:  case 0x41:
	  v.u32 = uRg.svc [vp->nItem & 0x0F];                  // Supervisor
	  goto x32;

    case 0x400:
	  v.u32 = uRg.svc [2];                                 // SPSR_svc
	  goto x32;
    case 0x401:                                            // N
    case 0x402:                                            // Z
    case 0x403:                                            // C
    case 0x404:                                            // V
    case 0x405:                                            // I
    case 0x406:                                            // F
    case 0x407:                                            // T
      n = vp->nItem - 0x401;                               // use item-number as index
      z = uRg.svc [2];
	  goto xbit;
	case 0x408:                                            // M4..M0
	  z = uRg.svc [2];
	  goto xm;

    case 0x50:  case 0x51:
	  v.u32 = uRg.abt [vp->nItem & 0x0F];                  // Abort
	  goto x32;

    case 0x500:
	  v.u32 = uRg.abt [2];                                 // SPSR_abt
	  goto x32;
    case 0x501:                                            // N
    case 0x502:                                            // Z
    case 0x503:                                            // C
    case 0x504:                                            // V
    case 0x505:                                            // I
    case 0x506:                                            // F
    case 0x507:                                            // T
      n = vp->nItem - 0x501;                               // use item-number as index
      z = uRg.abt [2];
	  goto xbit;
	case 0x508:                                            // M4..M0
	  z = uRg.abt [2];
	  goto xm;

    case 0x60:  case 0x61:
	  v.u32 = uRg.und [vp->nItem & 0x0F];                  // Undefined
	  goto x32;
      
    case 0x600:
	  v.u32 = uRg.und [2];                                 // SPSR_und
	  goto x32;
    case 0x601:                                            // N
    case 0x602:                                            // Z
    case 0x603:                                            // C
    case 0x604:                                            // V
    case 0x605:                                            // I
    case 0x606:                                            // F
    case 0x607:                                            // T
      n = vp->nItem - 0x601;                               // use item-number as index
      z = uRg.und [2];
	  goto xbit;
	case 0x608:                                            // M4..M0
	  z = uRg.und [2];
	  goto xm;

    case 0x70:                            // 'PC  $'
	  nPC = uRg.cur[15] & ((uRg.cpsr & 0x20) ? 0xFFFFFFFE : 0xFFFFFFFC);
      if (nPC != vp->v.u32 || vp->szVal[0] == 0)  {
        // ioc.DecodeAdr (0, nPC, vp->szVal);
		sprintf (vp->szVal, "0x%08X", nPC);
        vp->iDraw = 1;
      }
      vp->v.u32 = nPC;
      vp->iHigh = 0;                      // never highlighted
      return;

	case 0x71:                            // Mode
	  z = uRg.cpsr;
      v.u32 = z & 0x1F;
	  if (v.u32 != vp->v.u32 || vp->szVal[0] == 0)  {
		switch (v.u32) {
		  case 0x10: sprintf (vp->szVal, "User");       break;
		  case 0x11: sprintf (vp->szVal, "FIQ");        break;
		  case 0x12: sprintf (vp->szVal, "IRQ");        break;
		  case 0x13: sprintf (vp->szVal, "Supervisor"); break;
		  case 0x17: sprintf (vp->szVal, "Abort");      break;
		  case 0x1B: sprintf (vp->szVal, "Undefined");  break;
		  case 0x1F: sprintf (vp->szVal, "System");     break;
		  default:   sprintf (vp->szVal, "???");
		}
	  }
	  break;
/*
    case 0x72:                            // States
      v.u64 = uRg.nCycles;
      if ((U64) uRg.nCycles != vp->v.u64 || vp->szVal[0] == 0)  {
        sprintf (vp->szVal, "%I64u", uRg.nCycles);
        vp->iDraw = 1;
      }
      vp->v.u64 = uRg.nCycles;            // Cycles as reference value
      vp->iHigh = 0;                      // never highlighted
      return;

    case 0x73:                            // Seconds
      if ((U64) uRg.nCycles != vp->v.u64 || vp->szVal[0] == 0)  {
        if (ioc.OscFrq != 0)  {
          vp->iDraw = 1;
          dr = ioc.pCalcTime (uRg.nCycles);  // 16.12.99
          p1 = &szV[0];
          if (v.u64 < 10000) sprintf (p1, "%10.8f", dr);
          else               sprintf (p1, "%10.8g", dr);
          while (*p1 == ' ') ++p1;
          strcpy (vp->szVal, p1);
        }
        else vp->szVal[0] = 0;            // no OscFrq, no time !
      }
      vp->v.u64 = uRg.nCycles;            // Cycles as reference value
      vp->iHigh = 0;                      // never highlighted 
      return;
*/
  }

  if (v.u32 == vp->v.u32)  {              // value unchanged
    if (vp->iHigh)  {                     // item yet highlighted
      vp->iHigh = 0;                      // clear highlight color
      vp->iDraw = 1;                      // need repaint.
    }
  }
  else  {                                 // value changed
    vp->iHigh = 1;
    vp->iDraw = 1;                        // need repaint
  }
  vp->v.u64 = v.u64;                      // update 'On Screen' value
}

/*
 * uVision2 want's to change some registers value...
 */

static I32 RegSet (RITEM *vp, GVAL *pV)  {
  I32       n;
  U32     val;

  if (nRmode == 1) return (0);            
  if (PlayDead)    return (0);
  if (iRun)        return (0);
  switch (vp->nItem)  {
    default:
      return (0);

    case 0x00:  case 0x01:  case 0x02:  case 0x03:
    case 0x04:  case 0x05:  case 0x06:  case 0x07:
	  uRg.cur [vp->nItem] = pV->u32;
x:    SetRegs (&uRg);
      break;

	case 0x0F:
	  uRg.cur [15] = pV->u32;
	  goto xPC;

    case 0x08:  case 0x09:  case 0x0A:  case 0x0B:
    case 0x0C:
      switch (REGARM.cpsr & 0x1F) {
        case 0x1F:
        case 0x10:
        case 0x12:
        case 0x13:
        case 0x17:
        case 0x1B:
          uRg.usr [vp->nItem - 0x08] = pV->u32;
		  break;
        case 0x11:
          uRg.fiq [vp->nItem - 0x08] = pV->u32;
		  break;
		default:
	      uRg.cur [vp->nItem] = pV->u32;
	  }
	  goto x;

    case 0x0D:  case 0x0E:
      switch (REGARM.cpsr & 0x1F) {
        case 0x1F:
        case 0x10:
          uRg.usr [vp->nItem - 0x08] = pV->u32;
		  break;
        case 0x11:
          uRg.fiq [vp->nItem - 0x08] = pV->u32;
		  break;
        case 0x12:
          uRg.irq [vp->nItem - 0x0D] = pV->u32;
		  break;
        case 0x13:
          uRg.svc [vp->nItem - 0x0D] = pV->u32;
		  break;
        case 0x17:
          uRg.abt [vp->nItem - 0x0D] = pV->u32;
		  break;
        case 0x1B:
          uRg.und [vp->nItem - 0x0D] = pV->u32;
		  break;
		default:
	      uRg.cur [vp->nItem] = pV->u32;
	  }
	  goto x;

    case 0x800:                           // CPSR
    case 0x801:                           // N
    case 0x802:                           // Z
    case 0x803:                           // C
    case 0x804:                           // V
    case 0x805:                           // I
    case 0x806:                           // F
    case 0x807:                           // T
  	case 0x808:                           // M4..M0
	  if (vp->nItem == 0x800) {
        uRg.cpsr = pV->u32;
	  }
	  else {
        n   = vp->nItem - 0x801;          // use Item-number for index
        val = pV->u32;
	    val = (val << bPSR[n].shft) & bPSR[n].mask;
	    uRg.cpsr &= ~bPSR[n].mask;
	    uRg.cpsr |= val;
	  }
      SetRegs (&uRg);
      // ioc.Notify (22, NULL);              // update various windows.
      break;

    case 0x900:                           // Current SPSR
      switch (REGARM.cpsr & 0x1F) {
        case 0x11:
          uRg.fiq [7] = pV->u32;
		  break;
        case 0x12:
          uRg.irq [2] = pV->u32;
		  break;
        case 0x13:
          uRg.svc [2] = pV->u32;
		  break;
        case 0x17:
          uRg.abt [2] = pV->u32;
		  break;
        case 0x1B:
          uRg.und [2] = pV->u32;
		  break;
	  }
      goto x;
    case 0x901:                           // N
    case 0x902:                           // Z
    case 0x903:                           // C
    case 0x904:                           // V
    case 0x905:                           // I
    case 0x906:                           // F
    case 0x907:                           // T
    case 0x908:                           // M4..M0
      n   = vp->nItem - 0x901;            // use Item-number for index
      val = pV->u32;
      val = (val << bPSR[n].shft) & bPSR[n].mask;
      switch (REGARM.cpsr & 0x1F) {
        case 0x11:
          uRg.fiq [7] &= ~bPSR[n].mask;
          uRg.fiq [7] |= val;
          break;
        case 0x12:
          uRg.irq [2] &= ~bPSR[n].mask;
          uRg.irq [2] |= val;
	  break;
        case 0x13:
          uRg.svc [2] &= ~bPSR[n].mask;
          uRg.svc [2] |= val;
          break;
        case 0x17:
          uRg.abt [2] &= ~bPSR[n].mask;
          uRg.abt [2] |= val;
          break;
        case 0x1B:
          uRg.und [2] &= ~bPSR[n].mask;
          uRg.und [2] |= val;
          break;
      }
      goto x;

    case 0x10:  case 0x11:  case 0x12:  case 0x13:
    case 0x14:  case 0x15:  case 0x16:
      uRg.usr [vp->nItem & 0x0F] = pV->u32;
      goto x;

    case 0x20:  case 0x21:  case 0x22:  case 0x23:
    case 0x24:  case 0x25:  case 0x26:
      uRg.fiq [vp->nItem & 0x0F] = pV->u32;
      goto x;

    case 0x200:                           // SPSR_fiq
      uRg.fiq [7] = pV->u32;
      goto x;

    case 0x201:                           // N
    case 0x202:                           // Z
    case 0x203:                           // C
    case 0x204:                           // V
    case 0x205:                           // I
    case 0x206:                           // F
    case 0x207:                           // T
    case 0x208:                           // M4..M0
      n   = vp->nItem - 0x201;            // use Item-number for index
      val = pV->u32;
      val = (val << bPSR[n].shft) & bPSR[n].mask;
      uRg.fiq [7] &= ~bPSR[n].mask;
      uRg.fiq [7] |= val;
      goto x;

    case 0x30:  case 0x31:
      uRg.irq [vp->nItem & 0x0F] = pV->u32;
      goto x;

    case 0x300:                           // SPSR_irq
      uRg.irq [2] = pV->u32;
      goto x;

    case 0x301:                           // N
    case 0x302:                           // Z
    case 0x303:                           // C
    case 0x304:                           // V
    case 0x305:                           // I
    case 0x306:                           // F
    case 0x307:                           // T
    case 0x308:                           // M4..M0
      n   = vp->nItem - 0x301;            // use Item-number for index
      val = pV->u32;
      val = (val << bPSR[n].shft) & bPSR[n].mask;
      uRg.irq [2] &= ~bPSR[n].mask;
      uRg.irq [2] |= val;
      goto x;

    case 0x40:  case 0x41:
      uRg.svc [vp->nItem & 0x0F] = pV->u32;
      goto x;

    case 0x400:                           // SPSR_svc
      uRg.svc [2] = pV->u32;
      goto x;

    case 0x401:                           // N
    case 0x402:                           // Z
    case 0x403:                           // C
    case 0x404:                           // V
    case 0x405:                           // I
    case 0x406:                           // F
    case 0x407:                           // T
    case 0x408:                           // M4..M0
      n   = vp->nItem - 0x401;            // use Item-number for index
      val = pV->u32;
      val = (val << bPSR[n].shft) & bPSR[n].mask;
      uRg.svc [2] &= ~bPSR[n].mask;
      uRg.svc [2] |= val;
      goto x;

    case 0x50:  case 0x51:
      uRg.abt [vp->nItem & 0x0F] = pV->u32;
      goto x;

    case 0x500:                           // SPSR_abt
      uRg.abt [2] = pV->u32;
      goto x;

    case 0x501:                           // N
    case 0x502:                           // Z
    case 0x503:                           // C
    case 0x504:                           // V
    case 0x505:                           // I
    case 0x506:                           // F
    case 0x507:                           // T
    case 0x508:                           // M4..M0
      n   = vp->nItem - 0x501;            // use Item-number for index
      val = pV->u32;
      val = (val << bPSR[n].shft) & bPSR[n].mask;
      uRg.abt [2] &= ~bPSR[n].mask;
      uRg.abt [2] |= val;
      goto x;

    case 0x60:  case 0x61:
      uRg.und [vp->nItem & 0x0F] = pV->u32;
      goto x;

    case 0x600:                           // SPSR_und
      uRg.und [2] = pV->u32;
      goto x;

    case 0x601:                           // N
    case 0x602:                           // Z
    case 0x603:                           // C
    case 0x604:                           // V
    case 0x605:                           // I
    case 0x606:                           // F
    case 0x607:                           // T
    case 0x608:                           // M4..M0
      n   = vp->nItem - 0x601;            // use Item-number for index
      val = pV->u32;
      val = (val << bPSR[n].shft) & bPSR[n].mask;
      uRg.und [2] &= ~bPSR[n].mask;
      uRg.und [2] |= val;
      goto x;

    case 0x70:
      if (iRun) return (0);    // $ PC: can't change while running...
      uRg.cur [15] = pV->ul & ((uRg.cpsr & 0x20) ? 0xFFFFFFFE : 0xFFFFFFFC);
xPC:  SetRegs (&uRg);
      WritePC (uRg.cur[15]);
      // ioc.Notify (30, NULL);       // $ has changed, sync Views
      break;

//  case 0x71:                            // mode, can't change
//  case 0x72:                            // states, can't change
//  case 0x73:                            // sec, can't change
//    break;
    if (PlayDead == 1) {   // Disables the driver after the communication breaks down.
      // TODO frees all allocated memory and closes the serial connection
      PostMessage (hMfrm, Uv2Msg, MSG_UV2_TERMINATE, 0);
    }

  }
  RegUpToDate = 0;
  return (1);                             // Ok.
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
