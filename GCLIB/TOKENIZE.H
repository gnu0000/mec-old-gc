/*
 *
 * tokenize.h
 * Friday, 2/21/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 */

typedef struct _fileinfo
   {
   PSZ   pszFile;     // filename
   UINT  uLine;       // current file line
   FILE  *fp;         // file handle
   struct _fileinfo *next;
   } FINFO;
typedef FINFO *PFINFO;

extern TOKEN Token;

extern PFINFO pINFILE;


UINT TokInit (PSZ pszFile);
UINT TokTerm ();

BOOL TokGet (void);
UINT TokPeek (void);
void TokUnget (void);

BOOL TokEat (UINT uID);
UINT TokTry (UINT uID);
BOOL TokTry2 (UINT u1, UINT u2);
BOOL TokTry4 (UINT u1, UINT u2, UINT u3, UINT u4);
void TokPrint (void);

PSZ TokReservedString(UINT uID);

