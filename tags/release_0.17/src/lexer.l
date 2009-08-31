%{
/***************************************************************************
 *
 * $Id$
 *
 * Mscgen language lexer definition.
 * Copyright (C) 2009 Michael C McTernan, Michael.McTernan.2001@cs.bris.ac.uk
 *
 * This file is part of msclib.
 *
 * Msc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Msclib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msclib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "msc.h"
#include "safe.h"
#include "language.h"  /* Token definitions from Yacc/Bison */

/* Counter for error reporting */
static unsigned long lex_linenum = 1;

%}

/* Not used, so prevent compiler warning */
%option nounput noinput

%x IN_COMMENT
%%

<IN_COMMENT>{
"*/"                              BEGIN(INITIAL);
[^*\n]+
"*"
\r\n|\r|\n                        lex_linenum++;
}


<INITIAL>{
"/*"                              BEGIN(IN_COMMENT);

\r\n|\r|\n                        lex_linenum++;

#.*$                              /* Ignore lines after '#' */
\/\/.*$                           /* Ignore lines after '//' */

msc                               return TOK_MSC;
HSCALE|hscale                     yylval.optType = MSC_OPT_HSCALE;              return TOK_OPT_HSCALE;
WIDTH|width                       yylval.optType = MSC_OPT_WIDTH;               return TOK_OPT_WIDTH;
ARCGRADIENT|arcgradient           yylval.optType = MSC_OPT_ARCGRADIENT;         return TOK_OPT_ARCGRADIENT;
URL|url                           yylval.attribType = MSC_ATTR_URL;             return TOK_ATTR_URL;
LABEL|label                       yylval.attribType = MSC_ATTR_LABEL;           return TOK_ATTR_LABEL;
IDURL|idurl                       yylval.attribType = MSC_ATTR_IDURL;           return TOK_ATTR_IDURL;
ID|id                             yylval.attribType = MSC_ATTR_ID;              return TOK_ATTR_ID;
LINECOLO(U?)R|linecolo(u?)r       yylval.attribType = MSC_ATTR_LINE_COLOUR;     return TOK_ATTR_LINE_COLOUR;
TEXTCOLO(U?)R|textcolo(u?)r       yylval.attribType = MSC_ATTR_TEXT_COLOUR;     return TOK_ATTR_TEXT_COLOUR;
TEXTBGCOLO(U?)R|textbgcolo(u?)r   yylval.attribType = MSC_ATTR_TEXT_BGCOLOUR;   return TOK_ATTR_TEXT_BGCOLOUR;
ARCLINECOLO(U?)R|arclinecolo(u?)r yylval.attribType = MSC_ATTR_ARC_LINE_COLOUR; return TOK_ATTR_ARC_LINE_COLOUR;
ARCTEXTCOLO(U?)R|arctextcolo(u?)r yylval.attribType = MSC_ATTR_ARC_TEXT_COLOUR; return TOK_ATTR_ARC_TEXT_COLOUR;
\.\.\.                            yylval.arctype = MSC_ARC_DISCO;    return TOK_SPECIAL_ARC;        /* ... */
---                               yylval.arctype = MSC_ARC_DIVIDER;  return TOK_SPECIAL_ARC;        /* --- */
\|\|\|                            yylval.arctype = MSC_ARC_SPACE;    return TOK_SPECIAL_ARC;        /* ||| */
-\>                               yylval.arctype = MSC_ARC_SIGNAL;   return TOK_REL_SIG_TO;         /* -> */
\<-                               yylval.arctype = MSC_ARC_SIGNAL;   return TOK_REL_SIG_FROM;       /* <- */
=\>                               yylval.arctype = MSC_ARC_METHOD;   return TOK_REL_METHOD_TO;      /* => */
\<=                               yylval.arctype = MSC_ARC_METHOD;   return TOK_REL_METHOD_FROM;    /* <= */
\>\>                              yylval.arctype = MSC_ARC_RETVAL;   return TOK_REL_RETVAL_TO;      /* >> */
\<\<                              yylval.arctype = MSC_ARC_RETVAL;   return TOK_REL_RETVAL_FROM;    /* << */
:\>                               yylval.arctype = MSC_ARC_DOUBLE;   return TOK_REL_DOUBLE_TO;      /* :> */
\<:                               yylval.arctype = MSC_ARC_DOUBLE;   return TOK_REL_DOUBLE_FROM;    /* <: */
=\>\>                             yylval.arctype = MSC_ARC_CALLBACK; return TOK_REL_CALLBACK_TO;    /* =>> */
\<\<=                             yylval.arctype = MSC_ARC_CALLBACK; return TOK_REL_CALLBACK_FROM;  /* <<= */
box                               yylval.arctype = MSC_ARC_BOX;      return TOK_REL_BOX;            /* box */
abox                              yylval.arctype = MSC_ARC_ABOX;     return TOK_REL_ABOX;           /* abox */
rbox                              yylval.arctype = MSC_ARC_RBOX;     return TOK_REL_RBOX;           /* rbox */
[A-Za-z0-9_]+                     yylval.string = strdup_s(yytext); return TOK_STRING;
\"(\\\"|[^\"])*\"                 yylval.string = strdup_s(yytext + 1); yylval.string[strlen(yylval.string) - 1] = '\0'; return TOK_QSTRING;
=                                 return TOK_EQUAL;
,                                 return TOK_COMMA;
\;                                return TOK_SEMICOLON;
\{                                return TOK_OCBRACKET;
\}                                return TOK_CCBRACKET;
\[                                return TOK_OSBRACKET;
\]                                return TOK_CSBRACKET;
\*                                return TOK_ASTERISK;
[ \t]+                            /* ignore whitespace */;
<*>.|\n                           return TOK_UNKNOWN;

}

%%

unsigned long lex_getlinenum(void)
{
    return lex_linenum;
}

/* END OF FILE */
