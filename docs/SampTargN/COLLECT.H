#ifndef __COLLECT_INCED__
  #define __COLLECT_INCED__

/*
 * Collect.h
 */

#include "Agdi.h"




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


/*
 * Register-Definitions
 */

#define nR0     0x00     // R0
#define nR1     0x01     // R1
#define nR2     0x02     // R2
#define nR3     0x03     // R3
#define nR4     0x04     // R4
#define nR5     0x05     // R5
#define nR6     0x06     // R6
#define nR7     0x07     // R7
#define nR8     0x08     // R8
#define nR9     0x09     // R9
#define nR10    0x0A     // R10
#define nR11    0x0B     // R11
#define nR12    0x0C     // R12
#define nR13    0x0D     // R13
#define nR14    0x0E     // R14
#define nR15    0x0F     // R15

#define nCPSR   0x10     // CPSR
#define nSPSR   0x11     // SPSR

#define iR0     0x80     // @R0
#define iR1     0x81     // @R1
#define rAB     0x12     // AB
#define rCY     0x13     // C
#define rA      0x14     // A
#define rDPTR   0x15     // DPTR
#define iDPTR   0x16     // @DPTR
#define iADPTR  0x17     // @A+DPTR
#define iAPC    0x18     // @A+PC

#define nWR0    0x20     // WR0
#define nWR1    0x21     // WR2
#define nWR2    0x22     // WR4
#define nWR3    0x23     // WR6
#define nWR4    0x24     // WR8
#define nWR5    0x25     // WR10
#define nWR6    0x26     // WR12
#define nWR7    0x27     // WR14
#define nWR8    0x28     // WR16
#define nWR9    0x29     // WR18
#define nWR10   0x2A     // WR20
#define nWR11   0x2B     // WR22
#define nWR12   0x2C     // WR24
#define nWR13   0x2D     // WR26
#define nWR14   0x2E     // WR28
#define nWR15   0x2F     // WR30

#define iWR0    0x30     // @WR0
#define iWR15   0x3F     // @WR30
#define iWR0p   0x40     // @WR0  + disp16
#define iWR15p  0x4F     // @WR30 + disp16

#define nDR0    0x50     // DR0
#define nDR1    0x51     // DR4
#define nDR2    0x52     // DR8
#define nDR3    0x53     // DR12
#define nDR4    0x54     // DR16
#define nDR5    0x55     // DR20
#define nDR6    0x56     // DR24
#define nDR7    0x57     // DR28
#define nDR8    0x58     // DR32
#define nDR14   0x5E     // DR56
#define nDR15   0x5F     // DR60

#define iDR0    0x60     // @DR0
#define iDR7    0x67     // @DR28
#define iDR0p   0x70     // @DR0  + disp16
#define iDR7p   0x77     // @DR28 + disp16


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

// cache operations

#define WRITE_BEHIND_CACHE  0x0001    
#define READ_CODE_CACHE     0x0002
#define READ_CACHE          0x0004


// monitor cache options
#define CACHE_CODE     0x0001
#define CACHE_MEM      0x0002
#define CACHE_SFR      0x0004
#define CACHE_MONLOAD  0x0008
#define CACHE_PERI     0x0010

extern char   MonRestrArea[200];


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

/*
 * Map Memory
 */

#define ATRX_EXEC    0x00000001   // 'executable' Attribute
#define ATRX_READ    0x00000002   // 'readable' Attribute
#define ATRX_WRITE   0x00000004   // 'writable' Attribute
#define ATRX_WATCH   0x00000008   // Location has a Watch
#define ATRX_THUMB   0x00000080   // iMCSARM: 'Thumb' code
#define ATRX_ARM     0x00008000   // iMCSARM: 'ARM' code
#define ATRX_WRBRK   0x00010000   // Loc has a write-access break
#define ATRX_RDBRK   0x00020000   // Loc has a read-access break
#define ATRX_PAP     0x00040000   // Location has a Perf.-Analyzer point
#define ATRX_NOINST  0x00080000   // iMCSARM: 'No instruction' Attribute

#define ATRX_EXECD   0x00000100   // 'Executed' Attribute
#define ATRX_ITAKEN  0x00000200   // iMCSARM: Instruction-taken Attribute
#define ATRX_BREAK   0x00000400   // 'Exec-Break' Attribute
#define ATRX_BPDIS   0x00000800   // 'disabled Exec-Break' Attribute
#define ATRX_EXECDO  0x01000000   // iMCSARM: 'Executed' Attribute for odd WORD address (Thumb)
#define ATRX_ITAKENO 0x02000000   // iMCSARM: Instruction-taken Attribute for odd WORD address (Thumb)
#define ATRX_BREAKO  0x04000000   // iMCSARM: 'Exec-Break' Attribute for odd WORD address (Thumb)
#define ATRX_BPDISO  0x08000000   // iMCSARM: 'disabled Exec-Break' Attribute for odd WORD address (Thumb)

#define ATRX_UPTD    0x00000010   // 'Up to date' Attribute

#define  _MSGM   (65536 + 4)



/* Extended Memory Management */
struct EMM {
  BYTE   *mem;          // Pointer to Memory Image
  DWORD  *atr;          // Pointer to Memory Attributes
};



extern void   CacheData (BYTE *pB, DWORD nAdr, BYTE nCnt);
extern void  Invalidate (void);    // clean up after Go/Step


#endif
