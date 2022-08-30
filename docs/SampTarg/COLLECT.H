#ifndef __COLLECT_INCED__
  #define __COLLECT_INCED__

/*
 * Collect.h
 */

#include "Agdi.h"

#define nCFAMILY   8051              // identifes '8051 Target driver'


extern int       DoDlgSetup (void);  // target-config setup dialog

extern U32   ReInitTarget (void);
extern void    StopTarget (void);



/*************************************************/
/* definition of communication control character */
/*************************************************/
#define STX    0x02
#define CtrlC  0x03
#define ENQ    0x05
#define ACK    0x06
#define CtrlQ  0x11  /* DC1 */
#define CtrlS  0x13
#define NACK   0x15
//#define ESC    0x1b
#define CHKS   0xfe		// temporary solution
#define UNDEF  0xfd		// temporary solution


#if 0    // Serial I/O stuff
extern void	   ConstructSPort (void);
extern void	    DestructSPort (void);
extern int          InitSPort (UL32 portnr, UL32 baud);
extern BOOL	  StartMonitoring (void);
extern BOOL	RestartMonitoring (void);
extern BOOL	   StopMonitoring (void);
extern void	      WriteToPort (UC8 *string,int nLength);
extern BYTE     SyncAfterStep (void);
extern void          FlushSer (void);
#endif




extern void       AnalyzeMonParas (char *pPath, char *pArgs);
extern void         WriteMonParms (char *pArgs);
extern char          MonErrBuffer[500];


union mamap {
  UL32      a32;
  U16       w16;
  UC8     ub[4];
};
#define MAMAP   union mamap

struct MonConf  {
  DWORD      comnr;      // 1 = COM1, 2 = COM2 ...
  DWORD   baudrate;      // Baudrate for serial interface
  DWORD        Opt;      // see monitor cache option definitions below.
  char   MonPath [MAX_PATH + 2];
}; 
extern struct MonConf MonConf;


// monitor cache options
#define CACHE_DATA     0x0001
#define CACHE_XDATA    0x0002
#define CACHE_CODE     0x0004

// Flash Download options
#define FLASH_ERASE    0x0100
#define FLASH_PROGRAM  0x0200
#define FLASH_VERIFY   0x0400


extern HWND   hwnd_ser_err;

extern HWND         *pHwnd;
extern HWND          hMfrm;      // pointer to parent handle (CMainFrame)
extern HMODULE       hInst;      // this DLL's instance handle
extern UL32         Uv2Msg;      // Uv2-registered Message token
extern pCBF        pCbFunc;      // call-back function of s166
extern struct dbgblk *pdbg;      // startup values


extern BYTE     StopGoStep;      // Stop Go or single step function
extern BYTE       PlayDead;      // Disables the driver after the communication breaks down.
extern BYTE       bootflag;      // Specifies whether first boot loader is in progress or not
extern BYTE    RegUpToDate;      // Specifies whether register structure is up to date
extern BYTE       NotInMon;      // 1:=Remote Setup Mode via Options-Debug


void txtout (char *fmt, ...);    // this is a debug 'printf'

#define AG_ATR_UPTD   0x8000     // 'Up to date' Attribute
#define AG_ATR_WRPROT 0x1000     // Location is (software) write protected
#define AG_ATR_MOD    0x0080     // Location has been modified


#define  _MSGM   (65536 + 4)
#define  _ASGM   _MSGM * 2

struct MMM  {
  UC8   *mem;                // Pointer to Memory Image
  U16   *atr;                // Pointer to Memory Attributes
};
extern struct MMM mslots [520];

extern void   CacheData (BYTE *pB, DWORD nAdr, BYTE nCnt);
extern void  Invalidate (void);    // clean up after Go/Step


#endif
