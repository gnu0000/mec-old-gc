/*
 *
 * odbc.h
 * Friday, 4/11/1997.
 *
 * simplified odbc interface for use with gc
 *
 */

typedef struct
   {
   HDBC   hdbc;
   } CON;
typedef CON *PCON;

typedef struct
   {
   HSTMT   hstmt;
   PCON    pcon;
   PSZ     pszSQL;
   SHORT   iCols;
   PSHORT  piTypes;
   PVOID   *ppBind;
   PLONG   plSizes;
   } STM;
typedef STM *PSTM;

SHORT ODBCInit (void);
SHORT ODBCTerm (void);

PCON  ODBCConnect (PSZ pszDSN, PSZ pszID, PSZ pszPwd);
SHORT ODBCDisconnect (PCON pcon)   ;

SHORT ODBCStmtInfo (PSTM pstm, INT iCol, INT iCmd, PINT piRet, PSZ pszRet);

PSTM  ODBCExec (PCON pcon, PSZ pszSQL, BOOL bMakeAllStrings);
SHORT ODBCCancel (PSTM pstm);

PVOID *ODBCGetRow (PSTM pstm);

