/*
 *
 * genglob.h
 * Monday, 3/17/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

void StoreGlobalDecl (PNODE pVar);

void GenGlobals (void);

UINT AddStatic (PSZ pszBuff, UINT uLen);

