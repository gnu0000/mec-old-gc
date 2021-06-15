/*
 *
 * extern.c
 * Sunday, 3/9/1997.
 * Craig Fitzgerald
 *
 * This module contains an API to store external functions callable from 
 * gc src.  There are 2 types of external functions:
 * C functions and vm environment functions
 *
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
#include <GnuMisc.h>
#include <GnuFile.h>
#include <GnuArg.h>
#include <GnuScr.h>
#include <GnuWin.h>
#include <Gnukbd.h>
#include "gclib.h"
#include "gc.h"
#include "opcodes.h"
#include "binfile.h"
#include "internal.h"
#include "codmaint.h"
#include "type.h"
#include "vm.h"

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

typedef void (CCONV *INPROC) (void);

typedef struct _fnod
   {
   PVOID  pfn;
   USHORT uRetType;
   USHORT uTag;

   struct _fnod *left;
   struct _fnod *right;
   } FNOD;
typedef FNOD *PFNOD;


static PFNOD pEXTREE = NULL;


static PFNOD _addFn (PFNOD pTree, PFNOD pfnod, BOOL bOrder)
   {
   if (!pTree)
      return pfnod;

   if (bOrder && pfnod->uTag == pTree->uTag)
      {
      pTree->pfn   = pfnod->pfn     ;
      pTree->uRetType = pfnod->uRetType;
      free (pfnod);
      }
   else if ((bOrder && (pfnod->uTag < pTree->uTag)) || (!bOrder && rand() % 2))
      pTree->left = _addFn (pTree->left, pfnod, bOrder);
   else
      pTree->right = _addFn (pTree->right, pfnod, bOrder);
   return pTree;
   }



void AddFn (PVOID pFn, UINT uRetType, UINT uTag)
   {
   PFNOD pfnod;

   pfnod = calloc (1, sizeof (FNOD));
   pfnod->pfn   = pFn;

   if (uRetType == DATATYPE_INT)
      uRetType = IntTyp ();

   pfnod->uRetType = uRetType;
   pfnod->uTag  = uTag;

   pEXTREE = _addFn (pEXTREE, pfnod, FALSE);
   }


/*
 * this must be LV R or RVL
 * a LRV or VLR will result in an unbalanced tree
 * due to its partially sorted nature
 */
static void _orderFn (PFNOD pTree)
   {
   PFNOD pRight;

   if (!pTree)
      return;

   _orderFn (pTree->left);

   pRight = pTree->right;
   pTree->left = pTree->right = NULL;
   pEXTREE = _addFn (pEXTREE, pTree, TRUE);

   _orderFn (pRight);
   }


void OrderFn (void)
   {
   PFNOD pfnod;

   pfnod   = pEXTREE;
   pEXTREE = NULL;
   _orderFn (pfnod);
   }


static PFNOD _findFn (PFNOD pTree, UINT uTag)
   {
   if (!pTree)
      return NULL;

   if (uTag == pTree->uTag)
      return pTree;
   else if (uTag < pTree->uTag)
      return _findFn (pTree->left, uTag);
   else
      return _findFn (pTree->right, uTag);
   }


static PVOID FindFn (UINT uTag)
   {
   return _findFn (pEXTREE, uTag);
   }



/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#if defined (__386__)

PVOID pGFN = NULL;

/*
 * interface between gc and c run time
 * gc stack is unwound and pushed onto c stack
 * c function is called
 * c stack is unwound and results put in gc stack
 *
 * The 32 bit run tome executable must be compiled using
 * stack based calling convention (not reg based).  For watcom
 * this means using -3s rather than -3r
 *
 * Watcom does not support calls from a stack var (IE no BP indexes 
 * for calls) hence the need for the pGFN global.  Note that it needs
 * the = NULL initializer to compile correctly!
 */
static void C_Call (PVOID pfn, UINT uRetType)
   {
   ULONG  w, lTypes;
   SHORT  i, iParms, iSpace, iGnuSize, iGStkIdx, iTyp;
   SHORT  iIntelSize = 0; 

   w = plSTACK[iSP-2]; // parm status#1 data
   iParms = (UINT)(w & 0xFF);
   iSpace = (UINT)((w >> 16) & 0xFF);
   iGStkIdx = iSP-3-iSpace; 

   lTypes = plSTACK[iSP-3]; // parm status#2 data
   iIntelSize=iGnuSize=0;
   for (i=iParms; i; i--)
      {

      iTyp = (UINT)((lTypes >> (i-1)*3) & 0x07);
      w = plSTACK[iGStkIdx];

      switch (iTyp)
         {
         case 0x04: // ptr
            w = (LONG)VMFixPtr (w);
            _asm {mov  eax, w}
            _asm {push eax}
            iIntelSize += 4;
            iGStkIdx   += 1;
            break;

         case M_BYTE:
         case M_SHORT:
         case M_WORD:
         case 0x05: // int
            _asm {mov  eax, w}
            _asm {push eax}
            iIntelSize += 4;
            iGStkIdx   += 1;
            break;

         case M_FLOAT:
            w = plSTACK[iGStkIdx+1];
            _asm {mov  eax, w}
            _asm {push eax}
            w = plSTACK[iGStkIdx+1];
            _asm {mov  eax, w}
            _asm {push eax}
            iIntelSize += 8;
            iGStkIdx   += 2;
            break;
         }
      }
   pGFN = pfn;

   _asm {call pGFN};
   _asm {add sp, iIntelSize};
   _asm {mov w, eax};

   i = (INT)plSTACK[iSP-1];

   switch (uRetType)
      {
      case DATATYPE_CHAR : 
         w &= 0xFF;   
         break;

      case DATATYPE_SHORT: 
         w &= 0xFFFF; 
         break;

      case DATATYPE_FLOAT:
         Error ("Float return not implemented!");
         break;
      }
   plSTACK[iSP+i+1] = w;
   }

#else

/*
 * interface between gc and c run time
 * gc stack is unwound and pushed onto c stack
 * c function is called
 * c stack is unwound and results put in gc stack
 */
static void C_Call (PVOID pfn, UINT uRetType)
   {
   ULONG  w, ulRetVal;
   SHORT  i, iParms, iSpace, iGnuSize, iGStkIdx, iTyp;
   USHORT i1, i2;
   SHORT  iIntelSize = 0; 
   PMONOT pmt;

   w = plSTACK[iSP-2]; // parm status#1 data
   iParms = (UINT)(w & 0xFF);
   iSpace = (UINT)((w >> 16) & 0xFF);
   iGStkIdx = iSP-3-iSpace; 

   w = plSTACK[iSP-3]; // parm status#2 data
   iIntelSize=iGnuSize=0;
   for (i=iParms; i; i--)
      {
      iTyp = (UINT)((w >> (i-1)*3) & 0x07);
      pmt = (PVOID)(plSTACK + iGStkIdx);

      switch (iTyp)
         {
         case M_BYTE:
         case M_SHORT:
         case 0x05: // int
            i1 = pmt->u1;
            _asm {push i1};
            iIntelSize += 2;
            break;

         case M_WORD:
            i1 = pmt->u1;
            i2 = pmt->u2;
            _asm {push i2};
            _asm {push i1};
            iIntelSize += 4;
            break;

         case 0x04: // ptr
            pmt->psz = VMFixPtr (pmt->ul);

            i1 = pmt->u1;
            i2 = pmt->u2;
            _asm {push i2};
            _asm {push i1};
            iIntelSize += 4;
            break;

         case M_FLOAT:
            i1 = pmt[1].u1;
            i2 = pmt[1].u2;
            _asm {push i2};
            _asm {push i1};
            i1 = pmt[0].u1;
            i2 = pmt[0].u2;
            _asm {push i2};
            _asm {push i1};
            iIntelSize += 8;
            break;
         }
      iGStkIdx += (iTyp == M_FLOAT ? 2 : 1);
      }
   _asm {call pfn};
   _asm {add sp, iIntelSize};
   _asm {mov i1, ax};
   _asm {mov i2, dx};

   i = (INT)plSTACK[iSP-1];

   switch (uRetType)
      {
      case DATATYPE_CHAR:
         ulRetVal = (ULONG)i1;
         break;

      case DATATYPE_SHORT:
         ulRetVal = (LONG)((SHORT)i1);
         break;

      case DATATYPE_FLOAT:
         Error ("Float return not implemented!");
         break;

      default:
         ulRetVal = MAKEULONG (i1, i2);
         break;
      }
   plSTACK[iSP+i+1] = ulRetVal;
   }

#endif

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


BOOL HandleInternalCall (UINT uTag)
   {
   PFNOD pfnod;

   if (!(pfnod = FindFn (uTag)))
         Error ("Internal Function not implemented: %d", uTag);

   if (uTag >= 32000) // special GNU functions
      ((INPROC)(pfnod->pfn))();
   else
      C_Call (pfnod->pfn, pfnod->uRetType); // C functions
   return TRUE;
   }

