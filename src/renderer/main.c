/***************************************************************************
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "cmdparse.h"
#include "adraw.h"
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

    /** Radius of rounded box corner arcs. */
    unsigned int rboxArc;

    /** Anguluar box slope in pixels. */
    unsigned int aboxSlope;

    /** Horizontal width of the arrow heads. */
    unsigned int arrowWidth;

    /** Vertical depth of the arrow heads. */
    unsigned int arrowHeight;

    /** Horizontal gap between text and horizonal lines. */
    unsigned int textHGapPre;

    /** Horizontal gap between text and horizonal lines. */
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

/** Command line switches.
 * This gives the command line switches that can be interpreted by mscgen.
 */
static CmdSwitch gClSwitches[] =
{
    {"-i",     &gInputFilePresent,  "%4096[^?]", gInputFile },
    {"-o",     &gOutputFilePresent, "%4096[^?]", gOutputFile },
    {"-T",     &gOutTypePresent,    "%10[^?]",   gOutType },
    {"-l",     &gDumpLicencePresent,NULL,        NULL },
    {"-p",     &gPrintParsePresent, NULL,        NULL }
};


static GlobalOptions gOpts =
{
    600,    /* idealCanvasWidth */

    80,     /* entitySpacing */
    20,     /* entityHeadGap */
    25,     /* arcSpacing */
    0,      /* arcGradient */
    8,      /* boxSpacing */
    6,      /* rboxArc */
    6,      /* aboxSlope */

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



/** Count the number of lines in some string.
 * This counts line breaks that are written as a litteral '\n' in the line.
 *
 * \param[in] l  Pointer to the input string to inspect.
 * \retuns       The count of lines that should be output for the given string.
 */
static unsigned int countLines(const char *l)
{
    unsigned int c = 0;

    do
    {
        c++;

        l = strstr(l, "\\n");
        if(l) l += 2;
    }
    while(l != NULL);

    return c;
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
Boolean isBroadcastArc(const char *entity)
{
    return entity != NULL && (strcmp(entity, "*") == 0);
}


/** Check if some arc type indicates a box.
 */
Boolean isBoxArc(const MscArcType a)
{
    return a == MSC_ARC_BOX || a == MSC_ARC_RBOX  || a == MSC_ARC_ABOX;
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
                     x, y,
                     x - gOpts.arrowWidth, y - gOpts.arrowHeight);
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
 * \param  ismap     If not \a NULL, write an ismap description here.
 * \param  x         The x position at which the entity text should be centered.
 * \param  y         The y position where the text should be placed
 * \param  entLabel  The label to render, which maybe \a NULL in which case
 *                     no ouput is produced.
 * \param  entUrl    The URL for rendering the label as a hyperlink.  This
 *                     maybe \a NULL if not required.
 * \param  entId     The text identifier for the arc.
 * \param  entIdUrl  The URL for rendering the test identifier as a hyperlink.
 *                     This maybe \a NULL if not required.
 * \param  entColour The text colour name or specification for the entity text.
 *                    If NULL, use default colouring scheme.
 */
static void entityText(FILE             *ismap,
                       unsigned int      x,
                       unsigned int      y,
                       const char       *entLabel,
                       const char       *entUrl,
                       const char       *entId,
                       const char       *entIdUrl,
                       const char       *entColour)
{
    unsigned int lines = countLines(entLabel);
    unsigned int l;
    char         lineBuffer[1024];

    if(entLabel)
    {
        /* Adjust y to be above the writing line */
        y -= drw.textHeight(&drw) * lines;

        for(l = 0; l < lines; l++)
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

            /* Set to the explicit colour if directed */
            if(entColour != NULL)
            {
                drw.setPen(&drw, ADrawGetColour(entColour));
            }

            /* Render text and restore pen */
            drw.textC (&drw, x, y, lineLabel);
            drw.setPen(&drw, ADRAW_COL_BLACK);

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


/** Draw vertical lines stemming from entites.
 * This function will draw a single segment of the vertical line that
 * drops from an entity.
 * \param m          The \a Msc for which the lines are drawn
 * \param row        The row number indentifying the segment
 * \param dotted     If #TRUE, produce a dotted line, otherwise solid.
 * \param colourRefs Colour references for each entity.
 */
static void entityLines(Msc                m,
                        unsigned int       row,
                        Boolean            dotted,
                        const ADrawColour *colourRefs)
{
    const unsigned int ymin = (gOpts.arcSpacing * row) + gOpts.entityHeadGap;
    const unsigned int ymax = ymin + gOpts.arcSpacing;
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



/** Draw vertical lines and boxes stemming from entites.
 * \param row        The row number indentifying the segment
 * \param boxStart   Column in which the box starts.
 * \param boxEnd     Column in which the box ends.
 * \param boxType    The type of box to draw, MSC_ARC_BOX, MSC_ARC_RBOX etc.
 */
static void entityBox(unsigned int       row,
                      unsigned int       boxStart,
                      unsigned int       boxEnd,
                      MscArcType         boxType)
{
    const unsigned int ymin = (gOpts.arcSpacing * row) + gOpts.entityHeadGap;
    unsigned int       ymax = ymin + gOpts.arcSpacing;
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
    drw.setPen(&drw, ADRAW_COL_BLACK);

    /* Draw the outline */
    switch(boxType)
    {
        case MSC_ARC_BOX:
            drw.line(&drw, x1, ymin, x2, ymin);
            drw.line(&drw, x1, ymax, x2, ymax);
            drw.line(&drw, x1, ymin, x1, ymax);
            drw.line(&drw, x2, ymin, x2, ymax);
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
            drw.line(&drw, x1 + gOpts.aboxSlope, ymax, x1, ymid);
            drw.line(&drw, x2 - gOpts.aboxSlope, ymin, x2, ymid);
            drw.line(&drw, x2 - gOpts.aboxSlope, ymax, x2, ymid);
            break;

        default:
            assert(0);
    }
}


/** Render text on an arc.
 * Draw the text on some arc.
 * \param m              The Msc for which the text is being rendered.
 * \param ismap          If not \a NULL, write an ismap description here.
 * \param outwidth       Width of the output image.
 * \param row            The row on which the text should be placed.
 * \param startCol       The column at which the arc being labelled starts.
 * \param endCol         The column at which the arc being labelled ends.
 * \param arcLabel       The label to render.
 * \param arcLabelLines  Count of lines of text in arcLabel.
 * \param arcUrl         The URL for rendering the label as a hyperlink.  This
 *                        maybe \a NULL if not required.
 * \param arcId          The text identifier for the arc.
 * \param arcIdUrl       The URL for rendering the test identifier as a hyperlink.
 *                        This maybe \a NULL if not required.
 * \param arcTextColour  Colout for the arc text, or NULL to use default.
 * \param arcType        The type of arc, used to control output semantics.
 */
static void arcText(Msc                m,
                    FILE              *ismap,
                    unsigned int       outwidth,
                    unsigned int       row,
                    unsigned int       startCol,
                    unsigned int       endCol,
                    const char        *arcLabel,
                    const unsigned int arcLabelLines,
                    const char        *arcUrl,
                    const char        *arcId,
                    const char        *arcIdUrl,
                    const char        *arcTextColour,
                    const char        *arcLineColour,
                    const MscArcType   arcType)
{
    char         lineBuffer[1024];
    unsigned int l;

    for(l = 0; l < arcLabelLines; l++)
    {
        char *lineLabel = getLine(arcLabel, l, lineBuffer, sizeof(lineBuffer));
        unsigned int y = (gOpts.arcSpacing * row) +
                          gOpts.entityHeadGap + (gOpts.arcSpacing / 2) +
                          (l * drw.textHeight(&drw)) - l;
        unsigned int width = drw.textWidth(&drw, lineLabel);
        int x = ((startCol + endCol + 1) * gOpts.entitySpacing) / 2;

        /* Discontinunity arcs have central text, otherwise the
         *  label is above the rendered arc (or horizontally in the
         *  centre of a non-straight arc).
         */
        if(arcType == MSC_ARC_DISCO || arcType == MSC_ARC_DIVIDER || arcType == MSC_ARC_SPACE ||
           isBoxArc(arcType))
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


        /* Set to the explicit colour if directed */
        if(arcTextColour != NULL)
        {
            drw.setPen(&drw, ADrawGetColour(arcTextColour));
        }

        /* Render text and restore pen */
        drw.textR (&drw, x, y, lineLabel);
        drw.setPen(&drw, ADRAW_COL_BLACK);

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

        /* Dividers also have a horizontal line */
        if(arcType == MSC_ARC_DIVIDER)
        {
            const unsigned int margin = gOpts.entitySpacing / 4;

            y -= drw.textHeight(&drw) / 2;

            if(arcLineColour != NULL)
            {
                drw.setPen(&drw, ADrawGetColour(arcLineColour));
            }

            drw.dottedLine(&drw,
                           margin, y,
                           x - gOpts.textHGapPre,  y);
            drw.dottedLine(&drw,
                           x + width + gOpts.textHGapPost,
                           y,
                           (MscGetNumEntities(m) * gOpts.entitySpacing) - margin,
                           y);

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
 * \param  m        The Msc for which the text is being rendered.
 * \param  row      Row in the output at which the arc should be rendered.
 * \param  startCol Starting column for the arc.
 * \param  endCol   Column at which the arc terminates.
 * \param  arcType  The type of the arc, which dictates its rendered style.
 */
static void arcLine(Msc               m,
                    unsigned int      row,
                    unsigned int      startCol,
                    unsigned int      endCol,
                    const char       *arcLineCol,
                    const MscArcType  arcType)
{
    const unsigned int y  = (gOpts.arcSpacing * row) +
                             gOpts.entityHeadGap + (gOpts.arcSpacing / 2);
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
            drw.dottedLine(&drw, sx, y, dx, y + gOpts.arcGradient);
        }
        else if(arcType == MSC_ARC_DOUBLE)
        {
            drw.line(&drw, sx, y - 1, dx, y - 1 + gOpts.arcGradient);
            drw.line(&drw, sx, y + 1, dx, y + 1 + gOpts.arcGradient);
        }
        else
        {
            drw.line(&drw, sx, y, dx, y + gOpts.arcGradient);
        }

        /* Now the arrow heads */
        if(startCol < endCol)
        {
            arrowR(dx, y + gOpts.arcGradient, arcType);
        }
        else
        {
            arrowL(dx, y + gOpts.arcGradient, arcType);
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
        else
        {
            drw.arc(&drw,
                    sx, y,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    90,
                    270);
        }

        arrowR(dx, y + (gOpts.arcSpacing / 4), arcType);
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
        else
        {
            drw.arc(&drw,
                    sx, y,
                    gOpts.entitySpacing,
                    gOpts.arcSpacing / 2,
                    270,
                    90);
        }

        arrowL(dx, y + (gOpts.arcSpacing / 4), arcType);
    }

    /* Restore pen if needed */
    if(arcLineCol != NULL)
    {
        drw.setPen(&drw, ADRAW_COL_BLACK);
    }
}


/** Print program usage and return.
 */
static void usage()
{
    printf(
"Usage: mscgen -T <type> [-i <infile>] -o <file>\n"
"       mscgen -l\n"
"\n"
"Where:\n"
" -T <type>   Specifies the output file type, which maybe one of 'png', 'eps',\n"
"             'svg' or 'ismap'\n"
" -i <infile> The file from which to read input.  If omitted or specified as\n"
"             '-', input will be read from stdin.\n"
" -o <file>   Write output to the named file.  This option must be specified.\n"
" -p          Print parsed msc output (for parser debug).\n"
" -l          Display program licence and exit.\n"
"\n"
"Mscgen version %s, Copyright (C) 2007 Michael C McTernan,\n"
"                                        Michael.McTernan.2001@cs.bris.ac.uk\n"
"Mscgen comes with ABSOLUTELY NO WARRANTY.  This is free software, and you are\n"
"welcome to redistribute it under certain conditions; type `mscgen -l' for\n"
"details.\n",
PACKAGE_VERSION);
}


/** Print program licence and return.
 */
static void licence()
{
    printf(
"Mscgen, a message sequence chart renderer.\n"
"Copyright (C) 2007 Michael C McTernan, Michael.McTernan.2001@cs.bris.ac.uk\n"
"\n"
"TTPCom Ltd., hereby disclaims all copyright interest in the program `mscgen'\n"
"(which renders message sequence charts) written by Michael McTernan.\n"
"\n"
"Rob Meades of TTPCom Ltd, 1 August 2005\n"
"Rob Meades, director of Software\n"
"\n"
"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation; either version 2 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program; if not, write to the Free Software\n"
"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA\n");
}

int main(const int argc, const char *argv[])
{
    FILE            *ismap = NULL;
    ADrawOutputType  outType;
    ADrawColour     *entColourRef;
    char            *outImage;
    Msc              m;
    unsigned int     w, h, row, col, arc;
    Boolean          addLines;
    float            f;

    /* Parse the command line options */
    if(!CmdParse(gClSwitches, sizeof(gClSwitches) / sizeof(CmdSwitch), argc - 1, &argv[1]))
    {
        usage();
        return EXIT_FAILURE;
    }

    if(gDumpLicencePresent)
    {
        licence();
        return EXIT_SUCCESS;
    }

    /* Check that the ouput type was specified */
    if(!gOutTypePresent || !gOutputFilePresent)
    {
        fprintf(stderr, "-T <type> must be specified on the command line\n"
                        "-o <file> must be specified on the command line\n");
        usage();
        return EXIT_FAILURE;
    }

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
        usage();
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
    if(!ADrawOpen(10, 10, outImage, outType, &drw))
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
        gap = (lines + 1) * drw.textHeight(&drw);
        if(gap > gOpts.entityHeadGap)
        {
            gOpts.entityHeadGap = gap;
        }

        MscNextEntity(m);
    }

    /* Work out the width and height of the canvas */
    w = MscGetNumEntities(m) * gOpts.entitySpacing;
    h = ((MscGetNumArcs(m) - MscGetNumParallelArcs(m)) * gOpts.arcSpacing) +
        gOpts.entityHeadGap;

    /* Close the temporary output file */
    drw.close(&drw);

    /* Open the output */
    if(!ADrawOpen(w, h, outImage, outType, &drw))
    {
        fprintf(stderr, "Failed to create output context\n");
        return EXIT_FAILURE;
    }

    /* Allocate storage for entity heading colours */
    entColourRef = malloc(MscGetNumEntities(m) * sizeof(ADrawColour));

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
                   MscGetCurrentEntAttrib(m, MSC_ATTR_TEXT_COLOUR));

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
    row = 0; arc = 0;
    while(arc < MscGetNumArcs(m))
    {
        const MscArcType   arcType       = MscGetCurrentArcType(m);
        const char        *arcUrl        = MscGetCurrentArcAttrib(m, MSC_ATTR_URL);
        const char        *arcLabel      = MscGetCurrentArcAttrib(m, MSC_ATTR_LABEL);
        const char        *arcId         = MscGetCurrentArcAttrib(m, MSC_ATTR_ID);
        const char        *arcIdUrl      = MscGetCurrentArcAttrib(m, MSC_ATTR_IDURL);
        const char        *arcTextColour = MscGetCurrentArcAttrib(m, MSC_ATTR_TEXT_COLOUR);
        const char        *arcLineColour = MscGetCurrentArcAttrib(m, MSC_ATTR_LINE_COLOUR);
        const unsigned int arcLabelLines = arcLabel ? countLines(arcLabel) : 1;
        int                startCol, endCol;

        if(arcType == MSC_ARC_PARALLEL)
        {
            row--;
            addLines = FALSE;
        }
        else
        {
            /* Get the entitiy indices */
            if(arcType != MSC_ARC_DISCO && arcType != MSC_ARC_DIVIDER && arcType != MSC_ARC_SPACE)
            {
                startCol = MscGetEntityIndex(m, MscGetCurrentArcSource(m));
                endCol   = MscGetEntityIndex(m, MscGetCurrentArcDest(m));

                /* Check for entity arc colouring if not set explicity on the arc */
                if(arcTextColour == NULL)
                {
                    arcTextColour = MscGetEntAttrib(m, startCol, MSC_ATTR_ARC_TEXT_COLOUR);
                    arcLineColour = MscGetEntAttrib(m, startCol, MSC_ATTR_ARC_LINE_COLOUR);
                }

            }
            else
            {
                /* Discontinuity or parallel arc spans whole chart */
                startCol = 0;
                endCol   = MscGetNumEntities(m) - 1;
            }

            /* Check if this is a broadcast message */
            if(isBroadcastArc(MscGetCurrentArcDest(m)))
            {
                unsigned int t;

                /* Ensure startCol is valid */
                if(startCol == -1)
                {
                   fprintf(stderr,
                           "Unknown source entity '%s'\n",
                           MscGetCurrentArcSource(m));
                   return EXIT_FAILURE;
                }

                /* Add in the entity lines */
                if(addLines)
                {
                    entityLines(m, row, FALSE, entColourRef);
                }

                /* Draw arcs to each entity */
                for(t = 0; t < MscGetNumEntities(m); t++)
                {
                    if((signed)t != startCol)
                    {
                        arcLine(m, row, startCol, t, arcLineColour, arcType);
                    }
                }

                /* Fix up the start/end columns to span chart */
                startCol = 0;
                endCol   = MscGetNumEntities(m) - 1;
            }
            else
            {
                /* Check that the start and end columns are known */
                if(startCol == -1)
                {
                    fprintf(stderr,
                            "Unknown source entity '%s'\n",
                            MscGetCurrentArcSource(m));
                    return EXIT_FAILURE;
                }

                if(endCol == -1)
                {
                    fprintf(stderr,
                            "Unknown destination entity '%s'\n",
                            MscGetCurrentArcDest(m));
                    return EXIT_FAILURE;
                }

                /* Check if it is a box, discontinunity arc etc... */
                if(isBoxArc(arcType))
                {
                    if(addLines)
                    {
                        entityLines(m, row, FALSE, entColourRef);
                    }
                    entityBox(row, startCol, endCol, arcType);
                }
                else if(arcType == MSC_ARC_DISCO && addLines)
                {
                    entityLines(m, row, TRUE /* dotted */, entColourRef);
                }
                else if ((arcType == MSC_ARC_DIVIDER || arcType == MSC_ARC_SPACE) && addLines)
                {
                    entityLines(m, row, FALSE, entColourRef);
                }
                else
                {
                    if(addLines)
                    {
                        entityLines(m, row, FALSE, entColourRef);
                    }
                    arcLine    (m, row, startCol, endCol, arcLineColour, arcType);
                }
            }

            /* All may have text */
            if(arcLabel)
            {
                arcText(m, ismap, w, row,
                        startCol, endCol, arcLabel, arcLabelLines,
                        arcUrl, arcId, arcIdUrl,
                        arcTextColour, arcLineColour, arcType);
            }

            row++;
            addLines = TRUE;
        }

        MscNextArc(m);
        arc++;
    }

    /* Close the image map if needed */
    if(ismap)
    {
        fclose(ismap);
    }

    /* Close the context */
    drw.close(&drw);

    return EXIT_SUCCESS;
}

/* END OF FILE */
