/*
 *
 * internal.c
 * Sunday, 3/9/1997.
 * Craig Fitzgerald
 *
 * This module includes functions that are callable from gc code
 * these are c functions that are mapped to gc callable format
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuScr.h>
#include <GnuWin.h>
#include "gclib\gclib.h"

void InitGnuFunctions (void)
   {
   AddFn (ScrInitMetrics,         DATATYPE_PTR,  1000);
   AddFn (ScrGetMetrics,          DATATYPE_PTR,  1001);
   AddFn (ScrSetVideoBase,        DATATYPE_VOID, 1002);
   AddFn (ScrGetScreenSize,       DATATYPE_INT,  1003);
   AddFn (ScrSetScreenSize,       DATATYPE_INT,  1004);
   AddFn (ScrRestoreScreen,       DATATYPE_INT,  1005);
   AddFn (ScrIsEgaVga,            DATATYPE_INT,  1006);
   AddFn (ScrScrollDown,          DATATYPE_VOID, 1007);
   AddFn (ScrScrollUp,            DATATYPE_VOID, 1008);
   AddFn (ScrScrollLR,            DATATYPE_INT,  1009);
   AddFn (ScrClear,               DATATYPE_VOID, 1010);
   AddFn (ScrSetIntenseBkg,       DATATYPE_VOID, 1011);
   AddFn (ScrGetDACReg,           DATATYPE_VOID, 1012);
   AddFn (ScrSetDACReg,           DATATYPE_VOID, 1013);
   AddFn (ScrGetPALReg,           DATATYPE_INT,  1014);
   AddFn (ScrSetPALReg,           DATATYPE_VOID, 1015);
   AddFn (ScrStoreColors,         DATATYPE_VOID, 1016);
   AddFn (ScrRestoreColors,       DATATYPE_VOID, 1017);
   AddFn (ScrGetCursorPos,        DATATYPE_VOID, 1018);
   AddFn (ScrSetCursorPos,        DATATYPE_VOID, 1019);
   AddFn (ScrShowCursor,          DATATYPE_VOID, 1020);
   AddFn (ScrCursorVisible,       DATATYPE_INT,  1021);
   AddFn (ScrPushCursor,          DATATYPE_VOID, 1022);
   AddFn (ScrPopCursor,           DATATYPE_VOID, 1023);
   AddFn (ScrAttachIO,            DATATYPE_INT,  1024);
   AddFn (ScrReadBlk,             DATATYPE_INT,  1025);
   AddFn (ScrReadStr,             DATATYPE_INT,  1026);
   AddFn (ScrWriteBlk,            DATATYPE_INT,  1027);
   AddFn (ScrWriteStr,            DATATYPE_INT,  1028);
   AddFn (ScrWriteStrAtt,         DATATYPE_INT,  1029);
   AddFn (ScrWriteNCell,          DATATYPE_INT,  1030);
   AddFn (scrCheckGetMetrics,     DATATYPE_PTR,  1031);
   AddFn (Int10GetReg,            DATATYPE_INT,  1032);
   AddFn (Int10,                  DATATYPE_VOID, 1033);
   AddFn (ScrIncrementColor,      DATATYPE_INT,  1034);
                                                   
   AddFn (GnuCreateWin,           DATATYPE_PTR,  1100);
   AddFn (GnuCreateWin2,          DATATYPE_PTR,  1101);
   AddFn (GnuDestroyWin,          DATATYPE_PTR,  1102);
   AddFn (GnuPaintAtCreate,       DATATYPE_VOID, 1103);
   AddFn (GnuPaint,               DATATYPE_INT,  1104);
   AddFn (GnuPaintStr,            DATATYPE_INT,  1105);
   AddFn (GnuPaint2,              DATATYPE_INT,  1106);
   AddFn (GnuPaintBig,            DATATYPE_INT,  1107);
   AddFn (GnuPaintNChar,          DATATYPE_INT,  1108);
   AddFn (GnuClearWin,            DATATYPE_VOID, 1109);
   AddFn (GnuPaintBorder,         DATATYPE_VOID, 1110);
   AddFn (GnuMoveWin,             DATATYPE_INT,  1111);
   AddFn (GnuSetColor,            DATATYPE_INT,  1112);
   AddFn (GnuGetColor,            DATATYPE_INT,  1113);
   AddFn (GnuGetAtt,              DATATYPE_INT,  1114);
   AddFn (GnuSetBorderChars,      DATATYPE_INT,  1115);
   AddFn (GnuDrawBox,             DATATYPE_INT,  1116);
   AddFn (GnuMoveCursor,          DATATYPE_VOID, 1117);
   AddFn (GnuGetCursor,           DATATYPE_VOID, 1118);
   AddFn (GnuPaintWin,            DATATYPE_INT,  1119);
   AddFn (GnuMakeVisible,         DATATYPE_INT,  1120);
   AddFn (GnuScrollTo,            DATATYPE_INT,  1121);
   AddFn (GnuScrollWin,           DATATYPE_VOID, 1122);
   AddFn (GnuSelectLine,          DATATYPE_INT,  1123);
   AddFn (GnuDoListKeys,          DATATYPE_INT,  1124);
   AddFn (GnuSelectColorsWindow2, DATATYPE_INT,  1126);
   AddFn (GnuModifyColorsWindow,  DATATYPE_INT,  1127);
   AddFn (GnuFileWindow,          DATATYPE_INT,  1128);
   AddFn (GnuMsg,                 DATATYPE_INT,  1129);
   AddFn (GnuMsgBox,              DATATYPE_INT,  1130);
   AddFn (GnuMsgBox2,             DATATYPE_INT,  1131);
   AddFn (GnuComboBox,            DATATYPE_INT,  1132);
   AddFn (GnuHexEdit,             DATATYPE_INT,  1133);
   AddFn (WinShimmer,             DATATYPE_VOID, 1134);
   AddFn (WinUsedColorList,       DATATYPE_INT,  1135);
   AddFn (gnuDefPaintProc,        DATATYPE_INT,  1136);
   AddFn (gnuScreenDat,           DATATYPE_PTR,  1137);
   AddFn (gnuDisplayDat,          DATATYPE_INT,  1138);
   AddFn (gnuFreeDat,             DATATYPE_VOID, 1139);
   AddFn (gnuSwapDat,             DATATYPE_VOID, 1140);
   AddFn (gnuWinToScreen,         DATATYPE_VOID, 1141);
   AddFn (gnuScreenToWin,         DATATYPE_VOID, 1142);
   AddFn (gnuShowScrollPos,       DATATYPE_VOID, 1143);
   AddFn (gnuDrawShadow,          DATATYPE_VOID, 1144);
   AddFn (gnuFixScroll,           DATATYPE_INT,  1145);
   AddFn (gnuClientWinToScreen,   DATATYPE_VOID, 1146);
   AddFn (gnuScreenToClientWin,   DATATYPE_VOID, 1147);
   AddFn (gnuGetBorderChars,      DATATYPE_PTR,  1148);
   OrderFn ();
   }

