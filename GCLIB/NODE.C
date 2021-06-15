/*
 *
 * node.c
 * Monday, 3/17/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include "gclib.h"
#include "gc.h"
#include "tokenize.h"
#include "error.h"
#include "opcodes.h"
#include "type.h"
#include "parse.h"


PNODE _newNode (void)
   {
   return (PNODE) calloc (sizeof (NODE), 1);
   }


/*
 * uses current token
 */
PNODE NodeBuild (void)
   {
   PNODE pnode;

   pnode          = _newNode ();
   pnode->uID     = Token.uID;
   pnode->ps      = Token.ps;
   pnode->val     = Token.val;

   pnode->pszFile = Token.pszFile;
   pnode->uLine   = Token.uLine;

   return pnode;
   }


/*
 * uID must be a reserved token!
 */
PNODE NodeBuild4 (UINT uID, PNODE pn1, PNODE pn2, PNODE pn3, PNODE pn4)
   {
   PNODE pnode;

   pnode          = _newNode ();
   pnode->uID = uID;
   pnode->pn1 = pn1;
   pnode->pn2 = pn2;
   pnode->pn3 = pn3;
   pnode->pn4 = pn4;
   return pnode;
   }

PNODE NodeBuild3 (UINT uID, PNODE pn1, PNODE pn2, PNODE pn3)
   {
   return NodeBuild4 (uID,  pn1, pn2,  pn3,  NULL);
   }

PNODE NodeBuild2 (UINT uID, PNODE pn1, PNODE pn2)
   {
   return NodeBuild4 (uID,  pn1, pn2,  NULL, NULL);
   }

PNODE NodeBuild1 (UINT uID, PNODE pn1)
   {
   return NodeBuild4 (uID,  pn1, NULL, NULL, NULL);
   }



/*
 * adds pnEntry to the end of pnList list
 *
 */
PNODE NodeAddToEnd (PNODE pnList, PNODE pnEntry)
   {
   PNODE pn1, pnEnd = NULL;

   for (pn1 = pnList; pn1; pn1 = pn1->next)
      pnEnd = pn1;

   if (!pnEnd)
      return pnEntry;
   pnEnd->next = pnEntry;
   return pnList;
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

PNODE NodeFree (PNODE pnode)
   {
   if (!pnode)
      return NULL;

//
// no - string literals must stick around until GenGlobals call
//
//   if (pnode->uID == TOK_STRINGLITERAL && pnode->val.psz)
//      MemFreeData (pnode->val.psz);

   return (PNODE) MemFreeData (pnode);
   }


PNODE NodeFreeTree (PNODE pnode)
   {

   PNODE pn1, pn2;

   if (!pnode)
      return NULL;

   for (pn1 = pnode; pn1; pn1 = pn2)
      {
      pn2 = pn1->next;
      NodeFreeTree (pn1->pn1);
      NodeFreeTree (pn1->pn2);
      NodeFreeTree (pn1->pn3);
      NodeFreeTree (pn1->pn4);
      NodeFree (pn1);
      }
   return NULL;
   }


//PNODE PNodeTree (PNODE pnode)
//   {
//
//   PNODE pn1, pn2;
//
//   if (!pnode)
//      return NULL;
//
//   for (pn1 = pnode; pn1; pn1 = pn2)
//      {
//      pn2 = pn1->next;
//      PNodeTree (pn1->pn1);
//      PNodeTree (pn1->pn2);
//      PNodeTree (pn1->pn3);
//      PNodeTree (pn1->pn4);
//
//      printf ("PNT: %2.2d %p: %p %p %p %p   %p\n", pn1->uID, pn1, 
//            pn1->pn1, pn1->pn2, pn1->pn3, pn1->pn4, pn1->next);
//      }
//   return NULL;
//   }





PNODE NodeBuildLit (UINT uType)
   {
   PNODE p1;
   UINT  uTok;

   switch (uType)
      {
      case DATATYPE_CHAR  : uTok = TOK_CHARLITERAL  ; break;
      case DATATYPE_SHORT : uTok = TOK_SHORTLITERAL ; break;
      case DATATYPE_LONG  : uTok = TOK_LONGLITERAL  ; break;
      case DATATYPE_FLOAT : uTok = TOK_FLOATLITERAL ; break;
      case DATATYPE_STRING: uTok = TOK_STRINGLITERAL; break;
      case DATATYPE_PTR   : uTok = TOK_LONGLITERAL  ; break;
      default : Error ("Bad type in Build Literal [%d]", uType);
      }
   p1 = NodeBuild1 (uTok, NULL);
   p1->uTypeVal = uType;
   return NodeBuild1 (TOK_LITERAL, p1);
   }
