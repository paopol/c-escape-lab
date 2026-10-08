#include <stdio.h>
#include "escape.h"

/* Integer HSV hue wheel: h in [0, 60) -> s=1, v=1 -> RGB. */
static void rainbow(int h, int *r, int *g, int *b)
{
    int sector = h / 10;
    int t      = (h % 10) * 25;   /* 0..225 */
    int up     = t;
    int down   = 255 - t;

    switch (sector)
    {
        case 0: *r = 255; *g = up;   *b = 0;    break; /* red   -> yellow  */
        case 1: *r = down; *g = 255; *b = 0;    break; /* yellow-> green   */
        case 2: *r = 0;   *g = 255; *b = up;    break; /* green -> cyan    */
        case 3: *r = 0;   *g = down; *b = 255;  break; /* cyan  -> blue    */
        case 4: *r = up;  *g = 0;   *b = 255;   break; /* blue  -> magenta */
        default:*r = 255; *g = 0;   *b = down;  break; /* magenta-> red    */
    }
}

int main(void)
{
    char seq[64];

    /* Title */
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) EST_CSI_SGR_SSTR(EST_CSI_SGR_BFC_CYAN));
    printf("c-escape-lab");
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET));
    printf("  ");
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_FAINT) "header-only ANSI/VT control sequences");
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n\n");

    /* Text styles */
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) "Styles   ");
    printf(ESCAPE_COLORIZE_SSTR("bold", EST_CSI_SGR_BOLD) "  ");
    printf(ESCAPE_COLORIZE_SSTR("italic", EST_CSI_SGR_ITALIC) "  ");
    printf(ESCAPE_COLORIZE_SSTR("underline", EST_CSI_SGR_UNDERLINE) "  ");
    printf(ESCAPE_COLORIZE_SSTR("reverse", EST_CSI_SGR_REVERSE_VIDEO) "  ");
    printf(ESCAPE_COLORIZE_SSTR("strike", EST_CSI_SGR_STRIKE));
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n\n");

    /* Base 16 colors: normal (40..47) then bright (100..107). */
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) "Base 16 colors");
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n");
    for (int row = 0; row < 2; row++)
    {
        for (int i = 0; i < 8; i++)
        {
            snprintf(seq, sizeof seq, EST_CSI_SGR_STR("%d"), row * 60 + 40 + i);
            fputs(seq, stdout);
            fputs("  ", stdout);
        }
        fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
        fputs("\n", stdout);
    }
    fputs("\n", stdout);

    /* 256 colors as a 16x16 grid. */
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) "256 colors");
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n");
    for (int i = 0; i < 256; i++)
    {
        snprintf(seq, sizeof seq, EST_CSI_SGR_BC_EXT_256_STR("%d"), i);
        fputs(seq, stdout);
        fputs("  ", stdout);
        if ((i + 1) % 16 == 0)
        {
            fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
            fputs("\n", stdout);
        }
    }
    fputs("\n", stdout);

    /* Truecolor hue wheel. */
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) "Truecolor");
    printf(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n");
    for (int i = 0; i < 60; i++)
    {
        int r, g, b;
        rainbow(i, &r, &g, &b);
        snprintf(seq, sizeof seq, EST_CSI_SGR_BC_EXT_TRUE_STR("%d", "%d", "%d"), r, g, b);
        fputs(seq, stdout);
        fputs(" ", stdout);
    }
    fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
    fputs("\n", stdout);

    return 0;
}
