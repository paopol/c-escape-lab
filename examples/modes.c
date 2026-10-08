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
    /* DEC private mode: show cursor (DECTCEM = 25). */
    printf("Show-cursor sequences (DECTCEM = 25):\n");
    printf("  SM (on):  "); show(EST_CSI_SM_DEC_PRIVATE_MODE_STR("25")); printf("\n");
    printf("  RM (off): "); show(EST_CSI_RM_DEC_PRIVATE_MODE_STR("25")); printf("\n\n");

    /* ANSI standard mode: insert mode (IRM = 4). */
    printf("Insert-mode sequences (IRM = 4):\n");
    printf("  SM: "); show(EST_CSI_SM_ANSI_STANDARD_MODE_STR("4")); printf("\n");
    printf("  RM: "); show(EST_CSI_RM_ANSI_STANDARD_MODE_STR("4")); printf("\n\n");

    /* Mode query (DECRQM). */
    printf("Request mode (DECRQM 25): ");
    show(EST_CSI_DECRQM_DEC_PRIVATE_MODE_STR("25"));
    printf("\n\n");

    /* Device-status queries: these macros only SEND the request; reading and
       parsing the terminal's response is outside this library's scope. */
    printf("Device-status requests (send-only):\n");
    printf("  DSR:       "); show(EST_CSI_DSR_STR()); printf("\n");
    printf("  CPR:       "); show(EST_CSI_CPR_STR()); printf("\n");
    printf("  DA1:       "); show(EST_CSI_DA1_STR()); printf("\n");
    printf("  DA2:       "); show(EST_CSI_DA2_STR()); printf("\n");
    printf("  XTVERSION: "); show(EST_CSI_XTVERSION_STR()); printf("\n");

    return 0;
}
