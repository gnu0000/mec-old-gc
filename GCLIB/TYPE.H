/*
 *
 * type.h
 * Wednesday, 3/12/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */


void TypCombineVal (PSYM ps, UINT uTypeVal);

void TypXfer (PSYM ps, PNODE pType);

UINT TypBuildVal (PNODE pnode);

BOOL TypIsBaseType (UINT uID);

BOOL TypIsType (void);

UINT TypPtrBase (UINT uTypeVal);


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/*
 * uPtrs: 0 - no ptrs allowed                  x
 *        1 - ptrs ok
 *        2 - + rules: ptr/ptr -> invalid      x
 *        3 - - rules: ptr/ptr -> int          x
 *
 * uMix:  0 - no mixing i & p                  x
 *        1 - mixing ok
 *        2 - ptr on left only                 x
 */
UINT TypResolve (PNODE pExpr, UINT bFloats, UINT uPtrs, UINT uMix);


/*
 *
 *
 *
 */
PNODE TypInsert (PNODE pnode, UINT uOldType, UINT uNewType, BOOL bAllowPtrConversion, BOOL bDieOnError);


PNODE TypCast (PNODE pnode, UINT uNewType);


void TypCheckPointers (PNODE pnode, UINT uTypeVal1, UINT uTypeVal2, BOOL bVoidOK);

void TypPrecisionWarning (PNODE pnode);

void TypNoVoid (PNODE pnode); 


BOOL TypIntegral (UINT uTypeVal);

BOOL TypFloat (UINT uTypeVal);

BOOL TypPtr (UINT uTypeVal);

BOOL TypString (UINT uTypeVal);

/*
 * 32 bit compiles return DATATYPE_LONG
 * 16 bit compiles return DATATYPE_SHORT
 */
INT IntTyp (void);

/* 
 * uMix 0: int -> ptr not allowed
 *      1: int -> ptr allowed with a warning
 *
 *
 */
void TypConvertStatic (PNODE pLit, UINT uTargetType, UINT uMix);

/*
 *
 */
void CheckStructTypes (PNODE pnode, PSYM ps1, PSYM ps2);

