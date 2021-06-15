/*
 *
 * gco.c
 * Friday, 4/11/1997.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <SQL.H>
#include <SQLEXT.H>
#include <GnuType.h>
#include "odbc.h"

static HENV  henv;

SHORT ODBCInit (void)
   {
   return SQLAllocEnv (&henv);
   }

SHORT ODBCTerm (void)
   {
   return SQLFreeEnv(henv);
   }

PCON ODBCConnect (PSZ pszDSN, PSZ pszID, PSZ pszPwd)
   {
   PCON  pcon;
   SHORT rc;

   pcon = calloc (1, sizeof (CON));

   rc = SQLAllocConnect(henv, &(pcon->hdbc));
   if (rc && rc != 1)
      return NULL;

   rc = SQLConnect(pcon->hdbc, pszDSN, SQL_NTS, pszID, SQL_NTS, pszPwd, SQL_NTS);
   if (rc && rc != 1)
      return NULL;

   return pcon;
   }


SHORT ODBCDisconnect (PCON pcon)   
   {
   SHORT rc;

   rc = SQLDisconnect(pcon->hdbc);
   free (pcon);
   return rc;
   }

///* SQLColAttributes defines */
//#define SQL_COLUMN_COUNT                0
//#define SQL_COLUMN_NAME                 1
//#define SQL_COLUMN_TYPE                 2
//#define SQL_COLUMN_LENGTH               3
//#define SQL_COLUMN_PRECISION            4
//#define SQL_COLUMN_SCALE                5
//#define SQL_COLUMN_DISPLAY_SIZE         6
//#define SQL_COLUMN_NULLABLE             7
//#define SQL_COLUMN_UNSIGNED             8
//#define SQL_COLUMN_MONEY                9
//#define SQL_COLUMN_UPDATABLE            10
//#define SQL_COLUMN_AUTO_INCREMENT       11
//#define SQL_COLUMN_CASE_SENSITIVE       12
//#define SQL_COLUMN_SEARCHABLE           13
//#define SQL_COLUMN_TYPE_NAME            14
//#define SQL_COLUMN_TABLE_NAME           15
//#define SQL_COLUMN_OWNER_NAME           16
//#define SQL_COLUMN_QUALIFIER_NAME       17
//#define SQL_COLUMN_LABEL                18
//
SHORT ODBCStmtInfo (PSTM pstm, INT iCol, INT iCmd, PINT piRet, PSZ pszRet)
   {
   SHORT rc, iSize;
   LONG  iRet;

   rc = SQLColAttributes (pstm->hstmt, iCol, iCmd, pszRet, 128, &iSize, &iRet);
   if (rc && rc != 1)
      return rc;
   *piRet = iRet;
   return 0;
   }

PSTM  ODBCExec (PCON pcon, PSZ pszSQL, BOOL bMakeAllStrings)
   {
   PSTM   pstm;
   INT    i, iType, iLen, rc, iResultType;
   SHORT  iCols;

   pstm = calloc (1, sizeof (STM));
   pstm->pcon   = pcon;
   pstm->pszSQL = strdup (pszSQL);

   if (SQLAllocStmt(pcon->hdbc, &(pstm->hstmt)))
      return NULL;

   rc = SQLExecDirect(pstm->hstmt, pszSQL, SQL_NTS);
   if (rc && rc != 1)
      return NULL;

   rc = SQLNumResultCols (pstm->hstmt, &iCols);

   pstm->iCols  = iCols;
   pstm->piTypes= calloc (iCols+1, sizeof (SHORT));
   pstm->ppBind = calloc (iCols+1, sizeof (PVOID));
   pstm->plSizes= calloc (iCols+1, sizeof (LONG));

   /*--- create memory for the bindings ---*/
   iResultType = (bMakeAllStrings ? SQL_C_CHAR : SQL_C_DEFAULT);
   for (i=1; i<=iCols; i++)
      {
      rc = ODBCStmtInfo (pstm, i, SQL_COLUMN_TYPE, &iType, "");

      pstm->piTypes[i] = iType;
      if (!bMakeAllStrings)
         {
         rc = ODBCStmtInfo (pstm, i, SQL_COLUMN_LENGTH, &iLen, "");
         }
      else
         {
         if (iType == SQL_C_BINARY || SQL_C_CHAR)
            rc = ODBCStmtInfo (pstm, i, SQL_COLUMN_LENGTH, &iLen, "");
         else
            iLen = 60; // dates when converted can be pretty big
         }
      (pstm->ppBind)[i] = calloc (1, iLen);

      rc = SQLBindCol(pstm->hstmt, i, iResultType, 
                      (pstm->ppBind)[i], iLen, pstm->plSizes +i);
      }
   return pstm;
   }


void PrintStmtError (PSTM pstm)
   {
   CHAR szState [128];
   CHAR szError [128];
   SWORD uLen;
   SDWORD uError;

   SQLError (henv, pstm->pcon->hdbc, pstm->hstmt,
            szState, &uError, szError, 128, &uLen);
   printf ("Error: %s = %s", szState, szError);
   }

static PSTM FreeStm (PSTM pstm)
   {
   SHORT i;

   if (pstm->pszSQL)  free (pstm->pszSQL);
   if (pstm->piTypes) free (pstm->piTypes);
   if (pstm->plSizes) free (pstm->plSizes);
   if (pstm->ppBind)
      {
      for (i=0; i<-pstm->iCols; i++)
         free ((pstm->ppBind)[i]);
      free (pstm->ppBind);
      }
   free (pstm);
   return NULL;
   }

PVOID *ODBCGetRow (PSTM pstm)
   {
   SHORT rc;

   rc = SQLFetch (pstm->hstmt);
   if (rc && rc != 1)
      {
      if (rc == -1)
         PrintStmtError (pstm);
      SQLFreeStmt (pstm->hstmt, SQL_DROP);
      return (PVOID)FreeStm (pstm);
      }
   return pstm->ppBind;
   }


SHORT ODBCCancel (PSTM pstm)
   {
   return SQLCancel (pstm->hstmt);
   }
