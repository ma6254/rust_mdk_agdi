#ifndef COMTYP_INCED
  #define COMTYP_INCED

/*
 * Common type defintions
 */

#ifndef _WINDEF_        // WinDef.h not included
 typedef   signed char         CHAR;
 typedef unsigned int         DWORD;
 typedef   signed int         INT32;
 typedef unsigned short int  UINT16;
// typedef unsigned short int    WORD;
 typedef signed int           INT32;
 typedef unsigned char         BOOL;
#endif

typedef unsigned char          BYTE;
typedef   signed short int    INT16;
typedef unsigned short int   WORD16;
typedef __int64               INT64;
typedef unsigned __int64     UINT64;
typedef WORD16                 *WFP;
typedef BYTE                   *CFP;


#define iMCS51   0           // ToolSet Definitions
#define iMCS251  1
#define iMCS166  2
#define iMCSARM  3           // Gnu-ARM tools
#define iARMADS  4           // ARM-ADS tools
#define iARMKEIL 5           // Keil-ARM tools

#define LIMMCS   iARMADS     // last valid environment
#define MAXTDRV  40          // max. target drivers per [Section]
#define MAXRTOS  12          // max. RTOS entries per [Section]
#define MAXAGSI  20          // max. automatically loaded AGSI DLL's per [Section]

#define MAXTIP   1024



union bw  {
  WORD16    word;        // UINT16
  BYTE   byte[2];
};

union uw  {
  WORD16   ui[2];
  DWORD       ul;
};

#pragma pack(1)


/*
 * 28.10.2001: MainFrame.CPP:
 */
extern int GetTabSize (int nTools, int fType);



/*
 * Message-Subtypes for Registered Message 'WM_OUTOUT':
 */

#define MSG_UV2_TEXTOUT     0x0002
#define MSG_UV2_SERBOUT     0x0003
#define MSG_UV2_SERWOUT     0x0004
#define MSG_UV2_TERMINATE   0x0005
#define MSG_UV2_TRNLINE     0x0006   // Write into Build-Output window
#define MSG_UV2_AFTERBLD    0x0007   // post build/rebuild/translate
#define MSG_UV2_AFTERSVCS   0x0008   // post SVCS command
#define MSG_UV2_AFTERLINT   0x0009   // post LINT command
#define MSG_UV2_NOTIFY      0x000A   // Notify message
#define MSG_UV2_SERVWKEY    0x000B   // Key pressed in Serial View
#define MSG_UV2_CMDLINE     0x000C   // Debug: write into command window
#define MSG_UV2_MSGSTRING   0x000D   // Setup Message String
#define MSG_UV2_PRJCHANGE   0x000E   // Project File has changed
#define MSG_UV2_SERBPACK    0x000F   // packed serial byte out
#define MSG_UV2_CMDBUFLINE  0x0010   // Write puffered line into Command-Window
#define MSG_UV2_STARTDBG    0x0011   // Start Debugging
#define MSG_UV2_REBUILD     0x0012   // Start Build/Rebuild
#define MSG_UV2_STOPBUILD   0x0013   // Stop Build/Rebuild


#define EBRK  struct edbrk

struct edbrk  {
  EBRK          *next;
  DWORD         nLine;       // line number
  DWORD          nAdr;       // and it's address

  WORD16     type : 3;       // 0:=AdrBrk, 1:=CondBrk, 2:=WatchBrk
  WORD16  enabled : 1;       // Break is enabled/disabled
  WORD16   BytObj : 1;       // WatchBrk: 0:=Bytes, 1:=Objects
  DWORD          many;       // WatchBrk: many objects or bytes
  DWORD         tsize;       // WatchBrk: size of object
  WORD16          acc;       // WatchBrk: 1:=Read, 2:=Write, 3:=Both

//WORD16       number;       // BreakPoint-Number
//WORD16       BitPos;
  int          rcount;       // Break is taken when rcount = 1
  char           *cmd;       // Exec-Command
  char          *Line;       // Breakpoint-Expression Line for Display

  EBRK         *enext;       // for editor internal use
  BYTE         killed;       // for editor internal use
  char  pF [MAX_PATH + 2];   // name of file
};



#define SITEM  struct sItem

struct sItem  {
  struct sItem  *next;
  char            *pS;       // item text
  char            *pL;       // button label
  WORD16         nTyp;       // SubType
};

#pragma pack()


struct pax  {
  INT64       cyc;
  double      tim;
};


#pragma pack(1)
typedef struct  {            // Memory-Range descriptor
  BYTE          mTyp;        // 0=RAM, 1=ROM
  DWORD       nStart;        // Memory start address
  DWORD        nSize;        // Size of memory block (0=unused)
} MEMRANGE;        
#pragma pack()


/*
 * Debug-Startup values
 */

struct dbgblk  {
  char    MainDll [MAX_PATH];    // 's166.dll'
  char    PeriDll [MAX_PATH];    // '80167.dll'
  char   MainArgs [MAX_PATH];    // args to MainDll
  char   PeriArgs [MAX_PATH];    // args to PeriDll

  char    TargDll [MAX_PATH];    // target dll (MON, Emu,...)
  char       TargArgs [1024];    // Arguments to/from MON,Emu,...
  char       tKey [MAX_PATH];    // ...Mon166.DLL --> Mon166

  char         *pjPath;          // Project Path
  char    AppName [MAX_PATH];    // App name to load
  char    IniFile [MAX_PATH];    // INI-file to execute
  char    FncFile [MAX_PATH];    // File with target specific functions

  EBRK           *pBrk;          // list of breakpoints
  SITEM          *pWa1;          // Watch #1 Items
  SITEM          *pWa2;          // Watch #2 Items
  SITEM         *pMem1;          // Memory #1 Item
  SITEM         *pMem2;          // Memory #2 Item
  SITEM          *pBox;          // ToolBox Buttons
  SITEM          *pPad;          // PA definitions
  SITEM          *pSym;          // SymDlg Settings
//SITEM          *pCov;          // Coverage Dialog attrib(s)

  DWORD       targ : 1;
  DWORD      LdApp : 1;          // load app at on startup
  DWORD     Gomain : 1;          // Sim:  Go till main()
  DWORD     Rbreak : 1;          // Sim: restore breakpoints
  DWORD     Rwatch : 1;          // Sim: restore watchpoints
  DWORD       Rmem : 1;          // Sim: restore memory display
  DWORD      Rfunc : 1;          // Sim: restore functions
  DWORD       Rbox : 1;          // Sim: restore Toolbox
  DWORD   AutoFail : 1;          // Sim: Autoload failed

  SITEM         *pMem3;          // Memory #3 Item
  SITEM         *pMem4;          // Memory #4 Item

  char     RtxDll [MAX_PATH];    // Rtx-Display dll
  char    RtxArgs [1024];        // Arguments to/from Rtx-Dll,...
  char     RtxKey [MAX_PATH];    // ...RtxTiny.DLL --> RtxTiny

                                 // iMCS166
  MEMRANGE   ExtMem[6];          // up to 6 external memory ranges
  MEMRANGE        Ican;          // address range of on-chip CAN
  MEMRANGE        Irom;          // address range of on-chip internal Rom
  MEMRANGE       Xram1;          // address range of on-chip internal Xram
  MEMRANGE       Xram2;          // address range of on-chip internal Xram
  MEMRANGE        Iram;          // address range of on-chip internal Ram
  MEMRANGE      Dflash;          // address range of on-chip data flash
  BYTE          uocROM;          // use on-chip rom (166+51)
  BYTE         uocXCAN;          // use on-chip xram+can
  BYTE            Rtos;          // OS: 0:=none, 1:=RtxTiny, 2:=RtxFull
  BYTE          Mod167;          // 1:=use 167 instructions
                                 // Common
  DWORD          Clock;          // clock frequency
  char     Vendor [64];          // Device-Vendor: example 'Siemens'
  char     Device [64];          // Device-Name: example 'C167CR-16FM'

                                 // iMCS51
  WORD16         RomSz;          // 0:=Rom(Sm), 1:=Rom(Cp), 2:=Rom(La)
  DWORD    uocArit : 1;   // use on chip Arithmetic Unit
  DWORD    uocMdpt : 1;   // use multiple data pointers
  DWORD    uocXRAM : 1;   // use on chip XRAM
  DWORD    hadIRAM : 1;   // IRAM(...) given per database
  DWORD    hadXRAM : 1;   // XRAM(...) given per database
  DWORD    hadIROM : 1;   // IROM(...) given per database
  DWORD      Moda2 : 1;   // MODA2 (Atmel dual dptr)
  DWORD     Moddp2 : 1;   // MODDP2 (Dallas dual dptr)
  DWORD      Modp2 : 1;   // MODP2 (Philips+Temic dual dptr)
  DWORD   Mod517dp : 1;   // multiple dptr for Infineon C500
  DWORD   Mod517au : 1;   // Arithmetic Unit for Infineon C500
  DWORD      Mode2 : 1;   // MODE2 given per database
  DWORD     ModAB2 : 1;   // Analog Devices B2 Series given per Database
  DWORD      useCB : 1;   // CODE-banking (1:=activated)
  DWORD      useXB : 1;   // XDATA-banking (1:=activated)
//DWORD    useL251 : 1;   // use L251 instead of BL51
//DWORD    useA251 : 1;   // use A251 instead of A51
  DWORD    Moddp2x : 1;   // Dallas 89C420
  DWORD     Mod390 : 1;   // Dallas 89C390
  DWORD    ModC812 : 1;   // ADuC812 DPP Register freigeben
  DWORD      Mx51P : 1;   // 80C51MX+Ext


  WORD16        cBanks;   // 0:=2, 1:=4, 2:=8, 3:=16, 4:=32
  WORD16        xBanks;   // 0:=2, 1:=4, 2:=8, 3:=16, 4:=32

  MEMRANGE         rCB;   // CODE-bank  range
  MEMRANGE         rXB;   // XDATA-bank range
  MEMRANGE    eprom[3];   // off-chip code memory
  MEMRANGE    exram[3];   // off-chip xram memory
  MEMRANGE        irom;   // IROM range
  MEMRANGE        iram;   // IRAM range
  MEMRANGE        xram;   // XRAM range

  char     CanDll [MAX_PATH];    // Rtx-Display dll
  char    CanArgs [1024];        // Arguments to/from Can-Dll,...
//char     CanKey [MAX_PATH];    // ...Can.DLL

  char    szMsg [1024];          // for error-message transfer to Uv2
  char     ** AgsiDlls;   // HS 9.4.2002 line added: Pointer to string array containing names of AGSI DLL's

//---more possible Bitfields go on here...
  DWORD          n1Res;   // reserved for future devices
  DWORD          n2Res;   // reserved for future devices
  DWORD          n3Res;   // reserved for future devices
  DWORD          n4Res;   // reserved for future devices
  DWORD          n5Res;   // reserved for future devices
  DWORD          n6Res;   // reserved for future devices
  DWORD          n7Res;   // reserved for future devices
//---
};




/*
 * Language-defines for INI-Env strings
 */

#define LANG_N      0       // undefined language
#define LANG_C      1       // C
#define LANG_A      2       // Asm
#define LANG_CPP    3       // C++ 
#define LANG_UT     4       // Utils



/*
 * Exec: Translators
 */

#define  xxWHAT    0        /* unknown translator */
#define  xxC251    1
#define  xxC51     2
#define  xxC166    3
#define  xxPLM51   4
#define  xxL251    5
#define  xxL51     6
#define  xxL166    7
#define  xxA51     8
#define  xxA251    9
#define  xxA166   10
#define  xxA66    11
#define  xxC66    12
#define  xxL66    13
#define  xxLIB166 14
#define  xxLIB66  15
#define  xxLIB51  16
#define  xxLIB251 17
#define  xxOH51   18
#define  xxOH251  19
#define  xxOH166  20
#define  xxOH66   21
#define  xxOC51   22
#define  xxAX51   23      // Ax51 (Philips 80C51MX)
#define  xxLX51   24
#define  xxCX51   25
#define  xxLIBX51 26
#define  xxOHX51  27
#define  xxLP51   28
#define  xxCC51   29  // Ceibo ECPP Preprocessor
#define  xxECP166 30  // Edg-EC++ for 166/167
#define  xxGARMCC 31  // Gnu-ARM-CC
#define  xxGARMAS 32  // Gnu-ARM-AS
#define  xxGARMLD 33  // Gnu-ARM-LD
#define  xxGARMOH 34  // Gnu-ARM-ObjCopy (Obj-Elf/Hex)
#define  xxADSCC  35  // ARM-ADS: armcc
#define  xxADSAS  36  // ARM-ADS: armas
#define  xxADSLD  37  // ARM-ADS: armlink
#define  xxADSFE  38  // ARM-ADS: fromelf
#define  xxADSAR  39  // ARM-ADS: armar
#define  xxADSCPP 40  // ARM-ADS: armcpp
#define  xxADSTCC 41  // ARM-ADS: tcc (Thumb-Mode-Compiler)

#define  xxUSER   42
#define  xxLINT   43




#ifdef __cplusplus
  extern "C"  {
#endif


/*
 * in HelpFox.Asm: (in any Syntax-builder help file)
 */

extern int         HELPEXPR;
extern int         CANTHELP;
extern int        HELPSTART;



extern void  ShowError (BOOL box, char *fmt, ...);
extern void TrnOutLine (char *pLine);    // into Build Window
extern void    TrnOutL (char *pLine);    // into Build Window (thread-safe)
extern void CmdOutLine (char *pLine);    // into Command Window
extern void  CmdCmdOut (char *pLine);    // into Command Window
extern void OutOutLine (char *pLine);    // into Output Window
extern void   DoEvents (void);           // keep System alive

extern DWORD   Notify1 (DWORD nCode, void *lp1);  // within thread
extern DWORD    Notify (DWORD nCode, void *lp1);  // thread-safe.


extern int  SerByteOut (int nCom, BYTE *pS, DWORD nMany);
extern int  SerWordOut (int nCom, WORD16 *pW, DWORD nMany);
extern void   BootHelp (DWORD hlpExpr, DWORD noHelp, DWORD hStart);



extern WORD16 HandleCommand (char *pC, int nLen);   // Uv3.cpp




struct vExec  {
  WORD16       nTrans;          // Translator number
  char          *pCmd;          // Command line

  char         *pUnit;          // unit (unit name - src-name)
  char        *pfUnit;          // full path of unit

  char       *pExPath;          // Executable path (with \)
  char        *pSpawn;          // Spawn command and path
  char       *cmdFile;          // pathname of command file
  char       *objFile;          // pathname of object file
  char          *pEnv;          // Environment or NULL
  char        *cpptrn;          // name of ECPP trn via Tools.ini

  int          Status;          // Process status
  int          Errors;          // Errors
  int        Warnings;          // Warnings
  int        exitCode;
  int             PID;          // Process ID

  WORD16    isObj : 1;          // File is an OBJ file, can't create
  WORD16    isLib : 1;          // File is a  LIB file, can't create
  WORD16  PubOnly : 1;
  WORD16   GnuArm : 1;
  WORD16   GnuHex : 1;          // ObjCopy: gen hex file

  WORD16         mark;          // RegColor-Mark from DEPLST
  WORD16        nBank;          // BankNo
};


extern void ExecTrn (struct vExec *pX);      // run translator
extern char   szFileCT [512];                // current TargApp's pathname



#ifdef __cplusplus
  }                           // End of 'extern "C"'
#endif

#endif
