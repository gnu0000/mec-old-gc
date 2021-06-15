/*
 *
 * gc.c
 * Saturday, 3/1/1997.
 * Craig Fitzgerald
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuArg.h>
#include <GnuMisc.h>
#include "gclib\gclib.h"


static void Usage (void)
   {
   printf ("GC   Gnu GC source file compiler               v0.1       %s %s\n\n", __DATE__, __TIME__);
   printf ("USAGE: GC [options] gcfile[.gc]\n\n");
   printf ("WHERE: gcfile     is GC file to compile.\n\n");
   printf ("WHERE: [options]  Are 0 or more of:\n");
   printf ("         /? ........... This help.\n");
   printf ("         /Log=# ....... Debug messages (4=tokens, 8=parse trees).\n");
   printf ("         /Opt=# ....... Set peephole optimization (0,1,3,7 default=7).\n");
   printf ("         /Out=name .... name output file (default is infile.GX).\n");
   exit (0);
   }


int CCONV main (int argc, char *argv[])
   {
   CHAR szIn[256], szOut[256];
   PSZ  p;

#if defined (MEM_DEBUG)
      MemSetDebugMode (MEM_TEST | MEM_LOG | MEM_CLEAR | MEM_EXITLIST);
      MemSetDebugStream (fopen ("gc.mem", "wt"));
#endif

   ArgBuildBlk ("? *^Help *^Log% *^Opt? *^Out%");
   if (ArgFillBlk (argv))
      Error (ArgGetErr (), 0, 0);

   if (ArgIs ("help") || ArgIs ("?") || !ArgIs(NULL))
      Usage ();

   if (ArgIs ("Log"))
      SetCompileOption (CO_LOGVAL, atoi(ArgGet ("Log", 0)));

   if (ArgIs ("Opt"))
      SetCompileOption (CO_OPTIMIZE, (ArgGet("Opt", 0) ? atoi (ArgGet("Opt", 0)) : 0xFF));

   /*--- make input and output file names ---*/
   strcpy (szIn, ArgGet (NULL, 0));
   if (!(p = strrchr (szIn, '\\')))
      p = szIn;
   if (!strrchr (p, '.'))
      strcat (szIn, EXT_C);

   if (ArgIs ("Out"))
      {
      strcpy (szOut, ArgGet ("Out", 0));
      }
   else
      {
      strcpy (szOut, szIn);
      *strrchr (szOut, '.') = '\0';
      }
   if (!(p = strrchr (szIn, '\\')))
      strcat (szOut, EXT_EXE);

   printf ("Compiling %s\n", szIn);
   Compile (szIn, szOut);
   printf ("Done.\n", szIn);
   exit (0);
   return 0;
   }

