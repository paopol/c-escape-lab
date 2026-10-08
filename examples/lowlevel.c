#include <stdio.h>
#include "escape.h"

/* Print a sequence with ESC shown as "\e" instead of being executed. */
static void show(const char *s)
{
    for (; *s; s++)
    {
        unsigned char c = (unsigned char)*s;
        if (c == 0x1b)
            printf("\\e");
        else if (c >= 0x20 && c <= 0x7e)
            putchar(c);
        else
            printf("\\x%02x", c);
    }
}

int main(void)
{
    printf("Low-level builders (ESC is shown as \\e):\n");

    /* ESC: ESC <final byte>. */
    printf("  EST_ESC_NOI_STR(\"7\"):             "); show(EST_ESC_NOI_STR("7")); printf("\n");

    /* CSI: ESC [ <params> <intermediates> <final>. */
    printf("  EST_CSI_STR(\"10;20\", \"\", \"H\"):      "); show(EST_CSI_STR("10;20", "", "H")); printf("\n");

    /* OSC: ESC ] <payload> ST. */
    printf("  EST_OSC_STR(\"0;title\"):           "); show(EST_OSC_STR("0;title")); printf("\n");

    /* DCS: ESC P <params> <intermediates> <final> <data> ST. */
    printf("  EST_DCS_STR(\"\", \">\", \"|\", \"1.0\"):  "); show(EST_DCS_STR("", ">", "|", "1.0")); printf("\n");

    /* APC / ST / SOS / PM. */
    printf("  EST_APC_STR(\"payload\"):           "); show(EST_APC_STR("payload")); printf("\n");
    printf("  EST_ST_STR():                    "); show(EST_ST_STR()); printf("\n");
    printf("  EST_SOS_STR(\"s\"):                "); show(EST_SOS_STR("s")); printf("\n");
    printf("  EST_PM_STR(\"p\"):                 "); show(EST_PM_STR("p")); printf("\n");

    return 0;
}
