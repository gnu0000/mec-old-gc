/*
 *
 * compile.c
 * Monday, 3/31/1997.
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
#include "define.h"
#include "symbol.h"
#include "tokenize.h"
#include "parse.h"
#include "gen.h"
#include "error.h"


/*
 *
 *
 */
void Compile (PSZ pszIn, PSZ pszOut)
   {
   SymInit ();
   TokInit (pszIn);
   GenInit (pszOut);

   CompileDriver ();

   GenTerm ();
   TokTerm ();
   SymTerm ();    // terminate symbols module
   DefineTerm (); // terminate defines module
   }


void SetCompileOption (UINT uOption, UINT uVal)
   {
   switch (uOption)
      {
      case CO_OPTIMIZE  :  uOPT = uVal;              break;
      case CO_STRUCTPACK:  uSTRUCT_PACK_SIZE = uVal; break;
      case CO_LOGVAL    :  LOG_VAL = uVal;           break;
      default: Error ("unknown compiler option [%d]", uOption);
      }
   }


