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
#include "fn_odbc.h"

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

//int Test (PSZ pszServer, PSZ pszID, PSZ pszPwd)
//   {
//   HENV   henv;
//   HDBC   hdbc;
//   HSTMT  hstmt;
//   SHORT  rc;
//   CHAR   szStmt[256], szCustID[256], szCustName[256];
//   LONG   lLen1, lLen2;
//
//   rc = SQLAllocEnv(&henv);
//   printf ("SQLAllocEnv = %x\n", (int)rc);
//   SQLAllocConnect(henv, &hdbc);
//   printf ("SQLAllocConnect = %x\n", (int)rc);
//
//   rc = SQLConnect(hdbc, pszServer, SQL_NTS, pszID, SQL_NTS, pszPwd, SQL_NTS);
//   printf ("SQLConnect = %x\n", (int)rc);
//   if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
//      return printf("SQLConnect returned %x\n", (int)rc);
//
//   rc = SQLAllocStmt(hdbc, &hstmt);
//   printf ("SQLAllocStmt = %x\n", (int)rc);
//
//   strcpy (szStmt, "select * from Customers");
//
//   rc = SQLExecDirect(hstmt, szStmt, SQL_NTS);
//   printf ("SQLExecDirect = %x\n", (int)rc);
//   if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
//      return printf("SQLExecDirect returned %x\n", (int)rc);
//
//   rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, szCustID, sizeof(szCustID), &lLen1);
//   printf ("SQLBindCol = %x\n", (int)rc);
//
//   rc = SQLBindCol(hstmt, 2, SQL_C_CHAR, szCustName, sizeof(szCustName), &lLen2);
//   printf ("SQLBindCol = %x\n", (int)rc);
//
//   while (TRUE)
//      {
//      rc = SQLFetch(hstmt);
//      if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
//         break;
//      printf ("[%s], [%s]\n", szCustID, szCustName);
//      }
//   SQLFreeStmt(hstmt, SQL_DROP);
//   SQLDisconnect(hdbc);
//   SQLFreeConnect(hdbc);
//   SQLFreeEnv(henv);
//   return(0);
//   }


int CCONV main (int argc, char *argv[])
   {
   UINT i, uStack;

//Test ("Northwind", NULL, NULL);

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
   InitODBCFunctions ();

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

