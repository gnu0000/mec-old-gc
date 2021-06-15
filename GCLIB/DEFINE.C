/*
 *
 * tokenize.c
 * Friday, 2/21/1997.
 * Craig Fitzgerald
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuFile.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "tokenize.h"
#include "error.h"
#include "define.h"

typedef struct _def
   {
   PSZ pszID;
   PSZ pszString;
   INT iIdx;
   INT iLen;
   struct _def *left;
   struct _def *right;
   } DEF;
typedef DEF *PDEF;

#define DEFINE_BUFFER_LEN 1024

PDEF pDEFTREE  = NULL;
INT  iDEFLEN   = 0; // rel to DEFPTR
CHAR szDEFBUFF [DEFINE_BUFFER_LEN];
PSZ  pszDEFPTR = NULL;
INT  iDEFIDX   = 1;
INT  iDEFCOUNT = 0;

/***************************************************************************/


INT t_getc (void)
   {
   if (!iDEFLEN)
      {
      return f_getc (pINFILE->fp);
      }
   iDEFLEN--;
   return *pszDEFPTR++;
   }

INT t_ungetc (INT c)
   {
   if (!iDEFLEN)
      return f_ungetc (c, pINFILE->fp);

   iDEFLEN++;
   return *(--pszDEFPTR);
   }


/***************************************************************************/

static PSZ GetString (void)
   {
   CHAR szTmp[512];
   INT  i, c;

   for (i=0; i<511; i++)
      {
      c = f_getc (pINFILE->fp);
      if (c == '\n' || c == EOF)
         break;
      szTmp[i] = c;
      }
   szTmp[i] = '\0';
   return strdup (szTmp);
   }


static PDEF _addDefine (PDEF pTree, PSZ pszDef)
   {
   INT i;

   if (!pTree)
      {
      pTree = calloc (1, sizeof (DEF));
      pTree->pszID     = strdup (pszDef);
      pTree->pszString = GetString ();
      pTree->iLen      = strlen (pTree->pszString);
      pTree->iIdx      = iDEFCOUNT++;;
      return pTree;
      }
   i = strcmp (pszDef, pTree->pszID);
   if (i<0)
      pTree->left = _addDefine (pTree->left, pszDef);
   else
      pTree->right = _addDefine (pTree->right, pszDef);
   return pTree;
   }


void DefineAdd (PSZ pszDef)
   {
   pDEFTREE = _addDefine (pDEFTREE, pszDef);
   }


static PDEF _findDefine (PDEF pTree, PSZ pszDef)
   {
   INT i;

   if (!pTree)
      return NULL;
   i = strcmp (pszDef, pTree->pszID);
   if (!i)
      return pTree;
   if (i<0)
      return _findDefine (pTree->left, pszDef);
   else
      return _findDefine (pTree->right, pszDef);
   }

/***************************************************************************/


static INT InDefine (void)
   {
   if (!iDEFLEN)
      return 0;
   return iDEFIDX;
   }


static void InsertDefine (PDEF pDef)
   {
   if (iDEFLEN) // data already there
      {
      if (iDEFLEN + pDef->iLen >= DEFINE_BUFFER_LEN)
         InFileError ("internal error: preprocessor buffer overflow");
      memmove (szDEFBUFF + pDef->iLen, pszDEFPTR, iDEFLEN);
      }
   memcpy  (szDEFBUFF, pDef->pszString,  pDef->iLen);
   iDEFLEN  += pDef->iLen;
   iDEFIDX   = pDef->iIdx;
   pszDEFPTR = szDEFBUFF;
   }



static void _defTerm (PDEF pTree)
   {
   if (!pTree)
      return;

   _defTerm (pTree->left);
   _defTerm (pTree->right);
   MemFreeData (pTree->pszID);
   MemFreeData (pTree->pszString);
   MemFreeData (pTree);
   }


void DefineTerm (void) // terminate defines module
   {
   _defTerm (pDEFTREE);
   pDEFTREE = NULL;
   }

/***************************************************************************/

BOOL DefineCheck (PSZ pszIdent)
   {
   PDEF pDef;

   if (!(pDef = _findDefine (pDEFTREE, pszIdent)))
      return FALSE;

   if (InDefine () && InDefine () <= pDef->iIdx)
      return FALSE;

   InsertDefine (pDef); // add define to input stream;
   return TRUE;
   }                                


/***************************************************************************/
