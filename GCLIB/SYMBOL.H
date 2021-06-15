/*
 *
 * symbol.h
 * Tuesday, 2/25/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

typedef struct _res
   {
   UINT  uID;        // TOK_HAT
   PSZ   pszString;  // "^"
   struct _res *next;
   } RES;
typedef RES *PRES;


void SymInit (void);
void SymTerm (void);

void SymAddReserved  (PRES);
PRES SymFindReserved (PSZ pszStr);

PSYM SymAdd (PSZ pszSym, BOOL bAddAtEnd);
PSYM SymFind (PSZ pszSym);
BOOL SymDelete (PSZ pszSym);

void SymDeleteLocals (void);

void SymDump (void);
