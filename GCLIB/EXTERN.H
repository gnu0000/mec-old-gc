/*
 *
 * extern.h
 * Sunday, 3/30/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 *
 *
 */

void AddFn (PVOID pFn, UINT uSize, UINT uTag);

void OrderFn (void);

BOOL HandleInternalCall (UINT uTag);
