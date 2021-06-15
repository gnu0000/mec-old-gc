/*
 *
 * type.c
 * Wednesday, 3/12/1997.
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
#include "symbol.h"
#include "tokenize.h"
#include "parse.h"
#include "check.h"
#include "error.h"
#include "opt.h"
#include "type.h"
#include "node.h"
#include "mem.h"



/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/*
 * deref partially set in parse
 */
void TypCombineVal (PSYM ps, UINT uTypeVal)
   {
   ps->uTypeBase   =  uTypeVal & 0x00FF;
   ps->uTypeDeref += (uTypeVal >> 8);
   }



/*
 * this takes a datatype subtree 
 * and builds a TypeVal from it
 */
UINT TypBuildVal (PNODE pnode)
   {
   switch (pnode->uID)
      {
      case TOK_VOID:   
         return DATATYPE_VOID;

      case TOK_CHARLITERAL:
      case TOK_CHAR:   
         return DATATYPE_CHAR;

      case TOK_SHORTLITERAL:
      case TOK_SHORT:  
         return DATATYPE_SHORT;

      case TOK_INTLITERAL:
      case TOK_INT:    
         return IntTyp ();

      case TOK_LONGLITERAL:
      case TOK_LONG:   
         return DATATYPE_LONG;

      case TOK_FLOATLITERAL:
      case TOK_FLOAT:  
         return DATATYPE_FLOAT;

      case TOK_STRINGLITERAL:
      case TOK_STRING: 
         return 0x0100 + DATATYPE_CHAR;

      case TOK_STRUCT: 
      case TOK_UNION: 
         return DATATYPE_STRUCT;

      case TOK_IDENTIFIER: // a defined type
         return pnode->ps->uTypeVal;

      default: NodeError (pnode, "internal: TypBuildVal [%d]", pnode->uID);
      }
   return 0;
   }


/*
 * null unless pType is struct or ptr to struct
 *
 */
PSYM StructTypePtr (PNODE pType)
   {
   /*--- struct type ---*/
   if (pType->uID == TOK_STRUCT || pType->uID == TOK_UNION)
      {
      return pType->pn1->ps;
      }

   if (pType->uID != TOK_IDENTIFIER)
      return NULL;

   /*--- typedef ident ---*/
   if (pType->ps->uKind != KIND_TYPEDEF)
      return NULL;

   return pType->ps->psStruct; // null unless a struct typedef
   }



void TypXfer (PSYM ps, PNODE pType)
   {
   UINT uTypeVal;

   uTypeVal = TypBuildVal (pType);
   TypCombineVal (ps, uTypeVal);

   ps->psStruct = StructTypePtr (pType); // null unless pType is struct or ptr to struct

   /*--- if type is a defined type, it may have array info ---*/
   if (pType->uID == TOK_IDENTIFIER)
      {
      ps->bArray     = pType->ps->bArray;
      ps->uArraySize = pType->ps->uArraySize;
      }
   }


UINT TypPtrBase (UINT uTypeVal)
   {
//   if (!(uTypeVal & 0x00FF))
//      Error ("TypPtrBase: val not a pointer [%d]", uTypeVal);
   if (!(uTypeVal & 0xFF00))
      Error ("TypPtrBase: val not a pointer [%d]", uTypeVal);
   return uTypeVal - 0x100;
   }


BOOL TypIsBaseType (UINT uID)
   {
   switch (uID)
      {
      case TOK_VOID  :
      case TOK_CHAR  :
      case TOK_SHORT :
      case TOK_INT   :
      case TOK_LONG  :
      case TOK_FLOAT :
      case TOK_STRING: // ?? delete this type?
         return TRUE;
      }
   return FALSE;
   }


BOOL TypIsType (void)
   {
   if (TypIsBaseType (Token.uID))
      return TRUE;

   if (Token.uID == TOK_STRUCT || Token.uID == TOK_UNION)
      return TRUE;
   
   if (Token.uID == TOK_IDENTIFIER)
      return (Token.ps->uKind == KIND_TYPEDEF);

   return FALSE;
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

/*
 * Avoid the void!
 *
 */
void TypNoVoid (PNODE pnode) 
   {
   if (!pnode->uTypeVal)
      NodeError (pnode, "void type in expression");
   }


BOOL TypIntegral (UINT uTypeVal)
   {
   return (uTypeVal == DATATYPE_CHAR  ||
           uTypeVal == DATATYPE_SHORT ||
           uTypeVal == DATATYPE_LONG);
   }


BOOL TypFloat (UINT uTypeVal)
   {
   return (uTypeVal == DATATYPE_FLOAT);
   }


/*
 * Pointy bird, so pointy pointy.
 * anoint my head, anointy nointy.
 */
BOOL TypPtr (UINT uTypeVal)
   {
   return (uTypeVal == DATATYPE_PTR    ||
           uTypeVal == DATATYPE_STRING ||
           uTypeVal > 0xFF);
   }


/*
 * Jointed bird, anointed jointly
 *
 */
BOOL TypString (UINT uTypeVal)
   {
   return (uTypeVal == DATATYPE_STRING ||
           uTypeVal == 0x100 + DATATYPE_CHAR);
   }


/*
 * this is one precise warning
 */
void TypPrecisionWarning (PNODE pnode)
   {
   NodeWarn (pnode, "possible loss of precision");
   }

/*
 * 
 */
void TypPrecisionWarning2 (PNODE pnode)
   {
   NodeWarn (pnode, "loss of precision");
   }

/*
 * perform inverse portfolio translation of the 
 * cross indexed foil vectors to ensure trans gendering for
 * each sub element - who the hell am I commenting this for anyway!
 */
void TypCheckPointers (PNODE pnode, UINT uTypeVal1, UINT uTypeVal2, BOOL bVoidOK)
   {
   if (uTypeVal1 == uTypeVal2)
      return;

   if (bVoidOK && uTypeVal1 == 0x100+DATATYPE_VOID)
      return;

   if ((uTypeVal1 >> 8) != (uTypeVal2 >> 8))
      NodeWarn (pnode, "different levels of indirection");
   else
      NodeWarn (pnode, "Indirection to different types");
   }



PNODE BuildCastNode (UINT uID, PNODE pnode, UINT uTypeVal)
   {
   PNODE pn1;

   pn1 = NodeBuild1 (uID, pnode);
   pn1->uTypeVal = uTypeVal;
   return pn1;
   }

PNODE TypCast (PNODE pnode, UINT uNewType)
   {
   UINT uOldType;

   uOldType = pnode->uTypeVal;

   if (TypFloat (uOldType) && TypIntegral (uNewType))
      return BuildCastNode (TOK_FTOW, pnode, uNewType);
   if (TypIntegral (uOldType) && TypFloat (uNewType))
      return BuildCastNode (TOK_WTOF, pnode, uNewType);

//   pnode->uTypeVal = uNewType; 

   return pnode;
   }


/*
 * uAllowPtrConversion: 0 - no
 *                      1 - yes with warning
 *                      2 - yes, and old type can be pvoid wo warning
 */
PNODE TypInsert (PNODE pnode, UINT uOldType, UINT uNewType, BOOL bAllowPtrConversion, BOOL bDieOnError)
   {
   PNODE pTmp;

   if (bAllowPtrConversion && TypPtr(uOldType) && TypPtr (uNewType))
      {
      TypCheckPointers (pnode, uOldType, uNewType, bAllowPtrConversion == 2);
      return pnode;
      }

   if (uOldType == uNewType)
      return pnode;

   if (TypIntegral (uOldType) && TypFloat (uNewType))
      return BuildCastNode (TOK_WTOF, pnode, uNewType);

   if (TypFloat (uOldType) && TypIntegral (uNewType))
      {
      TypPrecisionWarning (pnode);
      return BuildCastNode (TOK_FTOW, pnode, uNewType);  
      }
   if (TypIntegral (uOldType) && TypIntegral (uNewType))
      {
      if (MemSize (uOldType, NULL) > MemSize (uNewType, NULL))
         {
         pTmp = (pnode->uID == TOK_EXPR ? pnode->pn1 : pnode);
         if (pTmp->uID == TOK_LITERAL)
            {
            if (uNewType == DATATYPE_CHAR && (pTmp->pn1->val.l > 255 || pTmp->pn1->val.l < 0) )
               TypPrecisionWarning2 (pnode);
            if (uNewType == DATATYPE_SHORT && (pTmp->pn1->val.l > 32767 || pTmp->pn1->val.l < -32768))
               TypPrecisionWarning2 (pnode);
            }
         else
            TypPrecisionWarning (pnode);
         }
      return pnode;
      }
   if (TypIntegral (uOldType) && TypPtr (uNewType))
      {
      pTmp = (pnode->uID == TOK_EXPR ? pnode->pn1 : pnode);
      if (pTmp->uID == TOK_LITERAL && !pTmp->pn1->val.l)
         return pnode; // 0 is a valid ptr
      }

   NodeError (pnode, "Incompatible types");
   return pnode;
   }



/*
 * uPtrs: 0 - no ptrs allowed                  x
 *        1 - ptrs ok
 *        2 - + rules: ptr/ptr -> invalid      x
 *        3 - - rules: ptr/ptr -> int          x
 *
 * uMix:  0 - no mixing i & p                  x
 *        1 - mixing ok
 *        2 - ptr on left only                 x
 */
UINT TypResolve (PNODE pExpr, UINT bFloats, UINT uPtrs, UINT uMix)
   {
   UINT uTypeVal1, uTypeVal2, uma, umi;

   uTypeVal1 = pExpr->pn1->uTypeVal;
   uTypeVal2 = pExpr->pn2->uTypeVal;

   uma = max (uTypeVal1, uTypeVal2);
   umi = min (uTypeVal1, uTypeVal2);

   if (!uTypeVal1 || !uTypeVal2)
      NodeError (pExpr, "Operation on void type");

   if (!bFloats && (TypFloat (uTypeVal1) || TypFloat (uTypeVal2)))
      NodeError (pExpr, "Incompatible type for operation");

   if (!uPtrs && TypPtr (uma))
      NodeError (pExpr, "Incompatible type for operation");

   if (TypPtr (uma) && TypFloat (umi))
      NodeError (pExpr, "Incompatible types for operation");

   if (TypPtr (uma) && TypIntegral (umi))
      {
      if (!uMix || (uMix == 2 && uma == uTypeVal2))
         NodeError (pExpr, "Incompatible types for operation");
      return uma;
      }

   if (TypIntegral (uma) && TypIntegral (umi))
      return uma;
   
   if (TypPtr (umi)) //  && TypPtr (uma))
      {
      if (uPtrs == 2)
         NodeError (pExpr, "Incompatible type for operation");
      TypCheckPointers (pExpr, uma, umi, 0);
      return (uPtrs == 3 ? IntTyp() : uma);
      }

   if (uma == umi)
      return uma;

   /*--- f & i  or  i & f ---*/
   pExpr->pn1 = TypInsert (pExpr->pn1, uTypeVal1, uma, 0, 1);
   pExpr->pn2 = TypInsert (pExpr->pn2, uTypeVal2, uma, 0, 1);

   return uma;
   }




/* 
 * uMix 0: int -> ptr not allowed
 *      1: int -> ptr allowed with a warning
 *
 *
 */
void TypConvertStatic (PNODE pLit, UINT uTargetType, UINT uMix)
   {
   UINT uLitType;

   uLitType = pLit->uTypeVal;
   if (uLitType == uTargetType)
      return;

   if (TypIntegral (uLitType) && TypIntegral (uTargetType))
      {
      if (MemSize (uLitType, NULL) > MemSize (uTargetType, NULL))
         TypPrecisionWarning (pLit);
      pLit->uTypeVal = uTargetType;
      return;
      }

   if (TypIntegral (uLitType) && TypFloat (uTargetType))
      {
      if (uTargetType == DATATYPE_SHORT)
         pLit->val.bg = (BIG) pLit->val.s; 
      else
         pLit->val.bg = (BIG) pLit->val.l; // char type ok here
      pLit->uTypeVal = uTargetType;
      return;
      }

   if (TypFloat (uLitType) && TypIntegral (uTargetType))
      {
      TypPrecisionWarning (pLit);
      switch (uTargetType)
         {
         case DATATYPE_CHAR : pLit->val.c = (CHAR )pLit->val.bg; break;
         case DATATYPE_SHORT: pLit->val.s = (SHORT)pLit->val.bg; break;
         case DATATYPE_LONG : pLit->val.l = (LONG) pLit->val.bg; break;
         }
      pLit->uTypeVal = uTargetType;
      return;
      }

   if (TypPtr (uLitType) && TypPtr (uTargetType))
      {
      TypCheckPointers (pLit, uLitType, uTargetType, 1); // 0->1 $$
      return;
      }

   if (uMix==1 && TypIntegral (uLitType) && TypPtr (uTargetType))
      {
      NodeWarn (pLit, "different levels of indirection");
      pLit->uTypeVal = 0x0100 + DATATYPE_VOID;
      return;
      }
   NodeError (pLit, "invalid static type");
   }

/*
 *
 */
void CheckStructTypes (PNODE pnode, PSYM ps1, PSYM ps2)
   {
   if (!ps1 && !ps2) // neither are structures
      return; 
   if (ps1 && !ps2 || !ps1 && ps2) // one is structure
      NodeError (pnode, "different basic types");
   if (ps1 == ps2) // same structure
      return;
   if (ps1->uStructSize == ps2->uStructSize)
      NodeWarn (pnode, "different structure type (of same size)");
   else
      NodeWarn (pnode, "different structure type (of different size!)");
   }


INT IntTyp (void)
   {
#if defined (__386__)
   return DATATYPE_LONG;
#else
   return DATATYPE_SHORT;
#endif
   }

