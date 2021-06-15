/*
 *
 * vm.h
 * Friday, 3/28/1997.
 * Craig Fitzgerald
 *
 * Not an external header - This header is intended for 
 * internal use by the gclib library
 *
 * most of the vm api is in gclib.h
 */

typedef union
   {
   LONG   l;
   PSZ    psz;
   PSHORT ps;
   PLONG  pl;
   PBIG   pbg;
   } MONO;

typedef union
   {
   struct
      {
      LONG  l1;
      LONG  l2;
      };
   BIG   bg;
   } MONOF;


extern INT  iSF;          // stack frame register
extern INT  iSP;          // stack pointer
extern INT  iIP;          // instruction ptr

extern PFUNC pfnCURR;     // currently executing fn
extern PLONG plSTACK;     // malloced at startup
extern BOOL  bDEBUG ;     // writes trace if TRUE

