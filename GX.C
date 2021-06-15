/*
 *
 * gx.c
 * Monday, 3/31/1997.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuArg.h>
#include <GnuMisc.h>
#include <GnuStr.h>
#include "gclib\gclib.h"
#include "fn_c.h"
#include "fn_gnu.h"

#define STACK_SIZE 0x1000

BOOL bDebug;

void Usage (void)
   {
   printf ("GX  Gnu GX module executer                     v0.1       %s %s\n\n", __DATE__, __TIME__);
   printf ("USAGE: GX [options] filelist\n\n");
   printf ("WHERE: filelist   is a list of GX files to load.\n\n");
   printf ("       [options]  Are 0 or more of:\n");
   printf ("         /? ........... This help.\n");
   printf ("         /Debug ....... Debug messages.\n");
   printf ("         /Stack=# ..... Specify Stack Size (def is 4K).\n");
   printf ("         /Call=name ... Specify fn to call (def is main).\n");
   printf ("\n");
   printf (" Note that if multiple files are specified on the command line, modules are\n");
   printf (" loaded from left to right, and where functions are defined in more than 1\n");
   printf (" module, the last one loaded is the one executed.\n");
   exit (0);
   }

void LoadGX (PSZ pszFile)
   {
   UCHAR szInFile [256];
   PSZ p;

   strcpy (szInFile, pszFile);

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

int CCONV main (int argc, char *argv[])
   {
   UINT i, uStack;

#if defined (MEM_DEBUG)
      MemSetDebugMode (MEM_TEST | MEM_LOG | MEM_CLEAR | MEM_EXITLIST);
      MemSetDebugStream (fopen ("gx.mem", "wt"));
#endif

   ArgBuildBlk ("? *^Help *^Debug *^Stack% *^Call=% *^Load%");

   if (ArgFillBlk (argv))
      Error (ArgGetErr (), 0, 0);

   if (ArgIs ("help") || ArgIs ("?") || !ArgIs(NULL))
      Usage ();

   bDebug = ArgIs ("Debug");
   uStack = (ArgIs ("Stack") ? atoi(ArgGet("Stack", 0)) : STACK_SIZE);

   VMInit (uStack, bDebug);
   InitInternalFunctions ();
   InitCFunctions ();
   InitGnuFunctions ();

   LoadGX (ArgGet (NULL, 0));
   for (i=0; i < ArgIs ("Load"); i++)
      LoadGX (ArgGet ("Load", i));

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

