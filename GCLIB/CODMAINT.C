/*
 *
 * codmaint.c
 * Friday, 3/28/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include <GnuFile.h>
#include "gclib.h"
#include "gc.h"
#include "binfile.h"
#include "codmaint.h"


PFUNC pfnTREE = NULL;


static PFUNC _findFunction (PFUNC pTree, PSZ pszName)
   {
   INT i;

   if (!pTree)
      return NULL;

   if (!(i = strcmp (pTree->pszName, pszName)))
      return pTree;

   if (i<0)
      return _findFunction (pTree->left, pszName);
   else
      return _findFunction (pTree->right, pszName);
   }


PFUNC CodFindFunction (PSZ pszName)
   {
   return _findFunction (pfnTREE, pszName);
   }



PFUNC CodNewFunction (void)
   {
   PFUNC pFunc;

   pFunc = calloc (sizeof (FUNC), 1);
   return pFunc;
   }


static PFUNC _addFunction (PFUNC pTree, PFUNC pFunc)
   {
   INT i;

   if (!pTree)
      return pFunc;

   i = strcmp (pTree->pszName, pFunc->pszName);

   if (!i) // replace the old function
      {
      MemFreeData (pTree->pszCode);
      MemFreeData (pFunc->pszName);
      pTree->uSize      = pFunc->uSize     ;
      pTree->pszCode    = pFunc->pszCode   ;
      pTree->pszGlobals = pFunc->pszGlobals;
      MemFreeData (pFunc);
      return pTree;
      }
   else if (i<0)
      pTree->left = _addFunction (pTree->left, pFunc);
   else
      pTree->right = _addFunction (pTree->right, pFunc);

   return pTree;
   }

void CodAddFunction (PFUNC pFunc)
   {
   pfnTREE = _addFunction (pfnTREE, pFunc);
   }


static void _setGlobalPtr (PFUNC pTree, PSZ pszGlobals)
   {
   if (!pTree)
      return;
   pTree->pszGlobals = (pTree->pszGlobals ? pTree->pszGlobals : pszGlobals);
   _setGlobalPtr (pTree->left, pszGlobals);
   _setGlobalPtr (pTree->right, pszGlobals);
   }


/*
 * loading a module ...
 * We first load the functions and then we load the globals
 * this fn sets the globals ptr for all the fns loaded previously
 * that haven't had thier Globals ptr set.
 *
 * If we then load a new module, and that module has its own globals
 * then the functions get thier own globals area.  If the new module
 * doesn't have its own globals, this fn will get called with a NULL
 * and the globals from the first loaded globals area will be assigned
 * to these new functions.
 */
void CodSetGlobalPtr (PSZ pszGlobals)
   {
   if (!pszGlobals)
      {
      if (!pfnTREE || !pfnTREE->pszGlobals)
         return;
      pszGlobals = pfnTREE->pszGlobals;
      }
   _setGlobalPtr (pfnTREE, pszGlobals);
   }
