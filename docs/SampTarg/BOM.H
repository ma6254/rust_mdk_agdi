#ifndef _BOM_H_INCLUDED_
  #define _BOM_H_INCLUDED_ 


#include <sys\types.h>        // wg. 'struct _stat'
#include <sys\stat.h>
#include <Windows.h>          // wg. HANDLE
#include <stdio.h>

#include "ComTyp.h"



#if 0   // these are in ComTyp.h
 #define iMCS51   0           // ToolSet Definitions
 #define iMCS251  1
 #define iMCS166  2
 #define iMCSARM  3           // Gnu-ARM tools
#endif

/*
 * Common Sim/Target Stuff
 */

#ifdef __cplusplus
  extern "C"  {
#endif


#define EXPRT __declspec(dllexport)


extern void   StopDDE (void);
extern void   SendDDE (BYTE *pS, BYTE CmdTyp, int nLen);
extern void   InitDDE (HANDLE hInst);




/*
 * Find in Files structure
 */

struct FiF  {
  BYTE        bFind;          // Find active flag.
  BYTE        bStop;          // stop thread flag.
  HWND       m_hWnd;          // MainFrame's hwnd.
  HANDLE        hTh;          // thread handle.
//HANDLE     hEvent;

  BYTE       SubDir;
  BYTE       RegExp;
  BYTE        mCase;
  BYTE       mhWord;

  char       What [300];
  char       Type [300];
  char       Fold [MAX_PATH + 10];
};

extern void FindinFiles (struct FiF *pF);




//--- Types

#define  ACONST    0x01
#define  AVOLAT    0x02

#define  bt_void      0
#define  bt_bit       1
#define  bt_char      2
#define  bt_uchar     3
#define  bt_int       4
#define  bt_uint      5
#define  bt_short     6
#define  bt_ushort    7
#define  bt_long      8
#define  bt_ulong     9
#define  bt_float    10
#define  bt_double   11
#define  bt_ptr      12
#define  bt_union    13
#define  bt_struct   14
#define  bt_func     15
#define  bt_string   16
#define  bt_enum     17
#define  bt_field    18
#define  bt_int64    19
#define  bt_uint64   20



/*
 * Storage class specifiers
 */

#define  sc_strsym    0
#define  sc_static    1
#define  sc_auto      2
#define  sc_regi      3
#define  sc_global    4
#define  sc_extern    5
#define  sc_type      6
#define  sc_const     7
#define  sc_member    8
#define  sc_label     9
#define  sc_ulabel   10
#define  sc_parm     11
#define  sc_sfr      12
#define  sc_abit     13
#define  sc_tag      14
#define  sc_bind     15


/*
 * Expression - Node types
 */

#define  x_const     1    /* some constant */
#define  e_strcon    2    /* string object */
#define  e_labcon    3    /* static object */
#define  e_autcon    4    /* local object  */
#define  e_namcon    5    /* dScope function reference */
#define  e_simvar    6    /* dScope predefined name */
#define  e_regid     7    /* some Register name */
#define  e_vtreg     8    /* some vtreg */
#define  e_usrcon    9    /* namcon of user program */
#define  e_vcon      10   /* namcon of a vSym */

#define  e_ref       11   /* some reference */
#define  e_reff      12   /* bit-field-reference */
#define  e_cast      13   /* some cast */
#define  e_postinc   14   /* ++ (postfix !) */
#define  e_postdec   15   /* -- (postfix !) */
#define  e_addr      16   /* & (addr_of) */
#define  e_cont      17   /* * (content_of) */
#define  e_plus      18   /* unary + */
#define  e_minu      19   /* unary - */
#define  e_not       20   /* ! (not) */
#define  e_compl     21   /* ~ (complement) */
#define  e_asn       22   /* = (assign to) */
#define  e_asnor     23   /* |= */
#define  e_asnxor    24   /* ^= */
#define  e_asnand    25   /* &= */
#define  e_asnlsh    26   /* <<= */
#define  e_asnrsh    27   /* >>= */
#define  e_asnadd    28   /* += */
#define  e_asnsub    29   /* -= */
#define  e_asnmul    30   /* *= */
#define  e_asndiv    31   /* /= */
#define  e_asnmod    32   /* %= */
#define  e_hook      33   /* e0 ? e1 : e2 */
#define  e_land      34   /* && */
#define  e_lor       35   /* || */
#define  e_and       36   /* & (bit-and) */
#define  e_or        37   /* | (bit-or) */
#define  e_xor       38   /* ^ (bit-xor) */
#define  e_equ       39   /* == */
#define  e_nequ      40   /* != */
#define  e_lequ      41   /* <= */
#define  e_gequ      42   /* >= */
#define  e_gt        43   /* >  */
#define  e_lt        44   /* <  */
#define  e_lsh       45   /* << */
#define  e_rsh       46   /* >> */
#define  e_add       47   /* +  */
#define  e_sub       48   /* -  */
#define  e_mod       49   /* %  */
#define  e_div       50   /* /  */
#define  e_mul       51   /* *  */
#define  e_preinc    52   /* ++ (prefix !) */
#define  e_predec    53   /* -- (prefix !) */
#define  e_indx      54   /* [] */

#define  e_comma     55   /* expr, expr */
#define  e_param     56   /* actual parameter */
#define  e_void      57   /* void-node */
#define  e_fcall     58   /* direct function call */
#define  e_size      59   /* sizeof (expr) */
#define  e_move      60   /* some memcpy */
#define  e_struid    61   /* 'struct -> id' or 'struct.id' */
#define  e_lea       62   /* load effective address */
#define  e_exec      63   /* exec ("command") */



//--- Symbol-Handling

union amap {
  DWORD     a32;
  WORD16    w16;
  BYTE    ub[4];
};
#define AMAP   union amap



#pragma pack(1)

struct ytp  {                 /* decl.-specifiers Eimer */
  BYTE            base;       /* Basic-Type */
  BYTE        sign : 3;       /* 1=Signed, 2=Unsigned */
  BYTE        regi : 1;       /* register */
  BYTE        cvol : 4;       /* 1=const, 2=volatile */
  BYTE         err : 1;
  BYTE    tblocked : 1;       /* block other types, typedef type given */
  BYTE          Xclass;
};


/*
 * mode-0: default mode
 * mode-1: Typedef mode
 * mode-2: Structure member mode
 * mode-3: Alien Mode
 * mode-4: ANSI-Parameter mode
 */

struct clxs  {
  BYTE       mode;    // declarator Mode
  BYTE     sclass;    // Storage class
  BYTE     dclspc;    // dcl-specifiers given flag
  BYTE     cgiven;    // class given explictly
};


/*
 * mSpaces (F66, 166/167)
 */

#define  xNEAR        1
#define  xFAR         2
#define  xHUGE        3
#define  xXHUGE       4

#define  mDATA        1         // fox _near
#define  mIDATA       2         // fox _idata
#define  mPDATA       3         // fox _shuge
#define  mXDATA       4         // fox _sdata
#define  mBDATA       5         // fox _bdata
#define  mECODE       6         // fox code-space


/*
 * nfh:   0 := ndata-ptr (16-Bit, page2:pof14)
 *        1 := far-ptr   (32-Bit, seg16:sof16)
 *        2 := huge-ptr  (32-Bit, linear)
 *        3 := xhuge-ptr (32-Bit, linear)
 */

struct typ  {                   /* Structure for a Type */
  BYTE              type;       /* bt_??? */
  DWORD             size;       /* Size of Item described */

  DWORD       v_flag : 1;       /* 1 on Array | Struct | Func */
  DWORD          vpf : 1;       /* voidpointer */
  DWORD         duse : 2;       /* 1=const, 2=volatile, 3=both */
  DWORD          nfh : 4;       /* 1:=near, 2:=far, 3:=huge, 7=short */
  DWORD          tps : 1;       // ptr-to-char for 'e_strcon'
  DWORD        iduse : 2;
//DWORD        pduse : 2;
  DWORD        parmb : 2;       /* 1=f(void), 2=f(), 3=f(p,...)' */
//DWORD           kr : 1;       /* Function: K&R-style */
  DWORD        tType : 1;       /* a Typedef-Type ! */
  DWORD        zTemp : 1;
  DWORD       SpcSpc : 1;       /* explicite spaced pointer */
  DWORD       eSpace : 1;       /* explicite mSpace given to spaced-ptr */

  BYTE             space;       /* mspace on pointers */
  BYTE            tspace;       /* typedef'd Space ! */
  BYTE             parms;       /* Function: Nr. of Parameters */

  struct typ        *btp;       /* type-Link */
  struct sym       *head;       /* subtable struct/union/func */
  char            *sname;       /* ptr to type-Name */
};
#define TYP   struct typ



#if 0
/*
 * OMF66 Type descriptors
 */

struct t2_x21  {                /* 0x21: Pointer descriptor */
  BYTE        nTyp;             /* 0x21 */
  TYP          *tp;

  BYTE        Size;             /* pointer size: 16 or 32  */
  BYTE      Attrib;
  WORD16  reserved;             /* unused, set to zero     */
  WORD16        TI;             /* Type-Index of refererred type */
};

struct t2_x22  {                /* 0x22: Array descriptor */
  BYTE        nTyp;             /* 0x22 */
  TYP          *tp;

  BYTE       nDims;             /* number of dimensions */
  BYTE      Attrib;             /* Bit-0: huge-array, Bit-1: xhuge-array */
  WORD16        TI;             /* Index of Array-Type */
  DWORD  DimSz [1];             /* actually 'nDims' values */
};

struct t2_x29  {                /* 0x29: Fox-Struct/union descriptor */
  BYTE        nTyp;             /* 0x29 */
  TYP          *tp;

  BYTE      Attrib;             /* 1:=struct, 2:=union */
  DWORD       Size;             /* struct/union size */
  WORD16        TI;             /* Type-Index of component list */
  BYTE        nLen;             /* NameLen */
  BYTE     name[1];             /* actually 'nLen' characters */
};

struct t2_x23  {                /* 0x23: Function descriptor */
  BYTE        nTyp;             /* 0x23 */
  TYP          *tp;

  BYTE      Attrib;             /* 1:=near Function, 2:=far Function */
  WORD16       rTI;             /* Return-Type Index */
  WORD16       pTI;             /* Parameter-List Index (Component List) */
};

struct t2_x25  {                /* 0x25: Bitfield descriptor */
  BYTE        nTyp;             /* 0x25 */
  TYP          *tp;

  WORD16        TI;             /* type of underlying object */
  BYTE      Offset;             /* Field-Offset in bits */
  BYTE       Width;             /* Field-Width in bits  */
};

struct t2_x26  {                /* 0x26: Typedef descriptor */
  BYTE        nTyp;
  TYP          *tp;

  WORD16        TI;             /* Type index */
  BYTE       Attr8;             /* unused */
  DWORD      Res32;             /* unused */
  BYTE        nLen;             /* NameLen */
  BYTE     name[1];             /* actually 'nLen' characters */
};

struct t2_x27  {                /* 0x27: Component */
  BYTE        nTyp;
  TYP          *tp;

  WORD16        TI;             /* component's type */
  DWORD     Offset;             /* component's offset */
  BYTE        Rep8;             /* unused */
  BYTE        Pos8;             /* unused */
  BYTE        nLen;             /* NameLen */
  BYTE     name[1];             /* actually 'nLen' characters */
};

struct t2_x28  {                /* 0x28: Component-List */
  BYTE        nTyp;
  TYP          *tp;

  DWORD     nComps;             /* Nr. of Components (word->dword by loader) */
  WORD16   nArr[1];             /* 'nComps' entries */
};

union flv {
  DWORD          sizz;      /* Typ muss gleich wie typ.size sein ! */
  struct fld  {
    BYTE          fwd;      /* Field-Width */
    BYTE          fof;      /* Field-Offset in Basic-Skalar */
    BYTE          bas;      /* 0 = char, 1 = unsigned */
    BYTE         sign;      /* 'signed' field */
  } fld;                    /* Field-Code fuer tp->size */
};


typedef union  {
  BYTE            *pB;
  struct t2_x21  *p21;              // 0x21: Pointer
  struct t2_x22  *p22;              // 0x22: Array
  struct t2_x29  *p29;              // 0x29: Fox-struct union
  struct t2_x23  *p23;              // 0x23: function
  struct t2_x25  *p25;              // 0x25: Field
  struct t2_x26  *p26;              // 0x26: TypeTag
  struct t2_x27  *p27;              // 0x27: Component
  struct t2_x28  *p28;              // 0x27: Component-List
} tpU;
#endif




struct zVal  {
  BYTE       Rep;              // 1:=Bits, 2:=Bytes
  UINT64     v64;
};
#define ZVAL  struct zVal

union v  {                     // expression-value union
  unsigned long       ul;
  signed char          c;
  unsigned char       uc;
  signed short       i16;
  unsigned short     u16;
  unsigned short      ui;
  signed long          l;
  unsigned long  ul2 [2];
  unsigned char  uc8 [8];     // uc[7] holds dpp-info of near-ptr
  int                  i;
  INT64              i64;
  UINT64             u64;
  float                f;
  double               d;
  struct vtr        *pVR;       // VtReg-descriptor
  char                *s;
  BYTE               *pB;
  WORD16             *pW;
  DWORD             *pDW;
  struct zVal        v64;
  void           *vp [2];     // used for dScope-functions
};


struct lPa  {
  DWORD         n1;
  DWORD         n2;
  DWORD         n3;
  DWORD         n4;
  union v        v;
};



/*
 * VTREG structure
 */

struct vtr  {
  char           *name;          // name of VtReg
  TYP              *tp;          // Type of VtReg
  WORD16         vtrNo;          // laufende nummer (0...MAXVTR - 1)
  WORD16    OscFrq : 1;          // special for 'XTAL'
  WORD16     Clock : 1;          // special for 'CLOCK'
  union v            v;          // VtReg's value
  WORD16           Atr;          // 0 | ATR_RDBRK | ATR_WRBRK
};

#define VTR      struct vtr
#define MAXVTR   256            // max. of 256 VTR-Slots.



struct enode  {                 // Expression-Node
  BYTE              etyp;       // Node-Type
  BYTE              rtyp;       // Rslt-type on operators

  DWORD         cvol : 2;       // const/volatile */
  DWORD          cfl : 1;       // const-expr */
  DWORD         mark : 1;       // expl-cast or struid mark
//DWORD        bable : 1;       // Expr. may have Type 'bit'
  DWORD      TopNeed : 1;       // don't remove Top-cast, if present
  DWORD        tprom : 1;       // Param: promote type (no parm-type given)
  DWORD          one : 1;       // if cfl=1: constant value-1
  DWORD         zero : 1;       // if cfl=1: constant value-0
  DWORD          max : 1;       // if cfl=1: constant has max-value
  DWORD        ssref : 1;       // e_ref for struct/union from nameref()
  DWORD      StruRet : 1;
  DWORD       iStore : 1;       // this e_ref is a store
  DWORD          wid : 6;       // e_reff: field-width
  DWORD          ofs : 6;       // e_reff: field-offset
  DWORD      iNotVis : 1;       // expr is not in Scope
  DWORD       AccBrk : 1;       // Access-break top node
  DWORD       iNoAcc : 1;       // can't access target mem or regs

  BYTE            OpType;       // Op-Type on NOT and all RelOps
  WORD16           space;       // Referencing Memory Space
  WORD16          rspace;       // Resulting Memory Space
  BYTE            nCount;       // temp: used for GenParms
  union v              v;       // Node value
  struct typ         *tp;       // Operation-Type, normally Result-Type
  struct typ        *tp1;       // Result-Type on RelOps and Not

  struct enode        *L;       // left child
  struct enode        *R;       // right child
  WORD16            Line;       // Line-Number
  WORD16           index;       // Line-index
  WORD16            nCnt;       // eg. strlen on e_strcon
  WORD16            nPos;       // Reg-based Bit position
  WORD16             Dpp;       // per dppn:override
  BYTE            nRadix;       // output radix for expr (watch)
  void               *vp;       // dScope-f(): rwatch/wwatch() (WATCH *)
};

#define EXP  struct enode



#define WATCH   struct watch
#define ACCESS  struct access

typedef struct watch *pWATCH;

struct watch  {
  WATCH            *next;  // next watch
  WATCH            *prev;  // previous watch
  void      (*fp) (void);  // Watch-function
  DWORD              adr;  // memory Address
  BYTE          acc_type;  // Trigger-Access 1:=READ, 2:=WRITE, 3:=BOTH
  WORD16        acc_size;  // Access-Size: 1:=BYTE, 2:=WORD,  4:=DWORD
  BYTE           special;  // 1=WBREAK
  BYTE              mask;  // iMCS51: Bit-Mask
};

/* ACCESS-LIST STRUCTURE */

typedef struct access *pACCESS;
struct access  {
  DWORD           address;    /* Addresse im Memory */
  WORD16             mask;    /* Read-/Write Mask */
};




struct zxfile  {
  struct zxfile    *next;            /* next file descriptor */
  WORD16             nFI;            /* File-Index */
  WORD16           nMode;            /* 0:=Src, 1:=Lst */
  char             *name;            /* File name */
  char             *path;            /* File path */
  DWORD             time;            /* File time */

  struct lin        *pLn;            /* debug-lines head */
  int             nLines;            /* lines in 'lhd' */
  int               nMax;            /* lines limit for current 'pLn' */
  DWORD            uLine;            /* highest line number in table */

//HANDLE           hFile;            /* File Handle */
//HANDLE           hFmap;            /* View-map handle */
  BYTE               *pB;            /* mapped view of File */
  DWORD            fSize;            /* File size */
  struct _stat       sts;            /* File status */
  DWORD          *pArray;
//DWORD            cline;            /* current line */
//int              cindx;            /* current View-Index */
};

#define ZXFIL   struct zxfile


struct lin  {                        /* LINE-NUMBER structure */
  WORD16          line;              /* the linenumber */
  DWORD           offs;              /* linenumber-offset */
};
#define LIN  struct lin

#pragma pack(1)
struct sym  {                        /* SYMBOL structure */
  struct sym     *next;              /* link to next Symbol */
  struct sym     *alnk;
  char           *name;
  struct zbk     *pBlk;              /* bounding Scope-Block */
  WORD16          type;
  BYTE              sc;              // Stoage class
  union v            v;
  TYP              *tp;              /* type index */
  WORD16           Dpp;
  WORD16           Reg;              /* Register number */
  WORD16           Pos;              // BitPos or FuncAttr
};
#pragma pack()


#define SYM  struct sym

struct adr  {                        /* ADDRESS-FRAME link */
  struct adr     *next;              /* next frame */
  DWORD           ladr;              /* address frame low */
  DWORD           hadr;              /* address frame high */
};


struct vSym  {             // a user created value symbol
  struct vSym  *next;      // next vSym
  char         *name;      // vSym's name
  TYP            *tp;
  union v          v;      // the value (Scalar types only !)
};
#define vSYM  struct vSym



#define ZBK  struct zbk
#define BLK_THEADR   1
#define BLK_PROC     2
#define BLK_DO       3
#define BLK_TASK     4


struct zbk  {                      /* SCOPE-BLOCK structure */
  ZBK            *next;            /* next block */
  char           *name;            /* BlockName */
  ZBK            *anxt;            /* Address: next Block */
  ZBK          *parent;            /* the Parent Block */
  DWORD           offs;            /* BLOCK Start address */
  DWORD           bend;            /* BLOCK End address */
  WORD16           nTI;            /* Type Index */
  ZBK            *down;            /* nested block */
  ZXFIL         *pFile;
  struct sym      *shd;            /* debug-symbols head */
  BYTE          mark : 3;          /* block marker */
  BYTE          ccov : 1;          // nr. of executed instr. has changed
  DWORD          nInst;            // number of instructions in Func-Block
  DWORD          nExec;            // number of executed instructions
  DWORD         nCjmps;            // number of cond-jumps in block
  DWORD         nTjmps;            // number of taken jumps in block
};


struct iF66  {
  DWORD         ErrAdr;
  SYM           *Hpubs;        // Public symbols - Head
  SYM           *Tpubs;        // Public symbols - Tail
  SYM            *SfrH;        // Head of SFR symbols
  SYM            *SfrT;        // Tail of SFR symbols
  ZXFIL        *FilesH;        // Files - Head
  ZXFIL        *FilesT;        // Files - Tail
  DWORD         nTypes;        // Nr. of Type-Recs (ab 0x80)
  DWORD            tpN;        // current max Type-Slots
  char           **tpA;        // Type-Pointer Array
  int            yindx;
  ZBK            **pA3;        // a24-a31 Cluster
  ZBK           *pRoot;        // Head of Theader Blocks.

  vSYM         *vShead;        // Head of user-create value symbols
  vSYM         *vStail;        // Tail of user-create value symbols
  
  DWORD            low;        // Low Scope address
  DWORD            hig;        // High Scope address
};





struct scop66  {               // Scope-Descriptor
  DWORD            low;        // Low Scope address
  DWORD            hig;        // High Scope address
  ZXFIL         *pFile;
  LIN             *pLn;        // extracted from 'pFile'
  int           nLines;        // extracted from 'pFile'

  DWORD           targ;  
  WORD16          mode;        // HLL, ASM
  WORD16       set : 1;
  WORD16     isact : 1;        // HLL File is active

  ZBK              *pT;        // Translation-Unit
  ZBK              *pF;        // Function in Theader
  ZBK              *pB;        // Block in Function
  ZBK           *pTask;        // 166: Task Header
};

#define CUS    struct scop66

#define IF66  struct iF66

extern IF66          i66;      // Fox Symbol-Info
extern struct scop66 Sc6;      // Scope-Info


/*
 * Qualified references
 */

struct iqual  {
  int        comps;          // Nr. of Components
  char      *tname;          // \\TASKNAME
  int         tndx;          // Index of TaskName
  char    *name[3];          // \THEADR\FUNCTION\LOCSYM
  int       ndx[3];          // Component Line-Indices
  int         Bank;
  ZBK          *tk;          // Task specifier
  ZBK          *th;          // Parent-Theader
  ZBK          *fc;          // Parent-Function
  SYM          *sp;          // if a fully qualified Symbol
  DWORD     LineNo;          // Linenumber to search for
  DWORD    LineOfs;          // Linenumber-Offset
  int        ErrIx;          // Line-Index in case of error
  int         bufi;
  char   buf [512];
  BYTE        line;          // Qualifier specifies '\line'
};
#define IQUAL struct iqual   // Qualifier-Reference


struct squal  {
  int        index;
  int         Line;
  ZBK        *Task;
  ZBK      *Theadr;
  ZBK        *Func;
  SYM          *sp;
};
#define SQUAL  struct squal  // Qualifier-Reference


#pragma pack()


//--- Basic-types
extern TYP tp_void;    // = { bt_void,   { 0, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_bit;     // = { bt_bit,    { 0, 0, 0, 0 }, 0, 0, 1,  };
extern TYP tp_char;    // = { bt_char,   { 0, 0, 0, 0 }, 0, 0, 1,  };
extern TYP tp_uchar;   // = { bt_uchar,  { 0, 0, 0, 0 }, 0, 0, 1,  };
extern TYP tp_int;     // = { bt_int,    { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_uint;    // = { bt_uint,   { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_short;   // = { bt_short,  { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_ushort;  // = { bt_ushort, { 0, 0, 0, 0 }, 0, 0, 2,  };
extern TYP tp_long;    // = { bt_long,   { 0, 0, 0, 0 }, 0, 0, 4,  };
extern TYP tp_ulong;   // = { bt_ulong,  { 0, 0, 0, 0 }, 0, 0, 4,  };
extern TYP tp_float;   // = { bt_float,  { 0, 0, 0, 0 }, 0, 0, 4,  };
extern TYP tp_double;  // = { bt_double, { 0, 0, 0, 0 }, 0, 0, 8,  };
extern TYP tp_func;    // = { bt_func,   { 1, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_funcN;   // = { bt_func,   { 1, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_funcF;   // = { bt_func,   { 1, 0, 0, 0 }, 0, 0, 0,  };
extern TYP tp_funcS;   // = { bt_func,   { 1, 0, 0, 7 }, 0, 0, 0,  };

extern TYP tp_culong;  // = { bt_ulong,    0, 0, 1, 0, 0, 0, 4,  }; // const !


/*
 * Symbol-Types
 */

#define F66_VAR    0x0001         // non-bit Variables
#define F66_CON    0x0002         // named Constants
#define F66_BIT    0x0004         // Bit in Memory
#define F66_LOC    0x0008         // Location, some label
#define F66_REGV   0x0010         // Register-Variable
#define F66_AUTO   0x0020         // Stack-based variable
#define F66_REGB   0x0040         // Bit in Register
#define F66_SFRB   0x0080         // Bit of SFR
#define F66_TDEF   0x0100         // Typedef name
#define F66_SFR    0x0200         // Fox-SFR name
#define F66_MEMB   0x0400         // MOS/MOU/MOE */

#define F66_FUNC   0x0800         // dScope function name
#define F66_FPARM  0x1000         // dScope function parameter
#define F66_FLAB   0x2000         // dScope function local label
#define F66_FLOC   0x4000         // dScope function local symbol



/*
 * Fox-Compound-Types
 */

#define ZT_CLIST       0x20     /* OMF166: component list descriptor */
#define ZT_PTR         0x21     /* OMF166: pointer descriptor        */
#define ZT_ARRAY       0x22     /* OMF166: array descriptor          */
#define ZT_FUNC        0x23     /* OMF166: function descriptor       */
#define ZT_STRU        0x24     /* OMF166: struct/union descriptor   */
#define ZT_FIELD       0x25     /* OMF166: bitfield descriptor       */
#define ZT_TYPTAG      0x26     /* Fox-Typedef */
#define ZT_COMP        0x27     /* Fox-Component */
#define ZT_FCLIST      0x28     /* Fox-Component-List */
#define ZT_FSTRU       0x29     /* Fox-struct/union descriptor */


/*
 * Fox Scalar-Types
 */

#define TI_NIL         0x40     /* untyped */
#define TI_BIT         0x41     /* bit */
#define TI_CHAR        0x42     /* char */
#define TI_UCHAR       0x43     /* uchar */
#define TI_INT         0x44     /* int */
#define TI_UINT        0x45     /* uint */
#define TI_LONG        0x46     /* long */
#define TI_ULONG       0x47     /* ulong */
#define TI_FLOAT       0x48     /* float (32-bit)  */
#define TI_DOUBLE      0x49     /* double (64-bit) */
#define TI_VOID        0x4A     /* void */
#define TI_LABEL       0x4B     /* label (near) */
#define TI_BITWORD     0x4C     /* a166 bitword */
#define TI_NEAR        0x4D     /* a166 near */
#define TI_FAR         0x4E     /* a166 far */
#define TI_DATA3       0x4F     /* a166 data3 */
#define TI_DATA4       0x50     /* a166 data4 */
#define TI_DATA8       0x51     /* a166 data8 */
#define TI_DATA16      0x52     /* a166 data16 */
#define TI_INTNO       0x53     /* a166 intno */
#define TI_REGBANK     0x54     /* a166 regbank */
#define TI_IFAR        0x55     /* a166 interrupt far */
#define TI_DATA32      0x56     /* a166 data32 */
#define TI_DATA64      0x57     /* a166 data64 */
#define TI_FRACT16     0x58     /* fractional-16 type */
#define TI_FRACT32     0x59     /* fractional-32 type */
#define TI_SHORT       0x7F     /* dummy type for Jumps */


/*
 * Loader-Errors
 */

#define LOD_OK         0x0000


//--- DisAsm Stuff

/*
 * Language Window Definitions
 */

#define HLEVEL   0x00       /* View-Modes */
#define MIXED    0x01
#define ASSMBL   0x02

#define LINENUM  0x20       /* Information Number */
#define ASMLINE  0x21
#define LABLINE  0x22       /* Line with Label only */
#define HISTNUM  0x23
#define HISTLAB  0x24
#define HISTLIN  0x25
#define EMPTYLN  0x26       /* no more lines available */

#define ATTRBRKE 0x40       /* Attributes */
#define ATTRPC   0x80
#define ATTRHIST 0x20
#define ATTRLOG  0x10
#define ATTRHIG  0x08       /* HighLight Attribute */
#define ATTREXEC 0x01       /* hold in member 'cover' */
#define ATTRBRKD   0x100    /* disabled exec-break */
#define ATTRJTAKEN 0x200    // 



#define CRT  struct crt
struct crt  {           /* CRT struct */
  WORD16       inumb;   /* information number     */
  WORD16        cont;   /* content (attribute)    */
  WORD16        attr;   /* attribute CURPC, BRKPT */
  WORD16        bpnr;   /* breakpoint number      */
  ZXFIL       *pFile;
  int             fi;   /* File-Index of 'line'  */
  DWORD          off;   /* offset                 */
  DWORD         line;   /* line number bei HLL    */
  int           hidx;   /* History Index          */
  char         *pLab;   /* LabelName on HISTLAB/LABLINE */
  WORD16        scra;   /* Attribute on Screen    */
  WORD16        nLen;   /* Line-Length            */
  BYTE         cover;   /* Coverage Attribute     */
};
typedef struct crt   *pCRT;





#define ABREAK  0
#define CBREAK  1
#define WBREAK  2
#define RBREAK  3           /* Address-Range Break */

#pragma pack(1)
struct brk  {
  struct brk    *next;
  struct brk    *prev;

  DWORD      type : 4;      // ABREAK, CBREAK, WBREAK
  DWORD   enabled : 1;      // 1:=enabled, 0:=disabled
  DWORD    ReCalc : 1;      // recalc expr flag
  DWORD    BytObj : 1;      // WatchBrk: 0:=Bytes, 1:=Objects

  DWORD           Adr;      // Break-Address
  DWORD        mSpace;      // memory-space
  VTR             *pV;      // used on VTR-access breakpoints

  DWORD         tsize;      // WatchBrk: size of one object
  DWORD          many;      // WatchBrk: many objects or bytes
  WORD16          acc;      // WatchBrk: 1:=Read, 2:=Write, 3:=ReadWrite
  WORD16       BitPos;      // currently not used
  DWORD        number;      // BreakPoint-Number
  int          rcount;      // Break is taken when rcount = 1

  int          ocount;      // Original Count
  EXP             *ep;      // Cond-Expression
  char           *cmd;      // Exec-Command
  char          *Line;      // Breakpoint-Expression Line for Display
  char            *pF;      // module file name
  DWORD         nLine;      // line number
  BYTE         Opc[8];      // Opcode-Save Area for Monitors
};
#pragma pack()

#define BKS  struct brk



struct BpBlk  {
  BKS        Bp;             // Bp to clone from
  BKS      *nBp;             // the new Bp
  WORD16    Err;             // Error-Number
  BYTE     Item;             // 1 := expr, 2 := count, 3 := command */
  BYTE      Atr;             // 1 := Bp redefined
  BYTE    BpDlg;             // 1 := Bp create from Dialog
};



#if 0
extern void       LRwLF (pCRT X);             /* Language-Reverse-Line */
extern void       LFwLF (pCRT X);             /* Language-Forward-Line */
extern void      CrtAtt (pCRT X);             /* get LL-Attributes */
extern char       *LGet (pCRT X);
extern DWORD   AsmBelow (DWORD off);          /* Sync for ThumbTrack */
extern int               yindx;
extern CRT               cwLog;
extern DWORD            MARKPC;        /* used for LWin-HighLight */
extern BYTE   DynaScope (DWORD nAdr);
extern WORD16   DynaHll (DWORD nAdr);
extern ZXFIL *FileScope (DWORD nAdr);  /* map nAdr to 'ZXFIL *' */
extern BYTE    FileLine (ZXFIL *pF, DWORD nLine);
//--- End DisAsm Stuff
#endif


/*
 * Fox-Section types (taken from OMF267-Specification)
 */

#define   SF66_BIT        0x0000        /* Bit-Section */
#define   SF66_DATA       0x0001        /* Data-Section */
#define   SF66_CODE       0x0002        /* Code-Section */
#define   SF66_NCONST     0x0003        /* NConst-Section */
#define   SF66_HDATA      0x0005        /* Hdata-Section */
#define   SF66_HCONST     0x0007        /* HConst-Section */
#define   SF66_XDATA      0x0009        /* Xhuge-Data-Section */
#define   SF66_XCODE      0x000A        /* XCode-Section */
#define   SF66_XCONST     0x000B        /* XConst-Section */








#define ATR_EXEC   0x01         // 'executable ' Attribute
#define ATR_READ   0x02         // 'readable' Attribute
#define ATR_WRITE  0x04         // 'writable' Attribute
#define ATR_BREAK  0x08         // 'Exec-Break' Attribute
#define ATR_EXECD  0x10         // 'Executed' Attribute
#define ATR_WATCH  0x20         // Location has a Watch */
#define ATR_BPDIS  0x40         // 'disabled Exec-Break' Attribute
#define ATR_PAP    0x80         // Location has a Perf.-Analyzer point */
#define ATR_WRBRK  0x100        // Loc has a write-access break
#define ATR_RDBRK  0x200        // Loc has a read-access break
#define ATR_COMC   0x400        // iMCS51/251: Common code marker
#define ATR_VNM    0x800        // iMCS51: von Neumann mapped
#define ATR_BEP    0x1000       // iMCS51: Bank-Entry Point

#define ATR_EXTR   0x2000       // 166/167: within range of an EXTR sequence
#define ATR_JTAKEN 0x4000       // Jump-taken attribute


                                // for DASMFOX:
#define LOBOUND    0x00000000   // current low bound
#define HIBOUND    0xFFFFFFFF   // Fox: current high bound





struct irx  {
  char       *pS;           // the expression text
  WORD16  uError;           // Error-code
  int     uIndex;           // Error-Index
  int     nRadix;           // Expr.-Radix (-1:=auto, 10 or 16)  
  EXP        *e0;           // watch-expression
  EXP        *e1;           // store-expression
};
#define IRX  struct irx

/*
 * Real Scanner for Fox (SCANFOX1.C)
 */

#define  CINS  struct cins
#define  HNODES    1024   // 13.4.2001, 256
#define  IDHEAP   32768   // 2.8.2000, wg. G&D BullFuncs !!!  (old: 2048)

struct cins  {            // Command Input level structure
  char           achr;
  char         *pLine;    // the input line(s)
  int             ndx;    // read index
  int            lndx;    // index in current line
  int          iStart;    // start of following line index
  char         *fname;    // in case of FuncDef

  char         *iname;    // name of include file
  HANDLE        hFile;
  HANDLE        hFmap;
  DWORD         fSize;    // file length in bytes

  int          inHook;

  int          LineNo;
  int          nNodes;
  EXP          *nPool;
  WORD16         type;
  WORD16    PromptLen;    // length of Line-Prompt
  char         *idbuf;
  int           idndx;
  int            deep;    // FuncDef: '{' depth
//int       LastToken;

  DWORD       eof : 1;
  DWORD LineReady : 1;
  DWORD   InStruc : 1;    // struct/union member mode
  DWORD   LitMode : 1;    // get next token literally
  DWORD       eol : 1;    // end of line marker (delayed eol)
  DWORD      iDbm : 1;    // use ENV_DBM-Env for GetMem
  DWORD     iShow : 1;
  DWORD    iNoRes : 1;    // Watch-Asn expr.: no result output

  int           stlin;
  WORD16       uError;    // Error number
  int          uIndex;    // Error index
  BYTE          shoot;    // Line number
  BYTE        ExprErr;    // Expression-Error Flag.
  DWORD        uAddre;    // Error address (eg. load)
  IRX            *vpx;
  void           *yyp;    // Yacc's value Stack

  BYTE          __eoi;    // placeholder
};



#define YYMAXD   300            // from YACCPAR !
#define  IPMAX    30            // input stack 1...29

extern int          nStack;     // Input-Stack index
extern CINS *iStack[IPMAX];     // Input Stack
extern CINS          *cinp;     // current input
extern char  yytext [1024];

//extern YYSTYPE      yylval;

extern int          ExpLin;     // Line-Number
//extern int             ndx;     // line index
extern BYTE           tdef;     // typedef-search flag
extern int           stlin;

#if 0      // moved to 166.h
 extern WORD16       uError;     // Error number
 extern int          uIndex;     // Error index
 extern BYTE          shoot;     // Line number
 extern DWORD        uAddre;     // Error address (eg. load)
 extern char         *uName;     // some name, eg. function name
#endif


extern CINS  *CreateInput (char *pLine, WORD16 PromptLen, WORD16 type);
extern void     KillInput (void);
extern void   InitScanner (void);
extern void     ClearHeap (void);            // after command
extern void    SetLitMode (void);

extern EXP      *HeapNode (void);
extern TYP      *MakeType (BYTE type, DWORD nSz, TYP *btp);

extern SYM      *vFindSym (SYM *table, char *name);
//extern ZBK      *FindTMod (char *name);
extern ZBK      *FindFunc (ZBK *Theadr, char *name);
extern LIN    *FindLineNo (ZBK *Theadr, DWORD LineNo);
extern SYM       *FindSym (char *s);
extern vSYM      *SrcvSym (char *name);
extern vSYM     *MakevSym (char *name, WORD16 index, TYP *tp);

extern int          yylex (void);
extern int        yyparse (void);


/*
 * NodeFox.C:
 */

extern void HandleErr (EXP *ep);    // special node with etyp := 0
extern EXP     *MkOp2 (WORD16 Op, WORD16 index, EXP *L, EXP *R);
extern EXP     *MkOp1 (WORD16 Op, WORD16 index, EXP *L);
extern EXP      *Hook (EXP *X, EXP *L, EXP *R);
extern EXP    *castex (EXP *L, TYP *tp, WORD16 Expl, WORD16 index);
extern EXP      *call (EXP *X, EXP *args);
extern EXP  *parmlist (EXP *e1, EXP *e2);
extern EXP   *SizeObj (WORD16 index, EXP *L);
extern EXP   *SizeTyp (WORD16 index, TYP *tp);
extern EXP   *undefid (EXP *L);
extern EXP   *StruRef (EXP *L, char *Member, WORD16 index, WORD16 ptr);
extern EXP    *RefArr (EXP *L, EXP *R);
extern EXP     *ExecC (WORD16 index, EXP *parms);
extern EXP   *DoQuali (SQUAL *pq);
extern EXP  *HandleDp (int dpn, EXP *e2);

extern TYP    *DoType (void);
extern void     makep (DWORD nAtr, DWORD cv);
extern void    ArrTyp (EXP *ep);
extern void     linki (TYP *t1, TYP *t2);
extern void    dclspc (DWORD mode);

extern void     popdp (void);
extern void    p_type (WORD16 base, WORD16 index);
extern void    p_sign (WORD16 sign, WORD16 index);
extern void    attrib (WORD16 attr, WORD16 index);
extern void        md (BYTE mode);
extern void        cl (BYTE sclass);
extern void    dclspc (DWORD mode);
extern TYP    *DoType (void);


extern void      CmdErr (void);
extern void   LineFlush (void);

extern void   DoExitCmd (void);
extern void   DoExprCmd (EXP *ep, BYTE Run);
extern void   DoEvalCmd (EXP *ep);
extern void   DoLoadCmd (char *name, WORD16 index, int lOpt);  // 1:=nocode
extern void   DoLogShow (void);
extern void    DoLogOff (void);
extern void    LogWrite (int nSel, char *pLine, int nMany);
extern void     DoReset (int nCmd);
extern void     MkWatch (EXP *ep, int index);
extern void   CreaWatch (IRX *pX);
extern WORD16  AsnWatch (char *exp, int nLen); // Assignment via WatchVw
extern WORD16 WatchCalc (EXP *ep, char *pB);
extern ZBK     *WaScope (DWORD nAdr);          // DasmFox.c
extern LIN    *FindLine (DWORD nAdr);          // search for Line

extern void      VaShow (char *pVar, WORD16 nIndex);
extern void       VaSet (char *pVar, WORD16 index1, char *pVal, WORD16 index2);
extern void     VaReset (char *pVar, WORD16 index);
extern void   InitLpath (char *name);

extern void    DoLogSet (int nMode, char *name, WORD16 index);
extern void   DoInclude (char *name, WORD16 index);
extern void       GoCmd (EXP *e1, EXP *e2);
extern void      DoStep (int type, EXP *ep);
extern EXP     *CalcAdr (EXP *ep);

extern void      SetBrk (EXP *ep, int ndx, int acc);
extern void       DlgBp (struct BpBlk *pB);     // create Dialog Bp
extern void      LstBrk (void);
extern void      BpComm (int nCode, EXP *ep);

extern void      PaDisp (void);        // list PA items
extern void      PaKill (EXP *ep);     // kill all or given PA items
extern void       PaDef (int ndx, EXP *e1, EXP *e2);


extern void     dirsym (void);                  // dir syms of current block
extern void     dirQua (SQUAL *sq, BYTE lines); // syms/lines of given module
extern void     dirlin (void);                  // dir lines of current module
extern void     dirpbs (void);                  // dir publics
extern void     dirioc (void);                  // dir VtRegs
extern void     dirmod (void);                  // dir modules
extern void    dirdsym (void);                  // dir 'define type id' symbols
extern void     dirfnc (int nSel);              // dir functions
extern void   dirScope (SQUAL *sq);             // display scope

extern WORD16   RunCmd (char *cmd, int nLen);
extern void    BootExp (void);

extern void     ClrExp (void);
extern EXP  *UpNodeTop (EXP *ep);            // NodeFox.c
extern void    DumpExp (EXP *ep);            // PriFox.c






//--- Show/Set/Reset env-vars:

#define  VST   struct vset
struct vset  {
  char    name [14];                               // name of env-var
  void    (*sfc) (VST *pV, char *pS, WORD16 ndx);  // 'set' function
  void    (*xfc) (VST *pV);                        // 'show' function
  void    (*rfc) (VST *pV);                        // 'reset' function
  int     which;
};

struct szPAT  {                // search path list
  struct szPAT  *next;
  char          *text;
};


/*
 * CALL-Stack
 */

struct calstk  {
  DWORD      caller;      // callers opcode address
  DWORD      callee;      // called address
  DWORD          sp;      // stack pointer value before call
  DWORD       nextA;      // instruction after Call
  BYTE        icall;      // indirect call mark
};
#define CSTK      struct calstk
#define MAXCSTK   256




/*
 * Performance Analyzer
 */

struct pa  {
  struct pa     *next;     // next PA range
  struct pa     *prev;     // prev PA range
  DWORD         entry;     // PA-Range entry address
  DWORD          exit;     // PA-Range exit address
  DWORD        icount;     // Invocation count
  UINT64          cyc;     // cycles of last measure
  UINT64       mincyc;     // minimum cycles (for average)
  UINT64       maxcyc;     // maximum cycles (for average)
  UINT64       totcyc;     // total cycles of PA-Range
  WORD16         Slot;     // Slot-Number
  char         *ptext;     // PA-Expression
  int            nLen;     // Length of PA-Expr. Text
  int          Extent;     // Extent from GetTextExtent ...
//float        spcent;     // Percent on Screen
  float        cpcent;     // current Percent

  BYTE           type;     // 1:=FuncPA, 0:=RangePA
  BYTE     active : 1;     // active bit
};

struct past  {             // used for PA-Runtime-Stack
  UINT64      ecyc;
  DWORD     paexit;        // for PA entry,  return address, otherwise 0
  struct pa    *pa;
};

#define  PA      struct pa
#define  PAST    struct past
#define  PADEP   256
#define  RPAT    0         // Range-PA
#define  FPAT    1         // Function-PA Range

extern void    PaDef (int ndx, EXP *e1, EXP *e2);
extern void   PaKill (EXP *ep);
extern void   PaDisp (void);
extern void  PaReset (void);

extern DWORD  PaFunc (int nCode, int nAdr, void *p);



//extern void SimInit (WORD16 nFamily);
//extern void SimStep (DWORD nCount);



/*
 * Dynamic Menu construction and maintanance
 */

#pragma pack(1)

#define DLGD struct DlgDat
struct DlgDat   {            // every dialog has it's own structure
  DWORD             iOpen;   // auto reopen dialog (pos := 'rc')
  HWND                 hw;   // Hwnd of Dialog
  BOOL (CALLBACK *wp) (HWND hw, UINT msg, WPARAM wp, LPARAM lp);
  RECT                 rc;   // Position rectangle
  void   (*Update) (void);   // Update dialog content
  void (*Kill) (DLGD *pM);   // Kill dialog
  void                *vp;   // reserved for C++ Dialogs (Dlg *this)
};

#define DYM  struct DynaM
struct DynaM  {             // Menu item data structure
  int              nDelim;  // Menu template delimiter
  char            *szText;  // Menu item text
  void    (*fp) (DYM *pM);  // create/bringDlgtoTop function
  DWORD               nID;  // uv3 assigned ID_xxxx
  DWORD            nDlgId;  // Dialog ID
  DLGD              *pDlg;  // link to dialog attributes
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


/*
 * Dll-Query structure
 */

struct qdll  {
  HWND       parent;        // parent window handle
  char     *pathUv3;        // path of Target-DLL
  HMODULE     Linst;        // DLL Instance handle

  int       ValSize;        // size of 'value[]' Buffer
  char         *key;        // Register key for driver DLL's
  char       *value;        // some command line, for example
                            // more info could be here ...
  char     *prjname;        // project path+name
};
#define QDLL  struct qdll




/* Memory Management */
struct  MM {
  BYTE    *mem;            // Pointer to Memory Image
  WORD16  *atr;            // Pointer to Memory Attributes
};



union op  {                /* Opcode of current instruction */
  BYTE       b[8];
  WORD16     w[4];
  DWORD        ul;
  DWORD    ula[2];
  UINT64      u64;
};

//static union op      op;     // Opcodes

union rstat  {             /* Execution Status of dScope */
  BYTE          status;

  struct  {
    BYTE     BreakOp : 1;    /* Break-Opcode reached */
    BYTE     CntZero : 1;    /* Step-count reached zero. */
    BYTE     SomeErr : 1;    /* some error occurred */
    BYTE     StopBut : 1;    /* Stop-Button has been pressed. */ 
    BYTE     ExecBrk : 1;    /* stopped by an execution breakpoint */
    BYTE      AccBrk : 1;    /* stopped by an access breakpoint */
    BYTE      CndBrk : 1;    /* stopped by a conditional breakpoint */
    BYTE        Stop : 1;    /* stopped via '_break_' variable */
  } inf;
};

#define  TIMS  256           // 100
#define  TMW   struct tmw

struct tmw  {
  INT64             tmv;     /* Cycle value */
  void     (*tmf)(void);     /* function to be activated */
  int              tndx;     /* Time-Watch Number */
  DWORD            uSec;     // micro-seconds based timewatch
};

extern TMW   tixx [TIMS];    /* Array of Time-Watches */
extern int   numtim;         /* # of time-entries */
extern int   tndx;
extern int   tact;           /* TimeWatch-Activators id */


//--- FOX specific:

union itrp  {              /* Interrupt Status of Instructions */
  struct  {
    WORD16   hwtrap;         /* Hardware Traps */
    WORD16     flag;         /* Interrupt Flag */
  } w;
  DWORD      stat;
};


struct FoxReg  {
  union rfox   {
    DWORD   dregs [16];      /* D0  ... D30 */
    WORD16  wregs [16];      /* R0  ... R15 */
    BYTE    bregs [16];      /* RL0 ... RH7 */
  } r;
  DWORD        Ndpp[4];      /* Normierte DPP Register */ 
  DWORD            nPC;      /* full address ! */
  DWORD           cPsw;      /* DPSW */
  DWORD            cCP;      /* DCP */
  INT64        nCycles;      /* cycle counter */
  DWORD        cGPRCON;      /* DGPRCON */
};

struct FoxCont  {
  union itrp      itrp;    // Interrupt status
  BYTE           sleep;    // CPU sleep mode
  BYTE           trace;    // record trace information
  BYTE       intchange; 
  BYTE          intdis;
  DWORD           cPsw;    // current PSW
  DWORD           oPsw;    // old PSW
  struct FoxReg      r;    // Fox-Registers
};


//--- 166/167 specific's: -------------------------------

#define OFO(s,m)   (DWORD)&(((s *) 0)->m)     /* Offsetof Macro */

#pragma pack(1)
struct Reg166  {
  union  {
    WORD16  wregs [16];      // R0  ... R15
    BYTE    bregs [16];      // RL0 ... RH7
  } r;
  DWORD        Ndpp[4];      // full linear base address values
  DWORD            nPC;      // full address !
  WORD16          cPsw;      // current Psw
  WORD16           cSP;
  WORD16          cMDL;      // current MDL
  WORD16          cMDH;      // current MDH
  DWORD            cCP;      // CP
  INT64        nCycles;      // cycle counter

  INT64           macc;      // 40-Bit value
  WORD16           mah;      // MAH MAC-Unit Accumulator High
  WORD16           mal;      // MAL MAC-Unit Accumulator Low
  WORD16           mas;      // MAS limited MAH/signed
  WORD16           msw;      // MSW MAC-Unit Status Word
  WORD16           mcw;      // MCW MAC-Unit Control Word
  WORD16           mrw;      // MRW MAC-Unit Repeat Word

  WORD16          idx0;
  WORD16          idx1;
  WORD16           qx0;
  WORD16           qx1;
  WORD16           qr0;
  WORD16           qr1;
};

#pragma pack()
                             // use default packing !!!
struct Regx51  {             // iMCS51-Registers
//union  {
//  BYTE    bregs [16];      // R0 ... R7
//} r;
  BYTE      bregs [16];
  DWORD            nPC;      // full address !
  BYTE              sp;      // SP
  BYTE             psw;      // PSW-sfr
  BYTE               b;      // B-sfr
  BYTE             acc;      // ACC-sfr

  BYTE          dpl[8];      // DPL-sfr (a0...a7)
  BYTE          dph[8];      // DPH-sfr (a8...a15)
  BYTE          dpx[8];      // DPX-sfr (a16...a23) /Dallas C390/
  BYTE           dpSel;      // DPSEL-sfr
  BYTE             esp;      // ESP-sfr /Dallas C390/

//BYTE        ports[8];
  INT64        nCycles;      // cycle counter
};


#pragma pack(1)
struct Reg51MX  {            // 80C51MX-Registers (Philips)
  BYTE       bregs [8];      // R0...R7
  BYTE              sp;      // SP
  BYTE             spe;      // MX SP-extendend (high byte)
  BYTE             psw;      // PSW-sfr
  BYTE               b;      // B-sfr
  BYTE             acc;      // ACC-sfr

  BYTE          dpl[2];      // DPL0 (DPL sfr)
  BYTE          dph[2];      // DPH0 (DPH sfr)
  BYTE           mxcon;      // MXCON sfr
  BYTE           auxr1;      // AUXR1 sfr
  BYTE             epl;      // EPL sfr
  BYTE             epm;      // EPM sfr
  BYTE             eph;      // EPH sfr
  BYTE        ports[4];

  DWORD            nPC;      // full address !
  INT64        nCycles;      // cycle counter
  DWORD         ZF : 1;      // Zero-Flag: 80C51MX+Ext
};

#if 0
struct Rg5016 {              // Mifare Pro X P8RF516 Registers
  DWORD            nPC;
  INT64        nCycles;      // cycle counter
  BYTE             psw;
  BYTE              sp;
  BYTE               b;
  BYTE             acc;
  BYTE             dpl;
  BYTE             dph;
  BYTE       bregs [8];
  BYTE           dpsel;
  BYTE           adrxh;      // page for MOVX @Ri

  BYTE          rename;      // Accu rename sfr
  WORD16           ssp;      // SP - system mode
  WORD16           usp;      // SP - user mode
  BYTE            pswh;      // PSW high (mode register)
  BYTE          repeat;      // Repeat instr. n times
  BYTE          xbase0;      // BaseAddr of Seg-0 Xram for all modes
  BYTE          xbase1;      // BaseAddr of Seg-1 Xram for all modes
  BYTE          xsize0;      // Size of Seg-0 Xram, user mode
  BYTE          xsize1;      // Size of Seg-1 Xram, user mode
  WORD16         cbas0;      // CodeBaseAdr Seg-0, user mode
  WORD16         cbas1;      // CodeBaseAdr Seg-1, user mode
  BYTE          csize0;      // Size CodeSeg-0, user mode
  BYTE          csize1;      // Size CodeSeg-1, user mode
  BYTE             lbs;      // Low bank select
  BYTE             hbs;      // High bank select
};
#endif


struct Reg251  {         // 80251 Registers
  union  {
    BYTE    bregs [16];
    WORD16  wregs [16];
    DWORD   dregs [16];
  } r;

  DWORD            nPC;  // full address !
  BYTE              sp;  // SP
  BYTE             psw;  // PSW-sfr
  BYTE            psw1;  // PSW1-sfr
  BYTE               b;  // B-sfr
  BYTE             acc;  // ACC-sfr
  BYTE             dpl;  // DPL-sfr
  BYTE             dph;  // DPH-sfr
  BYTE        ports[4];
  INT64        nCycles;  // cycle counter
};

struct RegS6  {          // iMCS51 - SLE66Cxx (ECO2000) Registers
  DWORD          nPC;
  INT64      nCycles;    // cycle counter
  BYTE           psw;
  BYTE            sp;
  BYTE             b;
  BYTE           acc;
  BYTE           dpl;
  BYTE           dph;
  BYTE     bregs [8];
  BYTE         dpsel;
  BYTE         adrxh;    // page for MOVX @Ri

  WORD16    dptx [8];    // DPTR save area
};

struct RegARM   {            // iMCSARM Registers
  DWORD  cur[16];            // Current Mode:   R0..R15(PC)
  DWORD  CPSR;               // CPSR
  DWORD  SPSR;               // Current SPSR
  DWORD  usr[7];             // User & System:  R8..R14
  DWORD  fiq[8];             // Fast Interrupt: R8..R14, SPSR
  DWORD  irq[3];             // Interrupt:      R13,R14, SPSR
  DWORD  svc[3];             // Supervisor:     R13,R14, SPSR
  DWORD  abt[3];             // Abort:          R13,R14, SPSR
  DWORD  und[3];             // Undefined:      R13,R14, SPSR
  INT64  nCycles;            // cycle counter
};

#pragma pack()

#define SZ_R166   (OFO (struct Reg166, macc))
#define SZ_ST10   (sizeof (struct Reg166))
#define SZ_RX51   (sizeof (struct Regx51))
#define SZ_SLSZ   (sizeof (struct RegS6))
#define SZ_S251   (sizeof (struct Reg251))
#define SZ_51MX   (sizeof (struct Reg51MX))
#define SZ_RARM   (sizeof (struct RegARM))


/*
 * iMCS-51 SFR-Attribute codes
 */

#define nSF   0x0000      // SFR not present
#define iSF   0x8000      // SFR is present
#define pSF   0x4000      // SFR is present and is Port-0...3
#define cSF   0x2000      // ECO 'external SFR' (EEPROM,TIMER,UART)
#define dSF   0x1000      // 80320: Sfr is DPL/DPH or DPL1/DPH1
// Note: the above need to be combined with ATR_READ/ATR_WRITE
//       for selectively attribute configuration
//       iAV replaced by ATR_READ/ATR_WRITE


/*
 * Memory spaces
 */

#define   mmNONE   0x0000           // not spaced
#define   mmXDATA  0x0001           // XDATA
#define   mmBANK0  0x0080           // BANK0
#define   mmBANK31 0x009F           // BANK31
#define   mmDATA   0x00F0           // DATA
#define   mmBIT    0x00F1           // BIT
#define   mmEDATA  0x00F2           // EDATA (i251)
#define   mmIDATA  0x00F3           // IDATA
#define   mmECODE  0x00F4           // 251 ecode
#define   mmHDATA  0x00F5           // 251 hdata
#define   mmHCONS  0x00F6           // 251 hconst
#define   mmCONST  0x00F7           // 251 const
#define   mmPDATA  0x00FE           // PDATA (c51 macht das bei generic)
#define   mmCODE   0x00FF           // CODE 
#define   mmPHYS   0x0100           // Physical SLE66+ Memory
#define   mmPHLIM  0x0110           // 1MB Physical Memory

#define   mmU1MEM  0x00F8           // UsrMem #1 (E2-PROM)
#define   mmU2MEM  0x00F9           // UsrMem #2 (undefined)
#define   mmU3MEM  0x00FA           // UsrMem #3 (undefined)
#define   mmU4MEM  0x00FB           // UsrMem #4 (undefined)
#define   mmU5MEM  0x00FC           // UsrMem #5 (undefined)
#define   mmU6MEM  0x00FD           // UsrMem #6 (undefined)

#define   mmSFR000 0x0200           // SFR-Bank 0...255
#define   mmSFR255 0x02FF           // SFR-Bank 255 (D:255)



#if 0
//--- SLE88Cxx specific's: -------------------------------

struct RegSle  {
  union  {
//  WORD16   wregs[16];      // ECO-2 word regs
    DWORD    dregs [9];      // SLE R0-R7,A
  } r;
  DWORD            nPC;      // LoWord is relevant !!!
  DWORD            Psw;      // SLE PSW (32-Bit)
  DWORD           Reco;      //
  DWORD           Aram;      // 
  INT64        nCycles;      // cycle counter
  WORD16            sp;
  BYTE              cs;
  BYTE              ss;
  BYTE             ds0;
  BYTE             ds1;
  BYTE             ts0;
  BYTE             ts1;
};
#endif


//--------------------------------------------------------------

#define iSLE88     100     // dead !
#define i80x51     0x00    // generic 80x51
#define iEXTMAC    0x01    // MAC167
#define iSLE66     0x02    // SLE66
#define iC51MX     0x03    // Philips 80C51MX
#define iRF5016    0x04    // Philips P8RF5016 (Mifare pro X)
#define iSMARTMX   0x05    // Philips SmartMX


#define xxMENU    UsrDwrd[0]   // holds CAN-Extension Menu (DYM *)
#define xxUPDATE  UsrDwrd[1]   // holds 'void (*CanUpdate) (DWORD nCode)'
#define xxMEMCB   UsrDwrd[2]   // holds 'DWORD (*pFlashAcc) (DWORD nAdr, DWORD nCnt)

#define xxSFRMUX  UsrDwrd[13]  // holds 'SfrMux' ((BYTE *) &SfrMux[0])  /SLE66/
 
#define xxDCIPRM  UsrDwrd[14]  // holds 'DWORD &_dciUV2' for DCI Interface
#define Dio    ((struct dciUV2 *)(pio->UsrDwrd[14]))->dio
#define Dconn  ((struct dciUV2 *)(pio->UsrDwrd[14]))->dconn

//
// Sle66: ioc.pUv[0] = WORD16 *pTab;  // 256 Words fuer Peripheral-Config
//


#define   CMD_EXTCOVER   0x1000
// S8051/S166: Extended Coverage:
//      DWORD bR = ioc.SimFunc (CMD_EXTCOVER, char *"filename", DWORD nFlags);
//   bR:      1:=Ok, 0:=Error (file open/close/io)
//   nFlags:  1:=show jump-coverage details (like 'coverage details')




struct bom  {
  DWORD          nSize;
  WORD16  bInitialized;
  WORD16          iMCS;    // ToolSet Definitions 0=51, 1=251, 2=166, 3=Fox
  WORD16        iMinor;    // SubCode: iMCS166(1):=MAC167
  DWORD         iModel;    // memory model (based on iMCS)
  DWORD           iRtx;    // which RTX (0=none, 1=RtxTiny, 2=RtxFull)

  DWORD     RunDeb : 1;    // dScope is running
  DWORD     CmdLog : 1;    // 1 := command log file is active
  DWORD     SerLog : 1;    // 1 := serial log file active
  DWORD       iRun : 1;    // GO or Step currently running
  DWORD   InfoiRun : 1;    // Sim about to start running
  DWORD      iBoot : 1;    // currently starting up Sim
  DWORD     iReset : 1;    // Reset issued via Button
  DWORD   wdog_rst : 1;    // Reset via Watchdog Timer
  DWORD     logmod : 1;    // 0:=LOG, 1:=SLOG
  DWORD     target : 1;    // 1:=target mode
  DWORD    jSilent : 1;    // kill function without query

  DWORD    MemAccR : 1;    // Memory-access possible while running
  DWORD    RegAccR : 1;    // Register-access possible while running
  DWORD     hTrace : 1;    // trace support
  DWORD     hCover : 1;    // code coverage support
  DWORD    hPaLyze : 1;    // Performance Analyzer support
  DWORD    hMemMap : 1;    // Memory-Map support
  DWORD     ResetR : 1;    // Reset possible while running
  DWORD     WinAcc : 1;    // ReadMem/WriteMem/LoadTyp activated
                           // by some non-simulation vehicle

  ZBK            *pScp;    // current Scope via WaScope()
  ZXFIL         *pScpF;    // and the associated file
  LIN             *pLn;    // and the line number
  SYM            *SfrH;    // SFR-Symbols supplied by Cpu/Peri-DLL
  SYM           *hIntr;    // head of Intrinsic's
  SYM           *tIntr;    // head of Intrinsic's
  SYM           *hFunc;    // head of Functions (User/Signal

  struct szPAT  *hSrcP;    // head of search pathes (via 'set src')
  struct szPAT  *tSrcP;    // tail of search pathes (via 'set src')

  DWORD       startasm;    // DasmFox exports for LWinVw
  DWORD       altstart;
  DWORD         MarkPC;

                           // for enhancements:
  BYTE     UsrByte[16];    // currently not used.
  WORD16   UsrWord[16];    // currently not used.
  DWORD       movb : 1;    // MOVB instruction executed (SLE66)
  DWORD     SfrRdy : 1;    // data from sfr is valid (SLE66)
  DWORD      DemoF : 1;    // about to debug a demo file...
  DWORD      ModV2 : 1;    // 166 MODV2
  DWORD    Loading : 1;    // flag: loading a file
  DWORD  FlashLoad : 1;    // Flash-Load mode

  DWORD           : 26;    // currently not used.
  DWORD    UsrDwrd[15];    // currently not used.


//--- debug specific flags: -----------------------------------

  DWORD   periodic : 1;    // periodic window update Flag
  DWORD      aLwin : 1;
  DWORD     aCover : 1;
  DWORD      aSer1 : 1;
  DWORD      aSer2 : 1;
  DWORD        aPa : 1;
  DWORD       aBox : 1;
  DWORD   viewmode : 3;    // LwinVw: view mode
  DWORD      vrSel : 4;    // LwinVw: address-range selection
  DWORD       aSym : 1;    // Symbols Dialog active
  DWORD      aTbox : 1;    // ToolBox is active
  DWORD      AscS1 : 1;    // Serial #1 - 0:=Ascii, 1:=Hex mode
  DWORD      AscS2 : 1;    // Serial #2 - 0:=Ascii, 1:=Hex mode

  BYTE           trace;    // Record trace information
//-------------------------------------------------------------

  BYTE           ptree;    // dump expression tree (_ptree_)
  BYTE           traps;    // enable/disable interrupt/trap messages
  BYTE           *trap;    // Trace buffer
  int           curtra;    // current trace index
  int           numtra;    // records in buffer

  int          cbreaks;    // number of C-breaks
  BKS          *BpHead;    // List head of Breakpoints
  BKS          *BpTail;    // List tail of Breakpoints
  DWORD         AccOff;    // if > 0: ignore access breaks
  BYTE     BpBlock : 1;    // Block Bp-commands if Bp-Dialog is active
  BYTE     PaBlock : 1;    // Block PA-commands if PA-Dialog is active
  BYTE       PaDlg : 1;    // PA-Dialog is open
  BYTE       PaRun : 1;    // PA is running
  BYTE       PaTim : 1;    // Update PA time information
  BYTE         FuncGen;
  BYTE          KeyEsc;    // Escape key has been pressed

                           // log file
  char        *CmdLogN;    // name of command log file
  FILE        *CmdLogH;    // file handle of command log file
  char        *SerLogN;    // name of serial log file
  FILE        *SerLogH;    // file handle of serial log file

  
  HMODULE        Linst;    // DLL Instance handle
  HMODULE        Ginst;    // AGDI Instance handle
  HMODULE        Rinst;    // RTX-DLL Instance handle
  HWND            hwnd;    // CMainFrame's m_hWnd, Parent for Dll-Dialogs
  DWORD           hmsg;    // WM_OUTOUT Message

  DYM           *pMenu;    // attached by peripheral dll
  DYM           *pMtrg;    // attached via AGDI
  DYM           *pMrtx;    // attached via RTX dll

  int       (*vp) (struct bom *pioc, DWORD nCode);   // DLL init/uninit
  int      (*rvp) (DWORD nCode, void *p1);           // RTX-DLL init/uninit

  HWND          curDlg;    // currently active modeless dialog

  union rstat       st;    // Run-Status Info
  DWORD            nPC;    // Offset (16-bit) of the PC
  INT64         cycles;    // number of cycles executed
  int            etime;    // execution time, counts down from tdelay to 0
  int           tdelay;    // Time till next time watch
  WORD16         error;    // Error-code
  DWORD         ErrAdr;    // Opt. Error-address

  DWORD         OscFrq;    // Oscillator frequency

  VTR            *pVTA;    // allocated array of MAXVTR VtReg's
  WATCH        **pVTwa;    // and watch pointer list
  DWORD            nVT;    // number of currently used VtReg's

  CSTK           *pStk;    // call-stack entries
  int             nStk;    // call-stack depth tracker

  int              tos;    // PA-stack index
  PAST           *StPa;    // PA-stack slots
  PA           *PaBgHd;    // background PA always present.
  PA           *PaHead;    // Head of PA ranges
  PA           *PaTail;    // Tail of PA ranges
  DWORD         paEadr;    // PA current range entry/exit address
  INT64         CapCyc;    // captures cycles


//  struct FoxCont   fox;
#if 1
  union itrp      itrp;    // Interrupt status
  BYTE           sleep;    // CPU sleep mode
  BYTE         _break_;    // Stop execution, if _break_ := 1
  BYTE           radix;    // number base, for compat. to dScope
  BYTE       intchange; 
  DWORD        intvect;    // interrupt vector address if itrp.w.flag = 1
  BYTE          intdis;
  DWORD           cPsw;    // current PSW
  DWORD           oPsw;    // old PSW
#endif

  DWORD     EINIT_exec;    // 0=EINIT not yet executed / 1=EINIT executed
  DWORD    SRVWDT_exec;    // 0=SRVWDT not yet executed / 1=SRVWDT executed
  DWORD     WDOGACTIVE;    // 0=Watchdog disabled / 1=Watchdog enabled

                                         // Polymorphic function
  DWORD      (*SimFunc) (DWORD nCode, void *p1, void *p2);

  void         (*LFwLF) (CRT *X);        // DasmFox exports for LwinVw
  void         (*LRwLF) (CRT *X);
  void        (*CrtAtt) (CRT *X);
  char         *(*LGet) (CRT *X);
  BYTE      (*FileLine) (ZXFIL *pF, DWORD nLine);
  ZXFIL   *(*FileScope) (DWORD nAdr);    // End of DasmFox exports
  ZXFIL    *(*DocToZXF) (LPCTSTR pN);    // Map Docname to ZXFIL

  WORD16    *(*GetAttr) (DWORD nAdr);    // get attr for address

  void       (*StopRun) (void);          // Uv3-Stop-Button was pressed
  void      (*ServeWDT) (void);          // Watchdog timer function
  void         (*GoTil) (DWORD nAdr);    // Go until for Fox-Sim test

  int       (*HistFunc) (int nCode, int indx, int n1, BYTE *pR);
  DWORD       (*PaFunc) (int nCode, int nAdr, void *p);

  void        (*GetOpc) (DWORD nAdr, DWORD *vp); // get 4-Bytes Opcode
  DWORD       (*SetOpc) (DWORD nAdr, BYTE *pB, DWORD mMany);
  DWORD      (*ReadMem) (DWORD nAdr, DWORD nMany, BYTE *vp);
  DWORD     (*WriteMem) (DWORD nAdr, DWORD nMany, BYTE *vp);

  void	        (*Step) (DWORD nMany, DWORD type);   // Step function
  void	        (*Stop) (void);          // Notbremse
  void         (*Reset) (WORD16 nFlags); // Reset Simu
  void       (*GetRegs) (void *vp /*struct FoxReg *vp*/);
  void       (*SetRegs) (void *vp /*struct FoxReg *vp*/);
  DWORD       (*MapMem) (DWORD SegTyp, DWORD nAdr1, DWORD nLen);

  int       (*ValidAdr) (DWORD nAdr, int nCode);
  DWORD       (*BpInfo) (DWORD nAdr, WORD16 nCode);
  DWORD     (*BpExists) (DWORD nAdr);
  BKS     *(*BreakFunc) (WORD16 nCode, WORD16 n1, DWORD nAdr, BKS *pB);
  BKS      *(*NewBreak) (DWORD nAdr, WORD16 nType, WORD16 nCode);
  void         (*DlgBp) (struct BpBlk *pB);        // create Dialog Bp

  VTR      *(*p_create) (char *name, WORD16 nTyp);    // create VTREG
  void      (*p_putval) (VTR *pV, union v *pCont);
  void      (*p_getval) (VTR *pV, union v *pCont);
  VTR       *(*FindVTR) (char *name);                 // find VTREG
  void        (*w_read) (pACCESS ac, WFP cur, WFP pnew);
  void       (*w_write) (pACCESS ac, WFP pnew);
  void         (*inval) (DWORD val);
  int       (*w_access) (DWORD adr, DWORD mask);
  void     (*ResetPeri) (void);        // Reset Peripherals
  void      (*w_create) (pWATCH wp);
  int       (*t_create) (INT64 tv, void (*fc)(void));
  void      (*t_remove) (int indx);
  void      (*intcheck) (void);

                                         // Serial I/O
  void         (*SerIn) (int nCom, WORD16 val);  // Keypress in SerVw

  void     (*DlgUpdate) (void);          // update peripheral dialogs
  void     (*ExtDlgUpd) (void);          // update AGDI extension dialogs
  void     (*RtxUpdate) (DWORD nCode);   // update RTX extension dialog(s)


//------------------------
//--- Kernel functions ---
//------------------------

  void      (*DoEvents) (void);
  DWORD       (*Notify) (DWORD nCode, void *lp1);
  WORD16      (*RunCmd) (char *cmd, int nLen);
  void     (*CmdWinOut) (char *text);
  void     (*CmdCmdOut) (char *text);
  void       (*FreeEnv) (DWORD env);
  void       *(*GetMem) (DWORD many, int env);
  char   *(*SaveString) (char *s, int env);

  void       (*CleanUp) (void);                // clean up after Stop-Debug
  int     (*SerByteOut) (int nCom,   BYTE *pS, DWORD nMany); // write into SerWin
  int     (*SerWordOut) (int nCom, WORD16 *pW, DWORD nMany); // write into SerWin

  void      (*LogWrite) (int nSel, char *pLine, int nMany);
  void      (*BootHelp) (DWORD hlpExpr, DWORD noHelp, DWORD hStart);
  void     (*CreaWatch) (IRX *pX);
  WORD16   (*WatchCalc) (EXP *ep, char *pB);
  WORD16    (*AsnWatch) (char *exp, int nLen);
  void       (*LoadTyp) (char *pS, DWORD nAdr, int base, TYP *tp, union v *pU);
  void       (*LoadStr) (char *pS, DWORD nAdr, int nMax);
  ZBK       *(*WaScope) (DWORD nAdr);
  void        (*m_copy) (DWORD nDest, DWORD nSec, DWORD nMany);

  DWORD      (*TaskRun) (union v *pV);
  SYM       *(*FindPub) (char *name);
  SYM   *(*PubSymByVal) (UINT64 nVal, DWORD nMask);
  DWORD    (*FetchItem) (UINT64 nAdr, TYP *tp, union v *pU);
  INT32    (*DecodeAdr) (DWORD nCode, UINT64 nAdr, char *pB);
  DWORD      (*pCbFunc) (DWORD nCode, void *vp);     // AGDI-style callback

  WORD16      *pSfrTab;      // iMCS51: Sfr-Attribute table
  WORD16         ticks;
  WORD16        nticks;      // default: 12 Ticks per Cycle
  BYTE             iip;
  BYTE          int_ff;
  BYTE           delay;
//BYTE       intchange;  // use above declaration !
  BYTE       interrupt;
  BYTE        cpusleep;
//DWORD        intvect;  // use above declaration !
  DWORD       xpagereg;
  DWORD     *exectimes;  // execution times table
  WORD16      xonepage;
  WORD16   exectimeint;  // i251: interrupt exec time
  WORD16     execsleep;  // i251: sleep exec time
  BYTE          oddjmp;  // i251: odd target jump time
  BYTE        jmptaken;  // i251: taken 251-jumps time
  BYTE       bjmptaken;  // i251: add for jmp taken JB,JNB,JBC
  BYTE       cjnetaken;  // i251: add for cjne taken
  BYTE       djnztaken;  // i251: add for djnz taken
  BYTE      jmp51taken;  // i251: add for 51 jmp taken JZ,JNZ,JB,JNB,JB,JC,JNC 

  double   (*pCalcTime) (INT64 cyc);

  DWORD       *pOldAdr;  // Old address (requ. for Sle66)
  union v    *pUv [15];  // place holder ([16] --> [15] wg. u_create
  int       (*u_create) (INT64 uSec, void (*fc)(void));

  char     szMsg[2048];  // place holder for future extensions
};


extern struct bom   ioc;
extern struct bom  *pio;



//#pragma pack(push, 4)
struct EvCA  {        // used for DTC-Calc Expr. Event (Notify --> SimFunc)
  LPCTSTR   pStr;     // expression-text
  __int64   nRes;     // Result
  int       nErr;
};
//#pragma pack(pop)

struct DtcMessage  {  // for DTC-Target-Messaging
  char    szB [12];
  int     len;
  int     seq;
  int     cha;
  INT64   tSt;        // Time-Stamp
};


typedef struct DTC_MRW {   // for DTC-Memory Read/Write
  UINT64     nAdr;    // Memory Address
  int       nMany;    // number of Bytes to read/write
  BYTE        *pB;    // Buffer
  int       nMode;    // Mode: 
  int        nErr;    // Error-Code
  UINT64  nErrAdr;    // Error-Address
  BYTE    Res[32];    // reserved.
  INT64        v1;    // z.B. States (DecodeTimeStamp)
  INT64        v2;
  double       v3;    // z.B. Seconds (-++-)
} MEM_RW;
#define MEM_RW_RD    0x01
#define MEM_RW_WR    0x02
#define CNV_TSTAMP   0x03
#define MEM_DEC_ADR  0x04


/*
 * DCI command messages (uses Windows MSG structure)
 */

#define DCI_BUILDPRJ        0x1000     // build current project
#define DCI_GETBUILDSTATUS  0x1001     // get current project build status
#define DCI_OPENPROJECT     0x1002     // open project
#define DCI_CLOSEPROJECT    0x1003     // close project


//--- DCI-Command return values via 'pAck':


#define DCI_CMDOK           1   // command was Ok.
#define DCI_CMDNOTALLOWED   2   // command cannot be executed now
#define DCI_BLDERRORS       3   // prevous build-project had error(s)
#define DCI_CMDFAILED       4   // dci command failed (e.g. open project)

#define DCI_DONTKNOW       -1   // don't know (example build-status)
#define DCI_UV2READY       -2   // Uv2 booted (Uv2 - not debug mode)
#define DCI_UV2DEBUG       -3   // Uv2 debug modus


/*
 * Flash-Capabilities provided by Target-DLL (Monxxx.dll)
 */
#define CAP_WRITE   0x0001    // Flash-Write (DownLoad)
#define CAP_ERASE   CAP_WRITE    // Flash-Erase



#ifdef __cplusplus
  }                           // End of 'extern "C"'
#endif

#endif
