/*
 *
 * gen.c
 * Thursday, 2/27/1997.
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
#include "gclib.h"
#include "gc.h"
#include "label.h"
#include "binfile.h"
#include "error.h"
#include "genop.h"
#include "gen.h"

void _cdecl _defTag       (UINT uTag);
void _cdecl _defFunction  (PSZ pszName);
void _cdecl _defGlobStart (UINT uLen);
void _cdecl _defGlobData  (PVOID p, UINT uLen);
void _cdecl _defGlobEnd   (void);
                     
void (_cdecl *GenDumpTag)       (UINT uTag)          = _defTag;
void (_cdecl *GenDumpFunction)  (PSZ pszName)        = _defFunction;
void (_cdecl *GenDumpGlobStart) (UINT uLen)          = _defGlobStart;
void (_cdecl *GenDumpGlobData)  (PVOID p, UINT uLen) = _defGlobData; 
void (_cdecl *GenDumpGlobEnd)   (void)               = _defGlobEnd;


static FILE *fpOUT = NULL;


/***************************************************************************/
/*                                                                         */
/* Dump function record                                                    */
/* Dump globals record support routines                                    */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void _cdecl _defTag (UINT uTag)
   {
   FilWriteUShort (fpOUT, uTag);
   }

void _cdecl _defFunction (PSZ pszName)
   {
   PSZ  pszCode;
   UINT uLen;

   pszCode = CodBuffer (&uLen);

   GenDumpTag (BIN_FUNCTIONHEADER);
   FilWriteStr (fpOUT, pszName);
   FilWriteUShort (fpOUT, uLen);
   fwrite (pszCode, 1, uLen, fpOUT);
   GenDumpTag (BIN_TERM);
   }

void _cdecl _defGlobStart (UINT uLen)
   {
   GenDumpTag (BIN_GLOBALHEADER);
   FilWriteUShort (fpOUT, uLen);
   }

void _cdecl _defGlobData (PVOID p, UINT uLen)
   {
   fwrite (p, 1, uLen, fpOUT);
   }

void _cdecl _defGlobEnd (void)
   {
   GenDumpTag (BIN_TERM);
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


void GenSetOutputFn (PVOID pFn, UINT uFn)
   {
   switch (uFn)
      {
      case FTYP_WRITE_FILE_TAGS   : GenDumpTag       = pFn; break;
      case FTYP_WRITE_FN          : GenDumpFunction  = pFn; break;
      case FTYP_WRITE_GLOBAL_START: GenDumpGlobStart = pFn; break;
      case FTYP_WRITE_GLOBAL_DATA : GenDumpGlobData  = pFn; break;
      case FTYP_WRITE_GLOBAL_END  : GenDumpGlobEnd   = pFn; break;
      default:
         Error ("Unknown fn id in SetCompileFunction [%d]", uFn);
      }
   }

void GenInit (PSZ pszOutFile)
   {
   if (pszOutFile)
      {
      if (!(fpOUT = fopen (pszOutFile, "wb")))
         Error ("Cannot open output file '%s'", pszOutFile);
      }
   GenDumpTag (BIN_FILEHEADER);
   }

void GenTerm(void)
   {
   if (fpOUT)
      fclose (fpOUT);
   }

