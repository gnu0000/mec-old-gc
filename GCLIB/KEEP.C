/*
 *
 * keep.c
 * Friday, 3/28/1997.
 * Craig Fitzgerald
 *
 *
 * combines: gen.c  (code generation dump)
 *           loadgx (load exe code)
 *
 * this allows compiler and vm to be combined
 * to make a gc interpreter
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
#include "label.h"
#include "binfile.h"
#include "codmaint.h"
#include "genop.h"
#include "gen.h"

PSZ pszGLOBALS;

INT iGP;

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void _cdecl _keepTag       (UINT uTag)
   {
   }

void _cdecl _keepFunction  (PSZ pszName)
   {
   PSZ  pszCode;
   UINT uLen;
   PFUNC pFunc;

   pszCode = CodBuffer (&uLen);

   pFunc = CodNewFunction ();
   pFunc->pszName = strdup (pszName);
   pFunc->uSize   = uLen;
   pFunc->pszCode = calloc (uLen, 1);
   memcpy (pFunc->pszCode, pszCode, uLen);
   CodAddFunction (pFunc);
   }

void _cdecl _keepGlobStart (UINT uLen)
   {
   pszGLOBALS = calloc (uLen, 1);
   iGP = 0;
   }

void _cdecl _keepGlobData  (PVOID p, UINT uLen)
   {
   memcpy (pszGLOBALS + iGP, p, uLen);
   iGP += uLen;
   }

void _cdecl _keepGlobEnd   (void)
   {
   CodSetGlobalPtr (pszGLOBALS);
   }

