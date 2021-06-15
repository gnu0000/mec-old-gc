/*
 *
 * tokenize.c
 * Friday, 2/21/1997.
 * Craig Fitzgerald
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuFile.h>
#include "gclib.h"
#include "gc.h"
#include "symbol.h"
#include "tokenize.h"
#include "error.h"
#include "define.h"


/*
 * open file stack
 * - a stack because of nestable includes -
 */
PFINFO pINFILE = NULL;

/*
 * the current token
 */
TOKEN Token;


#define MAX_TOKSTR 4096

UINT uTOKLEN;
CHAR szTOKSTR[MAX_TOKSTR+1];


/*
 * reserved token list
 */ 
RES ReservedWords[] =
  {
   {TOK_LESSTHAN,       "<",       }, // relational operator
   {TOK_GREATERTHAN,    ">",       },
   {TOK_LESSOREQUAL,    "<=",      },
   {TOK_GREATEROREQUAL, ">=",      },
   {TOK_EQUIVALENT,     "==",      },  
   {TOK_NOTEQUAL,       "!=",      },

   {TOK_EQUALS,         "=",       }, // assignment operator
   {TOK_PLUSEQUAL,      "+=",      },
   {TOK_MINUSEQUAL,     "-=",      },
   {TOK_STAREQUAL,      "*=",      },
   {TOK_SLASHEQUAL,     "/=",      },
   {TOK_PERCENTEQUAL,   "%=",      },
   {TOK_SHIFTRIGHTEQUAL,">>=",     },
   {TOK_SHIFTLEFTEQUAL, "<<=",     },
   {TOK_ANDEQUAL,       "&=",      },
   {TOK_XOREQUAL,       "^=",      },
   {TOK_OREQUAL,        "|=",      },

   {TOK_LOGICALOR,      "||",      }, // boolean logical operator
   {TOK_LOGICALAND,     "&&",      },
   {TOK_EXCLAMATION,    "!",       },

   {TOK_AMPERSAND,      "&",       }, // bitwise logical operator
   {TOK_OR,             "|",       },
   {TOK_HAT,            "^",       },
   {TOK_SHIFTRIGHT,     ">>",      },
   {TOK_SHIFTLEFT,      "<<",      },
   {TOK_TILDA,          "~",       },

   {TOK_INCREMENT,      "++",      }, // inc & dec operator
   {TOK_DECREMENT,      "--",      },
   {TOK_ARROW,          "->",      },

   {TOK_SLASH,          "/",       }, // arithmetic operator
   {TOK_PERCENT,        "%",       },
   {TOK_STAR,           "*",       },
   {TOK_MINUS,          "-",       },
   {TOK_PLUS,           "+",       },

   {TOK_OCOMMENT,       "/*",      }, // comment token
   {TOK_CCOMMENT,       "*/",      },
   {TOK_CPPCOMMENT,     "//",      },

   {TOK_AT,             "@",       }, 
   {TOK_POUND,          "#",       },
   {TOK_DOLLAR,         "$",       },
   {TOK_BACKSLASH,      "\\",      },
   {TOK_OPAREN,         "(",       },
   {TOK_CPAREN,         ")",       },
   {TOK_OBRACKET,       "[",       },
   {TOK_CBRACKET,       "]",       },
   {TOK_OBRACE,         "{",       },
   {TOK_CBRACE,         "}",       },
   {TOK_SINGLEQUOTE,    "'",       },
   {TOK_DOUBLEQUOTE,    "\"",      },
   {TOK_COMMA,          ",",       },
   {TOK_PERIOD,         ".",       },
   {TOK_QUESTION,       "?",       },
   {TOK_COLON,          ":",       },
   {TOK_SEMICOLON,      ";",       },

   {TOK_IF,            "if",       },
   {TOK_ELSE,          "else",     },
   {TOK_FOR,           "for",      },
   {TOK_WHILE,         "while",    },
   {TOK_SWITCH,        "switch",   },
   {TOK_CASE,          "case",     },
   {TOK_DEFAULT,       "default",  },
   {TOK_BREAK,         "break",    },
   {TOK_CONTINUE,      "continue", },
   {TOK_RETURN,        "return",   },
   {TOK_TYPEDEF,       "typedef",  },
   {TOK_SIZEOF,        "sizeof",   },

   {TOK_VOID,          "void",     },
   {TOK_CHAR,          "char",     },
   {TOK_SHORT,         "short",    },
   {TOK_INT,           "int",      },
   {TOK_LONG,          "long",     },
   {TOK_FLOAT,         "float",    },
   {TOK_STRING,        "string",   },
   {TOK_STRUCT,        "struct",   },
   {TOK_UNION,         "union",    },

   {TOK_INCLUDE,       "#include", },
   {TOK_DEFINE,        "#define",  },
   {TOK_DUMPSYM,       "#dumpsym",  },

   {TOK_IDENTIFIER       , " identifier",    }, // these entries are never
   {TOK_CHARLITERAL      , " char literal",  }, // matched, but are here
   {TOK_SHORTLITERAL     , " short literal", }, // for printing messages
   {TOK_LONGLITERAL      , " long literal",  }, //
   {TOK_INTLITERAL       , " int literal",   }, //
   {TOK_FLOATLITERAL     , " float literal", }, // 
   {TOK_STRINGLITERAL    , " string literal",}, //
   {TOK_DECLARATIONLIST  , " decl list",     }, //
   {TOK_COMPOUNDSTATEMENT, " comp stmt",     }, //
   {TOK_EXPR             , " expression",    }, //
   {TOK_FUNCTION         , " function",      }, //
   {TOK_VARIABLE         , " variable",      },
   {TOK_LITERAL          , " literal",       },

   {TOK_FTOW              , " FtoW",         },
   {TOK_WTOF              , " WtoF",         },

   {0, NULL}};

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/*
 * for error reporting
 */ 
PSZ TokReservedString(UINT uID)
   {
   UINT i;

   for (i=0; ReservedWords[i].pszString; i++)
      if (uID == ReservedWords[i].uID)
         return ReservedWords[i].pszString;
   return "unknown";
   }


static BOOL IsWhiteSpace (INT c)
   {
   return (c == ' ' || c == '\t' || c == '\n');
   }

static INT SkipWhiteSpace (void)
   {
   INT c;

   for (c = t_getc (); IsWhiteSpace (c); c = t_getc ())
      ;

   return c;
   }


/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


static void StartTokenString (void)
   {
   uTOKLEN = 0;
   szTOKSTR[uTOKLEN] = '\0';
   }


static void AddToTokenString (INT c)
   {
   if (uTOKLEN >= MAX_TOKSTR)
      InFileError ("string too large: %s\n", szTOKSTR);
   szTOKSTR[uTOKLEN++] = c;
   szTOKSTR[uTOKLEN] = '\0';
   }


static PSZ TokenString (void)
   {
   return szTOKSTR;
   }

/***************************************************************************/

static FILE *PushFile (PSZ pszFile)
   {
   FILE   *fp;
   PFINFO pFInfo;

   if (pINFILE)
      pINFILE->uLine = (UINT)FilGetLine ();

   if (!(fp = fopen (pszFile, "rt")))
      return NULL;
   FilSetLine (1);

   /*--- create entry and push on stack ---*/
   pFInfo = calloc (sizeof (FINFO), 1);
   pFInfo->pszFile = strdup (pszFile);
   pFInfo->uLine  = 0;
   pFInfo->fp      = fp;
   pFInfo->next    = pINFILE;
   pINFILE = pFInfo;

   return fp;
   }


static FILE *PopFile (void)
   {
   PFINFO pFInfo;

   if (!pINFILE)
      return NULL;
   pFInfo = pINFILE;
   pINFILE = pFInfo->next;

   free (pFInfo->pszFile);
   free (pFInfo);

   if (!pINFILE)
      return NULL;
   FilSetLine (pINFILE->uLine);
   return pINFILE->fp;
   }

/***************************************************************************/

static BOOL SetToken ()
   {
   Token.pszFile = pINFILE->pszFile;
   Token.uLine   = (UINT)FilGetLine ();
   Token.bEOF    = FALSE;
   Token.bUnget  = FALSE;
   return TRUE;
   }

static BOOL SetReservedToken (PRES pr)
   {
   Token.uID        = pr->uID;         // TOK_HAT
   Token.pszString  = pr->pszString;   // "^"
   Token.ps         = NULL;
   return SetToken ();
   }


static BOOL SetLiteralToken (UINT uID, PSZ pszString)
   {
   Token.uID        = uID;                // TOK_LITERALINT
   Token.pszString  = pszString;          // "float value"
   Token.ps         = NULL;
   return SetToken ();
   }


static BOOL SetSymbolToken (PSYM ps)
   {
   Token.uID        = TOK_IDENTIFIER;     // TOK_IDENTIFIER
   Token.pszString  = "identifier";       // "identifier"
   Token.ps         = ps;
   return SetToken ();
   }


/***************************************************************************/


static void SkipCComment ()
   {
   INT c;
   UINT uCommentStartLine;

   uCommentStartLine = (UINT)FilGetLine ();

   while (TRUE)
      {
      c = t_getc ();
      if (c == EOF)
         {
         FilSetLine (uCommentStartLine); // makes error more meaningful
         InFileError ("comment not closed");
         }
      if (c == '*') // end of comment
         {
         c = t_getc ();
         if (c == '/')
            return;
         t_ungetc (c);
         }
      if (c == '/') // nested comment
         {
         c = t_getc ();
         if (c == '*')
            SkipCComment ();
         else
            t_ungetc (c);
         }
      }
   }


static void SkipCPPComment ()
   {
   INT c;

   for (c = t_getc (); c != '\n' && c != EOF; c = t_getc ())
      ;
   }

/***************************************************************************/

/*
 * reads a reserved word or an identifier
 * returns TRUE if we read a defined symbol
 * and need to loop
 */
static BOOL ReadSymbol (INT c)
   {
   PSYM ps;
   PRES pr;

   StartTokenString ();
   if (c)
      AddToTokenString (c);

   for (c = t_getc (); __iscsym (c); c = t_getc ())
      AddToTokenString (c);
   t_ungetc (c);

   if (DefineCheck (TokenString ()))
      {
      Token.uID = 0;
      StartTokenString ();
      return TRUE;
      }

   if (pr = SymFindReserved (TokenString ()))
      SetReservedToken (pr);

   else if (ps = SymFind (TokenString ()))
      SetSymbolToken (ps);
   
   else /*--- new identifier ---*/
      SetSymbolToken (SymAdd (TokenString (), FALSE));
   return FALSE;
   }


/*
 *
 * opening ' already read in
 */
static void ParseCharLiteral (void)
   {
   INT c;

   StartTokenString ();
   c = t_getc ();

   if (c == EOF || c == '\n')
      InFileError ("closing quote not found");
   if (c == 0x27)
      {
      InFileError ("empty character constant");
      }
   else if (c == '\\')
      {
      switch (c = t_getc ())
         {
         case 'n' : AddToTokenString ('\n'); break;
         case 'r' : AddToTokenString ('\r'); break;
         case '0' : AddToTokenString ('\0'); break;
         case '\\': AddToTokenString ('\\'); break;
         default  : AddToTokenString (c);    break;
         }
      }
   else
      AddToTokenString (c);

   c = t_getc ();
   if (c != 0x27)
      InFileError ("too many characters in constant");

   SetLiteralToken (TOK_CHARLITERAL, "char value");
   Token.val.c = *TokenString();
   }



/*
 * handle backslash escapes
 * (multiline strings - not handled yet)
 * opening " already read in
 */
static void ParseStringLiteral (void)
   {
   UINT uStringStartLine;
   INT c;

   uStringStartLine = (UINT)FilGetLine ();;
   StartTokenString ();

   while (TRUE)
      {
      c = t_getc ();
      if (c == EOF)
         {
         FilSetLine (uStringStartLine); // makes error more meaningful
         InFileError ("closing quote not found");
         }
//    if (c == '\n') allow for now
      if (c == '"')
         break;
      if (c == '\\')
         {
         switch (c = t_getc ())
            {
            case 'n' : AddToTokenString ('\n'); break;
            case 'r' : AddToTokenString ('\r'); break;
            case '0' : AddToTokenString ('\0'); break;
            case '\\': AddToTokenString ('\\'); break;
            default  : AddToTokenString (c);    break;
            }
         }
      else
         AddToTokenString (c);
      }
   SetLiteralToken (TOK_STRINGLITERAL, "string literal");
   Token.val.psz = strdup(TokenString());
   }



/*
 * read a decimal integer
 *        hex     integer
 *        binary  integer
 *
 *        float
 *        scientific notation  float
 *
 *
 * for now, all numbers will be floats of form ###.### or integers
 */
static void ReadNumber (INT c)
   {
   PSZ  pszTmp;

   StartTokenString ();
   AddToTokenString (c);

   while (TRUE)
      {
      c = t_getc ();
      if (c == EOF || !isalnum (c) && c != '.' /*---!strchr ("-+.", c)---*/)
         break;
      AddToTokenString (c);
      }
   t_ungetc (c);

   if (strchr (TokenString (), '.'))
      {
      SetLiteralToken (TOK_FLOATLITERAL, "float value");
      Token.val.bg = strtod (TokenString (), &pszTmp);
      if (*pszTmp)
         InFileError ("number format error: %s\n", TokenString ());
      }
   else
      {
      Token.val.l = strtol (TokenString (), &pszTmp, 0);
      if (*pszTmp)
         InFileError ("number format error: %s\n", TokenString ());
         
//      if (Token.val.l < 256 && Token.val.l >= 0)
//         SetLiteralToken (TOK_CHARLITERAL, "number literal");
//      else if (Token.val.l < 32768 && Token.val.l > -32768)
//         SetLiteralToken (TOK_SHORTLITERAL, "short literal");
//      else
//         SetLiteralToken (TOK_LONGLITERAL, "long literal");
      SetLiteralToken (TOK_INTLITERAL, "int literal");
      }
   }


/*
 * match reserved symbols that are not identifiers
 *
 * This fn is highly reserved word specific and may need
 * to be restructured when adding a sticky word
 *
 */
static BOOL ReadReserved (INT c)
   {
   INT c2;

   StartTokenString ();
   AddToTokenString (c);

   /*---handles: ~ @ # $ ( ) [ ] { } , . ? : ; \ ------*/
   if (strchr ("~@#$()[]{},.?:;\\", c))
      return SetReservedToken (SymFindReserved (TokenString()));

   /*---handles: = == ! != ^ ^= % %= ------------------*/
   if (strchr ("=!^%", c))
      {
      c2 = t_getc ();
      if (c2 == '=')
         AddToTokenString (c2);
      else
         t_ungetc (c2);
      return SetReservedToken (SymFindReserved (TokenString()));
      }

   /*---handles + += ++ - -= -- & &= && | |= || -------*/
   if (strchr ("+-&|", c))
      {
      c2 = t_getc ();
      if (c2 == '=' || c2 == c || (c == '-' && c2 == '>'))
         AddToTokenString (c2);
      else
         t_ungetc (c2);

      return SetReservedToken (SymFindReserved (TokenString()));
      }

   /*---handles: >= >> >>=  < <= << <<= ------------*/
   if (c == '<' || c == '>')
      {
      c2 = t_getc ();
      if (c2 == '=')
         AddToTokenString (c2);
      else if (c2 == c)
         {
         AddToTokenString (c2);
         c2 = t_getc ();
         if (c2 == '=')
            AddToTokenString (c2);
         else
            t_ungetc (c2);
         }
      else
         t_ungetc (c2);
      return SetReservedToken (SymFindReserved (TokenString()));
      }

   /*---handles: * *= *./ -----------------------------*/
   if (c == '*')
      {
      c2 = t_getc ();
      if (c2 == '=' || c2 == '/')
         AddToTokenString (c2);
      else
         t_ungetc (c2);
      return SetReservedToken (SymFindReserved (TokenString()));
      }

   /*---handles: / /= // /.* --------------------------*/
   if (c == '/')
      {
      c2 = t_getc ();
      if (c2 == '=' || c2 == '/' || c2 == '*')
         AddToTokenString (c2);
      else
         t_ungetc (c2);
      return SetReservedToken (SymFindReserved (TokenString()));
      }
   return 0;
   }


/*
 * here we do a little parsing within the tokenizer
 *
 */
static void ParseDirective (INT c)
   {
   BOOL bIncPath;

   ReadSymbol (c);

   if (Token.uID == TOK_INCLUDE)
      {
      switch (c = SkipWhiteSpace ())
         {
         case '<': bIncPath = TRUE;  break;
         case '"': bIncPath = FALSE; break;
         default : InFileError ("invalid #include statement");
         }
      StartTokenString ();
      c = 0;
      for (c = t_getc (); c != '>' && c != '"';c = t_getc ())
         {
         if (c == EOF || c == '\n')
            InFileError ("invalid #include statement '%s'", TokenString());
         AddToTokenString (c);
         }
      if (bIncPath)
         ; // later

      if (!PushFile (TokenString ()))
         InFileError ("Cannot open include file '%s'", TokenString ());

      }
   else if (Token.uID == TOK_DUMPSYM)
      {
      SymDump ();
      }
   else if (Token.uID == TOK_DEFINE)
      {
      c = SkipWhiteSpace ();
      if (ReadSymbol (c))
         InFileError ("symbol already defined: %s", TokenString ());
      DefineAdd (TokenString ());
      }
   else
      InFileError ("unknown directive '%s'\n", TokenString ());
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


/*
 * 0 - OK
 * 1 - cannot open file
 */
UINT TokInit (PSZ pszFile)
   {
   FILE *fp;
   UINT i;

   if (!(fp = PushFile (pszFile)))
      InFileError ("Cannot open input file '%s'", pszFile);

   /*--- create the reserved words hash table ---*/
   for (i=0; ReservedWords[i].pszString; i++)
      SymAddReserved (ReservedWords + i);

   return 0;
   }


UINT TokTerm (void)
   {
   return 0;
   }


/*
 * works with the global variable "Token"
 *
 */
UINT TokGet (void)
   {
   BOOL bLoop = TRUE;
   INT c;

   if (Token.bUnget)
      {
      Token.bUnget = FALSE;
      return Token.uID;
      }
   while (bLoop)
      {
      bLoop = FALSE;

      StartTokenString (); // not necessary

      if ((Token.bEOF || !pINFILE))
         return 0;

      c = SkipWhiteSpace ();

      if (c == EOF)
         {
         if (PopFile ()) // coming out of an include
            bLoop = TRUE;
         else            // end of main file
            {
            Token.uID = 0;
            Token.bEOF  = TRUE;
            }
         }
      else if (__iscsymf (c))
         bLoop = ReadSymbol (c);

      else if (isdigit (c))
         ReadNumber (c);

      else if (c == '"')
         ParseStringLiteral ();

      else if (c == 0x27)
         ParseCharLiteral ();

      else if (ReadReserved (c))
         {
         }
      else
         InFileError ("Unknown Character '%c' [%2x]\n", c, c);

      /*----------------------------------------------------------*/

      if (Token.uID == TOK_OCOMMENT)
         {
         SkipCComment ();
         bLoop = TRUE;
         }

      if (Token.uID == TOK_CPPCOMMENT)
         {
         SkipCPPComment ();
         bLoop = TRUE;
         }

      if (Token.uID == TOK_POUND)
         {
         ParseDirective (c);
         bLoop = TRUE;
         }
      }

   if (LOG_VAL & 0x04)
      TokPrint ();

   return Token.uID;
   }
         

/***************************************************************************/

/*
 * only 1 level
 */
void TokUnget (void)
   {
   if (Token.bUnget)
      InFileError ("Internal error: cannot unget twice '%s'\n", Token.pszString);
   Token.bUnget = TRUE;
   }


/*
 *
 *
 */
BOOL TokEat (UINT uID)
   {
   TokGet ();
   if (Token.uID != uID)
      InFileError ("Expected '%s', got '%s'\n", TokReservedString(uID), Token.pszString);

   return TRUE;
   }


/*
 *
 *
 */
UINT TokPeek (void)
   {
   TokGet ();
   TokUnget ();

   return Token.uID;
   }

/*
 *
 *
 */
UINT TokTry (UINT uType)
   {
   TokGet ();
   if (Token.uID == uType)
      return Token.uID;
   TokUnget ();
   return FALSE;
   }

/*
 *
 *
 */
UINT TokTry2 (UINT u1, UINT u2)
   {
   TokGet ();
   if (Token.uID == u1 || Token.uID == u2)
      return Token.uID;
   TokUnget ();
   return FALSE;
   }


/*
 *
 *
 */
UINT TokTry4 (UINT u1, UINT u2, UINT u3, UINT u4)
   {
   TokGet ();
   if (Token.uID == u1 || Token.uID == u2 | Token.uID == u3 || Token.uID == u4)
      return Token.uID;
   TokUnget ();
   return FALSE;
   }

/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

void TokPrint (void)
   {
   Log (4, "%s(%3d): ID:[%3d] STR:[%-16s]  (%-24s)\n",
   Token.pszFile,
   Token.uLine,
   Token.uID,
   Token.pszString,
   (Token.ps ? Token.ps->pszLiteral : ""));
   }
