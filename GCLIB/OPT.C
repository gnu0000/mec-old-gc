/*
 *
 * opt.c
 * Monday, 3/17/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
//#include <assert.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "tokenize.h"
#include "parse.h"
#include "check.h"
#include "error.h"
#include "type.h"
#include "opcodes.h"




PNODE OptChangeLit (PNODE pnode, UINT uOP, UINT uVal)
   {
   PNODE pLit;

//   assert (pnode->uID == TOK_LITERAL);
   pLit = pnode->pn1;
   if (TypIntegral (pLit->uTypeVal))
      {
      switch (uOP)
         {
         case OP_ADD: pLit->val.l += uVal; break;
         case OP_SUB: pLit->val.l -= uVal; break;
         case OP_MUL: pLit->val.l *= uVal; break;
         case OP_DIV: pLit->val.l /= uVal; break;
         case OP_MOD: pLit->val.l %= uVal; break;
         case OP_NEG: pLit->val.l = -pLit->val.l; break;
         default: NodeError (pnode, "internal error: op not supported in OptChangeLit [%d]", uOP);
         }
      pLit->uTypeVal = DATATYPE_LONG;
      }
   if (TypFloat (pLit->uTypeVal))
      {
      switch (uOP)
         {
         case OP_ADD: pLit->val.bg += (BIG)uVal; break;
         case OP_SUB: pLit->val.bg -= (BIG)uVal; break;
         case OP_MUL: pLit->val.bg *= (BIG)uVal; break;
         case OP_DIV: pLit->val.bg /= (BIG)uVal; break;
         case OP_NEG: pLit->val.bg = -pLit->val.bg; break;
         case OP_MOD: 
         default: NodeError (pnode, "internal error: op not supported in OptChangeLit [%d]", uOP);
         }
      }
   return pnode;
   }




//
//
//
//
//
////   /*--- optimize if we can ---*/
////   if (pnode->uID == TOK_LITERAL && TypIntegral (pnode->pn1->uTypeVal)) 
////      {
////      return OptChangeLit (pnode->pn1, TOK_MUL, uSize)
////      }
//
//
//
//
//
//
//#define PRUNE_LEFT  0
//#define PRUNE_RIGHT 1
//
//BOOL IsParticularVal (PNODE pnode, UINT u)
//   {
//   if (!pnode || pnode->uID != TOK_LITERAL)
//      return FALSE;
//
//   switch (pnode->pn1->uID)
//      {
//      case TOK_CHARLITERAL  : 
//      case TOK_INTLITERAL   : 
//         return pnode->pn1->val.i  == u;
//
//      case TOK_FLOATLITERAL : 
//         return pnode->pn1->val.bg == (BIG)u;
//      }
//   return FALSE;
//   }
//
//BOOL IsLiteralNum (PNODE pnode)
//   {
//   return (pnode && 
//           pnode->uID == TOK_LITERAL && 
//           pnode->pn1->uID != TOK_STRINGLITERAL);
//   }
//
//
//PNODE Prune (PNODE pnode, UINT uPrune)
//   {
//   pTmp = pnode;
//
//   if (uPrune == PRUNE_LEFT)
//      {
//      pTmp = pnode->pn2;
//      pnode->pn2 = NULL;
//      NodeFree (pnode);
//      return pTmp;
//      }
//   else // (PRUNE_RIGHT)
//      {
//      pTmp = pnode->pn1;
//      pnode->pn1 = NULL;
//      NodeFree (pnode);
//      return pTmp;
//      }
//   }
//
//
//
///*
// * 0+e e+0  -> e
// * e-0      -> e
// * 0-e      -> -1
// */
//PNODE CheckAdd (PNODE pExpr)
//   {
//   if (pExpr->uID == TOK_PLUS)
//      {
//      if (IsParticularVal ((*ppExpr)->pn1, 0))
//         pExpr = Prune (pExpr, PRUNE_LEFT);
//      else if (IsParticularVal (pExpr->pn2, 0))
//         pExpr = Prune (pExpr, PRUNE_RIGHT);
//      }
//
//   if (pExpr->uID == TOK_MINUS && pExpr->pn2) // and not a unary
//      {
//      if (IsParticularVal ((*ppExpr)->pn1, 0))
//         {
//         NodeFree (pExpr->pn1);   // turn 0 - e
//         pExpr->pn1 = pExpr->pn2; // into a unary minus op
//         pExpr->pn2 = NULL;       //
//         }
//      else if (IsParticularVal (pExpr->pn2, 0))
//         {
//         pExpr = Prune (pExpr, PRUNE_RIGHT);
//         }
//      }
//   return pExpr;
//   }
//
///*
// * 1*e e*1  -> e
// * e/1      -> e
// */
//PNODE CheckMultiply (PNODE pExpr)
//   {
//   if (pExpr->uID == TOK_STAR)
//      {
//      if (IsParticularVal ((*ppExpr)->pn1, 1))
//         pExpr = Prune (pExpr, PRUNE_LEFT);
//      else if (IsParticularVal (pExpr->pn2, 1))
//         pExpr = Prune (pExpr, PRUNE_RIGHT);
//      }
//   if (pExpr->uID == TOK_SLASH)
//      {
//      if (IsParticularVal (pExpr->pn2, 1))
//         pExpr = Prune (pExpr, PRUNE_RIGHT);
//      }
//
//   }
//
//PNODE CheckUnaryMinus (PNODE pExpr)
//   {
//   if (pExpr->uID == TOK_MINUS)
//   }
//
//
//
//PNODE CheckLiteralConversions (PNODE pExpr)
//   {
//
//   }
//

/*
 * This function performs logical reductions on an
 * expression parse tree
 *
 * Most of the reductions involve constant rollups
 * and a few identity reductions
 *
 * literal type conv
 *
 * 0+e e+0  -> e
 * e-0      -> e
 * 0-e      -> -e
 *
 * 1*e e*1  -> e
 * e/1      -> e
 *
 * i*2      -> i<<SHL1
 * i*4      -> i<<SHL2
 * i*8      -> i<<SHL3
 *
 *
 * - c      -> -c
 *
 * c op c   -> ceval
 *
 * e/c      -> e*(1/c)
 *
 */
PNODE OptCollapseExpr (PNODE pExpr)
   {
//   OptCollapseExpr (pExpr->pn1);
//   OptCollapseExpr (pExpr->pn2);

//   pExpr = CheckLiteralConversions (pExpr);
//   pExpr = CheckAdd (pExpr);
//   pExpr = CheckMultiply (pExpr);
//   pExpr = CheckUnaryMinus (pExpr);
//   pExpr = CheckConstOperations (pExpr);
//   pExpr = CheckConstDivision (pExpr);
   return pExpr;
   }



