/***************************************************************************
 *
 * $Id$
 *
 * This file is part of mscgen, a message sequence chart renderer.
 * Copyright (C) 2007 Michael C McTernan, Michael.McTernan.2001@cs.bris.ac.uk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 **************************************************************************/

/***************************************************************************
 * Include Files
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef  HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <assert.h>
#include "cmdparse.h"
#include "lexer.h"
#include "usage.h"
#include "adraw.h"
#include "safe.h"
#include "msc.h"

/***************************************************************************
 * Types
 ***************************************************************************/

/** Structure for holding global options.
 * This structure groups all the options that affect the text output into
 * one structure.
 */
typedef struct GlobalOptionsTag
{
    /** Ideal width of output canvas.
     * If this value allows the entitySpacing to be increased, then
     * entitySpacing will be set to the larger value of it's original
     * value and idealCanvasWidth / number of entities.
     */
    unsigned int idealCanvasWidth;

    /** Horizontal spacing between entities. */
    unsigned int entitySpacing;

    /** Gap at the top of the page. */
    unsigned int entityHeadGap;

    /** Vertical spacing between arc. */
    unsigned int arcSpacing;

    /** Arc gradient.
     * Y offset of arc head, relative to tail, in pixels.
     */
    int arcGradient;

    /** Gap between adjacent boxes. */
    unsigned int boxSpacing;

    /** Minimum distance between box edges and text. */
    unsigned int boxInternalBorder;

    /** Radius of rounded box corner arcs. */
    unsigned int rboxArc;

    /** Size of 'corner' added to note boxes. */
    unsigned int noteCorner;

    /** Anguluar box slope in pixels. */
    unsigned int aboxSlope;

    /** If TRUE, wrap arc text as well as box contents. */
    Boolean      wordWrapArcLabels;

    /** Horizontal width of the arrow heads. */
    unsigned int arrowWidth;

    /** Vertical depth of the arrow heads. */
    unsigned int arrowHeight;

    /** Horizontal gap between text and horizontal lines. */
    unsigned int textHGapPre;

    /** Horizontal gap between text and horizontal lines. */
    unsigned int textHGapPost;
}
GlobalOptions;


/***************************************************************************
 * Local Variables.
 ***************************************************************************/

static Boolean gInputFilePresent = FALSE;
static char    gInputFile[4096];

static Boolean gOutputFilePresent = FALSE;
static char    gOutputFile[4096];

static Boolean gOutTypePresent = FALSE;
static char    gOutType[10];

static Boolean gDumpLicencePresent = FALSE;

static Boolean gPrintParsePresent = FALSE;

static Boolean gOutputFontPresent = FALSE;
static char    gOutputFont[256];

/** Command line switches.
 * This gives the command line switches that can be interpreted by mscgen.
 */
static CmdSwitch gClSwitches[] =
{
    {"-i",     &gInputFilePresent,  "%4096[^?]", gInputFile },
    {"-o",     &gOutputFilePresent, "%4096[^?]", gOutputFile },
    {"-T",     &gOutTypePresent,    "%10[^?]",   gOutType },
    {"-l",     &gDumpLicencePresent,NULL,        NULL },
    {"-p",     &gPrintParsePresent, NULL,        NULL },
    {"-F",     &gOutputFontPresent, "%256[^?]",  gOutputFont }
};


static GlobalOptions gOpts =
{
    600,    /* idealCanvasWidth */

    80,     /* entitySpacing */
    20,     /* entityHeadGap */
    26,     /* arcSpacing */
    0,      /* arcGradient */
    8,      /* boxSpacing */
    4,      /* boxInternalBorder */
    6,      /* rboxArc */
    12,     /* noteCorner */
    6,      /* aboxSlope */
    FALSE,  /* wordWrapArcLabels */

    /* Arrow options */
    10, 6,

    /* textHGapPre, textHGapPost */
    2, 2
};

/** The drawing. */
static ADraw drw;

/** Name of a file to be removed by deleteTmp(). */
static char *deleteTmpFilename = NULL;

/***************************************************************************
 * Functions
 ***************************************************************************/

/** Delete the file named in deleteTmpFilename.
 * This function is registered with atexit() to delete a possible temporary
 * file used when generating image map files.
 */
static void deleteTmp()
{
    if(deleteTmpFilename)
    {
        unlink(deleteTmpFilename);
    }
}

/** Remove any file extension from the passed filename.
 */
static void trimExtension(char *s)
{
    int l = strlen(s);

    while(l > 0)
    {
        l--;
        switch(s[l])
        {
            case '.':
                /* Don't truncate hidden files */
                if(l > 0 && s[l - 1] != '\\' && s[l -1] != '/')
                {
                    s[l] = '\0';
                }
                return;
            case '/':
            case '\\':
                return;
        }
    }
}


/** Check if some arc type indicates a box.
 */
static Boolean isBoxArc(const MscArcType a)
{
    return a == MSC_ARC_BOX || a == MSC_ARC_RBOX  ||
           a == MSC_ARC_ABOX || a== MSC_ARC_NOTE;
}


/** Count the number of lines in some string.
 * This counts line breaks that are written as a literal '\n' in the line to
 * determine how many lines of output are needed.
 *
 * \param[in] l  Pointer to the input string to inspect.
 * \returns      The count of '\n' characters appearing in the input string + 1.
 */
static unsigned int countLines(const char *l)
{
    unsigned int c = 1;

    do
    {
        c++;

        l = strstr(l, "\\n");
        if(l) l += 2;
    }
    while(l != NULL);

    return c;
}


/** Word wrap a line of text until the first line is less than \a width wide.
 * This removes words from the input line and builds them into a 2nd new
 * string until the input line is shorter than the supplied width.  The
 * input string is directly truncated, while the remaining characters are
 * returned in a new memory allocation.  On return, the input line of text
 * will be shorter than \a width, while the newly returned string will contain
 * all the remaining characters.
 *
 * If the input line is already shorter than \a width, the function returns
 * NULL and does not modify the input line of text.
 *
 * \param[in,out] l     Input line of text which maybe modified if needed.
 * \param[in]     width Maximum allowable text line width.
 * \returns       NULL if \a l was already less then \a width long,
 *                 otherwise a new string giving the remained of the string.
 */
static char *splitStringToWidth(char *l, unsigned int width)
{
    char *p = l + strlen(l);
    char *orig = NULL;
    int   m, n;

    if(drw.textWidth(&drw, l) > width)
    {
        /* Duplicate the original string */
        orig = strdup_s(l);

        /* Now remove words from the line until it fits the available width */
        do
        {
            /* Skip back 1 word */
            while(!isspace(*p) && p > l)
            {
                p--;
            }

            *p = '\0';
        }
        while(drw.textWidth(&drw, l) > width && p > l);

        /* Copy the remaining line to the start of the string */
        m = 0;
        n = (p - l);

        while(isspace(orig[n]) && orig[n] != '\0')
        {
            n++;
        }

        do
        {
            orig[m++] = orig[n++];
        }
        while(orig[m - 1] != '\0');
    }

    return orig;
}


/** Split an input arc label into lines, word-wrapping if needed.
 * This takes the literal label supplied from the input and splits it into an
 * array of char * text lines.  Splitting is first done according to literal
 * '\n' character sequences added by the user, then according to word wrapping
 * to fit available space, if appropriate.
 *
 * \param[in]     m         The MSC for which the lines are to be split.
 * \param[in]     arcType   The type of the arc being labelled.
 * \param[in,out] lines     Pointer to be filled with output line array.
 * \param[in]     label     Original arc label from input file.
 * \param[in]     startCol  Column in which the arc starts.
 * \param[in]     endCol    Column in which the arc ends, or -1 for broadcast arcs.
 *
 * \note The returned strings and array must be free()'d.  freeLabelLines() can
 *        be used for this purpose.
 */
static unsigned int computeLabelLines(Msc               m,
                                      const MscArcType  arcType,
                                      char           ***lines,
                                      const char       *label,
                                      int               startCol,
                                      int               endCol)
{
    unsigned int  width;
    unsigned int  nAllocLines = 8;
    char        **retLines = malloc_s(sizeof(char *) * nAllocLines);
    unsigned int  c = 0;

    assert(startCol >= 0 && startCol < (signed)MscGetNumEntities(m));
    assert(startCol >= -1 && startCol < (signed)MscGetNumEntities(m));

    /* Compute available width for text */
    if(isBoxArc(arcType) || gOpts.wordWrapArcLabels)
    {
        if(endCol == -1)
        {
            /* This is a special case for a broadcast arc */
            width = gOpts.entitySpacing * MscGetNumEntities(m);
        }
        else if(startCol < endCol)
        {
            width = gOpts.entitySpacing * (1 + (endCol - startCol));
        }
        else
        {
            width = gOpts.entitySpacing * (1 + (startCol - endCol));
        }

        /* Reduce the width due to the box borders */
        if(isBoxArc(arcType))
        {
            width -= (gOpts.boxSpacing + gOpts.boxInternalBorder) * 2;
        }

        if(arcType == MSC_ARC_NOTE)
        {
            width -= gOpts.noteCorner;
        }
    }
    else
    {
        width = UINT_MAX;
    }

    /* Split the input label into lines */
    while(label != NULL)
    {
        /* First split around user specified lines with literal '\n' */
        char *nextLine = strstr(label, "\\n");
        if(nextLine)
        {
            const int lineLen = nextLine - label;

            /* Allocate storage and duplicate the line */
            retLines[c] = malloc_s(lineLen + 1);
            memcpy(retLines[c], label, lineLen);
            retLines[c][lineLen] = '\0';

            /* Advance the label */
            label = nextLine + 2;
        }
        else
        {
            /* Duplicate the final line */
            retLines[c] = strdup_s(label);
            label = NULL;
        }

        /* Now split the line as required to wrap into the space available */
        do
        {
            /* Check if more storage maybe needed */
            if(c + 2 >= nAllocLines)
            {
                nAllocLines += 8;
                retLines = realloc_s(retLines, sizeof(char *) * nAllocLines);
            }

            retLines[c + 1] = splitStringToWidth(retLines[c], width);
            c++;
        }
        while(retLines[c] != NULL);
    }

    /* Return the array of lines and the count */
    *lines = retLines;

    return c;
}


/** Free memory allocated for the label lines.
 */
static void freeLabelLines(unsigned int n, char **lines)
{
    while(n > 0)
    {
        n--;
        free(lines[n]);
    }

    free(lines);
}


/** Get some line from a string containing '\n' delimiters.
 * Given a string that contains literal '\n' delimiters, return a subset in
 * a passed buffer that gives the nth line.
 *
 * \param[in] string  The string to parse.
 * \param[in] line    The line number to return from the string, which should
 *                     count from 0.
 * \param[in] out     Pointer to a buffer to fill with line data.
 * \param[in] outLen  The length of the buffer pointed to by \a out, in bytes.
 * \returns  A pointer to \a out.
 */
static char *getLine(const char        *string,
                     unsigned int       line,
                     char *const        out,
                     const unsigned int outLen)
{
    const char  *lineStart, *lineEnd;
    unsigned int lineLen;

    /* Setup for the loop */
    lineEnd = NULL;
    line++;

    do
    {
        /* Check if this is the first or a repeat iteration */
        if(lineEnd)
        {
            lineStart = lineEnd + 2;
        }
        else
        {
            lineStart = string;
        }

        /* Search for next delimited */
        lineEnd = strstr(lineStart, "\\n");

        line--;
    }
    while(line > 0 && lineEnd != NULL);

    /* Determine the length of the line */
    if(lineEnd != NULL)
    {
        lineLen = lineEnd - lineStart;
    }
    else
    {
        lineLen = strlen(string) - (lineStart - string);
    }

    /* Clamp the length to the buffer */
    if(lineLen > outLen - 1)
    {
        lineLen = outLen - 1;
    }

    /* Copy desired characters */
    memcpy(out, lineStart, lineLen);

    /* NULL terminate */
    out[lineLen] = '\0';

    return out;
}


/** Check if some arc name indicates a broadcast entity.
 */
static Boolean isBroadcastArc(const char *entity)
{
    return entity != NULL && (strcmp(entity, "*") == 0);
}


/** Get the skip value in pixels for some the current arc in the Msc.
 */
static int getArcGradient(Msc m)
{
    const char  *s    = MscGetCurrentArcAttrib(m, MSC_ATTR_ARC_SKIP);
    unsigned int skip = gOpts.arcGradient;

    if(s != NULL)
    {
        if(sscanf(s, "%d", &skip) == 1)
        {
            skip = (skip * gOpts.arcSpacing) + gOpts.arcGradient;
        }
        else
        {
            fprintf(stderr, "Warning: Non-integer arcskip value: %s\n", s);
        }
    }

    return skip;
}


/** Add a point to the output imagemap.
 * If \a ismap and \a url are non-NULL, this function will add a rectangle
 * to the imagemap according to the parameters passed.
 *
 * \param ismap  The file to which the imagemap should be rendered.
 * \param url    The URL to which the imagemap area should link.
 * \param x1     The x coordinate for the upper left point.
 * \param y2     The y coordinate for the upper left point.
 * \param x2     The x coordinate for the lower right point.
 * \param y2     The y coordinate for the lower right point.
 */
static void ismapRect(FILE        *ismap,
                      const char  *url,
                      unsigned int x1,
                      unsigned int y1,
                      unsigned int x2,
                      unsigned int y2)
{
    if(ismap && url)
    {
        assert(x1 <= x2); assert(y1 <= y2);

        fprintf(ismap,
                "rect %s %d,%d %d,%d\n",
                url,
                x1, y1,
                x2, y2);
    }
#if 0
    /* For debug render a cross onto the output */
    drw.line(&drw, x1, y1, x2, y2);
    drw.line(&drw, x2, y1, x1, y2);
#endif
}


/** Draw an arrow pointing to the right.
 * \param x     The x co-ordinate for the end point for the arrow head.
 * \param y     The y co-ordinate for the end point for the arrow head.
 * \param type  The arc type, which controls the format of the arrow head.
 */
static void arrowR(unsigned int x,
                   unsigned int y,
                   MscArcType   type)
{
    switch(type)
    {
        case MSC_ARC_SIGNAL: /* Unfilled half */
            drw.line(&drw,
                     x, y,
                     x - gOpts.arrowWidth, y + gOpts.arrowHeight);
            break;

        case MSC_ARC_DOUBLE:
        case MSC_ARC_METHOD: /* Filled */
        case MSC_ARC_RETVAL: /* Filled, dotted arc (not rendered here) */
            drw.filledTriangle(&drw,
                               x, y,
                               x - gOpts.arrowWidth, y + gOpts.arrowHeight,
                               x - gOpts.arrowWidth, y - gOpts.arrowHeight);
            break;

        case MSC_ARC_CALLBACK: /* Non-filled */
            drw.line(&drw,
                     x, y,
                     x - gOpts.arrowWidth, y + gOpts.arrowHeight);
            drw.line(&drw,
                     x - gOpts.arrowWidth, y - gOpts.arrowHeight,
                     x, y);
            break;

        default:
            assert(0);
            break;
    }
}


/** Draw an arrow pointing to the left.
 * \param x     The x co-ordinate for the end point for the arrow head.
 * \param y     The y co-ordinate for the end point for the arrow head.
 * \param type  The arc type, which controls the format of the arrow head.
 */
static void arrowL(unsigned int x,
                   unsigned int y,
                   MscArcType   type)
{
    switch(type)
    {
        case MSC_ARC_SIGNAL: /* Unfilled half */
            drw.line(&drw,
                     x, y,
                     x + gOpts.arrowWidth, y + gOpts.arrowHeight);
            break;

        case MSC_ARC_DOUBLE:
        case MSC_ARC_METHOD: /* Filled */
        case MSC_ARC_RETVAL: /* Filled, dotted arc (not rendered here) */
            drw.filledTriangle(&drw,
                               x, y,
                               x + gOpts.arrowWidth, y + gOpts.arrowHeight,
                               x + gOpts.arrowWidth, y - gOpts.arrowHeight);
            break;

        case MSC_ARC_CALLBACK: /* Non-filled */
            drw.line(&drw,
                     x, y,
                     x + gOpts.arrowWidth, y + gOpts.arrowHeight);
            drw.line(&drw,
                     x, y,
                     x + gOpts.arrowWidth, y - gOpts.arrowHeight);
            break;

        default:
            assert(0);
            break;
    }
}


/** Render some entity text.
 * Draw the text for some entity.
 * \param  ismap       If not \a NULL, write an ismap description here.
 * \param  x           The x position at which the entity text should be centered.
 * \param  y           The y position where the text should be placed
 * \param  entLabel    The label to render, which maybe \a NULL in which case
 *                       no ouput is produced.
 * \param  entUrl      The URL for rendering the label as a hyperlink.  This
 *                       maybe \a NULL if not required.
 * \param  entId       The text identifier for the arc.
 * \param  entIdUrl    The URL for rendering the test identifier as a hyperlink.
 *                       This maybe \a NULL if not required.
 * \param  entColour   The text colour name or specification for the entity text.
 *                      If NULL, use default colouring scheme.
 * \param  entBgColour The text background colour name or specification for the
 *                      entity text. If NULL, use default colouring scheme.
 */
static void entityText(FILE             *ismap,
                       unsigned int      x,
                       unsigned int      y,
                       const char       *entLabel,
                       const char       *entUrl,
                       const char       *entId,
                       const char       *entIdUrl,
                       const char       *entColour,
                       const char       *entBgColour)
{
    if(entLabel)
    {
        const unsigned int lines = countLines(entLabel);
        unsigned int       l;
        char               lineBuffer[1024];

        /* Adjust y to be above the writing line */
        y -= drw.textHeight(&drw) * (lines - 1);

        for(l = 0; l < lines - 1; l++)
        {
            char         *lineLabel = getLine(entLabel, l, lineBuffer, sizeof(lineBuffer));
            unsigned int  width     = drw.textWidth(&drw, lineLabel);

            /* Push text down one line */
            y += drw.textHeight(&drw);

            /* Check if a URL is associated */
            if(entUrl)
            {
                /* If no explict colour has been set, make URLS blue */
                drw.setPen(&drw, ADRAW_COL_BLUE);

                /* Image map output */
                ismapRect(ismap,
                          entUrl,
                          x - (width / 2), y - drw.textHeight(&drw),
                          x + (width / 2), y);
            }

            /* Set to the explicit colours if directed */
            if(entColour != NULL)
            {
                drw.setPen(&drw, ADrawGetColour(entColour));
            }

            if(entBgColour != NULL)
            {
                drw.setBgPen(&drw, ADrawGetColour(entBgColour));
            }

            /* Render text and restore pen */
            drw.textC (&drw, x, y, lineLabel);
            drw.setPen(&drw, ADRAW_COL_BLACK);
            drw.setBgPen(&drw, ADRAW_COL_WHITE);

            /* Render the Id of the title, if specified and for first line only */
            if(entId && l == 0)
            {
                unsigned int idwidth;
                int          idx, idy;

                idy = y - drw.textHeight(&drw);
                idx = x + (width / 2);

                drw.setFontSize(&drw, ADRAW_FONT_TINY);

                idwidth = drw.textWidth(&drw, entId);
                idy    += (drw.textHeight(&drw) + 1) / 2;

                if(entIdUrl)
                {
                    drw.setPen(&drw, ADRAW_COL_BLUE);
                    drw.textR (&drw, idx, idy, entId);
                    drw.setPen(&drw, ADRAW_COL_BLACK);

                    /* Image map output */
                    ismapRect(ismap,
                              entIdUrl,
                              idx, idy - drw.textHeight(&drw),
                              idx + idwidth, idy);
                }
                else
                {
                    drw.textR(&drw, idx, idy, entId);
                }

                drw.setFontSize(&drw, ADRAW_FONT_SMALL);
            }
        }
    }
}


/** Compute the output canvas size required for some MSC.
 * This computes the dimensions for the canvas as well as the height for each
 * row.
 *
 * \param[in]     m    The MSC to analyse.
 * \param[in,out] w    Pointer to be filled with the output width.
 * \param[in,out] h    Pointer to be filled with the output height.
 * \returns  An array giving the height of each row.
 */
static unsigned int *computeCanvasSize(Msc           m,
                                       unsigned int *w,
                                       unsigned int *h)
{
    const unsigned int rowCount = MscGetNumArcs(m) - MscGetNumParallelArcs(m);
    const unsigned int textHeight = drw.textHeight(&drw);
    unsigned int *rowHeight;
    unsigned int  nextYmin, ymin, ymax, yskipmax, row, height;
    Boolean       addLines = TRUE;

    /* Allocate storage for the height of each row */
    rowHeight = zalloc_s(sizeof(unsigned int) * rowCount);
    row = -1;

    nextYmin = ymin = gOpts.entityHeadGap;
    ymax = gOpts.entityHeadGap + gOpts.arcSpacing;
    yskipmax = 0;

    MscResetArcIterator(m);
    do
    {
        const MscArcType   arcType           = MscGetCurrentArcType(m);
        const int          arcGradient       = getArcGradient(m);
        char             **arcLabelLines     = NULL;
        unsigned int       arcLabelLineCount = 0;
        int                startCol = -1, endCol = -1;

        if(arcType == MSC_ARC_PARALLEL)
        {
            addLines = FALSE;
        }
        else
        {
            /* Get the entity indices */
            if(arcType != MSC_ARC_DISCO && arcType != MSC_ARC_DIVIDER && arcType != MSC_ARC_SPACE)
            {
                startCol = MscGetEntityIndex(m, MscGetCurrentArcSource(m));
                endCol   = MscGetEntityIndex(m, MscGetCurrentArcDest(m));
            }
            else
            {
                /* Discontinuity or parallel arc spans whole chart */
                startCol = 0;
                endCol   = MscGetNumEntities(m) - 1;
            }

            /* Work out how the label fits the gap between entities */
            arcLabelLineCount = computeLabelLines(m, arcType, &arcLabelLines,
                                                  MscGetCurrentArcAttrib(m, MSC_ATTR_LABEL),
                                                  startCol, endCol);

            /* Advance Y position if not in a parallel arc */
            if(addLines)
            {
                ymin = nextYmin;
                row++;
            }

            /* Compute the height of this arc */
            height = gOpts.arcSpacing;
            if(arcLabelLineCount > 1)
            {
                height += (arcLabelLineCount - 2) * textHeight;
            }

            /* Update the height of the row */
            if(height > rowHeight[row])
            {
                assert(row < rowCount);
                rowHeight[row] = height;
            }

            /* Compute row bounds */
            ymax = ymin + height;

            freeLabelLines(arcLabelLineCount, arcLabelLines);

            addLines = TRUE;

            /* Update next potential row start */
            if(ymax + 1 > nextYmin)
            {
                nextYmin = ymax + 1;
            }
        }

        /* Keep a track of where the gradient may cause the graph to end */
        if(ymax + arcGradient > ymax)
        {
            yskipmax = ymax + arcGradient;
        }

    }
    while(MscNextArc(m));

    if(ymax < yskipmax)
      ymax = yskipmax;

    /* Set the return values */
    *w = MscGetNumEntities(m) * gOpts.entitySpacing;
    *h = ymax + 1;

    return rowHeight;
}


/** Draw vertical lines stemming from entities.
 * This function will draw a single segment of the vertical line that
 * drops from an entity.
 *
 * \param m          The \a Msc for which the lines are drawn
 * \param ymin       Top of the row.
 * \param ymax       Bottom of the row.
 * \param dotted     If #TRUE, produce a dotted line, otherwise solid.
 * \param colourRefs Colour references for each entity.
 */
static void entityLines(Msc                m,
                        const unsigned int ymin,
                        const unsigned int ymax,
                        Boolean            dotted,
                        const ADrawColour *colourRefs)
{
    unsigned int t;

    for(t = 0; t < MscGetNumEntities(m); t++)
    {
        unsigned int x = (gOpts.entitySpacing / 2) + (gOpts.entitySpacing * t);

        drw.setPen(&drw, colourRefs[t]);

        if(dotted)
        {
            drw.dottedLine(&drw, x, ymin, x, ymax);
        }
        else
        {
            drw.line(&drw, x, ymin, x, ymax);
        }
    }

    drw.setPen(&drw, ADRAW_COL_BLACK);

}



/** Draw vertical lines and boxes stemming from entities.
 * \param ymin          Top of the row.
 * \param ymax          Bottom of the row.
 * \param boxStart      Column in which the box starts.
 * \param boxEnd        Column in which the box ends.
 * \param boxType       The type of box to draw, MSC_ARC_BOX, MSC_ARC_RBOX etc.
 * \param lineColour    Colour of the lines to use for rendering the box.
 */
static void entityBox(unsigned int       ymin,
                      unsigned int       ymax,
                      unsigned int       boxStart,
                      unsigned int       boxEnd,
                      MscArcType         boxType,
                      const char        *lineColour)
{
    unsigned int t;

    /* Ensure the start is less than or equal to the end */
    if(boxStart > boxEnd)
    {
        t = boxEnd;
        boxEnd = boxStart;
        boxStart = t;
    }

    /* Now draw the box */
    ymax--;

    unsigned int x1 = (gOpts.entitySpacing * boxStart) + gOpts.boxSpacing;
    unsigned int x2 = gOpts.entitySpacing * (boxEnd + 1) - gOpts.boxSpacing;
    unsigned int ymid = (ymin + ymax) / 2;

    /* Draw a while box to overwrite the entity lines */
    drw.setPen(&drw, ADRAW_COL_WHITE);
    drw.filledRectangle(&drw, x1, ymin, x2, ymax);

    /* Setup the colour for rendering the boxes */
    if(lineColour)
    {
        drw.setPen(&drw, ADrawGetColour(lineColour));
    }
    else
    {
        drw.setPen(&drw, ADRAW_COL_BLACK);
    }

    /* Draw the outline */
    switch(boxType)
    {
        case MSC_ARC_BOX:
            drw.line(&drw, x1, ymin, x2, ymin);
            drw.line(&drw, x1, ymax, x2, ymax);
            drw.line(&drw, x1, ymin, x1, ymax);
            drw.line(&drw, x2, ymin, x2, ymax);
            break;

        case MSC_ARC_NOTE:
            drw.line(&drw, x1, ymin, x2 - gOpts.noteCorner, ymin);
            drw.line(&drw, x1, ymax, x2, ymax);
            drw.line(&drw, x1, ymin, x1, ymax);
            drw.line(&drw, x2, ymin + gOpts.noteCorner, x2, ymax);
            drw.line(&drw, x2 - gOpts.noteCorner, ymin,
                           x2, ymin + gOpts.noteCorner);
            drw.line(&drw, x2 - gOpts.noteCorner, ymin,
                           x2 - gOpts.noteCorner, ymin + gOpts.noteCorner);
            drw.line(&drw, x2, ymin + gOpts.noteCorner,
                           x2 - gOpts.noteCorner, ymin + gOpts.noteCorner);
            break;

        case MSC_ARC_RBOX:
            drw.line(&drw, x1 + gOpts.rboxArc, ymin, x2 - gOpts.rboxArc, ymin);
            drw.line(&drw, x1 + gOpts.rboxArc, ymax, x2 - gOpts.rboxArc, ymax);
            drw.line(&drw, x1, ymin + gOpts.rboxArc, x1, ymax - gOpts.rboxArc);
            drw.line(&drw, x2, ymin + gOpts.rboxArc, x2, ymax - gOpts.rboxArc);

            drw.arc(&drw, x1 + gOpts.rboxArc,
                    ymin + gOpts.rboxArc, gOpts.rboxArc * 2, gOpts.rboxArc * 2,
                    180, 270);
            drw.arc(&drw, x2 - gOpts.rboxArc,
                    ymin + gOpts.rboxArc, gOpts.rboxArc * 2, gOpts.rboxArc * 2,
                    270, 0);
            drw.arc(&drw, x2 - gOpts.rboxArc,
                    ymax - gOpts.rboxArc, gOpts.rboxArc * 2, gOpts.rboxArc * 2,
                    0, 90);
            drw.arc(&drw, x1 + gOpts.rboxArc,
                    ymax - gOpts.rboxArc, gOpts.rboxArc * 2, gOpts.rboxArc * 2,
                    90, 180);
            break;

        case MSC_ARC_ABOX:
            drw.line(&drw, x1 + gOpts.aboxSlope, ymin, x2 - gOpts.aboxSlope, ymin);
            drw.line(&drw, x1 + gOpts.aboxSlope, ymax, x2 - gOpts.aboxSlope, ymax);
            drw.line(&drw, x1 + gOpts.aboxSlope, ymin, x1, ymid);
            drw.line(&drw, x1, ymid, x1 + gOpts.aboxSlope, ymax);
            drw.line(&drw, x2 - gOpts.aboxSlope, ymin, x2, ymid);
            drw.line(&drw, x2, ymid, x2 - gOpts.aboxSlope, ymax);
            break;

        default:
            assert(0);
    }

    /* Restore the pen colour if needed */
    if(lineColour)
    {
        drw.setPen(&drw, ADRAW_COL_BLACK);
    }
}


/** Render text on an arc.
 * Draw the text on some arc.
 * \param m              The Msc for which the text is being rendered.
 * \param ismap          If not \a NULL, write an ismap description here.
 * \param outwidth       Width of the output image.
 * \param ymin           Co-ordinate of the row on which the text should be placed.
 * \param startCol       The column at which the arc being labelled starts.
 * \param endCol         The column at which the arc being labelled ends.
 * \param arcLabelLineCount  Count of lines of text in arcLabelLines.
 * \param arcLabelLines  Array of lines of text from 0 to arcLabelLineCount - 1.
 * \param arcUrl         The URL for rendering the label as a hyperlink.  This
 *                        maybe \a NULL if not required.
 * \param arcId          The text identifier for the arc.
 * \param arcIdUrl       The URL for rendering the test identifier as a hyperlink.
 *                        This maybe \a NULL if not required.
 * \param arcTextColour  Colour for the arc text, or NULL to use default.
 * \param arcTextColour  Colour for the arc text backgroun, or NULL to use default.
 * \param arcType        The type of arc, used to control output semantics.
 */
static void arcText(Msc                m,
                    FILE              *ismap,
                    unsigned int       outwidth,
                    unsigned int       ymin,
                    int                ygradient,
                    unsigned int       startCol,
                    unsigned int       endCol,
                    const unsigned int arcLabelLineCount,
                    char             **arcLabelLines,
                    const char        *arcUrl,
                    const char        *arcId,
                    const char        *arcIdUrl,
                    const char        *arcTextColour,
                    const char        *arcTextBgColour,
                    const char        *arcLineColour,
                    const MscArcType   arcType)
{
    unsigned int l;

    for(l = 0; l < arcLabelLineCount; l++)
    {
        const char *lineLabel = arcLabelLines[l];
        unsigned int y = ymin + ((gOpts.arcSpacing + ygradient) / 2) +
                          (l * drw.textHeight(&drw));
        unsigned int width = drw.textWidth(&drw, lineLabel);
        int x = ((startCol + endCol + 1) * gOpts.entitySpacing) / 2;

        /* Discontinuity arcs have central text, otherwise the
         *  label is above the rendered arc (or horizontally in the
         *  centre of a non-straight arc).
         */
        if(arcLabelLineCount == 1 &&
           (arcType == MSC_ARC_DISCO || arcType == MSC_ARC_DIVIDER ||
            arcType == MSC_ARC_SPACE || isBoxArc(arcType)))
        {
            y += drw.textHeight(&drw) / 2;
        }

        if(startCol != endCol || isBoxArc(arcType))
        {
            /* Produce central aligned text */
            x -= width / 2;
        }
        else if(startCol < (MscGetNumEntities(m) / 2))
        {
            /* Form text to the right */
            x += gOpts.textHGapPre;
        }
        else
        {
            /* Form text to the left */
            x -= width + gOpts.textHGapPost;
        }

        /* Clip against edges of image */
        if(x + width > outwidth)
        {
            x = outwidth - width;
        }

        if(x < 0)
        {
            x = 0;
        }

        /* Check if a URL is associated */
        if(arcUrl)
        {
            /* Default to blue */
            drw.setPen(&drw, ADRAW_COL_BLUE);

            /* Image map output */
            ismapRect(ismap,
                      arcUrl,
                      x, y - drw.textHeight(&drw),
                      x + width, y);
        }


        /* Set to the explicit colours if directed */
        if(arcTextColour != NULL)
        {
            drw.setPen(&drw, ADrawGetColour(arcTextColour));
        }

        if(arcTextBgColour != NULL)
        {
            drw.setBgPen(&drw, ADrawGetColour(arcTextBgColour));
        }


        /* Render text and restore pen */
        drw.textR (&drw, x, y, lineLabel);
        drw.setPen(&drw, ADRAW_COL_BLACK);
        drw.setBgPen(&drw, ADRAW_COL_WHITE);

        /* Render the Id of the arc, if specified and for the first line*/
        if(arcId && l == 0)
        {
            unsigned int idwidth;
            int          idx, idy;

            idy = y - drw.textHeight(&drw);
            idx = x + width;

            drw.setFontSize(&drw, ADRAW_FONT_TINY);

            idwidth = drw.textWidth(&drw, arcId);
            idy    += (drw.textHeight(&drw) + 1) / 2;

            if(arcIdUrl)
            {
                drw.setPen(&drw, ADRAW_COL_BLUE);

                /* Image map output */
                ismapRect(ismap,
                          arcIdUrl,
                          idx, idy - drw.textHeight(&drw),
                          idx + idwidth, idy);
            }

            /* Render text and restore pen and font */
            drw.textR (&drw, idx, idy, arcId);
            drw.setPen(&drw, ADRAW_COL_BLACK);
            drw.setFontSize(&drw, ADRAW_FONT_SMALL);
        }

        /* Dividers also have a horizontal line at the middle */
        if(l == (arcLabelLineCount / 2) && arcType == MSC_ARC_DIVIDER)
        {
            const unsigned int margin = gOpts.entitySpacing / 4;

            if(arcLineColour != NULL)
            {
                drw.setPen(&drw, ADrawGetColour(arcLineColour));
            }

            /* Check for an even number of lines */
            if((arcLabelLineCount & 1) == 0)
            {
                /* Even, the divider falls between the two lines */
                y -= drw.textHeight(&drw);

                /* Draw line through middle of text */
                drw.dottedLine(&drw,
                               margin, y,
                               (MscGetNumEntities(m) * gOpts.entitySpacing) - margin,  y);
            }
            else
            {
                /* Odd, the divider is in the middle of the line */
                y -= drw.textHeight(&drw) / 2;

                /* Draw lines to skip the text */
                drw.dottedLine(&drw,
                              margin, y,
                              x - gOpts.textHGapPre, y);
                drw.dottedLine(&drw,
                              x + width + gOpts.textHGapPost,
                              y,
                              (MscGetNumEntities(m) * gOpts.entitySpacing) - margin,
                              y);
            }

            if(arcLineColour != NULL)
            {
                drw.setPen(&drw, ADRAW_COL_BLACK);
            }
        }
    }
}


/** Render the line and arrow head for some arc.
 * This will draw the arc line and arrow head between two columns,
 * noting that if the start and end column are the same, an arc is
 * rendered.
 * \param  m           The Msc for which the text is being rendered.
 * \param  ymin        Top of row.
 * \param  ymax        Bottom of row.
 * \param  ygradient   The gradient of the arc which alters the y position a
 *                      the ending column.
 * \param  startCol    Starting column for the arc.
 * \param  endCol      Column at which the arc terminates.
 * \param  hasArrows   If true, draw arc arrows, otherwise omit them.
 * \param  hasBiArrows If true, has arrows in both directions.
 * \param  arcType     The type of the arc, which dictates its rendered style.
 */
static void arcLine(Msc               m,
                    unsigned int      ymin,
                    unsigned int      ymax,
                    unsigned int      ygradient,
                    unsigned int      startCol,
                    unsigned int      endCol,
                    const char       *arcLineCol,
                    int               hasArrows,
                    const int         hasBiArrows,
                    const MscArcType  arcType)
{
    const unsigned int y  = (ymin + ymax) / 2;
    const unsigned int sx = (startCol * gOpts.entitySpacing) +
                             (gOpts.entitySpacing / 2);
    const unsigned int dx = (endCol * gOpts.entitySpacing) +
                             (gOpts.entitySpacing / 2);

    /* Check if an explicit line colour is requested */
    if(arcLineCol != NULL)
    {
        drw.setPen(&drw, ADrawGetColour(arcLineCol));
    }

    if(startCol != endCol)
    {
        /* Draw the line */
        if(arcType == MSC_ARC_RETVAL)
        {
            drw.dottedLine(&drw, sx, y, dx, y + ygradient);
        }
        else if(arcType == MSC_ARC_DOUBLE)
        {
            drw.line(&drw, sx, y - 1, dx, y - 1 + ygradient);
            drw.line(&drw, sx, y + 1, dx, y + 1 + ygradient);
        }
        else if(arcType == MSC_ARC_LOSS)
        {
            signed int   span = dx - sx;
            unsigned int mx = sx + (span / 4) * 3;

            drw.line(&drw, sx, y, mx, y + ygradient);
            hasArrows = 0;

            drw.line(&drw, mx - 4, y + ygradient - 4, mx + 4, y + ygradient + 4);
            drw.line(&drw, mx + 4, y + ygradient - 4, mx - 4, y + ygradient + 4);
        }
        else
        {
            drw.line(&drw, sx, y, dx, y + ygradient);
        }

        /* Now the arrow heads */
        if(hasArrows)
        {
            if(startCol < endCol)
            {
                arrowR(dx, y + ygradient, arcType);
            }
            else
            {
                arrowL(dx, y + ygradient, arcType);
            }

            if(hasBiArrows)
            {
                if(startCol < endCol)
                {
                    arrowL(sx, y + ygradient, arcType);
                }
                else
                {
                    arrowR(sx, y + ygradient, arcType);
                }
            }
        }
    }
    else if(startCol < (MscGetNumEntities(m) / 2))
    {
        /* Arc looping to the left */
        if(arcType == MSC_ARC_RETVAL)
        {
            drw.dottedArc(&drw,
                          sx, y,
                          gOpts.entitySpacing,
                          gOpts.arcSpacing / 2,
                          90,
                          270);
        }
        else if(arcType == MSC_ARC_DOUBLE)
        {
            drw.arc(&drw,
                    sx, y - 1,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    90,
                    270);
            drw.arc(&drw,
                    sx, y + 1,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    90,
                    270);
        }
        else if(arcType == MSC_ARC_LOSS)
        {
            unsigned int mx = sx - (gOpts.entitySpacing / 2) + 4;

            drw.arc(&drw,
                    sx, y - 1,
                    gOpts.entitySpacing - 8,
                    gOpts.arcSpacing / 2,
                    180,
                    270);

            hasArrows = 0;

            /* Draw a cross */
            drw.line(&drw, mx - 4, y - 5, mx + 4, y + 3);
            drw.line(&drw, mx + 4, y - 5, mx - 4, y + 3);
        }
        else
        {
            drw.arc(&drw,
                    sx, y,
                    gOpts.entitySpacing - 4,
                    gOpts.arcSpacing / 2,
                    90,
                    270);
        }

        if(hasArrows)
        {
            arrowR(dx, y + (gOpts.arcSpacing / 4), arcType);
        }
    }
    else
    {
        /* Arc looping to right */
        if(arcType == MSC_ARC_RETVAL)
        {
            drw.dottedArc(&drw,
                          sx, y,
                          gOpts.entitySpacing,
                          gOpts.arcSpacing / 2,
                          270,
                          90);
        }
        else if(arcType == MSC_ARC_DOUBLE)
        {
            drw.arc(&drw,
                    sx, y - 1,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    270,
                    90);
            drw.arc(&drw,
                    sx, y + 1,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    270,
                    90);
        }
        else if(arcType == MSC_ARC_LOSS)
        {
            unsigned int mx = sx + (gOpts.entitySpacing / 2) - 4;

            drw.arc(&drw,
                    sx, y - 1,
                    gOpts.entitySpacing - 8,
                    gOpts.arcSpacing / 2,
                    270,
                    0);

            hasArrows = 0;

            /* Draw a cross */
            drw.line(&drw, mx - 4, y - 5, mx + 4, y + 3);
            drw.line(&drw, mx + 4, y - 5, mx - 4, y + 3);
        }
        else
        {
            drw.arc(&drw,
                    sx, y,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    270,
                    90);
        }

        if(hasArrows)
        {
            arrowL(dx, y + (gOpts.arcSpacing / 4), arcType);
        }
    }

    /* Restore pen if needed */
    if(arcLineCol != NULL)
    {
        drw.setPen(&drw, ADRAW_COL_BLACK);
    }
}


int main(const int argc, const char *argv[])
{
    FILE            *ismap = NULL;
    ADrawOutputType  outType;
    ADrawColour     *entColourRef;
    char            *outImage;
    Msc              m;
    unsigned int     ymin, ymax, w, h, row, col;
    unsigned int    *rowHeights;
    Boolean          addLines;
    float            f;

    /* Parse the command line options */
    if(!CmdParse(gClSwitches, sizeof(gClSwitches) / sizeof(CmdSwitch), argc - 1, &argv[1], "-i"))
    {
        Usage();
        return EXIT_FAILURE;
    }

    if(gDumpLicencePresent)
    {
        Licence();
        return EXIT_SUCCESS;
    }

    /* Check that the output type was specified */
    if(!gOutTypePresent)
    {
        fprintf(stderr, "-T <type> must be specified on the command line\n");
        Usage();
        return EXIT_FAILURE;
    }

    /* Check that the output filename was specified */
    if(!gOutputFilePresent)
    {
        if(!gInputFilePresent || strcmp(gInputFile, "-") == 0)
        {
            fprintf(stderr, "-o <filename> must be specified on the command line if -i is not used or input is from stdin\n");
            Usage();
            return EXIT_FAILURE;
        }

        gOutputFilePresent = TRUE;
        snprintf(gOutputFile, sizeof(gOutputFile), "%s", gInputFile);
        trimExtension(gOutputFile);
        strncat(gOutputFile, ".", sizeof(gOutputFile) - (strlen(gOutputFile) + 1));
        strncat(gOutputFile, gOutType, sizeof(gOutputFile) - (strlen(gOutputFile) + 1));
    }
#ifdef USE_FREETYPE
    /* Check for an output font name from the environment */
    if(!gOutputFontPresent)
    {
        const char *envFont = getenv("MSCGEN_FONT");
        const int   bufLen  = sizeof(gOutputFont);

        if(!envFont)
        {
            /* Pick a default font */
            snprintf(gOutputFont, bufLen, "helvetica");
        }
        else if(snprintf(gOutputFont, bufLen, "%s", envFont) >= bufLen)
        {
            fprintf(stderr, "MSCGEN_FONT font name too long (must be < %d characters)\n",
                    bufLen);
            return EXIT_FAILURE;
        }
    }
#else
    if(gOutputFontPresent)
    {
      fprintf(stderr, "Note: -F option specified but ignored since mscgen was not built\n"
                      "      with USE_FREETYPE.\n");
    }
#endif
    /* Determine the output type */
    if(strcmp(gOutType, "png") == 0)
    {
        outType  = ADRAW_FMT_PNG;
        outImage = gOutputFile;
    }
    else if(strcmp(gOutType, "eps") == 0)
    {
        outType  = ADRAW_FMT_EPS;
        outImage = gOutputFile;
    }
    else if(strcmp(gOutType, "svg") == 0)
    {
        outType  = ADRAW_FMT_SVG;
        outImage = gOutputFile;
    }
    else if(strcmp(gOutType, "ismap") == 0)
    {
#ifdef __WIN32__
        /* When building for Windows , use the 'dangerous' tempnam()
         *  as mkstemp() isn't present.
         */
        outType  = ADRAW_FMT_PNG;
        outImage = tempnam(NULL, "png");
        if(!outImage)
        {
            perror("tempnam() failed");
            return EXIT_FAILURE;
        }

        /* Schedule the temp file to be deleted */
        deleteTmpFilename = outImage;
        atexit(deleteTmp);
#else
        static char tmpTemplate[] = "/tmp/mscgenXXXXXX";
        int h;

        outType  = ADRAW_FMT_PNG;
        outImage = tmpTemplate;

        /* Create temporary file */
        h = mkstemp(tmpTemplate);
        if(h == -1)
        {
            perror("mkstemp() failed");
            return EXIT_FAILURE;
        }

        /* Close the file handle */
        close(h);

        /* Schedule the temp file to be deleted */
        deleteTmpFilename = outImage;
        atexit(deleteTmp);
#endif
    }
    else
    {
        fprintf(stderr, "Unknown output format '%s'\n", gOutType);
        Usage();
        return EXIT_FAILURE;
    }

    /* Parse input, either from a file, or stdin */
    if(gInputFilePresent && !strcmp(gInputFile, "-") == 0)
    {
        FILE *in = fopen(gInputFile, "r");

        if(!in)
        {
            fprintf(stderr, "Failed to open input file '%s'\n", gInputFile);
            return EXIT_FAILURE;
        }
        m = MscParse(in);
        fclose(in);
    }
    else
    {
        m = MscParse(stdin);
    }

    /* Check if the parse was okay */
    if(!m)
    {
        return EXIT_FAILURE;
    }

    /* Print the parse output if requested */
    if(gPrintParsePresent)
    {
        MscPrint(m);
    }

#ifndef USE_FREETYPE
    if(outType == ADRAW_FMT_PNG && lex_getutf8())
    {
        fprintf(stderr, "Warning: Optional UTF-8 byte-order-mark detected at start of input, but mscgen\n"
                        "         was not configured to use FreeType for text rendering.  Rendering of\n"
                        "         UTF-8 characters in PNG output may be incorrect.\n");
    }
#endif

    /* Check if an ismap file should also be generated */
    if(strcmp(gOutType, "ismap") == 0)
    {
        ismap = fopen(gOutputFile, "w");
        if(!ismap)
        {
            fprintf(stderr, "Failed to open output file '%s':\n", gOutputFile);
            perror(NULL);
            return EXIT_FAILURE;
        }
    }

    /* Open the drawing context with dummy dimensions */
    if(!ADrawOpen(10, 10, outImage, gOutputFont, outType, &drw))
    {
        fprintf(stderr, "Failed to create output context\n");
        return EXIT_FAILURE;
    }

    /* Now compute ideal canvas size, which may use text metrics */
    if(MscGetOptAsFloat(m, MSC_OPT_WIDTH, &f))
    {
        gOpts.idealCanvasWidth = f;
    }
    else if(MscGetOptAsFloat(m, MSC_OPT_HSCALE, &f))
    {
        gOpts.idealCanvasWidth *= f;
    }

    /* Set the arc gradient if needed */
    if(MscGetOptAsFloat(m, MSC_OPT_ARCGRADIENT, &f))
    {
        gOpts.arcGradient = (int)f;
        gOpts.arcSpacing += gOpts.arcGradient;
    }

    /* Check if word wrapping on arcs other than boxes should be used */
    MscGetOptAsBoolean(m, MSC_OPT_WORDWRAPARCS, &gOpts.wordWrapArcLabels);

    /* Work out the entitySpacing */
    if(gOpts.idealCanvasWidth / MscGetNumEntities(m) > gOpts.entitySpacing)
    {
        gOpts.entitySpacing = gOpts.idealCanvasWidth / MscGetNumEntities(m);
    }

    /* Work out the entityHeadGap */
    MscResetEntityIterator(m);
    for(col = 0; col < MscGetNumEntities(m); col++)
    {
        unsigned int lines = countLines(MscGetCurrentEntAttrib(m, MSC_ATTR_LABEL));
        unsigned int gap;

        /* Get the required gap */
        gap = lines * drw.textHeight(&drw);
        if(gap > gOpts.entityHeadGap)
        {
            gOpts.entityHeadGap = gap;
        }

        MscNextEntity(m);
    }

    /* Work out the width and height of the canvas */
    rowHeights = computeCanvasSize(m, &w , &h);

    /* Close the temporary output file */
    drw.close(&drw);

    /* Open the output */
    if(!ADrawOpen(w, h, outImage, gOutputFont, outType, &drw))
    {
        fprintf(stderr, "Failed to create output context\n");
        return EXIT_FAILURE;
    }

    /* Allocate storage for entity heading colours */
    entColourRef = malloc_s(MscGetNumEntities(m) * sizeof(ADrawColour));

    /* Draw the entity headings */
    MscResetEntityIterator(m);
    for(col = 0; col < MscGetNumEntities(m); col++)
    {
        unsigned int x = (gOpts.entitySpacing / 2) + (gOpts.entitySpacing * col);
        const char  *line;

        /* Titles */
        entityText(ismap,
                   x,
                   gOpts.entityHeadGap - (drw.textHeight(&drw) / 2),
                   MscGetCurrentEntAttrib(m, MSC_ATTR_LABEL),
                   MscGetCurrentEntAttrib(m, MSC_ATTR_URL),
                   MscGetCurrentEntAttrib(m, MSC_ATTR_ID),
                   MscGetCurrentEntAttrib(m, MSC_ATTR_IDURL),
                   MscGetCurrentEntAttrib(m, MSC_ATTR_TEXT_COLOUR),
                   MscGetCurrentEntAttrib(m, MSC_ATTR_TEXT_BGCOLOUR));

        /* Get the colours */
        line = MscGetCurrentEntAttrib(m, MSC_ATTR_LINE_COLOUR);
        if(line != NULL)
        {
            entColourRef[col] = ADrawGetColour(line);
        }
        else
        {
            entColourRef[col] = ADRAW_COL_BLACK;
        }

        MscNextEntity(m);
    }

    /* Draw the arcs */
    addLines = TRUE;
    ymin = gOpts.entityHeadGap;
    row = 0;

    MscResetArcIterator(m);
    do
    {
        const MscArcType   arcType           = MscGetCurrentArcType(m);
        const char        *arcUrl            = MscGetCurrentArcAttrib(m, MSC_ATTR_URL);
        const char        *arcId             = MscGetCurrentArcAttrib(m, MSC_ATTR_ID);
        const char        *arcIdUrl          = MscGetCurrentArcAttrib(m, MSC_ATTR_IDURL);
        const char        *arcTextColour     = MscGetCurrentArcAttrib(m, MSC_ATTR_TEXT_COLOUR);
        const char        *arcTextBgColour   = MscGetCurrentArcAttrib(m, MSC_ATTR_TEXT_BGCOLOUR);
        const char        *arcLineColour     = MscGetCurrentArcAttrib(m, MSC_ATTR_LINE_COLOUR);
        const int          arcGradient       = getArcGradient(m);
        const int          arcHasArrows      = MscGetCurrentArcAttrib(m, MSC_ATTR_NO_ARROWS) == NULL;
        const int          arcHasBiArrows    = MscGetCurrentArcAttrib(m, MSC_ATTR_BI_ARROWS) != NULL;
        char             **arcLabelLines     = NULL;
        unsigned int       arcLabelLineCount = 0;
        int                startCol = -1, endCol = -1;

        if(arcType == MSC_ARC_PARALLEL)
        {
            addLines = FALSE;

            /* Rewind the row */
            assert(row > 1);
            row--;
            ymin -= rowHeights[row] + 1;
        }
        else
        {
            /* Get the entity indices */
            if(arcType != MSC_ARC_DISCO && arcType != MSC_ARC_DIVIDER && arcType != MSC_ARC_SPACE)
            {
                startCol = MscGetEntityIndex(m, MscGetCurrentArcSource(m));
                endCol   = MscGetEntityIndex(m, MscGetCurrentArcDest(m));

                /* Check that the start column is known */
                if(startCol == -1)
                {
                    fprintf(stderr,
                            "Unknown source entity '%s'\n",
                            MscGetCurrentArcSource(m));
                    return EXIT_FAILURE;
                }

                /* Check that the end column is known, or it's a broadcast arc */
                if(endCol == -1 && !isBroadcastArc(MscGetCurrentArcDest(m)))
                {
                    fprintf(stderr,
                            "Unknown destination entity '%s'\n",
                            MscGetCurrentArcDest(m));
                    return EXIT_FAILURE;
                }

                /* Check for entity colouring if not set explicity on the arc */
                if(arcTextColour == NULL)
                {
                    arcTextColour = MscGetEntAttrib(m, startCol, MSC_ATTR_ARC_TEXT_COLOUR);
                }

                if(arcTextBgColour == NULL)
                {
                    arcTextBgColour = MscGetEntAttrib(m, startCol, MSC_ATTR_ARC_TEXT_BGCOLOUR);
                }

                if(arcLineColour == NULL)
                {
                    arcLineColour = MscGetEntAttrib(m, startCol, MSC_ATTR_ARC_LINE_COLOUR);
                }

            }
            else
            {
                /* Discontinuity or parallel arc spans whole chart */
                startCol = 0;
                endCol   = MscGetNumEntities(m) - 1;
            }

            /* Work out how the label fits the gap between entities */
            arcLabelLineCount = computeLabelLines(m, arcType, &arcLabelLines,
                                                  MscGetCurrentArcAttrib(m, MSC_ATTR_LABEL),
                                                  startCol, endCol);

            /* Compute the height of this arc */
            ymax = ymin + gOpts.arcSpacing;
            if(arcLabelLineCount > 1)
            {
                ymax += (arcLabelLineCount - 2) * drw.textHeight(&drw);
            }

            /* Check if this is a broadcast message */
            if(isBroadcastArc(MscGetCurrentArcDest(m)))
            {
                unsigned int t;

                /* Add in the entity lines */
                if(addLines)
                {
                    entityLines(m, ymin, ymin + rowHeights[row], FALSE, entColourRef);
                }

                /* Draw arcs to each entity */
                for(t = 0; t < MscGetNumEntities(m); t++)
                {
                    if((signed)t != startCol)
                    {
                        arcLine(m, ymin, ymax, arcGradient, startCol, t,
                                arcLineColour, arcHasArrows, arcHasBiArrows, arcType);
                    }
                }

                /* Fix up the start/end columns to span chart */
                startCol = 0;
                endCol   = MscGetNumEntities(m) - 1;
            }
            else
            {
                /* Check if it is a box, discontinuity arc etc... */
                if(isBoxArc(arcType))
                {
                    if(addLines)
                    {
                        entityLines(m, ymin, ymin + rowHeights[row], FALSE, entColourRef);
                    }
                    entityBox(ymin, ymax, startCol, endCol, arcType, arcLineColour);
                }
                else if(arcType == MSC_ARC_DISCO && addLines)
                {
                    entityLines(m, ymin, ymin + rowHeights[row], TRUE /* dotted */, entColourRef);
                }
                else if(arcType == MSC_ARC_DIVIDER || arcType == MSC_ARC_SPACE)
                {
                    if(addLines)
                    {
                        entityLines(m, ymin, ymin + rowHeights[row], FALSE, entColourRef);
                    }
                }
                else
                {
                    if(addLines)
                    {
                        entityLines(m, ymin, ymin + rowHeights[row], FALSE, entColourRef);
                    }
                    arcLine(m, ymin, ymax, arcGradient, startCol, endCol,
                            arcLineColour, arcHasArrows, arcHasBiArrows, arcType);
                }
            }

            /* All may have text */
            if(arcLabelLineCount > 0)
            {
                arcText(m, ismap, w, ymin, arcGradient,
                        startCol, endCol,
                        arcLabelLineCount, arcLabelLines,
                        arcUrl, arcId, arcIdUrl,
                        arcTextColour, arcTextBgColour, arcLineColour, arcType);
            }

            freeLabelLines(arcLabelLineCount, arcLabelLines);

            /* Advance Y position */
            ymin += rowHeights[row] + 1;
            row++;

            addLines = TRUE;
        }
    }
    while(MscNextArc(m));

    /* The computed canvas size may have been extended for skip arcs */
    if(ymax < h)
    {
        entityLines(m, ymax, h, FALSE, entColourRef);
    }

    /* Close the image map if needed */
    if(ismap)
    {
        fclose(ismap);
    }

#ifndef NDEBUG
    /* Free the allocated memory to allow leak detection */
    free(entColourRef);
    free(rowHeights);
    MscFree(m);
#endif

    /* Close the context */
    drw.close(&drw);

    return EXIT_SUCCESS;
}

/* END OF FILE */
