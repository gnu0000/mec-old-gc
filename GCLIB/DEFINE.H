/*
 *
 * define.h
 * Wednesday, 4/2/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */


INT t_getc (void);
INT t_ungetc (INT c);

void DefineAdd (PSZ pszDef);

BOOL DefineCheck (PSZ pszIdent);

void DefineTerm (void); // terminate defines module
