/*
 *
 * gi.c
 * Tuesday, 3/4/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuArg.h>
#include <GnuMisc.h>
#include "gclib\gclib.h"
#include "fn_c.h"
#include "fn_gnu.h"

#define STACK_SIZE 0x1000

void Usage (void)
   {
   printf ("GI  Gnu GC source file interpreter             v0.1       %s %s\n\n", __DATE__, __TIME__);
   printf ("USAGE: GI [options] gcfile [gxfiles]\n\n");
   printf ("WHERE: gcfile     is the GC source file to interpret.\n");
   printf ("       gxfiles    is specified is a list of GX modules to load.\n\n");
   printf ("       [options]  Are 0 or more of:\n");
   printf ("         /? ........... This help.\n");
   printf ("         /Log=# ....... Compile Debug messages (4 and 8).\n");
   printf ("         /Debug ....... Exec Debug messages.\n");
   printf ("         /Opt=# ....... Set peephole optimization level (1,3,7).\n");
   printf ("         /Stack=# ..... Specify Stack Size (def is 4K).\n");
   printf ("         /Call=name ... Specify fn to call (def is main).\n");
   exit (0);
   }

int CCONV main (int argc, char *argv[])
   {
   UCHAR szInFile [256];
   PSZ  p;
   BOOL bDebug;
   UINT i, uStack;

#if defined (MEM_DEBUG)
      MemSetDebugMode (MEM_TEST | MEM_LOG | MEM_CLEAR | MEM_EXITLIST);
      MemSetDebugStream (fopen ("gi.mem", "wt"));
#endif

   ArgBuildBlk ("? *^Help *^Debug *^Opt? *^Log% *^Stack% *^Call%");
   if (ArgFillBlk (argv))
      Error (ArgGetErr (), 0, 0);

   if (ArgIs ("help") || ArgIs ("?") || !ArgIs(NULL))
      Usage ();

   if (ArgIs ("Log"))
      SetCompileOption (CO_LOGVAL, atoi(ArgGet ("Log", 0)));

   if (ArgIs ("Opt"))
      SetCompileOption (CO_OPTIMIZE, (ArgGet("Opt", 0) ? atoi (ArgGet("Opt", 0)) : 0xFF));

   bDebug = ArgIs ("Debug");
   uStack = (ArgIs ("Stack") ? atoi(ArgGet("Stack", 0)) : STACK_SIZE);

   strcpy (szInFile, ArgGet (NULL, 0));
   if (!(p = strrchr (szInFile, '\\')))
      p = szInFile;
   if (!strrchr (p, '.'))
      strcat (szInFile, EXT_C);

   GenSetOutputFn (_keepTag      , FTYP_WRITE_FILE_TAGS   );
   GenSetOutputFn (_keepFunction , FTYP_WRITE_FN          );
   GenSetOutputFn (_keepGlobStart, FTYP_WRITE_GLOBAL_START);
   GenSetOutputFn (_keepGlobData , FTYP_WRITE_GLOBAL_DATA );
   GenSetOutputFn (_keepGlobEnd  , FTYP_WRITE_GLOBAL_END  );

   Compile (szInFile, NULL);

   VMInit (uStack, bDebug);
   InitInternalFunctions ();
   InitCFunctions ();
   InitGnuFunctions ();

   /*--- look for GX files on the command line ---*/
   for (i=1; i < ArgIs (NULL); i++)
      {
      strcpy (szInFile, ArgGet (NULL, i));
      if (!(p = strrchr (szInFile, '\\')))
         p = szInFile;
      if (!strrchr (p, '.'))
         strcat (szInFile, EXT_EXE);

      if (bDebug)
         printf ("Loading module %s\n", szInFile);
   
      switch (LoadGXModule (szInFile))
         {
         case 1: Error ("Unable to open program file %s", szInFile);
         case 2: Error ("File is not a GNU program file: %s", szInFile);
         case 3: Error ("bin file in bad format!");
         }
      }

   if (ArgIs ("Call"))
      {
      VMSPush (0);    // place for return val;
      VMSPush (0);    // place for return val;
      VMSPush (0);    // status #2 (parm types info)
      VMSPush (0);    // status #1 (parm count info)
      VMSPush (-6);   // offset to return area

      VMExecFunction (ArgGet ("Call", 0));
      }
   else
      {
      VMExecMain (argc, argv);
      }

   exit ((UINT)VMReturn ()); // for memdebug option
   return (UINT)VMReturn ();
   }


