#ifndef __AGDI__INCED___
  #define __AGDI__INCED___


//---Revision History: ----------------------------------------------
//   17.10.2000, added AG_SYM_SFR
//
//-------------------------------------------------------------------


#ifdef __cplusplus
  extern "C"  {
#endif

#include <Windows.h>          // wg. HWND, RECT, CALLBACK, HANDLE

#define _EXPO_   __declspec(dllexport)


/*
 * Advanced GDI types
 */

typedef  unsigned long       UL32;
typedef    signed long       SL32;
typedef    signed char        SC8;
typedef  unsigned char        UC8;
typedef    signed int         I32;
typedef  unsigned int         U32;
typedef    signed short int   I16;
typedef  unsigned short int   U16;
typedef  __int64              I64;
typedef  unsigned __int64     U64;
typedef  float                F32;
typedef  double               F64;


typedef union  {
  U32            u32;       // 32-Bit signed int
  I32            i32;       // 32-Bit unsigned int
  UL32            ul;       // 32-Bit unsigned long
  SL32            sl;       // 32-Bit signed long
  UC8             uc;       // 8-Bit unsigned char
  SC8             sc;       // 8-Bit signed char
  U16            u16;       // 16-Bit unsigned short int
  I16            i16;       // 16-Bit signed short int
  U64            u64;       // 64-Bit unsigned int
  I64            i64;       // 64-Bit signed int
  F32            f32;       // 32-Bit single precision float
  F64            f64;       // 64-Bit single precision float
  UL32       ul2 [2];
  SL32       sl2 [2];
  U16       u16a [4];
  I16       i16a [4];
  UC8        uc8 [8];
  SC8        sc8 [8];
  SC8            *pS;
  UC8            *pU;
  U16            *pW;
  U32            *pD;
} GVAL;


/*
 * 166/167 specific's
 */

#define mWREG  0x100      // 0x100...0x10F := R0...R15
#define mBREG  0x200      // 0x200...0x20F := RL0...RH7
#define mDREG  0x300      // not used.
#define mDPP   0x400      // 0x400...0x403 := DPP0...DPP3
#define mPC    0x500      // PC
#define mPSW   0x600      // PSW
#define mSP    0x700      // SP
#define mCP    0x800      // CP
#define mCYC   0x900      // Cycles value


#pragma pack(1)
typedef struct  {
  union  {
    U16     wregs [16];      // R0  ... R15
    UC8     bregs [16];      // RL0 ... RH7
  } r;
  U32          Ndpp[4];      // full linear base address values
  U32              nPC;      // full address !
  U16             cPsw;      // current Psw
  U16              cSP;
  U16             cMDL;      // MDL
  U16             cMDH;      // MDH
  U32              cCP;      // CP
  I64          nCycles;      // cycle counter (or time ???)
} RG166;


/*
 * Sle66+ specific's
 */

#define nnR0    0x00     // R0
#define nnR1    0x01     // R1
#define nnR2    0x02     // R2
#define nnR3    0x03     // R3
#define nnR4    0x04     // R4
#define nnR5    0x05     // R5
#define nnR6    0x06     // R6
#define nnR7    0x07     // R7
//#define niR0    0x10     // @R0
//#define niR1    0x11     // @R1
//#define nrAB    0x12     // AB
#define nrCY    0x13     // C
#define nrA     0x14     // A
#define nrDPTR  0x18     // DPTR

#define nnPR0    0x1C     // PR0 (80C51MX)
#define nnPR1    0x1D     // PR1 (80C51MX)



typedef struct  {            // iMCS51 - SLE66Cxx Registers
  UL32           nPC;
  I64        nCycles;        // cycle counter
  UC8            psw;
  UC8             sp;
  UC8              b;
  UC8            acc;
  UC8            dpl;
  UC8            dph;
  UC8      bregs [8];
  UC8          dpsel;
  UC8          adrxh;        // page for MOVX @Ri
  U16       dptx [8];        // DPTR save area
} RGSLE66;

typedef struct  {            // iMCS51-Registers
  BYTE         Rn [16];      // R0 ... R7
  DWORD            nPC;      // full address !
  BYTE              sp;      // SP
  BYTE             psw;      // PSW-sfr
  BYTE               b;      // B-sfr
  BYTE             acc;      // ACC-sfr
  BYTE             dpl;      // DPL-sfr
  BYTE             dph;      // DPH-sfr
  BYTE        ports[8];
  I64          nCycles;      // cycle counter
} RG51;


typedef struct   {           // iMCSARM Registers
  DWORD  cur[16];            // Current Mode:   R0..R15(PC)
  DWORD  cpsr;               // CPSR
  DWORD  spsr;               // Current SPSR
  DWORD  usr[7];             // User & System:  R8..R14
  DWORD  fiq[8];             // Fast Interrupt: R8..R14, SPSR
  DWORD  irq[3];             // Interrupt:      R13,R14, SPSR
  DWORD  svc[3];             // Supervisor:     R13,R14, SPSR
  DWORD  abt[3];             // Abort:          R13,R14, SPSR
  DWORD  und[3];             // Undefined:      R13,R14, SPSR
  INT64  nCycles;            // cycle counter
} RgARM ;

typedef struct  {           // SmartMX-Registers
  BYTE     Rn [32];         // 7F:0000 .. 7F:001F (D:0 .. D:1F)
  BYTE         acc, pcl, pch, pcx, psw, pswh;

// System Mode Registers
  BYTE         s_B,  s_EPL,   s_EPM,   s_EPH,  s_DPL0, s_DPH0;
  BYTE      s_DPL1, s_DPH1, s_MXCON, s_AUXR1, s_XRAMP;
  BYTE        s_SP,  s_SPE, s_SPLOW,s_SPELOW,s_SPHIGH,s_SPEHIGH;
//User Context Registers
  BYTE         u_B,  u_EPL,   u_EPM,   u_EPH,  u_DPL0, u_DPH0;
  BYTE      u_DPL1, u_DPH1, u_MXCON, u_AUXR1, u_XRAMP;
  BYTE        u_SP,  u_SPE, u_SPLOW,u_SPELOW,u_SPHIGH,u_SPEHIGH;
// Physical PC Value
  BYTE       YPCL, YPCH, YPCX;
  BYTE    MMUCTRL;
  BYTE    MMUPTRL, MMUPTRM, MMUPTRH;
} RGSMX;

#pragma pack()


#pragma pack(1)

#define RGROUP    struct rGroup
#define RITEM     struct rItem
#define REGDSC    struct RegDsc

struct rGroup  {             // Register Group Descriptor
  UC8            desc;       // always 0x00
  UC8            ShEx;       // Bit.0 = 1  Show Group initially expanded
                             // Bit.1 = 1  Show Group description 'bold'
  char          *name;       // Group name
};

struct rItem  {              // Register Item Descriptor
  UC8            desc;       // always 0x01
  U16             nGi;       // Group-Index (0...nGroups-1)
  U16           nItem;       // Item indicator
  char      szReg[16];       // Name of Register
  UC8            isPC;       // is this the PC
  UC8          canChg;       // can this Reg be changed in Reg-Window
  UC8           iHigh;       // highlight the value
  UC8           iDraw;       // item needs to be repainted
  char      szVal[32];       // it's value in Ascii
  GVAL              v;       // it's binary item value
};

struct RegDsc  {
  I32         nGitems;       // number of group items
  I32         nRitems;       // number of register items
  RGROUP      *GrpArr;       // array of group descriptors
  RITEM       *RegArr;       // array of register descriptors
  void     (*RegGet) (RITEM *vp, int nR);   // get RegItem's value
  I32      (*RegSet) (RITEM *vp, GVAL *pV); // set RegItem's value
};
  
#pragma pack()

#define UPR_NORMAL  0x80000000  // update normal registers
#define UPR_HIST    0x40000000  // update history registers





/*
 * Advanced GDI Functions
 */

/*
 * Error codes
 */

#define  AG_OK          0     // Ok.
#define  AG_NOACCESS    1     // Cannot access while running.
#define  AG_RDFAILED    2     // memory read failed.
#define  AG_INVALOP     3     // invalid operation code
#define  AG_RO          4     // attempt to write Read-only item
#define  AG_WRFAILED    5     // memory write failed.
#define  AG_CANTMAP     6     // cannot map memory



/*
 * 80x51 + Sle66(+) memory spaces, added 12/19/99
 */

#define   amNONE   0x0000           // not spaced
#define   amXDATA  0x0001           // XDATA
#define   amBANK0  0x0080           // BANK0
#define   amBANK31 0x009F           // BANK31
#define   amDATA   0x00F0           // DATA
#define   amBIT    0x00F1           // BIT
#define   amEDATA  0x00F2           // EDATA (i251)
#define   amIDATA  0x00F3           // IDATA
#define   amECODE  0x00F4           // 251 ecode
#define   amHDATA  0x00F5           // 251 hdata
#define   amHCONS  0x00F6           // 251 hconst
#define   amCONST  0x00F7           // 251 const
#define   amPDATA  0x00FE           // PDATA (c51 macht das bei generic)
#define   amCODE   0x00FF           // CODE 
#define   amPHYS   0x0100           // Physical SLE66+ Memory
#define   amPHLIM  0x0110           // 1MB Physical Memory

#define   amYMEM   0x0040           // Mifare ProX Y: (physical code 1MB)
#define   amZMEM   0x0041           // Mifare ProX Z: (physical xdata)
#define   amSMEM   0x0042           // Mifare ProX S: (physical sfr)

#define   amU1MEM  0x00F8           // UsrMem #1 (E2-PROM)
#define   amU2MEM  0x00F9           // UsrMem #2 (reserved for future use)
#define   amU3MEM  0x00FA           // UsrMem #3 (reserved for future use)
#define   amU4MEM  0x00FB           // UsrMem #4 (reserved for future use)
#define   amU5MEM  0x00FC           // UsrMem #5 (reserved for future use)
#define   mmU6MEM  0x00FD           // UsrMem #6 (reserved for future use)

/*
 * Features
 */

typedef struct  {
  U32       MemAccR : 1;      // memory-access while running supported
  U32       RegAccR : 1;      // register-access while running supported
  U32        hTrace : 1;      // trace-recording supported
  U32        hCover : 1;      // code coverage supported
  U32       hPaLyze : 1;      // Performance-Analyzer support
  U32       hMemMap : 1;      // Memory-Map support
  U32        ResetR : 1;      // Reset possible while running
} SUPP;


#pragma pack(1)
typedef struct  {
  UL32         Adr;          // linear address
  UL32      ErrAdr;          // memory access failed address
  UL32        nLen;          // address range, used for memory-map
  U16       mSpace;          // memory space (not used on 166/167)
} GADR;


typedef struct  {            // Assemble/Disassemble
  UL32           Adr;        // linear address
  UC8        Opc [8];        // 167 needs max. 4 Opcode bytes
  I32         OpcLen;        // Result length in assemble mode
  SC8      szB [256];        // DisAsm-result or Asm-input line
  I32         Result;        // 0:=Ok, otherwise error
} DAAS;


typedef struct  {            // Memory-Range descriptor
  UC8           mTyp;        // 0=RAM, 1=ROM
  UL32        nStart;        // Memory start address
  UL32         nSize;        // Size of memory block (0=unused)
} OCM;        


typedef struct  {            // 166/167 Device Info
  UC8     Vendor [64];       // Device-Vendor: example 'Siemens'
  UC8     Device [64];       // Device-Name: example 'C167CR-16FM'
  UL32          Clock;       // clock frequency
  UC8       RestoreBp;       // 1 := restore Breakpoints
  UC8            Rtos;       // OS: 0:=none, 1:=RtxTiny, 2:=RtxFull
  UC8          Mod167;       // 0:=166-Instr. Set 1:=167-Instr Set
  UC8    useOnChipRom;       // 1:=use on chip Rom
  UC8   useOnChipXper;       // 1:=use on chip X-peripherals (XRAM+CAN)
  UC8          useMAC;       // 0:=no MACC, 1:=MACC instr. used

  OCM       ExtMem[6];       // up to 6 external memory ranges
  OCM            Ican;       // address range of on-chip CAN
  OCM            Irom;       // address range of on-chip internal Rom
  OCM           Xram1;       // address range of on-chip internal Xram
  OCM           Xram2;       // address range of on-chip internal Xram
  OCM            Iram;       // address range of on-chip internal Ram

  UC8   PrjPath [260];       // path of project
  UC8   AppName [260];       // path and name of loaded application
} DEV_X66;

#pragma pack()




/*
 * Symbol search masks (may be combined using |) :
 */

#define AG_SYM_VAR  0x0001         // search for non-bit Variables
#define AG_SYM_CON  0x0002         // search for named Constants
#define AG_SYM_BIT  0x0004         // search for Bit in Memory
#define AG_SYM_LOC  0x0008         // search for Function/Label
#define AG_SYM_SFR  0x0200         // search for SFR name

/*
 * Type of found symbol:
 */

#define  AG_TP_VOID     0
#define  AG_TP_BIT      1
#define  AG_TP_CHAR     2
#define  AG_TP_UCHAR    3
#define  AG_TP_INT      4
#define  AG_TP_UINT     5
#define  AG_TP_SHORT    6
#define  AG_TP_USHORT   7
#define  AG_TP_LONG     8
#define  AG_TP_ULONG    9
#define  AG_TP_FLOAT    10
#define  AG_TP_DOUBLE   11
#define  AG_TP_PTR      12
#define  AG_TP_UNION    13
#define  AG_TP_STRUCT   14
#define  AG_TP_FUNC     15
#define  AG_TP_STRING   16
#define  AG_TP_ENUM     17
#define  AG_TP_FIELD    18

#pragma pack(1)

typedef struct  {                  // Search for Sym by Name or Value.
  UL32        nMask;               // search mask (AG_SYM_LOC | ...)
  UC8  szName [256];               // search/found name (zero-terminated
  U64           val;               // search/found Adr/Value
  UL32         type;               // type of found symbol (AG_TP_???)
  UL32           Ok;               // 1:=Ok, else find failed.
} SYMDSC;

#pragma pack()


/*
 * Progress-Control Structure
 */

#define PROGRESS_INIT     1
#define PROGRESS_KILL     2
#define PROGRESS_SETPOS   3

#pragma pack(1)
typedef struct PgRess  {   // Progress-Setup and Control structure
  I32       Job;           // PROGRESS_INIT/KILL/SETPOS
  I32       pos;           // PROGRESS_SETPOS: position to set
  I32       low;           // low percent (normally 0) 
  I32       hig;           // high percent (normally 100)
  SC8    *label;           // text-label before progress-bar or NULL
} OIL;
#pragma pack()


/*
 * Flash-Parameter-Block, used in AG_CB_GETFLASHPARAM CallBack
 */
#pragma pack(1)
typedef struct FlashBlock   {
  UL32           start;   // Start-Address
  UL32            many;   // Number of Bytes
  UC8           *image;   // Content
  UL32         ActSize;   // total number of bytes
  UL32        Stop : 1;   // cancel FlashDownLoad
  UL32        Res [16];   // reserved, unused
} FLASHPARM;
#pragma pack()



/*
 * Init/Query Target features
 */

#define AG_INITFEATURES   0x0100       // Init target features
#define AG_GETFEATURE     0x0200       // extract a single feature
#define AG_INITITEM       0x0300       // Init item
#define AG_EXECITEM       0x0400       // Execute item


#define AG_F_MEMACCR      0x0001       // memory-access while running
#define AG_F_REGACCR      0x0002       // register-access while running
#define AG_F_TRACE        0x0003       // back-trace support
#define AG_F_COVERAGE     0x0004       // code-coverage support
#define AG_F_PALYZE       0x0005       // Performance-Analyzer support
#define AG_F_MEMMAP       0x0006       // Memory-Map support
#define AG_F_RESETR       0x0007       // Reset while running possible feature
#define AG_F_251MODE      0x0008       // 251: get mode (SRC_MODE/BIN_MODE)
#define AG_F_251FRMSZ     0x0009       // 251: get framesize (FRM_SIZE2/4)
#define AG_F_251CPU       0x000A       // 251: get CPU type (CPU_251/CPU_51)


                                       // Items for 'AG_INITITEM':
#define AG_INITMENU       0x0007       // init extension menu
#define AG_INITEXTDLGUPD  0x0008       // init modeless extesion dlg update function
#define AG_INITMHANDLEP   0x0009       // setup ptr to HWND of active modeless dlg
#define AG_INITPHANDLEP   0x000A       // pointer to parent handle (MainFrame)
#define AG_INITINSTHANDLE 0x000B       // pointer to Agdi-instance handle
#define AG_INITBPHEAD     0x000E       // pointer to Bp-head
#define AG_INITCURPC      0x000F       // pointer to Program counter
#define AG_INITDOEVENTS   0x0010       // pointer do DoEvents()
#define AG_INITUSRMSG     0x0011       // Registered Message for SendMessage
#define AG_INITCALLBACK   0x0012       // pointer to callback function

#define AG_UNINIT         0x000C       // Clean up target
#define AG_RESET          0x000D       // Reset target system
#define AG_GETMODE        0x000E       // for S8051.DLL: Get Dallas Contigious Mode
//---26.10.2000: added for Hitex Emulators:
#define AG_RUNSTART       0x0010       // Announcement: Go/Step about to start
#define AG_RUNSTOP        0x0011       //             : Go/Step completed.
//---

//---19.7.2002: added for Flash DownLoad:
#define AG_INITFLASHLOAD  0x0013       // Prepare for Flash Download
#define AG_STARTFLASHLOAD 0x0014       // Start Flash DownLoad

/*
 * Call-Back-Function in S166 and supported sub-functions
 */

typedef U32 (*pCBF) (U32 nCode, void *vp);
#define AG_CB_TRUEXPR      1       // vp := 'EXP *' (use for Bp->ep)
#define AG_CB_PROGRESS     2       // vp := 'struct PgRess *'
#define AG_CB_INITREGV     3       // vp := 'REGDSC *' (into RegView)
#define AG_CB_EXECCMD      4       // vp := 'char *' command string
#define AG_CB_FORCEUPDATE  5       // vp := NULL, force general windows update
#define AG_CB_DISASM       6       // vp := 'DAAS *', disasm opcodes
#define AG_CB_INLASM       7       // vp := 'DAAS *', assemble szB[] into Opc[]
#define AG_CB_MSGSTRING    8       // vp := 'char *' text for message pane
#define AG_CB_GETDEVINFO   9       // vp := 'DEV_X66 *', get device info
#define AG_CB_SYMBYVAL    10       // vp := 'SYMDESC *', find symbol by value
#define AG_CB_SYMBYNAME   11       // vp := 'SYMDESC *', find symbol by name
#define AG_CB_SLE66MM     12       // vp := &slots[0] out of [512]
#define AG_CB_PHYS2MMU    13       // vp := (void *) ((DWORD) physAdr)
#define AG_CB_MMU2PHYS    14       // vp := (void *) ((DWORD) logicalAdr)
#define AG_CB_GETFLASHPARAM  15    // vp := (FLASHPARM *) or NULL


#ifdef _IN_TARG_         // define if used in Mon166,Emu...
  extern _EXPO_ U32   AG_Init (U16 nCode, void *vp);
#else                    // S166
  extern        U32 (*AG_Init) (U16 nCode, void *vp);
#endif



/*
 * Memory attributes
 */

#define AG_ATR_EXEC   0x01    // 'executable ' Attribute
#define AG_ATR_READ   0x02    // 'readable' Attribute
#define AG_ATR_WRITE  0x04    // 'writable' Attribute
#define AG_ATR_BREAK  0x08    // 'Exec-Break' Attribute
#define AG_ATR_EXECD  0x10    // 'Executed' Attribute
#define AG_ATR_WATCH  0x20    // Location has a Watch
#define AG_ATR_BPDIS  0x40    // 'disabled Exec-Break' Attribute
#define AG_ATR_PAP    0x80    // Location has a Perf.-Analyzer point
#define AG_ATR_WRBRK  0x100   // Loc has a write-access break
#define AG_ATR_RDBRK  0x200   // Loc has a read-access break
#define AG_ATR_COMC   0x400   // iMCS51/251: Common code marker
#define AG_ATR_VNM    0x800   // iMCS51: von Neumann mapped
#define AG_ATR_BEP    0x1000  // iMCS51: Bank-Entry Point
#define AG_ATR_EXTR   0x2000  // 166/167: within range of an EXTR sequence
#define AG_ATR_JTAKEN 0x4000  // Jump-taken attribute



#define AG_MEMMAP     0x01    // map memory
#define AG_GETMEMATT  0x02    // get memory attribute
#define AG_SETMEMATT  0x03    // set memory attribute


#ifdef _IN_TARG_         // define if used in Mon166,Emu...
  extern _EXPO_ U32   AG_MemAtt (U16 nCode, UL32 nAttr, GADR *pA);
#else
  extern        U32 (*AG_MemAtt) (U16 nCode, UL32 nAttr, GADR *pA);
#endif



/*
 * Breakpoint query and access
 */

#define AG_ABREAK  0        // simple code address break
#define AG_CBREAK  1        // conditional break (check after each instr.)
#define AG_WBREAK  2        // data access break
#define AG_RBREAK  3        // Address-Range Break (currently not used)


#pragma pack(1)
struct AG_Bps  {
  struct AG_Bps *next;
  struct AG_Bps *prev;

  UL32       type : 4;      // ABREAK, CBREAK, WBREAK
  UL32    enabled : 1;      // 1:=enabled, 0:=disabled
  UL32     ReCalc : 1;      // recalc expr flag
  UL32     BytObj : 1;      // WatchBrk: 0:=Bytes, 1:=Objects

  UL32            Adr;      // Address (or Range)
  UL32         mSpace;      // memory-space
  void            *pV;      // used on VTR-access breakpoints

  UL32          tsize;      // WatchBrk: size of one object
  UL32           many;      // WatchBrk: many objects or bytes
  U16             acc;      // WatchBrk: 1:=Read, 2:=Write, 3:=ReadWrite
  U16          BitPos;      // currently not used
  UL32         number;      // BreakPoint-Number
  I32          rcount;      // Break is taken when rcount = 1

  I32          ocount;      // Original Count
  void            *ep;      // conditional-Expression
  char           *cmd;      // Exec-Command
  char          *Line;      // Breakpoint-Expression Line for Display
  char            *pF;      // module file name
  UL32          nLine;      // line number
  UC8          Opc[8];      // Opcode-Save Area for Monitors
};
#pragma pack()

#define AG_BP  struct AG_Bps



#define AG_BPQUERY     0x01
#define AG_BPTOGGLE    0x02       // not sent to target
#define AG_BPINSREM    0x03       // not sent to target
#define AG_BPACTIVATE  0x04       // not sent to target
#define AG_BPDISALL    0x05       // Notification: all Bp's have been disabled
#define AG_BPKILLALL   0x06       // Notification: all Bp's have been killed
#define AG_BPEXQUERY   0x07
                                  // added these /8.2.99/
#define AG_BPENABLE    0x08       // Notification: Enable Bp at address
#define AG_BPDISABLE   0x09       // Notification: Disable Bp at address
#define AG_BPKILL      0x0A       // Notification: Kill Bp at address
#define AG_BPSET       0x0B       // Notification: Set Bp at address


#ifdef _IN_TARG_         // define if used in Mon166,Emu...
  extern _EXPO_ U32    AG_BpInfo (U16 nCode, void *vp);
  extern _EXPO_ AG_BP *AG_BreakFunc (U16 nCode, U16 n1, GADR *pA, AG_BP *pB);
#else
  extern U32     (*AG_BpInfo) (U16 nCode, void *vp);
  extern AG_BP  *(*AG_BreakFunc) (U16 nCode, U16 n1, GADR *pA, AG_BP *pB);
#endif


/*
 * Go/Step function
 */

#define AG_STOPRUN    0x01            // force target to stop Go/Step.
#define AG_NSTEP      0x02            // exteute 'n' steps
#define AG_GOTILADR   0x03            // go til address
#define AG_GOFORBRK   0x04            // go forever or some Bp fires


#ifdef _IN_TARG_         // define if used in Mon166,Emu...
  extern _EXPO_ U32   AG_GoStep (U16 nCode, U32 nSteps, GADR *pA);
#else
  extern        U32 (*AG_GoStep) (U16 nCode, U32 nSteps, GADR *pA);
#endif



/*
 * Serial-Window I/O (thread-safe)
 */


struct SerAS  {
  DWORD         n1;
  DWORD         n2;
  DWORD         n3;
  DWORD         n4;
  GVAL           v;
};

#define AG_SERBOUT  0x01       // write nMany bytes to Serial #1 Window
#define AG_SERWOUT  0x02       // write nMany words to Serial #1 Window
#define AG_SERXIN   0x03       // Key was pressed in Serial Window #1 or #2

#ifdef _IN_TARG_               // define if used in Mon166,Emu...
  extern _EXPO_ U32  AG_Serial (U16 nCode, U32 nSerNo, U32 nMany, void *vp);
                               // nCode := AG_SER?OUT / AG_SERXIN
                               // nSerNo:  0:=Serial #1, 1:=Serial #2
                               // nMany:   number of items
                               // vp:      pointer to items (UC8 or U16)
#else
  extern       U32 (*AG_Serial) (U16 nCode, U32 nSerNo, U32 nMany, void *vp);
#endif


/*
 * Register and Memory access functions
 */

#define AG_READ     0x01     // read operation
#define AG_WRITE    0x02     // write operation
#define AG_WROPC    0x03     // write opcodes
#define AG_RDOPC    0x04     // read opcodes
#define AG_RDMMU66  0x05     // 19.12.99, read Sle66 MMU content
#define AG_WRMMU66  0x06     // 19.12.99, write single Sle66 MMU-Descr.
#define AG_RCRC     0x07     // Sle66 read CRC
#define AG_RDSHIELD 0x08     // Sle66 read Shield
#define AG_RDACE    0x09     // Sle66 read ACE

//--- 8.7.2002: added for flash download, otherwise like 'AG_WRITE'
#define AG_F_WRITE  0x0A     // Write to Flash memory (download)
#define AG_F_VERIFY 0x0B     // Verify FLash Memory
#define AG_F_ERASE  0x0C     // Erase Flash
#define AG_F_RUN    0x0D     // Start flashed App.




#ifdef _IN_TARG_             // define if used in Mon166,Emu...
  extern _EXPO_ U32  AG_MemAcc (U16 nCode, UC8 *pB, GADR *pA, UL32 nMany);
  extern _EXPO_ U32  AG_RegAcc (U16 nCode, U32 nReg, GVAL *pV);
  extern _EXPO_ U32  AG_AllReg (U16 nCode, void *pR);
#else
  extern U32 (*AG_MemAcc) (U16 nCode, UC8 *pB, GADR *pA, UL32 nMany);
  extern U32 (*AG_RegAcc) (U16 nCode, U32 nReg, GVAL *pV);
  extern U32 (*AG_AllReg) (U16 nCode, void *pR);
#endif




/*
 * extract trace history
 */

#ifdef _IN_TARG_             // define if used in Mon166,Emu...
  extern _EXPO_ U32   AG_HistFunc (U32 nCode, I32 indx, I32 dir, void *vp);
#else
  extern        U32 (*AG_HistFunc) (U32 nCode, I32 indx, I32 dir, void *vp);
#endif



#pragma pack(1)

#define DIAD struct DlgD
struct DlgD  {               // every dialog has it's own structure
  UL32              iOpen;   // auto reopen dialog (pos := 'rc')
  HWND                 hw;   // Hwnd of Dialog
  BOOL (CALLBACK *wp) (HWND hw, UINT msg, WPARAM wp, LPARAM lp);
  RECT                 rc;   // Position rectangle
  void   (*Update) (void);   // Update dialog content
  void (*Kill) (DIAD *pM);   // Kill dialog
  void                *vp;   // reserved for C++ Dialogs (Dlg *this)
};

#define DYMENU  struct DyMenu
struct DyMenu  {            // Menu item data structure
  I32              nDelim;  // Menu template delimiter
  char            *szText;  // Menu item text
  void (*fp) (DYMENU *pM);  // create/bringDlgtoTop function
  UL32                nID;  // uv3 assigned ID_xxxx
  UL32             nDlgId;  // Dialog ID
  DIAD              *pDlg;  // link to dialog attributes
};
#pragma pack()

/*
 * nDelim:  1 := normal Menu entry
 *          2 := Popup-Entry (nested submenu)
 *         -2 := end of Popup-Group-List
 *         -1 := total end of Menu-List
 *  text:   the name for the menu/popup-menu entry
 *    fp:   Function to be activated on menu-selection
 */




#if 0  // ----- Examples for AGDI's CallBack functions -----

//--- Example for Force Update:
  pCbFunc (AG_CB_EXECCMD, "dir public");  // execute dir public command
//--- Note: almost any dScope command but 'Exit' can be given.
//---       The 'Exit' command should not be executed !

//--- Example for Force Update:
  pCbFunc (AG_CB_FORCEUPDATE, NULL);
//--- Note: function returns when update is completed.


//--- Example for Set Message String:
  pCbFunc (AG_CB_MSGSTRING, "Running...");  // up to 20 characters
//---       up to 20 characters may be displayed in the statusbar pane.


//--- Example for DisAssemble Opcodes:
  DAAS    parms;

  parms.Adr = 0x10000;                     // disassemble address
  parms.Opc[0] = 0xC0;
  parms.Opc[1] = 0xF0;                     // MovBZ RH7,R0
  parms.Opc[2] = 0;
  parms.Opc[3] = 0;

  pCbFunc (AG_CB_DISASM, (void *) &parms); // disassemble...
//--- on return:  parms.OpcLen := length of opcode in bytes
//---             parms.szB[]  := disassembled instruction in ascii
//--- Note:       parms.Adr is used for ascii-address and relative jmps only.
//---             parms.Result always 0.


//--- Example for Inline Assemble:
  DAAS    parms;

  parms.Adr = 0x10000;                     // inline assemble address
  strcpy (parms.szB, "MOV [R8+#0x1200],R1");  // instruction to assemble

  pCbFunc (AG_CB_INLASM, (void *) &parms);    // assemble...
//--- on return:
//---   parms.Result = 0 if successful
//---     parms.OpcLen := length of opcode in bytes
//---     parms.Opc[]  := 'parms.OpcLen' Opcode bytes
//---   parms.Result != 0 if inline assembly failed
//---     everything of parms but 'Result' is invalid.


//--- Example for GetDeviceInfo:
  DEV_X66   parms;

  memset (&parms, 0, sizeof (parms));
  pCbFunc (AG_CB_INLASM, (void *) &parms);    // get device info
//--- On return:
//---   values in DEV_X66 structure.
//--- Note:
//---   AppName[] may be used to store a project target specific ini file
//---     AppName[] should be renamed (example: strcat (AppName, "hitex")
//---     before read or write operations can take place.


//--- Example for Find Symbol by value:

  SYMDSC   parms;

  parms.nMask = AG_SYM_LOC | AG_SYMCON;       // functions  and constants
  parms.val   = 0x10000;                      // some value
  pCbFunc (AG_CB_SYMBYVAL, (void *) &parms);  // search for symbol
//--- On return:
//---   parms.Ok = 1, search was successful
//---     parms.szName[] = name of symbol ('main' for example)
//---     parms.type     = type of symbol (AG_TP_FUNC for example)
//---     parms.val      = value of symbol
//---   parms.Ok = 0, search failed.
//--- Note: publics are searched only


//--- Example for Find Symbol by name:
  SYMDSC   parms;

  strcpy (parms.szName, "main");
  pCbFunc (AG_CB_SYMBYNAME, (void *) &parms);  // search for symbol by name
//--- On return:
//---   parms.Ok = 1, search was successful
//---     parms.szName[] = name of symbol ('main' for example)
//---     parms.type     = type of symbol (AG_TP_FUNC for example)
//---     parms.val      = value of symbol
//---   parms.Ok = 0, search failed.
//--- Note: publics are searched only


//--- Example for Progress-Bar handling:

static OIL  ProgressBar;

//--- initialize progress bar
  ProgressBar.pos   = 0;        // initial position
  ProgressBar.low   = 0;        //  low position percent (0%)
  ProgressBar.hig   = 100;      // high position percent (100%)
  ProgressBar.label = "Loading Monitor...";  // progress label text
  ProgressBar.Job   = PROGRESS_INIT;         // initialize
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);    // do initialization.

//--- Step 1: display progress bar position:
  while (processing)  {
//  ...
    ProgressBar.Job = PROGRESS_SETPOS;
    ProgressBar.pos++;
    pCbFunc (AG_CB_PROGRESS, &ProgressBar);  // set bar to position
//  ...
  }

//--- Step 2: remove progress bar:
  ProgressBar.Job = PROGRESS_KILL;
  pCbFunc (AG_CB_PROGRESS, &ProgressBar);    // kill Progress Bar.


#endif      // End of CallBack Samples.

#ifdef __cplusplus
  }
#endif

#endif  // __AGDI__INCED___
