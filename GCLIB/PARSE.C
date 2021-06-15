/*
 * 
 * parse.c
 * Saturday, 3/22/1997.
 * Craig Fitzgerald
 * 
 * 
 * ====================================================================
 * 
 * program             - {typedef | declaration} ...
 * 
 * typedef             - <later>
 * 
 * declaration         - typespec declarator_list ';'
 * declarator_list     - declarator [',' declarator]@
 * declarator          - ident {var_declarator | fn_declarator}
 * var_declarator      - {'=' expression | null}
 * fn_declarator       - initparmlist {compoundstatement | ';' | ':' number ';'}
 * 
 * typespec            - 'int' | 'float' | 'char' | 'string'
 * ident               -
 * 
 * initparmlist        - '(' {initparmentry | 'void'} ')'
 * initparmentry       - typespec ident [',' typespec ident]@
 * parmvalues          - typespec ident [',' parmvalues]
 * 
 * compoundstatement   - '{' statementlist '}'
 * statementlist       - statement [statementlist]
 * 
 * statement           - compoundstatement
 *                       declaration
 *                       expression;
 *                       'if' '(' expression ')' statement
 *                       'if' '(' expression ')' statement 'else' statement
 *                       'while' '(' expression ')' statement
 *                       'for' '(' [expr1]; [expr2]; [expr3] ')' statement
 *                       'switch' '(' expression ')' statement
 *                       'case' expression ':' statement
 *                       'default' ':' statement
 *                       break ';' 
 *                       continue ';'
 *                       return ';'
 *                       return expression ';'
 *                       ;
 * 
 * 
 *           
 * expatom - literal | identifier | function call
 * expf    - () []c -> .
 * expe    - expf [ '++' '--']
 * expd    - [ '!' '~' '-' '++' '--' '*' '&' expd] | expe
 * expc    - expd [ '*' '/' '%' expd]@
 * expb    - expc [ '+' '-' expc]@
 * expa    - expb [ '<<' '>>' expb]@
 * exp9    - expa [ '<' '>' '<=' '>=' expa]@
 * exp8    - exp9 [ '==' | '!=' exp9]@
 * exp7    - exp8 [ '&'  exp8]@
 * exp6    - exp7 [ '^'  exp7]@
 * exp5    - exp6 [ '|'  exp6]@
 * exp4    - exp5 [ '&&' exp5]@
 * exp3    - exp4 [ '||' exp3]@
 * exp2    - exp3 [ '?' exp3 ':' exp3]
 * exp1    - exp2 [ '=' '+=' '-=' '*=' '/=' '%=' '>>=' '<<=' '&=' '^=' '|=' exp1]
 * exp0    - exp1 [ ','  exp1]@
 * express - exp0
 * 
 * 
 * 
 * node type:
 * 
 *    node [ident] (on declaration)
 *       pn1 - expr (initialization)
 * 
 * 
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <stdarg.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "tokenize.h"
#include "parse.h"
#include "check.h"
#include "error.h"
#include "gen.h"
#include "genfn.h"
#include "genglob.h"
#include "type.h"
#include "node.h"


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

PNODE ParseLevelD (void);
PNODE ParseExpression (void);
PNODE ParseMaybeExpression (UINT uBlankTok);
PNODE ParseNoCommaExpr (void);
PNODE ParseExpr       (void);
PNODE ParseStatement  (void);
PNODE ParseCompoundStatement (void);
PNODE ParseDeclaration (PBOOL pbDefn);
PNODE ParseDeclarationStatement (void);
void  PrintTree (PNODE pnode, UINT uIndent);
PNODE ParseLevel1 (void);
PNODE ParseTypespec (void);


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

UINT uSCOPELIST[10];
UINT uSCPPTR = 0;

void PushScope (UINT uScope)
   {
   if (uSCPPTR > 4)
      ParseError ("Internal: Scope stack exceeded max val = Why is this?");
   uSCOPELIST[uSCPPTR++] = uScope;
   }

void PopScope ()
   {
   if (!uSCPPTR)
      ParseError ("Internal: Scope list empty error");
   uSCPPTR--;
   }

UINT CurrentScope (void)
   {
   return uSCOPELIST[uSCPPTR-1];
   }



/*
 * make sure var is not already defined in current scope
 * make new symbol if necessary
 * set symbol's scope
 *
 *   curr    var     action
 *   --------------  -------
 * 2 global  global  e-defined
 * 4 global  local   x
 * 4 global  parm    x
 * 4 global  struct  cpy
 * 
 * 4 local   global  cpy
 * 2 local   local   e-defined
 * 3 local   parm    e-defined
 * 4 local   struct  cpy
 * 
 * 4 parm    global  cpy
 * 4 parm    local   x
 * 2 parm    parm    e-defined
 * 4 parm    struct  cpy
 * 
 * 1 struct  global  cpy
 * 1 struct  local   cpy
 * 1 struct  parm    cpy
 * 1 struct  struct  cpy
 * 
 */ 
void SetScope (PNODE pIdent)
   {
   if (!pIdent->ps->uScope)              // zero if not already defined
      {
      pIdent->ps->uScope = CurrentScope ();
      }
   else if (CurrentScope () == SCOPE_STRUCT) /*1*/
      {
      pIdent->ps = SymAdd (pIdent->ps->pszLiteral, TRUE); // duplicate a local copy
      }
   else if (CurrentScope () == pIdent->ps->uScope) /*2*/
      {
      NodeError (pIdent, "Variable '%s' already defined", pIdent->ps->pszLiteral);
      }
   else if (CurrentScope () == SCOPE_LOCAL && pIdent->ps->uScope == SCOPE_PARAM)
      {
      NodeError (pIdent, "Variable '%s' already defined as parameter", pIdent->ps->pszLiteral);
      }
   else /*4*/
      {
      pIdent->ps = SymAdd (pIdent->ps->pszLiteral, FALSE); // duplicate a local copy
      }
   pIdent->ps->uScope = CurrentScope ();
   }


PSYM NotStructElement (PSYM ps)
   {
   if (ps->uScope == SCOPE_STRUCT)
      return SymAdd (ps->pszLiteral, FALSE);
   return ps;
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


PNODE ParseActuals (void)
   {
   PNODE pExpr, pHead = NULL;

   TokEat (TOK_OPAREN);

   if (TokTry (TOK_CPAREN))
      return NULL;

   while (TRUE)
      {
      pExpr = ParseNoCommaExpr ();

      pHead  = NodeAddToEnd (pHead, pExpr);
      if (!TokTry (TOK_COMMA))
         break;
      }
   TokEat (TOK_CPAREN);
   return pHead;
   }


/*
 * typedef or paren expr
 *
 */
PNODE ParseParens (PBOOL pbExpr)
   {
   PNODE pn1, pn2;
   UINT  iDeref;

   TokEat (TOK_OPAREN);
   TokPeek ();

   if (TypIsType ())     // type cast
      {
      pn1 = ParseTypespec ();

      for (iDeref=0; TokTry (TOK_STAR); iDeref++)
         ;
      TokEat (TOK_CPAREN);
      *pbExpr = FALSE;
      pn2 = NodeBuild1 (TOK_CAST, pn1);
      pn2->val.s = iDeref;
      return pn2;
      }
   else                  // paren expr
      {
      pn1 = ParseExpr ();
      TokEat (TOK_CPAREN);
      *pbExpr = TRUE;
      return pn1;
      }
   }


/*
 * atom l->r
 */
PNODE ParseAtom (void)
   {
   PNODE pnode, pLeaf, pList = NULL;
   UINT  uID;
   BOOL  bExpr;

   uID   = TokGet();
   pLeaf = NodeBuild ();

   switch (uID)
      {
      case TOK_IDENTIFIER:
         if (TokPeek() == TOK_OPAREN)
            {
            pList = ParseActuals (); // parse actuals
            pnode = NodeBuild2 (TOK_FUNCTION, pLeaf, pList);
            }
         else
            {
            pnode = NodeBuild1 (TOK_VARIABLE, pLeaf);
            }
         return pnode;

      case TOK_SIZEOF:
         pLeaf->pn1 = ParseParens (&bExpr); // typecast or paren expr
         return pLeaf;

      case TOK_CHARLITERAL  :
      case TOK_SHORTLITERAL :
      case TOK_LONGLITERAL  :
      case TOK_INTLITERAL   :
      case TOK_FLOATLITERAL :
      case TOK_STRINGLITERAL:
         pnode =  NodeBuild1 (TOK_LITERAL, pLeaf);
         return pnode;

      default:
         ParseError ("identifier or value expected [%d]", uID);
      }
   return NULL;
   }



/*
 * ()parens [] -> .  l->r
 *
 */
PNODE ParseLevelF(void)
   {
   PNODE pnExpr, pn1, pn2;
   UINT  uID;
   BOOL  bLoop;
   BOOL  bExpr;

   if (TokPeek() == TOK_OPAREN)
      {
      pn1 = ParseParens (&bExpr);
      if (!bExpr)
         {
         pn1->pn2 = ParseLevelD ();// a type cast
         return pn1;
         }
      }
   else
      {
      pn1 = ParseAtom ();
      }
   bLoop = TRUE;
   while (bLoop)
      {
      switch (uID = TokPeek ())
         {
         case TOK_PERIOD:
         case TOK_ARROW:
            TokGet ();
            pn2 = ParseAtom ();
            pn1 = NodeBuild2 (uID, pn1, pn2);
            break;

         case TOK_OBRACKET:
            TokGet ();
            pnExpr = ParseExpr ();
            TokEat (TOK_CBRACKET);
            pn2 = NodeBuild2 (TOK_PLUS, pn1, pnExpr);
            pn1 = NodeBuild1 (TOK_DEREF, pn2);
            break;

         default:
            bLoop = FALSE;
         }
      }
   return pn1;
   }


/*
 * post unaries r->l
 */
PNODE ParseLevelE(void)
   {
   PNODE pn1;
   UINT  uID;

   pn1 = ParseLevelF ();
   switch (uID = TokPeek ())
      {
      case TOK_INCREMENT: // post increment
      case TOK_DECREMENT: // post increment
         TokGet ();
         return NodeBuild2 (uID, 0, pn1);

      default:
         return pn1;
      }
   }


/*
 * unaries   r->l
 *  ! ~ ++ -- * & -
 *  (type) sizeof :  later
 */
PNODE ParseLevelD(void)
   {
   PNODE pn1;
   UINT  uID;

   switch (uID = TokPeek ())
      {
      case TOK_EXCLAMATION:
      case TOK_TILDA:
      case TOK_INCREMENT: // pre increment
      case TOK_DECREMENT: // pre increment
         TokGet ();
         pn1 = ParseLevelD ();
         return NodeBuild1 (uID, pn1);

      case TOK_STAR:
         TokGet ();
         pn1 = ParseLevelD ();
         return NodeBuild1 (TOK_DEREF, pn1);

      case TOK_AMPERSAND:
         TokGet ();
         pn1 = ParseLevelD ();
         return NodeBuild1 (TOK_ADDROF, pn1);

      case TOK_MINUS:
         TokGet ();
         pn1 = ParseLevelD ();

         if (pn1->uID == TOK_LITERAL)
            {
            if (pn1->pn1->uID == TOK_SHORTLITERAL || 
                pn1->pn1->uID == TOK_LONGLITERAL  ||
                pn1->pn1->uID == TOK_INTLITERAL)
               {
               pn1->pn1->val.l = -pn1->pn1->val.l;
               return pn1;
               }
            else if (pn1->pn1->uID == TOK_FLOATLITERAL)
               {
               pn1->pn1->val.bg = -pn1->pn1->val.bg;
               return pn1;
               }
            }
         return NodeBuild1 (uID, pn1);

//      case TOK_OPAREN:
//         pn1 = ParseParens ();
//         if (pn1->uID != TOK_CAST)
//            return pn1; // a parenthisized expression
//
//         pn1->pn2 = ParseLevelD ();  // a type cast
//         return pn1;

      default:
         return ParseLevelE ();
      }
   }


/*
 * * / %   l->r
 */
PNODE ParseLevelC (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevelD ();
   while (uID = TokTry4(TOK_STAR,   
                          TOK_SLASH,  
                          TOK_PERCENT,
                          0))
      {
      pn2 = ParseLevelD ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * + -   l->r
 */
PNODE ParseLevelB (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevelC ();
   while (uID = TokTry2(TOK_PLUS,
                          TOK_MINUS))
      {
      pn2 = ParseLevelC ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * shift   l->r
 */
PNODE ParseLevelA(void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevelB ();
   while (uID = TokTry2(TOK_SHIFTRIGHT,
                          TOK_SHIFTLEFT))
      {
      pn2 = ParseLevelB ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * comparison   l->r
 */
PNODE ParseLevel9(void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevelA ();
   while (uID = TokTry4 (TOK_LESSTHAN,    
                           TOK_GREATERTHAN,    
                           TOK_LESSOREQUAL, 
                           TOK_GREATEROREQUAL))
      {
      pn2 = ParseLevelA ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * equivalence   l->r
 */
PNODE ParseLevel8(void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel9 ();
   while (uID = TokTry2(TOK_EQUIVALENT, 
                          TOK_NOTEQUAL))
      {
      pn2 = ParseLevel9 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * and   l->r
 */
PNODE ParseLevel7 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel8 ();
   while (uID = TokTry(TOK_AMPERSAND))
      {
      pn2 = ParseLevel8 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * xor   l->r
 */
PNODE ParseLevel6 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel7 ();
   while (uID = TokTry(TOK_HAT))
      {
      pn2 = ParseLevel7 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * or   l->r
 */
PNODE ParseLevel5 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel6 ();
   while (uID = TokTry(TOK_OR))
      {
      pn2 = ParseLevel6 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }



/*
 * logical and   l->r
 */
PNODE ParseLevel4 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel5 ();
   while (uID = TokTry(TOK_LOGICALAND))
      {
      pn2 = ParseLevel5 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * logical or   l->r
 */
PNODE ParseLevel3 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel4 ();
   while (uID = TokTry(TOK_LOGICALOR))
      {
      pn2 = ParseLevel4 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }


/*
 * ternary  r->l
 */
PNODE ParseLevel2 (void)
   {
   PNODE pn1, pn2, pn3;

   pn3 = ParseLevel3 ();         // notice arrangement!
   if (!TokTry(TOK_QUESTION))
      return pn3;

   pn1 = ParseLevel3 ();
   TokEat (TOK_COLON);
   pn2 = ParseLevel3 ();

   return NodeBuild3 (TOK_QUESTION, pn1, pn2, pn3);
   }


/*
 * assignments  r->l
 */
PNODE ParseLevel1 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel2 ();
   switch (uID = TokPeek ())
      {
      case TOK_EQUALS:
      case TOK_PLUSEQUAL:
      case TOK_MINUSEQUAL:
      case TOK_STAREQUAL:
      case TOK_SLASHEQUAL:
      case TOK_PERCENTEQUAL:
      case TOK_SHIFTRIGHTEQUAL:
      case TOK_SHIFTLEFTEQUAL:
      case TOK_ANDEQUAL:
      case TOK_XOREQUAL:
      case TOK_OREQUAL:
         TokGet ();
         pn2 = ParseLevel1 ();
         return NodeBuild2 (uID, pn1, pn2);

      default:
         return pn1;
      }
   }


/*
 * ','  l->r
 */
PNODE ParseLevel0 (void)
   {
   PNODE pn1, pn2;
   UINT  uID;

   pn1 = ParseLevel1 ();
   while (uID = TokTry(TOK_COMMA))
      {
      pn2 = ParseLevel1 ();
      pn1 = NodeBuild2 (uID, pn1, pn2);
      }
   return pn1;
   }



/* prec  operations                          assoc
 * -----------------------------------------------
 * atom  literal identifier
 * f     ()                                  l->r
 * e     ++ -- (post op)                     r->l
 * d     ! ~ - ++ --                         r->l
 * c     * / %                               l->r
 * b     + -                                 l->r
 * a     << >>                               l->r
 * 9     < > <= >=                           l->r
 * 8     == !=                               l->r
 * 7     &                                   l->r
 * 6     ^                                   l->r
 * 5     |                                   l->r
 * 4     &&                                  l->r
 * 3     ||                                  l->r
 * 2     ?:                                  r->l
 * 1     = += -= *= /= %= >>= <<= &= ^= |=   r->l
 * 0     ,                                   l->r
 */
PNODE ParseExpr (void)
   {
   return ParseLevel0 ();
   }


/*
 * adds expression prefix
 *
 */
PNODE ParseExpression (void)
   {
   PNODE pExpr;

   pExpr = ParseExpr ();
   return NodeBuild1 (TOK_EXPR, pExpr);

   }


/*
 * adds expression prefix
 *
 */
PNODE ParseNoCommaExpr (void)
   {
   PNODE pExpr;

   pExpr = ParseLevel1 ();
   return NodeBuild1 (TOK_EXPR, pExpr);
   }



PNODE ParseMaybeExpression (UINT uBlankTok)
   {
   if (TokPeek () == uBlankTok)
      return NULL;

   return ParseExpression ();
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

PNODE ParseIfStatement (void)
   {
   PNODE pExpr, pStmt1, pStmt2 = NULL;

   TokEat (TOK_IF);
   TokEat (TOK_OPAREN);
   pExpr = ParseExpression ();
   TokEat (TOK_CPAREN);
   pStmt1 = ParseStatement ();

   if (TokTry (TOK_ELSE))
      pStmt2 = ParseStatement ();

   return NodeBuild4 (TOK_IF, pExpr, 0, pStmt2, pStmt1);
   }


PNODE ParseWhileStatement (void)
   {
   PNODE pExpr, pStmt;

   TokEat (TOK_WHILE);
   TokEat (TOK_OPAREN);
   pExpr = ParseExpression ();
   TokEat (TOK_CPAREN);
   pStmt = ParseStatement ();

   return NodeBuild4 (TOK_WHILE, pExpr, 0, 0, pStmt);
   }


PNODE ParseForStatement (void)
   {
   PNODE pExpr1, pExpr2, pExpr3, pStmt;

   TokEat (TOK_FOR);
   TokEat (TOK_OPAREN);
   pExpr1 = ParseMaybeExpression (TOK_SEMICOLON);
   TokEat (TOK_SEMICOLON);
   pExpr2 = ParseMaybeExpression (TOK_SEMICOLON);
   TokEat (TOK_SEMICOLON);
   pExpr3 = ParseMaybeExpression (TOK_CPAREN);
   TokEat (TOK_CPAREN);
   pStmt = ParseStatement ();

   return NodeBuild4 (TOK_FOR, pExpr1, pExpr2, pExpr3, pStmt);
   }


PNODE ParseSwitchStatement (void)
   {
   PNODE pExpr, pStmt;

   TokEat (TOK_SWITCH);
   TokEat (TOK_OPAREN);
   pExpr = ParseExpression ();
   TokEat (TOK_CPAREN);
   pStmt = ParseStatement ();

   return NodeBuild4 (TOK_SWITCH, pExpr, 0, 0, pStmt);
   }


PNODE ParseCaseStatement (void)
   {
   PNODE pExpr;

   TokEat (TOK_CASE);
   pExpr = ParseExpression ();
   TokEat (TOK_COLON);
   return NodeBuild1 (TOK_CASE, pExpr);
   }


PNODE ParseDefaultStatement (void)
   {
   TokEat (TOK_DEFAULT);
   TokEat (TOK_COLON);
   return NodeBuild1 (TOK_DEFAULT, NULL);
   }


PNODE ParseBreakStatement (void)
   {
   TokEat (TOK_BREAK);
   TokEat (TOK_SEMICOLON);

   return NodeBuild1 (TOK_BREAK, NULL);
   }


PNODE ParseContinueStatement (void)
   {
   TokEat (TOK_CONTINUE);
   TokEat (TOK_SEMICOLON);

   return NodeBuild1 (TOK_CONTINUE, NULL);
   }


/*
 * if we build the node immediately after getting the token
 * the node will have file name and file line info.
 */
PNODE ParseReturnStatement (void)
   {
   PNODE pRet, pExp = NULL;

   TokEat (TOK_RETURN);
   pRet = NodeBuild ();

   if (TokPeek () != TOK_SEMICOLON)
      pExp = ParseMaybeExpression (TOK_SEMICOLON);
   TokEat (TOK_SEMICOLON);

   pRet->pn1 = pExp;
   return pRet;
   }


PNODE ParseExpressionStatement (void)
   {
   PNODE pExpr = NULL;

   pExpr = ParseExpression ();
   TokEat (TOK_SEMICOLON);
   return pExpr;
   }



PNODE ParseStatement (void)
   {
   switch (TokPeek ())
      {
      case TOK_OBRACE:
         return ParseCompoundStatement ();

      case TOK_IF:
         return ParseIfStatement ();

      case TOK_WHILE:
         return ParseWhileStatement ();

      case TOK_FOR:
         return ParseForStatement ();

      case TOK_SWITCH:
         return ParseSwitchStatement ();

      case TOK_CASE:
         return ParseCaseStatement ();

      case TOK_DEFAULT:
         return ParseDefaultStatement ();

      case TOK_BREAK:
         return ParseBreakStatement ();

      case TOK_CONTINUE:
         return ParseContinueStatement ();

      case TOK_RETURN:
         return ParseReturnStatement ();

      case TOK_SEMICOLON:
         TokEat (TOK_SEMICOLON);
         return NULL;

      default:
         if (TypIsType ())
            return ParseDeclarationStatement ();
         else
            return ParseExpressionStatement ();
      }
   }



PNODE ParseCompoundStatement (void)
   {
   PNODE pStmt, pHead = NULL;

   TokEat (TOK_OBRACE);

   while (TRUE)
      {
      if (TokTry (TOK_CBRACE))
         break;

      pStmt = ParseStatement ();
      pHead = NodeAddToEnd (pHead, pStmt);
      }
   return NodeBuild4 (TOK_COMPOUNDSTATEMENT, 0, 0, 0, pHead);
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


PNODE ParseFormal (void)
   {
   PNODE pType, pIdent, pVar, pJunk;
   INT   iDeref;

   pType = ParseTypespec ();

   for (iDeref=0; TokTry (TOK_STAR); iDeref++)
      ;

   if (pType->uID == TOK_VOID && !iDeref)
      {
      NodeFree (pType);
      return NULL;
      }

   TokEat (TOK_IDENTIFIER);
   pIdent = NodeBuild ();

   /*---new---*/
   SetScope (pIdent);

   if (TokTry (TOK_OBRACKET))
      {
      /*--- allow a value in the array brackets, but ignore it! ---*/
      pJunk = ParseMaybeExpression (TOK_CBRACKET);
      TokEat (TOK_CBRACKET);
      iDeref++;
      pJunk = NodeFreeTree (pJunk);
      }

   pIdent->ps->uTypeDeref = iDeref;
   pVar = NodeBuild1 (TOK_VARIABLE, pIdent);

   return NodeBuild2 (TOK_DECLARATIONLIST, pType, pVar);
   }



PNODE ParseFormalList (void)
   {
   PNODE pDecl, pHead = NULL;

   TokEat (TOK_OPAREN);
   while (TRUE)
      {
      if (!(pDecl = ParseFormal ()))
         break;

      pHead = NodeAddToEnd (pHead, pDecl);
      if (!TokTry (TOK_COMMA))
         break;
      }
   TokEat (TOK_CPAREN);
   return pHead;
   }



PNODE ParseInitializer (BOOL bArray)
   {
   PNODE pExpr, pHead = NULL;

   if (!bArray || TokPeek () == TOK_STRINGLITERAL)
      return ParseExpression ();

   TokEat (TOK_OBRACE);
   while (TRUE)
      {
      pExpr = ParseNoCommaExpr ();
      pHead  = NodeAddToEnd (pHead, pExpr);
      if (!TokTry (TOK_COMMA))
         break;
      }
   TokEat (TOK_CBRACE);
   return pHead;
   }


//
//PNODE ParseArrayBounds (void)
//   {
//   if (!TokTry (TOK_OBRACKET)) // an array decl?
//      return NULL; 
//
//   pASize = ParseMaybeExpression (TOK_CBRACKET);
//
//   // we need pASize !NULL so we know its an array
//   if (!pASize) 
//      pASize = NodeBuild1 (TOK_VOID, NULL); // unsized array
//   TokEat (TOK_CBRACKET);
//
//   return pASize;
//   }
//
///*
// *
// *
// *
// *
// *
// *
// *
// */
//PNODE ParseArrayDeclarator (void)
//   {
//   if (!(pASize = ParseArrayBounds ()))
//      return NULL;
//
//   while (pASize2 = ParseArrayBounds ())
//      {
//      pTyp->pn1->ps = SymAdd ("__unnamed", 1);
//      }
//
//   if (TokTry (TOK_OBRACKET)) // an array decl?
//      {
//      pASize = ParseMaybeExpression (TOK_CBRACKET);
//
//      // we need pASize !NULL so we know its an array
//      if (!pASize) 
//         pASize = NodeBuild1 (TOK_VOID, NULL); // unsized array
//      TokEat (TOK_CBRACKET);
//      }
//
//   }
//
/*
 * This fn returns a bool param to tell if the
 * production is really reading a fn definition. 
 * This is a cheat to allow function definitions
 * be parsed the same as var/fn declarations.
 *
 */
PNODE ParseDeclarator (PBOOL pbDefinition)
   {
   PNODE pIdent, pParm;
   PNODE pInternalID=NULL, pBody=NULL;
   PNODE pASize=NULL, pInit=NULL;
   UINT  uDeref;

   *pbDefinition = FALSE;

   for (uDeref=0; TokTry (TOK_STAR); uDeref++)
      ;

   TokEat (TOK_IDENTIFIER);
   pIdent = NodeBuild ();

   if (TokPeek () == TOK_OPAREN)
      {
      *pbDefinition = TRUE;
      PushScope (SCOPE_PARAM);
      pParm = ParseFormalList ();
      PopScope ();
      if (!TokTry (TOK_SEMICOLON))
         {
         if (TokTry (TOK_COLON))
            {
            if (TokEat (TOK_INTLITERAL))
            pInternalID = NodeBuild ();

            TokEat (TOK_SEMICOLON);
            }
         else
            {
            PushScope (SCOPE_LOCAL);
            pBody = ParseCompoundStatement ();
            PopScope ();
            *pbDefinition = TRUE;
            }
         }
      pIdent->ps->uTypeDeref = uDeref;
      return NodeBuild4 (TOK_FUNCTION, pIdent, pParm, pInternalID, pBody);
      }
   else
      {
      if (TokTry (TOK_OBRACKET)) // an array decl?
         {
         pASize = ParseMaybeExpression (TOK_CBRACKET);

         // we need pASize !NULL so we know its an array
         if (!pASize) 
            pASize = NodeBuild1 (TOK_VOID, NULL); // unsized array
         TokEat (TOK_CBRACKET);
         }
      SetScope (pIdent); // this potentially replaces the symbol with a local copy
      pIdent->ps->uTypeDeref = uDeref;
      *pbDefinition = FALSE;
      if (TokTry (TOK_EQUALS))
         pInit = ParseInitializer (!!pASize);

      return NodeBuild3 (TOK_VARIABLE, pIdent, pInit, pASize);
      }
   }


PNODE ParseDeclaratorList (PBOOL pbDefn)
   {
   PNODE pDecl, pHead = NULL;

   while (TRUE)
      {
      pDecl = ParseDeclarator (pbDefn);
      pHead = NodeAddToEnd (pHead, pDecl);

      if (*pbDefn || !TokTry(TOK_COMMA))
         break;
      }
   return pHead;
   }

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


PNODE ParseStructDefinition (void)
   {
   PNODE pList, pHead = NULL;
   BOOL  bDefinition;

   TokEat (TOK_OBRACE);
   while (TRUE)
      {
      pList = ParseDeclaration (&bDefinition);
      if (bDefinition)
         NodeError (pList, "function definition in a struct?");
      TokEat (TOK_SEMICOLON);

      pHead = NodeAddToEnd (pHead, pList);
      if (TokTry (TOK_CBRACE))
         break;
      }
   return pHead;
   }


PNODE ParseTypespec (void)
   {
   PNODE pTyp;

   TokGet ();
   pTyp = NodeBuild ();

   /*--- basic typespec like int or char ---*/
   if (TypIsBaseType (pTyp->uID))
      return pTyp;

   /*-- previously defined user defined typedef like PSZ ---*/
   if (pTyp->uID == TOK_IDENTIFIER && pTyp->ps->uKind == KIND_TYPEDEF)
      return pTyp;


   if (pTyp->uID == TOK_IDENTIFIER)
      NodeError (pTyp, "datatype or 'struct' expected, got '%s'", 
                  pTyp->ps->pszLiteral);

   if (pTyp->uID != TOK_STRUCT && pTyp->uID != TOK_UNION)
      NodeError (pTyp, "datatype, 'struct' or 'union' expected, got '%s'", 
                 Token.pszString);

   /*--- struct ---*/
   if (TokPeek () == TOK_OBRACE) /*--- unnamed struct definition ---*/
      {
      pTyp->pn1 = NodeBuild ();
      NodeBuild1 (TOK_IDENTIFIER, NULL);
      pTyp->pn1->ps = SymAdd ("__unnamed", 1);
      }
   else
      {
      TokEat (TOK_IDENTIFIER);
      pTyp->pn1 = NodeBuild ();
      pTyp->pn1->ps = NotStructElement (pTyp->pn1->ps);
   
      if (pTyp->pn1->ps->uKind && pTyp->pn1->ps->uKind != KIND_STRUCTDEF)
         ParseError ("identifier not a struct: %s", pTyp->pn1->ps->pszLiteral);
      }
   pTyp->pn1->ps->uKind  = KIND_STRUCTDEF; // we need this for future parsing
   pTyp->pn1->ps->bUnion = (pTyp->uID == TOK_UNION);

   if (TokPeek () == TOK_OBRACE) /*--- struct definition ---*/
      {
      PushScope (SCOPE_STRUCT);

      if (pTyp->pn1->ps->ppElements)
         ParseError ("struct redefinition: %s", pTyp->pn1->ps->pszLiteral);

      pTyp->pn2 = ParseStructDefinition ();
      CheckStructDef (pTyp);
      PopScope ();
      }
   return pTyp;
   }


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


PNODE ParseDeclaration (PBOOL pbDefn)
   {
   PNODE pnode, pType, pList;

   pType = ParseTypespec ();

// typespec could be a struct defn, and decl vars would be optional

   pList = ParseDeclaratorList (pbDefn);
   pnode = NodeBuild2 (TOK_DECLARATIONLIST, pType, pList);
   return pnode;
   }



PNODE ParseDeclarationStatement (void)
   {
   PNODE pDecl;
   BOOL  bDefinition;

   pDecl = ParseDeclaration (&bDefinition);

   if (!bDefinition)
      TokEat (TOK_SEMICOLON);

   return pDecl;
   }



PNODE ParseTypedef (void)
   {
   PNODE pType, pIdent, pASize=NULL;
   INT   iDeref;

   TokEat (TOK_TYPEDEF);
   pType = ParseTypespec ();

   for (iDeref=0; TokTry (TOK_STAR); iDeref++)
      ;
   TokEat (TOK_IDENTIFIER);
   pIdent = NodeBuild ();
   pIdent->ps = NotStructElement (pIdent->ps);
   pIdent->ps->uTypeDeref = iDeref;

   if (pIdent->ps->uKind)
      ParseError ("identifier already defined: %s", pIdent->ps->pszLiteral);
   else
      pIdent->ps->uKind = KIND_TYPEDEF; // we need this for future parsing

   if (TokTry (TOK_OBRACKET)) // an array decl?
      {
      pASize = ParseMaybeExpression (TOK_CBRACKET);

      // we need pASize !NULL so we know its an array
      if (!pASize) 
         pASize = NodeBuild1 (TOK_VOID, NULL); // unsized array
      TokEat (TOK_CBRACKET);
      }
   TokEat (TOK_SEMICOLON);

   return NodeBuild3 (TOK_TYPEDEF, pType, pIdent, pASize);
   }


/*
 * The compilation process is driven from here
 * Heres the drill:
 * loop
 *    {
 *    parse declaration
 *    check declaration
 *    generate declaration
 *    free parse tree
 *    }
 * dump globals
 *
 * no linking takes place, functions are generated one at a time
 * globals and statics are stored and dumped in a block at the end
 *
 */
void CompileDriver (void)
   {
   PNODE pnode;

   PushScope (SCOPE_GLOBAL);
   while (TRUE)
      {
      TokPeek ();
      if (Token.bEOF)
         break;

      if (Token.uID == TOK_TYPEDEF)
         {
         pnode = ParseTypedef ();
         CheckTypedef (pnode);
         }
      else
         {
         pnode = ParseDeclarationStatement ();

         if (LOG_VAL & 0x08)
            PrintTree (pnode, 0);

         CheckDeclaration (pnode);
         GenDeclaration (pnode);
         }
      }
   GenGlobals ();
   PopScope ();
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/



void indent (UINT uIndent)
   {
   for (; uIndent; uIndent--)
      Log (8, " ");
   }


void PTree (PNODE pnode, UINT uIndent)
   {
   if (!pnode)
      return;

   Log (8, "ID:%3d  STR:%-16s  LIT:%-24s\n", 
            pnode->uID, TokReservedString(pnode->uID), (pnode->ps ? pnode->ps->pszLiteral : ""));
   if (pnode->pn1) 
      {indent (uIndent); Log (8, "p1:"); PTree (pnode->pn1, uIndent+3);}
   if (pnode->pn2) 
      {indent (uIndent); Log (8, "p2:"); PTree (pnode->pn2, uIndent+3);}
   if (pnode->pn3) 
      {indent (uIndent); Log (8, "p3:"); PTree (pnode->pn3, uIndent+3);}
   if (pnode->pn4) 
      {indent (uIndent); Log (8, "p4:"); PTree (pnode->pn4, uIndent+3);}
   if (pnode->next)
      {
      indent (uIndent); 
      Log (8, "xt: <next>\n");
      indent (uIndent); 
      PTree (pnode->next, uIndent);
      }
   }

void PrintTree (PNODE pnode, UINT uIndent)
   {
   Log (8, "************************[ParseTree]**************************\n");
   PTree (pnode, uIndent);
   }

