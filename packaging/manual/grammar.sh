#!/bin/bash
#
# Script to produce simplified human-readable grammar file from the mscgen
#  language source file.
#

bison -v ../../src/language.y

# Top and tail the output, remove leading space
cat language.output |
sed "/Terminals, with rules where they appear/q" |
head -n -1 |
tail -n '+7' |
cut -c 7- |
grep -v "\$accept:" |
sed "s/TOK_MSC/'msc'/g" |
sed "s/TOK_OCBRACKET/'{'/g" |
sed "s/TOK_CCBRACKET/'}'/g" |
sed "s/TOK_OSBRACKET/'['/g" |
sed "s/TOK_CSBRACKET/']'/g" |
sed "s/TOK_SEMICOLON/';'/g" |
sed "s/TOK_ASTERISK/'\*'/g" |
sed "s/TOK_COMMA/','/g" |
sed "s/TOK_EQUAL/'='/g" |
sed "s/TOK_OPT_HSCALE/'hscale'/g" |
sed "s/TOK_OPT_WIDTH/'width'/g" |
sed "s/TOK_OPT_ARCGRADIENT/'arcgradient'/g" |
sed "s/TOK_OPT_WORDWRAPARCS/'wordwraparcs'/g" |
sed "s/TOK_STRING/[A-Za-z0-9_]+/g" |
sed "s/TOK_QSTRING/\"[^\"]+\"/g"  |
sed "s/TOK_SPECIAL_ARC/\"\.\.\.\" | \"---\" \| \"|||\" /g" |
sed "s/TOK_REL_BOX/'box'/g" |
sed "s/TOK_REL_ABOX/'abox'/g" |
sed "s/TOK_REL_RBOX/'rbox'/g" |
sed "s/TOK_REL_SIG_BI/'<->'/g" |
sed "s/TOK_REL_SIG_TO/'->'/g" |
sed "s/TOK_REL_SIG_FROM/'<-'/g" |
sed "s/TOK_REL_SIG/'--'/g" |
sed "s/TOK_REL_METHOD_BI/'<=>'/g" |
sed "s/TOK_REL_METHOD_TO/'=>'/g" |
sed "s/TOK_REL_METHOD_FROM/'<='/g" |
sed "s/TOK_REL_METHOD/'=='/g" |
sed "s/TOK_REL_RETVAL_BI/'<\.>'/g" |
sed "s/TOK_REL_RETVAL_TO/'\.>'/g" |
sed "s/TOK_REL_RETVAL_FROM/'<\.'/g" |
sed "s/TOK_REL_RETVAL/'\.\.'/g" |
sed "s/TOK_REL_DOUBLE_BI/'<:>'/g" |
sed "s/TOK_REL_DOUBLE_TO/':>'/g" |
sed "s/TOK_REL_DOUBLE_FROM/'<:'/g" |
sed "s/TOK_REL_DOUBLE/'::'/g" |
sed "s/TOK_REL_CALLBACK_BI/'<<=>>'/g" |
sed "s/TOK_REL_CALLBACK_TO/'=>>'/g" |
sed "s/TOK_REL_CALLBACK_FROM/'<<='/g" |
sed "s/TOK_REL_METHOD/'=='/g" |
sed "s/TOK_REL_LOSS_TO/'-x'/g" |
sed "s/TOK_REL_LOSS_FROM/'x-'/g" |
sed "s/TOK_REL_NOTE/'note'/g" |
sed "s/TOK_ATTR_LABEL/'label'/g" |
sed "s/TOK_ATTR_URL/'url'/g" |
sed "s/TOK_ATTR_IDURL/'idurl'/g" |
sed "s/TOK_ATTR_ID/'id'/g" |
sed "s/TOK_ATTR_LINE_COLOUR/'linecolour'/g" |
sed "s/TOK_ATTR_TEXT_COLOUR/'textcolour'/g" |
sed "s/TOK_ATTR_TEXT_BGCOLOUR/'textbgcolour'/g" |
sed "s/TOK_ATTR_ARC_LINE_COLOUR/'arclinecolour'/g" |
sed "s/TOK_ATTR_ARC_TEXT_COLOUR/'arctextcolour'/g" |
sed "s/TOK_ATTR_ARC_TEXT_BGCOLOUR/'arctextbgcolour'/g" |
sed "s/TOK_ATTR_ARC_SKIP/'arcskip'/g"

rm language.output language.tab.c

# END OF SCRIPT


