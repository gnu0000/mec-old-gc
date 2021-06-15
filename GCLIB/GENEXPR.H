/*
 *
 * genexpr.h
 * Tuesday, 3/4/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 * part of the interface is in gclib.h
 */

void GenExpression (PNODE pExpr);

/*
 * a condition is an expression that always ends up being a
 * 0 or non zero integer
 * if pT ! NULL, jump there if result is non zero
 * if pF ! NULL, jump there if result is zero
 *
 * if expression is a natural int expr, nothing is done before
 * the jump checks are done.
 * (otherwise a NZ(C,F,S) is inserted first to get a bool result ???)
 *
 */
void GenCondition (PNODE pExpr, PLBL pT, PLBL pF);
