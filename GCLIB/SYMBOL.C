/*
 *
 * symbol.c
 * Tuesday, 2/25/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "error.h"
#include "type.h"


#define HASH_SIZE 257

PSYM ppsHASH [HASH_SIZE];

PRES pprHASH [HASH_SIZE];

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

static UINT HashValue (PSZ pszSymbol)
   {
   UINT i = 0;

   while (*pszSymbol)
      i = (i << 1) ^ *pszSymbol++;
   return (i % HASH_SIZE);
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

static PSYM NewSymbol (PSZ pszSym)
   {
   PSYM ps;

   ps = calloc (sizeof (SYM), 1);
   ps->pszLiteral = strdup (pszSym);

   return ps;
   }

static void FreeSymbol (PSYM ps)
   {
   if (ps->pszLiteral) 
      free (ps->pszLiteral);

   if (ps->uKind == KIND_FUNCTION && ps->pFormals)
      free (ps->pFormals);

   if (ps->uKind == KIND_STRUCTDEF && ps->ppElements)
      free (ps->ppElements);

   free (ps);
   }


PSYM SymAdd (PSZ pszSym, BOOL bAddAtEnd)
   {
   PSYM ps, psTmp;
   UINT uHashVal;

   uHashVal = HashValue (pszSym);
   ps = NewSymbol (pszSym);

   if (!bAddAtEnd)
      {
      ps->next = ppsHASH[uHashVal];
      ppsHASH[uHashVal] = ps;
      }
   else
      {
      for (psTmp = ppsHASH[uHashVal]; psTmp && psTmp->next; psTmp=psTmp->next)
         ;
      if (psTmp)
         psTmp->next = ps;
      else
         ppsHASH[uHashVal] = ps;
      }
   return ps;
   }


PSYM SymFind (PSZ pszSym)
   {
   PSYM ps;
   UINT uHashVal;

   uHashVal = HashValue (pszSym);
   for (ps = ppsHASH[uHashVal]; ps; ps = ps->next)
      if (!strcmp (pszSym, ps->pszLiteral))
         return ps;
   return NULL;
   }


BOOL SymDelete (PSZ pszSym)
   {
   PSYM ps, psPrev = NULL;
   UINT uHashVal;

   uHashVal = HashValue (pszSym);
   for (ps = ppsHASH[uHashVal]; ps; ps = ps->next)
      {
      if (strcmp (pszSym, ps->pszLiteral))
         {
         if (psPrev)
            psPrev->next = ps->next;
         else
            ppsHASH[uHashVal] = ps->next;
         FreeSymbol (ps);
         return TRUE;
         }
      psPrev = ps;
      }
   return FALSE;
   }


void SymDeleteLocals (void)
   {
   PSYM ps, psPrev, psNext;
   UINT i;

   for (i=0; i<HASH_SIZE; i++)
      {
      psPrev = NULL;
      for (ps = ppsHASH[i]; ps; ps = psNext)
         {
         psNext = ps->next;
         if (ps->uKind == KIND_VARIABLE && 
             (ps->uScope == SCOPE_LOCAL || ps->uScope == SCOPE_PARAM))
            {
            Log (16, "removing symbol %s\n", ps->pszLiteral);

            if (psPrev)
               psPrev->next = ps->next;
            else
               ppsHASH[i] = ps->next;
            FreeSymbol (ps);
            }
         else
            psPrev = ps;
         ps = psNext;
         }
      }
   }



/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


void SymAddReserved  (PRES pr)
   {
   UINT uHashVal;

   uHashVal = HashValue (pr->pszString);
   pr->next = pprHASH[uHashVal];
   pprHASH[uHashVal] = pr;
   }



PRES SymFindReserved (PSZ pszStr)
   {
   PRES pr;
   UINT uHashVal;

   uHashVal = HashValue (pszStr);
   for (pr = pprHASH[uHashVal]; pr; pr = pr->next)
      if (!strcmp (pszStr, pr->pszString))
         return pr;
   return NULL;
   }


void SymInit (void)
   {
   UINT i;

   for (i=0; i<HASH_SIZE; i++)
      {
      ppsHASH[i] = NULL;   
      pprHASH[i] = NULL;   
      }
   }


void SymTerm (void)
   {
   PSYM ps, psNext;
   UINT i;

   for (i=0; i<HASH_SIZE; i++)
      {
      for (ps = ppsHASH[i]; ps; ps = psNext)
         {
         psNext = ps->next;
         FreeSymbol (ps);
         }
      ppsHASH[i] = NULL;
      }
   }



/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void DumpDataTypeSymbol (PSYM ps)
   {
   printf ("symbol\n");
   }

void DumpFunctionSymbol (PSYM ps)
   {
   UINT u;

   u = ps->uTypeVal;
   printf ("%-16s   %s    %s    %4d\n",
           ps->pszLiteral,
           (u==DATATYPE_VOID      ? "DATATYPE_VOID  " :
            (u==DATATYPE_CHAR     ? "DATATYPE_CHAR  " :
             (u==DATATYPE_SHORT   ? "DATATYPE_SHORT " :
              (u==DATATYPE_LONG   ? "DATATYPE_LONG  " :
                (u==DATATYPE_FLOAT ? "DATATYPE_FLOAT " : "DATATYPE_STRING"))))),
           (ps->bDefined ? "DEFINED " : "DECLARED"),
           ps->iAddr);
   }

void DumpVariableSymbol (PSYM ps)
   {
   UINT u;

   u = ps->uTypeVal;
   printf ("%-16s   %s    %s    %4d\n",
           ps->pszLiteral,
           (u==DATATYPE_VOID      ? "DATATYPE_VOID  " :
            (u==DATATYPE_CHAR     ? "DATATYPE_CHAR  " :
             (u==DATATYPE_SHORT   ? "DATATYPE_SHORT " :
              (u==DATATYPE_LONG   ? "DATATYPE_LONG  " :
                (u==DATATYPE_FLOAT ? "DATATYPE_FLOAT " : "DATATYPE_STRING"))))),
 
           (ps->uScope==SCOPE_GLOBAL   ? "SCOPE_GLOBAL" :
            (ps->uScope==SCOPE_PARAM   ? "SCOPE_PARAM " :
             (ps->uScope==SCOPE_LOCAL  ? "SCOPE_LOCAL " :
              (ps->uScope==SCOPE_STRUCT ? "SCOPE_STRUCT" : "<none>      ")))),
           ps->iAddr);
   }

void SymDump (void)
   {
   PSYM ps;
   UINT i;

   printf ("*************************[Struct Def]*************************\n");
   for (i=0; i<HASH_SIZE; i++)
      for (ps = ppsHASH[i]; ps; ps = ps->next)
         if (ps->uKind == KIND_STRUCTDEF)
            DumpDataTypeSymbol (ps);

   printf ("*************************[Data Types]*************************\n");
   for (i=0; i<HASH_SIZE; i++)
      for (ps = ppsHASH[i]; ps; ps = ps->next)
         if (ps->uKind == KIND_TYPEDEF)
            DumpDataTypeSymbol (ps);

   printf ("*************************[Functions ]*************************\n");
   for (i=0; i<HASH_SIZE; i++)
      for (ps = ppsHASH[i]; ps; ps = ps->next)
         if (ps->uKind == KIND_FUNCTION)
            DumpFunctionSymbol (ps);

   printf ("*************************[Variables ]*************************\n");
   for (i=0; i<HASH_SIZE; i++)
      for (ps = ppsHASH[i]; ps; ps = ps->next)
         if (ps->uKind == KIND_VARIABLE)
            DumpVariableSymbol (ps);
   }
