/*
 *
 * codmaint.h
 * Tuesday, 3/??/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 * although... if you replace the loader like the _keep
 * functions do (they replace the compiler output functions)
 * you will need access to these functions ... I thought not.
 */

typedef struct _func
   {
   PSZ  pszName;
   UINT uSize;
   PSZ  pszCode;
   PSZ  pszGlobals;
   struct _func *left;
   struct _func *right;
   } FUNC;
typedef FUNC *PFUNC;

PFUNC CodFindFunction (PSZ pszName);

PFUNC CodNewFunction (void);

void CodAddFunction (PFUNC pFunc);

void CodSetGlobalPtr (PSZ pszGlobals);

