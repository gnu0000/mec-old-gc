/*
 *
 * genfn.c
 * Thursday, 2/27/1997.
 * Craig Fitzgerald
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <assert.h>
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
#include "genfn.h"
#include "genglob.h"
#include "genexpr.h"
#include "genop.h"
#include "gen.h"
#include "opcodes.h"
#include "binfile.h"
#include "type.h"
#include "error.h"
#include "mem.h"
#include "node.h"

#define STATUS_AREA_SIZE 2


typedef struct _case
   {
   LONG lVal;
   PLBL pLabel;
   struct _case *next;
   } CASE;
typedef CASE *PCASE;


typedef struct _sw
   {
   PLBL   pLabel;
   PCASE  pCase;
   PCASE  pDefault;
   struct _sw *next;
   } SWITCH;
typedef SWITCH *PSWITCH;


UINT CalcLocalOffsetList (PNODE pCmpStmt, UINT uLocalOffset);
void GenCmpStmt (PNODE pCmpStmt);
void GenStatement (PNODE pStmt);

static PSWITCH pSTACK_SWITCH = NULL;

static PLBL _pFNRET;         // Label to end of current function


/***************************************************************************/
/*                                                                         */
/* Fn Offsets calculations                                                 */
/*                                                                         */
/***************************************************************************/


static void CalcParmOffsets (PNODE pDLH)
   {
   INT   iOffset;
   PNODE pDecL, pV;
   PSYM  ps;

   if (!pDLH)
      return;

   iOffset = -2 - STATUS_AREA_SIZE;

   for (pDecL = pDLH; pDecL; pDecL = pDecL->next)
      for (pV = pDecL->pn2; pV; pV = pV->next)
         {
         ps = pV->pn1->ps;
         iOffset -= MemStkSize (ps->uTypeVal, ps->psStruct);
         ps->iAddr = iOffset;
         }
   }


static UINT CalcOffsets (PNODE pDCL, UINT uLocalOffset)
   {
   PNODE pV;
   PSYM  ps;
   LONG  l;

   for (pV = pDCL->pn2; pV; pV = pV->next)
      {
      ps = pV->pn1->ps;
      ps->iAddr = uLocalOffset;

      if (ps->bArray)
         {
         l = (LONG)ps->uArraySize * 
            (LONG) MemSize (TypPtrBase (ps->uTypeVal), ps->psStruct);
         if (l > 32767L - (LONG)uLocalOffset)
            NodeError (pDCL, "Array too big");

         uLocalOffset += (UINT)l / STACK_ELEMENT_SIZE +
                         ((UINT)l % STACK_ELEMENT_SIZE ? 1 : 0);
         }
      else
         {
         uLocalOffset += MemStkSize (ps->uTypeVal, ps->psStruct);
         }
      }
   return uLocalOffset;
   }


static UINT CalcLocalOffsets (PNODE pStmt, UINT uLocalOffset)
   {
   if (!pStmt)
      return uLocalOffset;

   switch (pStmt->uID)
      {
      case TOK_EXPR:
      case TOK_BREAK:
      case TOK_CONTINUE:
      case TOK_RETURN:
         break;

      case TOK_WHILE:
      case TOK_FOR:
      case TOK_SWITCH:
      case TOK_CASE:
      case TOK_DEFAULT:
         uLocalOffset = CalcLocalOffsets (pStmt->pn4, uLocalOffset);
         break;

      case TOK_IF:
         uLocalOffset = CalcLocalOffsets (pStmt->pn4, uLocalOffset);
         uLocalOffset = CalcLocalOffsets (pStmt->pn3, uLocalOffset);
         break;

      case TOK_COMPOUNDSTATEMENT:
         uLocalOffset = CalcLocalOffsetList (pStmt, uLocalOffset);
         break;
         
      case TOK_DECLARATIONLIST:
         uLocalOffset = CalcOffsets (pStmt, uLocalOffset);
         break;

      default:
         Error ("Internal Error: Unknown statement id: '%d'", pStmt->uID);
      }
   return uLocalOffset;
   }


static UINT CalcLocalOffsetList (PNODE pCmpStmt, UINT uLocalOffset)
   {
   PNODE pStmt;

   if (!pCmpStmt)
      return uLocalOffset;
   for (pStmt = pCmpStmt->pn4; pStmt; pStmt = pStmt->next)
      uLocalOffset = CalcLocalOffsets (pStmt, uLocalOffset);
   return uLocalOffset;
   }

/***************************************************************************/
/*                                                                         */
/* Code generation routines                                                */
/*                                                                         */
/***************************************************************************/

static void GenExpr (PNODE pStmt)
   {
   if (pStmt)
      {
      GenExpression (pStmt);
      AddOpS (OP_DSP, 0, MemStkSize (pStmt->uTypeVal, FindStructType (pStmt, 0)));
      }
   }


static void GenBreak (PNODE pStmt)
   {
   AddOpL (OP_JMP, 0, LabBreak ());
   }


static void GenContinue (PNODE pStmt)
   {
   AddOpL (OP_JMP, 0, LabContinue ());
   }


static void GenReturn (PNODE pStmt)
   {
   if (pStmt->pn1)
      GenExpression (pStmt->pn1);
   AddOpL (OP_JMP, 0, _pFNRET);
   }
   

static void GenIf (PNODE pStmt)
   {
   PLBL pF, pE;

   if (pStmt->pn3) // has else clause
      {
      pF = LabForward (); // false label
      pE = LabForward (); // end label

      GenCondition (pStmt->pn1, NULL, pF);
      GenStatement (pStmt->pn4);              // TRUE statement
      AddOpL (OP_JMP, 0, pE);
      LabSet (pF);
      GenStatement (pStmt->pn3);              // FALSE statement
      LabSet (pE);
      LabFree (pE);
      LabFree (pF);
      }
   else // no else clause
      {
      pE = LabForward (); // end label
      GenCondition (pStmt->pn1, NULL, pE);
      GenStatement (pStmt->pn4);              // TRUE statement
      LabSet (pE);
      LabFree (pE);
      }
   }


static void GenWhile (PNODE pStmt)
   {
   LabPushContinue ();
   LabSet (LabContinue ());
   LabPushBreak ();

   GenCondition (pStmt->pn1, NULL, LabBreak ());
   GenStatement (pStmt->pn4);
   AddOpL (OP_JMP, 0, LabContinue ());

   LabPopContinue ();
   LabPopBreak ();
   }


   
static void GenFor (PNODE pStmt)
   {
   GenExpr (pStmt->pn1);

   LabPushContinue ();
   LabSet (LabContinue ());
   LabPushBreak ();

   GenCondition (pStmt->pn2, NULL, LabBreak ());
   GenStatement (pStmt->pn4);
   GenExpr (pStmt->pn3);
   AddOpL (OP_JMP, 0, LabContinue ());

   LabPopContinue ();
   LabPopBreak ();
   }

/***************************************************************************/


static void FreeCases (PCASE pCase)
   {
   PCASE pTmp, pNext;

   for (pTmp = pCase; pTmp; pTmp = pNext)
      {
      pNext = pTmp->next;
      MemFreeData (pTmp);
      }
   }


static void PushSwitch ()
   {
   PSWITCH pSwitch;

   LabPushBreak ();

   pSwitch = calloc (1, sizeof (SWITCH));
   pSwitch->pLabel = LabBreak ();
   pSwitch->next   = pSTACK_SWITCH;
   pSTACK_SWITCH   = pSwitch;
   }


static void PopSwitch ()
   {
   PSWITCH pSwitch;

   pSwitch = pSTACK_SWITCH->next;
   FreeCases (pSTACK_SWITCH->pCase);
   MemFreeData (pSTACK_SWITCH);
   pSTACK_SWITCH = pSwitch;

   LabPopBreak ();
   }


static PSWITCH CurrentSwitch (void)
   {
   return pSTACK_SWITCH;
   }


/***************************************************************************/

static void GenCase (PNODE pnode)
   {
   PCASE pCase, pTmp;

   pCase = calloc (1, sizeof (CASE));
   pCase->pLabel = LabHere ();
   pCase->lVal   = pnode->pn1->pn1->pn1->val.l;

   for (pTmp = pSTACK_SWITCH->pCase; pTmp && pTmp->next; pTmp=pTmp->next)
      ;
   if (pTmp)
      pTmp->next = pCase;
   else
      pSTACK_SWITCH->pCase = pCase;
   }


static void GenDefault (PNODE pnode)
   {
   PCASE pCase;

   pCase = calloc (1, sizeof (CASE));
   pCase->pLabel = LabHere ();

   if (pSTACK_SWITCH->pDefault)
      NodeError (pnode, "default already defined");

   pSTACK_SWITCH->pDefault = pCase;
   }


static void GenSwitch (PNODE pStmt)
   {
   PLBL  pStart;
   PCASE pCase, pTmp;
   UINT  uCount;

   PushSwitch ();
   pStart = LabForward (); // false label
   AddOpL (OP_JMP, 0, pStart);

   GenStatement (pStmt->pn4);
   GenBreak (NULL);

   LabSet (pStart);
   GenExpression (pStmt->pn1);

   pCase = CurrentSwitch()->pCase;
   AddOp (OP_SWITCH, 0);

   for (uCount=0, pTmp=pCase; pTmp; pTmp=pTmp->next)
      uCount++;
   AddShort (uCount);
   AddShort (!!CurrentSwitch()->pDefault);

   for (pTmp=pCase; pTmp; pTmp=pTmp->next)
      {
      AddWord (pTmp->lVal);
      AddShort (LabPos(pTmp->pLabel));
      }
   if (pTmp = CurrentSwitch()->pDefault)
      {
      AddShort (LabPos(pTmp->pLabel));
      }
   PopSwitch ();
   }



static void GenDeclarationList (PNODE pDLST)
   {
   PNODE pVar, pInit;

   if (!pDLST)
      return;

   for (pVar = pDLST->pn2; pVar; pVar = pVar->next)
      if (pInit = pVar->pn2)
         GenExpression (pInit);
   }


static void GenStatement (PNODE pStmt)
   {
   if (!pStmt)
      return;

   switch (pStmt->uID)
      {
      case TOK_EXPR:             GenExpr        (pStmt); break;
      case TOK_BREAK:            GenBreak       (pStmt); break;
      case TOK_CONTINUE:         GenContinue    (pStmt); break;
      case TOK_RETURN:           GenReturn      (pStmt); break;
      case TOK_COMPOUNDSTATEMENT:GenCmpStmt     (pStmt); break;
      case TOK_IF:               GenIf          (pStmt); break;
      case TOK_WHILE:            GenWhile       (pStmt); break;
      case TOK_FOR:              GenFor         (pStmt); break;
      case TOK_SWITCH:           GenSwitch      (pStmt); break;
      case TOK_CASE:             GenCase        (pStmt); break;
      case TOK_DEFAULT:          GenDefault     (pStmt); break;
      case TOK_DECLARATIONLIST:  GenDeclarationList (pStmt); break;

      default:
         Error ("Internal Error: Unknown statement id: '%d'", pStmt->uID);
      }
   }


static void GenCmpStmt (PNODE pCmpStmt)
   {
   PNODE pStmt;

   if (!pCmpStmt)
      return;

   for (pStmt = pCmpStmt->pn4; pStmt; pStmt = pStmt->next)
      GenStatement (pStmt);
   }


/* 
 *
 */
static void GenFnPrefix (UINT uLocalSize)
   {
   _pFNRET = LabForward ();

   AddOp (OP_PUSHSF, 0);    // push stack frame
   AddOp (OP_PUSHSP, 0);    // push stack pointer
   AddOp (OP_POPSF,  0);    // pop stack frame

   AddOpS (OP_ISP, 0, uLocalSize);
   }

   
/*
 * if falling through to here:
 *   local variables on top of stack
 *   old stack frame under that
 *
 * if jumping to return label:
 *   assume return on top of stack
 *   local variables under that
 *   old stack frame under that
 *
 */
static void GenFnSuffix (UINT uLocalSize, PSYM ps)
   {
   PLBL pE;

   if (ps->uTypeVal == DATATYPE_VOID)       // void fn type - no return value
      {
      LabSet (_pFNRET);                     // return statements go to here
      if (uLocalSize)
         AddOpS    (OP_DSP, 0, uLocalSize); // wipe locals area by DECrementing SP
      AddOp    (OP_POPSF, 0);               // restore prev Stack Frame
      }
   else if (ps->uTypeVal == DATATYPE_STRUCT)// return any # of bytes   
      {
      pE = LabForward ();
      AddOpL (OP_JMP, 0, pE);        // jump past return in case no return stmt
      LabSet (_pFNRET);              // return statements go to here
      AddOpS (OP_PUSHL, 0, -2);      // push return variable index
      AddOpW (OP_PUSHI, 0, MemSize (ps->uTypeVal, ps->psStruct));
      AddOp  (OP_POPNLS, 0);         // pop return val to return variable
      LabSet (pE);                   // non return statements go to here

      if (uLocalSize)
         AddOpS    (OP_DSP, 0, uLocalSize); // wipe locals area by DECrementing SP
      AddOp    (OP_POPSF, 0);               // restore prev Stack Frame
      LabFree (pE);
      }
   else // base type return value - return stmts go to _pFNRET label
      {
      pE = LabForward ();      
      AddOpL (OP_JMP, 0, pE);          // jump past return in case no return stmt
      LabSet (_pFNRET);                // return statements go to here
      AddOpS (OP_PUSHL, 0, -2);        // push return variable index
      AddOp  (OP_POPLS, ps->uTypeVal); // pop return val to return variable
      LabSet (pE);                     // non return statements go to here

      if (uLocalSize)
         AddOpS    (OP_DSP, 0, uLocalSize); // wipe locals area by DECrementing SP
      AddOp    (OP_POPSF, 0);               // restore prev Stack Frame
      LabFree (pE);
      }
   LabFree (_pFNRET);
   }




/* 
 * in check: gen #actual parms, dummy up if not enough to satisfy formals
 *
 * calc address of each parm
 * calc addresses for each local
 * calc size of locals
 *
 */
static void GenGlobalFunction (PNODE pnode)
   {
   PNODE pIdent, pParms, pBody;
   UINT  uLocalSize;

   pIdent = pnode->pn1;
   pParms = pnode->pn2;
   pBody  = pnode->pn4;

   if (!pIdent->ps->bDefined) // if not defined
      return;

   CalcParmOffsets (pParms);
   uLocalSize = CalcLocalOffsets (pBody, 0);

   CodInitBuffer ();
   GenFnPrefix  (uLocalSize);
   GenStatement (pBody);
   GenFnSuffix  (uLocalSize, pIdent->ps);

   GenDumpFunction (pIdent->ps->pszLiteral);
   }
   


/*
 * If the declaration is for a global var, we just keep track of it
 * until we call GenGlobals.
 * 
 * Fn declarations are ignored, that info is kept in the symbol table
 * Fn definitions are produced immediately
 *
 */
void GenDeclaration (PNODE pnode)
   {
   if (pnode->pn2->uID == TOK_VARIABLE)
      {
      StoreGlobalDecl (pnode->pn2);
      NodeFree (pnode->pn1); // Free typedef node - we don't need it
      NodeFree (pnode);      // Free Declaration List node - we don't need it
      }
   else  
      {
      GenGlobalFunction (pnode->pn2);
      pnode = NodeFreeTree (pnode);
      SymDeleteLocals ();
      }
   }

