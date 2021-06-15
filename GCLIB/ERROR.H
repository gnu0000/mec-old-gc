/*
 *
 * error.h
 * Friday, 2/28/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

extern UINT LOG_VAL;

void _cdecl InFileError (PSZ psz, ...);               // uses global: pINFILE
void _cdecl ParseError  (PSZ psz, ...);               // uses global: Token
void _cdecl NodeError   (PNODE pnode, PSZ psz, ...);  //
void _cdecl Log         (UINT uLevel,  PSZ psz, ...);  //

void _cdecl NodeWarn (PNODE pnode, PSZ psz, ...);
