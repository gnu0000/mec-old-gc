/*
 *
 * internal.c
 * Sunday, 3/9/1997.
 * Craig Fitzgerald
 *
 * This module includes functions that are callable from gc code
 * that are external to gc src but part of the gc runtime
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <GnuType.h>
#include <GnuMem.h>
#include "gclib.h"
#include "gc.h"
#include "opcodes.h"
#include "codmaint.h"
#include "vm.h"
#include "extern.h"


typedef union
   {
   PSZ     psz;
   ULONG   ul;

   struct
      {
      USHORT u1;
      USHORT u2;
      };
   } MONOT;
typedef MONOT *PMONOT;


/*
 * int DumpStk (int i);
 * dumps i bytes from the stack
 */
void _cdecl GNU_DumpStack (void)
   {
   INT i, iMax;

   printf ("-----------------------[Stack Dump]-----------------------\n");
   printf ("iSF=%4.4x  iSP=%4.4x\n", iSF, iSP);

   iMax = (INT)VMSPeekIn (3);
   for (i=0; i<iMax; i++)
      {
      if (iSP <= i)
         break;
      printf ("[%4.4x] %p (%ld)\n", iSP-i+1, VMSPeekIn (i), VMSPeekIn (i));
      }
   printf ("----------------------------------------------------------\n");
   }



void _cdecl GNU_SetPtr (void)
   {
   INT  iRetOff;

   iRetOff = (INT)VMSPeekIn (0);             // return loc relative to StackWord fn
   plSTACK[iSP+iRetOff+1] = (LONG)pfnCURR;    // save parm val ro return loc
   }


/*
 * int DumpMem (int addr, int len)
 *
 */
void _cdecl GNU_DumpMem (void)
   {
   ULONG j, ul, ulAddr, ulLen;
   PSZ   psz;

   ulAddr = VMSPeekIn (3);
   ulLen  = VMSPeekIn (4);
   ul     = ulAddr - ulAddr % 16;

   printf ("Dump: ADDR:%p  LEN:%ld\n", ulAddr, ulLen);
   printf ("------------------------------------\n");
   while (TRUE)
      {
      printf ("%p: ", ul);
      for (j=0; j<16; j++)
         {
         psz = (PSZ)(ul+j);
         if (ul+j < ulAddr || ul+j > ulAddr+ulLen)
            printf ("   ");
         else
            printf ("%2.2x ", (int)*psz);
         }
      printf ("   ");
      for (j=0; j<16; j++)
         {
         psz = (PSZ)(ul+j);
         if (ul+j < ulAddr || ul+j > ulAddr+ulLen)
            printf (" ");
         else if (*psz < 32)
            printf (".");
         else
            printf ("%c", (int)*psz);
         }
      printf ("\n");
      ul+= 16;
      if (ul > ulAddr+ulLen)
         return;
      }
   }


/*
 * int ParmCount    (void): 7;
 * returns the number of params passed to the fn
 */
void _cdecl GNU_ParmCount (void)
   {
   LONG lRetVal;
   INT  iRetOff;

   lRetVal = plSTACK[iSF-3]; // parm status data relative to parent fn
   lRetVal &= 0xFF;

   iRetOff = (INT)VMSPeekIn (0);     // return loc relative to StackWord fn
   plSTACK[iSP+iRetOff+1] = lRetVal;  // save parm val ro return loc

   }


INT ParmIndex (INT iParm)
   {
   ULONG ulStat2;
   INT  i, iSize;

   ulStat2 = plSTACK[iSF-4];

   for (iSize=i=0; i<=iParm; i++, iSize++)
      {
      if (i<10 && ((ulStat2>>(i*3)) & 0x07) == M_FLOAT) // is parm #i a float?
         iSize++;
      }

   return iSF-4-iSize;
   }


/*
 * int ParmType    (int    i): 8;
 * returns the (0 based) i'th parameter type
 *
 */
void _cdecl GNU_ParmType (void)
   {
   LONG lRetVal;
   INT  iIdx, iRetOff;

   iIdx = (INT)VMSPeekIn (3);    // parm # relative to StackWord fn call

   lRetVal = plSTACK[iSF-4]; // parm status data relative to parent fn
   lRetVal = (lRetVal >> iIdx*2) & 0x03;

   iRetOff = (INT)VMSPeekIn (0); // return loc relative to StackWord fn

   plSTACK[iSP+iRetOff+1] = lRetVal;  // save parm val ro return loc
   }


/*
 * int ParmWord    (int    i): 6;
 * returns the (0 based) i'th parameter to the current function
 *
 */
void _cdecl GNU_ParmWord (void)
   {
   LONG lRetVal;
   INT  iIdx, iRetOff;

   iIdx = (INT)VMSPeekIn (3);        // parm # relative to StackWord fn call

   lRetVal = plSTACK[ParmIndex(iIdx)];// parm data relative to parent fn

   iRetOff = (INT)VMSPeekIn (0);     // return loc relative to StackWord fn
   plSTACK[iSP+iRetOff+1] = lRetVal;  // save parm val ro return loc
   }


/*
 * int ParmWord    (int    i): 6;
 * returns the (0 based) i'th parameter to the current function
 *
 */
void _cdecl GNU_ParmFloat (void)
   {
   LONG lRetVal;
   INT  iIdx, iRetOff, iParmLoc;

   iIdx = (INT)VMSPeekIn (3);        // parm # relative to StackWord fn call
   iParmLoc = ParmIndex(iIdx);

   iRetOff = (INT)VMSPeekIn (0);     // return loc relative to StackWord fn

   lRetVal = plSTACK[iParmLoc];       // parm data relative to parent fn
   plSTACK[iSP+iRetOff+1] = lRetVal;  // save parm val ro return loc

   lRetVal = plSTACK[iParmLoc+1];     // float is 2 words big
   plSTACK[iSP+iRetOff+2] = lRetVal;  // 

   }



void _cdecl GNU_CallByName (void)
   {
   LONG lOffset, lStat1, lStat2, lNewStat1, lFnStrAddr;
   PSZ  pszFn;

   lOffset    = VMSPop ();
   lStat1     = VMSPop ();
   lStat2     = VMSPop ();
   lFnStrAddr = VMSPop (); // were removing this parameter

   /*-- were going to remove param 1 from stack and references to it ---*/
   VMSPush (lStat2 >> 3);  // shift over param info

   lNewStat1 = (((lStat1      ) & 0xFF)-1)    | // dec aactuals count
               (((lStat1 >> 8 ) & 0xFF)-1)<<8 | // dec actuals count
               (((lStat1 >> 16) & 0xFF)-1)<<16; // dec parm space used
   VMSPush (lNewStat1);
   VMSPush (lOffset+1);    // return space is now 1 word closer

   pszFn = VMFixPtr(lFnStrAddr);

   VMExecFunction (pszFn);

   VMSPush (0); // caller is expecting lFnStrAddr to still be on the stack
   }


void _cdecl GNU_LoadModule (void)   
   {
   PSZ   pszModule;
   ULONG ulAddr;
   INT   iRetOff;
   LONG  lRet;

   ulAddr    = VMSPeekIn (3);
   pszModule = VMFixPtr(ulAddr);
   lRet      = (LONG)LoadGXModule (pszModule);

   iRetOff = (INT)VMSPeekIn (0);     // return loc relative to StackWord fn
   plSTACK[iSP+iRetOff+1] = lRet;  // save parm val ro return loc
   }


void _cdecl GNU_FunctionExists (void)
   {
   PSZ   pszModule;
   ULONG ulAddr;
   INT   iRetOff;
   LONG  lRet;

   ulAddr    = VMSPeekIn (3);
   pszModule = VMFixPtr(ulAddr);
   lRet = !!CodFindFunction (pszModule);
   iRetOff = (INT)VMSPeekIn (0);     // return loc relative to StackWord fn
   plSTACK[iSP+iRetOff+1] = lRet;  // save parm val ro return loc
   }


void InitInternalFunctions (void)
   {
   AddFn (GNU_DumpStack,      DATATYPE_INT, 32000);
   AddFn (GNU_DumpMem,        DATATYPE_INT, 32001);
   AddFn (GNU_ParmCount,      DATATYPE_INT, 32002);
   AddFn (GNU_ParmType,       DATATYPE_INT, 32003);
   AddFn (GNU_ParmWord,       DATATYPE_INT, 32004);
   AddFn (GNU_ParmFloat,      DATATYPE_INT, 32005);
   AddFn (GNU_CallByName,     DATATYPE_INT, 32006);
   AddFn (GNU_LoadModule,     DATATYPE_INT, 32007);
   AddFn (GNU_FunctionExists, DATATYPE_INT, 32008);

   AddFn (GNU_SetPtr,     4, 32700);
   OrderFn ();
   }
