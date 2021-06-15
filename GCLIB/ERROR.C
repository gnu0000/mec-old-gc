/*
 *
 * error.c
 * Friday, 2/28/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuFile.h>
#include <stdarg.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "tokenize.h"
#include "parse.h"
#include "check.h"


UINT LOG_VAL = 0;


/*
 * uses global: pINFILE
 *
 */
void _cdecl InFileError (PSZ psz, ...)
   {
   va_list vlst;

   if (pINFILE)
      printf ("%s(%d): Error: ", pINFILE->pszFile, (UINT)FilGetLine ());
   else
      printf ("Error: ");

   va_start (vlst, psz);
   vprintf (psz, vlst);
   va_end (vlst);
   printf ("\n");
   exit (1);
   }

/*
 * uses global: Token
 *
 *
 */
void _cdecl ParseError (PSZ psz, ...)
   {
   va_list vlst;

   printf ("%s(%d): Error: ", Token.pszFile, Token.uLine);
   va_start (vlst, psz);
   vprintf (psz, vlst);
   va_end (vlst);
   printf ("\n");
   exit (1);
   }


void _cdecl NodeError (PNODE pnode, PSZ psz, ...)
   {
   va_list vlst;

   while (pnode && pnode->pn1 && !pnode->pszFile)
      pnode = pnode->pn1;
   if (pnode)
      printf ("%s(%d): Error: ", pnode->pszFile, pnode->uLine);
   else
      printf ("Error: ", pnode->pszFile, pnode->uLine);

   va_start (vlst, psz);
   vprintf (psz, vlst);
   va_end (vlst);
   printf ("\n");
   exit (1);
   }


void _cdecl NodeWarn (PNODE pnode, PSZ psz, ...)
   {
   va_list vlst;

   while (pnode && pnode->pn1 && !pnode->pszFile)
      pnode = pnode->pn1;
   if (pnode)
      printf ("%s(%d): Warning: ", pnode->pszFile, pnode->uLine);
   else
      printf ("Warning: ", pnode->pszFile, pnode->uLine);

   va_start (vlst, psz);
   vprintf (psz, vlst);
   va_end (vlst);
   printf ("\n");
   }


void _cdecl Log (UINT uLevel,  PSZ psz, ...)
   {
   va_list vlst;

   if (!(uLevel & LOG_VAL))
      return;

   va_start (vlst, psz);
   vprintf (psz, vlst);
   va_end (vlst);
   }
