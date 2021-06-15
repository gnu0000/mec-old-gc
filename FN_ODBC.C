/*
 * internal.c
 * Sunday, 3/9/1997.
 * Craig Fitzgerald
 *
 * This module includes functions that are callable from gc code
 * these are c functions that are mapped to gc callable format
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuScr.h>
#include <GnuWin.h>
#include "gclib\gclib.h"
#include "odbc.h"

void InitODBCFunctions (void)
   {
   AddFn (ODBCInit      , DATATYPE_SHORT,  2100);
   AddFn (ODBCTerm      , DATATYPE_SHORT,  2101);
   AddFn (ODBCConnect   , DATATYPE_PTR,    2102);
   AddFn (ODBCDisconnect, DATATYPE_SHORT,  2103);
   AddFn (ODBCStmtInfo  , DATATYPE_SHORT,  2104);
   AddFn (ODBCExec      , DATATYPE_PTR,    2105);
   AddFn (ODBCCancel    , DATATYPE_SHORT,  2106);
   AddFn (ODBCGetRow    , DATATYPE_PTR,    2107);


   AddFn (SQLAllocConnect    , DATATYPE_SHORT, 2000);
   AddFn (SQLAllocEnv        , DATATYPE_SHORT, 2001);
   AddFn (SQLAllocStmt       , DATATYPE_SHORT, 2002);
   AddFn (SQLBindCol         , DATATYPE_SHORT, 2003);
   AddFn (SQLCancel          , DATATYPE_SHORT, 2004);
   AddFn (SQLColAttributes   , DATATYPE_SHORT, 2005);
   AddFn (SQLConnect         , DATATYPE_SHORT, 2006);
   AddFn (SQLDescribeCol     , DATATYPE_SHORT, 2007);
   AddFn (SQLDisconnect      , DATATYPE_SHORT, 2008);
   AddFn (SQLError           , DATATYPE_SHORT, 2009);
   AddFn (SQLExecDirect      , DATATYPE_SHORT, 2010);
   AddFn (SQLExecute         , DATATYPE_SHORT, 2011);
   AddFn (SQLFetch           , DATATYPE_SHORT, 2012);
   AddFn (SQLFreeConnect     , DATATYPE_SHORT, 2013);
   AddFn (SQLFreeEnv         , DATATYPE_SHORT, 2014);
   AddFn (SQLFreeStmt        , DATATYPE_SHORT, 2015);
   AddFn (SQLGetCursorName   , DATATYPE_SHORT, 2016);
   AddFn (SQLNumResultCols   , DATATYPE_SHORT, 2017);
   AddFn (SQLPrepare         , DATATYPE_SHORT, 2018);
   AddFn (SQLRowCount        , DATATYPE_SHORT, 2019);
   AddFn (SQLSetCursorName   , DATATYPE_SHORT, 2020);
   AddFn (SQLTransact        , DATATYPE_SHORT, 2021);
   AddFn (SQLSetParam        , DATATYPE_SHORT, 2022);
   AddFn (SQLColumns         , DATATYPE_SHORT, 2023);
   AddFn (SQLGetConnectOption, DATATYPE_SHORT, 2024);
   AddFn (SQLGetData         , DATATYPE_SHORT, 2025);
   AddFn (SQLGetFunctions    , DATATYPE_SHORT, 2026);
   AddFn (SQLGetInfo         , DATATYPE_SHORT, 2027);
   AddFn (SQLGetStmtOption   , DATATYPE_SHORT, 2028);
   AddFn (SQLGetTypeInfo     , DATATYPE_SHORT, 2029);
   AddFn (SQLParamData       , DATATYPE_SHORT, 2030);
   AddFn (SQLPutData         , DATATYPE_SHORT, 2031);
   AddFn (SQLSetConnectOption, DATATYPE_SHORT, 2032);
   AddFn (SQLSetStmtOption   , DATATYPE_SHORT, 2033);
   AddFn (SQLSpecialColumns  , DATATYPE_SHORT, 2034);
   AddFn (SQLStatistics      , DATATYPE_SHORT, 2035);
   AddFn (SQLTables          , DATATYPE_SHORT, 2036);
   AddFn (SQLDataSources     , DATATYPE_SHORT, 2037);
   AddFn (SQLSetScrollOptions, DATATYPE_SHORT, 2038);
   AddFn (SQLDriverConnect   , DATATYPE_SHORT, 2039);
   AddFn (SQLBrowseConnect   , DATATYPE_SHORT, 2040);
   AddFn (SQLColumnPrivileges, DATATYPE_SHORT, 2041);
   AddFn (SQLDescribeParam   , DATATYPE_SHORT, 2042);
   AddFn (SQLExtendedFetch   , DATATYPE_SHORT, 2043);
   AddFn (SQLForeignKeys     , DATATYPE_SHORT, 2044);
   AddFn (SQLMoreResults     , DATATYPE_SHORT, 2045);
   AddFn (SQLNativeSql       , DATATYPE_SHORT, 2046);
   AddFn (SQLNumParams       , DATATYPE_SHORT, 2047);
   AddFn (SQLParamOptions    , DATATYPE_SHORT, 2048);
   AddFn (SQLPrimaryKeys     , DATATYPE_SHORT, 2049);
   AddFn (SQLProcedureColumns, DATATYPE_SHORT, 2050);
   AddFn (SQLProcedures      , DATATYPE_SHORT, 2051);
   AddFn (SQLSetPos          , DATATYPE_SHORT, 2052);
   AddFn (SQLTablePrivileges , DATATYPE_SHORT, 2053);
//   AddFn (SQLDrivers         , DATATYPE_SHORT, 2054);
//   AddFn (SQLBindParameter   , DATATYPE_SHORT, 2055);
   OrderFn ();
   }




