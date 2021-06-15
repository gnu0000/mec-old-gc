/*
 *
 * dis.c
 * Tuesday, 3/4/1997.
 * Craig Fitzgerald
 *
 *
 * This file contains several globals and is in general quite bulky
 * for speed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include <GnuFile.h>
#include <GnuArg.h>
#include "gclib\gclib.h"
#include "gclib\opcodes.h"
#include "gclib\binfile.h"

typedef struct _func
   {
   PSZ  pszName;
   UINT uSize;
   PSZ  pszCode;
   PSZ  pszGlobals;
   struct _func *left;
   struct _func *right;
   } FUNC;
typedef FUNC *PFUNC;

#define STACK_SIZE 0x1000

CHAR szINFILE [256];

PSZ   pszGLOBALS = NULL; // globals area malloced at file load
PSZ   pszCODE    = NULL; // assigned to pszCode of current fn ???
PLONG plSTACK    = NULL; // malloced at startup


INT  iSF = 0;           // stack frame register
INT  iSP = 0;           // stack pointer
INT  iIP = 0;           // instruction ptr

UCHAR cOP;
UCHAR cOPKIND;
UCHAR cOPTYP;

void DumpFunction (PFUNC pFunc);
void DumpGlobals (PSZ pszGlobals, UINT uSize);

void Usage (void)
   {
   printf ("DIS  Gnu GX module dissasembler                v0.1       %s %s\n\n", __DATE__, __TIME__);
   printf ("USAGE: DIS gxfile\n\n");
   printf ("WHERE: gxfile  is a module compiled with GC\n");
   exit (0);
   }


void BadOp (UCHAR cOp)
   {
   Error ("Bad Operand %2.2x IP:%4.4x(%5d) SP:%4.4x(%5d) SF:%4.4x(%5d)", 
           cOp, iIP, iIP, iSP, iSP, iSF, iSF);
   }


/***************************************************************************/

PFUNC CodNewFunction (void)
   {
   PFUNC pFunc;

   pFunc = calloc (sizeof (FUNC), 1);
   return pFunc;
   }


/***************************************************************************/

/*
 * allocs result
 */
PSZ ReadName (FILE *fp)
   {
   UCHAR szTmp[128];
   PSZ  pszTmp = szTmp;
   INT  c;

   for (c = f_getc (fp); c && c != EOF ; c = f_getc (fp))
      *pszTmp++ = c;
   *pszTmp = '\0';

   return strdup (szTmp);
   }


/*
 * allocs result
 */
PSZ ReadBuffer (FILE *fp, UINT uLen)
   {
   PSZ pszBuff;

   pszBuff = calloc (uLen + 1, 1);
   if (fread (pszBuff, 1, uLen, fp) < uLen)
      Error ("Unexpected EOF");
   return pszBuff;
   }


/*
 * global memory is a contiguous memory block
 * strings are actually 4byte pointers to strings outside
 * this block
 */
void LoadGlobals (FILE *fp)
   {
   UINT uSize;

   uSize = FilReadUShort (fp);
   pszGLOBALS = ReadBuffer (fp, uSize);

   if (FilReadUShort (fp) != BIN_TERM)
      Error ("bin file in bad format! (term)");
   DumpGlobals (pszGLOBALS, uSize);
   }


void LoadFunction (FILE *fp)
   {
   PFUNC pFunc;

   pFunc = CodNewFunction ();
   pFunc->pszName = ReadName (fp);
   pFunc->uSize   = FilReadUShort (fp);
   pFunc->pszCode = ReadBuffer (fp, pFunc->uSize);

   if (FilReadUShort (fp) != BIN_TERM)
      Error ("bin file in bad format! (term)");

   pszCODE = pFunc->pszCode;
   DumpFunction (pFunc);
   }


void LoadProgram (PSZ pszFile)
   {
   FILE *fp;
   UINT u;
   BOOL bLoop = TRUE;

   if (!(fp = fopen (pszFile, "rb")))
      Error ("Unable to open program file %s", pszFile);

   if (FilReadUShort (fp) != BIN_FILEHEADER)
      Error ("File is not a GNU program file: %s", pszFile);

   while (bLoop && (u = FilReadUShort (fp)) && !feof (fp))
      {
      switch (u)
         {
         case BIN_FUNCTIONHEADER:
            LoadFunction (fp);
            break;

         case BIN_GLOBALHEADER:
            LoadGlobals (fp);
            break;

         case BIN_TERM:
            bLoop = FALSE;

         default:
            Error ("bin file in bad format!");
            break;
         }
      }
   fclose (fp);
   }



/***************************************************************************/
/*                                                                         */
/* code stream manipulation                                                */
/*                                                                         */
/***************************************************************************/


UCHAR sz1[128];
UINT uOpLen;


INT NextByte (void)
   {
   UCHAR c;

   c = *(PUCHAR)(pszCODE + iIP);
   uOpLen++;
   return c;
   }

SHORT NextShort (void)
   {
   INT i;

   i = *(PSHORT)(pszCODE + iIP);
   uOpLen += MEMSIZE_SHORT;
   return i;
   }

LONG NextWord (void)
   {
   LONG l;

   l = *(PLONG)(pszCODE + iIP);
   uOpLen += MEMSIZE_WORD;
   return l;
   }

/***************************************************************************/

int VMCleanup (void)
   {
   return 0;
   }


void PByte (void)
   {
   sprintf (sz1, "%2.2x", NextByte ());
   }

void PShort(void)
   {
   sprintf (sz1, "%4.4x", NextShort ());
   }

void POff(void)
   {
   INT i;

   i = NextShort ();
   sprintf (sz1, "%4.4x (%4.4x)", i, iIP + i + 2);
   }

void PWord(void)
   {
   sprintf (sz1, "%p", NextWord ());
   }

void PAddr(void)
   {
   sprintf (sz1, "%Fp", NextWord ());
   }

void PString (void)
   {
   sprintf (sz1, "\"%s\"", pszCODE + iIP);
   uOpLen += strlen (pszCODE + iIP) + 1;
   }

void PFloat (void)
   {
   PBIG pbg;

   pbg = (PBIG) (pszCODE + iIP);
   sprintf (sz1, "%lf", *pbg);
   uOpLen += MEMSIZE_FLOAT;
   }


void PSwitch (void)
   {
   INT  i, iLen, iTmp;
   LONG l;
   BOOL bDef;

   iLen = NextShort ();
   iIP+=2;
   bDef = NextShort ();
   iIP-=2;
   for (i=0; i<iLen; i++)
      {
      l = NextWord ();
      iTmp = NextShort ();
      }
   if (bDef)
      NextShort ();

   sprintf (sz1, "<%d>%s", iLen, (bDef ? ", def" : ""));
   }

/***********************************************************************/



void PrintBytes (PSZ pszCODE, PINT piAddr, PINT piLen, INT iMaxLen)
   {
   INT i;

   for (i=0; i<iMaxLen; i++)
      {
      if (!*piLen)
         {
         printf ("   ");
         continue;
         }
      printf ("%2.2x ", pszCODE[(*piAddr)++]);
      (*piLen)--;
      }
   }


/*
 * globals: pszCODE, uOpLen
 *
 *
 */
void PrintOp (INT iAddr, UINT uLen, PSZ pszOp, PSZ pszOper)
   {
   BOOL  bSwitch;
   PLONG pLong;
   PINT  pJump;

   bSwitch = (pszCODE[iAddr] == OP_SWITCH);

   /*--- print Address ---*/
   printf ("[%4.4x] ", iAddr);

   /*--- print bytes ---*/
   printf ("%2.2x ", pszCODE[iAddr++]);       // opcode
   PrintBytes (pszCODE, (PINT)&iAddr, (PINT)&uLen, 4);

   printf ("   %s%s\n", pszOp, pszOper);

   if (!uLen)
      return;

   while (uLen)
      {
      if (bSwitch)
         {
         printf ("[%4.4x]    ", iAddr);
         pLong = (PLONG)(pszCODE + iAddr);
         pJump = (PINT)(pszCODE + iAddr+4);
         if (uLen == 2)
            {
            pJump = (PINT)(pszCODE + iAddr);
            PrintBytes (pszCODE, (PINT)&iAddr, (PINT)&uLen, 6);
            printf ("              %4.4x (%4.4x)\n", *pJump, iAddr + *pJump);
            }
         else
            {
            PrintBytes (pszCODE, (PINT)&iAddr, (PINT)&uLen, 6);
            printf ("   %8.8lx   %4.4x (%4.4x)\n", *pLong, *pJump, iAddr + *pJump);
            }
         }
      else
         {
         printf ("[%4.4x]    ", iAddr);
         PrintBytes (pszCODE, (PINT)&iAddr, (PINT)&uLen, 4);
         printf ("\n");
         }
      }
   }




/***********************************************************************/


/*
 * This writes a disassembly listing of a GX function
 * remember: no functions over 25 lines!
 */
void DumpFunction (PFUNC pFunc)
   {
   INT iAddr;
   PSZ p1;

   printf ("-----------------------------------------------\n");
   printf ("Name:%-25s Size:%4.4x(%5d)\n", pFunc->pszName, pFunc->uSize, pFunc->uSize);
   printf ("-----------------------------------------------\n");

   for (iIP = 0; iIP < (INT)pFunc->uSize;)
      {
      iAddr = iIP;
      iIP++;
      *sz1= '\0';
      uOpLen = 0;
      switch (pFunc->pszCode[iAddr])
         {
         case OP_NOP:              p1 = "NOP       ";             break;
         case OP_CALL:             p1 = "CALL      "; PString();  break;
         case OP_CALLI:            p1 = "CALLI     "; PShort();   break;
         case OP_JMP:              p1 = "JMP       "; POff();     break;
         case OP_JZ:               p1 = "JZ        "; POff();     break;
         case OP_JNZ:              p1 = "JNZ       "; POff();     break;
         case OP_JZK:              p1 = "JZK       "; POff();     break;
         case OP_JNZK:             p1 = "JNZK      "; POff();     break;
         case OP_PUSHSF:           p1 = "PUSHSF    ";             break;
         case OP_PUSHSP:           p1 = "PUSHSP    ";             break;
         case OP_POPSF:            p1 = "POPSF     ";             break;
         case OP_PUSH0:            p1 = "PUSH0     ";             break;
         case OP_PUSH1:            p1 = "PUSH1     ";             break;
         case OP_DUPS:             p1 = "DUPS      ";             break;
         case OP_SWAPS:            p1 = "SWAPS     ";             break;

         case OP_POPL + M_BYTE:    p1 = "POPL[B]   "; PShort();   break;
         case OP_POPL + M_SHORT:   p1 = "POPL[S]   "; PShort();   break;
         case OP_POPL + M_WORD:    p1 = "POPL[W]   "; PShort();   break;
         case OP_POPL + M_FLOAT:   p1 = "POPL[F]   "; PShort();   break;
         case OP_POPLS + M_BYTE:   p1 = "POPLS[B]  ";             break;
         case OP_POPLS + M_SHORT:  p1 = "POPLS[S]  ";             break;
         case OP_POPLS + M_WORD:   p1 = "POPLS[W]  ";             break;
         case OP_POPLS + M_FLOAT:  p1 = "POPLS[F]  ";             break;
         case OP_POPG + M_BYTE:    p1 = "POPG[B]   "; PAddr();    break;
         case OP_POPG + M_SHORT:   p1 = "POPG[S]   "; PAddr();    break;
         case OP_POPG + M_WORD:    p1 = "POPG[W]   "; PAddr();    break;
         case OP_POPG + M_FLOAT:   p1 = "POPG[F]   "; PAddr();    break;
         case OP_POPGS + M_BYTE:   p1 = "POPGS[B]  ";             break;
         case OP_POPGS + M_SHORT:  p1 = "POPGS[S]  ";             break;
         case OP_POPGS + M_WORD:   p1 = "POPGS[W]  ";             break;
         case OP_POPGS + M_FLOAT:  p1 = "POPGS[F]  ";             break;
         case OP_PUSHL + M_BYTE:   p1 = "PUSHL[B]  "; PShort();   break;
         case OP_PUSHL + M_SHORT:  p1 = "PUSHL[S]  "; PShort();   break;
         case OP_PUSHL + M_WORD:   p1 = "PUSHL[W]  "; PShort();   break;
         case OP_PUSHL + M_FLOAT:  p1 = "PUSHL[F]  "; PShort();   break;
         case OP_PUSHLS + M_BYTE:  p1 = "PUSHLS[B] ";             break;
         case OP_PUSHLS + M_SHORT: p1 = "PUSHLS[S] ";             break;
         case OP_PUSHLS + M_WORD:  p1 = "PUSHLS[W] ";             break;
         case OP_PUSHLS + M_FLOAT: p1 = "PUSHLS[F] ";             break;
         case OP_PUSHG + M_BYTE:   p1 = "PUSHG[B]  "; PAddr();    break;
         case OP_PUSHG + M_SHORT:  p1 = "PUSHG[S]  "; PAddr();    break;
         case OP_PUSHG + M_WORD:   p1 = "PUSHG[W]  "; PAddr();    break;
         case OP_PUSHG + M_FLOAT:  p1 = "PUSHG[F]  "; PAddr();    break;
         case OP_PUSHGS + M_BYTE:  p1 = "PUSHGS[B] ";             break;
         case OP_PUSHGS + M_SHORT: p1 = "PUSHGS[S] ";             break;
         case OP_PUSHGS + M_WORD:  p1 = "PUSHGS[W] ";             break;
         case OP_PUSHGS + M_FLOAT: p1 = "PUSHGS[F] ";             break;
         case OP_PUSHI + M_BYTE:   p1 = "PUSHI[B]  "; PByte();    break;
         case OP_PUSHI + M_SHORT:  p1 = "PUSHI[S]  "; PShort();   break;
         case OP_PUSHI + M_WORD:   p1 = "PUSHI[W]  "; PWord();    break;
         case OP_PUSHI + M_FLOAT:  p1 = "PUSHI[F]  "; PFloat();   break;
         case OP_INCL + M_BYTE:    p1 = "INCL[B]   "; PShort();   break;
         case OP_INCL + M_SHORT:   p1 = "INCL[S]   "; PShort();   break;
         case OP_INCL + M_WORD:    p1 = "INCL[W]   "; PShort();   break;
         case OP_INCL + M_FLOAT:   p1 = "INCL[F]   "; PShort();   break;
         case OP_INCG + M_BYTE:    p1 = "INCG[B]   "; PAddr();    break;
         case OP_INCG + M_SHORT:   p1 = "INCG[S]   "; PAddr();    break;
         case OP_INCG + M_WORD:    p1 = "INCG[W]   "; PAddr();    break;
         case OP_INCG + M_FLOAT:   p1 = "INCG[F]   "; PAddr();    break;
         case OP_DECL + M_BYTE:    p1 = "DECL[B]   "; PShort();   break;
         case OP_DECL + M_WORD:    p1 = "DECL[W]   "; PShort();   break;
         case OP_DECL + M_FLOAT:   p1 = "DECL[F]   "; PShort();   break;
         case OP_DECG + M_BYTE:    p1 = "DECG[B]   "; PAddr();    break;
         case OP_DECG + M_WORD:    p1 = "DECG[W]   "; PAddr();    break;
         case OP_DECG + M_FLOAT:   p1 = "DECG[F]   "; PAddr();    break;
         case OP_ISP:              p1 = "ISP       "; PShort();   break;
         case OP_DSP:              p1 = "DSP       "; PShort();   break;
         case OP_FTOW:             p1 = "FTOW      ";             break;
         case OP_WTOF:             p1 = "WTOF      ";             break;
         case OP_ADDRL:            p1 = "ADDRL     ";             break;
         case OP_ADDRG:            p1 = "ADDRG     ";             break;
         case OP_BYTE:             p1 = "BYTE      ";             break;
         case OP_SHORT:            p1 = "SHORT     ";             break;

         case OP_SAVL + M_BYTE:    p1 = "SAVL[B]   "; PShort();   break;
         case OP_SAVL + M_SHORT:   p1 = "SAVL[S]   "; PShort();   break;
         case OP_SAVL + M_WORD:    p1 = "SAVL[W]   "; PShort();   break;
         case OP_SAVL + M_FLOAT:   p1 = "SAVL[F]   "; PShort();   break;
         case OP_SAVLS + M_BYTE:   p1 = "SAVLS[B]  ";             break;
         case OP_SAVLS + M_SHORT:  p1 = "SAVLS[S]  ";             break;
         case OP_SAVLS + M_WORD:   p1 = "SAVLS[W]  ";             break;
         case OP_SAVLS + M_FLOAT:  p1 = "SAVLS[F]  ";             break;
         case OP_SAVG + M_BYTE:    p1 = "SAVG[B]   "; PAddr();    break;
         case OP_SAVG + M_SHORT:   p1 = "SAVG[S]   "; PAddr();    break;
         case OP_SAVG + M_WORD:    p1 = "SAVG[W]   "; PAddr();    break;
         case OP_SAVG + M_FLOAT:   p1 = "SAVG[F]   "; PAddr();    break;
         case OP_SAVGS + M_BYTE:   p1 = "SAVGS[B]  ";             break;
         case OP_SAVGS + M_SHORT:  p1 = "SAVGS[S]  ";             break;
         case OP_SAVGS + M_WORD:   p1 = "SAVGS[W]  ";             break;
         case OP_SAVGS + M_FLOAT:  p1 = "SAVGS[F]  ";             break;

         case OP_SWITCH:           p1 = "SWITCH    "; PSwitch();  break;
         case OP_PUSHNL:           p1 = "PUSHNL    "; PShort();   break;
         case OP_PUSHNG:           p1 = "PUSHNG    "; PAddr ();   break;
         case OP_POPNL :           p1 = "POPNL     "; PShort();   break;
         case OP_POPNG :           p1 = "POPNG     "; PAddr ();   break;
         case OP_POPNLS:           p1 = "POPNLS    ";             break;
         case OP_SAVNL :           p1 = "SAVNL     "; PShort();   break;
         case OP_SAVNG :           p1 = "SAVNG     "; PAddr ();   break;

         case OP_GT + M_WORD:      p1 = "GT[W]     ";             break;
         case OP_LT + M_WORD:      p1 = "LT[W]     ";             break;
         case OP_GE + M_WORD:      p1 = "GE[W]     ";             break;
         case OP_LE + M_WORD:      p1 = "LE[W]     ";             break;
         case OP_EQ + M_WORD:      p1 = "EQ[W]     ";             break;
         case OP_NE + M_WORD:      p1 = "NE[W]     ";             break;
         case OP_ZE + M_WORD:      p1 = "ZE[W]     ";             break;
         case OP_NZ + M_WORD:      p1 = "NZ[W]     ";             break;
         case OP_GT + M_FLOAT:     p1 = "GT[F]     ";             break;
         case OP_LT + M_FLOAT:     p1 = "LT[F]     ";             break;
         case OP_GE + M_FLOAT:     p1 = "GE[F]     ";             break;
         case OP_LE + M_FLOAT:     p1 = "LE[F]     ";             break;
         case OP_EQ + M_FLOAT:     p1 = "EQ[F]     ";             break;
         case OP_NE + M_FLOAT:     p1 = "NE[F]     ";             break;
         case OP_ZE + M_FLOAT:     p1 = "ZE[F]     ";             break;
         case OP_NZ + M_FLOAT:     p1 = "NZ[F]     ";             break;
         case OP_ADD + M_WORD:     p1 = "ADD[W]    ";             break;
         case OP_SUB + M_WORD:     p1 = "SUB[W]    ";             break;
         case OP_MUL + M_WORD:     p1 = "MUL[W]    ";             break;
         case OP_DIV + M_WORD:     p1 = "DIV[W]    ";             break;
         case OP_MOD + M_WORD:     p1 = "MOD[W]    ";             break;
         case OP_NEG + M_WORD:     p1 = "NEG[W]    ";             break;
         case OP_ADD + M_FLOAT:    p1 = "ADD[F]    ";             break;
         case OP_SUB + M_FLOAT:    p1 = "SUB[F]    ";             break;
         case OP_MUL + M_FLOAT:    p1 = "MUL[F]    ";             break;
         case OP_DIV + M_FLOAT:    p1 = "DIV[F]    ";             break;
         case OP_MOD + M_FLOAT:    p1 = "MOD[F]    ";             break;
         case OP_NEG + M_FLOAT:    p1 = "NEG[F]    ";             break;
         case OP_SHR:              p1 = "SHR       ";             break;
         case OP_SHL:              p1 = "SHL       ";             break;
         case OP_AND:              p1 = "AND       ";             break;
         case OP_XOR:              p1 = "XOR       ";             break;
         case OP_OR:               p1 = "OR        ";             break;
         case OP_NOT:              p1 = "NOT       ";             break;
         case OP_LAND + M_WORD:    p1 = "LAND[W]   ";             break;
         case OP_LOR  + M_WORD:    p1 = "LOR[W]    ";             break;
         case OP_LNOT + M_WORD:    p1 = "LNOT[W]   ";             break;
         case OP_LAND + M_FLOAT:   p1 = "LAND[F]   ";             break;
         case OP_LOR  + M_FLOAT:   p1 = "LOR[F]    ";             break;
         case OP_LNOT + M_FLOAT:   p1 = "LNOT[F]   ";             break;
         case OP_SHL1:             p1 = "SHL1      ";             break;
         case OP_SHL2:             p1 = "SHL2      ";             break;
         case OP_SHL3:             p1 = "SHL3      ";             break;

         default:                  p1 = "?????     ";             break;  
         }
      PrintOp (iAddr, uOpLen, p1, sz1);
      iIP += uOpLen;
      }
   }




/*
 *
 *
 */
void DumpGlobals (PSZ pszGlobals, UINT uSize)
   {
   UINT i, j;

   printf ("-------------------------------------------------------------------------\n");
   printf ("Globals Size: %d\n", uSize);
   printf ("-------------------------------------------------------------------------\n");
   printf ("------0--1--2--3--4--5--6--7--8--9--a--b--c--d--e--f-----0123456789abcdef\n");

   for (i=0; i<uSize; i += 16)
      {
      printf ("%4.4x: ", i);
      for (j=0; j<16; j++)
         {
         if (i+j >= uSize)
            printf ("   ");
         else
            printf ("%2.2x ", pszGlobals [i+j]);
         }
      printf ("   ");
      for (j=0; j<16; j++)
         {
         if (i+j >= uSize)
            break;
         if (pszGlobals [i+j] < 32)
            printf (".");
         else
            printf ("%c", pszGlobals [i+j]);
         }
      printf ("\n");
      }
   }


/*
 *
 *
 */
int CCONV main (int argc, char *argv[])
   {
   PSZ p;

   ArgBuildBlk ("? *^Help");

   if (ArgFillBlk (argv))
      Error (ArgGetErr (), 0, 0);

   if (ArgIs ("help") || ArgIs ("?") || !ArgIs(NULL))
      Usage ();

   strcpy (szINFILE, ArgGet (NULL, 0));
   if (!(p = strrchr (szINFILE, '\\')))
      p = szINFILE;
   if (!strrchr (p, '.'))
      strcat (szINFILE, EXT_EXE);

   plSTACK = calloc (STACK_SIZE, sizeof (LONG));
   LoadProgram (szINFILE);
   return 0;
   }


