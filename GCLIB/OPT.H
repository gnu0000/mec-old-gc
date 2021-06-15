/*
 *
 * opt.h
 * Monday, 3/10/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*
 * This function performs logical reductions on an
 * expression parse tree
 *
 * Most of the reductions involve constant rollups
 * and a few identity reductions
 *
 * literal type conv
 *
 * 0+e e+0  -> e
 * e-0      -> e
 * 0-e      -> -1
 *
 * 1*e e*1  -> e
 * e/1      -> e
 *
 * - c      -> -c
 *
 * c op c   -> ceval
 *
 * e/c      -> e*(1/c)
 *
 */
PNODE OptCollapseExpr (PNODE pExpr);





PNODE OptChangeLit (PNODE pnode, UINT uOP, UINT uVal);
