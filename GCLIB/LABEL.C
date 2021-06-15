/*
 *
 * label.c
 * Sunday, 3/2/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include <stdarg.h>
#include "gclib.h"
#include "gc.h"
#include "label.h"
#include "genop.h"


/*
 * creates a new label defined to be at this IP
 */
PLBL LabHere (void)
   {
   PLBL pl;
   INT  iIP;

   CodBuffer ((PUINT)(&iIP));

   pl = calloc (sizeof (LBL), 1);
   pl->bDefined = TRUE;
   pl->iIP      = iIP;

   CodClearMerge ();

   return pl;
   }

/*
 * creates a new label whose pos is not defined
 */
PLBL LabForward (void)
   {
   PLBL pl;

   pl = calloc (sizeof (LBL), 1);
   pl->bDefined = FALSE;
   pl->iIP      = 0;
   return pl;
   }

/*
 * if label is defined, returns its position relatve to IP
 * if not defined, 0 is returned, and an entry is kept to
 * update this position when the label is resolved
 */
INT LabPos(PLBL pl)
   {
   PREF pref;
   INT  iIP;

   CodBuffer ((PUINT)&iIP);

   if (pl->bDefined)
      return pl->iIP - iIP - 2;

   pref = calloc (sizeof(REF), 1);
   pref->iRefPos = iIP;
   pref->next = pl->pref;
   pl->pref = pref;
   return 0;
   }

/*
 * this is called to resolve a forward label.  This fn will
 * also backpatch previous references to this label
 */
void LabSet (PLBL pl)
   {
   PREF pref;
   INT  iIP;

   if (pl->bDefined)
      return; // label already set

   CodBuffer ((PUINT)&iIP);

   pl->bDefined = TRUE;
   pl->iIP      = iIP;

   CodClearMerge ();

   for (pref = pl->pref; pref; pref = pref->next)
      SetShort (pref->iRefPos, pl->iIP - pref->iRefPos - 2);
   pl->pref = LabFreeRefs (pl->pref);
   }

PREF LabFreeRefs (PREF pref)
   {
   PREF pTmp, pNext;

   for (pTmp = pref; pTmp; pTmp = pNext)
      {
      pNext = pTmp->next;
      MemFreeData (pTmp);
      }
   return NULL;
   }

void LabFree (PLBL pl)
   {
   LabFreeRefs (pl->pref);
   MemFreeData (pl);
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

/*
 *
 */
PLBL LabPush (PLBL plStack)
   {
   PLBL pL;

   pL       = LabForward ();
   pL->next = plStack;
   plStack  = pL;
   return plStack;
   }

/*
 *
 */
PLBL LabPop (PLBL plStack)
   {
   PLBL  pL;

   if (!plStack)
      Error ("Internal error: pop from empty label stack");

   LabSet (plStack);
   pL = plStack->next;
   LabFree (plStack);
   plStack = pL;
   return plStack;
   }

/*
 *
 */
PLBL LabTop (PLBL plStack)
   {
   if (!plStack)
      Error ("Internal error: reference empty label stack");
   return plStack;
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

static PLBL pSTACK_CONTINUE = NULL;
static PLBL pSTACK_BREAK    = NULL;

void LabPushContinue (void)
   {
   pSTACK_CONTINUE = LabPush (pSTACK_CONTINUE);
   }

void LabPopContinue (void)
   {
   pSTACK_CONTINUE = LabPop (pSTACK_CONTINUE);
   }

PLBL LabContinue (void)
   {
   return LabTop (pSTACK_CONTINUE);
   }

void LabPushBreak (void)
   {
   pSTACK_BREAK = LabPush (pSTACK_BREAK);
   }

void LabPopBreak (void)
   {
   pSTACK_BREAK = LabPop (pSTACK_BREAK);
   }

PLBL LabBreak (void)
   {
   return LabTop (pSTACK_BREAK);
   }

