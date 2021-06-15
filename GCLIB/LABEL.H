/*
 *
 * label.h
 * Sunday, 3/2/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 * this module makes use of the globals
 * iIP and pszCODE in GEN.C
 */

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
/* PLBL LabHere (void);
 * PLBL LabForward (void);
 * INT  LabPos(PLBL pl);
 * void LabSet (PLBL pl);
 * PREF LabFreeRefs (PREF pref);
 * void LabFree (PLBL pl);
 *
 * PLBL LabPush (PLBL plStack);
 * PLBL LabPop (PLBL plStack);
 * PLBL LabTop (PLBL plStack);
 *
 * void LabPushContinue (void);
 * void LabPopContinue (void);
 * PLBL LabContinue (void);
 * void LabPushBreak (void);
 * void LabPopBreak (void);
 * PLBL LabBreak (void);
 *
 *
 */




typedef struct _ref
   {
   UINT iRefPos;
   struct _ref *next;
   } REF;
typedef REF *PREF;


typedef struct _lbl
   {
   BOOL bDefined;
   INT  iIP;
   PREF pref;
   struct _lbl *next;
   struct _lbl *nextcase;
   } LBL;
typedef LBL *PLBL;


/*
 * creates a new label defined to be at this IP
 */
PLBL LabHere (void);


/*
 * creates a new label whose pos is not defined
 */
PLBL LabForward (void);


/*
 * if label is defined, returns its position relatve to IP
 * if not defined, 0 is returned, and an entry is kept to
 * update this position when the label is resolved
 */
INT LabPos(PLBL pl);


/*
 * this is called to resolve a forward label.  This fn will
 * also backpatch previous references to this label
 */
void LabSet (PLBL pl);


PREF LabFreeRefs (PREF pref);


void LabFree (PLBL pl);


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

PLBL LabPush (PLBL plStack);
PLBL LabPop (PLBL plStack);
PLBL LabTop (PLBL plStack);

void LabPushContinue (void);
void LabPopContinue (void);
PLBL LabContinue (void);
void LabPushBreak (void);
void LabPopBreak (void);
PLBL LabBreak (void);

