/*
 *
 * parse.h
 * Wednesday, 2/26/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

void CompileDriver (void);

void PushScope (UINT uScope);
void PopScope ();
UINT CurrentScope (void);

void PrintTree (PNODE pnode, UINT uIndent);
