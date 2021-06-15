/*
 *
 * check.c
 * Thursday, 2/27/1997.
 * Craig Fitzgerald
 *
 *
 * decorate symbols in the symbol table
 * decorates nodes in the parse/syntax tree
 * performs type checking and type conversion
 * performs param matching
 * normalizes pointer arithmetic
 * checks syntax of some things that get by the parser
 *
 *
 * global declarations must be operating on new symbols else-error
 * param and local declarations must be operating on new symbols else-
 *   create new symbols for them
 * variable references must be to previously declared vars else-error
 *
 * global var initializers must be constants
 * param declarations cannot be fns or have initializers
 * local declarations cannot be fns
 *
 * ensure all referenced symbols are declared
 *
 * assignments must have lval on left
 * var types need to be resolved in expression trees
 * var types need to be resolved in fn calls
 *
 * on fn declaration (not defn) remove param vars from
 *   symbol table
 *
 * ----------------------------------------------------------
 * flow control checks: break, continue in loops
 * case uniqueness and case scoping
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
#include "opt.h"
#include "type.h"
#include "mem.h"
#include "opcodes.h"
#include "node.h"


void CheckStmtDecls (PNODE pStmt);
void CheckDeclarationList (PNODE pDecList);
void CheckDeclarationLists (PNODE pDecList);
void CheckActuals (PNODE pFn);
void CheckExpression (PNODE pExpr);

UINT uSTRUCT_PACK_SIZE = 1;


UINT uFNTYPE;

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/*
 * makes sure pnode is assignable
 * (i.e. an lvalue)
 *
 */
static void EnsureLValue (PNODE pnode)
   {
   if (pnode->uID == TOK_VARIABLE && !pnode->pn1->ps->bArray)
      return;
   if (pnode->uID == TOK_DEREF)
      return;
   if (pnode->uID == TOK_PERIOD || pnode->uID == TOK_ARROW)
      return;

   NodeError (pnode, "operand must be lvalue");
   }


PSYM FindStructType (PNODE pnode, BOOL bDie)
   {
   if (!pnode || pnode->uTypeBase != DATATYPE_STRUCT)
      {
      if (bDie)
         NodeError (pnode, "internal error: cannot find struct");
      return NULL;
      }
   if (pnode->uID == TOK_IDENTIFIER)
      return pnode->ps->psStruct;
   if (pnode->pn2 && pnode->pn2->uTypeBase == DATATYPE_STRUCT)
      return FindStructType (pnode->pn2, bDie);
   if (pnode->pn1 && pnode->pn1->uTypeBase == DATATYPE_STRUCT)
      return FindStructType (pnode->pn1, bDie);

/* implicit casts (for example passing a NULL actual param
 * when fn is expecting a struct ptr) would cause this error
 * therefore we don't always die here
 */
   if (bDie)
      NodeError (pnode, "internal error: type inconsistency");
   return NULL;
   }


/*
 * psStruct is a: structdef symbol
 *               
 * pnode is a:  variable or identifier node containing a struct element
 * 
 * This fn checks to make sure that the var pnode is an element of the
 * psStruct struct.  This fn may change the pnode->ps symbol.
 *
 */
PSYM FindElement (PSYM psStruct, PNODE pnode)
   {
   PSYM *pps, psElement;

   if (pnode->uID == TOK_VARIABLE)
      pnode = pnode->pn1; // TOK_IDENTIFIER
   psElement = pnode->ps;

   if (!psStruct->ppElements)
      NodeError (pnode, "struct %s was not defined", psStruct->pszLiteral);

   /*--- loop thru structure elements ---*/
   for (pps = psStruct->ppElements; *pps; pps++)
      if (!strcmp (psElement->pszLiteral, (*pps)->pszLiteral))
         break;
   if (!*pps)
      NodeError (pnode, "element '%s' not part of struct '%s'", 
                  psElement->pszLiteral, psStruct->pszLiteral);
   pnode->val.s = TRUE;
   return pnode->ps = *pps;
   }




/*
 * This fn type checks and/or type converts all elements of
 * an expression
 *
 *
 *
 */
static void PropagateTypes (PNODE pExpr)
   {
   UINT uTypeVal1, uTypeVal2, uResultType, uSize;
   PSYM psStructType;

   if (!pExpr)
      return;

   if (pExpr->uID != TOK_FUNCTION && 
       pExpr->uID != TOK_PERIOD   &&
       pExpr->uID != TOK_ARROW)
      {
      PropagateTypes (pExpr->pn1);
      PropagateTypes (pExpr->pn2);
      }
   uTypeVal1 = (pExpr->pn1 ? pExpr->pn1->uTypeVal : 0);
   uTypeVal2 = (pExpr->pn2 ? pExpr->pn2->uTypeVal : 0);

   switch (pExpr->uID)
      {
      case TOK_EXPR:
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_LITERAL:
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_SIZEOF:
         if (pExpr->pn1->val.s) // cast with a star  ie: (type *)
            uSize = MEMSIZE_WORD;

         /*--- simple expression or a user defined type ---*/
         else if (pExpr->pn1->pn1->uID == TOK_IDENTIFIER)
            uSize = MemSymSize (pExpr->pn1->pn1->ps); // datatype or variable

         /*--- arbitrary expression -or- predefined type cast ---*/
         else
            uSize = MemSize (uTypeVal1, FindStructType (pExpr->pn1, FALSE));

         /*--- free sizeof tree & replace with a constant ---*/
         pExpr->pn1 = NodeFreeTree (pExpr->pn1);
         pExpr->uID = TOK_LITERAL;
         pExpr->pn1 = NodeBuild1 (TOK_INTLITERAL, NULL);
         pExpr->pn1->val.s = uSize;
         PropagateTypes (pExpr->pn1);
         pExpr->uTypeVal = IntTyp ();
         break;

      case TOK_IDENTIFIER:
         /*-- new: from CheckIdentUse ---*/
         if (!pExpr->ps->uKind)
            NodeError (pExpr, "undeclared variable or function : '%s'", pExpr->ps->pszLiteral);
         if (pExpr->ps->uScope == SCOPE_STRUCT && pExpr->val.s != TRUE)
            NodeError (pExpr, "undeclared variable '%s' (struct element?): ", pExpr->ps->pszLiteral);
         pExpr->uTypeVal = pExpr->ps->uTypeVal;
         break;

      case TOK_FTOW:
         pExpr->uTypeVal = IntTyp ();
         break;

      case TOK_WTOF:
         pExpr->uTypeVal = DATATYPE_FLOAT;
         break;

      case TOK_VOID:
      case TOK_CHARLITERAL:
      case TOK_CHAR:
      case TOK_SHORTLITERAL:
      case TOK_SHORT:
      case TOK_INTLITERAL:
      case TOK_INT:
      case TOK_LONGLITERAL:
      case TOK_LONG:
      case TOK_FLOATLITERAL:
      case TOK_FLOAT:
      case TOK_STRINGLITERAL:
      case TOK_STRING:
         pExpr->uTypeVal = TypBuildVal (pExpr);
         break;

      case TOK_CAST:
         uTypeVal1  = TypBuildVal (pExpr->pn1);
         uTypeVal1 += (pExpr->val.s << 8);

         pExpr->pn2 = TypCast (pExpr->pn2, uTypeVal1);
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_FUNCTION:
         if (pExpr->pn1->ps->uKind != KIND_FUNCTION)
            NodeError (pExpr->pn1, "Call to undeclared function '%s'", pExpr->pn1->ps->pszLiteral);
         CheckActuals (pExpr);
         pExpr->pn1->uTypeVal =  pExpr->pn1->ps->uTypeVal;
         pExpr->uTypeVal      =  pExpr->pn1->uTypeVal;
         break;

      case TOK_VARIABLE:
         if (pExpr->pn1->ps->uKind != KIND_VARIABLE)
            NodeError (pExpr->pn1, "undeclared variable '%s'", pExpr->pn1->ps->pszLiteral);
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_INCREMENT:
      case TOK_DECREMENT:
         EnsureLValue (pExpr->pn1 ? pExpr->pn1 : pExpr->pn2);
         pExpr->uTypeVal = (pExpr->pn1 ? uTypeVal1 : uTypeVal2);
         break;

      case TOK_EXCLAMATION:
         TypNoVoid (pExpr->pn1);
         pExpr->uTypeVal = IntTyp ();
         break;

      case TOK_SHL1:
      case TOK_SHL2:
      case TOK_SHL3:
         TypNoVoid (pExpr->pn1);
         pExpr->uTypeVal = pExpr->pn1->uTypeVal;
         break;

      case TOK_LESSTHAN :
      case TOK_GREATERTHAN :
      case TOK_LESSOREQUAL :
      case TOK_GREATEROREQUAL:
      case TOK_EQUIVALENT :
      case TOK_NOTEQUAL :
      case TOK_LOGICALOR :
      case TOK_LOGICALAND :
         uResultType = TypResolve (pExpr, 1, 1, 0);
         pExpr->uTypeVal = IntTyp ();
         break;

      case TOK_AMPERSAND :
      case TOK_OR :
      case TOK_HAT :
      case TOK_SHIFTRIGHT :
      case TOK_SHIFTLEFT :
      case TOK_TILDA :
         pExpr->uTypeVal = TypResolve (pExpr, 0, 0, 0);
         break;

      case TOK_SLASH :
      case TOK_PERCENT:
      case TOK_STAR :
         pExpr->uTypeVal = TypResolve (pExpr, 1, 0, 0);
         break;

      case TOK_PERIOD: // struct element
         PropagateTypes (pExpr->pn1);
         if (pExpr->pn1->uTypeVal != DATATYPE_STRUCT)
            {
            if (pExpr->pn1->uTypeVal == 0x100 + DATATYPE_STRUCT)
               NodeError (pExpr, "left of '.' is struct ptr, use '->'");
            else
               NodeError (pExpr, "left of '.' must be struct type");
            }
         psStructType = FindStructType (pExpr->pn1, TRUE);
         FindElement (psStructType, pExpr->pn2);
         PropagateTypes (pExpr->pn2);
         pExpr->uTypeVal = pExpr->pn2->uTypeVal;
         break;

      case TOK_ARROW : // struct element
         PropagateTypes (pExpr->pn1);
         if (pExpr->pn1->uTypeVal != 0x100 + DATATYPE_STRUCT)
            {
            if (pExpr->pn1->uTypeVal == DATATYPE_STRUCT)
               NodeError (pExpr, "left of '->' points to struct, use '->'");
            else
               NodeError (pExpr, "left of '->' must be ptr to struct");
            }
         psStructType = FindStructType (pExpr->pn1, TRUE);
         FindElement (psStructType, pExpr->pn2);
         PropagateTypes (pExpr->pn2);
         pExpr->uTypeVal = pExpr->pn2->uTypeVal;
         break;

      case TOK_PLUS :
         pExpr->uTypeVal = TypResolve (pExpr, 1, 2, 1);
         break;

      case TOK_MINUS :
         if (pExpr->pn2)
            {
            pExpr->uTypeVal = TypResolve (pExpr, 1, 3, 1);
            break;
            }
         else // UNARY FORM
            {
            TypNoVoid (pExpr->pn1);
            pExpr->uTypeVal = uTypeVal1;
            }
         break;
            
      case TOK_QUESTION:
         PropagateTypes (pExpr->pn3);
         pExpr->uTypeVal = TypResolve (pExpr, 1, 1, 0);
         break;

      case TOK_EQUALS :
         EnsureLValue (pExpr->pn1);
         pExpr->pn2 = TypInsert (pExpr->pn2, uTypeVal2, uTypeVal1, 2, 1);
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_PLUSEQUAL :
      case TOK_MINUSEQUAL :
         EnsureLValue (pExpr->pn1);
         if (TypPtr(uTypeVal1) && TypIntegral (uTypeVal2))
            ;
         else
            pExpr->pn2 = TypInsert (pExpr->pn2, uTypeVal2, uTypeVal1, 0, 1);
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_STAREQUAL :
      case TOK_SLASHEQUAL :
      case TOK_PERCENTEQUAL :
         EnsureLValue (pExpr->pn1);
         pExpr->pn2 = TypInsert (pExpr->pn2, uTypeVal2, uTypeVal1, 0, 1);
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_SHIFTRIGHTEQUAL:
      case TOK_SHIFTLEFTEQUAL :
      case TOK_ANDEQUAL :
      case TOK_XOREQUAL :
      case TOK_OREQUAL :
         EnsureLValue (pExpr->pn1);
         TypResolve (pExpr, 0, 0, 0);
         pExpr->pn2 = TypInsert (pExpr->pn2, uTypeVal2, uTypeVal1, 0, 1);
         pExpr->uTypeVal = uTypeVal1;
         break;

      case TOK_DEREF:
         if (!pExpr->pn1->uTypeDeref)
            NodeError (pExpr, "dereference of a non pointer");
         pExpr->uTypeBase  = pExpr->pn1->uTypeBase;
         pExpr->uTypeDeref = pExpr->pn1->uTypeDeref - 1;
         break;

      case TOK_ADDROF:
         EnsureLValue (pExpr->pn1);
         pExpr->uTypeBase  = pExpr->pn1->uTypeBase;
         pExpr->uTypeDeref = pExpr->pn1->uTypeDeref + 1;
         break;

      default:
         NodeError (NULL, "Internal error Token not handled in PropagateTypes: '%d'", pExpr->uID);
      }

   }


/*
 * makes a dummy param
 * used by CheckActuals
 */
static PNODE MakeDummyActual (UINT uFormalType)
   {
   PNODE p1, p2;

   p1 = NodeBuildLit (uFormalType);
   p2 = NodeBuild1 (TOK_EXPR, p1);
   CheckExpression (p2); // we need to propagate the types
   return p2;
   }


/*
 * This examines params used in a function call
 * It does all the normal expression checking for each parm,
 * It makes sure the params are the right type
 * and dummies up params to make sure there at least as many
 * actuals as formal params
 */
static void CheckActuals (PNODE pFn)
   {
   PNODE pExpr, pDecl;
   PSYM  ps;
   UINT  uCount, uActualType, uFormalType;

   /*--- 1st check each expression in the parm list ---*/
   uCount = 0;
   for (pExpr = pFn->pn2; pExpr; pExpr = pExpr->next)
      {
      CheckExpression (pExpr);
      uCount++;
      }
   pFn->val.s = uCount; // store # of actuals

   /*--- type convert parms as necessary ---*/
   uCount = 0;
   for (pExpr = pFn->pn2; pExpr; pExpr = pExpr->next)
      {
      ps = pFn->pn1->ps;

      uFormalType = ps->pFormals[uCount].uTypeVal;
      if (uFormalType == 0xFF)
         break; // more actuals than formals

      uActualType = pExpr->uTypeVal;
      if (uActualType != uFormalType)
         {
         if (!(pExpr->pn1 = TypInsert (pExpr->pn1, uActualType, uFormalType, 2, 0)))
            NodeError (pExpr, "Incompatible types, parameter %d", uCount+1);
         pExpr->uTypeVal = uFormalType;
         }
      else if (ps->pFormals[uCount].psStruct)
         {
         CheckStructTypes (pExpr, FindStructType (pExpr, 0), ps->pFormals[uCount].psStruct);
         }
      uCount++;
      }

   /*--- if more formals than actuals, pad parm space ---*/
   for (; pFn->pn1->ps->pFormals[uCount].uTypeVal != 0xFF; uCount++)
      {
      uFormalType = pFn->pn1->ps->pFormals[uCount].uTypeVal;
      pDecl = MakeDummyActual (uFormalType);
      pFn->pn2 = NodeAddToEnd (pFn->pn2, pDecl);
      }
   }


/***************************************************************************/

///*
// * Makes sure all referenced idents are defined.
// *
// * This uses a brute force method of finding all identifiers
// * used in the parse tree.
// */
//static void CheckIdentUse (PNODE pExpr)
//   {
//   if (!pExpr)
//      return;
//
//   CheckIdentUse (pExpr->pn1);
//   CheckIdentUse (pExpr->pn2);
//   CheckIdentUse (pExpr->next); // fn calls have params linked via next ptr
//
//   if (pExpr->uID == TOK_PERIOD)
//   if (pExpr->uID == TOK_ARROW)
//
//
//   if (pExpr->uID != TOK_IDENTIFIER)
//      return;
//
//   if (!pExpr->ps->uKind)
//      NodeError (pExpr, "undeclared variable or function : '%s'", pExpr->ps->pszLiteral);
//   }

/***************************************************************************/

/*
 * used by ConvertPointerExpressions to size constants used 
 * in ptr arithmetic, all basetypes have a size that is a
 * power of 2 - therefore we can usually use a shift operation
 */
static PNODE InsertShift (PNODE pnode, UINT uTypeVal, PSYM psStructType)
   {
   UINT uSize;
   PNODE pn1, pn2;

   uSize = MemSize (TypPtrBase (uTypeVal), psStructType);
   if (uSize < 2)
      return pnode;

   /*--- optimize if we can ---*/
   if (pnode->uID == TOK_LITERAL && TypIntegral (pnode->pn1->uTypeVal)) 
      {
      OptChangeLit (pnode, OP_MUL, uSize);
      return pnode;
      }

   switch (uSize)
      {
      case 2: 
         return NodeBuild1 (TOK_SHL1, pnode);
      case 4: 
         return NodeBuild1 (TOK_SHL2, pnode);
      case 8: 
         return NodeBuild1 (TOK_SHL3, pnode);
      default: 
         pn1 = NodeBuild1 (TOK_SHORTLITERAL, NULL);
         pn1->val.s    = uSize;
         pn1->uTypeVal = DATATYPE_SHORT;
         pn2 = NodeBuild1 (TOK_LITERAL, pn1);
         return NodeBuild2 (TOK_STAR, pn2, pnode);
      }
   return NULL;
   }


/*
 * expressions involving pointers:
 * PINT+1 really means PTR to INT + size of INT
 * pointer arithmetic must be done in terms of elements
 * not bytes, so static values must be multiplied
 * by the size of an element
 */
static void ConvertPointerExpressions (PNODE pnode)
   {
   if (!pnode)
      return;

   if (pnode->uID == TOK_FUNCTION)
      return;

   ConvertPointerExpressions (pnode->pn1);
   ConvertPointerExpressions (pnode->pn2);

   if (!pnode->pn2  || 
      (pnode->uID != TOK_PLUS && pnode->uID != TOK_MINUS))
      return;

   if (TypPtr (pnode->pn1->uTypeVal) && TypIntegral (pnode->pn2->uTypeVal))
      pnode->pn2 = InsertShift (pnode->pn2, pnode->uTypeVal, FindStructType (pnode->pn1, 0));
   else if (TypPtr (pnode->pn2->uTypeVal) && TypIntegral (pnode->pn1->uTypeVal))
      pnode->pn1 = InsertShift (pnode->pn1, pnode->uTypeVal, FindStructType (pnode->pn1, 0));
   }


/*
 * This fn checks that all referenced variables have been defined
 * converts types in expressions and makes sure they are type compatible
 * performs expression simplification
 * adjusts pointer expressions (see ConvertPointerExpressions)
 *
 */
static void CheckExpression (PNODE pExpr)
   {
   if (!pExpr)
      return; // expressions sometimes can be empty

   if (pExpr->uID != TOK_EXPR)
      NodeError (pExpr, "Internal Error in CheckExpression");

   if (!pExpr->pn1)
      return;

//   CheckIdentUse  (pExpr);
   PropagateTypes (pExpr);

   OptCollapseExpr (pExpr);      // static initializers must be static
   ConvertPointerExpressions (pExpr); 
   }

/* $$
 *
 */
static BOOL StaticNode (PNODE pExpr)
   {
   if (!pExpr)
      return FALSE;
   if (pExpr->uID == TOK_LITERAL)
      return TRUE;
   if (pExpr->uID == TOK_CAST)
      return StaticNode (pExpr->pn2);
   return FALSE;
   }


/*
 * after OptCollapseExpr static expressions
 * reduce to a literal
 *
 * In this virtual machine environment I cannot support
 * ADDRESS OF '&' as a static value.  This is because memory
 * locations undergo a translation at run time.  (Unless
 * I am going to create relocatable code, a fixup table, 
 * and an associated loader!)
 *
 */
static BOOL StaticExpression (PNODE pExpr)
   {
   if (!pExpr)
      return FALSE;

   return StaticNode (pExpr->pn1);

//   return (pExpr && 
//           pExpr->pn1 &&
//           pExpr->pn1->uID == TOK_LITERAL);
   }


/*
 * local initializers generate code, 
 * this fn in essence converts:
 *
 *    int i=0;
 *
 * into
 *
 *    int i;
 *    i=0;
 *
 * so that the code generator can handle it just like
 * any other assignment
 *
 */
static PNODE FixLocalInitializer (PNODE pIdent, PNODE pExpr)
   {
   PNODE pIdentCpy, pnode, pnode2;
                      
   pIdentCpy  = NodeBuild1 (0, NULL);
   memcpy (pIdentCpy, pIdent, sizeof (NODE));

   pnode   = NodeBuild1 (TOK_VARIABLE, pIdentCpy);
   pnode2  = NodeBuild2 (TOK_EQUALS, pnode, pExpr->pn1);
   pExpr->pn1 = pnode2;
   return pExpr;
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/*
 * Here we create an array of ints, one per function parameter.  Each int 
 * contains the typeval of the parameter to be used to match up actual 
 * params when the fn is called.  Note that this does not allow structures
 * to be passed because vars of type struct also need a psStruct val to 
 * complete its type definition and we only store an int.
 * Also, when passing structure pointers I cannot fully typecheck - all I
 * know is that there is a structure at the end of the pointer.
 *
 * I should probably replace this with a linked list of elements that have
 * space for a ps.  But I rarely pass structures ...
 *
 *
 */
static PFORMAL MakeFormalsList (PNODE pFnDecl)
   {
   PNODE pDL, pV;
   UINT  uCount = 0;
   PFORMAL pFormals;

   for (pDL = pFnDecl; pDL; pDL = pDL->next)
      for (pV=pDL->pn2; pV; pV=pV->next)
         uCount++;
   pFormals = calloc (uCount+1, sizeof (FORMAL));

   uCount = 0;
   for (pDL = pFnDecl; pDL; pDL = pDL->next)
      for (pV=pDL->pn2; pV; pV=pV->next)
         {
         pFormals[uCount].uTypeVal = pV->pn1->ps->uTypeVal;
         pFormals[uCount].psStruct = pV->pn1->ps->psStruct;
         uCount++;
         }
   pFormals[uCount].uTypeVal = 0xFF; // terminator
   return pFormals;
   }



/*
 * Check statement list for general validity:
 *
 */
static void CheckCmpStmtDecls (PNODE pCmpStmt)
   {
   PNODE pStmt;

   if (!pCmpStmt)
      return;

   for (pStmt = pCmpStmt->pn4; pStmt; pStmt = pStmt->next)
      CheckStmtDecls (pStmt);
   }


/*
 * Check statement for general validity:
 *
 */
static void CheckStmtDecls (PNODE pStmt)
   {
   if (!pStmt)
      return;

   switch (pStmt->uID)
      {
      case TOK_EXPR:
         CheckExpression (pStmt);
         break;

      case TOK_BREAK:
      case TOK_CONTINUE:
         break;

      case TOK_RETURN:
         {
         if (!pStmt->pn1 && uFNTYPE)
            NodeError (pStmt, "return statement must have a value");
         if (pStmt->pn1 && !uFNTYPE)
            NodeError (pStmt, "value not allowed in return statement");
         if (pStmt->pn1)
            {
            CheckExpression (pStmt->pn1);
            /*--- make sure return type matches fn type ---*/
            TypInsert (pStmt->pn1, pStmt->pn1->uTypeVal, uFNTYPE, 2, 1);
            }
         }
         break;

      case TOK_COMPOUNDSTATEMENT:
         CheckCmpStmtDecls (pStmt);
         break;
         
      case TOK_IF:
         CheckExpression (pStmt->pn1);
         CheckStmtDecls  (pStmt->pn4);
         CheckStmtDecls  (pStmt->pn3);
         break;

      case TOK_WHILE:
         CheckExpression (pStmt->pn1);
         CheckStmtDecls  (pStmt->pn4);
         break;

      case TOK_FOR:
         CheckExpression (pStmt->pn1);
         CheckExpression (pStmt->pn2);
         CheckExpression (pStmt->pn3);
         CheckStmtDecls  (pStmt->pn4);
         break;

      case TOK_SWITCH:
         CheckExpression (pStmt->pn1);
         CheckStmtDecls  (pStmt->pn4);
         break;

      case TOK_CASE:
         CheckExpression  (pStmt->pn1);
         if (!StaticExpression (pStmt->pn1))
            NodeError (pStmt, "case value must be static");
         CheckStmtDecls   (pStmt->pn4);
         break;

      case TOK_DEFAULT:
         CheckStmtDecls   (pStmt->pn4);
         break;

      case TOK_DECLARATIONLIST:
         CheckDeclarationList (pStmt);
         break;

      default:
         Error ("Internal Error: Unknown statement id: '%d'", pStmt->uID);
      }
   }


/*
 * decorate a single function identifier symbol
 *
 *
 *
 */
static void CheckFnDeclaration (PNODE pDecl, PNODE pType)
   {
   PNODE pIdent, pFnDecl, pInternal, pBody;
   PFORMAL pFormals;
   UINT  i, uScope, uTypeVal;

   pIdent   = pDecl->pn1;
   pFnDecl  = pDecl->pn2;
   pInternal= pDecl->pn3;
   pBody    = pDecl->pn4;

   uTypeVal = TypBuildVal (pType);

   uScope   = CurrentScope ();

   if (uScope != SCOPE_GLOBAL)
      NodeError (pIdent, "Cannot declare nested functions: %s", pIdent->ps->pszLiteral);

   if (pIdent->ps->uKind) // already defined
      {
      if (!pBody)
         NodeError (pIdent, "function already declared: '%s'", pIdent->ps->pszLiteral);
      if (pIdent->ps->bDefined)
         NodeError (pIdent, "function already defined: '%s'", pIdent->ps->pszLiteral);
      if (pIdent->ps->uTypeVal != uTypeVal)
         NodeError (pIdent, "cannot redeclare fn return type: '%s'", pIdent->ps->pszLiteral);
      }

// TypCombineVal (pIdent->ps, uTypeVal);  

   TypXfer (pIdent->ps, pType);
//   pIdent->uTypeVal = pIdent->ps->uTypeVal;
//   pDecl->uTypeVal = pIdent->uTypeVal;

   pIdent->ps->uKind    = KIND_FUNCTION;  //
   pIdent->ps->bDefined = !!pBody;        //
                                          //
   uFNTYPE = pIdent->ps->uTypeVal;        // used when examining return stmts

   if (pInternal)
      {
      pIdent->ps->uInternal = (UINT)pInternal->val.l;
      if (pBody)
         NodeError (pIdent, "internal function cannot have a body: '%s'", pIdent->ps->pszLiteral);
      }

   PushScope (SCOPE_PARAM);
   CheckDeclarationLists (pFnDecl);
   pFormals = MakeFormalsList (pFnDecl);
   PopScope ();

   /*--- if already declared, make sure param list hasn't changed ---*/
   if (pIdent->ps->pFormals)
      {
      for (i=0; pFormals[i].uTypeVal != 0xFF; i++)
         if (pFormals[i].uTypeVal != pIdent->ps->pFormals[i].uTypeVal ||
             pFormals[i].psStruct != pIdent->ps->pFormals[i].psStruct)
            NodeError (pIdent, "Fn '%s' declarations differ, param #%d", pIdent->ps->pszLiteral, i);
      MemFreeData (pFormals);
      }
   else
      pIdent->ps->pFormals = pFormals;

   PushScope (SCOPE_LOCAL);
   CheckStmtDecls (pBody);
   PopScope ();
   }


/*
 * if bStringGlob, then a string can be used
 * in place of a series of char literals
 */
static UINT CheckInitializerElement (PNODE pIdent, PNODE pExpr, UINT uTypeVal, BOOL bStringGlob)
   {
   CheckExpression (pExpr);   // this should set the uTypeVals in pnodes
   if (!StaticExpression (pExpr))
      NodeError (pExpr, "Global initialization must be static: '%s'", pIdent->ps->pszLiteral);

   if (bStringGlob && uTypeVal == DATATYPE_CHAR && TypString (pExpr->uTypeVal))
      return strlen (pExpr->pn1->pn1->val.psz) + 1;

   TypConvertStatic (pExpr->pn1->pn1, uTypeVal, 1);
   return 1;
   }


/*
 * check initializer for a global var
 *
 */
static void CheckGlobalInitializer (PNODE pDecl)
   {
   PNODE pIdent, pInit, pExpr;
   PSYM  ps;
   UINT  uCount, uElementType;

   pIdent = pDecl->pn1;
   pInit  = pDecl->pn2;
   ps     = pIdent->ps;

   /* global non-array initializers
    * initializer of form 'type ident = value;'
    */
   if (!ps->bArray)
      {
      CheckInitializerElement (pIdent, pInit, ps->uTypeVal, 0);
      return;
      }
   uElementType = TypPtrBase (ps->uTypeVal);
   uCount = 0;
   for (pExpr = pInit; pExpr; pExpr = pExpr->next)
      {
      uCount+= CheckInitializerElement (pIdent, pExpr, uElementType, 1);
      }

   if (!ps->uArraySize) // size not given - so set it
      {
      ps->uArraySize = uCount;
      }
   else // verify initializers size
      {
      if (ps->uArraySize < uCount)
         NodeError (pInit, "More initializers (%d) than array elements (%d)",
                    uCount, ps->uArraySize);
      }
   }





/*
 *
 * decorate a single variable identifier symbol
 *
 */
static void CheckVarDeclaration (PNODE pDecl, PNODE pType)
   {
   PNODE pIdent, pInit, pASize;
   UINT  uScope;

   pIdent = pDecl->pn1;
   pInit  = pDecl->pn2;
   pASize = pDecl->pn3;

   TypXfer (pIdent->ps, pType);
   pIdent->ps->uKind  = KIND_VARIABLE;

   if (pASize) // array size expr
      {
      pIdent->ps->bArray = TRUE;
      pIdent->ps->uTypeDeref++;
      if (pASize->uID == TOK_VOID) // array size not given
         {
         pIdent->ps->uArraySize = 0;
         }
      else
         {
         CheckExpression (pASize);
         if (!StaticExpression (pASize))
            NodeError (pIdent, "Array Size must be static: '%s'", pIdent->ps->pszLiteral);
         TypConvertStatic (pASize->pn1->pn1, IntTyp (), 1);
         pIdent->ps->uArraySize = pASize->pn1->pn1->val.s;
         }
      }
   uScope = CurrentScope ();
   if (pInit)  // var has initializer?
      {
      if (uScope == SCOPE_STRUCT)
         {
         NodeError (pIdent, "struct elements cannot have initializers '%s'", pIdent->ps->pszLiteral);
         }
      if (uScope == SCOPE_PARAM)
         {
         NodeError (pIdent, "Parameter variables cannot have initializers '%s'", pIdent->ps->pszLiteral);
         }
      if (uScope == SCOPE_GLOBAL)
         {
         CheckGlobalInitializer (pDecl);
         }
      if (uScope == SCOPE_LOCAL)
         {
         if (pASize)
            NodeError (pIdent, "Array initializers not allowed for local variables");
         pInit = FixLocalInitializer (pIdent, pInit);
         CheckExpression (pInit);
         }
      }
   }


/*
 *
 * sets variable and function type info for var/fns in a declaration 
 * list chain.
 *
 * (pDecl is TOK_VARIABLE ot TOK_FUNCTION)
 *
 */
static void CheckDeclarationList (PNODE pDecList)
   {
   PNODE pDecl, pType;

//   assert (TOK_DECLARATIONLIST == pDecList->uID);

   pType = pDecList->pn1;

   for (pDecl = pDecList->pn2; pDecl; pDecl = pDecl->next)
      {
      if (pDecl->uID == TOK_VARIABLE)
         CheckVarDeclaration (pDecl, pType);
      else
         CheckFnDeclaration (pDecl, pType);
      }
   }


/*
 * sets variable and function type info for var/fns in a declaration 
 * list chain.
 *
 */
static void CheckDeclarationLists (PNODE pDecList)
   {
   PNODE pDecL;

   if (!pDecList)  // will be NULL checking fn params when it is "(void)"
      return;

//   assert (TOK_DECLARATIONLIST == pDecList->uID);

   for (pDecL = pDecList; pDecL; pDecL = pDecL->next)
      CheckDeclarationList (pDecL);
   }


/*
 * EXTERNAL
 *
 * This is the external function which 
 * checks a var or fn declaration (or definition)
 */
void CheckDeclaration (PNODE pnode)
   {
   PushScope (SCOPE_GLOBAL);
   CheckDeclarationLists (pnode);
   PopScope ();
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

/*
 * EXTERNAL
 * Check a typedef parse tree
 *
 *
 */
void CheckTypedef (PNODE pnode)
   {
   PNODE pIdent, pType, pASize;
   PSYM  ps;

   pType  = pnode->pn1;
   pIdent = pnode->pn2;
   pASize = pnode->pn3;
   ps = pIdent->ps;

   TypXfer  (ps, pType);
   ps->uKind = KIND_TYPEDEF;

   if (pASize) // array size expr
      {
      pIdent->ps->bArray = TRUE;
      pIdent->ps->uTypeDeref++;
      if (pASize->uID == TOK_VOID) // array size not given
         {
         ps->uArraySize = 0;
         }
      else
         {
         CheckExpression (pASize);
         if (!StaticExpression (pASize))
            NodeError (pIdent, "Array Size must be static: '%s'", ps->pszLiteral);
         TypConvertStatic (pASize->pn1->pn1, IntTyp (), 1);
         pIdent->ps->uArraySize = pASize->pn1->pn1->val.s;
         }
      }
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

/* 
 * builds element list array
 * calcs element offsets
 * calcs struct size
 */
static void CalcStructMetrics (PNODE pStruct)
   {
   PNODE pn1, pn2, pIdent;
   UINT  uVarSize, uElementCount, uElementSize;
   UINT  uMaxElementSize = 0, uOffset = 0;
   PSYM  *ppElementList;

   /*--- count elements in struct definition ---*/
   uElementCount = 0;
   for (pn1 = pStruct->pn2; pn1; pn1 = pn1->next)
      for (pn2 = pn1->pn2; pn2; pn2 = pn2->next)
         uElementCount++;

   ppElementList = calloc (uElementCount+1, sizeof (PSYM));

   /*--- fill struct element ptr array ---*/
   uElementCount = 0;
   for (pn1 = pStruct->pn2; pn1; pn1 = pn1->next)
      for (pn2 = pn1->pn2; pn2; pn2 = pn2->next)
         ppElementList[uElementCount++] = pn2->pn1->ps;
   ppElementList[uElementCount] = NULL;

   pStruct->pn1->ps->ppElements = ppElementList;

   /*--- calc size and offsets ---*/
   for (pn1 = pStruct->pn2; pn1; pn1 = pn1->next)
      {
      for (pn2 = pn1->pn2; pn2; pn2 = pn2->next)
         {
         pIdent = pn2->pn1;

         if (pStruct->pn1->ps->bUnion)
            pIdent->ps->iAddr = 0;
         else
            pIdent->ps->iAddr = uOffset;

         uVarSize = MemSymSize (pIdent->ps);
         uElementSize = uVarSize + (uVarSize > 1 ? uVarSize % uSTRUCT_PACK_SIZE : 0);
         uOffset += uElementSize;
         uMaxElementSize = max (uMaxElementSize, uElementSize);
         }
      }
   if (pStruct->pn1->ps->bUnion)
      pStruct->pn1->ps->uStructSize = uMaxElementSize;
   else
      pStruct->pn1->ps->uStructSize = uOffset;
   }


/*
 * EXTERNAL
 * Check a Structure definition parse tree
 *
 * decorate symbols and syntax tree nodes
 * make sure structure isn't already defined
 * make sure var names are unique in the structure
 * calc element offsets
 * calc structure size
 *
 */
void CheckStructDef (PNODE pStruct)
   {
   CheckDeclarationLists (pStruct->pn2);
   CalcStructMetrics (pStruct);
   pStruct->pn2 = NodeFreeTree (pStruct->pn2); // free definition
   }



