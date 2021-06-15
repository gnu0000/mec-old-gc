/*
 *
 * mem.c
 * Monday, 3/17/1997.
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
#include "gclib.h"
#include "gc.h"
#include "error.h"
#include "opcodes.h"
#include "type.h"
#include "mem.h"



/*
 * a variable of a given type will take up how many bytes
 * of memory you ask?
 */
UINT MemSize (UINT uTypeVal, PSYM psStruct)
   {
   if (uTypeVal > 255)
      return 4;

   switch (uTypeVal)
      {
      case DATATYPE_VOID  : 
         Error ("looking for void type in MemSize"); //return 0;

      case DATATYPE_CHAR  : 
         return 1;

      case DATATYPE_SHORT : 
         return 2;

      case DATATYPE_LONG  : 
         return 4;

      case DATATYPE_FLOAT : 
         return 8;

      case DATATYPE_PTR   : 
         return 4;

      case DATATYPE_STRING: 
         return 4;

      case DATATYPE_STRUCT: 
         if (!psStruct)
            Error ("internal error: null struct ref in memsize");
         return psStruct->uStructSize;

      default             : 
         Error ("Internal Error: Unknown Type in TypeMSize [%d]", uTypeVal);
      }
   return 4;
   }


/*
 * symbol can be of type var, structdef, or typedef
 *
 */
UINT MemSymSize (PSYM ps)
   {
   if (ps->uKind == KIND_VARIABLE)
      {
      if (ps->bArray)
         return MemSize (TypPtrBase(ps->uTypeVal), ps->psStruct) * ps->uArraySize;
      else
         return MemSize (ps->uTypeVal, ps->psStruct);
      }
   else if (ps->uKind == KIND_STRUCTDEF)
      {
      return MemSize (ps->uTypeVal, ps);
      }
   else // KIND_TYPEDEF
      {
      if (ps->bTArray)
         return MemSize (TypPtrBase(ps->uTypeVal), ps->psStruct) * ps->uTArraySize;
      else
         return MemSize (ps->uTypeVal, ps->psStruct);
      }
   }


/*
 * a variable of a given type will take up how much room
 * on the stack you ask?
 */
UINT MemStkSize (UINT uTypeVal, PSYM psStruct)
   {
   switch (uTypeVal)
      {
      case DATATYPE_CHAR  : 
         return 1;
      case DATATYPE_SHORT : 
         return 1;
      case DATATYPE_LONG  : 
         return 1;
      case DATATYPE_FLOAT : 
         return 2;
      case DATATYPE_STRING: 
         return 1;
      case DATATYPE_VOID  : 
         return 0;

      case DATATYPE_STRUCT: 
         if (!psStruct)
            Error ("internal error: null struct ref in memsize");
         return (psStruct->uStructSize + 3) / 4;

      default             : return 1; // ptr
      }
   }


