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
#include <GnuStr.h>
#include <GnuFile.h>
#include <GnuKbd.h>
#include <GnuMisc.h>
#include "gclib\gclib.h"

void InitCFunctions (void)
   {
   AddFn (malloc         , DATATYPE_PTR, 101);
   AddFn (calloc         , DATATYPE_PTR, 102);
   AddFn (realloc        , DATATYPE_PTR, 103);
   AddFn (free           , DATATYPE_VOID, 104);
   AddFn (strdup         , DATATYPE_PTR, 105);

   AddFn (printf         , DATATYPE_INT, 110);
   AddFn (sprintf        , DATATYPE_INT, 111);

   AddFn (strchr         , DATATYPE_PTR, 200);
   AddFn (strrchr        , DATATYPE_PTR, 201);
   AddFn (strcat         , DATATYPE_PTR, 202);
   AddFn (strcmp         , DATATYPE_PTR, 203);
   AddFn (strcpy         , DATATYPE_PTR, 204);
   AddFn (StrClip        , DATATYPE_PTR, 205);
   AddFn (StrStrip       , DATATYPE_PTR, 206);
   AddFn (StrGetWord     , DATATYPE_INT, 207);
   AddFn (StrGetIdent    , DATATYPE_INT, 208);
   AddFn (StrEatWord     , DATATYPE_INT, 209);
   AddFn (StrEatChar     , DATATYPE_INT, 210);
   AddFn (StrSkipBy      , DATATYPE_PTR, 211);
   AddFn (StrSkipTo      , DATATYPE_PTR, 212);
   AddFn (StrBlankLine   , DATATYPE_INT, 213);
   AddFn (strnchr        , DATATYPE_PTR, 214);
   AddFn (StrMatches     , DATATYPE_INT, 215);
   AddFn (StrMyCat       , DATATYPE_PTR, 216);
   AddFn (StrMakePPSZ    , DATATYPE_PTR, 217);
// AddFn (StrPPSZCut     , DATATYPE_PTR, 218);
   AddFn (StrMakeCSVField, DATATYPE_PTR, 219);
   AddFn (StrUnCSV       , DATATYPE_PTR, 220);
   AddFn (StrGetCSVField , DATATYPE_INT, 221);
   AddFn (StrAddQuotes   , DATATYPE_PTR, 222);
   AddFn (StrCookLine    , DATATYPE_PTR, 223);
// AddFn (Stristr        , DATATYPE_PTR, 224);
   AddFn (StrTrue        , DATATYPE_INT, 225);

   AddFn (fopen          , DATATYPE_PTR, 300);
   AddFn (fclose         , DATATYPE_PTR, 301);
   AddFn (fread          , DATATYPE_INT, 302);
   AddFn (fwrite         , DATATYPE_INT, 303);
   AddFn (FilReadLine    , DATATYPE_INT, 304);
   AddFn (FilReadTo      , DATATYPE_INT, 305);
   AddFn (FilReadWhile   , DATATYPE_INT, 306);
   AddFn (FilReadWord    , DATATYPE_INT, 307);
   AddFn (FilGetLine     , DATATYPE_PTR, 308);
   AddFn (FilSetLine     , DATATYPE_PTR, 309);
   AddFn (f_getc         , DATATYPE_INT, 310);
   AddFn (f_ungetc       , DATATYPE_INT, 311);

   AddFn (k_getch        , DATATYPE_INT, 400);
   AddFn (k_ungetch      , DATATYPE_INT, 401);
   AddFn (k_kbhit        , DATATYPE_INT, 402);

   AddFn (KeyGet         , DATATYPE_INT, 403);
   AddFn (KeyChoose      , DATATYPE_INT, 404);
   AddFn (KeyClearBuff   , DATATYPE_VOID, 405);

// AddFn (KeyEditCell    , DATATYPE_INT, 406);
// AddFn (KeyEditCellMode, DATATYPE_VOID, 407);

   AddFn (KeyRecord      , DATATYPE_INT, 408);
   AddFn (KeyPlay        , DATATYPE_INT, 409);
   AddFn (KeyStop        , DATATYPE_INT, 410);
   AddFn (KeyInfo        , DATATYPE_INT, 411);
   AddFn (KeyPlayDelay   , DATATYPE_VOID, 412);

   AddFn (atoi           , DATATYPE_INT,500);
   AddFn (Rnd            , DATATYPE_INT,502);
   AddFn (srand          , DATATYPE_VOID,503);
   AddFn (time           , DATATYPE_PTR,504);

   OrderFn ();
   }

