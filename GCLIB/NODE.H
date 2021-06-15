/*
 *
 * node.h
 * Monday, 3/17/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

PNODE NodeBuild (void);

PNODE NodeBuild4 (UINT uID, PNODE pn1, PNODE pn2, PNODE pn3, PNODE pn4);

PNODE NodeBuild3 (UINT uID, PNODE pn1, PNODE pn2, PNODE pn3);

PNODE NodeBuild2 (UINT uID, PNODE pn1, PNODE pn2);

PNODE NodeBuild1 (UINT uID, PNODE pn1);

PNODE NodeAddToEnd (PNODE pnList, PNODE pnEntry);

PNODE NodeFree (PNODE pnode);

PNODE NodeFreeTree (PNODE pnode);

PNODE NodeBuildLit (UINT uType);

