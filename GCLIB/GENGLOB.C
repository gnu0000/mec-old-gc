/*
 *
 * genglob.c
 * Thursday, 2/27/1997.
 * Craig Fitzgerald
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include <GnuFile.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "tokenize.h"
#include "parse.h"
#include "check.h"
#include "label.h"
#include "genglob.h"
#include "gen.h"
#include "genop.h"
#include "opcodes.h"
#include "binfile.h"
#include "type.h"
#include "error.h"
#include "mem.h"
#include "node.h"


typedef struct _glob
   {
   PNODE pnode;         // ptr to symbol entry if a global NULL if a static
   PSZ   psz;           // ptr to buffer if a static
   UINT  uAddr;         // offset into global buffer
   UINT  uLen;          // data size
   struct _glob *next;  // next ptr
   struct _glob *prev;  // next ptr
   } GLOB;
typedef GLOB *PGLOB;


static INT   iGP = 0;        // Global Area current size Index
static PGLOB pglHEAD = NULL; // global vars and static data linked list head
static PGLOB pglTAIL = NULL; // global vars and static data linked list tail


/***************************************************************************/
/*                                                                         */
/* Global varibles and Static values store/write routines                  */
/*                                                                         */
/***************************************************************************/

static PGLOB FreeGLOB (PGLOB pgl)
   {
   if (!pgl)
      return NULL;

   if (pgl->pnode)
      pgl->pnode = NodeFreeTree (pgl->pnode);
   else
      MemFreeData (pgl->psz);     // a static

   return MemFreeData (pgl);
   }

static PGLOB NewGlob (void)
   {
   PGLOB pglob;

   pglob = calloc (sizeof (GLOB), 1);
   pglob->uAddr = iGP;
   pglob->prev  = pglTAIL;
   pglTAIL      = pglob;

   if (pglob->prev)
      pglob->prev->next = pglob;
   return pglob;
   }





/*
 * This fn adds a static buffer to the global memory list
 *
 * Memory pointers are in the form 0x0000XXXX or 0xXXXXXXXX.  If the
 * 2 high bytes are zero, the ptr is assumed to be an index into the
 * global memory area.  This could lead to a problem for the first
 * entry in the global area because it's address would be 0 which is
 * the same as NULL.  So if the first entry is a static (not a global
 * var), we must add a dummy first so that its address is offset
 * hence the if (!iGP) condition
 *
 */
UINT AddStatic (PSZ pszBuff, UINT uLen)
   {
   PGLOB pglob;

   if (!iGP)
      {
      pglob = NewGlob ();
      pglob->psz   = strdup ("\x44\x44\x44\x44");
      pglob->uLen  = iGP = 4;
      pglHEAD = pglob;
      }
   pglob = NewGlob ();
   pglob->psz  = pszBuff;
   pglob->uLen = uLen;
   iGP += uLen;
   return pglob->uAddr;
   }



/*
 * This fn adds a global variable to the global memory list
 * Note that if the global var is of type string it will also
 * have a static component immediately following it
 *
 * a global has 3 types of initializers:
 *
 * simple : char c    ='A';      - initializer put in var's memspace
 * array  : char c[9] = "hello"; - initializer put in var's memspace
 * static : char *c   = "hello"; - static created, addr of static put in
 *                                 var's memspace
 */
static void AddGlobal (PNODE pnode)
   {
   PNODE pIdent, pInit, pExpr;
   PGLOB pglob;
   PSYM  ps;
   PSZ   psz;

   pIdent = pnode->pn1;
   pInit  = pnode->pn2;

   ps = pIdent->ps;

   /*--- if first global entry is a pointer, make a dummy first ---*/
   if (!iGP && ps->uTypeDeref)
      {
      pglob = NewGlob ();
      pglob->psz   = strdup ("\x44\x44\x44\x44");
      pglob->uLen  = iGP = 4;
      pglHEAD = pglob;
      }

   pglob = NewGlob ();
   pglob->pnode = pnode;

   ps->iAddr    = iGP;
   pglob->uLen  = MemSymSize (ps);
   iGP += pglob->uLen;
   if (!pglHEAD)
      pglHEAD = pglob;

   if (!pInit || !ps->uTypeDeref)
      return;

   /*--- char *sz = "Hello there cutie!"; ---*/
   if (!ps->bArray && pInit->pn1->pn1->uID == TOK_STRINGLITERAL)
      {
      psz = pInit->pn1->pn1->val.psz;
      pInit->val.l = AddStatic (psz, strlen (psz) + 1);
      return;
      }

   /*--- char *sz[] = {"Hello", "There", "Cutie!"}; ---*/
   if (ps->bArray && ps->uTypeDeref == 2)
      {
      for (pExpr = pInit; pExpr; pExpr = pExpr->next)
         {
         psz = pExpr->pn1->pn1->val.psz;
         if (pInit->pn1->pn1->uID == TOK_STRINGLITERAL)
            pExpr->val.l = AddStatic (psz, strlen (psz) + 1);
         }
      }
   }


/*
 * this actually stores them until
 * GenGlobals is called
 *
 */
void StoreGlobalDecl (PNODE pVar)
   {
   PNODE pnode, pnodeNext;

   for (pnode = pVar; pnode; pnode = pnodeNext)
      {
      pnodeNext = pnode->next; // snip chains
      pnode->next = NULL;      //
      AddGlobal (pnode);       //
      }
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

static void WriteFiller (UINT uBytes)
   {
   CHAR c0 = '\0';

   for (; uBytes; uBytes--)
      GenDumpGlobData (&c0, 1);
   }


static UINT WriteElement (PNODE pExpr, UINT uTypeVal, PSYM ps, BOOL bStringGlob)
   {
   PNODE pVal;

   if (!pExpr)
      {
      WriteFiller (MemSize (uTypeVal, ps->psStruct));
      return 1;
      }

   pVal = pExpr->pn1;
   while (pVal->uID == TOK_CAST)
      pVal = pVal->pn2;

   assert (pVal->uID == TOK_LITERAL);
   pVal = pVal->pn1;

   switch (uTypeVal)
      {
      case DATATYPE_CHAR :
         if (bStringGlob && TypString (pVal->uTypeVal))
            {
            GenDumpGlobData (pVal->val.psz, strlen (pVal->val.psz)+1);
            return strlen (pVal->val.psz) + 1;
            }
         GenDumpGlobData (&(pVal->val.c), 1);
         return 1;

      case DATATYPE_SHORT:
         GenDumpGlobData (&(pVal->val.s), 2);
         return 1;

      case DATATYPE_LONG :
         GenDumpGlobData (&(pVal->val.l), 4);
         return 1;

      case DATATYPE_FLOAT:
         GenDumpGlobData  (&pVal->val.bg, 8);
         return 1;

      default:
         if (TypString (pVal->uTypeVal))
            GenDumpGlobData (&pExpr->val.l, 4); // static offset ptr
         else
            GenDumpGlobData (&pVal->val.l, 4);  // literal ptr address
         return 1;
      }
   }

  

static void GenGlobalElement (PGLOB pgl)
   {
   PNODE pInit;
   PSYM  ps;
   UINT  uSize, uElementType;

   ps    = pgl->pnode->pn1->ps;
   pInit = pgl->pnode->pn2;

   if (!ps->bArray) // not an array?
      {
      WriteElement (pInit, ps->uTypeVal, ps, 0);
      return;
      }
   uElementType = TypPtrBase (ps->uTypeVal);

   for (uSize=0; uSize<ps->uArraySize; )
      {
      uSize += WriteElement (pInit, uElementType, ps, 1);
      pInit = (pInit ? pInit->next : NULL);
      }
   }


static void GenStaticElement (PGLOB pgl)
   {
   GenDumpGlobData (pgl->psz, pgl->uLen);
   }


/*
 * this writes out the globals record
 *
 */
void GenGlobals (void)
   {
   PGLOB pgl, pglNext;

   GenDumpGlobStart (iGP);
   for (pgl = pglHEAD; pgl; pgl = pglNext)
      {
      pglNext = pgl->next;

      if (pgl->pnode) // a global variable
         GenGlobalElement (pgl);
      else
         GenStaticElement (pgl);

      FreeGLOB (pgl);
      }
   GenDumpGlobEnd ();
   }

