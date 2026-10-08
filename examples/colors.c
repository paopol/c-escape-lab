#include <stdio.h>
#include "escape.h"

int main(void)
{
    /* Text styles: ESCAPE_COLORIZE_SSTR appends a reset after the text. */
    printf("Styles      ");
    printf(ESCAPE_COLORIZE_SSTR("bold", EST_CSI_SGR_BOLD) " ");
    printf(ESCAPE_COLORIZE_SSTR("italic", EST_CSI_SGR_ITALIC) " ");
    printf(ESCAPE_COLORIZE_SSTR("underline", EST_CSI_SGR_UNDERLINE) " ");
    printf(ESCAPE_COLORIZE_SSTR("reverse", EST_CSI_SGR_REVERSE_VIDEO) " ");
    printf(ESCAPE_COLORIZE_SSTR("strike", EST_CSI_SGR_STRIKE) "\n\n");

    /* The 16 base colors as background swatches (40..47, then 100..107). */
    printf("Base 16     ");
    for (int i = 0; i < 16; i++)
    {
        char seq[32];
        int code = (i < 8) ? (40 + i) : (100 + i - 8);
        snprintf(seq, sizeof seq, EST_CSI_SGR_STR("%d"), code);
        fputs(seq, stdout);
        fputs("  ", stdout);
    }
    fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
    printf("\n\n");

    /* 256-color foreground samples. */
    printf("256-color   ");
    printf(EST_CSI_SGR_FC_EXT_256_SSTR(196) "196" EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) " ");
    printf(EST_CSI_SGR_FC_EXT_256_SSTR(46)  "46"  EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) " ");
    printf(EST_CSI_SGR_FC_EXT_256_SSTR(21)  "21"  EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n\n");

    /* Truecolor foreground samples. */
    printf("Truecolor   ");
    printf(EST_CSI_SGR_FC_EXT_TRUE_SSTR(255, 80, 20)  "orange" EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) " ");
    printf(EST_CSI_SGR_FC_EXT_TRUE_SSTR(0, 180, 120)  "green"  EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) " ");
    printf(EST_CSI_SGR_FC_EXT_TRUE_SSTR(120, 120, 255) "blue"  EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n");

    return 0;
}
